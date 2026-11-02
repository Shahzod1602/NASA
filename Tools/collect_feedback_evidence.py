#!/usr/bin/env python3
"""Collect existing feedback validation evidence. Never launches Unreal or alters inputs.

The executable receipt is an explicit provenance assertion for legacy runners;
self-recorded run-summary hashes take precedence. Counts are PASS executions,
not unique tests, human participants, FPS measurements or final-build coverage.
"""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import sys

HASH = re.compile(r"^[0-9a-f]{64}$")
SAFE_NAME = re.compile(r"^[A-Za-z0-9_.-]+$")
SUMMARY_PATH = re.compile(r"^(?:Feedback validation|NASA task validation evidence|Loading typography evidence):\s*(.+?)\s*$", re.M)
PASS_LINE = re.compile(r"^PASS .+$", re.M)
FAIL_LINE = re.compile(r"^FAIL .+$", re.M)
RESULT = re.compile(r"^RESULT:\s*(\d+) failures\s*$", re.M)
PLANNED = ("feedback-functional", "feedback-review720", "feedback-review1080", "typography-review720", "typography-review1080", "loading-actual", "nasa-regression", "apollo-regression", "selection-functional", "frontend-functional", "experience-functional", "presentation-functional", "mars-movement", "frontend-review720", "frontend-review1080", "selection-review720", "selection-review1080")


def utc(value: str) -> datetime:
    return datetime.fromisoformat(value.replace("Z", "+00:00")).astimezone(timezone.utc)


def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def repo_path(root: Path, value: str) -> Path:
    path = Path(value.replace("\\", "/"))
    path = (path if path.is_absolute() else root / path).resolve()
    if not path.is_relative_to(root):
        raise ValueError("Evidence input must stay inside the repository")
    return path


def artifact(root: Path, path: Path) -> dict:
    return {"path": path.relative_to(root).as_posix(), "sha256": sha(path)}


def report_count(text: str) -> tuple[int, int]:
    passes, failures = len(PASS_LINE.findall(text)), len(FAIL_LINE.findall(text))
    results = [int(value) for value in RESULT.findall(text)]
    if not results or any(results) or failures:
        raise ValueError("No clean semantic RESULT or report contains failures")
    if passes == 0:
        raise ValueError("No exact PASS lines in assertion report")
    return passes, failures


def fresh(path: Path, start: str | None, finish: str | None) -> None:
    if not path.is_file():
        raise ValueError("Referenced evidence file is missing")
    if start and finish:
        modified = datetime.fromtimestamp(path.stat().st_mtime, timezone.utc)
        # NTFS / copied-file time resolution and wrapper flushing margin.
        if modified.timestamp() < utc(start).timestamp() - 2 or modified.timestamp() > utc(finish).timestamp() + 2:
            raise ValueError("Referenced evidence is outside the recorded run window")


def append_report(root: Path, record: dict, name: str, path: Path, category: str, expected: int | None = None) -> None:
    fresh(path, record.get("started_utc"), record.get("finished_utc"))
    count, _ = report_count(path.read_text(encoding="utf-8-sig"))
    if expected is not None and count != expected:
        raise ValueError("Copied raw PASS count disagrees with run summary")
    record["assertion_reports"].append({"case": name, "category": category, "pass_executions": count, **artifact(root, path)})


def from_summary(root: Path, record: dict, folder: Path) -> None:
    path = folder / "run-summary.json"
    fresh(path, record.get("started_utc"), record.get("finished_utc"))
    data = read_json(path)
    value = data.get("ExecutableSha256", data.get("executable_sha256", "")).lower()
    if not HASH.fullmatch(value):
        raise ValueError("Run summary is missing a valid executable SHA-256")
    if value != record["executable_sha256"]:
        raise ValueError("Runner executable hash conflicts with the supplied queue build receipt")
    record["executable_sha256"] = value
    record["executable_provenance"] = "runner-recorded executable hash"
    record["sources"].append(artifact(root, path))
    if data.get("HumanPlaytest") or data.get("human_playtest") or data.get("performance_measurement") or data.get("PerformanceMeasurement"):
        raise ValueError("This collector does not combine human or performance evidence")
    if "Cases" in data:
        staged = bool(data.get("SyntheticReview", data.get("Review", False)))
        for case in data["Cases"]:
            name = str(case["Case"])
            if not SAFE_NAME.fullmatch(name) or case.get("Outcome") != "passed" or int(case.get("Failures", 0)):
                raise ValueError("Invalid case identity or failed summary case")
            if "Assertions" in case:
                append_report(root, record, name, folder / (name + "-checks.txt"), "staged-layout" if staged else "functional", int(case["Assertions"]))
            frames = int(case.get("Frames", 0))
            if frames:
                record["staged_frames"] += frames
        record["resolution"] = [data.get("Width"), data.get("Height")]
    elif data.get("kind") == "layout-checks":
        append_report(root, record, "loading-typography", folder / "loading-typography-checks.txt", "functional", int(data["passed"]))
    elif data.get("kind") == "staged-layout-review":
        record["staged_frames"] += int(data["frames"])
        record["resolution"] = data.get("resolution")
    else:
        raise ValueError("Unsupported summary schema; do not guess its count")


