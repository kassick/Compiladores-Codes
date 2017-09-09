/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 20:55:04 2017"
 *         Updated: "2017-09-08 22:05:54 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#pragma once

#include <string>

namespace mmml {

using namespace std;

struct Type {
    int id;
    string name;

    int size;
};

}

namespace std {
static
string to_string(const mmml::Type &t) {
    return string("Type ") +
           to_string(t.id) +
            string(" -> ") +
            t.name;
}
}
