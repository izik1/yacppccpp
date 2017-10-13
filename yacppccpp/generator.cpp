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
#undef not // this got pulled in from somewhere, I don't want it.
static llvm::LLVMContext context;
auto undef_sign_64b_t = llvm::Type::getInt64Ty(context);
auto undef_sign_32b_t = llvm::Type::getInt32Ty(context);
auto undef_sign_16b_t = llvm::Type::getInt16Ty(context);
auto undef_sign_8b_t = llvm::Type::getInt8Ty(context);
auto bool_t = llvm::Type::getInt1Ty(context);

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
    case type::keyword_while:
    case type::keyword_until:
        return whileUntilCodeGen(tree);
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
    switch(type->m_defType) {
    case defType::primSInt:
        switch(expr->m_tok.m_type) {
        case type::minus: return {type, builder.CreateNeg(val, "negtmp")}; // negate
        case type::tilda: return {type, builder.CreateNot(val, "nottmp")}; // bitwise NOT.
        default: throw std::logic_error("unimplemented unary op for primitive signed types");
        }
    case defType::primUInt:
        switch(expr->m_tok.m_type) {
        case type::tilda: return {type, builder.CreateNot(val, "nottmp")}; // bitwise NOT.
        default: throw std::logic_error("unimplemented unary op for primitive unsigned types");
        }

    case defType::primBool:
        switch(expr->m_tok.m_type) {
        case type::not: return {type, builder.CreateNot(val, "nottmp")}; // bitwise NOT.
        default: throw std::logic_error("unimplemented unary op for primitive unsigned types");
        }

    case defType::user:
    {
        auto fn = type->lookupOp(expr->m_tok.m_type, {type});
        if(!fn) throw std::logic_error("unimplemented unary op for type");
        return {fn->retType, builder.CreateCall(fn->m_function, val, "unary call")};
    }
    default: throw std::logic_error("unexpected type-definition enum value in unary operation");
    }
}

exprVal generator::asCodeGen(std::shared_ptr<exprtree> &expr) {
    assert(expr->subtrees.at(1)->m_tok.m_type == type::identifier);

    codetype* lhs_type;
    llvm::Value* lhs_value;
    std::tie(lhs_type, lhs_value) = codeGen(expr->subtrees.at(0));
    auto rhs_type = types.at(expr->subtrees.at(1)->m_tok.m_strval);
    if(!lhs_type->isPrimitive() || !rhs_type->isPrimitive()) {
        return {rhs_type, builder.CreateCall(lhs_type->casts.at(rhs_type), lhs_value, "calltmp")};
    }

    switch(rhs_type->m_defType) {
    case defType::primSInt:
        switch(lhs_type->m_defType) {
        case defType::primSInt:
        case defType::primUInt:
            return {rhs_type, builder.CreateIntCast(lhs_value, rhs_type->getLlvmType(), true, "castmp")};
        }
    case defType::primUInt:
        switch(lhs_type->m_defType) {
        case defType::primSInt:
        case defType::primUInt:
            return {rhs_type, builder.CreateIntCast(lhs_value, rhs_type->getLlvmType(), false, "castmp")};
        }
    default: throw std::logic_error("unexpected type-definition enum value in cast");
    }
}

exprVal generator::whileUntilCodeGen(std::shared_ptr<exprtree> expr) {
    auto loophead = llvm::BasicBlock::Create(context, "loop_head", llvmmain);
    auto loopbody = llvm::BasicBlock::Create(context, "loop_body", llvmmain);
    auto looptail = llvm::BasicBlock::Create(context, "loop_tail", llvmmain);
    builder.CreateBr(loophead);
    builder.SetInsertPoint(loophead);
    auto condVal = unwrap(codeGen(expr->subtrees.at(0)));
    if(expr->m_tok.m_type == type::keyword_while)
        builder.CreateCondBr(condVal, loopbody, looptail);
    else builder.CreateCondBr(condVal, looptail, loopbody);

    builder.SetInsertPoint(loopbody);
    codeGen(expr->subtrees.at(1));
    builder.CreateBr(loophead);

    builder.SetInsertPoint(looptail);
    return voidExpr;
}

