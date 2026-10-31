# Automatic Fore detection

[English](AUTODETECTION.md) | [Русский](AUTODETECTION.ru.md)


Included since the first release, 1.0.0. This is a local heuristic; it does not compile code or guarantee identification of arbitrary snippets.

## Eligible documents

Both conditions must hold:

1. The current document language is **Normal Text**.
2. Its name ends in `.txt` (case insensitive), has no extension, or belongs to a new unsaved tab.

Other extensions, including `.log`, `.md`, `.sql` and `.py`, are excluded even if assigned Normal Text manually. Documents already assigned another language are excluded too. `.fore` files continue to select Fore by extension.

## Signals and limits

Common words such as Class, Begin and End are insufficient. Detection uses the lexer's tokens; strings, character literals and comments do not contribute to signatures. It checks for:

- a Sub / Function / Constructor declaration ending with a semicolon;
- a body starting with Begin and a named ending of the same kind with the matching name;
- or a named class with a parent type and a typed field.

Constructs must start at the beginning of a line after permitted modifiers. An incomplete snippet, another language with a similar structure or unusual formatting may remain undetected. Syntactically close Fore.NET and other snippets matching these patterns cannot always be distinguished: detection does not certify platform compatibility.

Opening, activating or editing a document schedules a check after 350 ms. At most the first 128 KiB of the active document is scanned. Further typing postpones the check. Nothing is written to disk, no network request is made, and the text is not modified.

## Manual language choices

The user's language selection takes priority. Explicitly choosing **Language → Normal Text** disables detection for that open document, even if it was already Normal Text. Switching from Fore back to Normal Text is not undone by subsequent pasting. Closing and reopening the document removes this restriction; it does not persist across editor sessions.

## Validation

`test.ps1` runs separate detector tests: positive constructs, Pascal, VB, FreeBASIC, C++, Python, SQL, strings, comments, mismatched ending names, size limits and extension filtering. Structural examples are checked with LF, CRLF and CR. Results: `qa/detection-validation.json`.

`qa/run-autodetect.ps1` starts a separate portable Notepad++ with `qa/AutoProbe.cpp`. It verifies real language changes on opening and pasting, a new tab, preserved manual choices, menu commands, closing/reopening and unchanged document text. Results: `qa/autodetect-validation.json`.

```powershell
pwsh -File qa/run-autodetect.ps1 -Zig 'C:\tools\zig-0.13.0\zig.exe'
```

Manual example: [Fore-autodetect-demo.txt](../examples/Fore-autodetect-demo.txt).
