#pragma once
#include <exception>
#include <stdint.h>
#include <string>
class compilation_exception : std::exception {
public:
    compilation_exception(size_t startPos, size_t len);
    ~compilation_exception();
    virtual const std::string what(const std::string str) const = 0;
protected:
    static std::pair<size_t, size_t> getStringPos(const std::string str, const size_t index);
    std::string getPositionString(const std::string str) const;
    const size_t m_startPos;
    const size_t m_len;
private:
    static std::string getStringPosRaw(const std::string str, std::pair<size_t, size_t> position);
};
