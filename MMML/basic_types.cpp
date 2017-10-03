/****************************************************************************
 *        Filename: "MMML/basic_types.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 21:06:03 2017"
 *         Updated: "2017-10-03 17:04:00 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/basic_types.H"
#include "mmml/TypeRegistry.H"

using namespace mmml;

Type::const_pointer Types::recursive_type = TypeRegistry::instance().find_by_name("@recursive");
Type::const_pointer Types::bool_type = TypeRegistry::instance().find_by_name("bool");
Type::const_pointer Types::char_type = TypeRegistry::instance().find_by_name("char");
Type::const_pointer Types::int_type = TypeRegistry::instance().find_by_name("int");
Type::const_pointer Types::float_type = TypeRegistry::instance().find_by_name("float");
Type::const_pointer Types::nil_type = TypeRegistry::instance().find_by_name("nil");

void __attribute__ ((constructor)) init_basic_types()
{
    auto& registry = mmml::TypeRegistry::instance();
    registry.add(make_shared<mmml::RecursiveType>());
    registry.add(make_shared<mmml::CharType>());
    registry.add(make_shared<mmml::IntType>());
    registry.add(make_shared<mmml::FloatType>());
    registry.add(make_shared<mmml::BoolType>());
    registry.add(make_shared<mmml::NilType>());

}
