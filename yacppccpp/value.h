#pragma once
#include <cstring>
#include "codetype.h"
namespace llvm {
    class AllocaInst;
}
namespace codegen {
    class value {
    public:
        llvm::AllocaInst* m_value;
        std::string m_name;
        std::shared_ptr<codetype> m_type;
        value(llvm::AllocaInst* p_value, std::string p_name, std::shared_ptr<codetype> p_type);
        ~value();
    };
}
