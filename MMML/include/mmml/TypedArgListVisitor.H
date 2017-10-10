/****************************************************************************
 *        Filename: "MMML/include/mmml/TypedArgListVisitor.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 17:10:53 2017"
 *         Updated: "2017-10-10 16:55:35 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <vector>
#include <set>
#include "mmml/TypeRegistry.H"
#include "mmml/Symbol.H"
#include "MMMLBaseVisitor.h"

namespace mmml {
using namespace std;

class TypedArgListVisitor : public MMMLBaseVisitor
{
    std::vector<Symbol::const_pointer> result;
    std::set<string> param_names;

    virtual antlrcpp::Any visitTyped_arg_list_rule(MMMLParser::Typed_arg_list_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitTyped_arg(MMMLParser::Typed_argContext *ctx) override ;

    virtual antlrcpp::Any visitTyped_arg_list_cont_rule(MMMLParser::Typed_arg_list_cont_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitTyped_arg_list_end(MMMLParser::Typed_arg_list_endContext *ctx) override ;

    virtual antlrcpp::Any visitType_basictype_rule(MMMLParser::Type_basictype_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitType_tuple_rule(MMMLParser::Type_tuple_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitType_custom_rule(MMMLParser::Type_custom_ruleContext *ctx) override;

    virtual antlrcpp::Any visitType_sequence_rule(MMMLParser::Type_sequence_ruleContext *ctx) override;

};
}
