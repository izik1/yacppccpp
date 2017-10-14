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
#include "unexpected_token_exception.h"
constexpr auto str = "\
fn i32 main(){               \n\
    let i32: i = 0;          \n\
    let i32: j = 15;         \n\
    while(j != 0){           \n\
        let i8: k = 2 as i8; \n\
        until(k == 0 as i8){ \n\
            k -= 1 as i8;    \n\
            i += 1;          \n\
            if(i / 2 == 10){ \n\
                i *= 5;      \n\
            }                \n\
        }                    \n\
                             \n\
        j -= 1;              \n\
    }                        \n\
                             \n\
    tst(i);                  \n\
    return i;                \n\
} fn tst(i32 a){}";

void printToken(token tok, size_t indent) {
    std::cerr << std::string(indent, ' ') << typeStringMap.at(tok.m_type) << ", " <<
        std::to_string(tok.m_startPos) << ", " << std::to_string(tok.m_len) << ", " << tok.m_strval << std::endl;
}

void printTree(std::shared_ptr<exprtree> tree, size_t depth) {
    printToken(tree->m_tok, depth);

    for each (auto subtree in tree->subtrees) printTree(subtree, depth + 1);
}

int main() {
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
    catch(const unexpected_token_exception& ex) {
        std::cerr << ex.what(str) << std::endl;
    }
    return 0;
}
