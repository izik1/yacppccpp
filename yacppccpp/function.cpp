#include "stdafx.h"
#include "function.h"

function::function(const std::vector<std::shared_ptr<codetype>> p_paramTypes,
    std::shared_ptr<codetype> p_retType,
    llvm::Function* p_function) :
    m_paramTypes(p_paramTypes), retType(p_retType), m_function(p_function) {}
