#include "stdafx.h"
#include "lexer.h"
#include "token.h"

const std::string wordEnds = "\t\n ";
const std::string numberChars = "1234567890";

bool contains(const std::string p_str, const std::string p_substr) {
    return p_str.find(p_substr) != std::string::npos;
}

std::string substr(const std::string p_str, const size_t p_start, const size_t p_end) {
    return p_str.substr(p_start, p_end - p_start);
}

bool isNum(const char p_charIn) {
    return contains(numberChars, std::string(1, p_charIn));
}

enum class modetype {
    num,
    skp,
    op,
    id,
};

bool isOp(std::string p_str) {
    for each (auto pair in lexerSymbolMap) if(contains(pair.first, p_str)) return true;

    return false;
}

bool isId(const char p_char) {
    return !isOp(std::string(1, p_char)) && !contains(wordEnds, std::string(1, p_char));
}

bool isKeyword(const std::string str) {
    return lexerKeywordMap.find(str) != lexerKeywordMap.end();
}

modetype getMode(char p_charIn) {
    if(isNum(p_charIn)) return modetype::num;
    if(isOp(std::string(1, p_charIn)))  return modetype::op;
    if(isId(p_charIn))  return modetype::id;
    else                return modetype::skp;
}

size_t lexNum(const std::string p_in, std::vector<token> &tokens, const size_t offset) {
    size_t len = 1;
    for(; offset + len < p_in.length() && isNum(p_in[offset + len]); len++) {}
    tokens.push_back(token(type::num, std::stoi(p_in.substr(offset, len)), p_in.substr(offset, len), offset, len));
    return len;
}

size_t lexIdentifier(const std::string p_in, std::vector<token> &tokens, const size_t offset) {
    size_t len = 1;
    for(; offset + len < p_in.length() && isId(p_in[offset + len]); len++) {}

    auto str = p_in.substr(offset, len);
    if(isKeyword(p_in.substr(offset, len))) tokens.push_back(token(lexerKeywordMap.at(str), 0, str, offset, len));
    else tokens.push_back(token(type::identifier, 0, str, offset, len));

    return len;
}

size_t lexOp(const std::string p_in, std::vector<token> &tokens, const size_t offset) {
    size_t len = 1;
    for(; offset + len < p_in.length() && isOp(p_in.substr(offset, len + 1)); len++) {}

    tokens.push_back(token(lexerSymbolMap[p_in.substr(offset, len)], 0, p_in.substr(offset, len), offset, len));

    return len;
}

inline size_t lexToken(const std::string p_in, std::vector<token> &tokens, const size_t offset) {
    switch(getMode(p_in[offset])) {
    case modetype::num: return lexNum(p_in, tokens, offset);
    case modetype::skp: return 1;
    case modetype::op: return lexOp(p_in, tokens, offset);
    case modetype::id: return lexIdentifier(p_in, tokens, offset);
    default: throw std::logic_error("unexpected mode type");
    }
}

std::vector<token> lexer::lex(const std::string p_in) {
    std::vector<token> vec = std::vector<token>();

    size_t currStartPos = 0;
    while(currStartPos < p_in.length()) currStartPos += lexToken(p_in, vec, currStartPos);

    vec.push_back(token::eof(p_in.length()));

    return vec;
}
