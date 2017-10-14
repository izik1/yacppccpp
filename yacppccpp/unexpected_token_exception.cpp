#include "stdafx.h"
#include "unexpected_token_exception.h"

unexpected_token_exception::unexpected_token_exception(type expected, type actual, size_t startPos, size_t length) :
    compilation_exception(startPos, length), m_expected(expected), m_actual(actual) {}

unexpected_token_exception::~unexpected_token_exception() {}

const std::string unexpected_token_exception::what(const std::string str) const {
    size_t start_line;
    size_t start_column;
    std::tie(start_line, start_column) = getStringPos(str, m_startPos);

    size_t end_line;
    size_t end_column;
    std::tie(end_line, end_column) = getStringPos(str, m_startPos + m_len);

    return "Unexpected token: \"" + str.substr(m_startPos, m_len) + "\" @(" + std::to_string(start_line) +
        ", " + std::to_string(start_column) + ") to (" + std::to_string(end_line) + ", " +
        std::to_string(end_column) + ") Expected " + typeToString(m_expected);
}
