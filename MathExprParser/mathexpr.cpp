/****************************************************************************
 *        Filename: "stackvm.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 13 11:00:28 2017"
 *         Updated: "2017-09-24 19:35:38 kassick"
 *
 *          Author:
 *
 *                    Copyright (C) 2017,
 ****************************************************************************/

#include <fstream>
#include <iostream>
#include <stack>
#include <vector>
#include <sstream>
#include <antlr4-runtime.h>
#include <string.h>

#include "MathExprListener.H"
#include "MathExprErrorListener.H"

#include "MathExprParser.h"
#include "MathExprLexer.h"
// #include "StackVMLabelListener.H"
// #include "StackVMInstructionListener.H"
// #include "StackVMErrorListener.H"

using namespace std;
using namespace antlr4;

void parse_stream(istream& in, ostream& out)
{
    try {
        ANTLRInputStream ain(in);

        MathExprLexer lexer(&ain);

        CommonTokenStream tokens(&lexer);
        tokens.fill();

        MathExprParser parser(&tokens);

        MathErrorListener errl(out);

        parser.addErrorListener(&errl);

        tree::ParseTree* tree = parser.start();

        MathToStackListener expr_listener(out);
        tree::ParseTreeWalker::DEFAULT.walk(&expr_listener, tree);

    } catch (exception& e) {
        out << "Error during parse : " << e.what() << endl;
    }
}

std::string parsestring(std::string input) {

    std::stringstream input_stream(input, ios_base::in);
    std::stringstream out(ios_base::out);

    parse_stream(input_stream, out);

    return out.str();
}

extern "C" {
    const char * parse_string_c(const char *input)
    {
        return strdup(parsestring(string(input)).c_str());
    }
}

int main(int argc, char *argv[])
{
    stringstream sout;
    istream * in = &cin;

    // cout << parse_string_c("readi\nprint\n", "3\n") << endl;

    if (argc > 1) {
        fstream * fh = new fstream(argv[1], ios_base::in);
        if (!fh->is_open()) {
            cerr << "Could not open input file ``"
                 << argv[1]
                 << "''"
                 << endl;
            return 1;
        }

        in = fh;
    }

    // Parse stream from stdin or a file
     parse_stream(*in, sout);
     cout << sout.str();

    return 0;
}