def classify_log(root: Path, record: dict, path: Path) -> None:
    fresh(path, record.get("started_utc"), record.get("finished_utc"))
    text = path.read_text(encoding="utf-8-sig")
    record["sources"].append(artifact(root, path))
    folders = SUMMARY_PATH.findall(text)
    if folders:
        if len(folders) != 1:
            raise ValueError("Ambiguous summary references in wrapper log")
        from_summary(root, record, repo_path(root, folders[0].strip()))
        return
    # Legacy Apollo wrapper logs report exits; count only its copied semantic reports.
    apollo = re.findall(r"^package Apollo12 ([a-z-]+): exit (\d+) \(PID \d+\)\s*$", text, re.M)
    if apollo:
        for name, code in apollo:
            if int(code):
                raise ValueError("Apollo child process failed")
            append_report(root, record, "apollo-" + name, root / "Reports" / ("package-apollo12-" + name + "-result.txt"), "functional")
        return
    # Legacy functional wrappers print the raw assertions. Aggregate wrapper lines
    # such as 'PASS name: 24 assertions' are never fed to this branch.
    if RESULT.search(text):
        if re.search(r"^PASS [^\n]+:\s*\d+ (?:assertions|checks)", text, re.M):
            raise ValueError("Aggregate PASS lines cannot be treated as raw assertions")
        append_report(root, record, record["name"], path, "actual-travel" if record["name"] == "loading-actual" else "functional")
        return
    # These runners validate PNGs but emit no individual assertion report.
    if record["name"].startswith("frontend-review") and re.search(r"^PASS: seven fresh screenshots at \d+x\d+:", text, re.M):
        record["staged_frames"] = 7
        record["assertion_count_available"] = False
        return
    selection = re.search(r"^PASS: seven menu states and six naturally ticking transition samples at \d+x\d+:\s*(.+)$", text, re.M)
    if record["name"].startswith("selection-review") and selection:
        manifest_path = repo_path(root, selection.group(1).strip()) / "review-manifest.json"
        fresh(manifest_path, record.get("started_utc"), record.get("finished_utc"))
        manifest = read_json(manifest_path)
        clicks = manifest.get("pointer_clicks", [])
        if not manifest.get("synthetic") or manifest.get("human_playtest") or manifest.get("performance_measurement") or len(clicks) != 3:
            raise ValueError("Invalid staged selection/pointer manifest")
        if any(not all(click.get(key) is True for key in ("button_in_hit_path", "down_handled", "up_handled", "selected")) for click in clicks):
            raise ValueError("Selection pointer event did not complete its routed hit/click checks")
        record["validated_pointer_events"] = len(clicks)
        record["sources"].append(artifact(root, manifest_path))
        record["staged_frames"] = 13
        record["assertion_count_available"] = False
        return
    raise ValueError("No supported exact assertion or frame evidence found")


def make_record(root: Path, item: dict, receipt_hash: str, receipt_ref: dict) -> dict:
    name = str(item["name"])
    if not SAFE_NAME.fullmatch(name):
        raise ValueError("Unsafe run name")
    record = {"name": name, "queue_success": item.get("success"), "verification": "not-collected", "started_utc": item.get("started_utc"), "finished_utc": item.get("finished_utc"), "executable_sha256": receipt_hash, "executable_provenance": "explicit build receipt supplied for this queue; legacy runner did not self-record hash", "build_receipt": receipt_ref, "assertion_reports": [], "staged_frames": 0, "validated_pointer_events": 0, "assertion_count_available": True, "sources": []}
    if item.get("success") is not True:
        record["verification"] = "queue-failed-or-incomplete"
        return record
    try:
        classify_log(root, record, repo_path(root, item["log"]))
        record["verification"] = "verified-existing-evidence"
    except (OSError, ValueError, KeyError, TypeError) as error:
        # Emit the collector's fixed error category, never queue raw error text.
        record["verification"] = "unverified"
        record["collector_issue"] = str(error) if isinstance(error, ValueError) else type(error).__name__
    return record


