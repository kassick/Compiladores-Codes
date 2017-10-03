/****************************************************************************
 *        Filename: "MMML/basic_types.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 21:06:03 2017"
 *         Updated: "2017-10-02 21:44:28 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/basic_types.H"
#include "mmml/TypeRegistry.H"

using namespace mmml;

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
