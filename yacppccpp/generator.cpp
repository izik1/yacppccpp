#include "stdafx.h"
#include "generator.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Instructions.h"
#include "helper.h"
#include "function.h"
#include <exception>
#include <cassert>
#include "codetype.h"
static llvm::LLVMContext context;
auto i32 = llvm::Type::getInt32Ty(context);
auto i8 = llvm::Type::getInt8Ty(context);

constexpr codetype* voidtype = nullptr;

constexpr exprVal voidExpr = {voidtype, nullptr};

llvm::Value* unwrap(const exprVal p_val) {
    codetype* type = voidtype;
    llvm::Value* val = nullptr;
    std::tie(type, val) = p_val;
    assert(type && "type must be non-void");
    return val;
}

llvm::IRBuilder<> builder(context);
codetype* generator::getTreeType(std::shared_ptr<exprtree> tree) {
    switch(tree->m_tok.m_type) {
    case type::identifier:
        return ValueTypes.at(tree->m_tok.m_strval);
    case type::num:
        return types.at("i32");
    default:
        throw std::logic_error("Couldn't find the type");
    }
}
exprVal generator::codeGen(std::shared_ptr<exprtree> tree) {
    if(isBinaryOp(tree->m_tok.m_type) && tree->subtrees.size() == 2) return binExprCodeGen(tree);
    if(isUnaryOp(tree->m_tok.m_type) && tree->subtrees.size() == 1) return unaryExprCodeGen(tree);
    switch(tree->m_tok.m_type) {
    case type::keyword_let: return letCodeGen(tree);
    case type::num: return {types.at("i32"), builder.getInt32(tree->m_tok.m_value)};
    case type::identifier:
        if(NamedValues.find(tree->m_tok.m_strval) == NamedValues.end()) throw std::logic_error("undefined indentifier");
        else return {ValueTypes.at(tree->m_tok.m_strval), builder.CreateLoad(NamedValues.at(tree->m_tok.m_strval))};
    case type::keyword_if:
        return ifCodeGen(tree);
    case type::block:
        for each (auto sub in tree->subtrees) {
            codeGen(sub);
        }

        return voidExpr;

    default: throw std::logic_error("unexpected op");
    }
}

exprVal generator::unaryExprCodeGen(std::shared_ptr<exprtree> expr) {
    if(expr->m_tok.m_type == type::plus) {
        throw std::logic_error("unary plus is invalid");
    }

    codetype* type;
    llvm::Value* val;
    std::tie(type, val) = codeGen(expr->subtrees.at(0));
    auto fn = type->lookupOp(expr->m_tok.m_type, {type});
    if(!fn) {
        throw std::logic_error("unimplemented unary op for type");
    }

    return {fn->retType, builder.CreateCall(fn->m_function, val, "unary call")};

    //switch(expr->m_tok.m_type) {
    //case type::minus: return builder.CreateNeg(codeGen(), "negtmp"); // negate
    //case type::tilda: return builder.CreateNot(codeGen(expr->subtrees.at(0)), "nottmp"); // bitwise NOT.
    //default: throw std::logic_error("unimplemented unary op");
    //}
}

exprVal generator::binExprCodeGen(std::shared_ptr<exprtree> expr) {
    if(expr->m_tok.m_type == type::keyword_as) {
        assert(expr->subtrees.at(1)->m_tok.m_type == type::identifier);

        codetype* lhs_type;
        llvm::Value* lhs_value;
        std::tie(lhs_type, lhs_value) = codeGen(expr->subtrees.at(0));
        auto rhsType = types.at(expr->subtrees.at(1)->m_tok.m_strval);

        return {rhsType, builder.CreateCall(lhs_type->casts.at(rhsType), lhs_value, "calltmp")};
    }
    switch(expr->m_tok.m_type) {
    case type::equals: return assignCodeGen(expr, type::equals);
    case type::plus_equals: return assignCodeGen(expr, type::plus);
    case type::minus_equals: return assignCodeGen(expr, type::minus);
    default:
        return binExprCodeGen(codeGen(expr->subtrees.at(0)), codeGen(expr->subtrees.at(1)), expr->m_tok.m_type);
    }
}

