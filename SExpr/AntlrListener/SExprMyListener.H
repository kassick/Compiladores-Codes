/****************************************************************************
 *        Filename: "SExprMyListener.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Aug 30 21:07:02 2017"
 *         Updated: "2017-08-31 11:04:53 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#pragma once

#include <iostream>
#include <string>
#include "SExprBaseListener.h"

static
std::ostream& print_indent(std::ostream * out, int indent, const std::string msg)
{
    for (int i = 0; i < indent; i++)
        *out << "    ";
    *out << msg << std::endl;

    return *out;
}

namespace sexpr_parser {

class SExprMyListener : public SExprBaseListener
{
  public:
    int indent = 0;
    std::ostream * out;

    virtual void enterSexpr(SExprParser::SexprContext * sexpr) override {
        print_indent(out, indent, std::string("( ") + sexpr->n->getText());

        indent++;
    }

    virtual void exitSexpr(SExprParser::SexprContext * /*ctx*/) override {
        indent--;

        print_indent(out, indent, ")");
    }

    virtual void enterSlistName(SExprParser::SlistNameContext * slistName ) override {
        print_indent(out, indent, slistName->NAME()->getText());
    }

    virtual void exitSlistName(SExprParser::SlistNameContext * /*ctx*/) override { }

    virtual void enterSlistSexpr(SExprParser::SlistSexprContext * slistSexpr) override {}

    virtual void exitSlistSexpr(SExprParser::SlistSexprContext * /*ctx*/) override { }

    virtual void enterSlistEmpty(SExprParser::SlistEmptyContext * /*ctx*/) override { }
    virtual void exitSlistEmpty(SExprParser::SlistEmptyContext * /*ctx*/) override { }

    virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
    virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
    virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
    virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }


};

}