def preserved_runs(root: Path, path: Path) -> tuple[list[dict], dict]:
    data = read_json(path)
    if data.get("schema_version") != 1:
        raise ValueError("Unsupported prior evidence schema")
    for run in data["runs"]:
        if not HASH.fullmatch(run["executable_sha256"]):
            raise ValueError("Prior run has an invalid executable hash")
        if run["verification"] != "verified-existing-evidence":
            continue
        for source in [*run["sources"], *run["assertion_reports"], run["build_receipt"]]:
            target = repo_path(root, source.get("frozen_path", source["path"]))
            if sha(target) != source["sha256"]:
                raise ValueError("Preserved evidence input hash changed")
        for report in run["assertion_reports"]:
            target = repo_path(root, report.get("frozen_path", report["path"]))
            if report_count(target.read_text(encoding="utf-8-sig"))[0] != report["pass_executions"]:
                raise ValueError("Preserved report count changed")
    return data["runs"], artifact(root, path)


def collect(root: Path, queue_paths: list[Path], receipt_path: Path, extras: list[str], prior_paths: list[Path] | None = None) -> dict:
    receipt = read_json(receipt_path)
    receipt_hash = str(receipt.get("staged_sha256", receipt.get("executable_sha256", ""))).lower()
    if not HASH.fullmatch(receipt_hash) or receipt.get("source_mismatches") != 0:
        raise ValueError("Receipt lacks a valid executable hash or unchanged source snapshot")
    if "staged_sha256" in receipt:
        if receipt.get("built_sha256") != receipt_hash:
            raise ValueError("Compiled and staged executable receipt hashes disagree")
        receipt_validation = "Matching built/staged hashes recorded by the supplied build receipt"
    else:
        # The controls receipt records one hash after its build helper compares
        # compiled and staged files. Independently confirm both current files.
        if not HASH.fullmatch(str(receipt.get("editor_module_sha256", ""))):
            raise ValueError("Single-hash receipt is missing its compiled editor-module identifier")
        binaries = (root / "Binaries/Win64/LunarRescue.exe", root / "Dist/Apollo12/Windows/LunarRescue/Binaries/Win64/LunarRescue.exe")
        if any(sha(binary) != receipt_hash for binary in binaries):
            raise ValueError("Current built/staged files do not match the single-hash controls receipt")
        receipt_validation = "Collector independently SHA-256 checked both current built/staged executables against the supplied single-hash receipt"
    reference = artifact(root, receipt_path)
    records, queues = [], []
    for queue_path in queue_paths:
        items = read_json(queue_path)
        if isinstance(items, dict):
            items = [items]
        if not isinstance(items, list):
            raise ValueError("Queue must contain an array of completed run records")
        queues.append({**artifact(root, queue_path), "completed_records": len(items), "names": [str(item["name"]) for item in items]})
        for item in items:
            record = make_record(root, item, receipt_hash, reference)
            record["queue"] = queue_path.relative_to(root).as_posix()
            records.append(record)
    for extra in extras:
        name, sep, path = extra.partition("=")
        if not sep:
            raise ValueError("--extra-log requires name=repository/path.log")
        records.append(make_record(root, {"name": name, "success": True, "log": path}, receipt_hash, reference))
    prior_refs = []
    for prior_path in prior_paths or []:
        prior_records, prior_ref = preserved_runs(root, prior_path)
        prior_refs.append(prior_ref)
        records.extend(prior_records)
    # Reading the same queue/evidence twice does not create another execution.
    unique, identities = [], {}
    for record in records:
        primary_source = tuple((source["path"], source["sha256"]) for source in record["sources"][:1])
        identity = (record["name"], record["started_utc"], record["finished_utc"], primary_source)
        if identity not in identities:
            identities[identity] = len(unique)
            unique.append(record)
        else:
            index = identities[identity]
            previous = unique[index]
            rank = lambda run: (run["verification"] == "verified-existing-evidence", run.get("validated_pointer_events", 0), len(run["sources"]))
            if rank(record) > rank(previous):
                unique[index] = record
    duplicates = len(records) - len(unique)
    records = unique
    buckets = {}
    for record in records:
        value = record["executable_sha256"]
        bucket = buckets.setdefault(value, {"verified_runs": 0, "unverified_runs": 0, "pass_executions": 0, "functional_pass_executions": 0, "staged_layout_pass_executions": 0, "actual_travel_pass_executions": 0, "staged_frames": 0, "validated_pointer_events": 0})
        if record["verification"] != "verified-existing-evidence":
            bucket["unverified_runs"] += 1
            continue
        bucket["verified_runs"] += 1
        bucket["staged_frames"] += record["staged_frames"]
        bucket["validated_pointer_events"] += record.get("validated_pointer_events", 0)
        for report in record["assertion_reports"]:
            count = report["pass_executions"]
            bucket["pass_executions"] += count
            key = {"functional": "functional_pass_executions", "staged-layout": "staged_layout_pass_executions", "actual-travel": "actual_travel_pass_executions"}[report["category"]]
            bucket[key] += count
    completed = {record["name"] for record in records}
    return {"schema_version": 1, "collected_utc": datetime.now(timezone.utc).isoformat(), "scope": "Read-only collection of existing test outputs; no engine launched", "count_semantics": "Exact PASS-line executions, including repeated suites/maps. PNGs and exit-zero summaries are not assertions. Totals remain separate per executable; no claim of complete final-build coverage.", "receipt_validation": receipt_validation, "source_artifacts": "Paths are repository-relative local validation artifacts; some Reports files are ignored by Git. SHA-256 identifies the exact input read. Raw source-group messages are not included.", "human_playtests": 0, "performance_measurement": False, "queues": queues, "preserved_collections": prior_refs, "duplicate_run_records_ignored": duplicates, "default_queue_cases_not_yet_recorded": [name for name in PLANNED if name not in completed], "runs": records, "totals_by_executable_sha256": buckets}


