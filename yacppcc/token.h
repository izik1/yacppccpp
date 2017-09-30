#pragma once
#include <string>
class token {
public:
    enum datatype {
        NUMBER,
        IDENTIFIER,
        OPERATOR,
    };

    std::string m_value;

    datatype m_type;
    size_t m_position;
    token(std::string p_value, const datatype p_type, const size_t p_position);
    ~token();
};
