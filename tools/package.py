"""Create binary and complete-source release archives with verified byte manifests."""
from pathlib import Path
import argparse
import hashlib
import json
import subprocess
import sys
import zipfile

from release_files import ROOT, VERSION, NAME, SLUG, ASSET, public_files
from inspect_binary import inspect

def sha(data): return hashlib.sha256(data).hexdigest()

def archive(path, payload):
    sums = ''.join(f'{sha(data)}  {name}\n' for name,data in sorted(payload.items()))
    payload = dict(payload); payload['SHA256SUMS.txt'] = sums.encode()
    # Stored entries avoid zlib-version differences; metadata and payload bytes are fixed.
    with zipfile.ZipFile(path,'w',zipfile.ZIP_STORED) as z:
        for name,data in sorted(payload.items()):
            info=zipfile.ZipInfo(name,date_time=(2026,10,31,0,0,0));info.compress_type=zipfile.ZIP_STORED;info.create_system=3;info.external_attr=0o100644<<16;z.writestr(info,data)
    with zipfile.ZipFile(path) as z:
        assert z.testzip() is None
        for name,data in payload.items(): assert z.read(name)==data,name
    (path.parent/(path.name+'.sha256')).write_text(f'{sha(path.read_bytes())}  {path.name}\n',encoding='utf-8',newline='\n')
    print(f'{path.name}: {len(payload)} files, {path.stat().st_size} bytes; all bytes and CRC verified.')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-only',action='store_true',help='Package verified source without requiring a local DLL')
    args=parser.parse_args()
    subprocess.run([sys.executable,str(ROOT/'tools/check_bytes.py')],check=True)
    subprocess.run([sys.executable,str(ROOT/'tools/generate_palettes.py'),'--check'],check=True)
    subprocess.run([sys.executable,str(ROOT/'tools/verify.py')],check=True)
    out=ROOT/'dist';out.mkdir(exist_ok=True)
    if not args.source_only:
        package_binaries(out)
    source={SLUG+'/'+p.relative_to(ROOT).as_posix():p.read_bytes() for p in public_files()}
    archive(out/(ASSET+'-source.zip'),source)


def package_binaries(out):
    core=json.loads((ROOT/'qa/core-validation.json').read_text());assert core['passed']
    detection=json.loads((ROOT/'qa/detection-validation.json').read_text());assert detection['passed']
    automatic=json.loads((ROOT/'qa/autodetect-validation.json').read_text());assert automatic['passed']
    themes=json.loads((ROOT/'qa/theme-validation.json').read_text());assert themes['passed']
    report=json.loads((ROOT/'validation.json').read_text());assert report['passed']
    dll=(ROOT/'build/x64/ForeLexer.dll').read_bytes();assert sha(dll)==report['dll_sha256']
    assert automatic['dll_sha256']==sha(dll), 'Automatic-detection tests belong to another DLL'
    assert themes['dll_sha256']==sha(dll), 'Theme-switching tests belong to another DLL'
    pe=inspect(dll)
    assert pe['embedded_light_sha256']==sha((ROOT/'styles/ForeLexer-Light.xml').read_bytes())
    assert pe['embedded_dark_sha256']==sha((ROOT/'styles/ForeLexer-Dark.xml').read_bytes())
    binary={'plugins/ForeLexer/ForeLexer.dll':dll}
    binary['THIRD_PARTY_NOTICES.ru.md']=(ROOT/'THIRD_PARTY_NOTICES.ru.md').read_bytes()
    binary['docs/AUTOMATIC-THEMES.md']=(ROOT/'docs/AUTOMATIC-THEMES.md').read_bytes()
    binary['qa/theme-validation.json']=(ROOT/'qa/theme-validation.json').read_bytes()
    for p in sorted((ROOT/'styles').glob('*.xml')):binary['presets/'+p.name]=p.read_bytes()
    for name in ['README.md','LICENSE','THIRD_PARTY_NOTICES.md','CHANGELOG.md','install.ps1','validation.json','docs/TESTING.md','docs/AUTODETECTION.md','docs/PLATFORM-COLORS.md','examples/Fore-autodetect-demo.txt','qa/core-validation.json','qa/detection-validation.json','qa/autodetect-validation.json','qa/palette-sampling.json','qa/dark-palette-contrast.json','qa/installation-validation.json']:
        binary[name]=(ROOT/name).read_bytes()
    for language, filename in [('','INSTALLATION.md'),('.ru','INSTALLATION.ru.md')]:
        text=(ROOT/'docs'/filename).read_text(encoding='utf-8')
        text=text.replace('(INSTALLATION.md)', '(README.md)').replace('(INSTALLATION.ru.md)', '(README.ru.md)')
        text=text.replace('(AUTODETECTION.md)', '(docs/AUTODETECTION.md)')
        binary['README'+language+'.md']=text.encode('utf-8')
    for p in (ROOT/'vendor').glob('LICENSE*'):binary['licenses/'+p.name]=p.read_bytes()
    archive(out/(ASSET+'-x64.zip'),binary)
    catalog={'ForeLexer.dll':dll}
    catalog['doc/AUTOMATIC-THEMES.md']=(ROOT/'docs/AUTOMATIC-THEMES.md').read_bytes()
    catalog['qa/theme-validation.json']=(ROOT/'qa/theme-validation.json').read_bytes()
    catalog['examples/Fore-autodetect-demo.txt']=(ROOT/'examples/Fore-autodetect-demo.txt').read_bytes()
    catalog['doc/AUTODETECTION.md']=(ROOT/'docs/AUTODETECTION.md').read_bytes()
    catalog['doc/CHANGELOG.md']=(ROOT/'CHANGELOG.md').read_bytes()
    catalog['doc/PLATFORM-COLORS.md']=(ROOT/'docs/PLATFORM-COLORS.md').read_bytes()
    catalog['qa/palette-sampling.json']=(ROOT/'qa/palette-sampling.json').read_bytes()
    catalog['qa/dark-palette-contrast.json']=(ROOT/'qa/dark-palette-contrast.json').read_bytes()
    for p in sorted((ROOT/'styles').glob('*.xml')):catalog['presets/'+p.name]=p.read_bytes()
    for name in ['LICENSE','THIRD_PARTY_NOTICES.md','THIRD_PARTY_NOTICES.ru.md']:
        catalog['doc/'+name]=(ROOT/name).read_bytes()
    for p in (ROOT/'vendor').glob('LICENSE*'):catalog['doc/licenses/'+p.name]=p.read_bytes()
    catalog['doc/README.md']=(ROOT/'docs/PLUGINS-ADMIN-INSTALLATION.md').read_bytes()
    asset=ASSET+'-plugins-admin-x64.zip'
    archive(out/asset,catalog)
    draft={
        'folder-name':'ForeLexer','display-name':NAME,'version':VERSION,
        'npp-compatible-versions':'[8.9.8.1,]',
        'id':sha((out/asset).read_bytes()),
        'repository':f'https://github.com/rhyots/{SLUG}/releases/download/v{VERSION}/{asset}',
        'description':'Native highlighting and code folding for Foresight Platform Fore, based on Prognoz Platform Fore 8.1 documentation. Light/dark presets; content detection for plain TXT and extensionless documents. Windows x64; not Fore.NET.',
        'author':'Artyom Nikiforov (rhyots)',
        'homepage':f'https://github.com/rhyots/{SLUG}'
    }
    (ROOT/'catalog/plugin-x64.draft.json').write_text(json.dumps(draft,indent=2)+'\n',encoding='utf-8',newline='\n')

if __name__=='__main__':main()
