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
    greater_than,
    less_than,
    plus_equals,
    minus_equals,
    equals_equals,
    curl_bracket_open,
    curl_bracket_close,
    identifier,
    equals,
    block,
    tilda,
    keyword_if, // cpp has keywords too so I can't just type "if"
    keyword_let,
    keyword_else,
    keyword_as,
};

extern std::unordered_map<type, std::string> typeStringMap;
extern std::unordered_map<std::string, type> lexerSymbolMap;
extern std::unordered_map<std::string, type> lexerKeywordMap;

inline std::string typeToString(const type t) {
    return typeStringMap.at(t);
}

constexpr bool isBinaryOp(const type t) {
    switch(t) {
    case type::plus:
    case type::minus:
    case type::astrisk:
    case type::carrot:
    case type::slash:
    case type::equals:
    case type::greater_than:
    case type::less_than:
    case type::equals_equals:
    case type::plus_equals:
    case type::minus_equals:
    case type::keyword_as:
        return true;
    default: return false;
    }
}

constexpr bool isUnaryOp(const type t) {
    switch(t) {
    case type::plus:
    case type::minus:
    case type::tilda:
        return true;
    default: return false;
    }
}
