#pragma once
#include <string>
enum class defType {
    user,
    primSInt,
    primUInt,
    primFP,
    primBool,
    primSpec,
};

defType stringTodefType(const std::string str);
