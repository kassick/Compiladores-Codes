/****************************************************************************
 *        Filename: "sparser_antlr.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Aug  1 23:16:10 2017"
 *         Updated: "2017-08-30 22:04:23 kassick"
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
#include "SExprVisitor.H"

using namespace sexpr_parser;
using namespace antlr4;
using namespace std;

void parse_antlr(istream& in, stringstream &sout)
{
    ANTLRInputStream ain(in);
    SExprLexer lexer(&ain);
    CommonTokenStream tokens(&lexer);
    tokens.fill();

    SExprParser parser(&tokens);
    SExprVisitor visitor;
    visitor.out = &cout;
    //parser.sexpr()->enterRule(&visitor);
    tree::ParseTree* tree = parser.sexpr();
    tree::ParseTreeWalker::DEFAULT.walk(&visitor, tree);

}

std::string parsestring(std::string s) {
    std::stringstream sin(s);
    std::stringstream sout;
    sout << "Antlr: " << endl;
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
