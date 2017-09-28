/****************************************************************************
 *        Filename: "MMML/TypeRegistry.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Thu Sep 28 00:12:18 2017"
 *         Updated: "2017-09-28 02:26:14 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/TypeRegistry.H"
using namespace mmml;

Type::const_pointer TypeRegistry::add(Type::pointer t)
{
    auto dup = find_by_type(t);
    if (dup)
        return dup;

    int id = next_id++;

    t->set_id(id);
    registry[id] = t;
    name_registry[t->name()] = t;

    return t;
}

Type::const_pointer
TypeRegistry::find_by_type(Type::const_pointer type)
        const
{
    auto it = name_registry.find(type->name());
    if (it == name_registry.cend())
        return nullptr;

    return it->second;
}

    Type::const_pointer
            TypeRegistry::find_by_name(const string name)
            const
    {
        for (const auto p: registry) {
            if (p.second->name() == name)
                return p.second;
        }

        return nullptr;
    }

    Type::const_pointer
            TypeRegistry::find_by_id(const int id)
            const
    {
        auto it = registry.find(id);
        if (it == registry.cend())
            return nullptr;

        return it->second;
    }
