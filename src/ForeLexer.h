#pragma once
#include <string>
#include <vector>
#include <utility>

namespace fore {
enum Style { Default, Keyword, Type, Constant, Number, String, Character, Comment,
             Operator, Identifier, DocComment, StyleCount };
struct Token {
    std::string word;
    size_t start = 0, end = 0, line = 0;
    bool identifier = false;
};
struct Analysis {
    std::vector<char> styles;
    std::vector<Token> tokens;
    std::vector<size_t> lineStarts;
    std::vector<std::pair<size_t, int>> commentEvents;
    std::vector<int> folds;
};
Analysis analyze(const std::string& text, bool foldComments = true);
const char* styleName(int style);
}
