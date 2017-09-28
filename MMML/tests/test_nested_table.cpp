/****************************************************************************
 *        Filename: "MMML/tests/test_nested_table.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 21:53:47 2017"
 *         Updated: "2017-09-27 23:49:20 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <iostream>
#include <memory>
#include "mmml/NestedSymbolTable.H"

using namespace mmml;
using namespace std;

int main(int argc, char *argv[])
{
    auto type = make_shared<const Type>(0, "padrao", 1);
    auto mainTable = make_shared<NestedSymbolTable>();
    mainTable->add(make_shared<Symbol>("a", type, 0, 0));
    mainTable->add(make_shared<Symbol>("b", type, 1, 0));
    auto subTable1 = mainTable->make_nested();
    subTable1->add(make_shared<Symbol>("c", type, 2, 1));
    subTable1->add(make_shared<Symbol>("d", type, 3, 1));
    subTable1->add(make_shared<Symbol>("d2", type, 4, 1));
    auto subTable2 = mainTable->make_nested();
    subTable2->add(make_shared<Symbol>("e", type, 2, 1));
    subTable2->add(make_shared<Symbol>("f", type, 3, 1));

    auto subSubTable21 = subTable2->make_nested();
    subSubTable21->add(make_shared<Symbol>("g", type, 4, 2));
    subSubTable21->add(make_shared<Symbol>("h", type, 5, 2));

    mainTable->add(make_shared<Symbol>("z", type, 2, 0));

    cout << mainTable->to_string() << endl;

    cout << "Main Table Size: " << mainTable->size()
         << " (should be 6)"
         << endl;

    cout << "SubTable1 size is " << subTable1->size()
         << " (should be 5)"
         << endl;

    cout << "SubTable1 FULL nest size is " << subTable1->fullNestedSize()
         << " (should be 6)"
         << endl;

    cout << "SubTable1 LOCAL size is " << (subTable1->size() - subTable1->offset)
         << " (should be 3)"
         << endl;

    return 0;
}
