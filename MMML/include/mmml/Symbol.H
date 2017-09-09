/****************************************************************************
 *        Filename: "MMML/include/Symbol.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 21:51:48 2017"
 *         Updated: "2017-09-08 22:04:16 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#pragma once

#include <string>

#include "mmml/Type.H"

namespace mmml {

using namespace std;

struct Symbol
{
    string name;
    Type type;

    int offset;

    int line, col;

    string to_string() const ;

    int size() const;

    Symbol(const string name_, const Type type_, int line_, int col_) :
            name(name_),
            type(type_),
            line(line_), col(col_)
    {}
};


}

namespace std {
string to_string(const mmml::Symbol &);
}
