#include "stdafx.h"
#include <unordered_map>
#include <iterator>
#include <cassert>
#include "exprtree.h"
#include "parser.h"
#include "token.h"

// larger number = higher precedence.
std::unordered_map<type, size_t> precedence = {
    {type::keyword_as, 1000},
    {type::plus, 150},
    {type::minus, 150},
    {type::greater_than, 100},
    {type::less_than, 100},
    {type::equals, 0},
    {type::slash, 200},
    {type::carrot, 300},
    {type::astrisk, 200},
    {type::equals_equals, 50},
    {type::astrisk_equals, 50},
    {type::slash_equals, 50},
    {type::not_equals, 50},
    {type::plus_equals, 50},
    {type::minus_equals, 50},
};
std::unordered_map<type, bool> isRightAssositve = {
    {type::keyword_as, false},
    {type::plus, false},
    {type::minus, false},
    {type::slash, false},
    {type::carrot, true},
    {type::equals, true},
    {type::astrisk, false},
    {type::plus_equals, false},
    {type::minus_equals, false},
    {type::equals_equals, false},
    {type::astrisk_equals, false},
    {type::slash_equals, false},
    {type::not_equals, false},
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
    return precedence.at(a.m_type) > precedence.at(b.m_type) ||
        (precedence.at(a.m_type) == precedence.at(b.m_type) && isRightAssositve.at(a.m_type));
}

