/****************************************************************************
 *        Filename: "MathExprParser/MathExprListener.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Sun Sep 24 19:01:07 2017"
 *         Updated: "2017-09-24 19:22:29 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <antlr4-runtime.h>
#include <MathExprBaseListener.h>
#include <iostream>
#include <string>

using namespace antlr4;
using namespace MathExpr;
using namespace std;

class MathToStackListener : public MathExpr::MathExprBaseListener
{
public:

    ostream & out;

    MathToStackListener(ostream& out_) : out(out_) {}

  virtual void exitExprPlusOrMinusRule(MathExprParser::ExprPlusOrMinusRuleContext * ctx) override {
      if (ctx->op->getText() == "+")
          out << "add" << endl;
      else
          out << "sub" << endl;
  }

  virtual void exitExprDivOrMulRule(MathExprParser::ExprDivOrMulRuleContext * ctx) override {
      if (ctx->op->getText() == "*")
          out << "mul" << endl;
      else
          out << "div" << endl;

}

  virtual void exitNumberfloatRule(MathExprParser::NumberfloatRuleContext * ctx) override {
      out << "push " << ctx->getText() << endl;
  }

  virtual void exitNumberdecimalRule(MathExprParser::NumberdecimalRuleContext * ctx) override {
      out << "push " << ctx->getText() << endl;
  }

  virtual void exitNumberhexadecimalRule(MathExprParser::NumberhexadecimalRuleContext * ctx) override {
      out << "push " << stoi(ctx->getText().substr(2, ctx->getText().length() - 2), 0, 16) << endl;
  }

  virtual void exitNumberbinaryRule(MathExprParser::NumberbinaryRuleContext * ctx) override {
      out << "push " << stoi(ctx->getText().substr(0, ctx->getText().length() - 1), 0, 2) << endl;
  }

};
