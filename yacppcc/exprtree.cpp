#include "stdafx.h"
#include "exprtree.h"

exprtree::exprtree() : m_tok(type::eof, 0, "", -1, -1) {
    subtrees = std::vector<std::shared_ptr<exprtree>>();
}

exprtree::exprtree(const token p_tok) : m_tok(p_tok) {
    subtrees = std::vector<std::shared_ptr<exprtree>>();
}

exprtree::~exprtree() {}
