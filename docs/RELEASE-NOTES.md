# 1.1.0 — Fore for both x64 and x86 Notepad++

[English](RELEASE-NOTES.md) | [Русский](RELEASE-NOTES.ru.md)


Native syntax highlighting and folding for **Foresight Platform Fore / Prognoz Platform Fore**. This release brings the same light/dark themes, custom colors and conservative plain-text detection to 32-bit Notepad++.

- **New x86 package**, tested in real Notepad++ 8.9.8.1 alongside x64. Select the editor's architecture, not Windows bitness.
- **Complete English and Russian documentation**, with English `.md` and Russian `.ru.md` files throughout the repository and binary packages.
- **Safe architecture matching** in the installer and packaging gates; x86 uses its own native evidence and correctly named lexer exports.
- **Plugins Admin preparation**: separate x64/x86 catalog ZIPs, full-archive SHA-256 and documentation links that survive WinGUp relocation. [Catalog status](PLUGINS-ADMIN-READINESS.md).

Unchanged daily features: **Language → F → Fore**, normal Style Configurator, 66 keywords, 12 types, 11 style categories, structural folding and theme following in both editor views. Manual colors/fonts are preserved. Automatic detection is limited to Normal Text in TXT, extensionless files and new tabs; explicit language choice wins.

Validation on **each architecture**: 381 native integration assertions, 17 detection cases, 40 theme checks and isolated installer validation. Shared core/detector checks: 53 and 104. Language rules come from Prognoz Platform Fore 8.1 help; later platform releases are not individually certified. Fore.NET and ARM64 are outside this release.

Download `*-x64.zip` or `*-x86.zip` for manual installation, the matching `*-plugins-admin-*.zip` for the catalog layout, or `*-source.zip` for complete source. Each ZIP has a SHA-256 sidecar. Save work and close Notepad++ before updating. Existing published 1.0.0 assets remain unchanged.

**MIT License** for original project code; third-party terms are retained. Author: [Artyom Nikiforov (rhyots)](https://github.com/rhyots).

[User guide](USER-GUIDE.md) · [Russian notes](RELEASE-NOTES.ru.md) · [Changelog](../CHANGELOG.md)
