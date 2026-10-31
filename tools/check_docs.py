"""Check public Markdown language pairs and local links before publication."""
import json
import re
from urllib.parse import unquote

from release_files import ROOT, public_files


def check_document(path):
    text = path.read_text(encoding='utf-8')
    russian = path.name.endswith('.ru.md')
    partner = path.with_name(path.name[:-6]+'.md') if russian else path.with_suffix('.ru.md')
    problems = []
    if not partner.is_file():
        problems.append('missing translation: '+partner.name)
    headings = re.findall(r'^#{1,6} (.+)$', text, re.M)
    if not russian and any(re.search('[А-Яа-яЁё]', h) for h in headings):
        problems.append('Russian heading in default English document')
    # Language navigation and literal examples are not a translation-quality proof.
    for target in re.findall(r'\]\(([^)]+)\)', text):
        if ':' in target or target.startswith('#'):
            continue
        location = unquote(target.split('#', 1)[0]).strip('<>')
        if not (path.parent/location).exists():
            problems.append('missing local link: '+target)
    return problems


def main():
    documents = [p for p in public_files() if p.suffix == '.md']
    errors = {p.relative_to(ROOT).as_posix(): problems for p in documents if (problems := check_document(p))}
    report = {
        'passed': not errors,
        'markdown_files': len(documents),
        'english_russian_pairs': len(documents)//2,
        'naming': 'English .md; complete Russian translation .ru.md',
        'checks': 'Counterparts, English headings, local file targets. Translation equivalence is manually reviewed.',
        'errors': errors,
    }
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return 1 if errors else 0


if __name__ == '__main__':
    raise SystemExit(main())
