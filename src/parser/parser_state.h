#ifndef EPICALYX_PARSER_STATE_H
#define EPICALYX_PARSER_STATE_H

#include "types/types.h"

#include <map>
#include <memory>
#include <string>
#include <stdexcept>

struct ParserState {
    /*
     * To be passed through the AST when checking types / values.
     * */
    std::vector<std::map<std::string, CTYPE>> ConstantScope = {{}};
    std::vector<std::map<std::string, CTYPE>> Scope = {{}};

    CType GetType(const std::string& name) const {
        for (auto t = Scope.rbegin(); t != Scope.rend(); t++) {
            if (t->contains(name)) {
                return *t->at(name);
            }
        }
        throw std::runtime_error("Unknown variable name: " + name);
    }

    void AddType(std::string name, CTYPE type) {
        auto& current_scope = Scope.back();
        current_scope.emplace(name, type);
    }

    void AddConstant(std::string name, CTYPE type) {
        Scope.back().emplace(name, type);
        ConstantScope.back().emplace(name, type);
    }

    bool IsConstant(const std::string& name) const {
        for (auto t = ConstantScope.rbegin(); t != ConstantScope.rend(); t++) {
            if (t->contains(name)) {
                return true;
            }
        }
        return false;
    }

//    bool GetBoolValue(const std::string& name) {
//
//    }
//
//    CType::OptionalNumericValue GetOptionalValue(const std::string& name) const {
//        return GetType(name).GetOptionalValue();
//    }
//
//    CType::NumericValue GetNumericValue(const std::string& name) const {
//        return GetType(name).GetNumericValue();
//    }
};

#endif //EPICALYX_PARSER_STATE_H
