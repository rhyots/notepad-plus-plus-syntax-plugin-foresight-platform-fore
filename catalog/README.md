# Plugins Admin entries

[English](README.md) | [Русский](README.ru.md)


`plugin-x64.draft.json` and `plugin-x86.draft.json` are generated from final ZIP bytes by `python tools/package.py --architecture x64` and `--architecture x86`. Each `id` hashes the entire architecture-specific ZIP. `folder-name` is `ForeLexer`, matching `ForeLexer.dll`.

The homepage and version-specific asset URLs point to the rhyots GitHub repository. Minimum tested compatibility is Notepad++ 8.9.8.1 for both architectures, not every earlier ILexer5 editor. No ARM64 entry is submitted.

Before submitting, download the public assets, verify SHA-256 against `id`, run the official validator, and check installation/update/removal with official Debug Notepad++ and WinGUp. Submit the two entries to `src/pl.x64.json` and `src/pl.x86.json` in `notepad-plus-plus/nppPluginList`. [Readiness, evidence and limitations](../docs/PLUGINS-ADMIN-READINESS.md).

Keep released bytes immutable; changed ZIPs require a new release and hash. Catalog acceptance does not make the plugin part of the Notepad++ core. The maintainers decide whether and when it is included.
