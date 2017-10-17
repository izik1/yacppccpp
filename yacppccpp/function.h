#pragma once
#include "codetype.h"
#pragma warning(push, 0)
#include "llvm/IR/Function.h"
#pragma warning(pop)
#include <vector>
class function {
public:
    const std::vector<std::shared_ptr<codetype>> m_paramTypes;
    std::shared_ptr<codetype> retType;
    llvm::Function* m_function;
    function(
        const std::vector<std::shared_ptr<codetype>> p_paramTypes,
        std::shared_ptr<codetype> p_retType,
        llvm::Function* p_function);
private:
};
