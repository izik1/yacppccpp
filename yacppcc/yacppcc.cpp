// yacppcc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <vector>
#include "lexer.h"
#include <iostream>
constexpr auto str = "int  i = 2; \
int j =1; #testssaetaet \
int k= i+  j; \
int name = 0; \
name += k + 1;";
int main()
{
    auto lexedStr = lexer::lex(str);
    std::cout << "type, position, value" << std::endl;
    for each (token t in lexedStr)
    {
        std::cout << t.m_type << ", " << t.m_position << ", '" << t.m_value << "'" << std::endl;
    }
    return 0;
}
