// yacppcc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include "lexer.h"
#include <iostream>
#include "token.h"
#include "parser.h"
#include "exprtree.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include "generator.h"
constexpr auto str = "\
let i32:  i = 20+0; \n\
let i8: j = i as i8;   \n\
let i8: k = 2 as i8;   \n\
let i8: l = j + k;   \n\
let i8: m = 2 as i8 + 1 as i8;   \n\
\n";

void printToken(token tok, size_t indent) {
    std::cerr << std::string(indent, ' ') << typeStringMap.at(tok.m_type) << ", " <<
        std::to_string(tok.m_startPos) << ", " << std::to_string(tok.m_len) << ", " << tok.m_strval << std::endl;
}

void printTree(std::shared_ptr<exprtree> tree, size_t depth) {
    printToken(tree->m_tok, depth);
    for each (auto subtree in tree->subtrees) {
        printTree(subtree, depth + 1);
    }
}

int main() {
    try {
        auto toks = lexer::lex(str);
        for each (auto tok in toks) {
            printToken(tok, 0);
        }

        std::cerr << std::endl;
        auto tree = parser::parse(toks.begin());
        printTree(tree, 0);
        generator().generate(tree);
        return 0;
    }
    catch(const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        throw;
        return 1;
    }
}
