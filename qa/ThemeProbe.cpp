// Integration test only: select real themes in the real Style Configurator.
// Control IDs: Notepad++ v8.9.8.1 WordStyleDlgRes.h, pinned SDK commit dd40fe4.
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "PluginInterface.h"
#include "menuCmdID.h"
#include "ForePaletteData.h"

namespace {
NppData npp;
HMODULE module;
FuncItem commands[1];
std::wstring directory;
HWND dialog;
bool custom = false, passed = true;
int phase = 0, checks = 0;
FILE* output;
std::vector<long long> cppColors;
constexpr int themeCombo = 2307, saveClose = 2301;

HWND currentView() {
    int view = 0; SendMessageW(npp._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&view));
    return view ? npp._scintillaSecondHandle : npp._scintillaMainHandle;
}
bool dark() {
    COLORREF c = static_cast<COLORREF>(SendMessageW(npp._nppHandle, NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR, 0, 0));
    return 299*GetRValue(c)+587*GetGValue(c)+114*GetBValue(c) < 128000;
}
BOOL CALLBACK findDialog(HWND window, LPARAM) {
    if (GetDlgItem(window, themeCombo)) dialog = window;
    return TRUE;
}
void theme(bool useDark) {
    HWND combo = GetDlgItem(dialog, themeCombo);
    int index = -1;
    for (int i = 0, n = static_cast<int>(SendMessageW(combo, CB_GETCOUNT, 0, 0)); i < n; ++i) {
        wchar_t text[300] = {}; SendMessageW(combo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(text));
        if ((useDark && !wcscmp(text, L"DarkModeDefault")) || (!useDark && !wcsncmp(text, L"Default", 7))) { index = i; break; }
    }
    if (index < 0) { passed = false; return; }
    SendMessageW(combo, CB_SETCURSEL, index, 0);
    SendMessageW(dialog, WM_COMMAND, MAKEWPARAM(themeCombo, CBN_SELCHANGE), reinterpret_cast<LPARAM>(combo));
}
void check(const char* label, bool expectedDark, int expectedViews) {
    const auto& expected = expectedDark ? fore::DarkColors : fore::LightColors;
    const auto background = SendMessageW(npp._nppHandle, NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR, 0, 0);
    bool ok = dark() == expectedDark;
    int views = 0;
    for (HWND view : {npp._scintillaMainHandle, npp._scintillaSecondHandle}) {
        char name[80] = {}; SendMessageW(view, SCI_GETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>(name));
        if (std::strcmp(name, "Fore")) continue;
        ++views;
        for (int i = 0; i < 11; ++i) {
            const auto color = custom && i == 1 ? 0x996633UL : expected[i];
            ok = ok && SendMessageW(view, SCI_STYLEGETFORE, i, 0) == color;
            ok = ok && SendMessageW(view, SCI_STYLEGETBACK, i, 0) == background;
        }
        ok = ok && SendMessageW(view, SCI_STYLEGETSIZE, 1, 0) == 13;
        ok = ok && SendMessageW(view, SCI_STYLEGETITALIC, 1, 0) != 0;
        ok = ok && SendMessageW(view, SCI_GETMODIFY, 0, 0) == 0;
    }
    ok = ok && views == expectedViews;
    passed = passed && ok;
    std::fprintf(output, "%s{\"case\":\"%s\",\"passed\":%s,\"dark\":%s,\"fore_views\":%d}",
        checks++ ? "," : "", label, ok ? "true" : "false", dark() ? "true" : "false", views);
    std::fflush(output);
}
void open(const wchar_t* filename) {
    const auto path = directory+L"\\"+filename;
    SendMessageW(npp._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(path.c_str()));
}
VOID CALLBACK timer(HWND, UINT, UINT_PTR id, DWORD) {
    switch (phase++) {
    case 0: open(L"sample.fore"); break;
    case 1:
        std::fprintf(output, "\"startup_dark\":%s,\"custom\":%s,\"cases\":[", dark() ? "true" : "false", custom ? "true" : "false");
        check("startup palette and fonts", dark(), 1);
        SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_LANGSTYLE_CONFIG_DLG);
        break;
    case 2:
        EnumThreadWindows(GetCurrentThreadId(), findDialog, 0);
        theme(true);
        break;
    case 3: check("real light-to-dark theme selection", true, 1); theme(false); break;
    case 4: check("real dark-to-light theme selection", false, 1);
        SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_VIEW_CLONE_TO_ANOTHER_VIEW); break;
    case 5: check("cloned Fore views", false, 2); theme(true); break;
    case 6: check("both Fore views after dark switch", true, 2); theme(false); break;
    case 7: check("both Fore views after light switch", false, 2); theme(true); break;
    case 8: check("repeat switch is stable", true, 2); open(L"sample.cpp"); break;
    case 9:
        theme(false);
        for (int i = 0; i < 11; ++i) cppColors.push_back(SendMessageW(currentView(), SCI_STYLEGETFORE, i, 0));
        break;
    case 10: {
        bool ok = true;
        for (int i = 0; i < 11; ++i) ok = ok && cppColors[i] == SendMessageW(currentView(), SCI_STYLEGETFORE, i, 0);
        passed = passed && ok;
        std::fprintf(output, ",{\"case\":\"other language colors untouched\",\"passed\":%s}", ok ? "true" : "false"); ++checks;
        check("Fore in other view follows theme", false, 1);
        theme(true);
        break;
    }
    case 11:
        SendMessageW(dialog, WM_COMMAND, saveClose, 0);
        break;
    case 12:
        check("saved dark theme with other active language", true, 1);
        std::fprintf(output, "],\"passed\":%s}\n", passed ? "true" : "false");
        std::fclose(output);
        KillTimer(npp._nppHandle, id);
        PostMessageW(npp._nppHandle, WM_CLOSE, 0, 0);
        break;
    }
}
}
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
extern "C" __declspec(dllexport) void setInfo(NppData data) { npp = data; }
extern "C" __declspec(dllexport) const wchar_t* getName() { return L"Fore automatic theme integration probe"; }
extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) { *count=1; wcscpy(commands[0]._itemName,L"Theme probe"); return commands; }
extern "C" __declspec(dllexport) LRESULT messageProc(UINT,WPARAM,LPARAM) { return TRUE; }
extern "C" __declspec(dllexport) void beNotified(SCNotification* notification) {
    if (notification->nmhdr.code != NPPN_READY) return;
    wchar_t path[32768] = {}; GetModuleFileNameW(module,path,32768); directory=path;
    for(int i=0;i<3;++i) directory.resize(directory.find_last_of(L"\\/"));
    custom = GetFileAttributesW((directory+L"\\custom.flag").c_str()) != INVALID_FILE_ATTRIBUTES;
    output=_wfopen((directory+L"\\theme-results.json").c_str(),L"wb"); if(!output)return;
    std::fprintf(output,"{");
    SetTimer(npp._nppHandle,reinterpret_cast<UINT_PTR>(&timer),250,timer);
}
BOOL WINAPI DllMain(HMODULE handle,DWORD reason,LPVOID) { if(reason==DLL_PROCESS_ATTACH)module=handle;return TRUE; }
