import hashlib
import json
import subprocess
import sys
import zipfile

import pytest
import package
from package import archive
from release_files import ASSET, ROOT, SLUG, public_files


def test_archive_preserves_payload_bytes_and_hashes(tmp_path):
    payload = {'styles/palette.xml': b'first\r\nsecond\r\n', 'README.md': 'Fore — test\n'.encode()}
    target = tmp_path/'release.zip'
    archive(target, payload)
    with zipfile.ZipFile(target) as result:
        assert result.testzip() is None
        assert all(info.date_time == (2026, 10, 31, 0, 0, 0) for info in result.infolist())
        assert all(info.compress_type == zipfile.ZIP_STORED for info in result.infolist())
        assert set(result.namelist()) == set(payload) | {'SHA256SUMS.txt'}
        for name, data in payload.items():
            assert result.read(name) == data
        for line in result.read('SHA256SUMS.txt').decode().splitlines():
            digest, name = line.split('  ', 1)
            assert hashlib.sha256(result.read(name)).hexdigest() == digest
    assert target.with_suffix('.zip.sha256').read_text().split()[0] == hashlib.sha256(target.read_bytes()).hexdigest()


def test_archive_is_independent_of_input_order(tmp_path):
    payload = {'z.txt': b'z', 'a.txt': b'a'}
    first, second = tmp_path/'one.zip', tmp_path/'two.zip'
    archive(first, payload)
    archive(second, dict(reversed(list(payload.items()))))
    assert first.read_bytes() == second.read_bytes()


@pytest.fixture
def release_tree(tmp_path, monkeypatch):
    for path in public_files():
        target = tmp_path/path.relative_to(ROOT)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(path.read_bytes())
    dll = b'Isolated packaging fixture; PE validation is tested against the built DLL separately.'
    target = tmp_path/'build/x64/ForeLexer.dll'
    target.parent.mkdir(parents=True)
    target.write_bytes(dll)
    for name in ['validation.json', 'qa/autodetect-validation.json', 'qa/theme-validation.json',
                 'qa/core-validation.json', 'qa/detection-validation.json']:
        path = tmp_path/name
        report = {'passed': True, 'dll_sha256': hashlib.sha256(dll).hexdigest()}
        path.write_text(json.dumps(report))
    monkeypatch.setattr(package, 'ROOT', tmp_path)
    monkeypatch.setattr(package, 'inspect', lambda data: {
        'embedded_light_sha256': hashlib.sha256((tmp_path/'styles/ForeLexer-Light.xml').read_bytes()).hexdigest(),
        'embedded_dark_sha256': hashlib.sha256((tmp_path/'styles/ForeLexer-Dark.xml').read_bytes()).hexdigest(),
    })
    return tmp_path


def test_binary_layouts_and_catalog_digest(release_tree):
    out = release_tree/'dist'
    out.mkdir()
    package.package_binaries(out)
    with zipfile.ZipFile(out/(ASSET+'-x64.zip')) as manual:
        assert manual.read('plugins/ForeLexer/ForeLexer.dll') == (release_tree/'build/x64/ForeLexer.dll').read_bytes()
        assert 'install.ps1' in manual.namelist()
        assert 'presets/ForeLexer-Dark.xml' in manual.namelist()
        assert '(docs/AUTODETECTION.md)' in manual.read('README.md').decode()
    catalog_zip = out/(ASSET+'-plugins-admin-x64.zip')
    with zipfile.ZipFile(catalog_zip) as catalog:
        assert catalog.read('ForeLexer.dll') == (release_tree/'build/x64/ForeLexer.dll').read_bytes()
        assert 'doc/README.md' in catalog.namelist()
        assert 'install.ps1' not in catalog.namelist()
    draft = json.loads((release_tree/'catalog/plugin-x64.draft.json').read_text())
    assert draft['id'] == hashlib.sha256(catalog_zip.read_bytes()).hexdigest()
    assert draft['repository'].endswith('/'+catalog_zip.name)
    assert draft['folder-name'] == 'ForeLexer'


@pytest.mark.parametrize('name', ['qa/core-validation.json', 'qa/detection-validation.json',
                                 'qa/autodetect-validation.json', 'qa/theme-validation.json', 'validation.json'])
def test_failed_validation_prevents_binary_release(release_tree, name):
    path = release_tree/name
    report = json.loads(path.read_text())
    report['passed'] = False
    path.write_text(json.dumps(report))
    with pytest.raises(AssertionError):
        package.package_binaries(release_tree)
    assert not list(release_tree.glob('*.zip'))


@pytest.mark.parametrize('source_only', [True, False])
def test_release_command_checks_sources_and_builds_source_archive(release_tree, monkeypatch, source_only):
    commands = []
    monkeypatch.setattr(package.subprocess, 'run', lambda command, **kwargs: commands.append(command))
    monkeypatch.setattr(package, 'public_files', lambda: public_files(release_tree))
    monkeypatch.setattr(sys, 'argv', ['package.py']+(['--source-only'] if source_only else []))
    package.main()
    assert [command[1].replace('\\', '/').split('/')[-1] for command in commands] == [
        'check_bytes.py', 'generate_palettes.py', 'verify.py']
    assert (release_tree/'dist'/(ASSET+'-x64.zip')).exists() == (not source_only)
    assert (release_tree/'dist'/(ASSET+'-plugins-admin-x64.zip')).exists() == (not source_only)
    with zipfile.ZipFile(release_tree/'dist'/(ASSET+'-source.zip')) as source:
        assert source.read(SLUG+'/README.md') == (release_tree/'README.md').read_bytes()
        assert all('/build/' not in name and '/dist/' not in name for name in source.namelist())
    package.main()  # A repeat run must also work when dist already exists.


@pytest.mark.parametrize('failing_tool', ['check_bytes.py', 'generate_palettes.py', 'verify.py'])
def test_failed_preflight_stops_packaging(release_tree, monkeypatch, failing_tool):
    for name in ['check_bytes.py', 'generate_palettes.py', 'verify.py']:
        (release_tree/'tools'/name).write_text('raise SystemExit('+('17' if name == failing_tool else '0')+')\n')
    monkeypatch.setattr(sys, 'argv', ['package.py', '--source-only'])
    with pytest.raises(subprocess.CalledProcessError) as failure:
        package.main()
    assert failure.value.returncode == 17
    assert not (release_tree/'dist').exists()
