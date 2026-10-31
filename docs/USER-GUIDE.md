# User guide

[English](USER-GUIDE.md) | [Русский](USER-GUIDE.ru.md) · [Project overview](../README.md)

## Installation

Download the manual `*-x64.zip` from [Releases](https://github.com/rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore/releases/latest). Use Notepad++ **Windows x64**; 8.9.8.1 is the tested version. Save your work, close the editor and extract the entire archive before installing.

With PowerShell 7, run `pwsh -File install.ps1 -NotepadDirectory 'C:\Program Files\Notepad++'` from the extracted folder. Use administrator rights for Program Files. For portable Notepad++, pass its EXE directory. The script checks architecture, backs up replaced files and removes only the competing `fore` extension from UDL definitions. Backups and receipts go to `ForeLexerBackups` in the configuration directory.

`-Palette Dark` selects the initial base XML. Both presets support automatic theme following. For a nonstandard profile use `-ConfigurationDirectory 'C:\path\to\profile'`. [Detailed installer behavior](INSTALLATION.md).

## Manual installation

1. Close Notepad++.
2. Copy `plugins\ForeLexer\ForeLexer.dll` from the manual ZIP into `plugins\ForeLexer\` beside the installed `notepad++.exe`.
3. Copy `presets\ForeLexer-Light.xml` to `%APPDATA%\Notepad++\plugins\Config\ForeLexer.xml`. For portable Notepad++ with local configuration, use its local `plugins\Config\ForeLexer.xml`. Create the directories if necessary. You may choose the Dark preset instead; keep the destination filename **ForeLexer.xml**.
4. Restart Notepad++ and open a `.fore` file. Select **Language → F → Fore** if needed.

An XML in the installation directory's `plugins\Config\` takes precedence over the user-profile XML. Keep one intended active configuration. Do not import these XML files through “Define your language”: they configure an external lexer.

The `*-plugins-admin-x64.zip` uses a different layout: its DLL is at ZIP root. It is prepared for catalog distribution; the plugin is not yet listed in Plugins Admin.

## Opening Fore code

`.fore` is the associated extension. Other files can be assigned **Language → F → Fore** manually. The status bar should show Fore. Selecting the language does not compile or execute the file.

If an old Fore UDL owns the extension, open **Language → User Defined Language → Define your language**, choose that UDL and remove only `fore` from its extension field. The installer performs this migration with a backup.

## Automatic detection

The detector considers only **Normal Text** documents named `.txt` (case insensitive), with no extension, or new unsaved tabs. It checks after opening/activation and after a 350 ms editing pause, reading at most the first 128 KiB.

It looks for consistent named Fore blocks, ignoring comments and strings. Common keywords alone are insufficient. Other file extensions and already-selected languages are preserved. Choosing a language explicitly — including Normal Text — disables detection for that open document until it is closed. Close and reopen to allow detection again.

To try it: open [Fore-autodetect-demo.txt](../examples/Fore-autodetect-demo.txt), or paste its complete text into a new untitled Normal Text tab. Short or incomplete snippets may remain Normal Text. Manual Fore selection is always available.

## Colors and fonts

Open **Settings → Style Configurator → Fore**, select a style category, change foreground/font/emphasis and save. There are 11 categories, including keywords, types, strings, comments and documentation comments.

Switch the editor theme in **Settings → Style Configurator → Select theme**. The plugin follows the actual editor background automatically in both views, including after restart. Merely changing the application's chrome need not change the editor background.

Standard palette colors are managed automatically. A custom color different from both standard palette colors for that category is preserved, as are font settings. Selecting a standard color opts that category back into automatic mapping. The base XML shown by Style Configurator may differ from the runtime-adapted colors; automatic theme following does not rewrite XML.

For the demonstrated custom dark appearance, use keyword `#FFD166`, comment `#8BE28B` and string `#FFA1B5` on a dark editor background. These are optional manual choices, not an additional shipped preset. [Static custom preview](media/custom.png).

## Folding

Use the fold margin next to line numbers to collapse a block. Method headers include local declarations in their scope. New-style properties have a property fold and separate Get/Set bodies; interface signatures and delegates do not create false method folds.

Classes, namespaces, enums, conditions, loops, Select Case, Try and With are supported. Individual Case branches do not have separate folds. LF, CRLF and CR line endings are handled directly. [Folding examples](../examples/Folding-edge-cases.fore).

## Updating or removing

For an update, save your work and close the editor before replacing the DLL. The installer creates a fresh backup of the previous DLL/XML. It installs the selected preset, so retain your backed-up XML if you want to restore custom settings afterward. Automatic theme changes themselves preserve custom settings.

For manual removal, close the editor and remove its `plugins\ForeLexer` plugin folder. The separate `ForeLexer.xml` configuration can be retained for a future install. Removing the plugin does not restore a previous UDL association; add `fore` back manually if desired.

## Troubleshooting

| Symptom | What to check |
|---|---|
| No Fore under F | Correct x64 DLL, folder `plugins\ForeLexer\ForeLexer.dll`, complete restart |
| XML error or no Fore styles | `ForeLexer.xml` location/name; avoid conflicting installation-wide and profile copies |
| `.fore` is still plain text or a UDL | Competing UDL extension, then select Fore manually |
| TXT stays Normal Text | Eligible filename, complete named blocks, first 128 KiB, no manual opt-out on that open document |
| Colors do not change with the theme | Editor background actually changed; nonstandard custom colors intentionally stay fixed |
| Installer cannot replace DLL | All instances using that Notepad++ installation are closed; write permission for the installation folder |
| Code runs incorrectly in the platform | The plugin only highlights/folds; use the platform's compiler/debugger for execution errors |

For a bug report include plugin and Notepad++ versions, Windows architecture, editor theme, filename/extension, line endings, and a minimal shareable snippet. [Open an issue](https://github.com/rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore/issues/new/choose).

## What has been tested

Notepad++ 8.9.8.1 x64 on Windows, both palettes, both views, real theme changes, restart, manual overrides, edits, actual menu/Style Configurator/About registration, installer backups, and LF/CRLF/CR handling. Language rules come from Prognoz Platform Fore 8.1 help. Later platform versions are not individually certified; Fore.NET, x86 and ARM64 are not supported targets. Platform programs were not compiled or executed during validation.
