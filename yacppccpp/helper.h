#pragma once
#include <string>

class token;
namespace helper {
    std::string getErrorLocation(const token errorTok);
}
