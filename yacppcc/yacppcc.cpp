// yacppcc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include "lexer.h"
#include <iostream>
#include "token.h"
#include "parser.h"
constexpr auto str = "let int:  i = 20; \n\
let int: j =1; \n\
let int: k= (i+  j); \n\
let int: name = 0; \n\
name = name + k + 1;\n";

void printTree(std::shared_ptr<exprtree> tree, size_t depth) {
    std::cout << std::string(depth, ' ') << typeStringMap[tree->m_tok.m_type] << std::endl;
    for each (auto subtree in tree->subtrees) {
        printTree(subtree, depth + 1);
    }
}

int main() {
    auto toks = lexer::lex(str);
    std::cout << "typeNum, start, len" << std::endl;
    for each (token tok in toks) {
        std::cout << typeStringMap[tok.m_type] << ", " << std::to_string(tok.m_startPos) <<
            ", " << std::to_string(tok.m_len) << std::endl;
    }
    auto tree = parser::parse(toks.begin());
    printTree(tree, 0);
    return 0;
}
