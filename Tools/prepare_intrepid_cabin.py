"""Recover only the retained cabin's authored GLB corner normals for a new UE asset.

This is an offline, read-only source processor. It does not write an OBJ, import
assets, repair inherited UVs, smooth geometry, or modify the original model.
The separate Editor builder duplicates the existing mesh, bakes its computed
render frames and repairs invalid tangent bases, then applies the selected GLB
normals. Geometry, UVs and materials are preserved by its independent receipt.
"""
from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import sys


TARGETS = frozenset({
    ("polySurfac.002", "initialShadingGr.001"),
    ("polySurfac.002_1", "blinn3SG.001"),
})
GEOMETRY_RECORDS = frozenset({"v", "vt", "f", "o", "g", "usemtl", "s"})
QUANTIZATION_CM = 0.0001


class MappingError(ValueError):
    """The source cannot be mapped without guessing."""


def sha256_bytes(data):
    return hashlib.sha256(data).hexdigest()


def source_path_label(path, output):
    """Keep bundled public derivatives reproducible without a user's paths."""
    try:
        return path.relative_to(output).as_posix()
    except ValueError:
        return str(path)


def canonical_records(text, kinds=GEOMETRY_RECORDS):
    records = []
    for line in text.splitlines():
        tokens = line.split()
        if tokens and tokens[0] in kinds:
            records.append(" ".join(tokens))
    return records


def record_hash(text, kinds=GEOMETRY_RECORDS):
    return sha256_bytes(("\n".join(canonical_records(text, kinds)) + "\n").encode("utf-8"))


def parse_source(text):
    vertices, texture_coordinates, faces = [], [], []
    object_name = material_name = ""
    for line_number, line in enumerate(text.splitlines(), 1):
        tokens = line.split()
        if not tokens:
            continue
        if tokens[0] == "v":
            if len(tokens) != 4:
                raise MappingError(f"Expected XYZ-only source vertex on line {line_number}")
            value = [float(value) for value in tokens[1:]]
            if not all(math.isfinite(value) for value in value):
                raise MappingError(f"Non-finite vertex on line {line_number}")
            vertices.append(value)
        elif tokens[0] == "vt":
            texture_coordinates.append([float(value) for value in tokens[1:]])
        elif tokens[0] == "vn":
            raise MappingError("Expected the retained original OBJ without normals")
        elif tokens[0] == "o":
            object_name = " ".join(tokens[1:])
        elif tokens[0] == "usemtl":
            material_name = " ".join(tokens[1:])
        elif tokens[0] == "f":
            if len(tokens) != 4:
                raise MappingError(f"Expected triangular source face on line {line_number}")
            corners = []
            for token in tokens[1:]:
                values = token.split("/")
                if len(values) > 2:
                    raise MappingError("Unexpected existing source corner normals")
                vertex_index = int(values[0])
                uv_index = int(values[1]) if len(values) > 1 and values[1] else None
                if vertex_index <= 0 or (uv_index is not None and uv_index <= 0):
                    raise MappingError("Only original positive OBJ indices are supported")
                corners.append((vertex_index, uv_index))
            faces.append({"object_name": object_name, "material_name": material_name, "corners": corners})
    if not vertices or not faces:
        raise MappingError("The source has no mesh geometry")
    if any(index > len(vertices) for face in faces for index, _ in face["corners"]):
        raise MappingError("Source vertex index exceeds the vertex table")
    return vertices, texture_coordinates, faces


def normalized(vector):
    values = [float(value) for value in vector]
    if len(values) != 3 or not all(math.isfinite(value) for value in values):
        raise MappingError("Authored corner normal must have three finite values")
    length = math.sqrt(sum(value * value for value in values))
    if length < 0.000001:
        raise MappingError("Missing/zero authored corner normal; no smoothing fallback is permitted")
    return [value / length for value in values]


def quantized(position):
    return tuple(round(value / QUANTIZATION_CM) for value in position)


def oriented_key(points):
    return min(tuple(points[index:] + points[:index]) for index in range(3))


