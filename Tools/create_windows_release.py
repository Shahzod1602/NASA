"""Create and independently hash-verify a clean, source-linked Windows ZIP.

The release source snapshot is recorded after review/commit. Compile-time dirty
worktree provenance stays separate; preparing an archive does not publish it.
"""
import argparse
import hashlib
import json
import re
import stat
import subprocess
import zipfile
from datetime import datetime, timezone
from pathlib import Path

REPOSITORY = 'https://github.com/Shahzod1602/NASA'
EXECUTABLE = 'LunarRescue/Binaries/Win64/LunarRescue.exe'
GENERATED = {'build_info.json', 'local_validation.json'}
PRIVATE_FOLDERS = {'saved', 'reports', 'backups', '.git', 'private', 'automatedtests'}
EXCLUDED_SUFFIXES = {'.pdb', '.log', '.csv', '.sav', '.dmp'}
REQUIRED = {
    'LunarRescue.exe', EXECUTABLE,
    'LunarRescue/Content/Paks/LunarRescue-Windows.pak',
    'LunarRescue/Content/Paks/LunarRescue-Windows.utoc',
    'LunarRescue/Content/Paks/LunarRescue-Windows.ucas',
    'READ_ME.txt', 'NOTICES.txt',
    'Engine/Extras/Redist/en-us/vc_redist.x64.exe',
    'Credits/CREDITS.md', 'Credits/SCIENCE_AND_SOURCES.md',
    'Credits/TEST_REPORT.md', 'Credits/NASA_TASKS_UPDATE.md',
    'Credits/Frontend/CREDITS.txt', 'Credits/Frontend/intro-manifest.json',
    'Credits/Apollo12/manifest.json', 'Credits/Apollo12/CREDITS.txt',
    'Credits/Apollo12/NAC_DTM_APOLLO12_README.TXT',
    'Credits/Apollo12/NAC_DTM_APOLLO12.LBL',
    'Credits/Apollo12/POLISH_UPDATE.md', 'Credits/Apollo12/HISTORICAL_AUDIT.md',
    'Credits/Apollo12/PLAYTEST_GUIDE.md',
}


def digest(stream):
    value = hashlib.sha256()
    while block := stream.read(1024 * 1024):
        value.update(block)
    return value.hexdigest()


def file_digest(path):
    with path.open('rb') as stream:
        return digest(stream)


def json_text(value):
    return json.dumps(value, indent=2, ensure_ascii=False) + '\n'


def number(row, names, label):
    values = [row[name] for name in names if name in row]
    if not values or any(type(value) is not int or value < 0 for value in values):
        raise ValueError('Missing/invalid nonnegative count: ' + label)
    if len(set(values)) != 1:
        raise ValueError('Contradictory count aliases: ' + label)
    return values[0]


def check_total(document, name, expected):
    if name in document and (type(document[name]) is not int or document[name] != expected):
        raise ValueError('Validation aggregate does not match its cases: ' + name)


def normalize_validation(document):
    """Accept historical case receipts and the current functional+regression receipt."""
    if not isinstance(document, dict):
        raise ValueError('Validation must be a JSON object')
    if 'new_functional_cases' in document or 'regressions' in document:
        if 'cases' in document:
            raise ValueError('Ambiguous mixed validation schemas')
        groups = [('functional', document.get('new_functional_cases')),
                  ('regression', document.get('regressions'))]
    else:
        groups = [('case', document.get('cases'))]
    cases = []
    subtotals = {}
    for group, rows in groups:
        if not isinstance(rows, list) or not rows:
            raise ValueError('Validation has an empty/missing case group: ' + group)
        seen = set()
        subtotal = 0
        for row in rows:
            if not isinstance(row, dict):
                raise ValueError('Invalid validation case')
            passed = number(row, ('passed', 'pass', 'Assertions', 'assertions', 'assertions_or_outcomes'), group + ' assertions')
            failed = number(row, ('failed', 'fail', 'Failures', 'failures'), group + ' failures')
            if not passed or failed:
                raise ValueError('Validation has a failed or empty case')
            outcome = row.get('Outcome', row.get('outcome', 'passed'))
            if not isinstance(outcome, str) or outcome.lower() not in {'passed', 'pass', 'success'}:
                raise ValueError('Validation case was not passed')
            name = next((row[key] for key in ('Case', 'case', 'name', 'Name', 'file', 'path') if key in row), None)
            if not isinstance(name, str) or not name.strip():
                raise ValueError('Validation case has no name')
            # Legacy receipts may contain a full local result path. Publish only
            # its filename; retain the original receipt checksum for provenance.
            name = name.replace('\\', '/').rsplit('/', 1)[-1]
            if name.casefold() in seen:
                raise ValueError('Duplicate validation case: ' + name)
            seen.add(name.casefold())
            cases.append({'group': group, 'case': name, 'passed': passed, 'failed': failed})
            subtotal += passed
        subtotals[group] = subtotal
    total = sum(subtotals.values())
    for name in ('failures', 'packaged_assertions_failed'):
        check_total(document, name, 0)
    for name in ('total_assertions_or_outcomes', 'packaged_assertions_passed'):
        check_total(document, name, total)
    for name in ('case_count', 'total_cases'):
        check_total(document, name, len(cases))
    if 'functional' in subtotals:
        check_total(document, 'new_assertions', subtotals['functional'])
        check_total(document, 'regression_assertions_or_outcomes', subtotals['regression'])
        check_total(document, 'functional_case_count', len(groups[0][1]))
        check_total(document, 'regression_case_count', len(groups[1][1]))
    hashes = [str(document[key]).lower() for key in ('executable_sha256', 'final_executable_sha256', 'validated_executable_sha256') if key in document]
    if not hashes or any(not re.fullmatch(r'[0-9a-f]{64}', value) for value in hashes) or len(set(hashes)) != 1:
        raise ValueError('Validation needs one consistent executable SHA-256')
    return {'executable_sha256': hashes[0], 'assertions_or_outcomes': total,
            'case_count': len(cases), 'failures': 0, 'cases': cases}


