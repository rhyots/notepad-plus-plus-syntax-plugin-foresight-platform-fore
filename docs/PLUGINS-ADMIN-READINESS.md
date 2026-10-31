# Plugins Admin readiness

[English](PLUGINS-ADMIN-READINESS.md) | [Русский](PLUGINS-ADMIN-READINESS.ru.md)


Checked on 2026-10-31. **Notepad++ Syntax Plugin for Foresight Platform Fore 1.1.0**, Windows **x64 and x86**. ARM64 is excluded from the release and required test matrix.

GitHub publication and acceptance into the [nppPluginList](https://github.com/notepad-plus-plus/nppPluginList) catalog are separate steps. Catalog inclusion does not make this plugin part of the Notepad++ core or default installation. **Submission is being prepared; maintainer acceptance is pending.**

## Official sources

Pinned snapshots make the audit reproducible:

- [Official manual: rules and local testing](https://github.com/notepad-plus-plus/npp-usermanual/blob/41d96945ec8d11d4100356293f4f30e60c1418c0/content/docs/plugins.md#rules-for-adding-your-plugins-into-list).
- [Catalog schema](https://github.com/notepad-plus-plus/nppPluginList/blob/9139ebdaa0fc1d26497bad36040f015f84bcf49f/pl.schema) and [official validator](https://github.com/notepad-plus-plus/nppPluginList/blob/9139ebdaa0fc1d26497bad36040f015f84bcf49f/validator.py).
- [x64 list](https://github.com/notepad-plus-plus/nppPluginList/blob/9139ebdaa0fc1d26497bad36040f015f84bcf49f/src/pl.x64.json) and [x86 list](https://github.com/notepad-plus-plus/nppPluginList/blob/9139ebdaa0fc1d26497bad36040f015f84bcf49f/src/pl.x86.json).

Source URLs, commits and hashes: [catalog/references.json](../catalog/references.json). Downloaded tooling remains in ignored build directories. No requirement to support every architecture or sign every DLL was found in these sources; this is an interpretation of the cited rules, not a guarantee of acceptance.

## Requirements

| Requirement | Project evidence |
|---|---|
| Architecture-specific entries | `catalog/plugin-x64.draft.json` and `catalog/plugin-x86.draft.json`; target `src/pl.x64.json` and `src/pl.x86.json` |
| Unique folder name matches DLL | `ForeLexer` / `ForeLexer.dll` |
| DLL at ZIP root | Separate `*-plugins-admin-x64.zip` and `*-plugins-admin-x86.zip` |
| DLL version matches catalog | Windows VERSIONINFO `1.1.0.0`, catalog `1.1.0` |
| Whole-package checksum | `id` is SHA-256 of the final ZIP, not the DLL; recomputed after packaging |
| Required JSON fields | folder-name, display-name, version, id, repository, description, author, homepage |
| Download URL | Version-specific GitHub Release asset; verify actual download before submission |
| Compatibility | `[8.9.8.1,]`, the minimum tested editor version for each architecture |
| Sources and licenses | Complete source ZIP, MIT for original work, retained third-party terms |

Unicode exports, machine type, ILexer5 entry points, version and both embedded palettes are checked from the actual DLL. On **each architecture**, real Notepad++ 8.9.8.1 passed 381 native assertions in 18 cases, 17 automatic-detection cases, 40 theme checks and isolated installer validation. Shared core/detector suites passed 53/104 assertions. [Validation details](TESTING.md).

## Installation, update and removal

The official instructions recommend portable Notepad++ plus official Debug Notepad++ and Debug WinGUp with a local test catalog. The isolated setup uses official artifacts, tied to the release's Notepad++ commit. The user's working installation is not modified.

The test Plugins Admin dialog was opened and its Available/Updates/Installed tabs observed. Subsequent accessibility and screenshot operations failed with tool timeouts. Therefore a complete mouse-driven Plugins Admin cycle is **not claimed**.

The official **WinGUp CLI**, the installation backend used by Plugins Admin, is exercised with public HTTPS release URLs and catalog SHA-256, followed by a real editor restart and the existing native integration probe. The initial x64 1.0.0 installation passed. The 1.0.0 → 1.1.0 update and x86 installation/removal are checked after publishing the immutable 1.1.0 assets; final results will be recorded on the repository's main branch.

The payload test independently checks first-run creation of `ForeLexer.xml`, preservation of customized XML, and actual registration in Language → F → Fore and Style Configurator. Native evidence is bound to DLL and palette hashes. A regular APPDATA deployment is not claimed by these portable tests. `-settingsDir` alone is not plugin-profile isolation: [upstream configuration code](https://github.com/notepad-plus-plus/notepad-plus-plus/blob/dd40fe45598791e9aa38f3aeb200e10fb3d6338a/PowerEditor/src/Parameters.cpp#L1330).

The manual describes relocation of `doc/` to `plugins/doc/ForeLexer/`; the tested WinGUp debug artifact retained it in `plugins/ForeLexer/doc/`. Archive documentation uses version-pinned repository links, so it works in either location.

## Reproducible checks and submission sequence

1. Build and test x64/x86 independently. Create the two final catalog ZIPs and corresponding JSON entries.
2. Pass Windows x64/x86 and Linux CI with LF/CRLF checkout coverage. [Workflow](https://github.com/rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore/actions/workflows/build.yml).
3. Publish immutable assets, download them again and compare whole-ZIP SHA-256 with `id`. If signing is used, sign before packaging/hashing.
4. Run the unmodified official validator and schema against actual public downloads; complete WinGUp install/update/remove checks and retain the method/limitations in evidence.
5. Submit only the x64/x86 catalog entries. Acceptance and distribution remain the catalog maintainers' decision.

Historical reports for 1.0.0 describe the earlier offline validator and extracted-payload checks. They are not substitutes for new-version public-download or lifecycle evidence. [Byte stability](BYTE-STABILITY.md) covers actual Git checkout tests and rejection of LF→CRLF drift.

To repeat the pinned local audit on Windows:

```powershell
python -m pip install --target build/catalog-deps -r requirements-catalog.txt
python tools/catalog_audit.py --refresh
```

`--refresh` downloads pinned references, not changing master. Refreshing the policy audit in the future requires deliberately updating those commits and repeating the checks.
