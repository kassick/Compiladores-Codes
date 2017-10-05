/****************************************************************************
 *        Filename: "MMML/include/mmml/FuncbodyVisitor.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Oct  4 14:20:23 2017"
 *         Updated: "2017-10-05 02:09:46 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/BaseCodeVisitor.H"
#include "mmml/NestedSymbolTable.H"

namespace mmml {

using namespace std;

class FuncbodyVisitor : public BaseCodeVisitor
{
  public:
    // Labels: out and in
    string lout, lin;
    // Labels: true and false
    string ltrue, lfalse;

    FuncbodyVisitor() :
            BaseCodeVisitor()
    {}

    FuncbodyVisitor(CodeContext::pointer ctx) :
            BaseCodeVisitor(ctx)
    {}

    FuncbodyVisitor(NestedSymbolTable::pointer symbols) :
            BaseCodeVisitor(make_shared<CodeContext>(symbols))
    {}

    void make_entry_label() {
        make_entry_label(this->code_ctx);
    }

    void make_entry_label(CodeContext::pointer ctx) {
        if (this->lin.length() > 0)
            *ctx << Instruction("nop").with_label(lin);
    }

    void make_out_jump(CodeContext::pointer ctx) {
        if (this->lout.length() > 0)
            *ctx << Instruction("jump", {this->lout});
    }

    void make_out_jump() {
        make_out_jump(this->code_ctx);
    }

    virtual antlrcpp::Any visitFbody_if_rule(MMMLParser::Fbody_if_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitFbody_expr_rule(MMMLParser::Fbody_expr_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitFbody_let_rule(MMMLParser::Fbody_let_ruleContext *ctx) override;

    //virtual antlrcpp::Any visitLetlist_rule(MMMLParser::Letlist_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLetvarattr_rule(MMMLParser::Letvarattr_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLetvarresult_ignore_rule(MMMLParser::Letvarresult_ignore_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLetunpack_rule(MMMLParser::Letunpack_ruleContext *ctx) override ;

};

}
