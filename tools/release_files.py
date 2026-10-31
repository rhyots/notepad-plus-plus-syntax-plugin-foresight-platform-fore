"""Shared public-source selection and release names; no workstation paths."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION = '1.0.0'
NAME = 'Notepad++ Syntax Plugin for Foresight Platform Fore'
SLUG = 'notepad-plus-plus-syntax-plugin-foresight-platform-fore'
ASSET = 'NotepadPlusPlus-Syntax-Plugin-Foresight-Platform-Fore-' + VERSION


def public_files(root: Path = ROOT):
    for path in sorted(root.rglob('*')):
        relative = path.relative_to(root)
        if any(part in {'build', 'dist', '__pycache__', '.git'} or part.startswith('notepad-')
               for part in relative.parts[:-1]):
            continue
        if not path.is_file() or path.suffix.lower() in {'.dll', '.exe', '.pyc', '.o', '.obj', '.res', '.pdb', '.lib', '.zip'}:
            continue
        yield path
