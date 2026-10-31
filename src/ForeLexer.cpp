#include "ForeLexer.h"
#include "ForeWords.h"
#include <algorithm>
#include <cctype>
#include <string_view>
#include <unordered_set>

namespace fore {
namespace {
bool alpha(unsigned char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'; }
bool digit(unsigned char c) { return c >= '0' && c <= '9'; }
bool alnum(unsigned char c) { return alpha(c) || digit(c); }
std::string lower(std::string s) {
    for (char& c : s) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return s;
}
std::unordered_set<std::string> words(const char* s) {
    std::unordered_set<std::string> result;
    std::string word;
    for (; *s; ++s) {
        if (*s == ' ') { if (!word.empty()) result.insert(lower(word)); word.clear(); }
        else word += *s;
    }
    if (!word.empty()) result.insert(lower(word));
    return result;
}
const auto reserved = words(kReserved);
const auto types = words(kTypes);
bool isNewProperty(const std::vector<Token>& tokens, size_t i) {
    // Old: Property P: T Get ReadP Set WriteP;
    // New: Property P: T Get Begin ... End Get End Property P;
    // Interfaces: Property P: T Get [Set] End Property P;
    int brackets = 0;
    for (size_t j = i + 1; j < tokens.size(); ++j) {
        const auto& w = tokens[j].word;
        if (w == "(" || w == "[") ++brackets;
        if (w == ")" || w == "]") --brackets;
        if (brackets > 0) continue;
        if (w == ";" || w == "property" || w == "class" || w == "interface") return false;
        if (w == "get" || w == "set") {
            if (j + 1 == tokens.size()) return false;
            const auto& next = tokens[j + 1].word;
            return next == "begin" || next == "end" || next == "get" || next == "set";
        }
    }
    return false;
}
void fold(Analysis& a, bool foldComments) {
    std::vector<std::vector<std::pair<size_t, int>>> events(a.lineStarts.size());
    if (foldComments) for (const auto& event : a.commentEvents) {
        size_t line = std::upper_bound(a.lineStarts.begin(), a.lineStarts.end(), event.first) - a.lineStarts.begin() - 1;
        events[line].push_back(event);
    }
    struct Scope { std::string kind; bool interfaceScope; };
    std::vector<Scope> stack;
    size_t position = 0;
    auto open = [&](const std::string& kind, size_t line) {
        stack.push_back({kind, kind == "interface"}); events[line].push_back({position, 1});
    };
    auto close = [&](const std::string& kind, size_t line) {
        if (!stack.empty() && stack.back().kind == kind) { stack.pop_back(); events[line].push_back({position, -1}); }
    };
    const std::unordered_set<std::string> basic {"class", "interface", "namespace", "enum", "if", "for", "while", "with", "try", "repeat", "select"};
    const std::unordered_set<std::string> methods {"sub", "function", "constructor"};
    for (size_t i = 0; i < a.tokens.size(); ++i) {
        const auto& token = a.tokens[i];
        position = token.start;
        const auto& w = token.word;
        if (!token.identifier) continue;
        if (i > 0 && a.tokens[i - 1].word == ".") continue;
        if (w == "end") {
            if (i + 1 < a.tokens.size()) {
                close(a.tokens[i + 1].word, token.line);
                ++i; // The second word of End If etc. must not reopen a fold.
            }
        } else if (w == "until") {
            close("repeat", token.line);
        } else if (w == "else" || w == "elseif" || w == "except" || w == "finally") {
            if (!stack.empty()) {
                const auto& kind = stack.back().kind;
                if (((w == "else" || w == "elseif") && (kind == "if" || kind == "select" || kind == "try")) ||
                    ((w == "except" || w == "finally") && kind == "try")) {
                    events[token.line].push_back({position, -1}); events[token.line].push_back({position, 1});
                }
            }
        } else if (basic.count(w)) {
            open(w, token.line);
        } else if (methods.count(w)) {
            bool inInterface = std::any_of(stack.begin(), stack.end(), [](const Scope& s) { return s.interfaceScope; });
            bool delegate = i > 0 && a.tokens[i - 1].word == "delegate";
            if (!inInterface && !delegate) open(w, token.line);
        } else if (w == "property" && isNewProperty(a.tokens, i)) {
            open(w, token.line);
        } else if ((w == "get" || w == "set") && !stack.empty() && stack.back().kind == "property" &&
                   i + 1 < a.tokens.size() && a.tokens[i + 1].word == "begin") {
            open(w, token.line);
        }
    }
    int depth = 0;
    a.folds.resize(a.lineStarts.size());
    for (size_t line = 0; line < a.lineStarts.size(); ++line) {
        int minimum = depth;
        std::stable_sort(events[line].begin(), events[line].end(), [](const auto& x, const auto& y) { return x.first < y.first; });
        for (const auto& event : events[line]) { depth = std::max(0, depth + event.second); minimum = std::min(minimum, depth); }
        a.folds[line] = (1024 + minimum) | ((1024 + depth) << 16);
        if (depth > minimum) a.folds[line] |= 0x2000;
    }
}
}

Analysis analyze(const std::string& text, bool foldComments) {
    Analysis a;
    a.styles.assign(text.size(), Default);
    a.lineStarts.push_back(0);
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n' || (text[i] == '\r' && (i + 1 == text.size() || text[i + 1] != '\n')))
            a.lineStarts.push_back(i + 1);
    }
    auto lineAt = [&](size_t p) { return static_cast<size_t>(std::upper_bound(a.lineStarts.begin(), a.lineStarts.end(), p) - a.lineStarts.begin() - 1); };
    auto paint = [&](size_t from, size_t to, int style) { std::fill(a.styles.begin() + from, a.styles.begin() + to, static_cast<char>(style)); };
    size_t i = 0;
    while (i < text.size()) {
        const size_t start = i;
        const unsigned char c = text[i];
        if (c == '/' && i + 1 < text.size() && text[i + 1] == '/') {
            const int style = i + 2 < text.size() && text[i + 2] == '/' ? DocComment : Comment;
            while (i < text.size() && text[i] != '\n' && text[i] != '\r') ++i;
            paint(start, i, style);
        } else if (c == '{') {
            ++i;
            while (i < text.size() && text[i] != '}') ++i;
            bool closed = i < text.size();
            if (closed) ++i;
            paint(start, i, Comment);
            size_t first = lineAt(start), last = lineAt(i ? i - 1 : 0);
            if (first < last) {
                a.commentEvents.emplace_back(start, 1);
                if (closed) a.commentEvents.emplace_back(i - 1, -1);
            }
        } else if (c == '"' || c == '\'') {
            ++i;
            while (i < text.size() && text[i] != '\r' && text[i] != '\n') {
                if (text[i] == c) {
                    ++i;
                    if (i < text.size() && text[i] == c) { ++i; continue; }
                    break;
                }
                ++i;
            }
            paint(start, i, c == '"' ? String : Character);
        } else if (c == '#' && i + 1 < text.size() && digit(text[i + 1])) {
            i += 2; while (i < text.size() && digit(text[i])) ++i;
            paint(start, i, Character);
        } else if (digit(c)) {
            while (i < text.size() && digit(text[i])) ++i;
            if (i + 1 < text.size() && text[i] == '.' && digit(text[i + 1])) {
                ++i; while (i < text.size() && digit(text[i])) ++i;
            }
            if (i < text.size() && (text[i] == 'e' || text[i] == 'E')) {
                size_t j = i + 1;
                if (j < text.size() && (text[j] == '+' || text[j] == '-')) ++j;
                size_t digits = j; while (j < text.size() && digit(text[j])) ++j;
                if (j > digits) i = j;
            }
            paint(start, i, Number);
        } else if (alpha(c)) {
            ++i; while (i < text.size() && alnum(text[i])) ++i;
            std::string w = lower(text.substr(start, i - start));
            int style = Identifier;
            if (w == "true" || w == "false" || w == "null") style = Constant;
            else if (types.count(w)) style = Type;
            else if (reserved.count(w)) style = Keyword;
            paint(start, i, style);
            a.tokens.push_back({std::move(w), start, i, lineAt(start), true});
        } else {
            ++i;
            if (std::string_view("+-*/=<>:;,.()[]?@").find(c) != std::string_view::npos) {
                paint(start, i, Operator);
                a.tokens.push_back({text.substr(start, 1), start, i, lineAt(start), false});
            }
        }
    }
    fold(a, foldComments);
    return a;
}
const char* styleName(int style) {
    static const char* names[] = {"Default", "Keywords", "Built-in types", "Constants", "Numbers", "Strings", "Characters", "Comments", "Operators", "Identifiers", "Documentation comments"};
    return style >= 0 && style < StyleCount ? names[style] : "";
}
}