/// <summary>
/// Generates code for a binary expression, excluding equals operators.
/// </summary>
/// <param name="lhs">The LHS.</param>
/// <param name="rhs">The RHS.</param>
/// <param name="t">The t.</param>
/// <returns></returns>
exprVal generator::binExprCodeGen(exprVal lhs, exprVal rhs, const type t) {
    codetype* lhs_type;
    llvm::Value* lhs_value;
    std::tie(lhs_type, lhs_value) = lhs;

    codetype* rhs_type;
    llvm::Value* rhs_value;
    std::tie(rhs_type, rhs_value) = rhs;
    assert(lhs_type && rhs_type && "subexpressions cannot be void");

    function* op = lhs_type->lookupOp(t, {lhs_type, rhs_type});
    if(!op) throw std::logic_error("non-existant operator for given type");

    return {op->retType, builder.CreateCall(op->m_function, llvm::makeArrayRef({lhs_value, rhs_value}), "opcall")};

    //switch(t) {
    //case type::equals_equals: return builder.CreateICmpEQ(lhs, rhs, "eqtmp");
    //case type::plus: return builder.CreateAdd(lhs, rhs, "addtmp");
    //case type::minus: return builder.CreateSub(lhs, rhs, "subtmp");
    //case type::astrisk: return builder.CreateMul(lhs, rhs, "multmp");
    //case type::slash: return builder.CreateMul(lhs, rhs, "divtmp");
    //case type::carrot: return builder.CreateXor(lhs, rhs, "xortmp");
    //default: throw std::logic_error("unimplemented binary op");
    //}
}

exprVal generator::assignCodeGen(std::shared_ptr<exprtree> expr, const type t) {
    auto lhs = expr->subtrees.at(0)->m_tok;

    if(lhs.m_type != type::identifier) throw std::logic_error("expected identifier on lhs of assignment");

    if(NamedValues.find(lhs.m_strval) == NamedValues.end()) throw std::logic_error("undefined indentifier on lhs of assignment");

    auto lhs_type = ValueTypes.at(lhs.m_strval);

    codetype* rhs_type;
    llvm::Value* rhs_value;
    std::tie(rhs_type, rhs_value) = codeGen(expr->subtrees.at(1));

    if(rhs_type == nullptr) throw std::logic_error("rhs of assignment cannot be void.");

    if(rhs_type->m_name != lhs_type->m_name) throw std::logic_error("rhs of assignment must be the same type as the lhs.");

    if(t != type::equals) {
        auto load = builder.CreateLoad(NamedValues[lhs.m_strval], lhs.m_strval);
        std::tie(rhs_type, rhs_value) = binExprCodeGen({lhs_type, (llvm::Value*)load}, {rhs_type, rhs_value}, t);
    }

    builder.CreateStore(rhs_value, NamedValues.at(lhs.m_strval));
    return voidExpr;
}

exprVal generator::letCodeGen(std::shared_ptr<exprtree> expr) {
    auto typeName = expr->subtrees.at(0)->m_tok.m_strval;
    assert(typeName == "i32" || typeName == "i8"); // only allow int for now.
    llvm::Type* type;
    if(typeName == "i32") {
        type = i32;
    } else {
        type = i8;
    }

    llvm::Value* val = nullptr;
    auto id = expr->subtrees.at(1)->m_tok.m_strval;
    auto alloc = builder.CreateAlloca(type, nullptr, id);
    NamedValues.insert(std::make_pair(id, alloc));
    ValueTypes.insert(std::make_pair(id, types.at(typeName)));
    if(expr->subtrees.size() == 3) {
        codeGen(expr->subtrees.at(2));
    }

    return voidExpr;
}

