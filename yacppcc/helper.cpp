#include "stdafx.h"
#include "helper.h"
#include "token.h"

std::string helper::getErrorLocation(const token errorTok) {
    return "{" + std::to_string(errorTok.m_startPos) + ", " + std::to_string(errorTok.m_len) + "}";
}
