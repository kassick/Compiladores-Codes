/****************************************************************************
 *        Filename: "StackVM/include/data.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 11:35:28 2017"
 *         Updated: "2017-09-19 04:06:12 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <cstring>

using namespace std;

enum DataType {
    UNINITIALIZED = -1,
    Char = 0,
    Int,
    Float,
    Double,
    Array,
};

const char * dataTypeNameStr(DataType t) {
    switch (t) {
        case Char: return "char"; break;
        case Int: return "int"; break;
        case Float: return "float"; break;
        case Double: return "double"; break;
        case Array: return "[]"; break;
        default: return "UNINITIALIZED"; break;
    }
}

struct DataItem {
    DataType type;
    vector<char> data;

    DataItem() {
        type = UNINITIALIZED;
    }

    template <typename T>
    DataItem(T v, DataType _type ) :
            type(_type),
            data(&v, &v + sizeof(T))
    {}

    DataItem(const char c) : DataItem(c, Char) {}

    DataItem(const int i) : DataItem(i, Int) {}

    DataItem(const float d) : DataItem(d, Float) {}

    DataItem(const double d) : DataItem(d, Double) {}

    DataItem(const std::vector<DataItem> &v) {
        vector<DataItem> * nv = new vector<DataItem>(v);
        type = Array;
        data.resize(sizeof(nv));
        memcpy(data.data(), &nv, sizeof(nv));
    }

    DataItem(std::vector<DataItem> &&v) {
        vector<DataItem> * nv = new vector<DataItem>(std::move( v ));
        type = Array;
        data.resize(sizeof(nv));
        memcpy(data.data(), &nv, sizeof(nv));
    }

    DataItem(const DataItem& other) :
            type(other.type),
            data(other.data)
    {
        if (other.type == Array) {
            const vector<DataItem> * const old_data = other.as<vector<DataItem>*>();
            vector<DataItem> * nv = new vector<DataItem>(*old_data);
            data.resize(sizeof(nv));
            memcpy(data.data(), &nv, sizeof(nv));
        }
    }

    ~DataItem() {
        if (type == Array) {
            delete this->as<vector<DataItem>*>();
        }
    }

    template <typename T>
    T as() const {
        if (sizeof(T) != data.size())
            throw logic_error("Trying to convert something of the wrong size");

        T tmp;
        memcpy(&tmp, data.data(), sizeof(tmp));

        return tmp;
    }

    template <typename T>
    T cast() const {
        switch(type) {
            case Char: return (T)this->as<char>(); break;
            case Int: return (T)this->as<int>(); break;
            case Float: return (T)this->as<float>(); break;
            case Double: return (T)this->as<double>(); break;
                //case Array: return (T)this->as<vector<DataItem>*>();

            default:
                throw logic_error("Invalid type in cast"); break;
        }

        throw logic_error("Invalid type in cast");
    }

    template <typename T>
    static DataItem createWithType(T i, DataType t) {
        switch (t) {
            case Char: return DataItem((char)i); break;
            case Int: return DataItem((int)i); break;
            case Float: return DataItem((float)i); break;
            case Double: return DataItem((double)i); break;
            default:
                throw logic_error("Invalid type in create_and_cast"); break;
        }
    }

    DataItem dataCastAs(DataType d) {
        if (d == this->type)
            return *this;

        switch(this->type) {
            case Char: return createWithType(this->as<char>(), d); break;
            case Int: return createWithType(this->as<int>(), d); break;
            case Float: return createWithType(this->as<float>(), d); break;
            case Double: return createWithType(this->as<double>(), d); break;
            default:
                throw logic_error("Invalid target type to dataCastAs"); break;
        }
    }

    bool isNull() const {
        return type == UNINITIALIZED;
    }

    bool isZero() const {
        // null is zero
        if (type == UNINITIALIZED)
            return true;

        // zero sized arrays are "zero"
        if (type == Array)
            return (as<vector<DataItem>*>()->size() == 0);

        // default: upcast and test for 0
        double tmp = cast<double>();

        return tmp == 0;
    }

    bool isPositive() const {
        // null is zero
        if (type == UNINITIALIZED)
            return false;

        // non-zero sized arrays are positive
        if (type == Array)
            return (as<vector<DataItem>*>()->size() > 0);

        // default: upcast and test for 0
        double tmp = cast<double>();
        return tmp > 0;
    }

    bool isNegative() const {
        // null is zero
        if (type == UNINITIALIZED)
            return false;

        // non-zero sized arrays are positive
        if (type == Array)
            return false;

        // default: upcast and test for 0
        double tmp = cast<double>();
        return tmp < 0;
    }

    string to_string() const {
        stringstream ss;

        ss << dataTypeNameStr(this->type)
           << " : ";
        switch (this->type) {
            case Char: ss << "'" << as<char>() << "'"; break;
            case Int: ss << as<int>(); break;
            case Float: ss << as<float>(); break;
            case Double: ss << as<double>(); break;
            case Array:
                {
                    ss << "[ ";
                    for (const DataItem& i : as<vector<DataItem>>())
                    {
                        ss << i.to_string() << " ";
                    }

                    ss << "]";
                }
                break;
            default: ss << "null"; break;
        }

        return ss.str();
    }

    string value_to_string() const {
        stringstream ss;

        switch (this->type) {
            case Char:   return std::to_string( as<char>() );
            case Int:    return std::to_string( as<int>() );
            case Float:  return std::to_string( as<float>() );
            case Double: return std::to_string( as<double>() );
            case Array:  return to_string();
            default:     return "null";
        }

        return ss.str();
    }
};
