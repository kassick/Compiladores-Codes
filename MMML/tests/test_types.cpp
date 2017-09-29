/****************************************************************************
 *        Filename: "MMML/tests/test_nested_table.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 21:53:47 2017"
 *         Updated: "2017-09-29 03:32:53 kassick"
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

template <class T>
shared_ptr<T> shared_from(T* ptr) {
    return shared_ptr<T>(ptr);
}

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

    auto& registry = TypeRegistry::instance();
    registry.add(tup1);
    registry.add(tup2);
    registry.add(tup3);

    auto s1 = make_shared<ClassType>("teste1",
                                     ClassType::base_field_vector_t(
                                         {
                                             {"a", ct},
                                             {"b", it}
                                         }
                                                                    ) );
    auto s2 = make_shared<ClassType>("teste2",
                                     ClassType::base_field_vector_t(
                                         {
                                             {"a", ct},
                                             {"b", it}
                                         }
                                                                    ) );

    auto s3 = make_shared<ClassType>("teste1",
                                     ClassType::base_field_vector_t(
                                         {
                                             {"b", it},
                                             {"a", ct},
                                             {"d", ct},
                                         }
                                                                    ) );
    cout << s1->to_string() << " == " << s2->to_string() << "? "
         << (s1->equals(s2) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << s1->to_string() << " ==equiv== " << s2->to_string() << "? "
         << (s1->equivalent(s2) ? "true" : "false")
         << " (should be true)"
         << endl;

    cout << s1->to_string() << " == " << s3->to_string() << "? "
         << (s1->equals(s3) ? "true" : "false")
         << " (should be true)"
         << endl;

    cout << s1->to_string() << " ==equiv== " << s3->to_string() << "? "
         << (s1->equivalent(s3) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << s1->to_string() << " == " << tup1->to_string() << "? "
         << (s1->equals(tup1) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << tup1->to_string() << " ==equiv== " << s1->to_string() << "? "
         << (s1->equivalent(tup1) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << tup1->to_string() << " ==storage== " << s1->to_string() << "? "
         << (s1->equal_storage(tup1) ? "true" : "false")
         << " (should be true)"
         << endl;

    registry.add(s1);
    registry.add(s2);
    cout << "After trying to register type again, got " << registry.add(s3)->to_string() << endl;

    auto seq1 = make_shared<SequenceType>(it);
    auto seq2 = make_shared<SequenceType>(ct);
    auto seq3 = make_shared<SequenceType>(it);

    cout << seq1->to_string() << " == " << seq2->to_string() << "? "
         << (seq1->equals(seq2) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << seq2->to_string() << " == " << seq1->to_string() << "? "
         << (seq2->equals(seq1) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << seq2->to_string() << " == " << seq2->to_string() << "? "
         << (seq2->equals(seq2) ? "true" : "false")
         << " (should be true)"
         << endl;

    cout << seq1->to_string() << " == " << seq3->to_string() << "? "
         << (seq1->equals(seq2) ? "true" : "false")
         << " (should be true)"
         << endl;

    registry.add(seq1);
    registry.add(seq2);
    cout << "Adding duplicated sequence " << seq3->to_string()
         << " returns " << registry.add(seq3)->to_string()
         << ", should be " << seq1->to_string()
         << endl;

    cout << registry.to_string();


    return 0;
}
