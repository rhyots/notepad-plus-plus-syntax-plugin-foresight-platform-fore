#include "ForeLexer.h"
#include "ForeWords.h"
#include <algorithm>
#include <string_view>
#include <initializer_list>
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
bool oneOf(std::string_view word, std::initializer_list<std::string_view> choices) {
    return std::find(choices.begin(), choices.end(), word) != choices.end();
}
size_t sourceLine(const Analysis& analysis, size_t position) {
    return std::upper_bound(analysis.lineStarts.begin(), analysis.lineStarts.end(), position)-analysis.lineStarts.begin()-1;
}
int nestingChange(std::string_view word) {
    if (oneOf(word, {"(", "["})) return 1;
    if (oneOf(word, {")", "]"})) return -1;
    return 0;
}
bool isNewProperty(const std::vector<Token>& tokens, size_t i) {
    int brackets = 0;
    for (size_t j = i+1; j < tokens.size(); ++j) {
        const auto& word = tokens[j].word;
        brackets += nestingChange(word);
        if (brackets > 0) continue;
        if (oneOf(word, {";", "property", "class", "interface"})) return false;
        if (!oneOf(word, {"get", "set"})) continue;
        if (j+1 == tokens.size()) return false;
        return oneOf(tokens[j+1].word, {"begin", "end", "get", "set"});
    }
    return false;
}
bool branchApplies(std::string_view branch, std::string_view scope) {
    if (oneOf(branch, {"else", "elseif"})) return oneOf(scope, {"if", "select", "try"});
    return scope == "try";
}
class Folder {
    using Event = std::pair<size_t, int>;
    struct Scope { std::string kind; bool interfaceScope; };
    Analysis& analysis;
    std::vector<std::vector<Event>> events;
    std::vector<Scope> scopes;
    size_t position = 0;

    void addComments(bool enabled) {
        if (!enabled) return;
        for (const auto& event : analysis.commentEvents)
            events[sourceLine(analysis, event.first)].push_back(event);
    }
    void open(const std::string& kind, size_t line) {
        scopes.push_back({kind, kind == "interface"});
        events[line].push_back({position, 1});
    }
    void close(const std::string& kind, size_t line) {
        if (scopes.empty()) return;
        if (scopes.back().kind != kind) return;
        scopes.pop_back();
        events[line].push_back({position, -1});
    }
    void end(size_t& i) {
        if (i+1 == analysis.tokens.size()) return;
        close(analysis.tokens[i+1].word, analysis.tokens[i].line);
        ++i; // The second word in End If must not reopen a fold.
    }
    void branch(const Token& token) {
        if (scopes.empty()) return;
        if (!branchApplies(token.word, scopes.back().kind)) return;
        events[token.line].push_back({position, -1});
        events[token.line].push_back({position, 1});
    }
    void method(size_t i) {
        const bool interfaceScope = std::any_of(scopes.begin(), scopes.end(), [](const Scope& s) { return s.interfaceScope; });
        const bool delegate = i > 0 && analysis.tokens[i-1].word == "delegate";
        if (!interfaceScope && !delegate) open(analysis.tokens[i].word, analysis.tokens[i].line);
    }
    void accessor(size_t i) {
        if (scopes.empty()) return;
        if (scopes.back().kind != "property") return;
        if (i+1 == analysis.tokens.size()) return;
        if (analysis.tokens[i+1].word == "begin") open(analysis.tokens[i].word, analysis.tokens[i].line);
    }
    void opening(size_t i) {
        const auto& token = analysis.tokens[i];
        const auto& word = token.word;
        if (oneOf(word, {"class", "interface", "namespace", "enum", "if", "for", "while", "with", "try", "repeat", "select"}))
            open(word, token.line);
        else if (oneOf(word, {"sub", "function", "constructor"})) method(i);
        else if (word == "property" && isNewProperty(analysis.tokens, i)) open(word, token.line);
        else if (oneOf(word, {"get", "set"})) accessor(i);
    }
    bool structural(size_t i) const {
        if (!analysis.tokens[i].identifier) return false;
        return !i || analysis.tokens[i-1].word != ".";
    }
    void token(size_t& i) {
        if (!structural(i)) return;
        const auto& current = analysis.tokens[i];
        position = current.start;
        if (current.word == "end") end(i);
        else if (current.word == "until") close("repeat", current.line);
        else if (oneOf(current.word, {"else", "elseif", "except", "finally"})) branch(current);
        else opening(i);
    }
    int lineLevel(size_t line, int& depth) {
        int minimum = depth;
        auto& lineEvents = events[line];
        std::stable_sort(lineEvents.begin(), lineEvents.end(), [](const Event& a, const Event& b) { return a.first < b.first; });
        for (const auto& event : lineEvents) {
            depth = std::max(0, depth+event.second);
            minimum = std::min(minimum, depth);
        }
        int level = (1024+minimum) | ((1024+depth) << 16);
        if (depth > minimum) level |= 0x2000;
        return level;
    }
public:
    Folder(Analysis& result, bool foldComments) : analysis(result), events(result.lineStarts.size()) { addComments(foldComments); }
    void run() {
        for (size_t i = 0; i < analysis.tokens.size(); ++i) token(i);
        int depth = 0;
        analysis.folds.resize(analysis.lineStarts.size());
        for (size_t line = 0; line < analysis.folds.size(); ++line) analysis.folds[line] = lineLevel(line, depth);
    }
};
void fold(Analysis& analysis, bool foldComments) { Folder(analysis, foldComments).run(); }

}

