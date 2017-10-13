#include "stdafx.h"
#include <llvm/IR/Instructions.h>
#include "value.h"

codegen::value::value(llvm::AllocaInst* p_value, std::string p_name, std::shared_ptr<codetype> p_type) {
    m_value = p_value;
    m_name = p_name;
    m_type = p_type;
}

codegen::value::~value() {}
