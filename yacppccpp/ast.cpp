#include "stdafx.h"
#include "ast.h"
#include "token.h"
#include <cassert>
ast::ast() : m_tok(type::eof, 0, "", -1, -1) {}

ast::ast(const token p_tok) : m_tok(p_tok), m_type(""), m_typeEvaluated(false), subtrees() {}

void ast::setType(std::string type) {
    m_type = type;
    m_typeEvaluated = true;
}

std::string ast::getType() {
    assert(m_typeEvaluated && "Type hasn't been evaluated yet"); // assert is more convenient in debug mode.
    if(!m_typeEvaluated) throw std::logic_error("Type hasn't been evaluated yet");
    return m_type;
}

ast::~ast() {}
