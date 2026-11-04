#!/usr/bin/env python3
"""Verify a local field-operations bundle without launching Unreal or using network.

Usage: python Tools/verify_field_operations_bundle.py --repo D:/LunarRescue
       --output Reports/FieldOperations20261104/bundle-verification.json
       --receipt Reports/.../run-summary.json [Reports/.../run-summary.json ...]

No receipt means bundle integrity only, not gameplay validation. Editor receipts
are deliberately rejected as proof of the packaged executable.
"""
from __future__ import annotations
import argparse
import datetime as dt
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import struct
import sys
import tempfile
import unittest
import zlib

# Historical expected hashes retained before Nov 4, not newly captured baselines.
MAP_HASHES = {
    "Content/Lunar/Maps/Apollo12.umap": "7cd63371b866a7914ad9bd178b109e217d57b9032c639e0e9ee4f6ac897569f4",
    "Content/Lunar/Maps/MoonBase.umap": "d5a76deedb6e6d438834929bc46b59a71bcc5fd14cf30e45dd294cbcc121e1b6",
    "Content/Mars/Maps/MarsBase.umap": "d01d7605533777664a5dda974d09c4640a8a91e5830fe01b67198b54d1f7d4e4",
    "Content/Rescue/Maps/MissionSelect.umap": "9f098ec909532fa9284dd0e930ffeb055ccaf20acaa824123011fbbdb3c2dd25",
}
MISSIONS = {"apollo": "/Game/Lunar/Maps/Apollo12", "selene": "/Game/Lunar/Maps/MoonBase", "mars": "/Game/Mars/Maps/MarsBase"}
SHORT_MAP = {"apollo": "Apollo12", "selene": "MoonBase", "mars": "MarsBase"}
FOLDERS = ("Source", "Config", "Content/Science/LunarWater")
EXTENSIONS = {".cpp", ".h", ".cs", ".ini", ".json"}
FIELD_DOCS = ("IMPLEMENTATION.md", "PERFORMANCE_RECIPE.md", "PLAYTEST.md", "SCIENCE_RESEARCH.md", "STORY_AND_REFERENCES.md")
HASH = re.compile(r"[0-9a-f]{64}\Z")
NAME = re.compile(r"[a-z0-9]+(?:-[a-z0-9]+)*\Z")


def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def read_text(path: Path) -> str:
    """Strict UTF-8 or BOM-labelled UTF-16, as emitted by Unreal FFileHelper.

    Never guess legacy code pages or replace malformed bytes: that could hide
    a FAIL/fatal line. UTF-32 and unlabelled UTF-16 are unsupported.
    """
    raw = path.read_bytes()
    if raw.startswith((b"\xff\xfe\x00\x00", b"\x00\x00\xfe\xff")):
        raise ValueError("Unsupported UTF-32 text: " + str(path))
    if raw.startswith(b"\xff\xfe"):
        text = raw[2:].decode("utf-16-le", errors="strict")
    elif raw.startswith(b"\xfe\xff"):
        text = raw[2:].decode("utf-16-be", errors="strict")
    else:
        text = raw.decode("utf-8-sig", errors="strict")
    if "\x00" in text:
        raise ValueError("NUL or unlabelled multibyte text: " + str(path))
    return text


def read_json(path: Path):
    return json.loads(read_text(path))


def child(root: Path, relative: str) -> Path:
    if not isinstance(relative, str) or not relative or "\\" in relative or ":" in relative:
        raise ValueError("Expected a canonical relative slash path")
    parts = PurePosixPath(relative)
    if parts.is_absolute() or any(p in ("", ".", "..") for p in relative.split("/")):
        raise ValueError("Unsafe relative path: " + relative)
    path = (root / relative).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError("Path escaped its evidence root: " + relative)
    return path


def exact_report(path: Path) -> int:
    text = read_text(path)
    passes = len(re.findall(r"^PASS .+\r?$", text, re.M))
    results = re.findall(r"^RESULT:.*$", text, re.M)
    if not passes or re.search(r"^FAIL ", text, re.M) or len(results) != 1 or results[0].rstrip("\r") != "RESULT: 0 failures":
        raise ValueError("Not a complete passing semantic report: " + str(path))
    return passes


