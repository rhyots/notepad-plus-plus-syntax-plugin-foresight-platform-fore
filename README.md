# Notepad++ Syntax Plugin for Foresight Platform Fore

[English](README.md) | [Русский](README.ru.md)

**Native syntax highlighting and code folding for Foresight Platform Fore in Notepad++ on Windows x64.**

The language appears under **Language → F → Fore**, with configurable colors and fonts under **Settings → Style Configurator → Fore**.

Version **1.0.0**. An independent project by Artyom Nikiforov (rhyots), not an official product of Foresight, Prognoz, or Notepad++.

The project name is **Notepad++ Syntax Plugin for Foresight Platform Fore**. Its language rules come from the original **Prognoz Platform Fore 8.1** documentation. `Fore` is the language identifier used by Notepad++; `ForeLexer.dll` and `ForeLexer.xml` are technical filenames retained for installation compatibility.

## Compatibility

| Component | Status |
|---|---|
| **Notepad++ 8.9.8.1, Windows x64** | Build and integration tested in the actual editor |
| **Prognoz Platform Fore 8.1** | Rules based on the original 8.1 help dated March 16, 2016 |
| Later Foresight Platform Fore releases | Not tested individually; compatibility is not certified |
| Fore.NET | Not a target language |
| Other Notepad++ releases, x86, ARM64 | Not tested; this release contains an x64 DLL only |

The plugin uses **ILexer5**, available in Notepad++ since version 8.4. The minimum **tested** editor version is **8.9.8.1 x64**. Programs have not been compiled or executed inside Prognoz Platform or Foresight Platform as part of this project's validation.

## Features

- 66 reserved keywords and 12 built-in types from the Prognoz Platform Fore 8.1 help.
- Strings, character literals, doubled quotes, and character codes such as `#1040`.
- Numbers, exponents, operators, and `//`, `///`, and `{ … }` comments.
- Case-insensitive keywords with identifier boundaries respected.
- Folding for classes, interfaces, namespaces, enumerations, methods, conditions, loops, `Select Case`, `Try`, and `With`.
- Method folding starts at `Sub`, `Function`, or `Constructor`, including local declarations. Interface signatures and delegates do not create spurious folds.
- New-style properties fold from `Property` to `End Property`, with separate `Get`/`Set` bodies. Old-style property declarations are handled separately.
- Correct fold markers verified with **LF, CRLF, and CR** line endings; converting files to CRLF is unnecessary.
- 11 configurable style categories, with light and dark presets.

This editor plugin provides lexical highlighting and folding. It does not include a compiler, IntelliSense, debugger, or type checker. Individual `Case` branches do not yet have separate folds.

## Installation

Save your work and close all Notepad++ windows before installation or update. Extract the **entire** `NotepadPlusPlus-Syntax-Plugin-NotepadPlusPlus-Syntax-Plugin-Foresight-Platform-Fore-1.0.0-x64.zip` archive.

### Install with PowerShell

Requires **PowerShell 7**. To install into `Program Files`, open **PowerShell 7 as Administrator** so Windows permits writing the DLL. From the extracted directory:

```powershell
pwsh -File install.ps1 -NotepadDirectory 'C:\Program Files\Notepad++'
```

The initial base XML can be selected with `-Palette Dark`; automatic theme following works with either base preset. For portable Notepad++, pass the directory containing its EXE. For a custom profile location, add `-ConfigurationDirectory 'C:\path\to\Notepad++-profile'`.

The installer:

1. Checks the EXE and DLL architecture.
2. Copies the DLL to `plugins\ForeLexer\ForeLexer.dll` beside the editor EXE.
3. Places `plugins\Config\ForeLexer.xml` in the **user configuration directory**, allowing regular Notepad++ sessions to save color changes.
4. Backs up files before changing them.
5. Removes the `fore` extension association from competing UDL definitions while keeping those definitions available.

Backups and the installation receipt are stored in `ForeLexerBackups` inside the configuration directory. The script does not close editor windows or change your source files.

### Manual installation

