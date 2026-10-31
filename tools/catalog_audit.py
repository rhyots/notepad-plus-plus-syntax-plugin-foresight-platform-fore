"""Audit the local catalog ZIP with the pinned official validator (offline download substitution)."""
from pathlib import Path
import argparse
import hashlib
import importlib.metadata
import json
import os
import runpy
import shutil
import sys
import urllib.request
import zipfile

from inspect_binary import inspect
from release_files import ROOT, ASSET, NAME

COMMIT = '9139ebdaa0fc1d26497bad36040f015f84bcf49f'
MANUAL_COMMIT = '41d96945ec8d11d4100356293f4f30e60c1418c0'
SOURCES = {
    name: f'https://raw.githubusercontent.com/notepad-plus-plus/nppPluginList/{COMMIT}/{name}'
    for name in ('pl.schema', 'validator.py', 'src/pl.x64.json')
}
SOURCES['plugins.md'] = f'https://raw.githubusercontent.com/notepad-plus-plus/npp-usermanual/{MANUAL_COMMIT}/content/docs/plugins.md'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--refresh', action='store_true', help='Download pinned official reference files into ignored build/')
    args = parser.parse_args()
    official = ROOT/'build/catalog-official'
    recorded = ROOT/'catalog/references.json'
    known = {item['url']: item['sha256'] for item in json.loads(recorded.read_text(encoding='utf-8'))['sources']} if recorded.exists() else {}
    references = []
    for name, url in SOURCES.items():
        path = official/name
        if args.refresh:
            path.parent.mkdir(parents=True, exist_ok=True)
            request = urllib.request.Request(url, headers={'User-Agent': 'Fore-release-readiness-audit'})
            with urllib.request.urlopen(request, timeout=30) as response:
                path.write_bytes(response.read())
        data = path.read_bytes()
        digest = hashlib.sha256(data).hexdigest()
        assert url not in known or known[url] == digest, f'Pinned official reference bytes changed: {name}'
        references.append({'file': name, 'url': url, 'sha256': digest})
    (ROOT/'catalog/references.json').write_text(json.dumps({'checked_on': '2026-10-31', 'sources': references}, indent=2)+'\n', encoding='utf-8', newline='\n')
    package = ROOT/'dist'/(ASSET+'-plugins-admin-x64.zip')
    payload = package.read_bytes()
    entry = json.loads((ROOT/'catalog/plugin-x64.draft.json').read_text(encoding='utf-8'))
    assert entry['id'] == hashlib.sha256(payload).hexdigest()
    with zipfile.ZipFile(package) as z:
        assert 'ForeLexer.dll' in z.namelist(), 'The official validator requires the DLL at ZIP root'
        assert z.testzip() is None
        binary = inspect(z.read('ForeLexer.dll'))
    assert binary['file_version'] == entry['version']+'.0'
    assert entry['display-name'] == NAME
    assert entry['folder-name'] == 'ForeLexer'
    catalog = json.loads((official/'src/pl.x64.json').read_text(encoding='utf-8'))
    for key in ('folder-name', 'display-name', 'repository'):
        assert entry[key] not in [item[key] for item in catalog['npp-plugins']], f'Catalog collision: {key}'
    deps = ROOT/'build/catalog-deps'
    for suffix in ('', 'win32', 'win32/lib', 'pywin32_system32'):
        sys.path.insert(0, str(deps/suffix))
    # pywin32 uses the real Windows version API; no binary metadata is mocked.
    dll_directory = os.add_dll_directory(str(deps/'pywin32_system32')) if sys.platform == 'win32' else None
    import requests
    sandbox = official/'local-fixture'
    (sandbox/'src').mkdir(parents=True, exist_ok=True)
    (sandbox/'doc').mkdir(exist_ok=True)
    shutil.copyfile(official/'pl.schema', sandbox/'pl.schema')
    fixture = {'name': 'npp-pluginList', 'version': catalog['version'], 'arch': '64', 'npp-plugins': [entry]}
    (sandbox/'src/pl.x64.json').write_text(json.dumps(fixture), encoding='utf-8', newline='\n')
    original_get = requests.Session.get
    old_argv, old_cwd = sys.argv[:], Path.cwd()
    api_url = os.environ.pop('APPVEYOR_API_URL', None)
    downloads = []
    def local_get(session, url, **kwargs):
        assert url == entry['repository'], 'Unexpected validator download'
        downloads.append(url)
        response = requests.Response()
        response.status_code = 200
        response._content = payload
        response.url = url
        return response
    requests.Session.get = local_get
    try:
        sys.argv = [str(official/'validator.py'), 'x64']
        os.chdir(sandbox)
        try:
            runpy.run_path(str(official/'validator.py'), run_name='__main__')
        except SystemExit as result:
            assert result.code in (None, 0), f'Official validator failed: {result.code}'
    finally:
        requests.Session.get = original_get
        sys.argv = old_argv
        os.chdir(old_cwd)
        if api_url is not None:
            os.environ['APPVEYOR_API_URL'] = api_url
        if dll_directory is not None:
            dll_directory.close()
    assert len(downloads) == 1
    report = {'passed': True, 'official_validator_commit': COMMIT,
              'mode': 'Unmodified official validator and schema; single-entry fixture; HTTP GET supplied with local ZIP bytes; real Windows version API',
              'archive': package.name, 'archive_sha256': entry['id'], 'dll_version': binary['file_version'],
              'machine': binary['machine'], 'unique_against_official_x64_catalog': True,
              'public_download_tested': False, 'contains_owner_placeholder': 'OWNER' in entry['repository'],
              'ready_to_submit': False,
              'dependencies': {name: importlib.metadata.version(name) for name in ('requests', 'jsonschema', 'pywin32')}}
    (ROOT/'qa/catalog-validation.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8', newline='\n')
    print('CATALOG: official schema, ZIP root, SHA-256, Windows DLL version and uniqueness passed offline. Public download and actual Plugins Admin lifecycle remain pending.')


if __name__ == '__main__':
    main()
