"""Regenerate the compiled Foresight Platform Fore dictionary from the source manifest."""
from pathlib import Path
import json

ROOT=Path(__file__).resolve().parents[1]
manifest=json.loads((ROOT/'sources.json').read_text(encoding='utf-8'))
text=('#pragma once\n// Generated from sources.json: original Prognoz Platform Fore 8.1 help.\nnamespace fore {\n'
      'inline constexpr const char* kReserved = "'+' '.join(manifest['reserved_keywords'])+'";\n'
      'inline constexpr const char* kTypes = "'+' '.join(manifest['built_in_types'])+'";\n}\n')
(ROOT/'src/ForeWords.h').write_bytes(text.replace('\n','\r\n').encode('utf-8'))
print('Generated ForeWords.h from sources.json')
