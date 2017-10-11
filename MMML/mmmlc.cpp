/****************************************************************************
 *        Filename: "MMML/mmmlc.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 19:36:14 2017"
 *         Updated: "2017-10-11 03:29:56 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <iostream>
#include <cstdio>
#include <unistd.h>

#include "mmml/NestedSymbolTable.H"
#include "mmml/TypeRegistry.H"
#include "mmml/basic_types.H"
#include "mmml/Instruction.H"
#include "mmml/InstructionBlock.H"
#include "mmml/CodeContext.H"
#include "mmml/FunctionRegistry.H"
#include "mmml/ToplevelVisitor.H"
#include "mmml/FuncbodyVisitor.H"
#include "mmml/MetaExprVisitor.H"
#include "mmml/error.H"
#include "antlr4-runtime.h"
#include "MMMLLexer.h"

#include "docopt/docopt.h"

using namespace std;
using namespace mmml;
using namespace antlr4;

static const char USAGE[] =
        R"(MMML Compiler

    Usage:
      mmmlc [-O N] [-o OUTPUT] [FILE]

    Options:
      -h --help         Show this screen.
      -o OUTPUT         Write output to file
      -O N              Optimization Level [default: 1]
)";

void parse_stream(istream& text_stream, ostream& err_stream, ostream& out_stream, ostream& code_stream)
{
    Report::out_stream = &out_stream;
    Report::err_stream = &err_stream;

    ToplevelVisitor visitor;
    try {
        ErrorListener err_listener(err_stream);

        ANTLRInputStream ain (text_stream);

        MMMLLexer lexer(&ain);
        lexer.addErrorListener(&err_listener);

        CommonTokenStream tokens(&lexer);
        tokens.fill();

        MMMLParser parser(&tokens);

        parser.addErrorListener(&err_listener);

        tree::ParseTree * tree = parser.program();


        visitor.visit(tree);
    } catch (exception & e) {
        err_stream << "Pikachu! " << e.what() << endl;
        Report::nerrors++;
    }

    err_stream << "Errors: " << Report::nerrors << endl;

    if (Report::nerrors == 0) {
        code_stream << *visitor.code_ctx->code;
    }
}

const string parse_string(const string &text)
{
    stringstream
            text_stream(text),
            err_stream,
            out_stream,
            asm_stream;

    parse_stream(text_stream, err_stream, out_stream, asm_stream);

    if (Report::nerrors > 0)
        return err_stream.str();

    stringstream ret_stream;

    auto str = err_stream.str();
    int pos;
    do {
         pos = str.find('\n');
         ret_stream << "# "
                    << str.substr(0, pos)
                    << "\n";
         str = str.substr(pos + 1);
    } while (str.length() > 0);

    ret_stream << endl;

    str = out_stream.str();
    do {
        pos = str.find('\n');
        ret_stream << "# "
                   << str.substr(0, pos)
                   << "\n";
        str = str.substr(pos + 1);
    } while (str.length() > 0);

    ret_stream << endl;

    ret_stream << asm_stream.str();

    return ret_stream.str();
}

extern "C" {
    const char * parse_string_c(const char * text)
    {
        return strdup(parse_string(string(text)).c_str());
    }
}

int main(int argc, char *argv[])
{
    std::map<std::string, docopt::value> args =
            docopt::docopt(USAGE,
                           { argv + 1, argv + argc },
                           true,               // show help if requested
                           "MMML 1.0");     // version string

    ostream* out = &cout;
    istream* in = &cin;

    if (args["FILE"])
    {
        fstream * fh = new fstream(args["FILE"].asString(), ios_base::in);
        if (!fh->is_open()) {
            cerr << "Could not open input file ``"
                 << args["FILE"].asString()
                 << "''"
                 << endl;
            return 1;
        }

        in = fh;
    }

    if (args["-o"])
    {
        fstream * fh = new fstream(args["-o"].asString(), ios_base::out);
        if (!fh->is_open()) {
            cerr << "Could not open output file ``"
                 << args["-o"].asString()
                 << "''"
                 << endl;
            return 1;
        }

        out = fh;
    }

    try {
        parse_stream(/*text=*/ *in,
                     cerr, cout,
                     /*code_out=*/ *out);
    } catch (const std::exception &e) {
        // void *array[50];
        // size_t size;

        cerr << "Got " << e.what();
        // // get void*'s for all entries on the stack
        // size = backtrace(array, 50);

        // backtrace_symbols_fd(array, size,
        //                      STDERR_FILENO);
        exit(1);
    }

    return 0;
}
