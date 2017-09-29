/****************************************************************************
 *        Filename: "MMML/tests/test_nested_table.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 21:53:47 2017"
 *         Updated: "2017-09-29 10:27:07 kassick"
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
    auto & registry = TypeRegistry::instance();

    auto ct = registry.add(make_shared<CharType>());
    auto it = registry.add(make_shared<IntType>());

    cout << "char == Int?"
         << (ct->equals(it) ? " true " : " false ")
         << " (should be false)"
         << endl;

    cout << "int == char?"
         << (it->equals(ct) ? " true " : " false ")
         << " (should be false)"
         << endl;

    cout << "int == int?"
         << (it->equals(it) ? " true " : " false ")
         << " (should be true)"
         << endl;

    cout << "char == char?"
         << (ct->equals(ct) ? " true " : " false ")
         << " (should be true)"
         << endl;


    auto tup1 = registry.add ( make_shared<TupleType>( std::vector<Type::const_weak_pointer>({ct, it} ) ) );
    auto tup2 = registry.add ( make_shared<TupleType>( TupleType::base_types_vector_t       ({ct, it} ) ) );
    auto tup3 = registry.add ( make_shared<TupleType>( TupleType::base_types_vector_t       ({it, ct} ) ) );

    cout << "{char, int} == {int, char}? " << (tup1->equals(tup3) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << "{int, char} == {char, int}? " << (tup2->equals(tup3) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << "{int, char} == {int, char}? " << (tup3->equals(tup3) ? "true" : "false")
         << " (should be true)"
         << endl;

    cout << "{int, char} == char? " << (tup3->equals(ct) ? "true" : "false")
         << " (should be false)"
         << endl;

    cout << "char == {int, char}? " << (ct->equals(tup3) ? "true" : "false")
         << " (should be false)"
         << endl;

    auto s1 =
            registry.add(
                make_shared<ClassType>("teste1",
                                       ClassType::base_field_vector_t
                                       ({ {"a", ct}, {"b", it}} )));
    auto s2 = registry.add(
        make_shared<ClassType>("teste2",
                               ClassType::base_field_vector_t({{"a", ct}, {"b", it}}) ));

    auto s3 = registry.add(
        make_shared<ClassType>("teste1",
                               ClassType::base_field_vector_t({{"b", it}, {"a", ct}, {"d", ct},}) ));

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
         << " (should be true, SINCE s3 HAS THE SAME NAME, THUS ADD RETURNED s1 FROM STORAGE)"
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

    auto s3_again = registry.add(
                make_shared<ClassType>("teste1",
                                       ClassType::base_field_vector_t({{"b", it}, {"a", ct}, {"d", ct},}) ));

    cout << "After trying to register type again, got " << s3_again->to_string() << endl;

    auto seq1 = registry.add(make_shared<SequenceType>(it));
    auto seq2 = registry.add(make_shared<SequenceType>(ct));
    auto seq3 = registry.add(make_shared<SequenceType>(it));

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

    auto seq3_again = registry.add(make_shared<SequenceType>(it));
    cout << "Adding duplicated sequence int[] returned" << seq3_again->to_string()
         << ", should be " << seq1->to_string()
         << endl;

    cout << registry.to_string();


    return 0;
}
