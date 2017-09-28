/****************************************************************************
 *        Filename: "MMML/include/Symbol.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 21:51:48 2017"
 *         Updated: "2017-09-27 20:36:53 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#pragma once

#include <string>
#include <memory>

#include "mmml/Type.H"

namespace mmml {

using namespace std;

class Symbol : public std::enable_shared_from_this<Symbol>
{
  public:
    typedef std::shared_ptr<Symbol> pointer;
    typedef std::shared_ptr<const Symbol> const_pointer;
    typedef string name_type;

    const name_type name;
    const int line, col;

    int pos; // is set when this symbol is added to a symbol table

    Symbol(const string name_, Type::const_pointer type_, int line_, int col_) :
            name(name_),
            _type(type_),
            line(line_),
            col(col_)
    {}

    string to_string() const ;

    int size() const;

    Type::const_pointer type() const;

  private:
    Type::const_weak_pointer _type;
};


}

namespace std {
string to_string(const mmml::Symbol &);
}
