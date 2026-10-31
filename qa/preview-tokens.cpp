// Documentation renderer: export the same lexer results used by the plugin.
#include "ForeLexer.h"
#include <fstream>
#include <iostream>
#include <iterator>

int main(int argc, char** argv) {
    if (argc != 2) return 2;
    std::ifstream input(argv[1], std::ios::binary);
    if (!input) return 3;
    const std::string text((std::istreambuf_iterator<char>(input)), {});
    const auto result = fore::analyze(text);
    std::cout << "{\"styles\":[";
    for (size_t i = 0; i < result.styles.size(); ++i)
        std::cout << (i ? "," : "") << static_cast<int>(result.styles[i]);
    std::cout << "],\"folds\":[";
    for (size_t i = 0; i < result.folds.size(); ++i)
        std::cout << (i ? "," : "") << result.folds[i];
    std::cout << "]}\n";
}
