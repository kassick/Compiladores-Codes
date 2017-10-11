/****************************************************************************
 *        Filename: "MMML/include/mmml/MetaExprVisitor.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:45:16 2017"
 *         Updated: "2017-10-10 20:30:56 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "BaseCodeVisitor.H"
#include "mmml/Type.H"
#include <vector>

namespace mmml {

class  FunctionCallArgListVisitor : public BaseCodeVisitor {

  public:

    typedef std::vector<std::pair<Type::const_pointer, CodeContext::pointer>> ArgVector;

    ArgVector args;

    FunctionCallArgListVisitor() : BaseCodeVisitor() {}

    FunctionCallArgListVisitor(CodeContext::pointer ctx) : BaseCodeVisitor(ctx) {}

    virtual antlrcpp::Any visitFuncallparams_rule(MMMLParser::Funcallparams_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitFuncallnoparam_rule(MMMLParser::Funcallnoparam_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitFuncall_params_cont_rule(MMMLParser::Funcall_params_cont_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitFuncall_params_end_rule(MMMLParser::Funcall_params_end_ruleContext *ctx) override ;

};

}
