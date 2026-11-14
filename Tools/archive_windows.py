"""Archive a verified staged Windows game without local player/test data."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import zipfile


def sha256(path):
    result = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(4 * 1024 * 1024), b''):
            result.update(block)
    return result.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--expected-exe-sha256', required=True)
    parser.add_argument('--expected-movie-sha256')
    args = parser.parse_args()
    source, output = args.source.resolve(), args.output.resolve()
    if not source.is_dir() or output.is_relative_to(source):
        raise ValueError('Output must be outside the existing staged source directory')
    if output.exists() or output.with_suffix(output.suffix + '.partial').exists():
        raise FileExistsError('Refusing to overwrite an earlier archive or partial output')
    executable = source / 'LunarRescue/Binaries/Win64/LunarRescue.exe'
    if sha256(executable) != args.expected_exe_sha256.lower():
        raise ValueError('Staged executable differs from the tested executable')
    if args.expected_movie_sha256:
        movie = source / 'LunarRescue/Content/Movies/MoonArrival.mp4'
        if sha256(movie) != args.expected_movie_sha256.lower():
            raise ValueError('Staged cinematic differs from the authorized original')
    excluded_dirs = {'saved', 'reports', 'backups', 'intermediate', '.git'}
    excluded_extensions = {'.pdb', '.ilk', '.dmp', '.log', '.sav', '.bak'}
    root_dirs = {'Engine', 'LunarRescue', 'Credits'}
    root_files = {'LunarRescue.exe', 'READ_ME.txt', 'NOTICES.txt',
                  'BUILD_INFO.json', 'LOCAL_VALIDATION.json', 'DEMO_BUILD_INFO.json'}
    files = []
    for base, directories, names in os.walk(source):
        current = Path(base)
        relative = current.relative_to(source)
        directories[:] = [name for name in directories
                          if name.lower() not in excluded_dirs
                          and (relative.parts or name in root_dirs)]
        for name in directories:
            if (current / name).is_symlink():
                raise ValueError('Refusing linked directories in the release tree')
        for name in names:
            path = current / name
            if not relative.parts and name not in root_files:
                continue
            if path.suffix.lower() in excluded_extensions or name.lower().startswith('.env'):
                continue
            if path.is_symlink():
                raise ValueError('Refusing linked files in the release tree')
            files.append(path)
    files.sort()
    members = ['Windows/' + path.relative_to(source).as_posix() for path in files]
    if 'Windows/LunarRescue.exe' not in members:
        raise ValueError('Missing player launcher')
    if args.expected_movie_sha256 and 'Windows/LunarRescue/Content/Movies/MoonArrival.mp4' not in members:
        raise ValueError('Cinematic was omitted from the archive inventory')
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(output.suffix + '.partial')
    with zipfile.ZipFile(temporary, 'x', compression=zipfile.ZIP_DEFLATED,
                         compresslevel=6, allowZip64=True) as archive:
        for path, name in zip(files, members):
            archive.write(path, name)
    with zipfile.ZipFile(temporary) as archive:
        if archive.testzip() is not None or archive.namelist() != members:
            raise ValueError('Archive CRC or member inventory validation failed')
        if any(excluded_dirs.intersection(part.lower() for part in name.split('/'))
               for name in members):
            raise ValueError('Private/generated directories appeared in the archive')
    digest = sha256(temporary)
    temporary.rename(output)
    output.with_suffix(output.suffix + '.sha256').write_text(
        digest + '  ' + output.name + '\n', encoding='ascii')
    receipt = {'archive': str(output), 'bytes': output.stat().st_size,
               'sha256': digest, 'members': len(members), 'crc_passed': True,
               'executable_sha256': args.expected_exe_sha256.lower(),
               'cinematic_sha256': args.expected_movie_sha256,
               'player_saved_data_included': False}
    output.with_suffix(output.suffix + '.json').write_text(
        json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(receipt, indent=2))


if __name__ == '__main__':
    main()