def build_payload(original, reproduced, authored_normals, targets=TARGETS):
    original_records = canonical_records(original)
    reproduced_records = canonical_records(reproduced)
    if original_records != reproduced_records:
        mismatch = next((index for index, pair in enumerate(zip(original_records, reproduced_records)) if pair[0] != pair[1]), min(len(original_records), len(reproduced_records)))
        raise MappingError(f"Original conversion geometry/UV/group/face records differ at record {mismatch}; refusing nearest-neighbor mapping")
    vertices, texture_coordinates, faces = parse_source(original)
    if len(authored_normals) != len(vertices):
        raise MappingError("Authored normal table does not match the verified exported vertex table")
    triangles = []
    matched_targets = set()
    oriented, unordered = Counter(), Counter()
    normal_errors = []
    for face_index, face in enumerate(faces):
        target = (face["object_name"], face["material_name"])
        if target not in targets:
            continue
        matched_targets.add(target)
        corners = []
        for vertex_index, uv_index in face["corners"]:
            raw_normal = authored_normals[vertex_index - 1]
            normal = normalized(raw_normal)
            normal_errors.append(abs(sum(value * value for value in normal) - 1.0))
            corners.append({
                "position": vertices[vertex_index - 1],
                "normal": normal,
                "source_vertex_index": vertex_index,
                "source_uv_index": uv_index,
            })
        points = [quantized(corner["position"]) for corner in corners]
        if len(set(points)) != 3:
            raise MappingError("Cabin triangle collapses at the documented position quantization")
        slot_name = face["material_name"].replace(".", "_")
        oriented[(slot_name, oriented_key(points))] += 1
        unordered[(slot_name, tuple(sorted(points)))] += 1
        triangles.append({
            "source_face_index": face_index,
            "object_name": face["object_name"],
            "material_name": face["material_name"],
            "material_slot_name": slot_name,
            "corners": corners,
        })
    if matched_targets != set(targets):
        raise MappingError(f"Missing requested cabin groups: {sorted(set(targets) - matched_targets)}")
    duplicate_oriented = sum(count > 1 for count in oriented.values())
    duplicate_unordered = sum(count > 1 for count in unordered.values())
    if duplicate_oriented or duplicate_unordered:
        raise MappingError("Cabin position/material mapping contains duplicate triangles; refusing an ambiguous patch")
    invalid_uv_corners = sum(uv is not None and uv > len(texture_coordinates) for face in faces for _, uv in face["corners"])
    payload = {
        "schema_version": 1,
        "coordinate_system": {
            "positions": "Original converted OBJ coordinates in centimeters, before 698 cm runtime normalization",
            "normals": "Authored GLB normals transformed using the exact original scene conversion and node inverse-transpose",
            "to_unreal": {"position": [1, -1, 1], "normal": [1, -1, 1]},
            "match_quantization_cm": QUANTIZATION_CM,
            "triangle_winding": "Original OBJ order; match each unique corner position rather than assuming Unreal index order",
        },
        "source": {
            "geometry_records_sha256": record_hash(original),
            "vertex_records_sha256": record_hash(original, {"v"}),
            "uv_records_sha256": record_hash(original, {"vt"}),
            "face_records_sha256": record_hash(original, {"f"}),
            "group_records_sha256": record_hash(original, {"o", "g", "usemtl", "s"}),
            "geometry_reproduction_exact": True,
            "vertices": len(vertices),
            "texture_coordinates": len(texture_coordinates),
            "triangles": len(faces),
            "inherited_out_of_range_uv_corners": invalid_uv_corners,
            "uv_policy": "Not repaired, reimported, or edited; the UE builder must duplicate the existing mesh attributes",
        },
        "coverage": {
            "triangles": len(triangles),
            "corners": len(triangles) * 3,
            "groups": [{"object_name": name, "material_name": material, "material_slot_name": material.replace(".", "_")} for name, material in sorted(targets)],
            "duplicate_oriented_triangles": duplicate_oriented,
            "duplicate_unordered_triangles": duplicate_unordered,
            "all_normals_finite_and_unit": True,
            "max_normal_squared_length_error": max(normal_errors, default=0.0),
            "non_cabin_records_emitted": 0,
        },
        "triangles": triangles,
    }
    return payload


