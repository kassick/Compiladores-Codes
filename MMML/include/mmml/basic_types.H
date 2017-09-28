/****************************************************************************
 *        Filename: "MMML/include/mmml/basic_types.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Thu Sep 28 00:18:58 2017"
 *         Updated: "2017-09-28 02:25:23 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/TypeRegistry.H"
#include <sstream>
#include <vector>
#include <unordered_map>
#include <initializer_list>

namespace mmml {

class CharType : public Type
{
  public:
    CharType() : Type("char", 1) {}
};

class IntType : public Type
{
  public:
    IntType() : Type("int", 1) {}
};

class FloatType : public Type
{
  public:
    FloatType() : Type("float", 1) {}
};

class SequenceType : public Type
{
  public:
    typedef std::shared_ptr<SequenceType> pointer;
    typedef std::shared_ptr<const SequenceType> const_pointer;
    typedef std::weak_ptr<SequenceType> weak_pointer;
    typedef std::weak_ptr<const SequenceType> const_weak_pointer;

    Type::const_weak_pointer base_type, decayed_type;
    int arity;

    SequenceType(Type::const_pointer base_, int arity_ = 1) :
            Type(make_name(base_, arity_), 1)
    {
        auto seq_decay = dynamic_pointer_cast<const SequenceType>(base_);
        if (seq_decay) {
            this->base_type = seq_decay->base_type;
            this->decayed_type = seq_decay;
            this->arity = seq_decay->arity + 1;
        } else {
            this->base_type = base_;
            this->decayed_type = base_;
        }
    }

  private:
    static string make_name(Type::const_pointer base_, int arity)
    {
        std::stringstream name_buf;

        auto seq_decay = dynamic_pointer_cast<const SequenceType>(base_);
        if (seq_decay) {
            name_buf << seq_decay->name();
            arity = seq_decay->arity + 1;
        } else {
            name_buf << base_->name();
        }

        while (arity-- > 0)
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

    TupleType(base_types_vector_t v) :
            base_types(v),
            Type(make_name(v), 1)
    {}

    Type::const_pointer get_nth_type(int n) const {
        auto wp = this->base_types[n];
        return wp.lock();
    }

  private:

    static string make_name(const base_types_vector_t v)
    {
        std::stringstream name_buf;

        auto b = v.cbegin();
        name_buf << '{';
        auto t = (*b).lock();
        if (!t)
            return "ERROR ERROR ERROR";

        name_buf << t->name();

        while (++b != v.cend())
        {
            t = b->lock();
            if (!t)
                return "ERROR ERROR ERROR";

            name_buf << ", "
                     << t->name();
        }

        return name_buf.str();
    }
};

class ClassType : public Type
{
  public:
    typedef std::shared_ptr<ClassType> pointer;
    typedef std::shared_ptr<const ClassType> const_pointer;
    typedef std::weak_ptr<ClassType> weak_pointer;
    typedef std::weak_ptr<const ClassType> const_weak_pointer;

    struct Field {
        std::string name;
        Type::const_weak_pointer type;
    };

  public:

    ClassType(string custom_name,
              const std::vector<Field> fields) :
            Type(make_name(custom_name,
                           fields),
                 1)
    {
        TupleType::base_types_vector_t field_types(fields.size());

        int i = 0;
        for (const auto field : fields)
        {
            if (name_map.find(field.name) == name_map.end())
            {
                name_map[field.name] = i;
                field_types[i] = field.type.lock();
                i++;
            } else {
                // should never happen!
                // skip
            }
        }

        Type::pointer tup = make_shared<TupleType>(field_types);
        this->base_tuple =
                static_pointer_cast<const TupleType>(
                    TypeRegistry::instance().add(tup));
    }

    Type::const_pointer get_type(string name) const {
        auto it = name_map.find(name);
        if (it != name_map.cend())
            return base_tuple.lock()->get_nth_type(it->second);

        return nullptr;
    }

  private:
    TupleType::const_weak_pointer base_tuple;
    std::unordered_map<std::string, int> name_map;

    static string make_name(string custom_name,
                            const std::vector< Field> fields)
    {
        if (fields.size() == 0) {
            return "CAN NOT INSTANTIATE 0 SIZED CLASS";
        }

        std::stringstream name_buf;

        auto b = fields.cbegin();

        name_buf << custom_name
                 << " { "
                 << b->name
                 << " : ";

        auto btype = b->type.lock();
        if (!btype)
            name_buf << "ERROR ERROR ERROR";
        else
            name_buf << btype->name();

        while (++b != fields.cend())
        {
            name_buf << ", "
                     << b->name
                     << " : ";
            btype = b->type.lock();
            if (!btype)
                name_buf << "ERROR ERROR ERROR";
            else
                name_buf << btype->name();
        }

        return name_buf.str();
    }
};

void __attribute__ ((constructor)) init_basic_types()
{
    auto& registry = TypeRegistry::instance();
    registry.add(make_shared<CharType>());
    registry.add(make_shared<IntType>());
    registry.add(make_shared<FloatType>());
}

// end namespace mmml /////////////////////////////////////////////////////////
}
