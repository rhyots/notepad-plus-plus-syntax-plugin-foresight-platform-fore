# Notepad++ Syntax Plugin for Foresight Platform Fore

Version 1.0.0, Windows x64. This ZIP uses the Plugins Admin layout:
`ForeLexer.dll` is at the ZIP root. Extracting the ZIP into
`<Notepad++>/plugins/ForeLexer/` reproduces its installed payload.

On first load the plugin creates `ForeLexer.xml` in the configuration directory
returned by Notepad++, using the embedded light preset. Existing configuration
files, including installation-wide configuration, are preserved.

Select **Language → F → Fore**. Customize colors under
**Settings → Style Configurator → Fore**. Standard Light/Dark palette colors follow
the editor background automatically, in both views and after restarting. Nonstandard
manual colors and fonts are preserved. XML replacement on theme changes is unnecessary.
See `AUTOMATIC-THEMES.md`. An installation-wide configuration still takes precedence
over the profile copy.

If an existing UDL owns `.fore`, remove `fore` from that UDL's **Ext.** field
for automatic selection. The plugin does not change UDL definitions at startup.

Tested editor: Notepad++ 8.9.8.1 x64. Rules: Prognoz Platform Fore 8.1 help.
Other platform releases have not been individually tested; Fore.NET is not a target.
License: GPL-3.0-or-later. Source repository: replace OWNER before publication:
`https://github.com/OWNER/notepad-plus-plus-syntax-plugin-foresight-platform-fore`.

This is an independent project, not an official product of Foresight, Prognoz, or Notepad++.

Content detection is enabled only for Normal Text documents with `.txt` or no
extension, including new unsaved tabs. It runs after opening/activation or an
edit, with a 350 ms delay and a 128 KiB scan limit. Matching named Fore blocks
are required; common keywords alone are insufficient. Strings and comments are
ignored. Explicit language choices, including Normal Text, take priority until
the document is closed. Detection changes the language, not the text.
See `AUTODETECTION.md` (Russian) and the TXT example under `../examples/`.

The light palette's four colors are sampled from the provided platform screenshot;
see `PLATFORM-COLORS.md` for measurements and limitations.
