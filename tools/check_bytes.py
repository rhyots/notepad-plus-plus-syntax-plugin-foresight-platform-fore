"""Validate exact test-bound bytes and LF in automation files."""
from pathlib import Path
import argparse
import hashlib
import json

ROOT = Path(__file__).resolve().parents[1]
LOCKED = ('src', 'styles', 'examples', 'vendor')


def fingerprints(root):
    return {p.relative_to(root).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
            for directory in LOCKED for p in sorted((root/directory).rglob('*')) if p.is_file()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--write', action='store_true', help='Record intentional source changes; rerun native tests before release')
    args = parser.parse_args()
    path = ROOT/'byte-lock.json'
    actual = fingerprints(ROOT)
    if args.write:
        path.write_text(json.dumps({'algorithm': 'SHA-256', 'files': actual}, indent=2)+'\n', encoding='utf-8', newline='\n')
        print(f'Recorded {len(actual)} byte fingerprints; this does not certify a native test run.')
        return 0
    expected = json.loads(path.read_text(encoding='utf-8'))['files']
    problems = [name for name in sorted(set(actual) | set(expected)) if actual.get(name) != expected.get(name)]
    for pattern in ['.github/workflows/*.yml', '.github/workflows/*.yaml', '*.ps1', 'qa/*.ps1', 'qa/*.py', 'tests/*.py', 'tools/*.py', 'requirements*.txt', '.gitattributes', '.editorconfig']:
        for file in ROOT.glob(pattern):
            data = file.read_bytes()
            if b'\r' in data or data.startswith(b'\xef\xbb\xbf'):
                problems.append(file.relative_to(ROOT).as_posix()+': expected UTF-8 without BOM, LF')
    for name in problems:
        print('BYTE ERROR:', name)
    print(f'BYTES: {len(actual)} locked files; ' + ('PASS' if not problems else 'FAIL'))
    return 1 if problems else 0


if __name__ == '__main__':
    raise SystemExit(main())
