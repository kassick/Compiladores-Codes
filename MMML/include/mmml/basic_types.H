/****************************************************************************
 *        Filename: "MMML/include/mmml/basic_types.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Thu Sep 28 00:18:58 2017"
 *         Updated: "2017-09-29 23:07:33 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/TypeRegistry.H"
#include <sstream>
#include <vector>
#include <iterator>
#include <unordered_map>
#include <initializer_list>
#include <iostream>

namespace mmml {

class BasicType : public Type
{
  public:
    BasicType(const string name, const int size)
            : Type(name, size)
    {}

    virtual bool is_basic() const override
    {
        return true;
    }
};

class CharType : public BasicType
{
  public:
    CharType() : BasicType("char", 1) {}
};

class IntType : public BasicType
{
  public:
    IntType() : BasicType("int", 1) {}
};

class FloatType : public BasicType
{
  public:
    FloatType() : BasicType("float", 1) {}
};

class NilType : public BasicType
{
  public:
    NilType() : BasicType("nil", 1) {}
};

class BoolType : public BasicType
{
  public:
    BoolType() : BasicType("bool", 1) {}
};

class SequenceType : public Type
{
  public:
    typedef std::shared_ptr<SequenceType> pointer;
    typedef std::shared_ptr<const SequenceType> const_pointer;
    typedef std::weak_ptr<SequenceType> weak_pointer;
    typedef std::weak_ptr<const SequenceType> const_weak_pointer;

    Type::const_weak_pointer base_type, decayed_type;
    int dim;

    SequenceType(Type::const_pointer base_, int dim_ = 1) :
            Type(make_name(base_, dim_), 1),
            dim(dim_)
    {
        auto base_sequence = base_->as<SequenceType>();
        if (base_sequence) {
            this->base_type = base_sequence->base_type;
            this->decayed_type = base_sequence;
            this->dim = base_sequence->dim + 1;
        } else {
            this->base_type = base_;
            this->decayed_type = base_;
            this->dim = 1;
        }
    }

    virtual
    bool equals(Type::const_pointer other) const
            override
    {
        auto other_seq = dynamic_pointer_cast<const SequenceType>(other);
        if (!other_seq)
            return false;

        if (this->dim != other_seq->dim)
            return false;

        auto this_base_type = this->base_type.lock();
        auto other_base_type = other_seq->base_type.lock();

        return this_base_type->equals(other_base_type);
    }

  private:
    static string make_name(Type::const_pointer base_, int dim)
    {
        std::stringstream name_buf;
        Type::const_pointer base_type;

        auto base_sequence = base_->as<SequenceType>();

        if (base_sequence) {
            base_type = base_sequence->base_type.lock();
            dim = base_sequence->dim + 1;
        } else {
            base_type = base_;
            dim = 1;
        }

        if (!base_type)
            name_buf << "INVALID BASE";

        name_buf << base_type->name();

        while (dim-- > 0)
            name_buf << "[]";

        return name_buf.str();
    }
};

class TupleType : public Type
{
  public:
    typedef std::vector<Type::const_weak_pointer> base_types_vector_t;

    typedef std::shared_ptr<TupleType> pointer;
    typedef std::shared_ptr<const TupleType> const_pointer;
    typedef std::weak_ptr<TupleType> weak_pointer;
    typedef std::weak_ptr<const TupleType> const_weak_pointer;

    base_types_vector_t base_types;

    template <class T>
    TupleType(std::initializer_list<T> l) :
            base_types(l),
            Type(make_name(std::begin(l), end(l)), 1)
    {}
    template <class T>
    TupleType(T&& v) :
            base_types(std::forward<T>(v)),
            Type(make_name(std::begin(v), end(v)), 1)
    {}

    // TupleType(std::vector<Type::const_pointer> v):
    //         base_types(make_weak_vector(v.begin(), v.end())),
    //         Type(make_name(make_weak_vector(v.begin(), v.end())), 1)
    // {}

    Type::const_pointer get_nth_type(int n) const {
        auto wp = this->base_types[n];
        return wp.lock();
    }

    virtual
    bool equals(Type::const_pointer other) const
    {
        auto other_tuple = dynamic_pointer_cast<const TupleType>(other);
        if (!other_tuple)
            return false;

        if (this->base_types.size() != other_tuple->base_types.size())
            return false;

        auto lit = this->base_types.cbegin();
        auto rit = other_tuple->base_types.cbegin();

        while(lit != this->base_types.cend())
        {
            auto lptr = lit->lock();
            auto rptr = rit->lock();

            if (!lptr || !rptr || !lptr->equals(rptr))
                return false;

            lit++;
            rit++;
        }

        return true;
    }

  private:

    // template <typename ForwardIt>
    // static base_types_vector_t make_weak_vector(ForwardIt begin, ForwardIt end)
    // {
    //     base_types_vector_t ret(end - begin);
    //     auto ret_it = ret.begin();

    //     while (begin != end)
    //         *(ret_it++) = *begin++;

    // }

    template <class ForwardIt>
    static string make_name(ForwardIt begin, ForwardIt end)
    {
        int i = 0;
        std::stringstream name_buf;

        if (begin == end)
            return "INVALID TUPLE";

        name_buf << "{ ";
        while (begin != end) {

            name_buf << "@" << i << " "
                     << type_name(*begin);
            begin++;
            i++;

            if (begin != end)
                name_buf << ", ";

        }

        name_buf << " }";

        return name_buf.str();
    }
};

struct ClassField {
    std::string name;
    Type::const_weak_pointer type;
};

struct class_field_name {
    string
    operator()(const ClassField& f) const {
        return f.name;
    }
};

struct class_field_type {
    Type::const_weak_pointer
    operator()(const ClassField& f) {
        return f.type;
    }
};

class ClassType : public Type
{
  public:
    typedef std::shared_ptr<ClassType> pointer;
    typedef std::shared_ptr<const ClassType> const_pointer;
    typedef std::weak_ptr<ClassType> weak_pointer;
    typedef std::weak_ptr<const ClassType> const_weak_pointer;

    typedef std::vector<ClassField> base_field_vector_t;

  public:

    template <class T,
              class GetName=class_field_name,
              class GetType=class_field_type>
    ClassType(string custom_name, T v,
              GetName get_name=GetName(),
              GetType get_type=GetType())
            :
            Type(custom_name, 1),
            custom_name(custom_name)
    {
        TupleType::base_types_vector_t field_types(v.size());

        int i = 0;
        for (const auto field : v)
        {
            auto name = get_name(field);
            if (name_map.find(name) == name_map.end())
            {
                name_map[name] = i;
                field_types[i] = get_type(field).lock();
                i++;
            } else {
                // should never happen!
                // skip
            }
        }

        Type::pointer tup = make_shared<TupleType>(field_types);
        this->base_tuple = static_pointer_cast<const TupleType>(TypeRegistry::instance().add(tup)) ;
    }

    Type::const_pointer get_type(string name) const {
        auto it = name_map.find(name);
        if (it != name_map.cend())
            return base_tuple.lock()->get_nth_type(it->second);

        return nullptr;
    }

    const string custom_name;

    virtual
    bool equals(Type::const_pointer other)
            const
            override
    {
        #ifdef CLASS_EQUALS_IS_EQUIV
        return equivalent(other);
        #else
        return equal_name(other);
        #endif
    }

    virtual bool equal_name(Type::const_pointer other) const override
    {
        auto other_class = dynamic_pointer_cast<const ClassType>(other);
        if (!other_class)
            return false;

        return (this->custom_name == other_class->custom_name);
    }

    virtual bool equivalent(Type::const_pointer other) const override
    {
        // true if both are classes, both have the same storage
        auto other_class = dynamic_pointer_cast<const ClassType>(other);
        if (!other_class)
            return false;

        auto this_tuple = this->base_tuple.lock();
        if (!this_tuple)
            return false;

        auto other_class_tuple = other_class->base_tuple.lock();

        return other_class_tuple && this_tuple->equals(other_class_tuple);
    }

    virtual bool equal_storage(Type::const_pointer other) const override
    {
        auto this_tuple = this->base_tuple.lock();
        if (!this_tuple)
            return false;

        // Class / Tuple:  Same Storage -> equivalent
        auto other_tuple = dynamic_pointer_cast<const TupleType>(other);
        if (other_tuple)
        {
            return this_tuple->equals(other_tuple);
        }

        // Class / Class : same base tuples, ok
        auto other_class = dynamic_pointer_cast<const ClassType>(other);
        if (!other_class)
            return false;

        auto other_class_tuple = other_class->base_tuple.lock();

        return other_class_tuple && this_tuple->equals(other_class_tuple);
    }

    virtual
    string to_string() const override
    {
        auto this_tuple = base_tuple.lock();
        if (!this_tuple)
            return "ERROR NULL TUPLE";

        std::stringstream name_buf;

        name_buf << Type::to_string()
                 << " { ";

        auto begin = name_map.cbegin();
        while(begin != name_map.cend()) {
            int nth = begin->second;
            name_buf << begin->first
                     << " : "<< "@" << nth << " "
                     << mmml::type_name(this_tuple->get_nth_type(nth));

            begin++;

            if(begin != name_map.cend())
                name_buf << ", ";

        }

        name_buf << " }";

        return name_buf.str();
    }

  private:
    TupleType::const_weak_pointer base_tuple;
    std::unordered_map<std::string, int> name_map;
};

void __attribute__ ((constructor)) init_basic_types();


// end namespace mmml /////////////////////////////////////////////////////////
}
