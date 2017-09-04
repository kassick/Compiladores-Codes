/****************************************************************************
 *        Filename: "sparser_antlr.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Aug  1 23:16:10 2017"
 *         Updated: "2017-09-04 17:21:34 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <iostream>
#include <SExprParser.h>
#include <SExprLexer.h>
#include <string>
#include <sstream>
#include <string.h>
#include "SExprMyVisitor.H"

using namespace sexpr_parser;
using namespace antlr4;
using namespace std;

void rec_print_tree(ostream* out, const TreeNode & n, int indent=0)
{
    print_indent(out, indent, std::string("(") + n.name);
    for(const auto& c: n.child) {
        rec_print_tree(out, c, indent + 1);
    }
    print_indent(out, indent, std::string(")"));
}

void parse_antlr(istream& in, stringstream &sout)
{
    ANTLRInputStream ain(in);
    SExprLexer lexer(&ain);
    CommonTokenStream tokens(&lexer);
    tokens.fill();

    SExprParser parser(&tokens);
    SExprMyVisitor visitor;
    //parser.sexpr()->enterRule(&listener);
    tree::ParseTree* tree = parser.sexpr();
    TreeNode n = visitor.visit(tree);
    rec_print_tree(&sout, n);
}

std::string parsestring(std::string s) {
    std::stringstream sin(s);
    std::stringstream sout;
    sout << "Antlr Listener: " << endl;
    parse_antlr(sin, sout);
    return sout.str();
}

extern "C" {
    const char * parse_string_c(const char *s)
    {
        return strdup(parsestring(string(s)).c_str());
    }
}

int main(int argc, char *argv[])
{

    stringstream sout;
    parse_antlr(cin, sout);

    //cout << tree->toStringTree(&parser) << endl;

    cout  << sout.str() << endl;
    return 0;
}
