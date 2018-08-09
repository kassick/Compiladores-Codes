/****************************************************************************
 *        Filename: "SExprMyListener.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Aug 30 21:07:02 2017"
 *         Updated: "2018-08-09 12:22:48 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "SExprBaseVisitor.h"

static
std::ostream& print_indent(std::ostream * out, int indent, const std::string msg)
{
    for (int i = 0; i < indent; i++)
        *out << "    ";
    *out << msg << std::endl;

    return *out;
}

namespace sexpr_parser {

struct TreeNode {
    std::string name;
    std::vector<TreeNode> child;
};

class SExprMyVisitor : public SExprBaseVisitor
{
  public:
    int indent = 0;
    std::ostream * out;public:

    virtual antlrcpp::Any visitSexpr(SExprParser::SexprContext *ctx) override {

        TreeNode n{ctx->NAME()->getText(), {}};

        n.child = visit(ctx->slist()).as<std::vector<TreeNode>>();

        return n;
    }

    virtual antlrcpp::Any visitSlistName(SExprParser::SlistNameContext *ctx) override {
        TreeNode n{ctx->NAME()->getText(), {}};
        std::vector<TreeNode> v = visit(ctx->childList);
        v.insert(v.begin(), n);
        return v;
    }

    virtual antlrcpp::Any visitSlistSexpr(SExprParser::SlistSexprContext *ctx) override {
        TreeNode n = visit(ctx->sexpr());
        std::vector<TreeNode> v = visit(ctx->childList);
        v.insert(v.begin(), n);
        return v;
    }

    virtual antlrcpp::Any visitSlistEmpty(SExprParser::SlistEmptyContext *ctx) override {
        return std::vector<TreeNode>();
    }
};

}
