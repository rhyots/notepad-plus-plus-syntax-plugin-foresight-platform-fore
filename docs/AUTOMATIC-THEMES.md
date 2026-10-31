# Automatic palette switching

[English](AUTOMATIC-THEMES.md) | [Русский](AUTOMATIC-THEMES.ru.md)


Enabled by default since release 1.0.0. Copying XML whenever the theme changes is unnecessary.

The plugin detects the editor background through `NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR` and responds to `NPPN_DARKMODECHANGED`, `NPPN_WORDSTYLESUPDATED`, document activation and assigning Fore. A 50 ms delay lets Notepad++ finish updating its own styles. Background changes are also checked during UI updates.

The decision follows the text background, including a dark editor theme with light application chrome. The threshold is weighted RGB brightness 128/255. Unusual medium-brightness or saturated backgrounds select one of the two palettes; this does not guarantee individual contrast for every possible theme.

Only Scintilla views using the Fore lexer are updated, in both editor views. Other languages, document text, modified state, font family, size and emphasis are left as configured.

## Custom settings

For each category, automatic mapping replaces only a foreground color equal to that category's standard Light or Dark color. Any other user-selected color is preserved. Selecting exactly a standard palette color opts that category into automatic mapping.

A nonstandard fixed category background is also preserved. An ordinary inherited background follows the editor theme. Automatic switching does not rewrite `ForeLexer.xml`; the normal Save action in Style Configurator may save the XML through Notepad++ itself.

Style Configurator displays the base XML settings. Automatic standard-color mapping is applied to text in the editor. Nonstandard colors selected in the configurator remain the way to make custom choices.

## Palette sources and validation

`styles/ForeLexer-Light.xml` and `styles/ForeLexer-Dark.xml` are the source palettes. `tools/generate_palettes.py` generates `src/ForePaletteData.h` with Scintilla BGR values; `--check` rejects drift. Both XML presets are embedded in the DLL and verified by SHA-256.

`qa/run-theme.ps1` starts separate portable profiles. Its test plugin selects the real Default and DarkModeDefault themes in the actual Style Configurator. It verifies all 11 style categories in both views, repeated transitions, a custom color/font, another active language and restart with the saved dark theme. Results: [theme-validation.json](../qa/theme-validation.json).

```powershell
python tools/generate_palettes.py --check
pwsh -File qa/run-theme.ps1 -Zig 'C:\tools\zig-0.13.0\zig.exe'
```

Implementation references: official [Notepad++ notifications and API](https://github.com/notepad-plus-plus/notepad-plus-plus/blob/dd40fe45598791e9aa38f3aeb200e10fb3d6338a/PowerEditor/src/MISC/PluginsManager/Notepad_plus_msgs.h) and [theme-change handler](https://github.com/notepad-plus-plus/notepad-plus-plus/blob/dd40fe45598791e9aa38f3aeb200e10fb3d6338a/PowerEditor/src/WinControls/ColourPicker/WordStyleDlg.cpp).
