#pragma once
#include <vector>
#include <memory>

class token;
class ast;
class parser {
public:
    static std::shared_ptr<ast> parse(std::vector<token>::iterator iterator);
private:
    parser(std::vector<token>::iterator iterator);
    std::shared_ptr<ast> parse();
    std::vector<token>::iterator m_iterator;
    token peek();
    token advance();
    token current();
    std::shared_ptr<ast> parseUrnary(const token &tok);
    std::shared_ptr<ast> parseFunction();
    std::shared_ptr<ast> parseStatement();
    std::shared_ptr<ast> parseIf();
    std::shared_ptr<ast> parseIdentifier();
    std::shared_ptr<ast> parseBlock();
    bool parseArgument(std::shared_ptr<ast> appendTree);
    std::shared_ptr<ast> parseVariable();
    std::shared_ptr<ast> parseExpression(std::shared_ptr<ast> lhs, size_t minPrecidence);
    std::shared_ptr<ast> parsePrimary();
    std::shared_ptr<ast> parsefnCall(std::shared_ptr<ast> tree);
};
