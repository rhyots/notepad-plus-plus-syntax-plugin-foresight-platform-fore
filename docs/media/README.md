# Visual provenance

[English](README.md) | [Русский](README.ru.md)

The GIFs and PNGs are **lexer-rendered code previews**, not recordings or screenshots of the Notepad++ interface. Styles and fold levels come from the actual `fore::analyze` implementation. Standard colors come from the shipped XML presets. The renderer's styles/folds on the complete own fixture were compared byte-for-byte with a native Notepad++ snapshot. [Rendering record](render-validation.json).

The theme preview uses [mcp-data-sources.fore](../../examples/third-party/mcp-data-sources.fore), an excerpt of lines 17–60 of `fap_component/mod_mcp_server.fore` from [ForesightPlatform/fap_mcp_server](https://github.com/ForesightPlatform/fap_mcp_server/blob/cd6e589860225ee6201aadb603d4c586e5e5ea5d/fap_component/mod_mcp_server.fore#L17). Copyright (c) 2026 **Evgeny Bogdanov**, MIT. [Original license](../../examples/third-party/LICENSE.fap_mcp_server.txt). The supplied local file was checked against that pinned upstream version; only extraction and LF normalization were applied. [Hashes and provenance](provenance.json).

Folding uses the project's own `Fore-demo.fore`. The custom-color frame shows optional manual colors: keywords `#FFD166`, comments `#8BE28B`, strings `#FFA1B5`; it is not a third automatic preset. Frame duration: three seconds per theme, 2.8 seconds per fold state. Static PNGs are provided as alternatives to animation. English and Russian labels are generated separately.

The images show lexical formatting, not proof that the example compiles in any platform version. Font files are not included. Other supplied GitHub examples without an established redistribution license are not copied into this public repository.