def reproduce_authored_normals(glb_path, dependency_dir=None):
    if dependency_dir:
        sys.path.insert(0, str(Path(dependency_dir).resolve()))
    try:
        import numpy as np
        import trimesh
        from trimesh.exchange.obj import export_obj
    except ImportError as error:
        raise MappingError("Requires numpy and trimesh with Draco GLB support; provide --dependency-dir for the existing local dependency directory") from error
    scene = trimesh.load(glb_path, force="scene")
    if not scene.geometry:
        raise MappingError("GLB did not decode to the retained source scene")
    # Capture GLB NORMAL data before scene.dump() copies meshes without caches.
    # Calling .vertex_normals here could silently calculate replacement normals.
    explicit_normals = {}
    for name, mesh in scene.geometry.items():
        cached = mesh._cache.cache.get("vertex_normals")
        if cached is None or len(cached) != len(mesh.vertices):
            raise MappingError(f"GLB geometry {name} has no explicit decoded NORMAL attribute")
        explicit_normals[name] = np.asarray(cached, dtype=float).copy()
    extent = max(float(scene.extents[0]), float(scene.extents[2]))
    if not math.isfinite(extent) or extent <= 0:
        raise MappingError("Invalid source scene extent")
    scale = 650.0 / extent
    basis = np.array([[1, 0, 0, 0], [0, 0, -1, 0], [0, 1, 0, 0], [0, 0, 0, 1]], dtype=float)
    basis[:3, :3] *= scale
    scene.apply_transform(basis)
    minimum, maximum = scene.bounds
    move = np.eye(4)
    move[:3, 3] = (-(minimum[0] + maximum[0]) / 2, -(minimum[1] + maximum[1]) / 2, -minimum[2])
    scene.apply_transform(move)
    normals = []
    for node_name in scene.graph.nodes_geometry:
        transform, geometry_name = scene.graph[node_name]
        linear = np.asarray(transform[:3, :3], dtype=float)
        if not np.all(np.isfinite(linear)) or abs(float(np.linalg.det(linear))) < 1e-12:
            raise MappingError(f"Singular/non-finite GLB node transform: {node_name}")
        transformed = explicit_normals[geometry_name] @ np.linalg.inv(linear)
        if not np.all(np.isfinite(transformed)):
            raise MappingError(f"Non-finite transformed source normals: {node_name}")
        normals.extend(transformed.tolist())
    reproduced, _ = export_obj(scene, include_texture=True, include_normals=False, return_texture=True)
    return reproduced, normals, {"trimesh_version": trimesh.__version__, "numpy_version": np.__version__, "source_geometry_primitives": len(scene.geometry), "original_width_conversion_scale": scale}


