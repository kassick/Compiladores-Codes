/****************************************************************************
 *        Filename: "MMML/include/mmml/ToplevelVisitor.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Oct  4 10:10:13 2017"
 *         Updated: "2017-10-04 10:24:58 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/BaseCodeVisitor.H"

namespace mmml {

using namespace std;

class ToplevelVisitor : public BaseCodeVisitor
{
    ToplevelVisitor() :
            BaseCodeVisitor(make_shared<CodeContext>())
    {}

    virtual antlrcpp::Any visitProgrammain_rule(MMMLParser::Programmain_ruleContext *ctx) override ;

    // Function Definition
    virtual antlrcpp::Any visitFuncdef_impl(MMMLParser::Funcdef_implContext *ctx) override;

    virtual antlrcpp::Any visitFuncdef_header(MMMLParser::Funcdef_headerContext *ctx) override ;

    virtual antlrcpp::Any visitCustom_type_decl_rule(MMMLParser::Custom_type_decl_ruleContext *ctx) override ;


};

}
