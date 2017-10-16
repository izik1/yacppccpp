#include "stdafx.h"
#include "ast.h"
#include "token.h"
ast::ast() : m_tok(type::eof, 0, "", -1, -1) {
    subtrees = std::vector<std::shared_ptr<ast>>();
}

ast::ast(const token p_tok) : m_tok(p_tok) {
    subtrees = std::vector<std::shared_ptr<ast>>();
}

ast::~ast() {}
