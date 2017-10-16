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
#include "function.h"
#include <exception>
#include <cassert>
#include "codetype.h"
#undef not // this got pulled in from somewhere, I don't want it.
#include "value.h"
namespace codegen {
    static llvm::LLVMContext context;
    auto undef_sign_64b_t = llvm::Type::getInt64Ty(context);
    auto undef_sign_32b_t = llvm::Type::getInt32Ty(context);
    auto undef_sign_16b_t = llvm::Type::getInt16Ty(context);
    auto undef_sign_8b_t = llvm::Type::getInt8Ty(context);
    auto bool_t = llvm::Type::getInt1Ty(context);
    constexpr auto externalLinkage = llvm::GlobalValue::LinkageTypes::ExternalLinkage;

    std::shared_ptr<codetype> voidtype(nullptr);
    bool currentBlockContainsReturn = false;
    exprVal voidExpr{voidtype, nullptr};
    function* currentlyParsingFunction;
    llvm::Value* unwrap(const exprVal p_val) {
        std::shared_ptr<codetype> type = voidtype;
        llvm::Value* val = nullptr;
        std::tie(type, val) = p_val;

        assert(type && "type must be non-void");
        return val;
    }

    llvm::IRBuilder<> builder(context);

    exprVal generator::codeGen(std::shared_ptr<ast> tree) {
        if(currentBlockContainsReturn) return voidExpr; // all the code past here is dead.

        if(isBinaryOp(tree->m_tok.m_type) && tree->subtrees.size() == 2) return binExprCodeGen(tree);
        if(isUnaryOp(tree->m_tok.m_type) && tree->subtrees.size() == 1) return unaryExprCodeGen(tree);

        switch(tree->m_tok.m_type) {
        case type::semicolon: return voidExpr;

        case type::keyword_let: return letCodeGen(tree);

        case type::num: return {types.at("i32"), builder.getInt32(tree->m_tok.m_value)};

        case type::identifier:
            if(NamedValues.find(tree->m_tok.m_strval) == NamedValues.end()) throw std::logic_error("undefined identifier");
            else return {NamedValues.at(tree->m_tok.m_strval).m_type, builder.CreateLoad(NamedValues.at(tree->m_tok.m_strval).m_value)};

        case type::keyword_if: return ifCodeGen(tree);

        case type::block:
            valueStack.push_back(NamedValues);
            for each (auto sub in tree->subtrees) codeGen(sub);
            NamedValues = valueStack.at(valueStack.size() - 1);
            valueStack.pop_back();
            return voidExpr;

        case type::keyword_fn:
            fnCodeGen(tree);
            return voidExpr;

        case type::keyword_while:
        case type::keyword_until:
            return whileUntilCodeGen(tree);

        case type::call: return generateCall(tree);

        case type::keyword_ret:
        {
            generateReturn(tree);
            currentBlockContainsReturn = true;
            return voidExpr;
        }
        default: throw std::logic_error("unexpected op");
        }
    }

    exprVal generator::generateCall(std::shared_ptr<ast> tree) {
        auto args = std::vector<llvm::Value*>();

        if(functions.find(tree->subtrees.at(0)->m_tok.m_strval) == functions.end()) throw std::logic_error("non-existant function");

        auto fn = functions.at(tree->subtrees.at(0)->m_tok.m_strval);

        if(tree->subtrees.size() - 1 != fn->m_paramTypes.size()) throw std::logic_error("invalid argument count");

        for(size_t i = 1; i < tree->subtrees.size(); i++) {
            llvm::Value* val;
            std::shared_ptr<codetype> type;
            std::tie(type, val) = codeGen(tree->subtrees.at(i));

            if(type != fn->m_paramTypes.at(i - 1)) throw std::logic_error("invalid argument type");

            args.push_back(val);
        }

        return {fn->retType, builder.CreateCall(fn->m_function, args, fn->retType == types.at("void") ? "" : "calltmp")};
    }

    void generator::generateReturn(std::shared_ptr<ast> tree) {
        if(currentlyParsingFunction->retType == types.at("void")) {
            assert(tree->subtrees.size() == 0 && "returns in a void function must be empty");
            builder.CreateRetVoid();
            return;
        }

        assert(tree->subtrees.size() == 1 && "returns in a non void function must not be empty");

        std::shared_ptr<codetype> type;
        llvm::Value* val;
        std::tie(type, val) = codeGen(tree->subtrees.at(0));

        assert(type == currentlyParsingFunction->retType && "return expression must have the same type as the functions return type.");
        builder.CreateRet(val);
    }

