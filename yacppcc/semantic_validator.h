#pragma once;
#include <memory>
#include <vector>
class exprtree;
namespace semantic_validator {
    std::vector<std::shared_ptr<exprtree>> validate(const exprtree &tree);
}
