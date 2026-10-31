"""Create binary and complete-source release archives with verified byte manifests."""
from pathlib import Path
import argparse
import hashlib
import json
import posixpath
import re
from urllib.parse import quote
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
    parser.add_argument('--architecture',choices=['x64','x86'],default='x64')
    args=parser.parse_args()
    subprocess.run([sys.executable,str(ROOT/'tools/check_bytes.py')],check=True)
    subprocess.run([sys.executable,str(ROOT/'tools/generate_palettes.py'),'--check'],check=True)
    subprocess.run([sys.executable,str(ROOT/'tools/verify.py'),'--architecture',args.architecture],check=True)
    out=ROOT/'dist';out.mkdir(exist_ok=True)
    if not args.source_only:
        package_binaries(out,args.architecture)
    source={SLUG+'/'+p.relative_to(ROOT).as_posix():p.read_bytes() for p in public_files()}
    archive(out/(ASSET+'-source.zip'),source)


def documentation_bytes(path):
    """Keep links usable in either installed documentation directory layout."""
    text=path.read_text(encoding='utf-8')
    def replace(match):
        target=match.group(1)
        if ':' in target or target.startswith('#'):
            return match.group(0)
        location,separator,anchor=target.partition('#')
        relative=posixpath.normpath((path.parent/ location).relative_to(ROOT).as_posix())
        if not (ROOT/relative).exists():
            return match.group(0)
        url=f'https://github.com/rhyots/{SLUG}/blob/v{VERSION}/'+quote(relative)
        return '('+url+(separator+anchor if separator else '')+')'
    return re.sub(r'(?<=\])\(([^)]+)\)',replace,text).encode('utf-8')


def release_documents():
    documents={p.name:documentation_bytes(p) for p in sorted((ROOT/'docs').glob('*.md'))}
    for name in ['CHANGELOG.md','CHANGELOG.ru.md','THIRD_PARTY_NOTICES.md','THIRD_PARTY_NOTICES.ru.md']:
        path=ROOT/name
        if path.exists():documents[name]=documentation_bytes(path)
    return documents


def native_evidence(architecture):
    base=ROOT if architecture=='x64' else ROOT/'qa'/architecture
    reports={'validation.json':base/'validation.json'}
    for name in ['autodetect-validation.json','theme-validation.json']:
        reports['qa/'+name]=(ROOT/'qa' if architecture=='x64' else base)/name
    for name in ['core-validation.json','detection-validation.json']:
        reports['qa/'+name]=ROOT/'qa'/name
    return reports


def validated_dll(reports,architecture):
    for path in reports.values():assert json.loads(path.read_text(encoding='utf-8'))['passed']
    dll=(ROOT/f'build/{architecture}/ForeLexer.dll').read_bytes()
    report=json.loads(reports['validation.json'].read_text())
    assert sha(dll)==report['dll_sha256'], 'Native tests belong to another DLL'
    for name,label in [('autodetect-validation.json','Automatic-detection'),('theme-validation.json','Theme-switching')]:
        assert json.loads(reports['qa/'+name].read_text())['dll_sha256']==sha(dll),label+' tests belong to another DLL'
    pe=inspect(dll)
    assert pe['machine']=={'x64':'AMD64','x86':'I386'}[architecture], 'Wrong DLL architecture'
    assert pe['file_version']==VERSION+'.0', 'Wrong DLL version'
    for name,key in [('Light','embedded_light_sha256'),('Dark','embedded_dark_sha256')]:
        assert pe[key]==sha((ROOT/f'styles/ForeLexer-{name}.xml').read_bytes())
    return dll


def package_binaries(out,architecture='x64'):
    reports=native_evidence(architecture)
    dll=validated_dll(reports,architecture)
    common={'presets/'+p.name:p.read_bytes() for p in sorted((ROOT/'styles').glob('*.xml'))}
    for name in ['Fore-demo.fore','Folding-edge-cases.fore','Fore-autodetect-demo.txt']:
        common['examples/'+name]=(ROOT/'examples'/name).read_bytes()
    common.update({name:path.read_bytes() for name,path in reports.items()})
    documents=release_documents()
    binary={**common,'plugins/ForeLexer/ForeLexer.dll':dll,'install.ps1':(ROOT/'install.ps1').read_bytes(),'LICENSE':(ROOT/'LICENSE').read_bytes()}
    binary.update({'docs/'+name:data for name,data in documents.items()})
    for name in ['CHANGELOG.md','CHANGELOG.ru.md','THIRD_PARTY_NOTICES.md','THIRD_PARTY_NOTICES.ru.md']:
        if name in documents:binary[name]=documents[name]
    for language in ['', '.ru']:
        binary['README'+language+'.md']=documentation_bytes(ROOT/f'docs/INSTALLATION{language}.md')
    binary.update({'licenses/'+p.name:p.read_bytes() for p in (ROOT/'vendor').glob('LICENSE*')})
    archive(out/(ASSET+f'-{architecture}.zip'),binary)
    catalog={**common,'ForeLexer.dll':dll,'doc/LICENSE':(ROOT/'LICENSE').read_bytes()}
    catalog.update({'doc/'+name:data for name,data in documents.items()})
    for language in ['', '.ru']:
        path=ROOT/f'docs/PLUGINS-ADMIN-INSTALLATION{language}.md'
        if path.exists():catalog['doc/README'+language+'.md']=documentation_bytes(path)
    catalog.update({'doc/licenses/'+p.name:p.read_bytes() for p in (ROOT/'vendor').glob('LICENSE*')})
    asset=ASSET+f'-plugins-admin-{architecture}.zip'
    archive(out/asset,catalog)
    draft={
        'folder-name':'ForeLexer','display-name':NAME,'version':VERSION,
        'npp-compatible-versions':'[8.9.8.1,]',
        'id':sha((out/asset).read_bytes()),
        'repository':f'https://github.com/rhyots/{SLUG}/releases/download/v{VERSION}/{asset}',
        'description':f'Native highlighting and folding for Foresight / Prognoz Platform Fore. Automatic light/dark palettes, custom colors and conservative plain-text detection. Windows {architecture}; not Fore.NET.',
        'author':'Artyom Nikiforov (rhyots)',
        'homepage':f'https://github.com/rhyots/{SLUG}'
    }
    (ROOT/f'catalog/plugin-{architecture}.draft.json').write_text(json.dumps(draft,indent=2)+'\n',encoding='utf-8',newline='\n')

if __name__=='__main__':main()
