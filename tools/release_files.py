"""Shared public-source selection and release names; no workstation paths."""
from pathlib import Path
import os

ROOT = Path(__file__).resolve().parents[1]
VERSION = '1.1.0'
NAME = 'Notepad++ Syntax Plugin for Foresight Platform Fore'
SLUG = 'notepad-plus-plus-syntax-plugin-foresight-platform-fore'
ASSET = 'NotepadPlusPlus-Syntax-Plugin-Foresight-Platform-Fore-' + VERSION
EXCLUDED_DIRECTORIES = frozenset({'build', 'dist', '__pycache__', '.git', '.metrics',
                                  '.pytest_cache', '.venv', 'venv', 'target', 'htmlcov'})
EXCLUDED_SUFFIXES = frozenset({'.dll', '.exe', '.pyc', '.o', '.obj', '.res', '.pdb', '.lib', '.zip'})


def source_directory(name: str) -> bool:
    return name not in EXCLUDED_DIRECTORIES and not name.startswith('notepad-')


def source_file(path: Path) -> bool:
    return path.name != '.coverage' and path.suffix.lower() not in EXCLUDED_SUFFIXES


def public_files(root: Path = ROOT):
    # os.walk does not follow directory symlinks by default.
    for current, directories, files in os.walk(root):
        directories[:] = sorted(name for name in directories if source_directory(name))
        for name in sorted(files):
            path = Path(current)/name
            if source_file(path):
                yield path
