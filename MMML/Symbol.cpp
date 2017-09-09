/****************************************************************************
 *        Filename: "MMML/include/NestedSymbolTable.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 21:28:33 2017"
 *         Updated: "2017-09-08 22:04:36 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/Symbol.H"
namespace mmml {

using namespace std;

string Symbol::to_string() const {
    return name + "@ " +
            std::to_string(line) +
            ":" +
            std::to_string(col);
}

int Symbol::size() const {
    // must use type-specific size?
    return 1;
}

}

namespace std {
string to_string(const mmml::Symbol &s) {
    return s.to_string();
}
}
