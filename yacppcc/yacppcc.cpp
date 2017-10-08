// yacppcc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include "lexer.h"
#include <iostream>
#include "token.h"
#include "parser.h"
constexpr auto str = "\
let int:  i = 20; \n\
let int: j = 1; \n\
let int: k = (i + j); \n\
let int: name = -(-0); \n\
name = name + k + 1;\n";

void printToken(token tok, size_t indent) {
    std::cout << std::string(indent, ' ') << typeStringMap.at(tok.m_type) << ", " <<
        std::to_string(tok.m_startPos) << ", " << std::to_string(tok.m_len) << std::endl;
}

void printTree(std::shared_ptr<exprtree> tree, size_t depth) {
    printToken(tree->m_tok, depth);
    for each (auto subtree in tree->subtrees) {
        printTree(subtree, depth + 1);
    }
}

int main() {
    try {
        auto tree = parser::parse(lexer::lex(str).begin());
        printTree(tree, 0);
        return 0;
    }
    catch(const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
    }
}
