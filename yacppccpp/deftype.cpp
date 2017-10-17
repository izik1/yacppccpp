#include "stdafx.h"
#include "deftype.h"
#include <unordered_set>

std::unordered_set<std::string> primSIntMap{"i8","i16","i32","i64"};

std::unordered_set<std::string> primUIntMap{"u8","u16","u32","u64"};

defType stringTodefType(const std::string str) {
    if(primSIntMap.find(str) != primSIntMap.end()) return defType::primSInt;
    if(primUIntMap.find(str) != primUIntMap.end()) return defType::primUInt;
    if(str == "bool") return defType::primBool;
    if(str == "void") return defType::primSpec;
    return defType::user;
}