def selected_files(package):
    selected = []
    seen = set()
    pending = [package]
    while pending:
        directory = pending.pop()
        for path in sorted(directory.iterdir()):
            relative = path.relative_to(package)
            if any(part.casefold() in PRIVATE_FOLDERS for part in relative.parts):
                continue
            attributes = getattr(path.lstat(), 'st_file_attributes', 0)
            if path.is_symlink() or attributes & getattr(stat, 'FILE_ATTRIBUTE_REPARSE_POINT', 0):
                raise ValueError('Refusing a linked runtime path: ' + str(path))
            if path.is_dir():
                pending.append(path)
                continue
            if not path.is_file():
                continue
            if path.name.casefold() in GENERATED or path.suffix.casefold() in EXCLUDED_SUFFIXES or path.name.casefold().startswith('manifest_'):
                continue
            name = relative.as_posix()
            if name.casefold() in seen:
                raise ValueError('Duplicate Windows runtime path: ' + name)
            seen.add(name.casefold())
            selected.append((path, name))
    selected.sort(key=lambda item: item[1])
    names = {name for _, name in selected}
    if not REQUIRED <= names:
        raise ValueError('Missing runtime/credits files: ' + ', '.join(sorted(REQUIRED - names)))
    if any(path.stat().st_size == 0 for path, name in selected if name in REQUIRED):
        raise ValueError('A required runtime/credits file is empty')
    for name in ('Credits/Frontend/intro-manifest.json', 'Credits/Apollo12/manifest.json'):
        if not isinstance(json.loads((package / name).read_text(encoding='utf-8-sig')), dict):
            raise ValueError('Unreadable source metadata: ' + name)
    return selected


def verify_archive(archive, bundle, expected):
    """Read every decompressed entry independently after writing, including JSON."""
    with zipfile.ZipFile(archive) as zipped:
        entries = zipped.namelist()
        if len(entries) != len(set(name.casefold() for name in entries)):
            raise ValueError('Duplicate ZIP entry names')
        if set(entries) != {bundle + '/' + name for name in expected}:
            raise ValueError('ZIP entry set mismatch')
        if zipped.testzip() is not None:
            raise ValueError('ZIP CRC failed')
        for name, sha256 in expected.items():
            with zipped.open(bundle + '/' + name) as stream:
                if digest(stream) != sha256:
                    raise ValueError('ZIP independent SHA-256 mismatch: ' + name)


