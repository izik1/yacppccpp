#include "stdafx.h"
#include "type.h"

std::unordered_map<type, std::string> typeStringMap{
    {type::num,                "num"},
    {type::eof,                "eof"},
    {type::plus,               "plus"},
    {type::minus,              "minus"},
    {type::slash,              "slash"},
    {type::astrisk,            "astrisk"},
    {type::paren_open,         "paren_open"},
    {type::paren_close,        "paren_close"},
    {type::carrot,             "carrot"},
    {type::identifier,         "identifier"},
    {type::equals,             "equals"},
    {type::greater_than,       "greater_than"},
    {type::less_than,          "equals"},
    {type::equals_equals,      "equals_equals"},
    {type::not_equals,         "equals_equals"},
    {type::not,                "not"},
    {type::call,               "call"},
    {type::plus_equals,        "plus_equals"},
    {type::astrisk_equals,     "astrisk_equals"},
    {type::slash_equals,       "slash_equals"},
    {type::minus_equals,       "minus_equals"},
    {type::keyword_let,        "let"},
    {type::keyword_if,         "if"},
    {type::keyword_else,       "else"},
    {type::keyword_as,         "as"},
    {type::keyword_ret,        "ret"},
    {type::keyword_fn,         "fn"},
    {type::keyword_do,         "do"},
    {type::keyword_dowhile,    "dowhile"},
    {type::keyword_true,       "true"},
    {type::keyword_false,      "false"},
    {type::keyword_while,      "while"},
    {type::keyword_until,      "until"},
    {type::fn_args,            "fn_args"},
    {type::colon,              "colon"},
    {type::comma,              "comma"},
    {type::semicolon,          "semicolon"},
    {type::block,              "block"},
    {type::tilda,              "tilda"},
    {type::curl_bracket_open,  "curl_bracket_open"},
    {type::curl_bracket_close, "curl_bracket_close"},
};

std::unordered_map<std::string, type> lexerSymbolMap{
    {"+", type::plus},
    {"-", type::minus},
    {"*", type::astrisk},
    {"/", type::slash},
    {"(", type::paren_open},
    {"{", type::curl_bracket_open},
    {">", type::greater_than},
    {"<", type::less_than},
    {")", type::paren_close},
    {"}", type::curl_bracket_close},
    {"^", type::carrot},
    {"=", type::equals},
    {"+=", type::plus_equals},
    {"*=", type::astrisk_equals},
    {"/=", type::slash_equals},
    {"-=", type::minus_equals},
    {"==", type::equals_equals},
    {"!=", type::not_equals},
    {":", type::colon},
    {";", type::semicolon},
    {"~", type::tilda},
    {"!", type::not},
    {",", type::comma},
};

std::unordered_map<std::string, type> lexerKeywordMap{
    {"if", type::keyword_if},
    {"while", type::keyword_while},
    {"until", type::keyword_until},
    {"return", type::keyword_ret},
    {"let", type::keyword_let},
    {"else", type::keyword_else},
    {"as", type::keyword_as},
    {"fn", type::keyword_fn},
    {"true", type::keyword_true},
    {"false",type::keyword_false},
    {"do",type::keyword_do},
};
