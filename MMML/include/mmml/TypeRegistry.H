/****************************************************************************
 *        Filename: "MMML/include/mmml/TypeRegistry.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 23:58:52 2017"
 *         Updated: "2017-09-28 21:36:47 kassick"
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

namespace mmml {

using namespace std;

struct hash_type_ptr
{
    size_t operator()(const Type& t) const
    {
        return t.hash();
    }
};

struct TypePtrCmpLess {
    bool operator()(const Type::const_pointer& ptr1, const Type::const_pointer& ptr2) const
    {
        if (ptr1->equals(ptr2))
            return false;

        return ptr1->id() < ptr2->id();
    }
};

class TypeRegistry {
  private:
    TypeRegistry() {}

    int next_id = 0;

    // Ownership is on the set
    std::set<Type::const_pointer, TypePtrCmpLess> type_storage_set;

    // These just hold non-onwing pointers to the types stored
    std::unordered_map<Type::id_type, Type::const_weak_pointer> registry;
    std::unordered_map<string, Type::const_weak_pointer> name_registry;

  public:
    TypeRegistry(const TypeRegistry& other) = delete;
    TypeRegistry(TypeRegistry&& other) = delete;
    static TypeRegistry& instance() {
        static TypeRegistry _instance;

        return _instance;
    }

    Type::const_pointer add(Type::pointer& t) ;

    Type::const_pointer add(Type::pointer&& t) ;

    Type::const_pointer find_by_type(Type::const_pointer) const;

    Type::const_pointer find_by_name(const string name) const;

    Type::const_pointer find_by_id(const int id) const;


};

// end namespace mmml /////////////////////////////////////////////////////////
};
