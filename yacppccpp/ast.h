#pragma once
#include <vector>
#include <memory>
#include "token.h"
class ast {
public:
    token m_tok;
    std::vector<std::shared_ptr<ast>> subtrees;
    ast(const token p_tok);
    ast();
    ~ast();
};
