/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 20:55:04 2017"
 *         Updated: "2017-09-27 21:12:22 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <vector>

#include "mmml/Symbol.H"

namespace mmml {

using namespace std;

class NestedSymbolTable : public enable_shared_from_this<NestedSymbolTable>
{
  private:
    int _size;
    unordered_map<Symbol::name_type, Symbol::const_pointer> data;

  public:
    typedef shared_ptr<NestedSymbolTable> pointer;
    typedef shared_ptr<const NestedSymbolTable> const_pointer;
    typedef weak_ptr<NestedSymbolTable> weak_pointer;
    typedef weak_ptr<const NestedSymbolTable> const_weak_pointer;

    const int level;
    const int offset;
    NestedSymbolTable::const_weak_pointer parent;
    std::vector<NestedSymbolTable::const_pointer> children;

    NestedSymbolTable() :
            parent(),
            level(0),
            offset(0),
            _size(0)
    {}

    NestedSymbolTable(NestedSymbolTable::pointer parent_) :
            parent(parent_),
            level(parent_->level + 1),
            offset(parent_->offset),
            _size(0)
    {
        parent_->children.push_back(shared_from_this());
    }

    NestedSymbolTable(const NestedSymbolTable& parent_) = delete;
    NestedSymbolTable(NestedSymbolTable&& parent_) = delete;

    Symbol::const_pointer find(const Symbol::name_type name) const;

    void add(const Symbol::pointer s);

    int size() const;


  private:
    Symbol::const_pointer find_local(const Symbol::name_type name) const ;

    Symbol::const_pointer findSymbolRecursive(const Symbol::name_type name) const;

    int sizeRecursiveDown() const;
};


}
