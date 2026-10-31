# Changelog

[English](CHANGELOG.md) | [Русский](CHANGELOG.ru.md)


## [1.1.0] — 2026-10-31

### Added

- Windows x86 build, with matching installer, release packages and separate native test evidence. Choose the architecture of Notepad++, not the operating system.
- Complete English/Russian documentation pairs: English `.md` and Russian `.ru.md`, including technical guides, changelog and catalog instructions.
- Architecture-aware packaging: the DLL, version, embedded palettes and native evidence must match before an archive can be created.

### Fixed

- Exported lexer entry-point names on x86 while preserving the required calling convention.
- Documentation links in binary archives remain usable after WinGUp relocates `doc/`.
- Removed the x64-only build restriction; the release and catalog targets are x64 and x86. ARM64 is outside the test and release matrix.

### Validation

- Notepad++ 8.9.8.1 x64 and x86: each passed 381 native integration assertions in 18 cases, 17 automatic-detection cases, 40 theme checks and the isolated installer test.
- Separate catalog packages and SHA-256 values for x64 and x86. Plugins Admin submission and lifecycle status are tracked in [the readiness report](docs/PLUGINS-ADMIN-READINESS.md); catalog inclusion requires maintainer acceptance.

## [1.0.0] — 2026-10-31

### Added

- English GitHub landing page and full Russian translation, bilingual user/publication guides, issue forms and animated theme/folding previews with localized captions.
- A real MIT-licensed excerpt from ForesightPlatform/fap_mcp_server, credited to Evgeny Bogdanov with pinned source and license.
- Native ILexer5 external lexer for Foresight Platform Fore, Windows x64; F → Fore menu and standard Style Configurator.
- Rules grounded in 36 Prognoz Platform Fore 8.1 documents: 66 keywords and 12 types; strings, characters, numbers, operators, comments and 11 styles.
- Folding for structures, methods with local declarations, new-style properties and Get/Set; interface prototypes, delegates and old-style properties handled separately; LF/CRLF/CR support.
- Content detection restricted to Normal Text in TXT, extensionless files and new tabs. Matching named blocks, strings/comments excluded, 128 KiB limit and 350 ms debounce. Manual language choices take priority until the document closes.
- Light palette sampled from a platform screenshot: #000000, #008787, #008000, #800000; measurements and evidence limitations retained.
- Dark palette: #D4D4D4, #4FD1D1, #6EC76E, #F08080 on #1E1E1E, minimum calculated contrast 6.43:1.
- Automatic light/dark colors follow the editor background, in both views and after restart; nonstandard manual colors and fonts are preserved without rewriting XML.
- Author in About: Artyom Nikiforov (rhyots), https://github.com/rhyots.
- Installer with architecture checks, backups and migration of the .fore extension from UDL.
- VERSIONINFO 1.0.0.0, both embedded XML palettes; missing user configuration is created without overwriting existing configuration.
- English/Russian README, instructions, reports and original project examples.
- Separate manual, Plugins Admin and source ZIPs; SHA-256 for files and whole archives; byte locks and Git LF/CRLF checks.
- Draft catalog entry and local audit against pinned official rules. The complete Plugins Admin lifecycle is verified separately from GitHub publication; status and limitations are in docs/PLUGINS-ADMIN-READINESS.md.

### Fixed during preparation

- MIT License selected by the author for original code, documentation and examples; LICENSE, README, instructions and DLL metadata updated. Third-party SDK/component licenses retained separately.
- Repeated ZIP-name prefix in README.
- Metrics, caches, virtual environments and working directories excluded from source archives; traversal stops before reading those directory trees.

### Quality and validation

- Detector, scanner, folding, notifications and palette application refactored into smaller functions without changing behavior.
- 53/53 core assertions, 104/104 detector assertions, 381/381 native integration assertions in 18 cases, 17/17 automatic-detection cases and 40 theme checks across four runs.
- 16 Python tests for source selection, reproducible archives, checksums, ZIP layouts and rejection of packaging when required checks fail.
- Isolated installer test: both palettes, update, backups and narrowly scoped UDL migration.
- ub-mode application and measurement boundaries documented in docs/UB-MODE-REVIEW.md.

The first public release was 1.0.0; there were no earlier public releases. Its checks ran on 2026-10-31 with Notepad++ 8.9.8.1 x64. Fore programs were not compiled inside the platform; later platform versions and Fore.NET were not certified.