exprVal generator::ifCodeGen(std::shared_ptr<exprtree> tree) {
    auto br_true = llvm::BasicBlock::Create(context, "if_true", llvmmain);
    auto br_false = llvm::BasicBlock::Create(context, "if_false", llvmmain);
    auto br_end = llvm::BasicBlock::Create(context, "if_end", llvmmain);
    builder.CreateCondBr(unwrap(binExprCodeGen(tree->subtrees.at(0))), br_true, br_false);
    builder.SetInsertPoint(br_true);
    codeGen(tree->subtrees.at(1));
    builder.CreateBr(br_end);
    if(tree->subtrees.size() == 3) {
        builder.SetInsertPoint(br_false);
        codeGen(tree->subtrees.at(2));
        builder.CreateBr(br_end);
    }

    builder.SetInsertPoint(br_end);
    return voidExpr;
}

void generator::generate(std::shared_ptr<exprtree> tree) {
    auto bb = llvm::BasicBlock::Create(context, "entry", llvmmain);
    builder.SetInsertPoint(bb);
    for each (auto sub in tree->subtrees) {
        codeGen(sub);
    }

    builder.CreateRet(builder.CreateLoad(NamedValues["i"]));
    llvm::errs() << "\n";
    llvm::verifyModule(*module.get(), &llvm::errs());
    module->print(llvm::errs(), nullptr);
}

void i32_add(std::map<std::string, codetype*> types, llvm::Module* mod) {
    auto i32_t = types.at("i32");
    auto i32_llvm_t = i32_t->getLlvmType();
}

void generator::generatePrimitives() {
    types.insert(std::make_pair("i32", new codetype(i32, "i32")));
    auto i32_t = types.at("i32");
    types.insert(std::make_pair("i8", new codetype(i8, "i8")));
    auto i8_t = types.at("i8");
    auto i32_llvm_type = i32_t->getLlvmType();
    auto i32_to_i8_fnType = llvm::FunctionType::get(i8_t->getLlvmType(), i32_llvm_type, false);
    auto i32_to_i8 = llvm::Function::Create(i32_to_i8_fnType, llvm::Function::ExternalLinkage, "i32_i8_cast", module.get());

    llvm::Value* num = i32_to_i8->arg_begin();
    auto i32_i8_cast_bb = llvm::BasicBlock::Create(context, "entry", i32_to_i8);
    builder.SetInsertPoint(i32_i8_cast_bb);

    builder.CreateRet(builder.CreateIntCast(num, i8_t->getLlvmType(), true, "cast"));
    i32_t->casts.insert(std::make_pair(i8_t, i32_to_i8));

    auto i32_arrith_type = llvm::FunctionType::get(i32_llvm_type, llvm::makeArrayRef({i32_llvm_type, i32_llvm_type}), false);
    auto i32_add = llvm::Function::Create(i32_arrith_type, llvm::GlobalValue::LinkageTypes::ExternalLinkage, "i32_add", module.get());
    llvm::Value* lhs = i32_add->arg_begin();
    llvm::Value* rhs = i32_add->arg_begin() + 1;
    auto i32_add_bb = llvm::BasicBlock::Create(context, "entry", i32_add);
    builder.SetInsertPoint(i32_add_bb);
    builder.CreateRet(builder.CreateAdd(lhs, rhs, "addtmp", false, true));
    auto f = new function({i32_t, i32_t}, i32_t, i32_add);
    i32_t->ops.push_back({type::plus, f});
}

generator::generator() {
    NamedValues = std::map<std::string, llvm::AllocaInst*>();
    ValueTypes = std::map<std::string, codetype*>();
    module = llvm::make_unique<llvm::Module>("yacppccpp", context);
    llvmmain = llvm::Function::Create(llvm::FunctionType::get(i32, false), llvm::Function::ExternalLinkage, "main", module.get());
    types = std::map<std::string, codetype*>();
    generatePrimitives();
}
