#include "stdafx.h"
#include "ast.h"

ast::ast()
{
    m_childtrees = std::vector<ast>();
}

ast::ast(std::vector<ast> p_childtrees) : m_childtrees(p_childtrees)
{
}

ast::~ast()
{
}
