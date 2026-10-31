# Notepad++ Syntax Plugin for Foresight Platform Fore

[English](README.md) | [Русский](README.ru.md)

Native syntax highlighting, code folding and automatic light/dark palettes for **Foresight Platform Fore** in Notepad++ on Windows x64. The language appears in **Language → F → Fore** and **Settings → Style Configurator → Fore**.

**1.0.0 — first release prepared, not published.** An independent project by [Artyom Nikiforov (rhyots)](https://github.com/rhyots). It is not an official Foresight, Prognoz or Notepad++ product.

## Compatibility

| Component | What has been verified |
|---|---|
| Notepad++ 8.9.8.1, Windows x64 | Built and tested in the actual editor |
| Prognoz Platform Fore 8.1 | Language rules derived from 36 original help documents dated March 16, 2016 |
| Later Foresight Platform Fore versions | Not individually tested or certified |
| Fore.NET | Not supported as a target language |
| Other editor versions, x86 and ARM64 | Not tested; only an x64 DLL is provided |

The plugin uses ILexer5, introduced in Notepad++ 8.4; **8.9.8.1 x64 is the minimum tested version**. Validation does not include compiling or running programs inside Prognoz Platform or Foresight Platform.

## Features

- Case-insensitive highlighting: 66 reserved words, 12 built-in types, constants, identifiers, numbers, operators, strings, character literals and character codes such as `#1040`.
- Doubled quotes, `//`, `///` and `{ … }` comments; 11 configurable style categories.
- Folding for classes, interfaces, namespaces, enumerations, methods, properties, conditions, loops, `Select Case`, `Try` and `With`.
- Method folds include local declarations; interface signatures and delegates do not create false folds. Old and new property declarations are distinguished.
- LF, CRLF and CR line endings; no source conversion is required.
- Automatic content detection for eligible plain-text documents, with manual language choices taking priority.
- Light and dark palettes follow the editor background automatically in both views and after restart. Nonstandard custom colors and font settings are preserved.
- About includes the author and GitHub profile.

There is no compiler, debugger, IntelliSense or semantic checking. Individual `Case` branches do not have separate folds. Analysis currently scans the whole document and caches it between Lex/Fold calls.

## Install

Save your work and close Notepad++ before installing or updating. Extract the **entire** `NotepadPlusPlus-Syntax-Plugin-Foresight-Platform-Fore-1.0.0-x64.zip` archive. In PowerShell 7, from the extracted folder:

```powershell
pwsh -File install.ps1 -NotepadDirectory 'C:\Program Files\Notepad++'
```

Run PowerShell as Administrator when writing into Program Files. For a portable installation, pass the directory containing `notepad++.exe`. Optional parameters: `-Palette Dark` for the initial base XML, and `-ConfigurationDirectory 'C:\path\to\profile'` for a custom profile.

The installer checks architecture, copies the DLL and XML, backs up previous files and removes only the competing `fore` extension from UDL definitions. Backups and receipts go into `ForeLexerBackups` in the configuration directory. Automatic theme following works with either base preset.

For manual installation, copy `plugins\ForeLexer\ForeLexer.dll` under the Notepad++ installation and a preset from `presets\` to `%APPDATA%\Notepad++\plugins\Config\ForeLexer.xml`. Portable installations use their local `plugins\Config\` folder. An installation-wide XML takes precedence over a user-profile XML. These are external-lexer settings: do not import them as a UDL.

Full instructions: [English](docs/INSTALLATION.md), [Russian](docs/INSTALLATION.ru.md). Existing Fore UDL associations may take precedence over the plugin; remove `fore` from their extension field. Open a `.fore` file and check that its language is **Fore**.

## Automatic detection

Content is checked only while the language is **Normal Text**, for `.txt` files (case insensitive), extensionless files and new unsaved tabs. The detector looks for matching named Fore blocks; `Class`, `Begin` and `End` alone are insufficient. Strings and comments are ignored.

Checks run after a 350 ms pause and inspect at most the first 128 KiB. Other extensions and selected languages are preserved. An explicit language choice, including Normal Text, disables detection for that document until it is closed. Text is never modified. Short, incomplete or ambiguous fragments may need manual language selection.

See [detection details](docs/AUTODETECTION.md) (Russian) and the [TXT example](examples/Fore-autodetect-demo.txt).

## Colors and themes

| Role | Light / white background | Dark / `#1E1E1E` background |
|---|---|---|
| Text | `#000000` | `#D4D4D4` |
| Keywords | `#008787` | `#4FD1D1` |
| Comments | `#008000` | `#6EC76E` |
| Strings | `#800000` | `#F08080` |

Light colors were sampled from a supplied platform screenshot. Antialiasing means these pixels do not establish the exact configured RGB values in the original IDE. Dark colors preserve those roles; the minimum calculated contrast on `#1E1E1E` is 6.43:1.

Standard role colors adapt to the actual editor background. Nonstandard custom colors and fonts are retained. Style Configurator displays the base XML; runtime theme adaptation does not rewrite it. See [color measurements](docs/PLATFORM-COLORS.md) and [automatic themes](docs/AUTOMATIC-THEMES.md) (Russian).

## Validation

Local results for the prepared DLL, Notepad++ 8.9.8.1 x64:

| Check | Result |
|---|---|
| C++ lexer core | 53/53 assertions |
| Content detector and filename policy | 104/104 assertions |
| Native editor integration | 381/381 assertions, 18 scenarios |
| Native automatic detection | 17/17 scenarios |
| Native theme changes, both views, restart, custom fonts/colors | 40 checks in four runs |
| Installer | Isolated portable install/update, hashes, backups and UDL migration |
| Catalog ZIP | Isolated first-run startup and existing XML preservation |
| Python release tooling | 16 tests; exact bytes, deterministic archives and source selection |

Native tests open the actual Style Configurator and About dialogs. Three additional GitHub Fore examples were opened for local validation; their source text is not redistributed. See [test details](docs/TESTING.md), [validation.json](validation.json) and [ub-mode review](docs/UB-MODE-REVIEW.md). Saved snapshots are evidence of a completed run, not a new run every time they are checked.

## Build and test

Requires Zig 0.13.0, C++17 and PowerShell 7. Pinned SDK headers are included; the build does not download them.

```powershell
pwsh -File build.ps1 -Zig 'C:\tools\zig-0.13.0\zig.exe'
pwsh -File test.ps1 -Zig 'C:\tools\zig-0.13.0\zig.exe'
python -m pip install -r requirements-dev.txt
python -m pytest -q tests/test_release_files.py tests/test_release_archive.py
```

DLL output: `build\x64\ForeLexer.dll`. CMake is also provided:

```powershell
cmake -S . -B build/cmake -A x64
cmake --build build/cmake --config Release
ctest --test-dir build/cmake -C Release --output-on-failure
```

`qa/run.ps1`, `qa/run-autodetect.ps1`, `qa/run-theme.ps1` and `qa/test-install.ps1` use isolated editor copies. `python tools/verify.py` rechecks saved evidence. `python tools/package.py` produces the manual, Plugins Admin and source ZIPs with SHA-256 manifests. [Exact-byte rules](docs/BYTE-STABILITY.md) cover LF/CRLF and real Git checkouts. The prepared GitHub Actions matrix has not yet run on GitHub.

## Publication status

Suggested repository: `notepad-plus-plus-syntax-plugin-foresight-platform-fore`.

Source and release packages are prepared locally. **No repository, Release or catalog submission has been published.** The [Plugins Admin entry](catalog/plugin-x64.draft.json) remains a draft: replace `OWNER`, publish the exact ZIP after authorization, verify the public download, run the official validator against it and test the complete Plugins Admin install/update/remove cycle. Passing an offline catalog audit does not complete those steps. See [readiness](docs/PLUGINS-ADMIN-READINESS.md).

## Sources and license

Rules are based on **Prognoz Platform Fore 8.1**, not Fore.NET. Document paths and hashes are listed in [sources.json](sources.json); proprietary help and platform binaries are not included. `Fore` is the editor language identifier; `ForeLexer.dll` and `ForeLexer.xml` are technical installation filenames.

**GPL-3.0-or-later**. SDK and runtime notices are preserved in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), [LICENSE](LICENSE) and [vendor/provenance.json](vendor/provenance.json).