1. Copy `plugins\ForeLexer\ForeLexer.dll` from the archive into `plugins\ForeLexer\` under the installed Notepad++ directory.
2. Copy `presets\ForeLexer-Light.xml` or `presets\ForeLexer-Dark.xml` to `%APPDATA%\Notepad++\plugins\Config\ForeLexer.xml`.
3. For portable Notepad++ with local configuration, use `plugins\Config\ForeLexer.xml` beside the EXE. The filename must be **ForeLexer.xml**.
4. If an existing Foresight Platform Fore UDL owns the extension, remove `fore` from its **Ext.** field: UDL associations take precedence when selecting the language automatically.
5. Start Notepad++ and open a `.fore` file. The status bar should show **Fore**.

An existing `plugins\Config\ForeLexer.xml` in the editor installation directory takes precedence over the user-profile copy. Back it up and choose which configuration to use before installing another copy.

This XML configures an external lexer. Do not import it through **Define your language**.

## Automatic content detection

Normal Text documents are checked only when their names end in `.txt` (case
insensitive), have no extension, or belong to new unsaved tabs. Matching named
Fore blocks can activate Fore automatically after opening or pasting code.
Comments and strings are ignored; keywords such as Class/Begin/End alone do not
trigger detection. Other extensions and other selected languages are preserved.
The check runs after a 350 ms pause and reads at most the first 128 KiB.

An explicit language choice takes priority until the document is closed. Choosing
**Language → Normal Text** disables detection for that open document, including
when it is already Normal Text. Close and reopen it to allow detection again.
Text is never changed. Short, incomplete, unusually formatted, or syntactically
similar code may remain ambiguous. See [details](docs/AUTODETECTION.md) (Russian),
the [TXT example](examples/Fore-autodetect-demo.txt), and [CHANGELOG.md](CHANGELOG.md).

## Colors and fonts

The light preset uses four colors sampled from a user-provided platform screenshot:
black `#000000`, teal `#008787`, green `#008000`, and dark red `#800000`.
These are observed image pixels; antialiasing means they do not prove the exact
RGB settings inside the platform. See [measurements and mapping](docs/PLATFORM-COLORS.md)
(Russian). The dark preset preserves the same color roles with light gray
`#D4D4D4`, teal `#4FD1D1`, green `#6EC76E`, and soft red `#F08080`.
Its minimum nominal contrast on `#1E1E1E` is 6.43:1; use it with a dark editor theme.

Open **Settings → Style Configurator → Fore**, select a category, set its color, font, and emphasis, then click **Save**. Changes are stored in `ForeLexer.xml`; the installer backs up that file on subsequent installations.

Light and Dark palettes now follow the editor background automatically, including theme changes and restart. Standard role colors are mapped in both views; nonstandard manual colors and fonts are preserved. The base XML remains available in Style Configurator. See [automatic theme behavior](docs/AUTOMATIC-THEMES.md) (Russian).

## Validation

The recorded validation includes **53/53 core assertions** and **377/377 native integration and dependency-integrity assertions across 18 scenarios**. It checks:

- **Language / F / Fore** in the actual Notepad++ menu.
- **Fore in the actual Style Configurator language list**: the test opens the dialog, reads its controls, and cancels it without saving.
- Automatic external-lexer selection for `.fore` files.
- Per-byte styles, colors for all 11 categories, and exact fold-start lines.
- Both presets, LF/CRLF, and edits to an open buffer; core tests also cover CR.
- Three real Prognoz/Foresight Platform Fore source files from GitHub: Minesweeper, Russian word inflection, and a database connection handler. Their text is not redistributed in this project.

See [validation.json](validation.json) and the [detailed test report in Russian](docs/TESTING.md). Native snapshots are stored in `qa/results/`. These are editor tests, not certification against every platform release.

## Build from source

Requires **Zig 0.13.0** and PowerShell 7. SDK headers are vendored and pinned to Notepad++ tag `v8.9.8.1`; the build does not download dependencies.

```powershell
pwsh -File build.ps1 -Zig 'C:\tools\zig-0.13.0\zig.exe'
pwsh -File test.ps1 -Zig 'C:\tools\zig-0.13.0\zig.exe'
```

The output is `build\x64\ForeLexer.dll`. A CMake build is also provided for Windows x64:

```powershell
cmake -S . -B build/cmake -A x64
cmake --build build/cmake --config Release
ctest --test-dir build/cmake -C Release --output-on-failure
```

The local release was built with Zig 0.13.0. The prepared GitHub Actions workflow uses CMake; it has not yet been run on GitHub.

## Run the native Notepad++ test

```powershell
pwsh -File qa/run.ps1 `
  -NotepadDirectory 'C:\Program Files\Notepad++' `
  -Zig 'C:\tools\zig-0.13.0\zig.exe' `
  -Python 'python'
```

The script uses a separate EXE copy in `qa/notepad-test`, its own settings, and a test plugin. It does not modify the working user installation. Pass `-ExtraExamplesDirectory` to include optional GitHub examples. Without it, only the project's own fixtures are used, so the scenario and assertion counts differ from the full recorded run.

`python tools/verify.py` rechecks saved snapshots; it does not launch a new editor session. Use `python tools/package.py` to produce release archives and SHA-256 manifests.

## Sources and license

The language rules are grounded in 36 original HTML documents from the **Prognoz Platform Fore 8.1** help. Relative paths and SHA-256 hashes are recorded in [sources.json](sources.json). The proprietary help files and platform binaries are not redistributed.

The project is licensed under **GPL-3.0-or-later**. Notepad++ and Scintilla SDK notices are preserved. Pinned versions and hashes are recorded in [vendor/provenance.json](vendor/provenance.json). See [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) (Russian).

The lexer analyzes the whole document and caches the result between Lex and Fold. Incremental parsing optimizations for very large files have not yet been implemented.

## Release preparation

Exact-byte rules and real Git checkout tests are described in [BYTE-STABILITY.md](docs/BYTE-STABILITY.md). See the [Plugins Admin readiness report](docs/PLUGINS-ADMIN-READINESS.md) (Russian) and [catalog draft](catalog/README.md). A separate `*-plugins-admin-x64.zip` places the versioned DLL at ZIP root. On first load, the embedded light XML is created only if configuration is absent; existing settings are preserved. The catalog download URL still contains `OWNER`. No release or catalog submission has been published.
