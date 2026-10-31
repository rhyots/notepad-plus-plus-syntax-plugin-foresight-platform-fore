# Plugins Admin package: Foresight Platform Fore

[English](PLUGINS-ADMIN-INSTALLATION.md) | [Русский](PLUGINS-ADMIN-INSTALLATION.ru.md)


Version 1.1.0 for Windows x64 and x86. Choose the package matching **Notepad++**, not Windows. The catalog ZIP has `ForeLexer.dll` at its root; Plugins Admin installs it in `<Notepad++>/plugins/ForeLexer/` and keeps documentation in `plugins/ForeLexer/doc/` with the tested WinGUp; the Notepad++ manual also describes `plugins/doc/ForeLexer/`.

Until the catalog contribution is accepted and distributed, use the manual ZIP from [Releases](https://github.com/rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore/releases). Publication on GitHub does not by itself make the plugin appear in Plugins Admin. See [catalog status](PLUGINS-ADMIN-READINESS.md).

After listing: open **Plugins → Plugins Admin → Available**, search for **Foresight**, tick the plugin and select **Install**. Save work before the requested editor restart. Later versions appear under **Updates**; removal is under **Installed → Remove**. Use only the package for your editor's architecture.

On first load the plugin creates `ForeLexer.xml` in the configuration directory returned by Notepad++, using its embedded light preset. Existing configuration, including installation-wide files, is preserved. Updates preserve that XML. Removing the plugin may leave settings behind; this preserves user customization for a later reinstall. If you want to remove those too, close Notepad++, back them up and remove only this plugin's `ForeLexer.xml` and any separate `plugins/doc/ForeLexer` directory. Documentation inside the plugin folder is removed with that folder.

Select **Language → F → Fore** and customize **Settings → Style Configurator → Fore**. Standard colors follow the editor's light/dark background in both views and after restart. Nonstandard colors and fonts remain unchanged; theme changes do not rewrite XML. Installation-wide configuration takes precedence over a profile copy. See [automatic themes](AUTOMATIC-THEMES.md).

If an older UDL owns `.fore`, remove `fore` from that UDL's **Ext.** field. The plugin does not modify UDL definitions at startup.

Content detection is restricted to Normal Text in TXT, extensionless files and new tabs. It checks after open/activation or edits, waits 350 ms and scans at most 128 KiB. Matching named Fore blocks are required; common words alone are insufficient. Strings/comments are ignored; explicit language choices, including Normal Text, take priority until close. Detection changes the language, never the text. See [detection details](AUTODETECTION.md) and `examples/Fore-autodetect-demo.txt` in the plugin directory.

Tested: Notepad++ 8.9.8.1 x64 and x86. Language rules: Prognoz Platform Fore 8.1 help. Later platform releases are not individually certified; Fore.NET and ARM64 are outside this release. The four light-palette colors were sampled from the supplied platform screenshot; [measurements and limitations](PLATFORM-COLORS.md).

Original project code, documentation and examples use the MIT License. Third-party components retain their own licenses; see [notices](../THIRD_PARTY_NOTICES.md). [Source repository](https://github.com/rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore). This is an independent project, not an official Foresight, Prognoz or Notepad++ product.
