#pragma once
#include "codetype.h"
#include <vector>
#include <string>
#include "llvm/IR/Type.h"
#include "function.h"
#include "exprtree.h"
class fndef {
public:
    std::vector<std::shared_ptr<codetype>> m_argTypes;
    std::vector<std::string> m_argNames;
    std::vector<llvm::Type*> m_argllvmTypes;
    function* m_fn;
    std::shared_ptr<exprtree> m_body;

    fndef(
        std::vector<std::shared_ptr<codetype>> p_argTypes,
        std::vector<std::string> p_argNames,
        std::vector<llvm::Type*> p_argllvmTypes,
        function* p_fn,
        std::shared_ptr<exprtree> p_body);
    ~fndef();
};