std::shared_ptr<exprtree> parser::parseExpression(std::shared_ptr<exprtree> lhs, size_t minPrecidence) {
    auto lookahead = peek();
    while(isBinaryOp(lookahead.m_type) && precedence.at(lookahead.m_type) >= minPrecidence) {
        auto op = lookahead;
        advance();

        auto rhs = parsePrimary();
        lookahead = peek();

        while(isBinaryOp(lookahead.m_type) && precidenceCompare(lookahead, op)) {
            rhs = parseExpression(rhs, precedence.at(lookahead.m_type));
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

std::shared_ptr<exprtree> parser::parseUrnary(const token &tok) {
    auto tree = std::make_shared<exprtree>(exprtree(token(tok)));
    tree->subtrees.push_back(parsePrimary());
    return tree;
}

std::shared_ptr<exprtree> parser::parsePrimary() {
    auto tok = advance();
    switch(tok.m_type) {
    case type::num: return std::make_shared<exprtree>(exprtree(tok));

    case type::paren_open:
    {
        auto tree = parseExpression(parsePrimary(), 0);

        advance().expect(type::paren_close);

        return tree;
    }

    case type::identifier:
    {
        auto tree = std::make_shared<exprtree>(exprtree(tok));
        return peek().m_type == type::paren_open ? parsefnCall(tree) : tree;
    }
    default:
        if(isUnaryOp(tok.m_type)) return parseUrnary(tok);
        else throw std::logic_error("Unexpected token type for primary: " + typeStringMap[tok.m_type]);
    }
}

std::shared_ptr<exprtree> parser::parsefnCall(std::shared_ptr<exprtree> tree) {
    auto fnCall = std::make_shared<exprtree>(exprtree(token(type::call, 0, "", tree->m_tok.m_startPos, 0)));
    fnCall->subtrees.push_back(tree);

    auto tok = advance();

    if(peek().m_type != type::paren_close) {
        do {
            fnCall->subtrees.push_back(parseExpression(parsePrimary(), 0));
            tok = advance();
        } while(tok.m_type == type::comma);

        fnCall->m_tok.m_len = token::getCombindedLen(fnCall->m_tok, tok.expect(type::paren_close));
    } else advance();

    return fnCall;
}

parser::parser(std::vector<token>::iterator iterator) {
    m_iterator = iterator;
}

std::shared_ptr<exprtree> parser::parseFunction() {
    auto tree = std::make_shared<exprtree>(exprtree(advance()));

    auto _tree = parseIdentifier(); // This is the return type if the next token is an identifier. otherwise it's the name.

    if(peek().m_type == type::identifier) tree->subtrees.push_back(parseIdentifier()); // Name if existant.
    else {
        tree->subtrees.push_back(_tree);
        _tree = nullptr;
    }

    auto fn_args = std::make_shared<exprtree>(exprtree(token(type::fn_args, 0, "", tree->m_tok.m_startPos, 0)));
    tree->subtrees.push_back(fn_args);

    advance().expect(type::paren_open);
    if(peek().m_type == type::identifier) while(parseArgument(fn_args)) advance().expect(type::comma);

    fn_args->m_tok.m_len = token::getCombindedLen(fn_args->m_tok, advance().expect(type::paren_close));
    advance().expect(type::curl_bracket_open);

    tree->subtrees.push_back(parseBlock());
    tree->m_tok.m_len = token::getCombindedLen(tree->m_tok, advance().expect(type::curl_bracket_close));

    if(_tree != nullptr) tree->subtrees.push_back(_tree);
    return tree;
}

std::shared_ptr<exprtree> parser::parseStatement() {
    switch(peek().m_type) {
    case type::semicolon: return std::make_shared<exprtree>(exprtree(advance()));
    case type::keyword_let: return parseVariable();

    case type::keyword_while:
    case type::keyword_until:
    {
        auto tree = std::make_shared<exprtree>(exprtree(advance()));
        peek().expect(type::paren_open);

        tree->subtrees.push_back(parseExpression(parsePrimary(), 0)); // condition.
        tree->subtrees.push_back(parseStatement());

        return tree;
    }

    case type::keyword_if: return parseIf();

    case type::curl_bracket_open:
    {
        advance();
        auto tree = parseBlock();
        advance().expect(type::curl_bracket_close);
        return tree;
    }

    case type::keyword_fn: return parseFunction();

    case type::keyword_ret:
    {
        auto tree = std::make_shared<exprtree>(exprtree(advance()));
        if(isPrimary(peek().m_type))  tree->subtrees.push_back(parseExpression(parsePrimary(), 0));
        tree->m_tok.m_len = token::getCombindedLen(tree->m_tok, advance().expect(type::semicolon));
        return tree;
    }
    default:
        auto tree = parseExpression(parsePrimary(), 0);
        tree->m_tok.m_len = token::getCombindedLen(tree->m_tok, advance().expect(type::semicolon));
        return tree;
    }
}

std::shared_ptr<exprtree> parser::parseIf() {
    auto tree = std::make_shared<exprtree>(exprtree(advance()));
    peek().expect(type::paren_open);

    tree->subtrees.push_back(parseExpression(parsePrimary(), 0)); // condition.
    auto brTrue = parseStatement();

    tree->subtrees.push_back(brTrue);

    if(peek().m_type == type::keyword_else) {
        advance();

        auto brElse = parseStatement();

        tree->subtrees.push_back(brElse);
        tree->m_tok.m_len = token::getCombindedLen(tree->m_tok, brElse->m_tok);
    } else tree->m_tok.m_len = (brTrue->m_tok.m_startPos - tree->m_tok.m_startPos) + brTrue->m_tok.m_len;

    return tree;
}

bool parser::parseArgument(std::shared_ptr<exprtree> appendTree) {
    appendTree->subtrees.push_back(parseIdentifier()); // type
    appendTree->subtrees.push_back(parseIdentifier()); // identifier.

    return peek().m_type == type::comma;
}

std::shared_ptr<exprtree> parser::parseBlock() {
    auto block = std::make_shared<exprtree>(exprtree(token(type::block, 0, "", peek().m_startPos, -1)));

    while(peek().m_type != type::curl_bracket_close && peek().m_type != type::eof) {
        auto tree = parseStatement();
        if(tree != nullptr) block->subtrees.push_back(tree);
    }

    block->m_tok.m_len = token::getCombindedLen(block->m_tok, peek());

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

    if(peek().m_type == type::equals) tree->subtrees.push_back(parseExpression(id, 0));

    tree->m_tok.m_len = token::getCombindedLen(tree->m_tok, advance().expect(type::semicolon));

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
