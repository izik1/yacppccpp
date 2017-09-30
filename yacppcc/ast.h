#pragma once
#include <vector>

class ast
{
public:
    std::vector<ast> m_childtrees;
    ast();
    ast(std::vector<ast> p_childtrees);
    ~ast();
};
