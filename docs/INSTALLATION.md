# Notepad++ Syntax Plugin for Foresight Platform Fore 1.0.0 — Windows x64

[English](INSTALLATION.md) | [Русский](INSTALLATION.ru.md)

Native syntax highlighting and folding for **Foresight Platform Fore**. The language appears under **Language → F → Fore**; colors and fonts are available under **Settings → Style Configurator → Fore**.

Tested in **Notepad++ 8.9.8.1 x64**. Language rules are based on the original **Prognoz Platform Fore 8.1** help. Later platform releases have not been tested individually. Fore.NET is not a target language. This independent project is not an official product of Foresight, Prognoz, or Notepad++.

## Installation

1. Extract the **entire** archive into a regular directory.
2. Save your work and close all Notepad++ windows.
3. For an installation into `Program Files`, open **PowerShell 7 as Administrator**.
4. From the extracted directory, run:

```powershell
pwsh -File install.ps1 -NotepadDirectory 'C:\Program Files\Notepad++'
```

The initial base XML can be selected with `-Palette Dark`; automatic theme following works with either base preset. For portable Notepad++, pass the directory containing its EXE. For a custom profile, use `-ConfigurationDirectory 'C:\path\to\profile'`.

The script checks x64 compatibility, backs up files, and installs the DLL and XML. If a UDL owns the `.fore` extension, only that association is removed; the UDL remains available for manual selection. Backups and the installation receipt are saved in `ForeLexerBackups` inside the editor configuration directory.

Start Notepad++ and open a `.fore` file. The status bar should show **Fore**. For manual selection, use **Language → F → Fore**.

## Manual installation

- Copy `plugins/ForeLexer/ForeLexer.dll` into `plugins/ForeLexer/` under the Notepad++ installation directory.
- Copy `presets/ForeLexer-Light.xml` or `presets/ForeLexer-Dark.xml` to `%APPDATA%\Notepad++\plugins\Config\ForeLexer.xml`.
- For portable Notepad++ with `doLocalConf.xml`, place the XML in `plugins/Config` beside the EXE.
- Remove `fore` from the **Ext.** field of any previously installed Foresight Platform Fore UDL.

`ForeLexer.dll`, `ForeLexer.xml`, and the language identifier `Fore` are technical names. The project's display name is **Notepad++ Syntax Plugin for Foresight Platform Fore**.

Notepad++ searches for the XML in the installation's `plugins/Config` directory before the user profile. Choose one configuration location and back up existing files first. Do not import this XML through the UDL dialog.

## Colors

Open **Settings → Style Configurator → Fore** to customize the base colors and fonts. Standard Light/Dark colors follow the editor background automatically. Nonstandard manual colors and font settings are preserved. Copying an XML preset for each theme change is unnecessary. See `docs/AUTOMATIC-THEMES.md` in the binary ZIP.

## Validation and source code

Recorded results: **53/53 core assertions**, **377/377 native integration and dependency-integrity assertions**, and **18 scenarios** across both presets. The integration test opened the actual Style Configurator and checked its language list. LF/CRLF folding was tested in the editor, with CR also covered by core tests.

See `validation.json` and `docs/TESTING.md` (Russian). Programs were not compiled or executed inside either platform during validation.

Project code, documentation and examples: **MIT License**. Third-party components retain their own licenses; see `THIRD_PARTY_NOTICES.md`. Complete source code, build tools, the full bilingual README, and native snapshots are supplied in the accompanying **NotepadPlusPlus-Syntax-Plugin-Foresight-Platform-Fore-1.0.0-source.zip** archive. Third-party license texts in this binary archive are under `licenses/`.

## Automatic detection

Normal Text documents with `.txt` or no extension (including new tabs) are checked after opening or pasting code. Matching named Fore blocks are required; strings and comments are ignored. Other languages/extensions are preserved. Choosing Normal Text explicitly disables detection until the document is closed. The check waits 350 ms and reads at most 128 KiB. Text is never modified. See [details](AUTODETECTION.md) and the binary ZIP's `examples/Fore-autodetect-demo.txt`.