def assemble_release(root, package, output, version, bundle, commit, validation, validation_path):
    archive = output / (bundle + '.zip')
    if archive.exists():
        raise FileExistsError('Refusing to overwrite existing ZIP: ' + str(archive))
    normalized = normalize_validation(validation)
    selected = selected_files(package)
    executable_hash = file_digest(package / EXECUTABLE)
    if file_digest(root / 'Binaries/Win64/LunarRescue.exe') != executable_hash:
        raise ValueError('Staged executable differs from the built executable')
    if normalized['executable_sha256'] != executable_hash:
        raise ValueError('Validated executable differs from the staged/built executable')
    now = datetime.now(timezone.utc).isoformat()
    receipt = {'schema_version': 2, 'version': version, 'release_tag': 'apollo12-v' + version,
               'source_commit': commit, 'executable': EXECUTABLE,
               'executable_sha256': executable_hash, 'generated_utc': now,
               'validation_document_sha256': file_digest(validation_path),
               **{key: value for key, value in normalized.items() if key != 'executable_sha256'}}
    for key in ('human_playtests', 'network_disconnected_test_performed', 'competition_eligibility', 'total_staged_pngs', 'total_routed_pointer_checks'):
        if key in validation:
            receipt[key] = validation[key]
    generated_receipt = json_text(receipt)
    rows = [{'path': name, 'bytes': path.stat().st_size, 'sha256': file_digest(path)} for path, name in selected]
    receipt_bytes = generated_receipt.encode('utf-8')
    rows.append({'path': 'LOCAL_VALIDATION.json', 'bytes': len(receipt_bytes), 'sha256': hashlib.sha256(receipt_bytes).hexdigest()})
    provenance = {}
    prior_path = package / 'BUILD_INFO.json'
    if prior_path.is_file():
        prior = json.loads(prior_path.read_text(encoding='utf-8-sig'))
        if prior.get('executable_sha256') == executable_hash:
            provenance = {key: prior[key] for key in ('build_kind', 'base_git_commit', 'source_worktree_dirty', 'engine_version', 'generated_utc') if key in prior}
    info = {'schema_version': 2, 'build_kind': 'release-artifact', 'published_release': False,
            'publication_status': 'prepared-for-publication', 'version': version,
            'release_tag': 'apollo12-v' + version, 'source_commit': commit,
            'source_repository': REPOSITORY, 'platform': 'Windows 64-bit', 'configuration': 'Development',
            'source_snapshot_role': 'Reviewed clean release source snapshot; compile-time provenance, when available, is recorded separately.',
            'original_build_record': provenance, 'generated_utc': now,
            'executable': EXECUTABLE, 'executable_sha256': executable_hash,
            'validated_executable_sha256': executable_hash,
            'validation_receipt': 'LOCAL_VALIDATION.json', 'validation_receipt_sha256': hashlib.sha256(receipt_bytes).hexdigest(),
            'validated_assertions_or_outcomes': normalized['assertions_or_outcomes'], 'files': rows}
    manifest = json_text(info)
    generated = {'BUILD_INFO.json': manifest, 'LOCAL_VALIDATION.json': generated_receipt}
    expected = {row['path']: row['sha256'] for row in rows}
    expected['BUILD_INFO.json'] = hashlib.sha256(manifest.encode('utf-8')).hexdigest()
    output.mkdir(parents=True, exist_ok=True)
    created = False
    try:
        with zipfile.ZipFile(archive, 'x', compression=zipfile.ZIP_DEFLATED, compresslevel=6, allowZip64=True) as zipped:
            created = True
            for path, name in selected:
                zipped.write(path, bundle + '/' + name)
            for name, content in generated.items():
                zipped.writestr(bundle + '/' + name, content)
        verify_archive(archive, bundle, expected)
    except Exception:
        # Only this invocation's newly-created file is removed after rejection.
        if created:
            archive.unlink(missing_ok=True)
        raise
    checksum = file_digest(archive)
    for name, content in generated.items():
        (output / name).write_text(content, encoding='utf-8', newline='\n')
    (output / (archive.name + '.sha256')).write_text(checksum + '  ' + archive.name + '\n', encoding='ascii', newline='\n')
    return {'archive': str(archive), 'bytes': archive.stat().st_size, 'sha256': checksum,
            'source_commit': commit, 'executable_sha256': executable_hash,
            'assertions_or_outcomes': normalized['assertions_or_outcomes'], 'cases': normalized['case_count'],
            'entries': len(expected), 'release_tag': info['release_tag']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version', required=True)
    parser.add_argument('--validation', type=Path, required=True)
    parser.add_argument('--package-dir', type=Path)
    parser.add_argument('--bundle-name')
    args = parser.parse_args()
    if not re.fullmatch(r'[0-9]+\.[0-9]+\.[0-9]+(?:-[A-Za-z0-9][A-Za-z0-9.-]*)?', args.version):
        parser.error('Version must be a numeric release version, optionally with a prerelease suffix')
    bundle = args.bundle_name or ('LunarRescue-Expeditions-v' + args.version + '-Windows')
    if not re.fullmatch(r'[A-Za-z0-9][A-Za-z0-9._-]*', bundle):
        parser.error('Bundle name must be one plain ASCII directory name')
    root = Path(__file__).resolve().parents[1]
    package = (args.package_dir or root / 'Dist/Apollo12/Windows').resolve(strict=True)
    if not package.is_dir():
        parser.error('Package directory does not exist')
    commit = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=root, text=True).strip()
    if subprocess.check_output(['git', 'status', '--porcelain'], cwd=root, text=True).strip():
        raise SystemExit('Commit the reviewed source before making a source-linked release')
    try:
        validation = json.loads(args.validation.read_text(encoding='utf-8-sig'))
        result = assemble_release(root, package, root / 'Dist/Releases' / ('apollo12-v' + args.version), args.version, bundle, commit, validation, args.validation)
    except (ValueError, OSError, KeyError, zipfile.BadZipFile) as error:
        raise SystemExit(str(error)) from error
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
