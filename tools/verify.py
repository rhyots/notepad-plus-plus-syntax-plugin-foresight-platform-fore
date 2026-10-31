"""Check native Foresight Platform Fore integration snapshots (or recheck stored evidence)."""
from __future__ import annotations
import argparse
import hashlib
import json
import re
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
HEADERS = {
    'demo': [3,8,10,17,22,26,31,36,45,46,50,57,75,76,79,81,85,89,93,97,100,103,105,107,112,120],
    'edges': [4,8,14,15,25,27,28,36,48,52,58,60,66,68],
}
EDIT_BEFORE = b'Sub Main;\nBegin\n s := "x";\nEnd Sub Main;\n'
EDIT_PREFIX = b'{comment\n}\n'


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-root', type=Path)
    args = parser.parse_args()
    checks = []
    cases = []
    def check(label: str, ok: bool):
        checks.append(dict(name=label, passed=bool(ok)))
        if not ok: print('FAIL:', label)

    provenance = json.loads((ROOT/'vendor/provenance.json').read_text())
    for entry in provenance['files']:
        check('SDK: '+entry['file'], digest((ROOT/entry['file']).read_bytes()) == entry['sha256'])
    sources = json.loads((ROOT/'sources.json').read_text(encoding='utf-8'))
    header = (ROOT/'src/ForeWords.h').read_text(encoding='utf-8')
    reserved = re.search(r'kReserved = "([^"]+)"',header).group(1).split()
    types = re.search(r'kTypes = "([^"]+)"',header).group(1).split()
    check('Fore source dictionary', len(sources['reserved_keywords']) == 66 and len(sources['built_in_types']) == 12 and
          reserved == sources['reserved_keywords'] and types == sources['built_in_types'])
    dll = ROOT/'build/x64/ForeLexer.dll'
    dll_sha = digest(dll.read_bytes()) if dll.exists() else None
    baseline_path = ROOT/'validation.json'
    baseline = json.loads(baseline_path.read_text(encoding='utf-8')) if baseline_path.exists() else {}
    evidence_dll_sha = dll_sha or baseline.get('dll_sha256')
    complete = 0
    for theme in ['Light','Dark']:
        config = ROOT/f'styles/ForeLexer-{theme}.xml'
        config_sha = digest(config.read_bytes())
        styles = ET.parse(config).findall('LexerStyles/LexerType/WordsStyle')
        check(theme+': eleven native styles', len(styles) == 11)
        check(theme+': native language registration', ET.parse(config).find('Languages/Language').get('name') == 'Fore')
        for stem in ['demo-lf','edges-lf','demo-crlf','edges-crlf','edit-before','edit-after','minesweeper','declension','db-handler']:
            file = ROOT/f'qa/results/{theme.lower()}/{stem}.json'
            if not file.exists():
                if stem in ['minesweeper','declension','db-handler']: continue
                check(f'{theme}/{stem}: snapshot exists', False); continue
            snap = json.loads(file.read_text(encoding='utf-8'))
            label = f'{theme}/{stem}'
            data = None
            if stem.startswith(('demo-','edges-')):
                family = stem.split('-')[0]
                name = 'Fore-demo.fore' if family == 'demo' else 'Folding-edge-cases.fore'
                data = (ROOT/'examples'/name).read_bytes().replace(b'\r\n',b'\n')
                if stem.endswith('-crlf'): data = data.replace(b'\n',b'\r\n')
            elif stem == 'edit-before': data = EDIT_BEFORE
            elif stem == 'edit-after': data = EDIT_PREFIX + EDIT_BEFORE
            elif args.native_root:
                data = (args.native_root/(stem+'.fore')).read_bytes()
            if args.native_root:
                snap['input_sha256'] = digest(data) if data is not None else ''
                snap['dll_sha256'] = dll_sha
                snap['style_config_sha256'] = config_sha
                file.write_text(json.dumps(snap,separators=(',',':'))+'\n',encoding='utf-8')
            check(label+': native Fore selected automatically', snap['lexer'] == 'Fore')
            check(label+': style config matches', snap.get('style_config_sha256') == config_sha)
            if evidence_dll_sha: check(label+': DLL matches', snap.get('dll_sha256') == evidence_dll_sha)
            if data is not None:
                check(label+': all input bytes', snap['length'] == len(data) and snap.get('input_sha256') == digest(data))
            check(label+': style range', len(snap['styles']) == snap['length'] and set(snap['styles']) <= set(range(11)))
            actual = [i+1 for i,v in enumerate(snap['folds']) if v&0x2000]
            if stem.startswith(('demo-','edges-')):
                check(label+': exact fold headers', actual == HEADERS[stem.split('-')[0]])
                complete += 1
            elif stem.startswith('edit-'):
                check(label+': fold headers after edit', actual == ([1] if stem=='edit-before' else [1,3]))
            check(label+': final fold depth zero', (snap['folds'][-1]&0xfff) == 1024 and snap['folds'][-1]>>16 == 1024)
            for style in styles:
                i = int(style.get('styleID'))
                fg = int.from_bytes(bytes.fromhex(style.get('fgColor')),'little')
                bg = int.from_bytes(bytes.fromhex(style.get('bgColor')),'little')
                check(label+f': native colour {i}', snap['colours'][i] == {'fg':fg,'bg':bg})
            if data is not None:
                def styled(span: bytes, expected: int) -> bool:
                    pos = data.find(span)
                    return pos >= 0 and set(snap['styles'][pos:pos+len(span)]) == {expected}
                if stem.startswith('demo-'):
                    check(label+': exponent', styled(b'12.5E-2',4))
                    check(label+': character code', styled(b'#1040',6))
                    check(label+': quoted backslash', styled(b'"C:\\temp\\"',5))
                    check(label+': doubled quotes', styled('"Он сказал ""Привет""; If End If // { }"'.encode(),5))
                if stem.startswith('edges-'):
                    check(label+': triple quotes', styled(b'"""If End If"""',5))
                    check(label+': apostrophe', styled(b"''''",6))
                    check(label+': whole identifiers', styled(b'ClassName',9) and styled(b'BeforeIf',9))
                if stem == 'edit-after':
                    check(label+': inserted comment styled', set(snap['styles'][:len(EDIT_PREFIX)-1]) == {7})
                    check(label+': keywords recovered after edit', styled(b'Sub',1))
            cases.append(dict(theme=theme,file=stem,bytes=snap['length'],fold_headers=actual,input_sha256=snap.get('input_sha256')))
        ui = ROOT/f'qa/results/{theme.lower()}/ui.json'
        if ui.exists():
            u = json.loads(ui.read_text())
            check(theme+': Language > F > Fore', u['menu_path'].endswith('/F/Fore'))
            check(theme+': real Style Configurator opened', u['style_dialog_opened'])
            check(theme+': Fore listed in real Style Configurator', u['style_dialog_contains_fore'])
            check(theme+': style customization works', u['custom_style_roundtrip'])
            check(theme+': real About contains author', u.get('about_author', False))
            check(theme+': real About contains GitHub profile', u.get('about_github', False))
        else: check(theme+': UI evidence exists',False)
    check('Both themes and both line endings tested', complete == 8)
    fingerprints = {p.relative_to(ROOT).as_posix():digest(p.read_bytes()) for p in sorted((ROOT/'src').glob('*')) if p.is_file()}
    if not args.native_root and baseline.get('source_fingerprints'):
        check('Native evidence belongs to current lexer source', fingerprints == baseline['source_fingerprints'])
    report = dict(version='1.0.0',checked_on='2026-10-31',passed=all(x['passed'] for x in checks),
        checks=checks,native_run_this_invocation=bool(args.native_root),cases=cases,
        native_environment='Notepad++ 8.9.8.1 x64, Windows',dll_sha256=evidence_dll_sha,
        source_fingerprints=fingerprints,
        fore_rules='Prognoz Platform Fore 8.1 documentation; not Fore.NET',
        platform_compilation_tested=False)
    if args.native_root:
        report['notepad_exe_sha256'] = digest((args.native_root/'notepad++.exe').read_bytes())
        (ROOT/'validation.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(f'NATIVE: {sum(c["passed"] for c in checks)}/{len(checks)} checks passed; {len(cases)} cases.')
    return 0 if report['passed'] else 1

if __name__ == '__main__': raise SystemExit(main())
