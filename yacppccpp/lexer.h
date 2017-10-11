#pragma once
#include <vector>
#include <string>
class token;
namespace lexer
{
    std::vector<token> lex(std::string str);
};