namespace {
class Scanner {
    const std::string& text;
    Analysis result;
    size_t position = 0;

    bool atLineEnd(size_t i) const { return text[i] == '\r' || text[i] == '\n'; }
    bool lineBreak(size_t i) const {
        if (text[i] == '\n') return true;
        if (text[i] != '\r') return false;
        return i+1 == text.size() || text[i+1] != '\n';
    }
    size_t lineAt(size_t p) const {
        return sourceLine(result, p);
    }
    void paint(size_t start, Style style) {
        std::fill(result.styles.begin()+start, result.styles.begin()+position, static_cast<char>(style));
    }
    bool lineCommentStart() const {
        return text[position] == '/' && position+1 < text.size() && text[position+1] == '/';
    }
    bool characterCodeStart() const {
        return text[position] == '#' && position+1 < text.size() && digit(text[position+1]);
    }
    void lineComment() {
        const auto start = position;
        const bool documentation = position+2 < text.size() && text[position+2] == '/';
        while (position < text.size() && !atLineEnd(position)) ++position;
        paint(start, documentation ? DocComment : Comment);
    }
    void braceComment() {
        const auto start = position++;
        while (position < text.size() && text[position] != '}') ++position;
        const bool closed = position < text.size();
        if (closed) ++position;
        paint(start, Comment);
        if (lineAt(start) == lineAt(position-1)) return;
        result.commentEvents.emplace_back(start, 1);
        if (closed) result.commentEvents.emplace_back(position-1, -1);
    }
    void quoted() {
        const auto start = position;
        const char quote = text[position++];
        while (position < text.size() && !atLineEnd(position)) {
            if (text[position++] != quote) continue;
            if (position < text.size() && text[position] == quote) ++position;
            else break;
        }
        paint(start, quote == '"' ? String : Character);
    }
    void characterCode() {
        const auto start = position;
        position += 2;
        while (position < text.size() && digit(text[position])) ++position;
        paint(start, Character);
    }
    void digits() { while (position < text.size() && digit(text[position])) ++position; }
    bool decimalFollows() const {
        return position+1 < text.size() && text[position] == '.' && digit(text[position+1]);
    }
    std::size_t skipSign(std::size_t offset) const {
        if (offset < text.size() && (text[offset] == '+' || text[offset] == '-')) return offset+1;
        return offset;
    }
    void exponent() {
        if (position == text.size()) return;
        if (text[position] != 'e' && text[position] != 'E') return;
        auto end = skipSign(position+1);
        const auto firstDigit = end;
        while (end < text.size() && digit(text[end])) ++end;
        if (end > firstDigit) position = end;
    }
    void number() {
        const auto start = position;
        digits();
        if (decimalFollows()) { ++position; digits(); }
        exponent();
        paint(start, Number);
    }
    static Style wordStyle(const std::string& word) {
        if (word == "true" || word == "false" || word == "null") return Constant;
        if (types.count(word)) return Type;
        if (reserved.count(word)) return Keyword;
        return Identifier;
    }
    void identifier() {
        const auto start = position++;
        while (position < text.size() && alnum(text[position])) ++position;
        auto word = lower(text.substr(start, position-start));
        paint(start, wordStyle(word));
        result.tokens.push_back({std::move(word), start, position, lineAt(start), true});
    }
    void other() {
        const auto start = position++;
        if (std::string_view("+-*/=<>:;,.()[]?@").find(text[start]) == std::string_view::npos) return;
        paint(start, Operator);
        result.tokens.push_back({text.substr(start, 1), start, position, lineAt(start), false});
    }
    void next() {
        const unsigned char c = text[position];
        if (lineCommentStart()) lineComment();
        else if (c == '{') braceComment();
        else if (c == '"' || c == '\'') quoted();
        else if (characterCodeStart()) characterCode();
        else if (digit(c)) number();
        else if (alpha(c)) identifier();
        else other();
    }
public:
    explicit Scanner(const std::string& source) : text(source) {
        result.styles.assign(text.size(), Default);
        result.lineStarts.push_back(0);
        for (size_t i = 0; i < text.size(); ++i) if (lineBreak(i)) result.lineStarts.push_back(i+1);
    }
    Analysis scan() {
        while (position < text.size()) next();
        return std::move(result);
    }
};
}

Analysis analyze(const std::string& text, bool foldComments) {
    auto result = Scanner(text).scan();
    fold(result, foldComments);
    return result;
}
const char* styleName(int style) {
    static const char* names[] = {"Default", "Keywords", "Built-in types", "Constants", "Numbers", "Strings", "Characters", "Comments", "Operators", "Identifiers", "Documentation comments"};
    return style >= 0 && style < StyleCount ? names[style] : "";
}
}
