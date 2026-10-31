# Validation of Notepad++ Syntax Plugin for Foresight Platform Fore

[English](TESTING.md) | [Русский](TESTING.ru.md)


## Automatic detection

The separate detector and filename-filter suite passed **104/104** checks with LF/CRLF/CR ([detection-validation.json](../qa/detection-validation.json)). Real Notepad++ tests passed **17/17** scenarios: opening TXT/extensionless files, pasting into TXT/new tabs, negative examples, retaining another language, explicit Normal Text selection through the API and window command, returning to Normal Text after detection, closing and reopening. The detector leaves document text unchanged. Report: [autodetect-validation.json](../qa/autodetect-validation.json).

Algorithm and limits: [AUTODETECTION.md](AUTODETECTION.md).

Date: **31 October 2026**. Editor: **Notepad++ 8.9.8.1**, Windows. Compiler: **Zig 0.13.0**, C++17. The original 1.0.0 validation covered x64; 1.1.0 adds separate x86 evidence under `qa/x86/`.

## Lexer core

`tests/core.cpp` has 53 assertions covering styles, quotes, comments, numbers, identifiers, property/method folding, interface prototypes, delegates, nested procedures, a multiline End If phrase and incorrect closing words. Expected fold-header line numbers are specified manually.

Every structural example is checked with **LF, CRLF and CR**. The large-document fixture has 471,000 bytes and 36,001 lines; its timing is recorded in `qa/core-validation.json`. It measures a particular environment, not a speed guarantee for every computer.

## Editor integration

`qa/Probe.cpp` is a separate test plugin, loaded only into an isolated instance. It reads real Scintilla APIs:

- `SCI_GETLEXERLANGUAGE`, `SCI_GETSTYLEAT`, `SCI_GETFOLDLEVEL`;
- `SCI_STYLEGETFORE`, `SCI_STYLEGETBACK`, changing a color and reading it back.

The **Language/F/Fore** position is read from the actual window menu. The normal Notepad++ command opens **Style Configurator**; the test checks Fore in its ComboBox/ListBox, then cancels the dialog. It tests real editor controls, not just XML or menu source code.

Both palettes, two own examples with LF/CRLF, two states of an edited buffer and three external source files are covered: **18 scenarios** and **381 integration/dependency-integrity assertions**. Results: [validation.json](../validation.json). Own LF/CRLF fixtures have identical expected fold-header line numbers.

SHA-256 binds snapshots to the DLL, XML palette and input text. `python tools/verify.py` only rechecks this evidence. `qa/run.ps1` performs a fresh native run.

## Additional real source files

Local validation used:

- [DarkwingDuck48/mineswapper](https://github.com/DarkwingDuck48/mineswapper), commit `544262472bdb5e06047588f465615bb514ee3d61` — Minesweeper.
- [SemenovDaniil/fsightRepo](https://github.com/SemenovDaniil/fsightRepo), commit `240806c5d3710ecad94dec9349eefea811c5c21a` — Russian word declension and a database handler.

The public source archive includes results and hashes without these third-party source texts. Their authors' platform versions have not been established; opening them does not test all Foresight Platform Fore versions.

## Limits

Validation did not compile or execute programs in the platform, test Fore.NET or certify every Notepad++ release. ARM64 can be cross-compiled but has no native editor validation on this x64 host. ARM64 is excluded from the release and mandatory test matrix. The plugin provides lexical highlighting and folding, not semantic correctness checks.

## Installation tests

`install.ps1` was tested in a separate portable copy: DLL/XML match the tested build, a UDL backup is created and only the `fore` extension is removed from the UDL definition. All other definition elements match the original. Report: [installation-validation.json](../qa/installation-validation.json).

`qa/test-install.ps1` checks initial installation and update of both palettes in a separate portable directory, current DLL SHA-256, backups and the narrowly scoped UDL extension change. It leaves the user's main installation unchanged. The report is bound to the DLL and installer hashes.

## Automatic themes

Four native runs select real themes in Style Configurator: standard/custom configurations, first launch and restart. They cover 40 checks/snapshots, all 11 foreground/background categories in both views, preserved custom color/font, another active language and restored dark theme after restart. [Report](../qa/theme-validation.json), [behavior](AUTOMATIC-THEMES.md).

## About and source packaging

The integration probe opens the actual About dialog and reads `Artyom Nikiforov (rhyots)` and `https://github.com/rhyots` in both palette runs.

Python tests check exclusion of working directories before traversal, exact bytes and hashes, ZIP reproducibility regardless of input order, both binary archive layouts and prevention of release when required reports fail. The architecture test additionally checks that x86 packaging uses its own DLL and detection evidence.

```powershell
python -m pytest -q tests/test_release_files.py tests/test_release_archive.py
```

Quality measurements and their scope: [UB-MODE-REVIEW.md](UB-MODE-REVIEW.md).

## Separate x86 validation

Each architecture passed 381 integration assertions, 17 automatic-detection cases, 40 theme checks and installer validation. [x86 results](../qa/x86/validation.json), [x86 detection](../qa/x86/autodetect-validation.json), [x86 themes](../qa/x86/theme-validation.json), [x86 installation](../qa/x86/installation-validation.json). The x86 editor ran on Windows x64.

PowerShell scripts accept `-Architecture x86`; `tools/verify.py` and `tools/package.py` accept `--architecture x86`. Catalog-installer checks are tracked in [the Plugins Admin report](PLUGINS-ADMIN-READINESS.md).
