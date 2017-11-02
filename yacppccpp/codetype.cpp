#include "stdafx.h"
#include "codetype.h"
#include "function.h"

bool codetype::isPrimitive() {
    return m_defType != defType::user;
}

bool codetype::isNumeric() {
    return isPrimitive() && (m_defType == defType::primFP || m_defType == defType::primSInt || m_defType == defType::primUInt);
}

llvm::Type* codetype::getLlvmType() const {
    return m_llvmType;
}

function* codetype::lookupOp(type t, std::vector<std::shared_ptr<codetype>> args) {
    for each (auto tuple in ops) {
        type fn_t;
        function* fn;
        std::tie(fn_t, fn) = tuple;

        if(fn_t == t && fn->m_paramTypes == args) return fn;
    }

    return nullptr;
}

codetype::codetype(llvm::Type* p_type, const std::string p_name, defType p_defType) :
    m_name(p_name), casts(), m_defType(p_defType) {
    m_llvmType = p_type;
}

bool codetype::operator==(codetype & other) {
    return this->m_name == other.m_name; // for now the only thing telling types apart are the name...
}

codetype::~codetype() {}
