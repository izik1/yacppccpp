#pragma once
#include "codetype.h"
#include "llvm/IR/Function.h"
#include <vector>
class function {
public:
    const std::vector<codetype*> m_paramTypes;
    codetype* retType;
    llvm::Function* m_function;
    function(const std::vector<codetype*> p_paramTypes, codetype* p_retType, llvm::Function* p_function);
private:
};
