/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 21:28:33 2017"
 *         Updated: "2017-09-08 22:07:16 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/NestedSymbolTable.H"

namespace mmml {

using namespace std;

void NestedSymbolTable::add(const Symbol s) {
    auto si = data.find(s.name);
    if (si != data.cend()) {
        throw runtime_error(string("Name class in symbol table: ") +
                            to_string(s) +
                            " would shadow " +
                            to_string(si->second) +
                            " on the same namespace");
    }

    Symbol& s_on_hash = data.at(s.name);
    s_on_hash = s;
    s_on_hash.offset = this->size;
    this->size += s_on_hash.size();
}

Symbol NestedSymbolTable::find(const string name) const {
    try {
        Symbol s = this->findSymbolRecursive(name);
        s.offset += this->offset;
        return s;
    } catch (string s) {
        throw std::out_of_range(string("Symbol not found " + name));
    }
}

Symbol NestedSymbolTable::find_local(const string name) const {
    auto s = data.find(name);
    if (s == data.cend()) {
        throw "NOTFOUND";
    }

    return s->second;
}

Symbol NestedSymbolTable::findSymbolRecursive(const string name) const {
    try {
        return find_local(name);
    } catch(string e) {
        if (parent)
            return parent->findSymbolRecursive(name);
        else
            throw "NOTFOUND";
    }
}

}
