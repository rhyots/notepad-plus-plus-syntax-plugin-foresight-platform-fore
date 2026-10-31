from pathlib import Path
import os
import re

from release_files import ASSET, public_files


def put(root, relative, data=b'example'):
    path = root/relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def test_source_bundle_excludes_development_state(tmp_path):
    included = ['README.md', '.github/workflows/build.yml', 'src/code.cpp', 'vendor/notice.txt']
    excluded = ['build/code.py', 'dist/report.json', '.git/config', '.metrics/crap.edn',
                '.pytest_cache/state', '.venv/config', 'target/mutation-workers/code.py',
                'htmlcov/index.html', 'qa/notepad-theme/config.xml', '__pycache__/cache.pyc',
                '.coverage', 'loose.pdb', 'loose.dll']
    for name in included+excluded:
        put(tmp_path, name)
    actual = {p.relative_to(tmp_path).as_posix() for p in public_files(tmp_path)}
    assert actual == set(included)


def test_source_scan_prunes_ignored_trees(tmp_path, monkeypatch):
    put(tmp_path, 'build/deep/private.txt')
    visited = []
    original = os.scandir

    def record(path):
        visited.append(Path(path).resolve())
        return original(path)

    monkeypatch.setattr(os, 'scandir', record)
    assert list(public_files(tmp_path)) == []
    assert (tmp_path/'build').resolve() not in visited


def test_readmes_use_actual_release_asset_names():
    expected = {ASSET+suffix for suffix in ('-x64.zip', '-x86.zip', '-source.zip', '-plugins-admin-x64.zip', '-plugins-admin-x86.zip')}
    for name in ('README.md', 'README.ru.md'):
        text = Path(name).read_text(encoding='utf-8')
        mentioned = re.findall(r'NotepadPlusPlus-Syntax-Plugin-[A-Za-z0-9.-]+\.zip', text)
        assert mentioned, name
        assert set(mentioned) <= expected, (name, mentioned)
