#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <algorithm>
#include <unordered_set>
#include "AutoDetect.h"
#include "ForeDetect.h"
#include "menuCmdID.h"

namespace {
NppData npp;
bool ready = false, selecting = false;
UINT_PTR pending = 0;
std::unordered_set<UINT_PTR> suppressed, assigned;
constexpr UINT_PTR subclassId = 0x464F5245;

UINT_PTR buffer() { return SendMessageW(npp._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0); }
HWND scintilla() {
    int view = 0;
    SendMessageW(npp._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&view));
    return view ? npp._scintillaSecondHandle : npp._scintillaMainHandle;
}
int foreCommand(HMENU menu) {
    for (int i = 0; i < GetMenuItemCount(menu); ++i) {
        int id = GetMenuItemID(menu, i);
        wchar_t text[128] = {};
        GetMenuStringW(menu, i, text, 128, MF_BYPOSITION);
        std::wstring label;
        for (wchar_t c : text) { if (!c || c == L'\t') break; if (c != L'&') label += c; }
        if (id >= IDM_LANG_EXTERNAL && id <= IDM_LANG_EXTERNAL_LIMIT && label == L"Fore") return id;
        if (HMENU child = GetSubMenu(menu, i)) { if (int found = foreCommand(child)) return found; }
    }
    return 0;
}
VOID CALLBACK detect(HWND, UINT, UINT_PTR timer, DWORD) {
    KillTimer(npp._nppHandle, timer);
    try {
        const UINT_PTR current = buffer();
        if (!ready || current != pending || suppressed.count(current)) return;
        const bool normal = SendMessageW(npp._nppHandle, NPPM_GETBUFFERLANGTYPE, current, 0) == L_TEXT;
        if (!normal) return;
        auto length = SendMessageW(npp._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, current, 0);
        if (length < 0 || length > 32767) return;
        std::wstring path(static_cast<size_t>(length)+1, L'\0');
        SendMessageW(npp._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, current, reinterpret_cast<LPARAM>(path.data()));
        path.resize(wcslen(path.c_str()));
        if (!fore::autoDetectEligible(path, normal)) return;
        HWND sci = scintilla();
        auto size = std::min<LRESULT>(SendMessageW(sci, SCI_GETLENGTH, 0, 0), fore::DetectionLimit);
        if (size <= 0) return;
        std::string text(static_cast<size_t>(size)+1, '\0');
        SendMessageW(sci, SCI_GETTEXT, size+1, reinterpret_cast<LPARAM>(text.data()));
        text.resize(static_cast<size_t>(size));
        if (!fore::looksLikeFore(text)) return;
        if (int command = foreCommand(GetMenu(npp._nppHandle))) {
            selecting = true;
            SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, command);
            selecting = false;
            assigned.insert(current);
        }
    } catch (...) { selecting = false; }
}
void schedule() {
    if (!ready || selecting) return;
    pending = buffer();
    if (suppressed.count(pending)) return;
    SetTimer(npp._nppHandle, reinterpret_cast<UINT_PTR>(&detect), 350, detect);
}
LRESULT CALLBACK observe(HWND window, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
    try {
        // Capture an explicit Normal Text choice even when it does not change the language.
        const bool explicitText = (message == WM_COMMAND && LOWORD(wParam) == IDM_LANG_TEXT) ||
                                  (message == NPPM_MENUCOMMAND && lParam == IDM_LANG_TEXT);
        if (explicitText && !selecting) suppressed.insert(buffer());
    } catch (...) {}
    return DefSubclassProc(window, message, wParam, lParam);
}
}

void autoDetectSetInfo(NppData data) { npp = data; }
void autoDetectNotify(SCNotification* notification) noexcept {
    try {
        switch (notification->nmhdr.code) {
        case NPPN_READY:
            ready = true;
            SetWindowSubclass(npp._nppHandle, observe, subclassId, 0);
            schedule();
            break;
        case NPPN_SHUTDOWN:
            ready = false;
            KillTimer(npp._nppHandle, reinterpret_cast<UINT_PTR>(&detect));
            RemoveWindowSubclass(npp._nppHandle, observe, subclassId);
            break;
        case NPPN_BUFFERACTIVATED:
        case NPPN_FILEOPENED:
            schedule();
            break;
        case NPPN_LANGCHANGED:
            if (!selecting && ready) {
                const UINT_PTR id = notification->nmhdr.idFrom;
                if (assigned.count(id) || SendMessageW(npp._nppHandle, NPPM_GETBUFFERLANGTYPE, id, 0) != L_TEXT)
                    suppressed.insert(id);
            }
            break;
        case NPPN_FILEBEFORECLOSE:
            suppressed.erase(notification->nmhdr.idFrom);
            assigned.erase(notification->nmhdr.idFrom);
            break;
        case SCN_MODIFIED:
            if ((notification->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) && notification->nmhdr.hwndFrom == scintilla()) schedule();
            break;
        }
    } catch (...) {}
}
