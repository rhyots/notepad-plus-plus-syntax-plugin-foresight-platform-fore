# Third-party notices

[English](THIRD_PARTY_NOTICES.md) | [Русский](THIRD_PARTY_NOTICES.ru.md)

Original project code, scripts, documentation and examples are under the **MIT License**, Copyright (c) 2026 Artyom Nikiforov (rhyots). The root LICENSE contains the standard [MIT text](https://opensource.org/license/mit). This does not relicense the third-party components below; their notices and full licenses are preserved.

## Notepad++ SDK

Headers in `vendor/npp-sdk/` come from [notepad-plus-plus/notepad-plus-plus](https://github.com/notepad-plus-plus/notepad-plus-plus), tag v8.9.8.1, commit `dd40fe45598791e9aa38f3aeb200e10fb3d6338a`. Original header notices are retained.

- Notepad++ API headers: GPL-3.0-or-later; `vendor/LICENSE.Notepad++.txt`.
- Scintilla interfaces and Lexilla definitions: permissive licenses by Neil Hodgson and other contributors; `vendor/LICENSE.Scintilla.txt` and `vendor/LICENSE.Lexilla.txt`.

Original paths, URLs and SHA-256 values are recorded in `vendor/provenance.json`. Git line-ending conversion is disabled for these files.

## Compiler and C++ runtime

The tested DLL is built with Zig 0.13.0 and its LLVM libc++, libc++abi and libunwind components. Their notices are preserved in `vendor/LICENSE.libcxx.txt`, `vendor/LICENSE.libcxxabi.txt` and `vendor/LICENSE.libunwind.txt`, including Apache License 2.0 with LLVM exceptions and historical component notices.

The compiler itself is not bundled. Building requires Zig or CMake with a compatible Windows x64 or x86 compiler. Complete plugin sources are supplied in the accompanying `*-source.zip`. Binary archives retain the runtime/SDK licenses under `licenses/` or `doc/licenses/`.

## Platform documentation and local validation examples

Proprietary Prognoz Platform 8.1 help and platform binaries are not redistributed. Only relative documentation paths, hashes and keyword/type lists are recorded. Product names, documentation and platform rights remain with their owners.

The three original GitHub source files used for local validation are not redistributed. Own fixtures in `examples/` were written for this project. The separately licensed documentation excerpt below is included in the source archive.

## Community example used in documentation

`examples/third-party/mcp-data-sources.fore` reproduces lines 17–60 of `fap_component/mod_mcp_server.fore` from [ForesightPlatform/fap_mcp_server](https://github.com/ForesightPlatform/fap_mcp_server), commit `cd6e589860225ee6201aadb603d4c586e5e5ea5d`.

Copyright (c) 2026 **Evgeny Bogdanov**, **MIT License**. The original license is retained alongside the excerpt in `examples/third-party/LICENSE.fap_mcp_server.txt`. The excerpt is included in the source archive and depicted in visual documentation. Source and extraction details: `docs/media/provenance.json`.

Preview images are rendered from lexer output, not screenshots of Notepad++. Consolas/Segoe UI font files are not distributed.