def fixture_checks():
    # Two corners occupy the same position but carry different authored normals.
    # Index/face verification must preserve that hard edge instead of averaging.
    source = "\n".join([
        "o polySurfac.002", "usemtl initialShadingGr.001",
        "v 0 0 0", "v 1 0 0", "v 0 1 0", "v 0 0 0", "v 0 0 1", "v 1 0 0",
        "vt 0 0", "vt 1 0", "vt 0 1", "vt 0 0", "vt 1 0", "vt 0 1",
        "f 1/1 2/2 3/3", "f 4/4 5/5 6/6",
        "o hard-metal", "usemtl blinn4SG.002", "f 2/2 5/5 6/6", "",
    ])
    normals = [[0, 0.6, 0.8]] * 3 + [[0.6, -0.8, 0]] * 3
    targets = {("polySurfac.002", "initialShadingGr.001")}
    payload = build_payload(source, source, normals, targets)
    checks = []

    def check(name, passed):
        if not passed:
            raise AssertionError(name)
        checks.append({"check": name, "passed": True})

    check("Coincident source vertices retain distinct authored hard-edge normals", payload["triangles"][0]["corners"][0]["normal"] == [0.0, 0.6, 0.8] and payload["triangles"][1]["corners"][0]["normal"] == [0.6, -0.8, 0.0])
    check("Only requested cabin faces are emitted", payload["coverage"]["triangles"] == 2 and payload["coverage"]["non_cabin_records_emitted"] == 0)
    check("Normal mapping output is deterministic", payload == build_payload(source, source, normals, targets))
    for name, reproduced, values in [
        ("Changed UV is rejected", source.replace("vt 0 1", "vt 0.5 1", 1), normals),
        ("Changed triangle order is rejected", source.replace("f 1/1 2/2 3/3", "f 1/1 3/3 2/2"), normals),
        ("Missing source normal is rejected", source, normals[:-1]),
        ("Non-finite source normal is rejected", source, [[float("nan"), 0, 1]] + normals[1:]),
        ("Zero source normal is rejected", source, [[0, 0, 0]] + normals[1:]),
    ]:
        rejected = False
        try:
            build_payload(source, reproduced, values, targets)
        except MappingError:
            rejected = True
        check(name, rejected)
    duplicate = source + "o polySurfac.002\nusemtl initialShadingGr.001\nf 1/1 2/2 3/3\n"
    rejected = False
    try:
        build_payload(duplicate, duplicate, normals, targets)
    except MappingError:
        rejected = True
    check("Duplicate position/material triangle ambiguity is rejected", rejected)
    return {"fixture": "Synthetic geometry mapping checks, not a gameplay or visual test", "checks": checks, "result": "PASS", "check_count": len(checks)}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--obj", type=Path)
    parser.add_argument("--glb", type=Path)
    parser.add_argument("--outputdir", "--output-dir", dest="outputdir", type=Path, required=True)
    parser.add_argument("--dependency-dir", type=Path)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args(argv)
    if bool(args.obj) != bool(args.glb) or (not args.self_test and not args.obj):
        parser.error("Provide both --obj and --glb, or use --self-test alone")
    output = args.outputdir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    fixtures = fixture_checks()
    (output / "mapping-fixture-checks.json").write_text(json.dumps(fixtures, indent=2, allow_nan=False) + "\n", encoding="utf-8")
    if args.obj:
        obj_path, glb_path = args.obj.resolve(), args.glb.resolve()
        original_bytes = obj_path.read_bytes()
        glb_bytes = glb_path.read_bytes()
        reproduced, normals, dependencies = reproduce_authored_normals(glb_path, args.dependency_dir)
        payload = build_payload(original_bytes.decode("utf-8"), reproduced, normals)
        payload["source"].update({"obj_sha256": sha256_bytes(original_bytes), "glb_sha256": sha256_bytes(glb_bytes), "obj_path": source_path_label(obj_path, output), "glb_path": source_path_label(glb_path, output), **dependencies})
        result = json.dumps(payload, indent=2, allow_nan=False) + "\n"
        target = output / "cabin-normals.json"
        target.write_text(result, encoding="utf-8", newline="\n")
        if obj_path.read_bytes() != original_bytes or glb_path.read_bytes() != glb_bytes:
            raise MappingError("Source changed during processing; discard this mapping")
        report = {"result": "PASS", "artifact": "cabin-normals.json", "artifact_sha256": sha256_bytes(result.encode("utf-8")), "source": payload["source"], "coverage": payload["coverage"], "source_files_unchanged": True, "geometry_changes": 0, "ue_import_performed": False, "editor_builder_required": "Duplicate the existing asset, bake computed render frames and explicitly repair invalid source tangent bases before applying selected GLB normals; preserve geometry, UV and material data. Do not import the inherited OBJ UV table. Consult the separate Editor receipt for frame changes and format bounds.", "fixture_checks": fixtures["check_count"]}
        (output / "source-normal-recovery-report.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n", encoding="utf-8")
        print(f"PASS source recovery: {len(payload['triangles'])} cabin triangles / {payload['coverage']['corners']} corners; {fixtures['check_count']} fixture checks; original geometry and UV records verified")
    else:
        print(f"PASS {fixtures['check_count']} synthetic mapping fixture checks")


if __name__ == "__main__":
    try:
        main()
    except MappingError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
