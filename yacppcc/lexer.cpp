#include "stdafx.h"
#include <algorithm>
#include <string>
#include "ParseException.h"
#include "lexer.h"

std::vector<std::string> operators{ "=", "+", "+=", ";" };
const std::string wordEnds = "\t\n;=+# ";
const std::string numberChars = "1234567890";
inline bool contains(const std::string p_str, const std::string p_substr) {
    return p_str.find(p_substr) != std::string::npos;
}

inline std::string substr(const std::string p_str, const size_t p_start, const size_t p_end) {
    return p_str.substr(p_start, p_end - p_start);
}

inline bool isNumber(const char p_charIn) {
    return contains(numberChars, std::string(1, p_charIn));
}

inline bool isOperator(const std::string p_in) {
    for each (std::string op in operators) {
        if (op == p_in) {
            return true;
        }
    }

    return false;
}

inline token::datatype getMode(char p_charIn) {
    if (isNumber(p_charIn))                   return token::NUMBER;
    if (isOperator(std::string(1, p_charIn))) return token::OPERATOR;
    else                                      return token::IDENTIFIER;
}

token_vec_t lexer::lex(const std::string p_in) {
    token_vec_t vec = token_vec_t();
    size_t currStartPos = 0;

    while (currStartPos < p_in.length()) {
        if (p_in[currStartPos] == '#') {
            while (p_in[currStartPos] != '\n' && currStartPos < p_in.length()) currStartPos++;
            if (currStartPos == p_in.length())return vec;
        }

        token::datatype mode = getMode(p_in[currStartPos]);
        size_t currEndPos = currStartPos;
        bool cont = true;
        for (; currEndPos < p_in.length() && cont; currEndPos++) {
            switch (mode) {
            case token::NUMBER:
                cont &= (isNumber(p_in[currEndPos]));
                break;
            case token::IDENTIFIER:
                cont &= !contains(wordEnds, std::string(1, p_in[currEndPos]));
                break;
            case token::OPERATOR:
                cont &= (currEndPos + 1 < p_in.length() && isOperator(substr(p_in, currStartPos, currEndPos + 1)));
                break;

            default: throw ParseException();
            }
        }

        currEndPos--;

        // this check is just-in-case spacing characters get seen as an identifier...
        if (currEndPos - currStartPos > 0) {
            vec.push_back(token(substr(p_in, currStartPos, currEndPos), mode, currStartPos));
            currStartPos = currEndPos;
        }
        else {
            currStartPos++;
        }
    }

    return vec;
}
