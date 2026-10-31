"""Exercise real Git add/clone/checkouts with core.autocrlf=false and true."""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import sys
import uuid

from release_files import ROOT, public_files


def run(*args, cwd):
    return subprocess.run(args, cwd=cwd, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout


def main():
    sources = list(public_files())
    expected = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
    work = ROOT/'build'/'checkout-audit'/uuid.uuid4().hex
    seed = work/'seed'
    seed.mkdir(parents=True)
    for path in sources:
        target = seed/path.relative_to(ROOT)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(path, target)
    run('git', 'init', '--initial-branch=codex/byte-audit', str(seed), cwd=ROOT)
    run('git', '-c', 'core.autocrlf=false', 'add', '.', cwd=seed)
    run('git', '-c', 'user.name=Local byte audit', '-c', 'user.email=byte-audit@example.invalid',
        '-c', 'commit.gpgsign=false', 'commit', '-m', 'Temporary local byte snapshot', cwd=seed)
    results = []
    archive_hash = None
    for autocrlf in ('false', 'true'):
        target = work/('checkout-'+autocrlf)
        run('git', 'clone', '--no-hardlinks', '--no-checkout', str(seed), str(target), cwd=ROOT)
        run('git', 'config', 'core.autocrlf', autocrlf, cwd=target)
        run('git', 'config', 'core.eol', 'crlf' if autocrlf == 'true' else 'lf', cwd=target)
        run('git', 'checkout', '--force', 'HEAD', cwd=target)
        actual = {p.relative_to(target).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in public_files(target)}
        assert actual == expected, f'Checkout changed bytes with core.autocrlf={autocrlf}: {[n for n in expected if actual.get(n) != expected[n]]}'
        for script in ('check_bytes.py', 'verify.py'):
            output = run(sys.executable, '-X', 'utf8', str(target/'tools'/script), cwd=target)
            print(output.decode('utf-8').strip())
        # A semantically harmless newline change must still fail the byte gate.
        probe = target/'src/ForeLexer.cpp'
        original = probe.read_bytes()
        try:
            probe.write_bytes(original.replace(b'\n', b'\r\n'))
            rejected = subprocess.run([sys.executable, '-X', 'utf8', str(target/'tools/check_bytes.py')],
                                      cwd=target, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            assert rejected.returncode == 1 and b'src/ForeLexer.cpp' in rejected.stdout, 'Byte drift was not rejected'
        finally:
            probe.write_bytes(original)
        run(sys.executable, '-X', 'utf8', str(target/'tools/generate_words.py'), cwd=target)
        assert hashlib.sha256((target/'src/ForeWords.h').read_bytes()).hexdigest() == expected['src/ForeWords.h']
        run(sys.executable, '-X', 'utf8', str(target/'tools/generate_palettes.py'), cwd=target)
        assert hashlib.sha256((target/'src/ForePaletteData.h').read_bytes()).hexdigest() == expected['src/ForePaletteData.h']
        run(sys.executable, '-X', 'utf8', str(target/'tools/package.py'), '--source-only', cwd=target)
        archives = list((target/'dist').glob('*-source.zip'))
        assert len(archives) == 1
        digest = hashlib.sha256(archives[0].read_bytes()).hexdigest()
        assert archive_hash is None or digest == archive_hash, 'Source ZIP bytes differ between checkouts'
        archive_hash = digest
        results.append({'core.autocrlf': autocrlf, 'all_source_bytes_identical': True,
                        'files': len(expected), 'crlf_byte_drift_rejected': True, 'audit_snapshot_source_zip_sha256': digest})
    report = {'passed': True, 'method': 'Temporary local Git repository, actual add/clone/checkout; no remote or public repository created',
              'platform': sys.platform, 'cases': results}
    (ROOT/'qa/checkout-validation.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8', newline='\n')
    print(f'CHECKOUT: {len(expected)} files unchanged under both Git settings; source ZIPs byte-identical.')


if __name__ == '__main__':
    main()
