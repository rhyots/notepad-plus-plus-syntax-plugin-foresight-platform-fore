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

bool managedForeground(COLORREF color, size_t style) {
    return color == fore::LightColors[style] || color == fore::DarkColors[style];
}
bool managedBackground(COLORREF color, COLORREF background) {
    return color == background || color == previousBackground || color == 0xFFFFFF || color == 0x1E1E1E;
}
void applyStyle(HWND view, size_t style, COLORREF color, COLORREF background) {
    const auto foreground = static_cast<COLORREF>(SendMessageW(view, SCI_STYLEGETFORE, style, 0));
    if (managedForeground(foreground, style)) SendMessageW(view, SCI_STYLESETFORE, style, color);
    const auto currentBackground = static_cast<COLORREF>(SendMessageW(view, SCI_STYLEGETBACK, style, 0));
    if (managedBackground(currentBackground, background)) SendMessageW(view, SCI_STYLESETBACK, style, background);
}
void applyView(HWND view, const std::array<unsigned long, 11>& colors, COLORREF background) {
        char name[80] = {};
        SendMessageW(view, SCI_GETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>(name));
        if (std::strcmp(name, "Fore")) return;
        for (size_t style = 0; style < colors.size(); ++style) {
            applyStyle(view, style, colors[style], background);
        }
        InvalidateRect(view, nullptr, FALSE);
}
void apply() {
    if (!ready || applying) return;
    applying = true;
    const COLORREF background = static_cast<COLORREF>(SendMessageW(npp._nppHandle, NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR, 0, 0));
    const auto& colors = isDark(background) ? fore::DarkColors : fore::LightColors;
    for (HWND view : {npp._scintillaMainHandle, npp._scintillaSecondHandle}) applyView(view, colors, background);
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
void interfaceUpdated() {
    if (!ready) return;
    const auto background = static_cast<COLORREF>(SendMessageW(npp._nppHandle, NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR, 0, 0));
    if (previousBackground != background) schedule();
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
            interfaceUpdated();
            break;
        case NPPN_SHUTDOWN:
            ready = false;
            KillTimer(npp._nppHandle, reinterpret_cast<UINT_PTR>(&timer));
            break;
        }
    } catch (...) { applying = false; }
}
