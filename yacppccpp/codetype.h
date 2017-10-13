#pragma once
#include "llvm/IR/Type.h"
#include <string>
#include <memory>
#include <utility>
#include <unordered_map>
#include <vector>
enum class type;
class function;
namespace llvm {
    class Function;
}

enum class defType {
    user,
    primSInt,
    primUInt,
    primFP,
    primBool,
};

class codetype {
public:
    const std::string m_name;
    const defType m_defType;
    bool isPrimitive();
    llvm::Type* getLlvmType() const;
    std::unordered_map<std::shared_ptr<codetype>, llvm::Function*> casts;
    std::vector<std::tuple<type, function*>> ops;
    function* lookupOp(type t, std::vector<std::shared_ptr<codetype>> args);
    codetype(llvm::Type* p_type, const std::string p_name, defType p_defType);
    bool operator==(codetype &other);
    ~codetype();
private:
    llvm::Type* m_llvmType;
};
