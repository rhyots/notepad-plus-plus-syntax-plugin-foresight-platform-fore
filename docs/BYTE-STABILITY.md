# Exact bytes across Git checkouts

The project preserves the raw bytes that native-test evidence fingerprints.
This prevents the Windows CRLF / Unix LF mismatch previously encountered in a
different project's packaged JavaScript bundle.

- `.gitattributes` uses LF for maintained text, including YAML workflows and
  PowerShell scripts. PowerShell 7 supports LF.
- `src/`, `styles/`, `examples/`, `vendor/`, and recorded native snapshots use
  `-text`: Git must not convert their line endings. `ForeWords.h` deliberately
  retains the generator's CRLF bytes; SDK notices retain their upstream bytes.
- `.editorconfig` records the intended editor settings.
- `byte-lock.json` records SHA-256 for source, preset, fixture, and vendor files.
  `tools/check_bytes.py` rejects byte drift and CRLF/BOM in automation scripts.
- Native evidence separately binds the DLL, presets, fixtures, and lexer sources.
  An updated byte lock does not replace a fresh native test when implementation
  or preset bytes change.

`python tools/checkout_audit.py` creates an isolated, temporary local Git
repository under ignored `build/checkout-audit/`. It actually stages and commits
the public source, clones twice, and checks out with `core.autocrlf=false` and
`true` (including `core.eol=crlf` in the second case). It compares every source
file's raw hash, regenerates the dictionary, rechecks evidence, and requires the
two source ZIP files to have the same SHA-256. It does not create a remote or
commit into the working project. Results: `qa/checkout-validation.json`.
The ZIP hashes in that report identify the temporary audit snapshot. Final
release ZIP hashes are recorded separately in `dist/*.sha256`.

The prepared GitHub Actions matrix repeats the checks on Windows and Ubuntu,
with both autocrlf settings. A locally passing audit is not a completed remote
GitHub Actions run.

Packaging reads payloads with `read_bytes()`. ZIP entries have sorted paths,
fixed timestamps, fixed Unix file attributes, and use ZIP_STORED to avoid
compression-library differences. No newline conversion occurs while packaging.
Every payload is checked after writing, with SHA-256 manifests and an archive
sidecar. This guarantees deterministic packaging for the same inputs; it does
not claim that different compilers produce identical DLLs.

For intentional changes: edit the source, regenerate `ForeWords.h` when needed,
run `python tools/check_bytes.py --write`, rebuild, rerun native tests when
applicable, then rerun the byte and checkout checks. Never change an expected
hash merely to silence a failure. Calculate the catalog `id` from the final
Plugins Admin ZIP; changing any payload invalidates that previous `id`.

Git's authoritative rules: [gitattributes](https://git-scm.com/docs/gitattributes).
