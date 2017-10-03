/****************************************************************************
 *        Filename: "MMML/TypedArgListVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 17:11:46 2017"
 *         Updated: "2017-10-03 17:16:56 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/TypedArgListVisitor.H"
#include "mmml/error.H"
#include "mmml/TypeRegistry.H"
#include "mmml/basic_types.H"

namespace mmml {

// TypedArgVisitor
antlrcpp::Any
TypedArgListVisitor::visitTyped_arg_list_rule(MMMLParser::Typed_arg_list_ruleContext *ctx)
{
    Symbol::pointer sym = visit(ctx->typed_arg());

    if (param_names.find(sym->name) != param_names.end())
    {
        Report::err(ctx) << "Duplicate symbol found in parameter list: "
                                       << sym->name
                                       << endl;
    } else {
        result.push_back(sym);
    }

    return visit(ctx->typed_arg_list_cont());
}

antlrcpp::Any
TypedArgListVisitor::visitTyped_arg(MMMLParser::Typed_argContext *ctx)
{

    auto type = TypeRegistry::instance().find_by_name(ctx->type()->getText());

    if (!type) {
        Report::err(ctx) << "Unknown type " << ctx->type()->getText()
                                       << endl;
        type = Types::int_type;
    }

    auto sstart = ctx->symbol()->getStart();
    auto sym = make_shared<Symbol>(ctx->symbol()->getText(), type,
                                   sstart->getLine(),
                                   sstart->getCharPositionInLine());

    return sym;
}

antlrcpp::Any
TypedArgListVisitor::visitTyped_arg_list_cont_rule(MMMLParser::Typed_arg_list_cont_ruleContext *ctx)
{
    return visit(ctx->typed_arg_list());
}

antlrcpp::Any
TypedArgListVisitor::visitTyped_arg_list_end(MMMLParser::Typed_arg_list_endContext *ctx)
{
    return this->result;
}


}  // mmml
