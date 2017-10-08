#pragma once
#include <unordered_map>

enum class type {
    plus,
    colon,
    semicolon,
    minus,
    astrisk,
    slash,
    num,
    eof,
    carrot,
    paren_open,
    paren_close,
    curl_bracket_open,
    curl_bracket_close,
    identifier,
    equals,
    block,
    keyword_if, // cpp has keywords too so I can't just type "if"
    keyword_let,
};

extern std::unordered_map<type, std::string> typeStringMap;
extern std::unordered_map<std::string, type> lexerSymbolMap;
extern std::unordered_map<std::string, type> lexerKeywordMap;
inline bool isBinaryOp(type t) {
    switch(t) {
    case type::plus:
    case type::minus:
    case type::astrisk:
    case type::carrot:
    case type::slash:
    case type::equals:
        return true;
    default: return false;
    }
}

inline bool isUnaryOp(type t) {
    switch(t) {
    case type::plus:
    case type::minus:
    case type::carrot:
        return true;

    default: return false;
    }
}
