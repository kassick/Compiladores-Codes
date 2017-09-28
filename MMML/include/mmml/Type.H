/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 20:55:04 2017"
 *         Updated: "2017-09-27 20:20:09 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#pragma once

#include <string>
#include <memory>

namespace mmml {

using namespace std;

struct Type : public enable_shared_from_this<Type>
{
    typedef std::shared_ptr<Type> pointer;
    typedef std::shared_ptr<const Type> const_pointer;
    typedef std::weak_ptr<Type> weak_pointer;
    typedef std::weak_ptr<const Type> const_weak_pointer;

    const int id;
    const string name;
    const int size;

    Type(const int id_, const string name_, const int size_):
            id(id_),
            name(name_),
            size(size_)
    {}
};

}

namespace std {
static
string to_string(const mmml::Type &t) {
    return string("Type ") +
           to_string(t.id) +
            string(" -> ") +
            t.name;
}
}
