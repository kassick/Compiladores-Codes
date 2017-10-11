/****************************************************************************
 *        Filename: "MMML/FunctionCallArgListVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct 10 19:33:25 2017"
 *         Updated: "2017-10-11 02:21:46 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/FunctionCallArgListVisitor.H"
#include "mmml/MetaExprVisitor.H"

using namespace mmml;

antlrcpp::Any
FunctionCallArgListVisitor::visitFuncallparams_rule(MMMLParser::Funcallparams_ruleContext *ctx)
{
    MetaExprVisitor mev(code_ctx->create_block());
    Type::const_pointer rtype = mev.visit(ctx->metaexpr());

    this->args.emplace_back(rtype, mev.code_ctx);

    return this->visit(ctx->funcall_params_cont());
}

antlrcpp::Any
FunctionCallArgListVisitor::visitFuncallnoparam_rule(MMMLParser::Funcallnoparam_ruleContext *ctx)
{
    return 0;
}

antlrcpp::Any
FunctionCallArgListVisitor::visitFuncall_params_cont_rule(MMMLParser::Funcall_params_cont_ruleContext *ctx)
{
    MetaExprVisitor mev(code_ctx->create_block());
    Type::const_pointer rtype = mev.visit(ctx->metaexpr());

    this->args.emplace_back(rtype, mev.code_ctx);

    return this->visit(ctx->funcall_params_cont());
}

antlrcpp::Any
FunctionCallArgListVisitor::visitFuncall_params_end_rule(MMMLParser::Funcall_params_end_ruleContext *ctx)
{
    return (int)this->args.size();
}
