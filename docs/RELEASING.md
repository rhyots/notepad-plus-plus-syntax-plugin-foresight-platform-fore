# Build and publish a release

[English](RELEASING.md) | [Русский](RELEASING.ru.md)

Repository: [rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore](https://github.com/rhyots/notepad-plus-plus-syntax-plugin-foresight-platform-fore).

## Prepare

1. Keep version values in CMake, the resource, packaging and installer consistent. Update both READMEs and CHANGELOG.
2. Build with Zig 0.13.0 and run `test.ps1`. For a changed DLL, run `qa/run.ps1`, `qa/run-autodetect.ps1`, `qa/run-theme.ps1` and `qa/test-install.ps1` against isolated copies of the tested Notepad++.
3. Update `byte-lock.json` only for intentional changes. Run `tools/check_bytes.py`, `tools/generate_palettes.py --check`, `tools/inspect_binary.py`, and the Python tests. Never replace a native test run by merely updating hashes.
4. Run `tools/package.py`, then `qa/test-catalog-install.ps1 -Archive <catalog-zip>` and `tools/catalog_audit.py`. Rebuild the source archive after updated reports.
5. Run `tools/checkout_audit.py`. It checks actual Git staging/checkouts with both autocrlf settings, including rejection of byte drift. Run `tools/package.py --source-only` afterward to include the new report.

## Publish to GitHub

Push the reviewed commit to `main`, let every matrix job pass, then tag the same commit as `v1.0.0`. Create the Release from [RELEASE-NOTES.md](RELEASE-NOTES.md), with [Russian notes](RELEASE-NOTES.ru.md) linked or appended. Upload all three prepared ZIPs and their `.sha256` sidecars; do not substitute GitHub's automatically generated source archives for tested assets.

After publishing, download the actual assets and compare SHA-256 to local bytes. Check that the README's download links, GIFs, language switch, license and guide links work. Repository description and topics should describe Fore, Foresight, Prognoz, Notepad++ and syntax highlighting accurately; do not imply support for Fore.NET or untested architectures.

If CI makes a source correction necessary before the first Release, complete it and use the corrected commit. Repackage source so its contents match the tagged public tree. Binary assets need rebuilding only when their inputs change; regenerate the catalog hash whenever its ZIP changes.

## Plugins Admin is a separate step

This GitHub release does not automatically add the plugin to Notepad++ Plugins Admin. See [catalog readiness](PLUGINS-ADMIN-READINESS.md). Its `id` is the SHA-256 of the entire `*-plugins-admin-x64.zip`. Validate the actual public URL, then complete the official install/update/remove procedure in an isolated test environment before submitting a catalog PR.

## Maintain visual documentation

The GIFs are code previews, not recordings of the editor. Build `qa/preview-tokens.cpp` with `src/ForeLexer.cpp`, then use `tools/render_previews.py --lexer <helper.exe> --language en` and `--language ru`. Install the optional dependencies from `requirements-media.txt`; the current renderer uses Windows Consolas/Segoe UI fonts. Keep code provenance and the upstream MIT license with the community excerpt. Details: [media provenance](media/README.md).
