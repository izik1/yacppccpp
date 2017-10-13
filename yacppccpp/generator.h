#pragma once
#include <tuple>
#include <memory>
#include "exprtree.h"
#include "llvm/IR/Module.h"
#include <map>
class codetype;
namespace llvm {
    class Value;
    class AllocaInst;
}

typedef std::tuple<codetype*, llvm::Value*> exprVal;
class generator {
public:
    void generate(std::shared_ptr<exprtree> tree);
    generator();
private:

    std::map<std::string, llvm::AllocaInst*> NamedValues;
    std::map<std::string, codetype*> types;
    std::map<std::string, codetype*> ValueTypes;
    void generatePrimitives();
    llvm::Function* llvmmain;
    std::unique_ptr<llvm::Module> module;
    codetype* getTreeType(std::shared_ptr<exprtree> tree);
    exprVal codeGen(std::shared_ptr<exprtree> tree);
    exprVal asCodeGen(std::shared_ptr<exprtree> &expr);
    exprVal binExprCodeGen(std::shared_ptr<exprtree> expr);
    exprVal binExprCodeGen(exprVal lhs, exprVal rhs, const type t);
    exprVal unaryExprCodeGen(std::shared_ptr<exprtree> expr);
    exprVal letCodeGen(std::shared_ptr<exprtree> expr);
    exprVal ifCodeGen(std::shared_ptr<exprtree> tree);
    exprVal assignCodeGen(std::shared_ptr<exprtree> expr, const type t);
};
