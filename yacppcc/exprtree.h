#pragma once
#include <vector>
#include <string>
#include "token.h"
#include <memory>
class exprtree {
public:
    token m_tok;
    std::vector<std::shared_ptr<exprtree>> subtrees;
    exprtree(const token p_tok);
    exprtree();
    ~exprtree();
};
