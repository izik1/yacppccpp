#pragma once
#include <vector>
#include <memory>
#include "token.h"
class exprtree {
public:
    token m_tok;
    std::vector<std::shared_ptr<exprtree>> subtrees;
    exprtree(const token p_tok);
    exprtree();
    ~exprtree();
};
