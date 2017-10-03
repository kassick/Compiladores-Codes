/****************************************************************************
 *        Filename: "MMML/include/mmml/FunctionRegistry.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Oct  2 20:18:58 2017"
 *         Updated: "2017-10-02 22:34:00 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/TypeRegistry.H"
#include "mmml/Symbol.H"

#include <string>
#include <vector>
#include <set>
#include <unordered_map>

namespace mmml {

class Function {
  public:
    typedef std::shared_ptr<Function> pointer;
    typedef std::shared_ptr<const Function> const_pointer;
    typedef std::weak_ptr<Function> weak_pointer;
    typedef std::weak_ptr<const Function> const_weak_pointer;

    typedef std::vector<Symbol::const_pointer> ArgList;

    const string name;
    const ArgList args;
    const int def_line, def_col;
    int impl_line, impl_col;
    Type::const_pointer rtype;
    bool implemented = false;

    template <class GenericArgList>
    Function(const string name, GenericArgList&& args, const int line, const int col) :
            name(name),
            args(std::forward<GenericArgList>(args)),
            def_line(line), def_col(col)
    {}

    Function* set_implemented(int line, int col) {
        this->impl_line = line;
        this->impl_col = col;
        this->implemented = true;
        return this;
    }

    bool is_sane() const {
        std::set<string> name_set;

        for(const auto & arg: args) {
            if (!arg->type())
                return false;

            auto it = name_set.find(arg->name);
            if (it != name_set.cend())
                return false;

            name_set.insert(arg->name);
        }

        return true;
    }

};

class FunctionRegistry {
  public:
    std::unordered_map<string, Function::pointer> map;

    static FunctionRegistry& instance() {
        static FunctionRegistry registry;

        return registry;
    }


    Function::pointer find(const string name) const {
        auto it = map.find(name);
        if (it == map.cend())
            return nullptr;

        return it->second;
    }

    Function::pointer add(Function::pointer nf)
    {
        if (!nf)
            return nullptr;

        auto it = map.find(nf->name);
        if (it != map.end())
            return it->second;

        map[nf->name] = nf;

        return nf;
    }


  private:
    FunctionRegistry() {}
};

} // end namespace mmml ///////////////////////////////////////////////////////
