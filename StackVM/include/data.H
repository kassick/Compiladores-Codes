/****************************************************************************
 *        Filename: "StackVM/include/data.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 11:35:28 2017"
 *         Updated: "2017-10-23 12:08:36 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <cstring>
#include <utility>

using namespace std;

enum DataType {
    UNINITIALIZED = -1,
    Char = 0,
    Int,
    Double,
    Array,
};

const char * dataTypeNameStr(DataType t) {
    switch (t) {
        case Char: return "char"; break;
        case Int: return "int"; break;
        case Double: return "double"; break;
        case Array: return "[]"; break;
        default: return "UNINITIALIZED"; break;
    }
}

const string char_escape(const char c) {
    switch(c) {
        case '\n': return "\\n";
        case '\t': return "\\t";
        case '\r': return "\\r";
        case '\b': return "\\b";
        case '\'': return "\\'" ;
        case '\\': return "\\\\";
        default:
            return string({c});
    }
}

const string str_escape(const string & s)
{
    stringstream sout;
    for (int i = 0 ; i < s.length() ; i++)
    {
        switch(s[i]) {
            case '\n': sout << "\\n"; break;
            case '\t': sout << "\\t"; break;
            case '\r': sout << "\\r"; break;
            case '\b': sout << "\\b"; break;
            case '"': sout <<  "\\\"" ; break;
            case '\\': sout << "\\\\";break;
            default:
                sout << s[i];
        }
    }

    return sout.str();
}

const string str_unescape(const string & s)
{
    stringstream sout;
    for (int i = 0 ; i < s.length() ; i++)
    {
        //cerr << "s[" << i << "]: " << s[i] << endl;
        if (s[i] == '\\') {
            // escape!
            if (i + 1 < s.length()) {
                switch(s[i + 1]) {
                    case 'n': sout << '\n'; break;
                    case 't': sout << '\t'; break;
                    case 'r': sout << '\r'; break;
                    case 'b': sout << '\b'; break;
                    case '"': sout << '"' ; break;
                    case '\\': sout << '\\'; break;
                    default:
                        // cerr << "s[i]: " << s[i]
                        //      << " s[i+1]: " << s[i+1]
                        //      << endl;
                        // oops, unknown escape
                        sout << "(UNKNOWN ESCAPE " << s[i+1] << ")";
                }
                i++;
            }
        } else {
            sout << s[i];
        }
    }
    return sout.str();
}

struct DataItem {
    DataType type;
    vector<char> data;

    DataItem() {
        type = UNINITIALIZED;
    }

    template <typename T>
    DataItem(T v, DataType _type ) :
            type(_type)
    {
        data.resize(sizeof(T));
        memcpy(data.data(), &v, sizeof(T));
    }

    DataItem(const char c) : DataItem(c, Char) {}

    DataItem(const int i) : DataItem(i, Int) {}

    DataItem(const double d) : DataItem(d, Double) {}

    DataItem(const std::vector<DataItem> &v) {
        // cerr << "copy ctor from "
        //      << (void*)&v
        //      << " to "
        //      << (void*) this
        //      << endl;
        vector<DataItem> * nv = new vector<DataItem>(v);
        type = Array;
        data.resize(sizeof(nv));
        memcpy(data.data(), &nv, sizeof(nv));
    }

    DataItem(std::vector<DataItem> &&other) {
        // cerr << "move ctor from "
        //      << (void*)&other
        //      << " to "
        //      << (void*) this
        //      << endl;
        vector<DataItem> * nv = new vector<DataItem>(std::move( other ));
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

            // cerr << "!copy ctor from "
            //      << (void*)&other
            //      << " to "
            //      << (void*) this
            //      << endl;
        }
    }

    DataItem(DataItem&& other) :
            type(other.type),
            data(std::move(other.data))
    {
        other.type = UNINITIALIZED;
        other.data.resize(0);
    }

    void swap(DataItem & other) {
        std::swap(this->type, other.type);
        std::swap(this->data, other.data);
    }

    DataItem& operator=(const DataItem& other) = delete;
    DataItem& operator=(DataItem&& other)
    {
        if (this->type == Array) {
            // cerr << "Eliminating an array at "
            //      << (void*) this->as<std::vector<DataItem>*>()
            //      << endl;
            delete this->as<std::vector<DataItem>*>();
            this->data.resize(0);
        }

        this->data = std::move(other.data);
        this->type = other.type;

        other.type = UNINITIALIZED;
        other.data.resize(0);

        return *this;
    }

    ~DataItem() {
        if (type == Array) {
            // cerr << "Deleting array for item "
            //      << (void*)this
            //      << " array at "
            //      << (void*)this->data.data()
            //      << endl;
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

        //ss << "("
        //   << dataTypeNameStr(this->type)
        //   << " ";
        switch (this->type) {
            case Char: ss << "'" << char_escape(as<char>()) << "'"; break;
            case Int: ss << as<int>(); break;
            case Double: ss << std::setprecision(10)
                            << std::fixed
                            << as<double>();
                break;
            case Array:
                {
                    // 0x8fafbd {3} [(int 1) (double 2) (null)]
                    //ss //<< (void*) data.data()
                            //<< " {" << as<vector<DataItem>*>()->size() << "}"

                    ss << " [ ";

                    for (const DataItem& i : *as<vector<DataItem>*>())
                    {
                        ss << i.to_string()
                           << ","
                           << " ";
                    }

                    ss << "]";
                }
                break;
            default: ss << "null"; break;
        }
        //ss << ")";

        return ss.str();
    }

    string value_to_string() const {
        stringstream ss;

        switch (this->type) {
            case Char:   return std::string( { as<char>() } );
            case Int:    return std::to_string( as<int>() );
            case Double: return std::to_string( as<double>() );
            case Array:  return to_string();
            default:     return "null";
        }

        return ss.str();
    }
};

namespace std {
void swap(DataItem &a, DataItem &b) {
    a.swap(b);
}
}