exprVal generator::binExprCodeGen(std::shared_ptr<exprtree> expr) {
    switch(expr->m_tok.m_type) {
    case type::equals: return assignCodeGen(expr, type::equals);
    case type::plus_equals: return assignCodeGen(expr, type::plus);
    case type::minus_equals: return assignCodeGen(expr, type::minus);
    case type::keyword_as: return asCodeGen(expr);
    default: return binExprCodeGen(codeGen(expr->subtrees.at(0)), codeGen(expr->subtrees.at(1)), expr->m_tok.m_type);
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
    if(lhs_type->isPrimitive()) {
        assert(lhs_type == rhs_type);
        switch(lhs_type->m_defType) {
        case defType::primSInt:
            switch(t) {
            case type::equals_equals: return {types.at("bool"), builder.CreateICmpEQ(lhs_value, rhs_value, "eqtmp")};
            case type::not_equals: return {types.at("bool"), builder.CreateICmpNE(lhs_value, rhs_value, "netmp")};
            case type::plus: return {lhs_type, builder.CreateNSWAdd(lhs_value, rhs_value, "addtmp")};
            case type::minus: return {lhs_type, builder.CreateNSWSub(lhs_value, rhs_value, "subtmp")};
            case type::astrisk: return {lhs_type, builder.CreateNSWMul(lhs_value, rhs_value, "multmp")};
            case type::slash: return {lhs_type, builder.CreateSDiv(lhs_value, rhs_value, "divtmp")};
            case type::carrot: return {lhs_type, builder.CreateXor(lhs_value, rhs_value, "xortmp")};
            default: throw std::logic_error("unimplemented binary op on signed integer primary");
            }
        case defType::primUInt:
            switch(t) {
            case type::equals_equals: return {types.at("bool"), builder.CreateICmpEQ(lhs_value, rhs_value, "eqtmp")};
            case type::not_equals: return {types.at("bool"), builder.CreateICmpNE(lhs_value, rhs_value, "netmp")};
            case type::plus: return {lhs_type, builder.CreateNUWAdd(lhs_value, rhs_value, "addtmp")};
            case type::minus: return {lhs_type, builder.CreateNUWSub(lhs_value, rhs_value, "subtmp")};
            case type::astrisk: return {lhs_type, builder.CreateNUWMul(lhs_value, rhs_value, "multmp")};
            case type::slash: return {lhs_type, builder.CreateUDiv(lhs_value, rhs_value, "divtmp")};
            case type::carrot: return {lhs_type, builder.CreateXor(lhs_value, rhs_value, "xortmp")};
            default: throw std::logic_error("unimplemented binary op on unsigned integer primary");
            }

        case defType::primBool:
            switch(t) {
            case type::equals_equals: return {types.at("bool"), builder.CreateICmpEQ(lhs_value, rhs_value, "eqtmp")};
            case type::not_equals:    return {types.at("bool"), builder.CreateICmpNE(lhs_value, rhs_value, "netmp")};
            default: throw std::logic_error("unimplemented binary op on boolean primary");
            }
        default: throw std::logic_error("unexpected type-definition enum value");
        }
    };
    function* op = lhs_type->lookupOp(t, {lhs_type, rhs_type});
    if(!op) throw std::logic_error("non-existant operator for given type");
    return {op->retType, builder.CreateCall(op->m_function, llvm::makeArrayRef({lhs_value, rhs_value}), "opcall")};
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
    assert(types.find(typeName) != types.end() && "Undefined type"); // assert the type exists.
    llvm::Type* type;
    if(typeName == "i32") {
        type = undef_sign_32b_t;
    } else {
        type = undef_sign_8b_t;
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
    builder.SetInsertPoint(br_false);
    if(tree->subtrees.size() == 3) codeGen(tree->subtrees.at(2));

    builder.CreateBr(br_end);
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
    types.insert(std::make_pair("i8", new codetype(undef_sign_8b_t, "i8", defType::primSInt)));
    types.insert(std::make_pair("u8", new codetype(undef_sign_8b_t, "u8", defType::primUInt)));
    types.insert(std::make_pair("i16", new codetype(undef_sign_8b_t, "i16", defType::primSInt)));
    types.insert(std::make_pair("u16", new codetype(undef_sign_8b_t, "u16", defType::primUInt)));
    types.insert(std::make_pair("i32", new codetype(undef_sign_8b_t, "i32", defType::primSInt)));
    types.insert(std::make_pair("u32", new codetype(undef_sign_8b_t, "u32", defType::primUInt)));
    types.insert(std::make_pair("i64", new codetype(undef_sign_8b_t, "i64", defType::primSInt)));
    types.insert(std::make_pair("u64", new codetype(undef_sign_8b_t, "u64", defType::primUInt)));
    types.insert(std::make_pair("bool", new codetype(bool_t, "bool", defType::primBool)));
}

generator::generator() {
    NamedValues = std::map<std::string, llvm::AllocaInst*>();
    ValueTypes = std::map<std::string, codetype*>();
    module = llvm::make_unique<llvm::Module>("yacppccpp", context);
    llvmmain = llvm::Function::Create(llvm::FunctionType::get(undef_sign_32b_t, false), llvm::Function::ExternalLinkage, "main", module.get());
    types = std::map<std::string, codetype*>();
    generatePrimitives();
}
