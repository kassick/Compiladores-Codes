/****************************************************************************
 *        Filename: "MMML/include/mmml/TypeRegistry.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 23:58:52 2017"
 *         Updated: "2017-09-28 01:34:44 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/Type.H"
#include <unordered_map>

namespace mmml {

class TypeRegistry {
  private:
    TypeRegistry() {}

    int next_id = 0;

    std::unordered_map<Type::id_type, Type::const_pointer> registry;
    std::unordered_map<string, Type::const_pointer> name_registry;

  public:
    TypeRegistry(const TypeRegistry& other) = delete;
    TypeRegistry(TypeRegistry&& other) = delete;
    static TypeRegistry& instance() {
        static TypeRegistry _instance;

        return _instance;
    }

    Type::const_pointer add(Type::pointer t) ;

    Type::const_pointer find_by_type(Type::const_pointer) const;

    Type::const_pointer find_by_name(const string name) const;

    Type::const_pointer find_by_id(const int id) const;


};


// end namespace mmml /////////////////////////////////////////////////////////
};
