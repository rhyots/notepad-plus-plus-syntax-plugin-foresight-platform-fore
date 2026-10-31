// Integration test plugin. Loaded only by qa/run.ps1 into its isolated Notepad++.
#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#include <string>
#include "PluginInterface.h"
#include "menuCmdID.h"

namespace {
NppData npp;
HMODULE module;
FuncItem commands[1];
std::wstring directory;
std::string menuPath;
bool stylesFound = false;
bool styleDialogFound = false;
bool aboutAuthorFound = false;
bool aboutLinkFound = false;
UINT aboutCommand = 0;
bool started = false;
int phase = 0;
constexpr UINT_PTR timerId = 0xF04E;

bool findFore(HMENU menu, const std::string& prefix) {
    for (int i = 0, count = GetMenuItemCount(menu); i < count; ++i) {
        wchar_t label[256] = {};
        GetMenuStringW(menu, i, label, 256, MF_BYPOSITION);
        std::string text;
        for (wchar_t c : label) { if (!c || c == '\t') break; if (c != '&' && c < 128) text += static_cast<char>(c); }
        std::string path = prefix.empty() ? text : prefix + "/" + text;
        if (text == "Fore") menuPath = path;
        if (text == "About Notepad++ Syntax Plugin for Foresight Platform Fore") aboutCommand = GetMenuItemID(menu, i);
        HMENU child = GetSubMenu(menu, i);
        if (child) findFore(child, path);
    }
    return false;
}
BOOL CALLBACK inspectAboutText(HWND control, LPARAM) {
    wchar_t text[2048] = {};
    GetWindowTextW(control, text, 2048);
    if (wcsstr(text, L"Artyom Nikiforov (rhyots)")) aboutAuthorFound = true;
    if (wcsstr(text, L"https://github.com/rhyots")) aboutLinkFound = true;
    return TRUE;
}
BOOL CALLBACK inspectAbout(HWND window, LPARAM) {
    wchar_t title[256] = {};
    GetWindowTextW(window, title, 256);
    if (!wcscmp(title, L"Notepad++ Syntax Plugin for Foresight Platform Fore")) {
        EnumChildWindows(window, inspectAboutText, 0);
        PostMessageW(window, WM_COMMAND, IDOK, 0);
    }
    return TRUE;
}
BOOL CALLBACK inspectControl(HWND control, LPARAM) {
    wchar_t cls[80] = {};
    GetClassNameW(control, cls, 80);
    bool combo = !wcscmp(cls, L"ComboBox"), list = !wcscmp(cls, L"ListBox");
    if (!combo && !list) return TRUE;
    int count = static_cast<int>(SendMessageW(control, combo ? CB_GETCOUNT : LB_GETCOUNT, 0, 0));
    for (int i = 0; i < count; ++i) {
        int len = static_cast<int>(SendMessageW(control, combo ? CB_GETLBTEXTLEN : LB_GETTEXTLEN, i, 0));
        if (len < 0 || len > 4096) continue;
        std::wstring text(static_cast<size_t>(len + 1), L'\0');
        SendMessageW(control, combo ? CB_GETLBTEXT : LB_GETTEXT, i, reinterpret_cast<LPARAM>(text.data()));
        if (!wcscmp(text.c_str(), L"Fore")) stylesFound = true;
    }
    return TRUE;
}
BOOL CALLBACK inspectWindow(HWND window, LPARAM) {
    wchar_t title[256] = {};
    GetWindowTextW(window, title, 256);
    if (wcsstr(title, L"Style Configurator")) {
        styleDialogFound = true;
        EnumChildWindows(window, inspectControl, 0);
        // Cancel: no user settings are saved by the test.
        PostMessageW(window, WM_COMMAND, IDCANCEL, 0);
    }
    return TRUE;
}
HWND activeScintilla() {
    int view = 0; SendMessageW(npp._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&view));
    return view ? npp._scintillaSecondHandle : npp._scintillaMainHandle;
}
void snapshot(HWND sci, const wchar_t* output) {
    SendMessageW(sci, SCI_COLOURISE, 0, -1);
    auto length = SendMessageW(sci, SCI_GETLENGTH, 0, 0);
    auto lines = SendMessageW(sci, SCI_GETLINECOUNT, 0, 0);
    char lexerName[256] = {};
    SendMessageW(sci, SCI_GETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>(lexerName));
    int type = 0;
    SendMessageW(npp._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&type));
    FILE* file = _wfopen((directory + L"\\" + output).c_str(), L"wb");
    if (!file) return;
    std::fprintf(file, "{\"lexer\":\"%s\",\"language_type\":%d,\"length\":%lld,\"styles\":[", lexerName, type, static_cast<long long>(length));
    for (LRESULT i = 0; i < length; ++i)
        std::fprintf(file, "%s%lld", i ? "," : "", static_cast<long long>(SendMessageW(sci, SCI_GETSTYLEAT, i, 0)));
    std::fprintf(file, "],\"folds\":[");
    for (LRESULT i = 0; i < lines; ++i)
        std::fprintf(file, "%s%lld", i ? "," : "", static_cast<long long>(SendMessageW(sci, SCI_GETFOLDLEVEL, i, 0)));
    std::fprintf(file, "],\"colours\":[");
    for (int i = 0; i < 11; ++i)
        std::fprintf(file, "%s{\"fg\":%lld,\"bg\":%lld}", i ? "," : "",
            static_cast<long long>(SendMessageW(sci, SCI_STYLEGETFORE, i, 0)),
            static_cast<long long>(SendMessageW(sci, SCI_STYLEGETBACK, i, 0)));
    std::fprintf(file, "]}\n"); std::fclose(file);
}
void dump(const wchar_t* name) {
    std::wstring path = directory + L"\\" + name + L".fore";
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) return;
    SendMessageW(npp._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(path.c_str()));
    snapshot(activeScintilla(), (std::wstring(name) + L".json").c_str());
}
void tests() {
    findFore(GetMenu(npp._nppHandle), "");
    for (const wchar_t* file : {L"demo-lf", L"edges-lf", L"demo-crlf", L"edges-crlf", L"minesweeper", L"declension", L"db-handler"}) dump(file);
    HWND sci = activeScintilla();
    const char* text = "Sub Main;\nBegin\n s := \"x\";\nEnd Sub Main;\n";
    SendMessageW(sci, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(text));
    snapshot(sci, L"edit-before.json");
    // A real incremental edit must invalidate previously cached lexical state.
    SendMessageW(sci, SCI_INSERTTEXT, 0, reinterpret_cast<LPARAM>("{comment\n}\n"));
    snapshot(sci, L"edit-after.json");
    SendMessageW(sci, SCI_SETSAVEPOINT, 0, 0);
    // Test Scintilla's normal style customization without changing the XML.
    SendMessageW(sci, SCI_STYLESETFORE, 1, 0x332211);
    bool customStyle = SendMessageW(sci, SCI_STYLEGETFORE, 1, 0) == 0x332211;
    FILE* file = _wfopen((directory + L"\\ui.json").c_str(), L"wb");
    if (file) {
        std::fprintf(file, "{\"menu_path\":\"%s\",\"style_dialog_opened\":%s,\"style_dialog_contains_fore\":%s,\"custom_style_roundtrip\":%s,\"about_author\":%s,\"about_github\":%s}\n",
            menuPath.c_str(), styleDialogFound ? "true" : "false", stylesFound ? "true" : "false", customStyle ? "true" : "false", aboutAuthorFound ? "true" : "false", aboutLinkFound ? "true" : "false");
        std::fclose(file);
    }
    PostMessageW(npp._nppHandle, WM_CLOSE, 0, 0);
}
VOID CALLBACK timer(HWND, UINT, UINT_PTR, DWORD) {
    if (phase == 0) {
        findFore(GetMenu(npp._nppHandle), "");
        phase = 1;
        SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_LANGSTYLE_CONFIG_DLG);
    } else if (phase == 1) {
        phase = 2;
        EnumThreadWindows(GetCurrentThreadId(), inspectWindow, 0);
    } else if (phase == 2) {
        phase = 3;
        if (aboutCommand) SendMessageW(npp._nppHandle, WM_COMMAND, aboutCommand, 0);
    } else if (phase == 3) {
        phase = 4;
        EnumThreadWindows(GetCurrentThreadId(), inspectAbout, 0);
    } else if (phase == 4) {
        KillTimer(npp._nppHandle, timerId);
        tests();
    }
}
}
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
extern "C" __declspec(dllexport) void setInfo(NppData data) { npp = data; }
extern "C" __declspec(dllexport) const wchar_t* getName() { return L"Foresight Platform Fore integration probe"; }
extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) { *count = 1; wcscpy(commands[0]._itemName, L"Integration probe"); return commands; }
extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM) { return TRUE; }
extern "C" __declspec(dllexport) void beNotified(SCNotification* notification) {
    if (notification->nmhdr.code != NPPN_READY || started) return;
    started = true;
    wchar_t path[32768] = {};
    GetModuleFileNameW(module, path, 32768); directory = path;
    for (int i = 0; i < 3; ++i) directory.resize(directory.find_last_of(L"\\/"));
    SetTimer(npp._nppHandle, timerId, 100, timer);
}
BOOL WINAPI DllMain(HMODULE handle, DWORD reason, LPVOID) { if (reason == DLL_PROCESS_ATTACH) module = handle; return TRUE; }
