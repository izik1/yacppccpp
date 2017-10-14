#include "stdafx.h"
#include "compilation_exception.h"

compilation_exception::compilation_exception(size_t startPos, size_t len) : m_startPos(startPos), m_len(len) {}

compilation_exception::~compilation_exception() {}

std::pair<size_t, size_t> compilation_exception::getStringPos(const std::string str, const size_t index) {
    size_t line = 1;
    size_t column = 1;

    for(size_t i = 0; i < index; i++) {
        if(str.at(i) == '\n') {
            line++;
            column = 1;
        } else column++;
    }

    return std::make_pair(line, column);
}
