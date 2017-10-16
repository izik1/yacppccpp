#include "stdafx.h"
#include "fndef.h"

fndef::fndef(
    std::vector<std::shared_ptr<codetype>> p_argTypes,
    std::vector<std::string> p_argNames,
    std::vector<llvm::Type*> p_argllvmTypes,
    function* p_fn,
    std::shared_ptr<ast> p_body) :
    m_argTypes(p_argTypes),
    m_argNames(p_argNames),
    m_argllvmTypes(p_argllvmTypes),
    m_fn(p_fn),
    m_body(p_body) {}

fndef::~fndef() {}
