#pragma once
#include "compilation_exception.h"
#include "type.h"
class invalid_primary_exception :
    public compilation_exception {
public:
    invalid_primary_exception(const size_t startPos, const size_t len, const type type);
    ~invalid_primary_exception();

    const type m_type;
    virtual const std::string what(const std::string str) const override;
};
