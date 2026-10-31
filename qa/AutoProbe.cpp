// Native integration tests for recognition and manual-language priority.
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
constexpr const char* code = "Sub Demo;\nBegin\n x := 1;\nEnd Sub Demo;\n";
struct Case { const char* name; const wchar_t* file; int expected; int action; };
// expected=-1 denotes the external Fore lexer; 0 is Normal Text.
const Case cases[] = {
    {"txt on open", L"auto.txt", -1, 0},
    {"extensionless on open", L"extensionless", -1, 0},
    {"TXT case insensitive", L"upper.TXT", -1, 0},
    {"ordinary text", L"plain.txt", L_TEXT, 0},
    {"VB stays text", L"basic.txt", L_TEXT, 0},
    {"comments ignored", L"comments.txt", L_TEXT, 0},
    {"strings ignored", L"strings.txt", L_TEXT, 0},
    {"other plain extension excluded", L"blocked.log", L_TEXT, 0},
    {"Python language preserved", L"blocked.py", L_PYTHON, 0},
    {"paste into TXT", L"paste.txt", -1, 1},
    {"explicit Normal Text before paste", L"manual.txt", L_TEXT, 2},
    {"Normal Text via window command before paste", L"manual-ui.txt", L_TEXT, 6},
    {"explicit C++ before paste", L"chosen.txt", L_CPP, 3},
    {"new untitled buffer paste", nullptr, -1, 1},
    {"manual return to text", L"revert.txt", L_TEXT, 4},
    {"close and reopen resets suppression", L"revert.txt", -1, 5},
    {"existing Fore extension", L"native.fore", -1, 0},
};
size_t current = 0;
int phase = 0;
bool passed = true;
FILE* output;
HWND sci() {
    int view = 0; SendMessageW(npp._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&view));
    return view ? npp._scintillaSecondHandle : npp._scintillaMainHandle;
}
void paste() {
    SendMessageW(sci(), SCI_REPLACESEL, 0, reinterpret_cast<LPARAM>(code));
    SendMessageW(sci(), SCI_SETSAVEPOINT, 0, 0);
}
VOID CALLBACK timer(HWND, UINT, UINT_PTR id, DWORD) {
    const auto& test = cases[current];
    if (phase == 0) {
        if (test.action == 5) {
            SendMessageW(sci(), SCI_SETSAVEPOINT, 0, 0);
            SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
        }
        if (test.file) {
            const auto path = directory+L"\\"+test.file;
            SendMessageW(npp._nppHandle, NPPM_DOOPEN, 0, reinterpret_cast<LPARAM>(path.c_str()));
        } else SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
        if (test.action == 2) SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_LANG_TEXT);
        if (test.action == 3) SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_LANG_CPP);
        if (test.action == 6) SendMessageW(npp._nppHandle, WM_COMMAND, IDM_LANG_TEXT, 0);
        if ((test.action >= 1 && test.action <= 3) || test.action == 6) paste();
        phase = 1;
        return;
    }
    if (phase == 1 && test.action == 4) {
        char name[80] = {}; SendMessageW(sci(), SCI_GETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>(name));
        if (std::string(name) != "Fore") passed = false;
        SendMessageW(npp._nppHandle, NPPM_MENUCOMMAND, 0, IDM_LANG_TEXT);
        SendMessageW(sci(), SCI_APPENDTEXT, 1, reinterpret_cast<LPARAM>("\n"));
        SendMessageW(sci(), SCI_SETSAVEPOINT, 0, 0);
        phase = 2;
        return;
    }
    int type = -1; SendMessageW(npp._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, reinterpret_cast<LPARAM>(&type));
    char name[80] = {}; SendMessageW(sci(), SCI_GETLEXERLANGUAGE, 0, reinterpret_cast<LPARAM>(name));
    const bool unmodified = SendMessageW(sci(), SCI_GETMODIFY, 0, 0) == 0;
    bool ok = (test.expected == -1 ? std::string(name) == "Fore" : type == test.expected) && unmodified;
    passed = passed && ok;
    std::fprintf(output, "%s{\"case\":\"%s\",\"passed\":%s,\"language\":\"%s\",\"language_type\":%d,\"buffer_not_modified\":%s}",
                 current ? "," : "", test.name, ok ? "true" : "false", name, type, unmodified ? "true" : "false");
    std::fflush(output);
    if (++current == sizeof(cases)/sizeof(cases[0])) {
        std::fprintf(output, "],\"passed\":%s}\n", passed ? "true" : "false");
        std::fclose(output); KillTimer(npp._nppHandle, id);
        PostMessageW(npp._nppHandle, WM_CLOSE, 0, 0);
    } else phase = 0;
}
}
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
extern "C" __declspec(dllexport) void setInfo(NppData data) { npp = data; }
extern "C" __declspec(dllexport) const wchar_t* getName() { return L"Fore auto-detection test probe"; }
extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) { *count = 1; wcscpy(commands[0]._itemName, L"Auto-detection probe"); return commands; }
extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM) { return TRUE; }
extern "C" __declspec(dllexport) void beNotified(SCNotification* notification) {
    if (notification->nmhdr.code != NPPN_READY) return;
    wchar_t path[32768] = {}; GetModuleFileNameW(module, path, 32768); directory = path;
    for (int i = 0; i < 3; ++i) directory.resize(directory.find_last_of(L"\\/"));
    output = _wfopen((directory+L"\\auto-results.json").c_str(), L"wb");
    if (!output) return;
    std::fprintf(output, "{\"cases\":[");
    SetTimer(npp._nppHandle, reinterpret_cast<UINT_PTR>(&timer), 600, timer);
}
BOOL WINAPI DllMain(HMODULE handle, DWORD reason, LPVOID) { if (reason == DLL_PROCESS_ATTACH) module = handle; return TRUE; }
