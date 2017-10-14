#include "stdafx.h"
#include "unexpected_token_exception.h"

unexpected_token_exception::unexpected_token_exception(type expected, type actual, size_t startPos, size_t length) :
    compilation_exception(startPos, length), m_expected(expected), m_actual(actual) {}

unexpected_token_exception::~unexpected_token_exception() {}

const std::string unexpected_token_exception::what(const std::string str) const {
    return "Unexpected token: \" " + str.substr(m_startPos, m_len) + " \" " +
        getPositionString(str) + " Expected " + typeToString(m_expected);
}
