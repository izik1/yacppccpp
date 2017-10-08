#include "stdafx.h"
#include "token.h"

token::token(
    const type p_type,
    const int64_t p_value,
    const std::string p_strval,
    const size_t p_startPos,
    const size_t p_len) {
    m_type = p_type;
    m_value = p_value;
    m_strval = p_strval;
    m_startPos = p_startPos;
    m_len = p_len;
}

token token::expect(const type p_type) const {
    if(this->m_type == p_type) return *this;
    throw std::logic_error("Unexpected type: (" + typeStringMap[this->m_type] + ") expected: " + typeStringMap[p_type]);
}

token token::eof(const size_t p_fileLen) {
    return token(type::eof, 0, "", p_fileLen, 1);
}
