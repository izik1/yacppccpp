#pragma once
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include <string>
#include <memory>
#include <utility>
#include <unordered_map>

class codetype {
public:
    const std::string m_name;
    std::pair<std::string, codetype> make_pair() const;
    llvm::Type* getLlvmType() const;
    std::unordered_map<std::string, llvm::Function*> casts;
    codetype(llvm::Type* p_type, const std::string p_name);
    ~codetype();
private:
    llvm::Type* m_llvmType;
};
