#include "stdafx.h"
#include "generator.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "helper.h"
#include <exception>
#include <cassert>
static llvm::LLVMContext context;
auto i32 = llvm::Type::getInt32Ty(context);
llvm::IRBuilder<> builder(context);
llvm::Value* generator::codeGen(std::shared_ptr<exprtree> tree) {
    if(isBinaryOp(tree->m_tok.m_type) && tree->subtrees.size() == 2) return binExprCodeGen(tree);
    if(isUnaryOp(tree->m_tok.m_type) && tree->subtrees.size() == 1) return unaryExprCodeGen(tree);

    switch(tree->m_tok.m_type) {
    case type::keyword_let: return letCodeGen(tree);
    case type::num: return builder.getInt32(tree->m_tok.m_value);
    case type::identifier:
        if(NamedValues.find(tree->m_tok.m_strval) == NamedValues.end()) throw std::logic_error("undefined indentifier");
        else return builder.CreateLoad(NamedValues[tree->m_tok.m_strval]);
    case type::keyword_if:
        return ifCodeGen(tree);
    case type::block:
        for each (auto sub in tree->subtrees) {
            codeGen(sub);
        }

        return nullptr;

    default: throw std::logic_error("unexpected op");
    }
}

llvm::Value* generator::unaryExprCodeGen(std::shared_ptr<exprtree> expr) {
    switch(expr->m_tok.m_type) {
    case type::plus:  throw std::logic_error("unary plus is invalid");
    case type::minus: return builder.CreateNeg(codeGen(expr->subtrees.at(0)), "negtmp"); // negate
    case type::tilda: return builder.CreateNot(codeGen(expr->subtrees.at(0)), "nottmp"); // bitwise NOT.
    default: throw std::logic_error("unimplemented unary op");
    }
}

llvm::Value* generator::binExprCodeGen(std::shared_ptr<exprtree> expr) {
    auto v = binExprCodeGen(codeGen(expr->subtrees.at(0)), codeGen(expr->subtrees.at(1)), expr->m_tok.m_type, false);
    if(v == nullptr) {
        switch(expr->m_tok.m_type) {
        case type::equals: return assignCodeGen(expr, type::equals);
        case type::plus_equals: return assignCodeGen(expr, type::plus);
        case type::minus_equals: return assignCodeGen(expr, type::minus);
        default: throw std::logic_error("unimplemented binary op");
        }
    }

    return v;
}

/// <summary>
/// Generates code for a binary expression, excluding equals operators.
/// </summary>
/// <param name="lhs">The LHS.</param>
/// <param name="rhs">The RHS.</param>
/// <param name="t">The t.</param>
/// <returns></returns>
llvm::Value* generator::binExprCodeGen(llvm::Value* lhs, llvm::Value* rhs, const type t, const bool throwOnUnexpectedType) {
    switch(t) {
    case type::equals_equals: return builder.CreateICmpEQ(lhs, rhs, "eqtmp");
    case type::plus: return builder.CreateAdd(lhs, rhs, "addtmp");
    case type::minus: return builder.CreateSub(lhs, rhs, "subtmp");
    case type::astrisk: return builder.CreateMul(lhs, rhs, "multmp");
    case type::slash: return builder.CreateMul(lhs, rhs, "divtmp");
    case type::carrot: return builder.CreateXor(lhs, rhs, "xortmp");
    default:
        if(throwOnUnexpectedType) throw std::logic_error("unimplemented binary op");
        else                      return nullptr;
    }
}

llvm::Value* generator::assignCodeGen(std::shared_ptr<exprtree> expr, const type t) {
    auto lhs = expr->subtrees.at(0)->m_tok;
    auto rhs = codeGen(expr->subtrees.at(1));
    if(lhs.m_type != type::identifier) throw std::logic_error("expected identifier on lhs of assignment");
    if(NamedValues.find(lhs.m_strval) == NamedValues.end())
        throw std::logic_error("undefined indentifier on lhs of assignment");
    if(rhs == nullptr) throw std::logic_error("invalid rhs of assignment.");

    if(t != type::equals)
        rhs = binExprCodeGen(builder.CreateLoad(NamedValues[lhs.m_strval], lhs.m_strval), rhs, t, true);
    builder.CreateStore(rhs, NamedValues.at(lhs.m_strval));
    return nullptr;
}

llvm::Value* generator::letCodeGen(std::shared_ptr<exprtree> expr) {
    assert(expr->subtrees.at(0)->m_tok.m_strval == "int"); // only allow int for now.

    llvm::Value* val = nullptr;
    auto id = expr->subtrees.at(1)->m_tok.m_strval;
    auto alloc = builder.CreateAlloca(i32, nullptr, id);
    NamedValues.insert(std::make_pair(id, alloc));
    if(expr->subtrees.size() == 3) {
        codeGen(expr->subtrees.at(2));
    }

    return nullptr;
}

llvm::Value* generator::ifCodeGen(std::shared_ptr<exprtree> expr) {
    auto br_true = llvm::BasicBlock::Create(context, "if_true", llvmmain);
    auto br_false = llvm::BasicBlock::Create(context, "if_false", llvmmain);
    auto br_end = llvm::BasicBlock::Create(context, "if_end", llvmmain);

    builder.CreateCondBr(binExprCodeGen(expr->subtrees.at(0)), br_true, br_false);
    builder.SetInsertPoint(br_true);
    codeGen(expr->subtrees.at(1));
    builder.CreateBr(br_end);
    if(expr->subtrees.size() == 3) {
        builder.SetInsertPoint(br_false);
        codeGen(expr->subtrees.at(2));
        builder.CreateBr(br_end);
    }

    builder.SetInsertPoint(br_end);
    return nullptr;
}

void generator::generate(std::shared_ptr<exprtree> tree) {
    auto bb = llvm::BasicBlock::Create(context, "entry", llvmmain);
    builder.SetInsertPoint(bb);
    for each (auto sub in tree->subtrees) {
        codeGen(sub);
    }

    builder.CreateRet(builder.CreateLoad(NamedValues["name"]));
    llvm::errs() << "\n";
    llvm::verifyModule(*module.get(), &llvm::errs());
    module->print(llvm::errs(), nullptr);
}

generator::generator() {
    NamedValues = std::map<std::string, llvm::AllocaInst*>();
    module = llvm::make_unique<llvm::Module>("yacppccpp", context);
    llvmmain = llvm::Function::Create(llvm::FunctionType::get(i32, false), llvm::Function::ExternalLinkage, "main", module.get());
}