def png_size(path: Path) -> tuple[int, int]:
    raw = path.read_bytes()
    if raw[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("Invalid PNG signature: " + str(path))
    offset, dimensions, has_data, ended = 8, None, False, False
    while offset + 12 <= len(raw):
        length = struct.unpack_from(">I", raw, offset)[0]
        kind = raw[offset + 4:offset + 8]
        end = offset + 12 + length
        if end > len(raw):
            raise ValueError("Truncated PNG chunk: " + str(path))
        payload = raw[offset + 8:offset + 8 + length]
        crc = struct.unpack_from(">I", raw, offset + 8 + length)[0]
        if zlib.crc32(kind + payload) & 0xffffffff != crc:
            raise ValueError("PNG CRC mismatch: " + str(path))
        if dimensions is None:
            if kind != b"IHDR" or length != 13:
                raise ValueError("Missing first PNG IHDR")
            dimensions = struct.unpack_from(">II", payload)
            if min(dimensions) <= 0 or max(dimensions) > 16384:
                raise ValueError("Invalid PNG dimensions")
        elif kind == b"IHDR":
            raise ValueError("Duplicate PNG IHDR")
        if kind == b"IDAT":
            has_data = True
        if kind == b"IEND":
            if length or end != len(raw):
                raise ValueError("Invalid PNG end/trailing bytes")
            ended = True
            break
        offset = end
    if dimensions is None or not has_data or not ended:
        raise ValueError("Incomplete PNG: " + str(path))
    return dimensions


def fingerprint(root: Path, supplied: dict) -> dict:
    rows = supplied.get("files") if isinstance(supplied, dict) else None
    if not isinstance(rows, list) or not rows:
        raise ValueError("Missing nonempty source fingerprint")
    seen, canonical, mismatches = set(), [], []
    for row in rows:
        path, expected = row.get("path"), row.get("sha256")
        source = child(root, path)
        if not isinstance(expected, str) or not HASH.fullmatch(expected) or path.casefold() in seen:
            raise ValueError("Invalid/duplicate source fingerprint row")
        seen.add(path.casefold())
        canonical.append(path + ":" + expected)
        actual = sha(source) if source.is_file() else None
        if actual != expected:
            mismatches.append({"path": path, "expected": expected, "actual": actual})
    # Preserve runner row order: PowerShell Sort-Object culture rules differ
    # from Python's lexical order. Hash that exact declared canonical sequence.
    claimed = supplied.get("sha256")
    calculated = hashlib.sha256("\n".join(canonical).encode("utf-8")).hexdigest()
    if claimed != calculated:
        raise ValueError("Source fingerprint aggregate does not match its rows")
    actual_paths = {p.relative_to(root).as_posix().casefold() for folder in FOLDERS for p in (root / folder).rglob("*") if p.is_file() and p.suffix.lower() in EXTENSIONS}
    if seen != actual_paths:
        raise ValueError("Fingerprint omits/adds source/config/data files: missing=" + str(sorted(actual_paths - seen)) + "; extra=" + str(sorted(seen - actual_paths)))
    return {"total": len(rows), "matched": len(rows) - len(mismatches), "mismatches": mismatches, "sha256": calculated}


class Verification:
    def __init__(self):
        self.checks, self.errors = {}, []

    def check(self, name, condition, detail=None):
        self.checks[name] = bool(condition)
        if not condition and not any(error["check"] == name for error in self.errors):
            self.errors.append({"check": name, "detail": detail})
        return bool(condition)

    def attempt(self, name, action):
        try:
            return action()
        except (OSError, ValueError, KeyError, TypeError, AttributeError, struct.error) as error:
            self.check(name, False, str(error))
            return None


def verify_review(receipt: dict, case: dict, folder: Path) -> dict:
    name = case["case"]
    staged = child(folder, name)
    manifest = read_json(staged / "field-operations-manifest.json")
    if manifest.get("schema") != 1 or manifest.get("synthetic") is not True or manifest.get("human_playtest") is not False or manifest.get("performance_measurement") is not False or manifest.get("map") != SHORT_MAP[name]:
        raise ValueError("Invalid review manifest identity/claim schema: " + name)
    frames, evidence = manifest.get("frames"), case.get("frame_evidence")
    if not isinstance(frames, list) or not frames or not isinstance(evidence, list) or len(evidence) != len(frames) or manifest.get("frame_count") != len(frames) or case.get("frames") != len(frames):
        raise ValueError("Review PNG/manifest counts disagree: " + name)
    indexed = {}
    for frame in frames:
        key = frame.get("name")
        if not isinstance(key, str) or not NAME.fullmatch(key) or key in indexed or frame.get("layout_passed") is not True:
            raise ValueError("Invalid duplicate/failed arranged frame")
        indexed[key] = frame
    seen, results = set(), []
    for row in evidence:
        key = row.get("name")
        if key not in indexed or key in seen or row.get("layout_passed") is not True:
            raise ValueError("PNG evidence does not match manifest")
        seen.add(key)
        frame = indexed[key]
        size = (receipt.get("width"), receipt.get("height"))
        if any(type(n) is not int or n <= 0 for n in size) or (row.get("width"), row.get("height")) != size or (frame.get("width"), frame.get("height")) != size:
            raise ValueError("Review dimension metadata disagrees")
        image = child(staged, key + ".png")
        actual = sha(image)
        if row.get("sha256") != actual or png_size(image) != size:
            raise ValueError("PNG content/hash/dimensions differ: " + key)
        results.append({"name": key, "sha256": actual, "width": size[0], "height": size[1]})
    return {"frames": len(results), "images": results, "semantic_report": "field-operations-review-checks.txt"}


def verify_receipt(root: Path, path: Path, executable_sha: str) -> dict:
    receipt = read_json(path)
    if receipt.get("schema") != 1 or receipt.get("completed") is not True or receipt.get("packaged") is not True or receipt.get("executable_sha256") != executable_sha:
        raise ValueError("Receipt is incomplete, not packaged, or belongs to a different executable")
    for claim in ("human_playtest", "performance_measurement", "network_isolation_tested"):
        if receipt.get(claim) is not False:
            raise ValueError("Unsupported receipt claim: " + claim)
    source = fingerprint(root, receipt.get("source_fingerprint"))
    if source["mismatches"]:
        raise ValueError("Receipt source fingerprint differs from current files: " + json.dumps(source["mismatches"]))
    cases = receipt.get("cases")
    if not isinstance(cases, list) or not cases:
        raise ValueError("Receipt contains no completed cases")
    seen, results = set(), []
    review = receipt.get("review")
    if type(review) is not bool:
        raise ValueError("Receipt review field must be boolean")
    if review and receipt.get("rendered") is not True:
        raise ValueError("Review does not identify rendered frames")
    for case in cases:
        name = case.get("case")
        if name not in MISSIONS or name in seen or case.get("map") != MISSIONS[name] or case.get("outcome") != "passed" or case.get("config_restored") is not True or type(case.get("assertions")) is not int or case["assertions"] <= 0:
            raise ValueError("Invalid, duplicate or failed mission case: " + str(name))
        seen.add(name)
        expected_flag = "FieldOperationsReview" if review else "FieldOperationsTest"
        if case.get("flag") != expected_flag:
            raise ValueError("Unexpected test flag: " + name)
        log = child(path.parent, name + ".log")
        log_text = read_text(log)
        if re.search(r"Fatal error:|Unhandled Exception:|Assertion failed:|Failed to compile Material|FIELD_OPERATIONS_REVIEW FAILED", log_text):
            raise ValueError("Fatal/assert/render error in receipt log: " + str(log))
        report = child(path.parent, name + "/field-operations-review-checks.txt" if review else name + "-checks.txt")
        count = exact_report(report)
        if count != case["assertions"]:
            raise ValueError("Assertion count differs from raw report: " + name)
        item = {"case": name, "map": MISSIONS[name], "assertions": count, "report_sha256": sha(report), "log_sha256": sha(log)}
        if review:
            item["review"] = verify_review(receipt, case, path.parent)
        elif case.get("frames") != 0:
            raise ValueError("Functional receipt unexpectedly claims frames")
        results.append(item)
    return {"path": path.relative_to(root).as_posix() if path.is_relative_to(root) else str(path), "sha256": sha(path), "review": review, "packaged": True, "executable_sha256": executable_sha, "source_fingerprint": source, "cases": results}


def run(repo: Path, bundle: Path, receipts: list[Path]) -> dict:
    v = Verification()
    report = {"schema": 1, "checked_utc": dt.datetime.now(dt.timezone.utc).isoformat(), "scope": "bundle integrity and supplied packaged receipts" if receipts else "bundle integrity only; no gameplay receipt supplied", "human_playtest": False, "performance_measurement": False, "network_isolation_tested": False}
    executable = bundle / "LunarRescue/Binaries/Win64/LunarRescue.exe"
    actual_exe = v.attempt("packaged_executable_present", lambda: sha(executable))
    report["executable_sha256"] = actual_exe
    v.check("bootstrap_present", (bundle / "LunarRescue.exe").is_file() and (bundle / "LunarRescue.exe").stat().st_size > 0)
    built = v.attempt("built_executable_present", lambda: sha(repo / "Binaries/Win64/LunarRescue.exe"))
    v.check("built_and_staged_executable_match", bool(actual_exe) and actual_exe == built)
    def build_identity():
        info = read_json(bundle / "BUILD_INFO.json")
        identity = child(bundle, info["executable"])
        if info.get("schema_version") != 1 or info.get("build_kind") != "local-development" or info.get("published_release") is not False or identity != executable.resolve() or info.get("executable_sha256") != actual_exe:
            raise ValueError("BUILD_INFO does not identify this local inner executable")
        if "verification" in info and info["verification"].get("executable_sha256") != actual_exe:
            raise ValueError("BUILD_INFO inherited another executable's verification claim")
        return {"sha256": sha(bundle / "BUILD_INFO.json"), "base_git_commit": info.get("base_git_commit"), "source_worktree_dirty": info.get("source_worktree_dirty"), "engine_version": info.get("engine_version")}
    report["build_info"] = v.attempt("build_info_identity", build_identity)
    v.check("build_info_identity", report["build_info"] is not None)
    paks = [p for p in (bundle / "LunarRescue/Content/Paks").glob("*.pak") if p.is_file() and p.stat().st_size > 0]
    v.check("nonempty_cooked_pak_present", bool(paks))
    report["pak_files"] = [{"path": p.relative_to(bundle).as_posix(), "bytes": p.stat().st_size} for p in paks]
    def science():
        folder = repo / "Content/Science/LunarWater"
        data, manifest_path = folder / "study.json", folder / "manifest.json"
        manifest, study = read_json(manifest_path), read_json(data)
        if manifest.get("schema") != 1 or manifest.get("kind") != "published_science_summary" or manifest.get("file") != "study.json" or manifest.get("runtime_network_required") is not False or manifest.get("raw_spectra_included") is not False or manifest.get("binary_downloads") != 0:
            raise ValueError("Unexpected canonical local published-summary manifest")
        if manifest.get("sha256") != sha(data) or manifest.get("bytes") != data.stat().st_size or manifest.get("dataset_id") != study.get("dataset_id"):
            raise ValueError("Source science data identity/checksum/size does not match manifest")
        for name in ("study.json", "manifest.json"):
            if sha(folder / name) != sha(bundle / "LunarRescue/Content/Science/LunarWater" / name):
                raise ValueError("Bundled science file differs from source: " + name)
        return {"dataset_id": manifest["dataset_id"], "data_sha256": sha(data), "manifest_sha256": sha(manifest_path), "bytes": data.stat().st_size}
    report["science"] = v.attempt("science_payload", science)
    v.check("science_payload", report["science"] is not None)
    map_rows = []
    for name, expected in MAP_HASHES.items():
        path = child(repo, name)
        actual = v.attempt("map_present:" + name, lambda p=path: sha(p))
        ok = actual == expected
        v.check("historical_map_unchanged:" + name, ok)
        map_rows.append({"path": name, "historical_expected_sha256": expected, "actual_sha256": actual, "matched": ok})
    report["source_maps"] = map_rows
    def credits():
        pairs = [(repo / "CREDITS.md", bundle / "Credits/CREDITS.md"), (repo / "SCIENCE_AND_SOURCES.md", bundle / "Credits/SCIENCE_AND_SOURCES.md"), (repo / "PLAYING.txt", bundle / "READ_ME.txt")]
        source_art = repo / "SourceArt"
        for p in source_art.rglob("*"):
            if p.is_file() and ("credits" in p.name.lower() or "license" in p.name.lower()):
                pairs.append((p, bundle / "Credits" / p.relative_to(source_art)))
        notes = repo / "Docs/FieldOperations20261104"
        for name in FIELD_DOCS:
            if not (notes / name).is_file():
                raise ValueError("Required new field-operations note missing: " + name)
        pairs.extend((p, bundle / "Credits/Docs/FieldOperations20261104" / p.name) for p in notes.iterdir() if p.is_file() and p.suffix.lower() in (".md", ".json"))
        for source, target in pairs:
            if sha(source) != sha(target):
                raise ValueError("Staged credit/doc differs or is absent: " + str(target))
        return {"matched_files": len(pairs), "field_notes": sorted(p.name for p in notes.iterdir() if p.is_file() and p.suffix.lower() in (".md", ".json"))}
    report["credits_and_docs"] = v.attempt("credits_and_docs", credits)
    v.check("credits_and_docs", report["credits_and_docs"] is not None)
    report["receipts"] = []
    for index, path in enumerate(receipts):
        validated = v.attempt("receipt:" + str(index), lambda p=path: verify_receipt(repo, p, actual_exe))
        v.check("receipt:" + str(index), validated is not None)
        if validated is not None:
            report["receipts"].append(validated)
    functional_maps, review_maps = set(), set()
    for receipt in report["receipts"]:
        (review_maps if receipt["review"] else functional_maps).update(c["case"] for c in receipt["cases"])
    report["coverage"] = {"receipts_requested": len(receipts), "receipts_validated": len(report["receipts"]), "functional_maps": sorted(functional_maps), "review_maps": sorted(review_maps), "all_three_functional_maps": functional_maps == set(MISSIONS), "all_three_review_maps": review_maps == set(MISSIONS), "functional_pass_executions": sum(c["assertions"] for r in report["receipts"] if not r["review"] for c in r["cases"]), "review_pass_executions": sum(c["assertions"] for r in report["receipts"] if r["review"] for c in r["cases"]), "validated_pngs": sum(c.get("review", {}).get("frames", 0) for r in report["receipts"] for c in r["cases"])}
    final_sha = v.attempt("final_executable_readable", lambda: sha(executable))
    v.check("executable_unchanged_during_verification", bool(actual_exe) and actual_exe == final_sha)
    report.update(checks=v.checks, errors=v.errors, passed=all(v.checks.values()) and not v.errors)
    report["limitations"] = ["Source map hashes do not inspect map contents inside cooked pak archives.", "Hashes and arranged screenshot geometry do not prove visual quality or animation smoothness.", "PASS executions across receipts are executions, not a count of unique requirements.", "No engine, network, human playtest or performance measurement is performed by this verifier."]
    return report


class HelperChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="field-bundle-verifier-")
        self.root = Path(self.temp.name).resolve()
    def tearDown(self):
        self.temp.cleanup()
    def test_paths(self):
        self.assertEqual(child(self.root, "a/b"), self.root / "a/b")
        for bad in ("../a", "/a", "C:/a", "a\\b", "a/../b", "a//b", "./a"):
            with self.assertRaises(ValueError): child(self.root, bad)
    def test_reports(self):
        p = self.root / "report.txt"
        p.write_text("PASS One\nPASS Two\nRESULT: 0 failures\n", encoding="utf8")
        self.assertEqual(exact_report(p), 2)
        for bad in ("RESULT: 0 failures\n", "PASS One\nFAIL Two\nRESULT: 0 failures\n", "PASS One\nRESULT: 0 failures\nRESULT: 0 failures\n", "PASS One\nRESULT: 1 failures\n"):
            p.write_text(bad, encoding="utf8")
            with self.assertRaises(ValueError): exact_report(p)
    def test_bom_reports_json_and_malformed_rejection(self):
        p = self.root / "encoded.txt"
        text = "PASS One — source summary\r\nPASS Two\r\nRESULT: 0 failures\r\n"
        for raw in (text.encode("utf-8"), text.encode("utf-8-sig"), b"\xff\xfe" + text.encode("utf-16-le"), b"\xfe\xff" + text.encode("utf-16-be")):
            p.write_bytes(raw)
            self.assertEqual(exact_report(p), 2)
        for encoding, bom in (("utf-8-sig", b""), ("utf-16-le", b"\xff\xfe"), ("utf-16-be", b"\xfe\xff")):
            p.write_bytes(bom + json.dumps({"title": "source — summary"}, ensure_ascii=False).encode(encoding))
            self.assertEqual(read_json(p), {"title": "source — summary"})
        for malformed in (b"\xff", b"\xff\xfeA", b"\xff\xfe\x00\xd8", b"\xfe\xff\xd8\x00", b"\xff\xfe\x00\x00A\x00\x00\x00", text.encode("utf-16-le")):
            p.write_bytes(malformed)
            with self.assertRaises(ValueError):
                read_text(p)

    def test_png_crc_and_truncation(self):
        def chunk(k, payload): return struct.pack(">I", len(payload)) + k + payload + struct.pack(">I", zlib.crc32(k + payload) & 0xffffffff)
        raw = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 2, 1, 8, 2, 0, 0, 0)) + chunk(b"IDAT", zlib.compress(b"\0\0\0\0\0\0\0")) + chunk(b"IEND", b"")
        p = self.root / "test.png"; p.write_bytes(raw)
        self.assertEqual(png_size(p), (2, 1))
        for bad in (raw[:-1], raw + b"x", raw[:40] + bytes([raw[40] ^ 1]) + raw[41:]):
            p.write_bytes(bad)
            with self.assertRaises(ValueError): png_size(p)
    def test_fingerprint_content_and_coverage(self):
        p = self.root / "Source/a.cpp";p.parent.mkdir();p.write_text("one", encoding="utf8")
        digest = sha(p);rows = [{"path": "Source/a.cpp", "sha256": digest}]
        claimed = {"files": rows, "sha256": hashlib.sha256(("Source/a.cpp:" + digest).encode()).hexdigest()}
        self.assertEqual(fingerprint(self.root, claimed)["matched"], 1)
        p.write_text("two", encoding="utf8")
        self.assertEqual(len(fingerprint(self.root, claimed)["mismatches"]), 1)
        (p.parent / "b.h").write_text("extra", encoding="utf8")
        with self.assertRaises(ValueError): fingerprint(self.root, claimed)
    def test_incomplete_or_editor_receipt_rejected(self):
        p = self.root / "run-summary.json"
        for receipt in ({"schema": 1, "completed": False}, {"schema": 1, "completed": True, "packaged": False, "executable_sha256": "a" * 64}):
            p.write_text(json.dumps(receipt), encoding="utf8")
            with self.assertRaises(ValueError): verify_receipt(self.root, p, "a" * 64)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--bundle", type=Path, help="Default: <repo>/Dist/Apollo12/Windows")
    parser.add_argument("--output", type=Path, help="Default: <repo>/Reports/FieldOperations20261104/bundle-verification.json")
    parser.add_argument("--receipt", nargs="+", action="extend", type=Path, default=[], help="Completed packaged run-summary.json files; may be repeated")
    parser.add_argument("--self-test", action="store_true", help="Run isolated synthetic verifier-helper tests only")
    args = parser.parse_args()
    if args.self_test:
        suite = unittest.defaultTestLoader.loadTestsFromTestCase(HelperChecks)
        return 0 if unittest.TextTestRunner(verbosity=2).run(suite).wasSuccessful() else 1
    root = args.repo.resolve()
    bundle = (args.bundle if args.bundle else root / "Dist/Apollo12/Windows").resolve()
    output = (args.output if args.output else root / "Reports/FieldOperations20261104/bundle-verification.json")
    if not output.is_absolute(): output = root / output
    output = output.resolve()
    inputs = [(p if p.is_absolute() else root / p).resolve() for p in args.receipt]
    if output in inputs or output.suffix.lower() != ".json" or any(output.is_relative_to(root / folder) for folder in ("Source", "Config", "Content", "Binaries", "Dist", "Tools")):
        parser.error("Output must be a separate JSON report outside source/assets/build inputs")
    report = run(root, bundle, inputs)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf8")
    print(json.dumps({"passed": report["passed"], "executable_sha256": report["executable_sha256"], "coverage": report["coverage"], "errors": report["errors"], "output": str(output)}, indent=2))
    return 0 if report["passed"] else 1


if __name__ == "__main__":
    sys.exit(main())
