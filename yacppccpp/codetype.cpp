#include "stdafx.h"
#include "codetype.h"

std::pair<std::string, codetype> codetype::make_pair() const {
    return std::make_pair(this->m_name, *this);
}

llvm::Type* codetype::getLlvmType() const {
    return m_llvmType;
}

codetype::codetype(llvm::Type* p_type, const std::string p_name) : m_name(p_name) {
    m_llvmType = p_type;
}

codetype::~codetype() {}
