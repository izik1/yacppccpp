#include "stdafx.h"
#include "parser.h"
#include "token.h"
#include <unordered_map>
#include <iterator>
#include <cassert>

// larger number = higher precedence.
std::unordered_map<type, size_t> precedence = {
    {type::plus, 1},
    {type::minus, 1},
    {type::slash, 2},
    {type::carrot, 3},
    {type::astrisk, 2},
    {type::equals, 0},
};
std::unordered_map<type, bool> isRightAssositve = {
    {type::plus, false},
    {type::minus, false},
    {type::slash, false},
    {type::carrot, true},
    {type::equals, true},
    {type::astrisk, false},
};

token parser::peek() {
    return *(m_iterator);
}

token parser::current() {
    return *m_iterator;
}

token parser::advance() {
    return (*m_iterator++);
}

bool precidenceCompare(token a, token b) {
    return precedence[a.m_type] > precedence[b.m_type] ||
        (precedence[a.m_type] == precedence[b.m_type] && isRightAssositve[a.m_type]);
}

std::shared_ptr<exprtree> parser::parseExpression(std::shared_ptr<exprtree> lhs, size_t minPrecidence) {
    auto lookahead = peek();
    while(isOp(lookahead.m_type) && precedence[lookahead.m_type] >= minPrecidence) {
        auto op = lookahead;
        advance();
        auto rhs = parsePrimary();
        lookahead = peek();
        while(isOp(lookahead.m_type) && precidenceCompare(lookahead, op)) {
            rhs = parseExpression(rhs, precedence[lookahead.m_type]);
            lookahead = peek();
        }

        auto opToken = token(op.m_type, 0, "", lhs->m_tok.m_startPos, token::getCombindedLen(lhs->m_tok, rhs->m_tok));
        auto tree = std::make_shared<exprtree>(exprtree(opToken));
        tree->subtrees.push_back(lhs);
        tree->subtrees.push_back(rhs);
        lhs = tree;
    }

    return lhs;
}

std::shared_ptr<exprtree> parser::parsePrimary() {
    auto tok = advance();
    switch(tok.m_type) {
    case type::num:
        return std::make_shared<exprtree>(exprtree(tok));
    case type::paren_open:
    {
        auto tree = parseExpression(parsePrimary(), 0);
        advance().expect(type::paren_close);
        return tree;
    }
    case type::minus:
    {
        auto num = advance().expect(type::num);
        return std::make_shared<exprtree>(
            exprtree(token(type::num, -num.m_value, "", tok.m_startPos, token::getCombindedLen(tok, num))));
    }

    case type::identifier: return std::make_shared<exprtree>(exprtree(tok));
    default: throw std::logic_error("Invalid operation");
    }
}

parser::parser(std::vector<token>::iterator iterator) {
    m_iterator = iterator;
}

std::shared_ptr<exprtree> parser::parseStatement() {
    switch(peek().m_type) {
    case type::semicolon:
        advance();
        return nullptr;
    case type::keyword_let:
        return parseVariable();
    default:
        auto tree = parseExpression(parsePrimary(), 0);
        advance().expect(type::semicolon);
        return tree;
    }
}

std::shared_ptr<exprtree> parser::parseBlock() {
    auto block = std::make_shared<exprtree>(exprtree(token(type::block, 0, "", peek().m_startPos, -1)));

    // It's up to the parent function to decide weather or not EOF is fine.
    while(peek().m_type != type::curl_bracket_close && peek().m_type != type::eof) {
        auto tree = parseStatement();
        if(tree != nullptr) {
            block->m_tok.m_len = (tree->m_tok.m_startPos - block->m_tok.m_startPos) + tree->m_tok.m_len;
            block->subtrees.push_back(tree);
        }
    }

    return block;
}

std::shared_ptr<exprtree> parser::parseIdentifier() {
    return std::make_shared<exprtree>(exprtree(advance().expect(type::identifier)));
}

std::shared_ptr<exprtree> parser::parseVariable() {
    auto tree = std::make_shared<exprtree>(exprtree(advance().expect(type::keyword_let)));
    tree->subtrees.push_back(parseIdentifier()); // type
    advance().expect(type::colon);
    auto id = parseIdentifier();
    tree->subtrees.push_back(id);
    if(peek().m_type == type::equals) {
        tree->subtrees.push_back(parseExpression(id, 0));
    }

    advance().expect(type::semicolon);
    return tree;
}

std::shared_ptr<exprtree> parser::parse(std::vector<token>::iterator iterator) {
    return parser(iterator).parse();
}

std::shared_ptr<exprtree> parser::parse() {
    auto tree = parseBlock();
    peek().expect(type::eof); // parseBlock() potentally stops on a '}'.
    return tree;
}
