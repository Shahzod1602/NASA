"""Summarize a rendered Unreal CSV capture; exclude startup frames explicitly."""
import argparse
import csv
import json
import math
import statistics
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("capture", type=Path)
parser.add_argument("--warmup", type=int, default=600)
parser.add_argument("--expected-frames", type=int, default=4200)
parser.add_argument("--full-mission", action="store_true")
parser.add_argument("--onscreen", action="store_true")
parser.add_argument("--fps-cap", type=int, default=60)
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
csv.field_size_limit(20 * 1024 * 1024)  # Unreal's EVENTS column can be large.
with args.capture.open(encoding="utf-8-sig", newline="") as handle:
    reader = csv.reader(handle)
    header = next(reader)
    frame_index = header.index("FrameTime")
    frames = []
    for row in reader:
        try:
            value = float(row[frame_index])
        except (ValueError, IndexError):
            continue  # Repeated header and metadata footer are not frames.
        if math.isfinite(value) and value > 0:
            frames.append(value)
if args.expected_frames and len(frames) != args.expected_frames:
    raise SystemExit(f"Expected {args.expected_frames} frames; found {len(frames)}")
sample = frames[args.warmup:]
if len(sample) < 60:
    raise SystemExit("Not enough frames after warmup")
ordered = sorted(sample)
slowest = ordered[-max(1, math.ceil(len(sample) * 0.01)):]
result = {
    "capture": str(args.capture),
    "captured_frames": len(frames),
    "warmup_frames_excluded": args.warmup,
    "sample_frames": len(sample),
    "sample_seconds": sum(sample) / 1000,
    "mean_fps": 1000 / statistics.mean(sample),
    "one_percent_low_fps": 1000 / statistics.mean(slowest),
    "p95_frame_ms": ordered[math.ceil(len(ordered) * 0.95) - 1],
    "p99_frame_ms": ordered[math.ceil(len(ordered) * 0.99) - 1],
    "largest_frame_ms": max(sample),
    "method": "1% low = 1000 / mean frame ms of the slowest 1% of sampled frames",
    "limitations": ("On-screen" if args.onscreen else "Rendered offscreen") + (" full automated mission" if args.full_mission else " opening route") + f", normal delta time, {str(args.fps_cap)+' FPS cap' if args.fps_cap else 'uncapped rendering'}; not a human playtest; captures alone do not validate presentation smoothness. VRAM is not inferred from zero DX11 CSV counters.",
}
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
print(json.dumps(result, indent=2))
