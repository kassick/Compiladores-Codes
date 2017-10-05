/****************************************************************************
 *        Filename: "MMML/include/mmml/TypeRegistry.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 23:58:52 2017"
 *         Updated: "2017-10-05 11:13:43 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/Type.H"
#include <unordered_map>
#include <set>
#include <exception>
#include <iostream>

namespace mmml {

using namespace std;

extern int output;

class TypeRegistry {
  private:
    TypeRegistry() {}

    int next_id = 0;

    std::unordered_map<Type::id_type, Type::const_weak_pointer> registry;
    std::unordered_map<string, Type::const_pointer> name_registry;

  public:
    TypeRegistry(const TypeRegistry& other) = delete;
    TypeRegistry(TypeRegistry&& other) = delete;
    static TypeRegistry& instance() {
        static TypeRegistry _instance;

        return _instance;
    }

    Type::const_pointer add(Type::pointer& t) ;

    Type::const_pointer add(Type::pointer&& t) ;

    Type::const_pointer add(Type::pointer&& t, int id);

    Type::const_pointer find_by_type(Type::const_pointer) const;

    Type::const_pointer find_by_name(const string name) const;

    Type::const_pointer find_by_id(const int id) const;

    string to_string() const;

};

// Easy access to the singleton
static
TypeRegistry& type_registry = TypeRegistry::instance();

// end namespace mmml /////////////////////////////////////////////////////////
};
