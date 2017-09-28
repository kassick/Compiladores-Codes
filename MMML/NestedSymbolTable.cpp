/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 21:28:33 2017"
 *         Updated: "2017-09-27 23:47:50 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <algorithm>
#include "mmml/NestedSymbolTable.H"

namespace mmml {

using namespace std;

void NestedSymbolTable::add(Symbol::pointer s) {
    auto si = data.find(s->name);
    if (si != data.cend()) {
        throw runtime_error(string("Name class in symbol table: ") +
                            std::to_string(*s) +
                            " would shadow " +
                            std::to_string(*si->second) +
                            " on the same namespace");
    }

    s->pos = this->_size;
    data[s->name] = s;
    this->_size += s->size();
}

Symbol::const_pointer
NestedSymbolTable::find(const Symbol::name_type name)
        const
{
    return this->findSymbolRecursive(name);
}

Symbol::const_pointer
NestedSymbolTable::find_local(const Symbol::name_type name)
        const
{
    auto s = data.find(name);
    if (s == data.cend()) {
        return nullptr;
    }

    return s->second;
}

Symbol::const_pointer
NestedSymbolTable::findSymbolRecursive(const Symbol::name_type name)
        const
{
    auto s = find_local(name);
    if (s)
        return s;

    auto parent_ = this->parent.lock();
    if (parent_)
        return parent_->findSymbolRecursive(name);

    return nullptr;
}

int NestedSymbolTable::size() const {
    return sizeRecursiveDown();
}

int NestedSymbolTable::sizeRecursiveDown() const {
    if (this->children.size() == 0)
        return this->_size;

    int max = -1;
    for (const auto& child : this->children)
        if (child->sizeRecursiveDown() > max)
            max = child->sizeRecursiveDown();

    return max;
}

int NestedSymbolTable::fullNestedSize() const {
    NestedSymbolTable::const_pointer
            cur = shared_from_this(),
            cur_parent = cur->parent.lock();

    while(cur_parent) {
        cur = cur_parent;
        cur_parent = cur->parent.lock();
    }

    return cur->size();
}

string NestedSymbolTable::to_string() const {
    stringstream out;

    for (int i = 0; i < this->level; i++)
        out << "  ";
    out << "--------" << endl;

    for (int i = 0; i < this->level; i++)
        out << "  ";
    out << "Starting offset: " << this->offset << endl;
    for (int i = 0; i < this->level; i++)
        out << "  ";
    out << "Size (local): " << this->_size << endl;

    for (const auto s : this->data) {
        for (int i = 0; i < this->level; i++)
            out << "  ";

        out << std::to_string(*s.second)
            << " at position " << s.second->pos
            << endl;
    }

    for (const auto& child : children)
    {
        out << child->to_string();
    }

    for (int i = 0; i < this->level; i++)
        out << "  ";
    out << "--------" << endl;

    return out.str();
}

}
