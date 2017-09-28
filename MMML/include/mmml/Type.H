/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 20:55:04 2017"
 *         Updated: "2017-09-28 02:17:21 kassick"
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
    typedef int id_type;

    int _id;
    const string _name;
    const int _size;

    Type(const string name_, const int size_):
            _id(-1),
            _name(name_),
            _size(size_)
    {}

    int id() const {return _id;}
    void set_id(int id) { _id = id; }

    virtual string name() const {return _name;}
    virtual int size() const {return _size;}
    virtual string to_string() const {
        return string("Type ") +
                std::to_string(_id) +
                string(" -> ") +
                _name;
    }

    virtual bool equals(Type::pointer other) {
        Type* other_ptr = other.get();
        if (other_ptr == nullptr)
            return false;

        return typeid(*this) == typeid(other_ptr);
    }
};

// end namespace mmml /////////////////////////////////////////////////////////
}

namespace std {
static
string to_string(const mmml::Type &t) {
    return t.to_string();
}

// end namespace  /////////////////////////////////////////////////////////////
}
