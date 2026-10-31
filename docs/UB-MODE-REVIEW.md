# Release preparation log: ub-mode

[English](UB-MODE-REVIEW.md) | [Русский](UB-MODE-REVIEW.ru.md)


Date: 2026-10-31. Initial goal: review the code, reduce justified complexity and prepare a clean first release, 1.0.0, without publishing.

The locally installed [ub-mode](https://github.com/unclebob/ub-mode/blob/master/SKILL.md) skill was applied. The initial instruction not to upload prohibited pushes and external publication; local commits were allowed. Later publication was explicitly authorized, as recorded below. This log preserves the sequence of decisions.

## Actions and reasons

1. Read SKILL.md and workspace instructions to establish the rules and scope before editing.
2. Reviewed public files, existing evidence and `.gitattributes` to preserve the tested baseline and byte-level provenance.
3. Checked availability and official READMEs for Crapper, Dryer and Mutator. They support Python, not C++; their results are not presented as C++ measurements.
4. Found the repeated ZIP name in README and a large detection function; selected concrete changes instead of mechanically rewriting the project.
5. Downloaded quality tools into ignored `build/ub-tools` so development tools/dependencies do not enter the release.
6. Git HTTPS failed with schannel SEC_E_NO_CREDENTIALS; used read-only official GitHub archives pinned to commits.
7. Created local branch `codex/first-release` and a baseline commit without a remote, preserving a reversible starting point. A baseline ZIP is in build/ub-review.
8. Repeated C++ baseline tests: 53/53 and 104/104. Added behavioral package tests; first reproduced three expected failures: working-file leakage, traversal of ignored trees and a wrong README ZIP name.
9. Lizard measured maximum C++ complexity of 65/62/41 for detection, scanning and folding. This is CC, not CRAP; C++ coverage was not measured.
10. Crapper and Dryer both failed on a shared tree-sitter cache in the user profile. Removed the faulty assumption that they must write to AppData: assigned a local build cache instead of expanding permissions.
11. Fixed archive names and public-file selection against the three reproduced failures. Working trees are pruned before traversal, removing redundant work instead of adding caching.
12. Preserved the baseline and refactored C++ into named operations for detection, scanning, folding, configuration, notifications and palettes. Repeated behavioral tests after each group of changes.
13. Added tests using real ZIPs and temporary files. Mutations run only in private copies, never in production sources.
14. Mutator exposed Windows assumptions: symlinks need permissions and the timeout handler uses unavailable os.killpg. qa/run-quality.py adapts only isolation I/O and termination of its own test process. Unchanged tests must pass in every copy before mutations; the official mutation rules remain unchanged.
15. After the first mutation run, strengthened metadata, repeated-run, archive-content and failed-preflight checks. Test fixtures are independent of the native run's current red/green state. Removed redundant followlinks=False from os.walk.
16. Added author and GitHub to About at the user's request. The real dialog first produced four expected failures (name/link × two palettes), then the corrected DLL passed all 381 checks.
17. Updated both READMEs, CHANGELOG and test documentation. Replaced a stale failed-installation note with fresh isolated installer results bound to DLL/script SHA-256.
18. After native tests passed, refreshed the 31-file byte lock, rechecked PE resources and built final binary ZIPs.
19. Checked the catalog ZIP in a clean portable profile. The official validator checked local bytes, schema, version and uniqueness. Public download and the full Plugins Admin lifecycle were not claimed as passed.
20. Prepared publication-1.0.0 with clean repository/release-assets copies. Build, dist, .git, caches, metrics and working editor copies are excluded; original work remains separate and nothing was uploaded at this stage.
21. A real Git checkout exposed line-ending conversion in new quality scripts. Fixed LF in launcher/requirements, checked automation bytes and protected raw quality results with -text. Recheck: 124 public files and source ZIPs matched under both core.autocrlf modes; deliberate CRLF drift was rejected.
22. Checked Git worktrees: only the current workspace, nothing to remove. The tool cleans completed mutation workers. After a failed run, the accessible part of its private copy was removed; Windows denied access to a remaining ignored target/.pytest_cache. It is excluded from source archives and does not affect production files. Baseline commit/ZIP were preserved.
23. Saved the changes locally with reasons/results. No remote, push or publication at that stage. Verified the clean kit separately against each file's SHA-256.

## Glossary

- **Fore**: the target Prognoz/Foresight Platform language; the plugin is not a compiler.
- **UDL**: earlier user-language definitions, outside this repository.
- **Base XML**: external-lexer settings read by Notepad++.
- **Palette**: standard Light/Dark colors; differing custom colors are preserved.
- **Automatic detection**: a heuristic for Normal Text in TXT, extensionless files and new tabs.
- **Publication kit**: a separate clean source copy and final ZIPs with SHA-256.

## Initial results

| Scope | Measurement and result |
|---|---|
| C++ src/*.cpp, 135 functions | Lizard 1.24.3: maximum CC 65 → 9 |
| tools/package.py, tools/release_files.py | Crapper: 7 functions, 100% function coverage, maximum CRAP 8 |
| tools/*.py | Dryer threshold 0.8: no duplication candidates |
| The two packaging modules | Mutator: 70 mutations, 69 killed, 1 equivalent, 0 uncovered |
| Behavior | 53/53 core, 104/104 detector, 381/381 integration, 17/17 automatic detection, 40 theme checks, 16 Python tests |

The sole surviving mutation changes the fixed ZIP timestamp's seconds from 0 to 1. DOS timestamps store seconds in two-second units; a separate comparison confirmed identical archive bytes. No test of an internal argument was added merely to kill an equivalent mutant. Mutator returns exit 3; this is **not** a claim of 100% killed mutations. Raw results are retained.

Crapper/Dryer/Mutator do not support C++ or PowerShell. C++ CC is not reported as CRAP or coverage. CRAP applies only to the two named Python modules, not every integration CLI. This is a scoped audit, not a claim that every repository function meets every metric.

Machine-readable report and source hashes: [qa/ub-mode-validation.json](../qa/ub-mode-validation.json). Raw metrics: [qa/quality-results](../qa/quality-results). Official tools: [Crapper](https://github.com/unclebob/crapper), [Dryer](https://github.com/unclebob/dryer), [Mutator](https://github.com/unclebob/mutator). Commits are pinned in the report and requirements-quality.txt.

## Repeat the Python checks

Users do not need these optional tools. In an isolated virtual environment, from the repository root:

```powershell
python -m pip install -r requirements-quality.txt
python -m coverage run --source=tools -m pytest -q tests/test_release_files.py tests/test_release_archive.py
python -m coverage lcov -o target/coverage/python/lcov.info
python qa/run-quality.py crapper --use-existing-coverage --threshold 9.99 tools/release_files.py tools/package.py
python qa/run-quality.py dryer --threshold 0.8 tools
python qa/run-quality.py mutator --no-coverage --max-workers 2 --test-command 'python -m pytest -q tests/test_release_files.py tests/test_release_archive.py' tools/release_files.py tools/package.py
```

For mutations, prefer an absolute Python path in --test-command. The Windows adapter copies files instead of symlinking them and validates unchanged copies before mutations. Parser cache lives in build; outputs are in ignored .metrics and target. This does not install a DLL into the user's main Notepad++.

## Addendum: MIT License — 2026-10-31

At the author's explicit request, original code, scripts, documentation and examples were licensed under MIT before the first public release. Verified kit hashes first and preserved a ZIP in ignored build.

Updated root LICENSE with the standard MIT text and Artyom Nikiforov (rhyots), both READMEs, CHANGELOG, instructions, readiness report and DLL VERSIONINFO. Third-party files/licenses were not relicensed; THIRD_PARTY_NOTICES.md defines the boundary.

Rebuilt the DLL; all 381 native assertions, 17 detection cases, four theme runs and installer checks passed. Only version resources changed, so unchanged C++/Python metrics were not rerun; qa/ub-mode-validation.json records this. No test merely checking license prose was added: final archives were inspected directly.

Refreshed byte locks, DLL reports, archives, catalog id and SHA-256. Isolated catalog-package and official offline-validator checks passed. Updated publication-1.0.0 with MIT in all three archives, retaining the prior kit as a local backup. Saved locally without publishing at that stage.

## Addendum: first publication and visual documentation — 2026-10-31

The user's new request explicitly authorized the public repository and Release 1.0.0, replacing the earlier no-publication limit. The same principles applied: preserve the working core and verify the finished artifacts.

- Prepared complete EN/RU README and user-guide pairs, benefits, installation, limitations, help and feedback forms.
- Verified the real Fore excerpt and MIT license from ForesightPlatform/fap_mcp_server at a pinned commit; kept credit, license and exact line range. Other supplied private sources were not copied.
- Rendered GIF/PNG from actual ForeLexer output and XML palettes. Clearly labeled them as previews, not editor recordings. Compared original-example styles/folds with the saved native Notepad++ snapshot.
- Window control stopped after a tool safety stop: the browser URL could not be reliably determined. No alternative window-automation route was used in that run.
- Added an illustration generator and a minimal lexer console adapter without changing DLL behavior. Prior C++/Python metrics cover their stated scope, not the new image helper.
- Added Russian third-party notices to packages and set the repository owner to rhyots. Verified archive contents and existing package tests instead of testing a text constant.
- Pinned verified actions/checkout v7 and setup-python v7 commits. CI checks real LF/CRLF checkouts, builds and tests on Windows/Linux.
- Checked links, images, source kit, SHA-256, byte locks and offline catalog validation before publishing. Native evidence remains bound to the same DLL; documentation-only ZIP changes are recorded separately without replacing old run hashes.
- After authorized publication, downloaded release assets again and compared them with local bytes; detailed URLs/hashes are in the local publication kit.

No separate worktrees were created. The previous kit remains local; build/dist, environments and unpublished examples are not uploaded.

## Addendum: x86 and Plugins Admin preparation — 2026-10-31

1. Rechecked official catalog rules and created branch `codex/plugins-admin` from the published release. The user explicitly authorized the catalog pull request.
2. Built x86; binary inspection exposed decorated lexer exports. Added a module-definition file with the three public lexer names, keeping the calling convention. Rebuilt and verified real x86 loading.
3. Wrote an architecture-isolation package test and observed the expected failure before adding architecture-aware packaging. DLL/evidence hashes, machine and version are checked together.
4. Kept x64 and x86 evidence/runtime directories separate. Each architecture passed 381 native assertions, 17 detection cases, 40 theme checks and installer validation. Fixed a UTF-8 test-helper literal after its four expected native assertions exposed encoding drift; lexer behavior did not change.
5. ARM64 cross-compilation succeeded, but no ARM execution host is available. Following the user's request, ARM64 is outside the mandatory test and published release matrix.
6. Translated all public Markdown documentation into complete English/Russian pairs; English is the default and Russian uses `.ru.md`. Adjusted archive links for WinGUp's `doc/` relocation.
7. The test Plugins Admin dialog was opened with the user's help. Accessibility reads and screenshots then failed with tool timeouts. The official WinGUp command-line backend is tested separately; GUI interaction is not inferred from backend success.

Release-specific lifecycle evidence and catalog status are maintained in [PLUGINS-ADMIN-READINESS.md](PLUGINS-ADMIN-READINESS.md). Later measurements must identify their own source hashes; the initial metrics above are historical, not a certification of subsequently changed scripts.

### Release 1.1.0 tooling results

The final package tests passed **22/22**. Crapper measured **11 functions**, **100% function coverage** and **maximum CRAP 7** in `tools/package.py` and `tools/release_files.py`. Dryer found no candidates at 0.8 across `tools/*.py`. A complete Mutator run selected **86 sites: 85 killed, one equivalent survivor, none uncovered**. The survivor again changes ZIP timestamp seconds 0 → 1; an independent byte comparison confirms identical archives. The tool's survivor exit is retained, not presented as zero survivors.

An initial mutation run encountered Windows child-output decoding errors. The launcher now sets `PYTHONUTF8=1`; all sites were rerun with UTF-8 child interpreters and the default ten-times-baseline timeout. The final report is [qa/ub-mode-1.1.0.json](../qa/ub-mode-1.1.0.json), with source hashes and [raw metrics](../qa/quality-results/v1.1.0). These results have the stated Python scope; unchanged lexer algorithms are covered by the separate native/editor tests.

Windows also held a completed worker directory open during cleanup. The adapter validates that cleanup stays inside the private mutation workspace and retains a locked ignored copy instead of discarding test results. No additional Git worktrees were created; retained diagnostic copies are excluded from publication.

The upstream timeout handler assumes POSIX SIGKILL. The Windows adapter supplies the signal token and checks termination of its own process tree. A real sleeping-child test passed in the normal user environment; the restricted automation profile denied the required process control. Run this optional quality audit in a normal PowerShell session.
