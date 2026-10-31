#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include "ThemePalette.h"
#include "ForePaletteData.h"

namespace {
NppData npp;
bool ready = false, applying = false;
COLORREF previousBackground = 0xFFFFFFFF;

bool isDark(COLORREF color) {
    // A foreground/background luminance boundary; UI chrome mode alone is insufficient.
    return (299*GetRValue(color) + 587*GetGValue(color) + 114*GetBValue(color)) < 128000;
}

void apply() {
    if (!ready || applying) return;
    applying = true;
    const COLORREF background = static_cast<COLORREF>(SendMessageW(npp._nppHandle, NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR, 0, 0));
    const auto& colors = isDark(background) ? fore::DarkColors : fore::LightColors;
    for (HWND view : {npp._scintillaMainHandle, npp._scintillaSecondHandle}) {
        char name[80] = {};
        SendMessageW(view, SCI_GETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>(name));
        if (std::strcmp(name, "Fore")) continue;
        for (size_t style = 0; style < colors.size(); ++style) {
            const auto foreground = static_cast<COLORREF>(SendMessageW(view, SCI_STYLEGETFORE, style, 0));
            // Only standard role colors are managed. Preserve nonstandard user overrides.
            if (foreground == fore::LightColors[style] || foreground == fore::DarkColors[style])
                SendMessageW(view, SCI_STYLESETFORE, style, colors[style]);
            const auto currentBackground = static_cast<COLORREF>(SendMessageW(view, SCI_STYLEGETBACK, style, 0));
            if (currentBackground == background || currentBackground == previousBackground ||
                currentBackground == 0xFFFFFF || currentBackground == 0x1E1E1E)
                SendMessageW(view, SCI_STYLESETBACK, style, background);
        }
        InvalidateRect(view, nullptr, FALSE);
    }
    previousBackground = background;
    applying = false;
}
VOID CALLBACK timer(HWND, UINT, UINT_PTR id, DWORD) {
    KillTimer(npp._nppHandle, id);
    try { apply(); } catch (...) { applying = false; }
}
void schedule() {
    if (ready && !applying)
        SetTimer(npp._nppHandle, reinterpret_cast<UINT_PTR>(&timer), 50, timer);
}
}

void themePaletteSetInfo(NppData data) { npp = data; }
void themePaletteNotify(SCNotification* notification) noexcept {
    try {
        switch (notification->nmhdr.code) {
        case NPPN_READY:
            ready = true;
            schedule();
            break;
        case NPPN_DARKMODECHANGED:
        case NPPN_WORDSTYLESUPDATED:
        case NPPN_BUFFERACTIVATED:
        case NPPN_LANGCHANGED:
            schedule();
            break;
        case SCN_UPDATEUI:
            if (ready && previousBackground != static_cast<COLORREF>(SendMessageW(npp._nppHandle, NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR, 0, 0))) schedule();
            break;
        case NPPN_SHUTDOWN:
            ready = false;
            KillTimer(npp._nppHandle, reinterpret_cast<UINT_PTR>(&timer));
            break;
        }
    } catch (...) { applying = false; }
}
