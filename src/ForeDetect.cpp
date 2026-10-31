#include "ForeDetect.h"
#include "ForeLexer.h"
#include <algorithm>
#include <vector>

namespace fore {
bool autoDetectEligible(const std::wstring& path, bool normalText) {
    if (!normalText) return false;
    const auto separator = path.find_last_of(L"/\\");
    const auto name = path.substr(separator == std::wstring::npos ? 0 : separator + 1);
    const auto dot = name.find_last_of(L'.');
    if (dot == std::wstring::npos) return true;
    auto extension = name.substr(dot);
    for (auto& c : extension) if (c >= L'A' && c <= L'Z') c += L'a' - L'A';
    return extension == L".txt";
}

bool looksLikeFore(const std::string& input) {
    const std::string text = input.substr(0, DetectionLimit);
    if (text.find('\0') != std::string::npos) return false;
    const auto analysis = analyze(text, false);
    const auto& tokens = analysis.tokens; // Strings and comments produce no tokens.
    auto identifier = [&](size_t i) {
        return i < tokens.size() && tokens[i].identifier && analysis.styles[tokens[i].start] == Identifier;
    };
    auto leading = [&](size_t i, bool modifiers) {
        size_t first = i;
        while (first && tokens[first-1].line == tokens[i].line) --first;
        auto begin = analysis.lineStarts[tokens[i].line];
        // UTF-8 BOM is permitted at the start of a document.
        if (!begin && text.compare(0, 3, "\xef\xbb\xbf") == 0) begin = 3;
        for (size_t p = begin; p < tokens[first].start; ++p)
            if (text[p] != ' ' && text[p] != '\t') return false;
        for (size_t j = first; j < i; ++j) {
            const auto& word = tokens[j].word;
            if (!modifiers || (word != "public" && word != "private" && word != "protected" &&
                word != "friend" && word != "shared" && word != "final")) return false;
        }
        return true;
    };
    struct Scope { std::string kind, name; bool body = false, inherited = false, member = false; };
    std::vector<Scope> scopes;
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& word = tokens[i].word;
        if (word == "end" && leading(i, false) && i+3 < tokens.size() &&
            identifier(i+2) && tokens[i+3].word == ";") {
            if (!scopes.empty()) {
                const auto scope = scopes.back();
                if (scope.kind == tokens[i+1].word && scope.name == tokens[i+2].word) {
                    scopes.pop_back();
                    const bool method = scope.kind == "sub" || scope.kind == "function" || scope.kind == "constructor";
                    // Named, matching terminator + declaration semicolon + Begin is a strong combined signature.
                    if ((method && scope.body) || (scope.kind == "class" && scope.inherited && scope.member)) return true;
                }
            }
            i += 3;
            continue;
        }
        if (word == "begin" && leading(i, false) && !scopes.empty()) scopes.back().body = true;
        if (identifier(i) && i+3 < tokens.size() && tokens[i+1].word == ":" &&
            tokens[i+2].identifier && tokens[i+3].word == ";" && !scopes.empty()) scopes.back().member = true;
        const bool method = word == "sub" || word == "function" || word == "constructor";
        const bool type = word == "class" || word == "interface" || word == "namespace";
        if ((!method && !type) || !identifier(i+1) || !leading(i, true)) continue;
        if (type) {
            scopes.push_back({word, tokens[i+1].word, false, i+2 < tokens.size() && tokens[i+2].word == ":", false});
            continue;
        }
        int brackets = 0;
        bool heading = false;
        for (size_t j = i+2; j < tokens.size() && tokens[j].line <= tokens[i].line+12; ++j) {
            const auto& next = tokens[j].word;
            if (next == "(") ++brackets;
            if (next == ")") --brackets;
            if (!brackets && next == ";") { heading = true; break; }
            if (brackets < 0 || next == "begin" || next == "end") break;
        }
        // Interface and delegate signatures have no implementation body.
        if (heading && std::none_of(scopes.begin(), scopes.end(), [](const Scope& s) { return s.kind == "interface"; }))
            scopes.push_back({word, tokens[i+1].word});
    }
    return false;
}
}
