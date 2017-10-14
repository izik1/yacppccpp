#include "stdafx.h"
#include "invalid_primary_exception.h"

invalid_primary_exception::invalid_primary_exception(const size_t startPos, const size_t len, const type type) :
    compilation_exception(startPos, len), m_type(type) {}

invalid_primary_exception::~invalid_primary_exception() {}

const std::string invalid_primary_exception::what(const std::string str) const {
    return "Unexpected token in primary: \" " + str.substr(m_startPos, m_len) + " \" " + getPositionString(str);
}
