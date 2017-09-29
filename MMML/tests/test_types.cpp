/****************************************************************************
 *        Filename: "MMML/tests/test_nested_table.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 21:53:47 2017"
 *         Updated: "2017-09-28 22:29:15 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <iostream>
#include <memory>
#include "mmml/basic_types.H"

using namespace mmml;
using namespace std;

int main(int argc, char *argv[])
{
    auto ct = make_shared<CharType>();
    auto it = make_shared<IntType>();

    cout << "Char == Int?"
         << (ct->equals(it) ? " true " : " false ")
         << " (should be false)"
         << endl;

    cout << "Int == Char?"
         << (it->equals(ct) ? " true " : " false ")
         << " (should be false)"
         << endl;

    cout << "Int == Int?"
         << (it->equals(it) ? " true " : " false ")
         << " (should be true)"
         << endl;

    cout << "Char == Char?"
         << (ct->equals(ct) ? " true " : " false ")
         << " (should be true)"
         << endl;


    auto tup1 = make_shared<TupleType>( std::vector<Type::const_weak_pointer>({ct, it}) );
    auto tup2 = make_shared<TupleType>(TupleType::base_types_vector_t( {ct, it} ));
    auto tup3 = make_shared<TupleType>( TupleType::base_types_vector_t ( {it, ct} ) );

    cout << "{char, int} == {int, char}? " << (tup1->equals(tup3) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << "{int, char} == {char, int}? " << (tup2->equals(tup3) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << "{int, char} == {int, char}? " << (tup3->equals(tup3) ? "true" : "false")
         << " (should be true)"
         << endl;

    cout << "{int, char} == Char? " << (tup3->equals(ct) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << "Char == {int, char}? " << (ct->equals(tup3) ? "true" : "false")
         << " (should be false)"
         << endl;


    return 0;
}
