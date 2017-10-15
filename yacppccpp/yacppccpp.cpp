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
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>
#include "generator.h"
#include "unexpected_token_exception.h"

void printToken(token tok, size_t indent) {

    //std::cerr << std::string(indent, ' ') << typeStringMap.at(tok.m_type) << ", " <<
    //    std::to_string(tok.m_startPos) << ", " << std::to_string(tok.m_len) << ", " << tok.m_strval << std::endl;
}

void printTree(std::shared_ptr<exprtree> tree, size_t depth) {
    printToken(tree->m_tok, depth);

    for each (auto subtree in tree->subtrees) printTree(subtree, depth + 1);
}

// "Borrowed" from https://stackoverflow.com/a/116220/6221420
std::string slurp(std::ifstream& in) {
    std::stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

int main(int argc, char* argv[]) {
    std::string str;
    if(argc != 2) {
        std::cerr << "Invalid arg count" << std::endl;
        return 1;
    } else {
        std::ifstream fin(argv[1]);
        str = slurp(fin);
        fin.close();
    }

    try {
        auto toks = lexer::lex(str);
        auto tree = parser::parse(toks.begin());

        printTree(tree, 0);
        codegen::generator().generate(tree);
    }
    catch(const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << std::endl;
        throw;
        return 1;
    }
    catch(const compilation_exception& ex) {
        std::cerr << ex.what(str) << std::endl;
        return 1;
    }
    return 0;
}
