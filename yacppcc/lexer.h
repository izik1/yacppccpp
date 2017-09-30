#pragma once
#include <string>
#include <vector>
#include "token.h"

typedef std::vector<token> token_vec_t;
namespace lexer {
    std::vector<token> lex(const std::string p_in);
}
