/****************************************************************************
 *        Filename: "MMML/tests/test_antlr_any.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Thu Oct  5 00:34:09 2017"
 *         Updated: "2017-10-05 02:00:24 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "antlr4-runtime.h"
#include <memory>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    shared_ptr<int> x(new int);
    antlrcpp::Any a = shared_ptr<int>(nullptr);
    cout << "a is null?" << (a.isNull()? " yes " : " no " ) << endl;
    antlrcpp::Any b = x;
    cout << x.use_count() << " refs to x" << endl;
    cout << a.as<shared_ptr<int>>() << endl;

    shared_ptr<int> outro = b;

    cout << outro << " " << x << endl;
    return 0;
}
