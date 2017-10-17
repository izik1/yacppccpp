#pragma once
#include "codetype.h"
#include <vector>
#include <string>
#pragma warning(push, 0)
#include "llvm/IR/Type.h"
#pragma warning(pop)
#include "function.h"
#include "ast.h"
class fndef {
public:
    std::vector<std::shared_ptr<codetype>> m_argTypes;
    std::vector<std::string> m_argNames;
    std::vector<llvm::Type*> m_argllvmTypes;
    function* m_fn;
    std::shared_ptr<ast> m_body;

    fndef(
        std::vector<std::shared_ptr<codetype>> p_argTypes,
        std::vector<std::string> p_argNames,
        std::vector<llvm::Type*> p_argllvmTypes,
        function* p_fn,
        std::shared_ptr<ast> p_body);
    ~fndef();
};
