#pragma once
#include <vector>
#include <memory>
#include "token.h"
class ast {
public:
    token m_tok;
    std::vector<std::shared_ptr<ast>> subtrees;
    ast(const token p_tok);
    void setType(std::string type);
    std::string getType();
    ast();
    ~ast();
private:
    bool m_typeEvaluated;
    std::string m_type;
};
