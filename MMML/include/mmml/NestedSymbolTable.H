/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 20:55:04 2017"
 *         Updated: "2017-10-10 17:44:35 kassick"
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
#include <sstream>

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

  private:
    NestedSymbolTable(NestedSymbolTable::pointer parent_) :
            parent(parent_),
            level(parent_->level + 1),
            offset(parent_->_size),
            _size(parent_->_size)
    {}

  public:

    NestedSymbolTable::pointer make_nested() {
        shared_ptr<NestedSymbolTable> nt(new NestedSymbolTable(shared_from_this()));
        this->children.push_back(nt);

        return nt;
    }

    NestedSymbolTable(const NestedSymbolTable& parent_) = delete;
    NestedSymbolTable(NestedSymbolTable&& parent_) = delete;

    Symbol::const_pointer find(const Symbol::name_type name) const;

    void add(const Symbol::pointer s);

    int size() const;

    int local_size() const {return _size;}

    int fullNestedSize() const;

    string to_string() const ;


    Symbol::const_pointer find_local(const Symbol::name_type name) const ;

  private:

    Symbol::const_pointer findSymbolRecursive(const Symbol::name_type name) const;

    int sizeRecursiveDown() const;
};


}
