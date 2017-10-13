#pragma once
#include <tuple>
#include <memory>
#include "exprtree.h"
#include "llvm/IR/Module.h"
#include <unordered_map>
#include "value.h"
class codetype;
namespace llvm {
    class Value;
    class AllocaInst;
}

namespace codegen {
    typedef std::tuple<std::shared_ptr<codetype>, llvm::Value*> exprVal;
    class generator {
    public:
        void generate(std::shared_ptr<exprtree> tree);
        generator();
    private:

        std::unordered_map<std::string, codegen::value> NamedValues;
        std::unordered_map<std::string, std::shared_ptr<codetype>> types;
        void generatePrimitives();
        llvm::Function* llvmmain;
        std::unique_ptr<llvm::Module> module;
        exprVal codeGen(std::shared_ptr<exprtree> tree);
        exprVal asCodeGen(std::shared_ptr<exprtree> &expr);
        exprVal whileUntilCodeGen(std::shared_ptr<exprtree> expr);
        exprVal binExprCodeGen(std::shared_ptr<exprtree> expr);
        exprVal binExprCodeGen(exprVal lhs, exprVal rhs, const type t);
        exprVal unaryExprCodeGen(std::shared_ptr<exprtree> expr);
        exprVal letCodeGen(std::shared_ptr<exprtree> expr);
        exprVal ifCodeGen(std::shared_ptr<exprtree> tree);
        exprVal assignCodeGen(std::shared_ptr<exprtree> expr, const type t);
    };
}
