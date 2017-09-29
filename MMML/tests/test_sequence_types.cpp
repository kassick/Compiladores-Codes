/****************************************************************************
 *        Filename: "MMML/tests/test_nested_table.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 27 21:53:47 2017"
 *         Updated: "2017-09-29 11:30:23 kassick"
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

    auto ct = registry.find_by_name("char");
    auto it = registry.find_by_name("int");

    auto seq_int = registry.add(make_shared<SequenceType>(it));
    auto seq_char = registry.add(make_shared<SequenceType>(ct));
    auto seq_int2 = registry.add(make_shared<SequenceType>(it));

    cout << seq_int->to_string() << " at " << seq_int
         << " == "
         << seq_int2->to_string() << " at " << seq_int2
         << "? "
         << (seq_int == seq_int2 ? "true" : "false")
         << " (should be true)"
         << endl;

    auto seq_seq_int = registry.add(make_shared<SequenceType>(seq_int))->as<SequenceType>();
    auto seq_seq_seq_int = registry.add(make_shared<SequenceType>(seq_seq_int))->as<SequenceType>();


    cout << seq_seq_int->to_string() << " == " << seq_seq_seq_int->to_string() << "? "
         << (seq_seq_int->equals(seq_seq_seq_int) ? "true" : "false")
         << " (should be false)"
         << endl;

    auto seq_int_again = registry.add(make_shared<SequenceType>(it));
    auto seq_seq_int_again = registry.add(make_shared<SequenceType>(seq_int_again))->as<SequenceType>();
    auto seq_seq_seq_int_again = registry.add(make_shared<SequenceType>(seq_seq_int_again))->as<SequenceType>();

    cout << seq_int_again->to_string() << " at " << seq_int_again
         << " == "
         << seq_int->to_string() << " at " << seq_int << "? "
         << (seq_int_again == seq_int ? "true" : "false")
         << " (should be true)"
         << endl;

    cout << seq_seq_int_again->to_string() << " at " << seq_seq_int_again
         << " == "
         << seq_seq_int->to_string() << " at " << seq_seq_int << "? "
         << (seq_seq_int_again == seq_seq_int ? "true" : "false")
         << " (should be true)"
         << endl;

    cout << seq_seq_seq_int_again->to_string() << " at " << seq_seq_seq_int_again
         << " == "
         << seq_seq_seq_int->to_string() << " at " << seq_seq_seq_int << "? "
         << (seq_seq_seq_int_again == seq_seq_seq_int ? "true" : "false")
         << " (should be true)"
         << endl;

    auto class1 =
            registry.add(
                make_shared<ClassType>(
                    "classe1",
                    ClassType::base_field_vector_t(
                        {
                            {"c1", seq_int},
                            {"c2", seq_seq_seq_int}
                        }
                                                   )
                                       )
                         );

    cout << registry.to_string();


    return 0;
}
