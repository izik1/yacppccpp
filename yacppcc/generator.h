#pragma once
#include <memory>
#include "exprtree.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include <map>
class generator {
public:
    void generate(std::shared_ptr<exprtree> tree);
    generator();
private:
    std::map<std::string, llvm::AllocaInst*> NamedValues;
    llvm::Function* llvmmain;
    std::unique_ptr<llvm::Module> module;
    llvm::Value* codeGen(std::shared_ptr<exprtree> tree);
    llvm::Value* binExprCodeGen(std::shared_ptr<exprtree> expr);
    llvm::Value* binExprCodeGen(llvm::Value* lhs, llvm::Value* rhs, const type t, const bool throwOnInvalidType);
    llvm::Value* unaryExprCodeGen(std::shared_ptr<exprtree> expr);
    llvm::Value* letCodeGen(std::shared_ptr<exprtree> expr);
    llvm::Value* ifCodeGen(std::shared_ptr<exprtree> expr);
    llvm::Value* assignCodeGen(std::shared_ptr<exprtree> expr, const type t);
};
