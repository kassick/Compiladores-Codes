/****************************************************************************
 *        Filename: "MMML/TypeRegistry.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Thu Sep 28 00:12:18 2017"
 *         Updated: "2017-09-29 02:41:49 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/TypeRegistry.H"

#include <iostream>
#include <sstream>
using namespace std;
using namespace mmml;

Type::const_pointer
TypeRegistry::add(Type::pointer&& t)
{
    auto dup_it = name_registry.find(t->name());
    if (dup_it != name_registry.end())
    {
        if (output)
            cout << "found equal at " << dup_it->second->to_string() << endl;

        t.reset();
        return dup_it->second;
    }

    int id = next_id++;

    if (output)
        cout << "Adding2 " << t->to_string() << " with id " << id << endl;

    t->set_id(id);
    name_registry[t->name()] = t;
    registry[id] = t;

    return t;
}

Type::const_pointer
TypeRegistry::add(Type::pointer& t)
{
    auto dup_it = name_registry.find(t->name());
    if (dup_it != name_registry.end())
    {
        if (output)
            cout << "found equal at " << dup_it->second->to_string() << endl;

        t.reset();
        return dup_it->second;
    }

    int id = next_id++;

    if (output)
        cout << "Adding " << t->to_string() << " with id " << id << endl;

    t->set_id(id);
    name_registry[t->name()] = t;
    registry[id] = t;

    return t;
}

Type::const_pointer
TypeRegistry::find_by_type(Type::const_pointer type)
        const
{
    auto it = name_registry.find(type->name());
    if (it != name_registry.cend())
        return it->second;

    return nullptr;
}

Type::const_pointer
TypeRegistry::find_by_name(const string name)
        const
    {
        auto it = name_registry.find(name);
        if (it != name_registry.cend())
            return it->second;

        return nullptr;
    }

Type::const_pointer
TypeRegistry::find_by_id(const int id)
        const
{
    auto it = registry.find(id);
    if (it == registry.cend())
        return nullptr;

    return it->second.lock();
}

string TypeRegistry::to_string() const
{
    std::stringstream s;
    s << endl;

    for (const auto &p: name_registry)
    {
        s << "Name: " << p.first << " -> " << p.second->to_string() << endl;
    }

    for (const auto &p : registry)
        if (p.second.lock())
            s << "Id: " << p.first << " -> " << p.second.lock()->name() << endl;

    return s.str();
}

int mmml::output = 0;
