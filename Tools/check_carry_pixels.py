"""Check paired PNG contribution of a carried cell; this is not segmentation."""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from datetime import datetime, timezone
from pathlib import Path

from PIL import Image, ImageChops

PAIR_NAMES = (
    ("cell-fp45-lookdown", "cell-fp45-hidden-control"),
    ("cell-fp45-text150", "cell-fp45-text150-hidden-control"),
)
DELTA_THRESHOLD = 24
MIN_CHANGED_FRACTION = 0.12
CENTRAL_FRACTION = 0.60
FOV_KEYS = ("fov", "field_of_view", "camera_fov", "FOV")


def number(value: object, label: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{label} must be numeric")
    value = float(value)
    if not math.isfinite(value):
        raise ValueError(f"{label} must be finite")
    return value


def region(frame: dict) -> tuple[float, float, float, float]:
    value = frame.get("cell_region")
    if not isinstance(value, list) or len(value) != 4:
        raise ValueError("cell_region must be four normalized UV bounds")
    result = tuple(number(x, "cell_region") for x in value)
    x0, y0, x1, y1 = result
    if not (0 <= x0 < x1 <= 1 and 0 <= y0 < y1 <= 1):
        raise ValueError("cell_region must be nonempty and within [0,1]")
    return result


def load_png(path: Path) -> Image.Image:
    with Image.open(path) as image:
        if image.format != "PNG":
            raise ValueError(f"Not a PNG: {path.name}")
        image.load()
        return image.convert("RGB")


def check_pair(folder: Path, frames: dict, names: tuple[str, str]) -> dict:
    shown_name, hidden_name = names
    result = {"shown": shown_name, "control": hidden_name, "passed": False}
    try:
        shown, hidden = frames[shown_name], frames[hidden_name]
        if shown.get("cell_hidden_control") is not False:
            raise ValueError("Shown frame must declare cell_hidden_control=false")
        if hidden.get("cell_hidden_control") is not True:
            raise ValueError("Control must declare cell_hidden_control=true")
        uv = region(shown)
        control_uv = region(hidden)
        if any(not math.isclose(a, b, rel_tol=0, abs_tol=1e-9)
               for a, b in zip(uv, control_uv)):
            raise ValueError("Paired cell regions differ")
        scale = number(shown.get("text_scale"), "shown text_scale")
        control_scale = number(hidden.get("text_scale"), "control text_scale")
        if scale <= 0 or not math.isclose(scale, control_scale, rel_tol=0, abs_tol=1e-6):
            raise ValueError("Paired text scales differ or are invalid")
        result["text_scale"] = scale
        result["fov_checks"] = {}
        for key in FOV_KEYS:
            if key in shown or key in hidden:
                a = number(shown.get(key), f"shown {key}")
                b = number(hidden.get(key), f"control {key}")
                if not 0 < a < 180 or not math.isclose(a, b, rel_tol=0, abs_tol=1e-6):
                    raise ValueError(f"Paired {key} values differ or are invalid")
                result["fov_checks"][key] = a
        result["fov_checked"] = bool(result["fov_checks"])
        shown_path, hidden_path = folder / (shown_name + ".png"), folder / (hidden_name + ".png")
        a, b = load_png(shown_path), load_png(hidden_path)
        if a.size != b.size:
            raise ValueError("Paired PNG dimensions differ")
        width, height = a.size
        x0, y0, x1, y1 = uv
        margin = (1 - CENTRAL_FRACTION) / 2
        inner = (x0 + (x1 - x0) * margin, y0 + (y1 - y0) * margin,
                 x1 - (x1 - x0) * margin, y1 - (y1 - y0) * margin)
        pixels = (max(0, math.floor(inner[0] * width)),
                  max(0, math.floor(inner[1] * height)),
                  min(width, math.ceil(inner[2] * width)),
                  min(height, math.ceil(inner[3] * height)))
        if pixels[2] <= pixels[0] or pixels[3] <= pixels[1]:
            raise ValueError("Central cell ROI has no pixels")
        difference = ImageChops.difference(a.crop(pixels), b.crop(pixels))
        red, green, blue = difference.split()
        maximum = ImageChops.lighter(ImageChops.lighter(red, green), blue)
        histogram = maximum.histogram()
        changed = sum(histogram[DELTA_THRESHOLD + 1:])
        total = (pixels[2] - pixels[0]) * (pixels[3] - pixels[1])
        fraction = changed / total
        result.update({
            "cell_region_uv": list(uv), "central_region_uv": list(inner),
            "central_region_pixels": list(pixels), "image_size": [width, height],
            "changed_pixels": changed, "roi_pixels": total,
            "changed_fraction": fraction,
            "shown_sha256": hashlib.sha256(shown_path.read_bytes()).hexdigest(),
            "control_sha256": hashlib.sha256(hidden_path.read_bytes()).hexdigest(),
            "passed": fraction >= MIN_CHANGED_FRACTION,
        })
        if not result["passed"]:
            result["error"] = "Carried cell contribution is below the required central-ROI fraction"
    except (KeyError, ValueError, OSError) as error:
        result["error"] = str(error)
    return result


def check_manifest(path: Path) -> dict:
    result = {
        "schema": 1, "kind": "paired_carried_cell_render_contribution",
        "passed": False, "synthetic": True, "human_playtest": False,
        "performance_measurement": False, "semantic_segmentation": False,
        "metric": "fraction of central-ROI pixels with max absolute RGB channel difference > threshold",
        "delta_threshold": DELTA_THRESHOLD,
        "minimum_changed_fraction": MIN_CHANGED_FRACTION,
        "central_fraction": CENTRAL_FRACTION,
        "limitations": "Measures rendered contribution between staged cell-visible/hidden pairs. It does not identify object pixels semantically, prove natural hand anatomy, or measure FPS. Scene/camera/hands must be held constant by the capture harness; only the available manifest ROI, text-scale and FOV fields are checked here.",
        "pairs": [], "checked_utc": datetime.now(timezone.utc).isoformat(),
    }
    try:
        raw = path.read_bytes()
        data = json.loads(raw.decode("utf-8-sig"))
        if not isinstance(data, dict):
            raise ValueError("Manifest must be an object")
        for key, expected in (("synthetic", True), ("human_playtest", False),
                              ("performance_measurement", False)):
            if data.get(key) is not expected:
                raise ValueError(f"Manifest must declare {key}={expected}")
        items = data.get("frames")
        if not isinstance(items, list) or not items:
            raise ValueError("Manifest must contain nonempty frames")
        frames = {}
        for item in items:
            if not isinstance(item, dict) or not isinstance(item.get("name"), str):
                raise ValueError("Manifest frame is malformed")
            name = item["name"]
            if name in frames:
                raise ValueError(f"Duplicate manifest frame: {name}")
            frames[name] = item
        result["manifest_sha256"] = hashlib.sha256(raw).hexdigest()
        result["pairs"] = [check_pair(path.parent, frames, names) for names in PAIR_NAMES]
        result["passed"] = all(pair["passed"] for pair in result["pairs"])
    except (ValueError, OSError) as error:
        result["error"] = str(error)
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", type=Path, required=True)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    output = args.output or args.manifest.parent / "carry-pixel-checks.json"
    if output.resolve() == args.manifest.resolve():
        parser.error("Output must not overwrite the input manifest")
    result = check_manifest(args.manifest)
    output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"{'PASS' if result['passed'] else 'FAIL'} carried-cell pixel contribution: {output}")
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
