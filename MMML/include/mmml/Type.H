/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 20:55:04 2017"
 *         Updated: "2017-10-02 21:50:56 kassick"
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
    virtual bool is_basic() const {return false;}

    // smart pointer cast:
    // t = registry.find_by_name(...).as<SequenceType>()
    template <class Target>
    using const_target= const typename std::decay<Target>::type;

    template <class Target>
    std::shared_ptr<const_target<Target>> as() const {
        return dynamic_pointer_cast<const_target<Target>>(shared_from_this());
    }


    virtual string to_string() const {
        return string("Type ") +
                std::to_string(_id) +
                string(" -> ") +
                name();
    }

    virtual bool equals(Type::const_pointer other) const {
        const Type* other_ptr = other.get();
        if (other_ptr == nullptr)
            return false;

        return typeid(*this) == typeid(*other_ptr);
    }

    virtual
    size_t hash() const {
        return std::hash<string>()(_name);
    }

    virtual
    bool equal_name(Type::const_pointer other) const
    {
        if (!other)
            return false;

        return this->name() == other->name();
    }

    virtual
    bool equivalent(Type::const_pointer other) const
    {
        return equals(other);
    }

    virtual
    bool equal_storage(Type::const_pointer other) const
    {
        return equals(other);
    }
};

// external accessors
static inline
const string type_name(const Type& t) {
    return t.name();
}

static inline
const string type_name(const Type::const_pointer & t) {
    if (!t)
        return "ERROR NULL TYPE ON TYPE_NAME";
    return t->name();
}

static inline
const string type_name(const Type::const_weak_pointer & t) {
    auto ptr = t.lock();
    if (!ptr)
        return "ERROR NULL TYPE ON TYPE_NAME";
    return ptr->name();
}

// end namespace mmml /////////////////////////////////////////////////////////
}

namespace std {
static inline
string to_string(const mmml::Type &t) {
    return t.to_string();
}

// template <>
// struct hash<mmml::Type>
// {
//     size_t operator()(const mmml::Type & t) const {
//         return t.hash();
//     }
// };

// end namespace  /////////////////////////////////////////////////////////////
}
