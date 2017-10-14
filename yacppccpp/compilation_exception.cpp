#include "stdafx.h"
#include "compilation_exception.h"
#include <tuple>
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

std::string compilation_exception::getPositionString(const std::string str) const {
    return "@" + getStringPosRaw(str, getStringPos(str, m_startPos)) +
        (m_len == 1 ? "" : "-" + getStringPosRaw(str, getStringPos(str, m_startPos + m_len - 1)));
}

std::string compilation_exception::getStringPosRaw(const std::string str, std::pair<size_t, size_t> position) {
    size_t start_line;
    size_t start_column;
    std::tie(start_line, start_column) = position;
    return "(" + std::to_string(start_line) + ", " + std::to_string(start_column) + ")";
}
