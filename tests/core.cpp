#include "ForeLexer.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <chrono>

static int checks = 0, failures = 0;
static void check(bool ok, const char* label) {
    ++checks;
    if (!ok) { ++failures; std::printf("FAIL: %s\n", label); }
}
static void style(const std::string& text, const std::string& span, int expected, const char* label) {
    auto a = fore::analyze(text); size_t p = text.find(span);
    bool ok = p != std::string::npos;
    for (size_t i = p; ok && i < p + span.size(); ++i) ok = a.styles[i] == expected;
    check(ok, label);
}
static void headers(const std::string& text, const std::vector<size_t>& expected, const char* label) {
    auto a = fore::analyze(text); std::vector<size_t> actual;
    for (size_t i = 0; i < a.folds.size(); ++i) if (a.folds[i] & 0x2000) actual.push_back(i + 1);
    if (actual != expected) { std::printf("%s headers:", label); for (auto i : actual) std::printf(" %zu", i); std::puts(""); }
    check(actual == expected, label);
    check((a.folds.back() & 0xFFF) == 1024 && (a.folds.back() >> 16) == 1024, "balanced fold depth at EOF");
}
static std::string endings(const std::string& s, const std::string& ending) {
    std::string r; for (char c : s) r += c == '\n' ? ending : std::string(1, c); return r;
}
int main() {
    using namespace fore;
    style("cLaSs X: Object", "cLaSs", Keyword, "case-insensitive keyword");
    style("x: Integer;", "Integer", Type, "built-in type");
    style("x:=True And Not False;", "True", Constant, "constant");
    style("x:=True And Not False;", "And", Keyword, "word operator");
    style("ClassName := BeforeIf;", "ClassName", Identifier, "no keyword prefix match");
    style("ClassName := BeforeIf;", "BeforeIf", Identifier, "no keyword suffix match");
    style("x:=12.5E-2;", "12.5E-2", Number, "decimal exponent");
    style("Array[0..2]", "..", Operator, "range stays separate from number");
    style("c:=#1040;", "#1040", Character, "Unicode character code");
    style("c:='A';", "'A'", Character, "character literal");
    style("c:='''';", "''''", Character, "apostrophe doubling");
    style("s:=\"\";", "\"\"", String, "empty string");
    style("s:=\"\"\"If End If\"\"\";", "\"\"\"If End If\"\"\"", String, "triple quotes close correctly");
    style("s:=\"C:\\temp\\\";", "\"C:\\temp\\\"", String, "backslash is not escape");
    style("// If End If\nx:=1;", "// If End If", Comment, "line comment");
    style("/// Class End Class\nx:=1;", "/// Class End Class", DocComment, "documentation line comment");
    style("{If\nEnd If}\nx:=1;", "{If\nEnd If}", Comment, "multiline comment");
    style("s:=\"unterminated\nClass C\nEnd Class C;", "Class", Keyword, "recover at newline after broken string");
    const std::string ordinary =
        "Class C: Object\n"                       // 1
        "Property Old: Integer Get ReadOld;\n"     // 2
        "Public Sub Run;\n"                       // 3
        "Var x: Integer;\n"                       // 4
        "Begin\n"                                 // 5
        "If True Then\n"                          // 6
        "Return;\n"                               // 7
        "Else\n"                                  // 8
        "Return;\n"                               // 9
        "End If;\n"                               // 10
        "End Sub Run;\n"                          // 11
        "End Class C;\n";                         // 12
    const std::string properties =
        "Interface I\n"                           // 1
        "Sub Run;\n"                              // 2
        "Function Read: Integer;\n"                // 3
        "Property P: Integer\n"                    // 4
        "Get\n"                                   // 5
        "End Property P;\n"                        // 6
        "End Interface I;\n"                       // 7
        "Delegate Sub Handler;\n"                  // 8
        "Class C: Object\n"                        // 9
        "Property P: Integer\n"                    // 10
        "Get\n"                                   // 11
        "Begin\n"                                 // 12
        "Return 1;\n"                              // 13
        "End Get\n"                               // 14
        "Set\n"                                   // 15
        "Begin\n"                                 // 16
        "Return;\n"                               // 17
        "End Set\n"                               // 18
        "End Property P;\n"                        // 19
        "End Class C;\n";                          // 20
    const std::string nested = "Sub Outer;\nSub Inner;\nBegin\nReturn;\nEnd Sub Inner;\nBegin\nInner;\nEnd Sub Outer;\n";
    const std::string comments = "Sub Main;\nBegin\n{ If\nEnd If }\ns:=\"If End If\";\nIf True Then\nReturn;\nEnd {comment}\nIf;\nEnd Sub Main;\n";
    const std::string mixed = "If True Then End If; {a\nb}\nSub Next;\nBegin\nReturn;\nEnd Sub Next;\n";
    for (const auto& eol : {std::string("\n"), std::string("\r\n"), std::string("\r")}) {
        headers(endings(ordinary, eol), {1,3,6,8}, "class, old property, method header and condition");
        headers(endings(properties, eol), {1,4,9,10,11,15}, "interface signatures and new properties");
        headers(endings(nested, eol), {1,2}, "nested procedures");
        headers(endings(comments, eol), {1,3,6}, "comments, strings and split End If");
        headers(endings(mixed, eol), {1,3}, "same-line close followed by multiline comment");
    }
    check(analyze("").styles.empty(), "empty document");
    check(analyze("End If; End Class C;").folds[0] == (1024 | (1024 << 16)), "unmatched closers do not underflow");
    auto unterminated = analyze("{unterminated\ncomment");
    check((unterminated.folds[0] & 0x2000) != 0, "unclosed comment remains foldable");
    auto noCommentFold = analyze("{a\nb}", false);
    check(!(noCommentFold.folds[0] & 0x2000), "comment folding property");
    std::string large;
    for (int i = 0; i < 3000; ++i) large += ordinary;
    auto start = std::chrono::steady_clock::now(); auto a = analyze(large);
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    check(a.styles.size() == large.size(), "large document styles every byte");
    std::printf("BENCHMARK: %zu bytes, %zu lines, %.2f ms\n", large.size(), a.lineStarts.size(), ms);
    std::printf("CORE: %d/%d checks passed\n", checks - failures, checks);
    return failures ? 1 : 0;
}
