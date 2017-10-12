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

class codetype {
public:
    const std::string m_name;
    std::pair<std::string, codetype> make_pair() const;
    llvm::Type* getLlvmType() const;
    std::unordered_map<codetype*, llvm::Function*> casts;
    std::vector<std::tuple<type, function*>> ops;
    function* lookupOp(type t, std::vector<codetype*> args);
    codetype(llvm::Type* p_type, const std::string p_name);
    bool operator==(codetype &other);
    ~codetype();
private:
    llvm::Type* m_llvmType;
};
