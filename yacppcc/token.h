#pragma once
#include <cassert>
#include <cstdint>
#include "type.h"
class token {
public:
    type m_type;
    int64_t m_value;
    size_t m_startPos;
    size_t m_len;
    std::string m_strval;
    static inline size_t getCombindedLen(const token tokA, const token tokB) {
        return (tokB.m_startPos - tokA.m_startPos) + tokB.m_len;
    }

    token(
        const type p_type,
        const int64_t p_value,
        const std::string p_strval,
        const size_t p_startPos,
        const size_t p_len);
    token expect(const type p_type) const;
    static token eof(const size_t p_fileLen);
};