def self_test() -> None:
    from tempfile import TemporaryDirectory
    assert report_count("PASS A\nPASS B\nRESULT: 0 failures\n") == (2, 0)
    for text in ("PASS A\nRESULT: 1 failures\n", "PASS A\nFAIL B\nRESULT: 0 failures\n", "PASS A\n"):
        try:
            report_count(text)
        except ValueError:
            pass
        else:
            raise AssertionError("Failed/missing semantic result was accepted")
    with TemporaryDirectory() as directory:
        root = Path(directory).resolve()
        try:
            repo_path(root, "../outside")
        except ValueError:
            pass
        else:
            raise AssertionError("Path escape was accepted")
        (root / "case-checks.txt").write_text("PASS x\nRESULT: 0 failures\n", encoding="utf-8")
        summary = {"ExecutableSha256": "b" * 64, "SyntheticReview": False, "Cases": [{"Case": "case", "Assertions": 2, "Outcome": "passed"}]}
        (root / "run-summary.json").write_text(json.dumps(summary), encoding="utf-8")
        record = {"sources": [], "assertion_reports": [], "staged_frames": 0, "executable_sha256": "b" * 64}
        try:
            from_summary(root, record, root)
        except ValueError:
            pass
        else:
            raise AssertionError("Summary/raw count mismatch was accepted")
    print("Collector self-checks passed: semantic failures, absent RESULT, path escape and mismatched summary rejected.")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--queue", action="append", help="Repository-relative queue JSON; repeat for additional receipts sharing this supplied binary receipt")
    parser.add_argument("--binary-receipt", default="Reports/Feedback20261102/compiled-typography-final.json")
    parser.add_argument("--extra-log", action="append", default=[], help="name=path.log; summary must self-record a binary hash when available")
    parser.add_argument("--prior-evidence", action="append", default=[], help="Preserved collector JSON; recount frozen raw inputs and keep their original executable provenance")
    parser.add_argument("--output", help="Write compact JSON to this repository-relative path; omit to print JSON only")
    parser.add_argument("--require-complete", action="store_true", help="Fail if any default queue case is missing or any input is unverified")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return 0
    root = args.repo.resolve()
    try:
        result = collect(root, [repo_path(root, path) for path in (args.queue or ["Reports/Feedback20261102/final-test-queue.json"])], repo_path(root, args.binary_receipt), args.extra_log, [repo_path(root, path) for path in args.prior_evidence])
        payload = json.dumps(result, indent=2) + "\n"
        if args.output:
            output = repo_path(root, args.output)
            if output.suffix.lower() != ".json" or output.relative_to(root).parts[0] not in ("Reports", "Docs"):
                raise ValueError("Output must be a JSON artifact under Reports or Docs")
            input_paths = {repo_path(root, path) for path in (args.queue or ["Reports/Feedback20261102/final-test-queue.json"])}
            input_paths.add(repo_path(root, args.binary_receipt))
            input_paths.update(repo_path(root, path) for path in args.prior_evidence)
            for run in result["runs"]:
                input_paths.update(repo_path(root, source["path"]) for source in [*run["sources"], *run["assertion_reports"]])
            if output in input_paths:
                raise ValueError("Output must not overwrite an evidence input")
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_text(payload, encoding="utf-8")
            print(json.dumps({"output": output.relative_to(root).as_posix(), "totals_by_executable_sha256": result["totals_by_executable_sha256"], "default_queue_cases_not_yet_recorded": result["default_queue_cases_not_yet_recorded"]}, indent=2))
        else:
            print(payload, end="")
        incomplete = bool(result["default_queue_cases_not_yet_recorded"]) or any(run["verification"] != "verified-existing-evidence" for run in result["runs"])
        return 2 if args.require_complete and incomplete else 0
    except (OSError, ValueError, KeyError, TypeError) as error:
        print("Evidence collection failed: " + str(error), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
