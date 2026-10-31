#include "ForeDetect.h"
#include <cstdio>
#include <string>
#include <vector>

int main() {
    struct Case { const char* name; std::string text; bool expected; };
    std::vector<Case> cases {
        {"named procedure", "Sub Demo;\nBegin\n Return;\nEnd Sub Demo;\n", true},
        {"typed function", "Public Function Twice(value: Integer): Integer;\nBegin\n Return value * 2;\nEnd Function Twice;\n", true},
        {"constructor", "Public Constructor Create;\nBegin\nEnd Constructor Create;\n", true},
        {"locals before body", "Sub Demo;\nVar\n x: Integer;\nBegin\n x := 1;\nEnd Sub Demo;\n", true},
        {"case insensitive", "sUb dEmO;\nbEgIn\nrEtUrN;\neNd SuB DeMo;\n", true},
        {"inherited class with typed field", "Class Box: Object\n x: Integer;\nEnd Class Box;\n", true},
        {"interface followed by method", "Interface IBox\nSub Demo;\nEnd Interface IBox;\nSub Demo;\nBegin\nEnd Sub Demo;\n", true},
        {"UTF8 BOM", "\xef\xbb\xbfSub Demo;\nBegin\nEnd Sub Demo;\n", true},
        {"ordinary text", "This text mentions class, begin, end, and function.\n", false},
        {"short fragment", "Begin\nx := 1;\nEnd;\n", false},
        {"Pascal", "procedure Demo;\nbegin\nx := 1;\nend;\n", false},
        {"VB", "Sub Demo()\nDim x As Integer\nx = 1\nEnd Sub\n", false},
        {"FreeBASIC", "Function Demo() As Integer\nReturn 1\nEnd Function\n", false},
        {"C++", "class Box { public: void Demo() { return; } };\n", false},
        {"Python", "class Box:\n def demo(self):\n  return 1\n", false},
        {"SQL", "BEGIN\nSELECT 'Class Box';\nEND;\n", false},
        {"mismatched method name", "Sub Demo;\nBegin\nEnd Sub Other;\n", false},
        {"missing Begin", "Sub Demo;\nReturn;\nEnd Sub Demo;\n", false},
        {"missing heading semicolon", "Sub Demo\nBegin\nEnd Sub Demo;\n", false},
        {"unnamed closer", "Sub Demo;\nBegin\nEnd Sub;\n", false},
        {"prose prefix", "Example Sub Demo;\nBegin\nEnd Sub Demo;\n", false},
        {"brace comment", "{\nSub Demo;\nBegin\nEnd Sub Demo;\n}\n", false},
        {"line comments", "// Sub Demo;\n// Begin\n// End Sub Demo;\n", false},
        {"strings", "\"Sub Demo;\"\n\"Begin\"\n\"End Sub Demo;\"\n", false},
        {"interface only", "Interface IBox\nSub Demo;\nEnd Interface IBox;\n", false},
        {"unclosed method", "Sub Demo;\nBegin\nx := 1;\n", false},
        {"unclosed comment", "{\nSub Demo;\nBegin\nEnd Sub Demo;\n", false},
        {"empty class", "Class Box: Object\nEnd Class Box;\n", false},
        {"binary NUL", std::string(1, '\0')+"Sub Demo;\nBegin\nEnd Sub Demo;\n", false},
        {"bounded scan", std::string(fore::DetectionLimit, ' ')+"Sub Demo;\nBegin\nEnd Sub Demo;\n", false}
    };
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const char* name) { ++checks; if (!ok) { ++failures; std::printf("FAIL: %s\n", name); } };
    for (const auto& test : cases) {
        for (const std::string ending : {"\n", "\r\n", "\r"}) {
            std::string text;
            for (char c : test.text) text += c == '\n' ? ending : std::string(1, c);
            check(fore::looksLikeFore(text) == test.expected, test.name);
        }
    }
    for (const auto& path : {L"example.txt", L"EXAMPLE.TXT", L"README", L"new 1", L"C:\\folder.with.dot\\script", L""})
        check(fore::autoDetectEligible(path, true), "eligible normal-text filename");
    for (const auto& path : {L"example.fore", L"example.py", L"example.sql", L"example.log", L"example.txt.bak", L".gitignore"})
        check(!fore::autoDetectEligible(path, true), "ineligible extension remains unchanged");
    check(!fore::autoDetectEligible(L"example.txt", false), "explicit non-text language is preserved");
    check(!fore::autoDetectEligible(L"README", false), "extensionless non-text language is preserved");
    std::printf("DETECTION: %d/%d checks passed\n", checks-failures, checks);
    return failures ? 1 : 0;
}
