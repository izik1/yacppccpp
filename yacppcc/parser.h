#pragma once
#include <vector>
#include "exprtree.h"
#include <deque>
class token;
class parser {
public:
    static std::shared_ptr<exprtree> parse(std::vector<token>::iterator iterator);
private:
    parser(std::vector<token>::iterator iterator);
    std::shared_ptr<exprtree> parse();
    std::vector<token>::iterator m_iterator;
    token peek();
    token advance();
    token current();
    std::shared_ptr<exprtree> parseStatement();
    std::shared_ptr<exprtree> parseIdentifier();
    std::shared_ptr<exprtree> parseBlock();
    std::shared_ptr<exprtree> parseVariable();
    std::shared_ptr<exprtree> parseExpression(std::shared_ptr<exprtree> lhs, size_t minPrecidence);
    std::shared_ptr<exprtree> parsePrimary();
};
