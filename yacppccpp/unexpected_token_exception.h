#pragma once
#include "compilation_exception.h"
#include "type.h"
class unexpected_token_exception : compilation_exception {
public:
    unexpected_token_exception(type expected, type actual, size_t startPos, size_t length);
    ~unexpected_token_exception();
    const std::string what(const std::string str) const override;

private:
    type m_expected;
    type m_actual;
};
