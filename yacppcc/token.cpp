#include "stdafx.h"
#include "token.h"

token::token(std::string p_value, const datatype p_type, const size_t p_position) {
    m_value = p_value;
    m_type = p_type;
    m_position = p_position;
}

token::~token() {
}
