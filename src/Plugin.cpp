#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include <new>
#include <filesystem>
#include "PluginInterface.h"
#include "ILexer.h"
#include "SciLexer.h"
#include "ForeLexer.h"
#include "AutoDetect.h"
#include "ThemePalette.h"

namespace {
NppData npp;
HMODULE module;
FuncItem commands[1];

std::filesystem::path missingConfiguration() {
    wchar_t executable[32768] = {};
    if (!GetModuleFileNameW(nullptr, executable, 32768)) return {};
    const auto global = std::filesystem::path(executable).parent_path()/L"plugins"/L"Config"/L"ForeLexer.xml";
    if (std::filesystem::exists(global)) return {};
    wchar_t directory[32768] = {};
    if (!SendMessageW(npp._nppHandle, NPPM_GETPLUGINSCONFIGDIR, 32768, reinterpret_cast<LPARAM>(directory))) return {};
    if (!directory[0]) return {};
    const auto config = std::filesystem::path(directory)/L"ForeLexer.xml";
    return std::filesystem::exists(config) ? std::filesystem::path{} : config;
}
void createConfiguration(const std::filesystem::path& config) {
    HRSRC resource = FindResourceW(module, MAKEINTRESOURCEW(101), RT_RCDATA);
    if (!resource) return;
    const DWORD size = SizeofResource(module, resource);
    const void* bytes = LockResource(LoadResource(module, resource));
    if (!bytes || !size) return;
    std::filesystem::create_directories(config.parent_path());
    HANDLE file = CreateFileW(config.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    const bool ok = WriteFile(file, bytes, size, &written, nullptr) && written == size;
    CloseHandle(file);
    if (!ok) DeleteFileW(config.c_str());
}
void ensureConfiguration() noexcept {
    try {
        const auto config = missingConfiguration();
        if (!config.empty()) createConfiguration(config);
    } catch (...) {
        // The editor reports its normal missing-XML error if creation fails.
    }
}

void about() {
    MessageBoxW(npp._nppHandle,
        L"Notepad++ Syntax Plugin for Foresight Platform Fore 1.1.0\nSyntax highlighting and folding.\n"
        L"Rules: Prognoz Platform Fore 8.1 documentation.\n"
        L"Language > F > Fore\nSettings > Style Configurator > Fore\n\n"
        L"Artyom Nikiforov (rhyots)\nhttps://github.com/rhyots",
        L"Notepad++ Syntax Plugin for Foresight Platform Fore", MB_OK | MB_ICONINFORMATION);
}

class ForeLexer final : public Scintilla::ILexer5 {
    bool foldEnabled = true;
    bool foldComments = true;
    std::string cachedText;
    fore::Analysis cached;
    bool cacheValid = false;
    const fore::Analysis& analyze(Scintilla::IDocument* doc) {
        std::string text(static_cast<size_t>(doc->Length()), '\0');
        if (!text.empty()) doc->GetCharRange(text.data(), 0, static_cast<Sci_Position>(text.size()));
        if (!cacheValid || text != cachedText) {
            cached = fore::analyze(text, foldComments); cachedText = std::move(text); cacheValid = true;
        }
        return cached;
    }
public:
    int SCI_METHOD Version() const override { return Scintilla::lvRelease5; }
    void SCI_METHOD Release() override { delete this; }
    const char* SCI_METHOD PropertyNames() override { return "fold\nfold.comment"; }
    int SCI_METHOD PropertyType(const char*) override { return 0; }
    const char* SCI_METHOD DescribeProperty(const char* name) override {
        return !std::strcmp(name, "fold.comment") ? "Fold multiline brace comments" : "Enable code folding";
    }
    Sci_Position SCI_METHOD PropertySet(const char* key, const char* value) override {
        bool enabled = std::strcmp(value, "0") != 0;
        if (!std::strcmp(key, "fold")) foldEnabled = enabled;
        else if (!std::strcmp(key, "fold.comment")) { foldComments = enabled; cacheValid = false; }
        else return -1;
        return 0;
    }
    const char* SCI_METHOD DescribeWordListSets() override { return ""; }
    Sci_Position SCI_METHOD WordListSet(int, const char*) override { return -1; }
    void SCI_METHOD Lex(Sci_PositionU, Sci_Position, int, Scintilla::IDocument* doc) override {
        try {
            const auto& a = analyze(doc);
            doc->StartStyling(0);
            if (!a.styles.empty()) doc->SetStyles(static_cast<Sci_Position>(a.styles.size()), a.styles.data());
        } catch (const std::bad_alloc&) { doc->SetErrorStatus(SC_STATUS_BADALLOC); }
        catch (...) { doc->SetErrorStatus(SC_STATUS_FAILURE); }
    }
    void SCI_METHOD Fold(Sci_PositionU, Sci_Position, int, Scintilla::IDocument* doc) override {
        try {
            const auto& a = analyze(doc);
            for (size_t i = 0; i < a.folds.size(); ++i)
                doc->SetLevel(static_cast<Sci_Position>(i), foldEnabled ? a.folds[i] : (SC_FOLDLEVELBASE | (SC_FOLDLEVELBASE << 16)));
        } catch (const std::bad_alloc&) { doc->SetErrorStatus(SC_STATUS_BADALLOC); }
        catch (...) { doc->SetErrorStatus(SC_STATUS_FAILURE); }
    }
    void* SCI_METHOD PrivateCall(int, void*) override { return nullptr; }
    int SCI_METHOD LineEndTypesSupported() override { return 0; }
    int SCI_METHOD AllocateSubStyles(int, int) override { return -1; }
    int SCI_METHOD SubStylesStart(int) override { return -1; }
    int SCI_METHOD SubStylesLength(int) override { return 0; }
    int SCI_METHOD StyleFromSubStyle(int style) override { return style; }
    int SCI_METHOD PrimaryStyleFromStyle(int style) override { return style; }
    void SCI_METHOD FreeSubStyles() override {}
    void SCI_METHOD SetIdentifiers(int, const char*) override {}
    int SCI_METHOD DistanceToSecondaryStyles() override { return 0; }
    const char* SCI_METHOD GetSubStyleBases() override { return ""; }
    int SCI_METHOD NamedStyles() override { return fore::StyleCount; }
    const char* SCI_METHOD NameOfStyle(int style) override { return fore::styleName(style); }
    const char* SCI_METHOD TagsOfStyle(int) override { return ""; }
    const char* SCI_METHOD DescriptionOfStyle(int style) override { return fore::styleName(style); }
    const char* SCI_METHOD GetName() override { return "Fore"; }
    int SCI_METHOD GetIdentifier() override { return SCLEX_AUTOMATIC; }
    const char* SCI_METHOD PropertyGet(const char* key) override {
        if (!std::strcmp(key, "fold")) return foldEnabled ? "1" : "0";
        if (!std::strcmp(key, "fold.comment")) return foldComments ? "1" : "0";
        return "";
    }
};
}

extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
extern "C" __declspec(dllexport) void setInfo(NppData data) { npp = data; ensureConfiguration(); autoDetectSetInfo(data); themePaletteSetInfo(data); }
extern "C" __declspec(dllexport) const wchar_t* getName() { return L"Notepad++ Syntax Plugin for Foresight Platform Fore"; }
extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) {
    *count = 1; wcscpy(commands[0]._itemName, L"About Notepad++ Syntax Plugin for Foresight Platform Fore"); commands[0]._pFunc = about; return commands;
}
extern "C" __declspec(dllexport) void beNotified(SCNotification* notification) { autoDetectNotify(notification); themePaletteNotify(notification); }
extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM) { return TRUE; }
extern "C" __declspec(dllexport) int __stdcall GetLexerCount() { return 1; }
extern "C" __declspec(dllexport) void __stdcall GetLexerName(unsigned int index, char* name, int length) {
    if (!name || length <= 0) return;
    const char* source = index == 0 ? "Fore" : "";
    std::strncpy(name, source, static_cast<size_t>(length - 1)); name[length - 1] = '\0';
}
extern "C" __declspec(dllexport) Scintilla::ILexer5* __stdcall CreateLexer(const char* name) {
    if (!name || std::strcmp(name, "Fore")) return nullptr;
    try { return new ForeLexer(); } catch (...) { return nullptr; }
}
BOOL WINAPI DllMain(HMODULE handle, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) module = handle;
    return TRUE;
}
