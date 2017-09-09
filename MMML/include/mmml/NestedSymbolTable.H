/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 20:55:04 2017"
 *         Updated: "2017-09-08 22:02:40 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include "mmml/Symbol.H"

namespace mmml {

using namespace std;

class NestedSymbolTable : public enable_shared_from_this<NestedSymbolTable>
{
    typedef shared_ptr<NestedSymbolTable> pointer;

    int offset;
    int size;
    unordered_map<string, Symbol> data;

    NestedSymbolTable::pointer parent;

  public:
    NestedSymbolTable() :
            offset(0),
            size(0),
            parent(nullptr)
    {}

    NestedSymbolTable(NestedSymbolTable::pointer parent_) :
            offset(parent_->offset),
            size(0),
            parent(parent_)
    {}

    Symbol find(const string name) const;

    NestedSymbolTable::pointer adopt(NestedSymbolTable::pointer nested)
    {
        nested->offset = this->size;
        return nested;
    }

    void add(const Symbol s);

  private:

    Symbol find_local(const string name) const ;

    Symbol findSymbolRecursive(const string name) const;

};


}
