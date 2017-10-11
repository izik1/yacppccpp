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
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include "generator.h"
constexpr auto str = "\
let int:  i = 20+0; \n\
let int: j = 1; \n\
let int: k = (i + j); \n\
let int: name = 0; \n\
name += k + 1;\n\
if(name == 22) { name = 11; }\n\
else { name = 22;} \n";

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
