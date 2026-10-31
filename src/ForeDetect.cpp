#include "ForeDetect.h"
#include "ForeLexer.h"
#include <algorithm>
#include <array>
#include <optional>
#include <string_view>

namespace fore {
namespace {
enum class Kind { Sub, Function, Constructor, Class, Interface, Namespace };
struct Scope {
    Kind kind;
    std::string name;
    bool body = false, inherited = false, member = false;
};
std::optional<Kind> kindOf(std::string_view word) {
    static constexpr std::pair<std::string_view, Kind> kinds[] {
        {"sub", Kind::Sub}, {"function", Kind::Function}, {"constructor", Kind::Constructor},
        {"class", Kind::Class}, {"interface", Kind::Interface}, {"namespace", Kind::Namespace}
    };
    for (const auto& pair : kinds) if (pair.first == word) return pair.second;
    return std::nullopt;
}
bool method(Kind kind) {
    return kind == Kind::Sub || kind == Kind::Function || kind == Kind::Constructor;
}
bool modifier(std::string_view word) {
    static constexpr std::string_view words[] {"public", "private", "protected", "friend", "shared", "final"};
    return std::find(std::begin(words), std::end(words), word) != std::end(words);
}
bool strong(const Scope& scope) {
    if (method(scope.kind)) return scope.body;
    return scope.kind == Kind::Class && scope.inherited && scope.member;
}
bool bodyBoundary(std::string_view word) { return word == "begin" || word == "end"; }

class Detector {
    const std::string text;
    const Analysis analysis;
    const std::vector<Token>& tokens;
    std::vector<Scope> scopes;

    bool identifier(size_t i) const {
        if (i >= tokens.size()) return false;
        return tokens[i].identifier && analysis.styles[tokens[i].start] == Identifier;
    }
    bool whitespacePrefix(size_t first) const {
        size_t begin = analysis.lineStarts[tokens[first].line];
        if (!begin && text.compare(0, 3, "\xef\xbb\xbf") == 0) begin = 3;
        return std::all_of(text.begin()+begin, text.begin()+tokens[first].start,
                           [](char c) { return c == ' ' || c == '\t'; });
    }
    bool leading(size_t i, bool modifiers) const {
        size_t first = i;
        while (first && tokens[first-1].line == tokens[i].line) --first;
        if (!whitespacePrefix(first)) return false;
        if (!modifiers) return first == i;
        for (size_t j = first; j < i; ++j) if (!modifier(tokens[j].word)) return false;
        return true;
    }
    bool namedEnd(size_t i) const {
        if (tokens[i].word != "end" || i+3 >= tokens.size()) return false;
        return leading(i, false) && identifier(i+2) && tokens[i+3].word == ";";
    }
    bool close(size_t i) {
        if (scopes.empty()) return false;
        const auto kind = kindOf(tokens[i+1].word);
        if (!kind || scopes.back().kind != *kind) return false;
        if (scopes.back().name != tokens[i+2].word) return false;
        const bool recognized = strong(scopes.back());
        scopes.pop_back();
        return recognized;
    }
    void noteBody(size_t i) {
        if (tokens[i].word != "begin" || scopes.empty()) return;
        if (leading(i, false)) scopes.back().body = true;
    }
    void noteField(size_t i) {
        if (scopes.empty() || i+3 >= tokens.size()) return;
        if (!identifier(i) || tokens[i+1].word != ":") return;
        if (tokens[i+2].identifier && tokens[i+3].word == ";") scopes.back().member = true;
    }
    bool heading(size_t i) const {
        int brackets = 0;
        for (size_t j = i+2; j < tokens.size(); ++j) {
            if (tokens[j].line > tokens[i].line+12) break;
            const auto& word = tokens[j].word;
            if (word == "(") ++brackets;
            if (word == ")") --brackets;
            if (!brackets && word == ";") return true;
            if (brackets < 0 || bodyBoundary(word)) break;
        }
        return false;
    }
    bool inInterface() const {
        return std::any_of(scopes.begin(), scopes.end(), [](const Scope& s) { return s.kind == Kind::Interface; });
    }
    void open(size_t i) {
        const auto kind = kindOf(tokens[i].word);
        if (!kind || !identifier(i+1) || !leading(i, true)) return;
        if (method(*kind)) {
            if (heading(i) && !inInterface()) scopes.push_back({*kind, tokens[i+1].word});
            return;
        }
        const bool inherited = i+2 < tokens.size() && tokens[i+2].word == ":";
        scopes.push_back({*kind, tokens[i+1].word, false, inherited, false});
    }
public:
    explicit Detector(std::string input) : text(std::move(input)), analysis(analyze(text, false)), tokens(analysis.tokens) {}
    bool recognized() {
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (namedEnd(i)) {
                if (close(i)) return true;
                i += 3;
                continue;
            }
            noteBody(i);
            noteField(i);
            open(i);
        }
        return false;
    }
};
}

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
    auto text = input.substr(0, DetectionLimit);
    if (text.find('\0') != std::string::npos) return false;
    return Detector(std::move(text)).recognized();
}
}