    exprVal generator::unaryExprCodeGen(std::shared_ptr<ast> expr) {
        if(expr->m_tok.m_type == type::plus) throw std::logic_error("unary plus is invalid");

        std::shared_ptr<codetype> type;
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

    exprVal generator::asCodeGen(std::shared_ptr<ast> &expr) {
        assert(expr->subtrees.at(1)->m_tok.m_type == type::identifier);

        std::shared_ptr<codetype> lhs_type;
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

    exprVal generator::whileUntilCodeGen(std::shared_ptr<ast> expr) {
        auto loophead = llvm::BasicBlock::Create(context, "loop_head", currentlyParsingFunction->m_function);
        auto loopbody = llvm::BasicBlock::Create(context, "loop_body", currentlyParsingFunction->m_function);
        auto looptail = llvm::BasicBlock::Create(context, "loop_tail", currentlyParsingFunction->m_function);

        builder.CreateBr(loophead);
        builder.SetInsertPoint(loophead);

        auto condVal = unwrap(codeGen(expr->subtrees.at(0)));
        if(expr->m_tok.m_type == type::keyword_while) builder.CreateCondBr(condVal, loopbody, looptail);
        else builder.CreateCondBr(condVal, looptail, loopbody);

        builder.SetInsertPoint(loopbody);
        codeGen(expr->subtrees.at(1));

        if(currentBlockContainsReturn) currentBlockContainsReturn = false;
        else builder.CreateBr(loophead);

        builder.SetInsertPoint(looptail);
        return voidExpr;
    }

    exprVal generator::binExprCodeGen(std::shared_ptr<ast> expr) {
        switch(expr->m_tok.m_type) {
        case type::equals: return assignCodeGen(expr, type::equals);
        case type::plus_equals: return assignCodeGen(expr, type::plus);
        case type::minus_equals: return assignCodeGen(expr, type::minus);
        case type::astrisk_equals: return assignCodeGen(expr, type::astrisk);
        case type::slash_equals: return assignCodeGen(expr, type::slash);
        case type::keyword_as: return asCodeGen(expr);
        default: return binExprCodeGen(codeGen(expr->subtrees.at(0)), codeGen(expr->subtrees.at(1)), expr->m_tok.m_type);
        }
    }

    exprVal generator::binExprCodeGen(exprVal lhs, exprVal rhs, const type t) {
        std::shared_ptr<codetype> lhs_type;
        llvm::Value* lhs_value;
        std::tie(lhs_type, lhs_value) = lhs;

        std::shared_ptr<codetype> rhs_type;
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

    exprVal generator::assignCodeGen(std::shared_ptr<ast> expr, const type t) {
        auto lhs = expr->subtrees.at(0)->m_tok;

        if(lhs.m_type != type::identifier) throw std::logic_error("expected identifier on lhs of assignment");
        if(NamedValues.find(lhs.m_strval) == NamedValues.end()) throw std::logic_error("undefined indentifier on lhs of assignment");

        auto lhs_type = NamedValues.at(lhs.m_strval).m_type;

        std::shared_ptr<codetype> rhs_type;
        llvm::Value* rhs_value;
        std::tie(rhs_type, rhs_value) = codeGen(expr->subtrees.at(1));

        if(rhs_type == nullptr) throw std::logic_error("rhs of assignment cannot be void.");
        if(rhs_type->m_name != lhs_type->m_name) throw std::logic_error("rhs of assignment must be the same type as the lhs.");

        if(t != type::equals) {
            llvm::Value* load = builder.CreateLoad(NamedValues.at(lhs.m_strval).m_value, lhs.m_strval);
            std::tie(rhs_type, rhs_value) = binExprCodeGen({lhs_type, load}, {rhs_type, rhs_value}, t);
        }

        builder.CreateStore(rhs_value, NamedValues.at(lhs.m_strval).m_value);
        return voidExpr;
    }

    exprVal generator::letCodeGen(std::shared_ptr<ast> expr) {
        auto typeName = expr->subtrees.at(0)->m_tok.m_strval;
        assert(types.find(typeName) != types.end() && "Undefined type"); // assert the type exists.

        auto id = expr->subtrees.at(1)->m_tok.m_strval;
        auto alloc = builder.CreateAlloca(types.at(typeName)->getLlvmType(), nullptr, id);
        assert(NamedValues.find(id) == NamedValues.end() && "attempt to redefine an already defined variable");
        NamedValues.insert(std::make_pair(id, codegen::value(alloc, id, types.at(typeName))));
        if(expr->subtrees.size() == 3) codeGen(expr->subtrees.at(2));

        return voidExpr;
    }

    exprVal generator::ifCodeGen(std::shared_ptr<ast> tree) {
        auto br_true = llvm::BasicBlock::Create(context, "if_true", currentlyParsingFunction->m_function);
        auto br_false = llvm::BasicBlock::Create(context, "if_false", currentlyParsingFunction->m_function);
        auto br_end = llvm::BasicBlock::Create(context, "if_end", currentlyParsingFunction->m_function);

        builder.CreateCondBr(unwrap(binExprCodeGen(tree->subtrees.at(0))), br_true, br_false);

        builder.SetInsertPoint(br_true);
        codeGen(tree->subtrees.at(1));

        if(currentBlockContainsReturn) currentBlockContainsReturn = false;
        else builder.CreateBr(br_end);

        builder.SetInsertPoint(br_false);
        if(tree->subtrees.size() == 3) codeGen(tree->subtrees.at(2));

        if(currentBlockContainsReturn) currentBlockContainsReturn = false;
        else builder.CreateBr(br_end);

        builder.SetInsertPoint(br_end);
        return voidExpr;
    }

    exprVal generator::fnCodeGen(std::shared_ptr<ast> tree) {
        auto fn_id = tree->subtrees.at(0);
        auto fn_args = tree->subtrees.at(1);
        auto fn_body = tree->subtrees.at(2);

        assert(fn_args->subtrees.size() % 2 == 0);

        // Does this function explicitly delcare its return type?
        auto retType = tree->subtrees.size() == 4 ? types.at(tree->subtrees.at(3)->m_tok.m_strval) : types.at("void");

        auto argTypes = std::vector<std::shared_ptr<codetype>>();
        auto argNames = std::vector<std::string>();
        auto llvmArgs = std::vector<llvm::Type*>();

        for(size_t i = 0; i < fn_args->subtrees.size() / 2; i++) {
            auto ty = types.at(fn_args->subtrees.at(i * 2)->m_tok.m_strval);
            argTypes.push_back(ty);
            argNames.push_back(fn_args->subtrees.at(i * 2 + 1)->m_tok.m_strval);
            llvmArgs.push_back(ty->getLlvmType());
        }

        auto llvmfnTy = llvm::FunctionType::get(retType->getLlvmType(), llvm::ArrayRef<llvm::Type*>::ArrayRef(llvmArgs), false);
        llvm::Function* llvmFn = llvm::Function::Create(llvmfnTy, externalLinkage, fn_id->m_tok.m_strval, mod.get());

        function* fn = new function(argTypes, retType, llvmFn); //FIXME: potential memory leak, use std::shared_ptr.

        functionCreationStack.push_back(fndef(argTypes, argNames, llvmArgs, fn, fn_body));
        functions.insert(std::make_pair(fn_id->m_tok.m_strval, fn));

        return voidExpr;
    }

    void generator::createFunctions() {
        for(size_t i = functionCreationStack.size(); i > 0; i--) {
            auto func = functionCreationStack.at(i - 1);

            builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", func.m_fn->m_function));
            valueStack.push_back(NamedValues);

            for(size_t j = 0; j < func.m_argNames.size(); j++) {
                auto name = func.m_argNames.at(j);
                auto type = func.m_argTypes.at(j);

                NamedValues.insert(std::make_pair(name, value(builder.CreateAlloca(type->getLlvmType(), nullptr, name), name, type)));
                builder.CreateStore(func.m_fn->m_function->args().begin() + j, NamedValues.at(name).m_value);
            }

            currentlyParsingFunction = func.m_fn;
            currentBlockContainsReturn = false;

            codeGen(func.m_body);

            if(!currentBlockContainsReturn) {
                if(func.m_fn->retType->m_name == "void") builder.CreateRetVoid();
                else throw std::logic_error("Missing return at end of non-void function");
            }

            NamedValues = valueStack.at(valueStack.size() - 1);
            valueStack.pop_back();

            functionCreationStack.pop_back();
        }
    }

    void generator::generate(std::shared_ptr<ast> tree) {
        for each (auto sub in tree->subtrees) codeGen(sub);

        createFunctions();
        llvm::errs() << "\n";
        llvm::verifyModule(*mod.get(), &llvm::errs());

        mod->print(llvm::outs(), nullptr);
    }

    std::pair<std::string, std::shared_ptr<codetype>> generatePrim(std::string name, defType deftype, llvm::Type* llvmType) {
        return std::make_pair(name, std::make_shared<codetype>(codetype(llvmType, name, deftype)));
    }

    void generator::generatePrimitives() {
        types.insert(generatePrim("i8", defType::primSInt, undef_sign_8b_t));
        types.insert(generatePrim("u8", defType::primUInt, undef_sign_8b_t));
        types.insert(generatePrim("i16", defType::primSInt, undef_sign_16b_t));
        types.insert(generatePrim("u16", defType::primUInt, undef_sign_16b_t));
        types.insert(generatePrim("i32", defType::primSInt, undef_sign_32b_t));
        types.insert(generatePrim("u32", defType::primUInt, undef_sign_32b_t));
        types.insert(generatePrim("i64", defType::primSInt, undef_sign_64b_t));
        types.insert(generatePrim("u64", defType::primUInt, undef_sign_64b_t));
        types.insert(generatePrim("bool", defType::primBool, bool_t));
        types.insert(generatePrim("void", defType::primSpec, llvm::Type::getVoidTy(context)));
    }

    generator::generator() : NamedValues(), types(), valueStack(), functions(), functionCreationStack() {
        mod = llvm::make_unique<llvm::Module>("yacppccpp", context);
        generatePrimitives();
    }
}
