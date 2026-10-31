from pathlib import Path
import runpy
import shutil
import subprocess
import sys
import os
import tree_sitter_language_pack as pack

pack.configure({'cache_dir': str(Path('build/ub-parser-cache').resolve())})
module = sys.argv.pop(1)
sys.argv[0] = module
if module == 'mutator' and sys.platform == 'win32':
    def stop_test_group(pid, signal):
        subprocess.run(['taskkill', '/PID', str(pid), '/T', '/F'], capture_output=True)
    os.killpg = stop_test_group
    import mutator.workers as workers
    excluded = {'build', 'dist', 'target', '.git', '.metrics', '__pycache__', '.pytest_cache', '.coverage'}
    def isolated_copy(link, target):
        if target.name in excluded:
            return
        link.parent.mkdir(parents=True, exist_ok=True)
        if target.is_dir():
            shutil.copytree(target, link, ignore=shutil.ignore_patterns(*excluded))
        else:
            shutil.copy2(target, link)
    workers.symlink = isolated_copy
    original_create = workers.create_workers
    def checked_workers(*args, **kwargs):
        created = original_create(*args, **kwargs)
        for directory in created:
            baseline = subprocess.run([sys.executable, '-m', 'pytest', '-q',
                'tests/test_release_files.py', 'tests/test_release_archive.py'],
                cwd=directory, capture_output=True, text=True)
            if baseline.returncode:
                raise RuntimeError('Unmutated Windows overlay failed: '+baseline.stdout+baseline.stderr)
        return created
    workers.create_workers = checked_workers
runpy.run_module(module, run_name='__main__')
