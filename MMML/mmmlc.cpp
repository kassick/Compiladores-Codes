/****************************************************************************
 *        Filename: "MMML/mmmlc.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 19:36:14 2017"
 *         Updated: "2017-10-10 23:25:28 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <execinfo.h>

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

using namespace std;
using namespace mmml;
using namespace antlr4;


void parse_stream(istream& text_stream, ostream& err_stream, ostream& out_stream, ostream& code_stream)
{
    Report::out_stream = &out_stream;
    Report::err_stream = &err_stream;

    ANTLRInputStream ain (text_stream);

    MMMLLexer lexer(&ain);

    CommonTokenStream tokens(&lexer);
    tokens.fill();

    MMMLParser parser(&tokens);

    ErrorListener err_listener(err_stream);
    parser.addErrorListener(&err_listener);

    tree::ParseTree * tree = parser.program();

    ToplevelVisitor visitor;

    visitor.visit(tree);

    err_stream << "Errors: " << Report::nerrors << endl;

    if (Report::nerrors == 0) {
        code_stream << *visitor.code_ctx->code;
    }
}

int main(int argc, char *argv[])
{
    try {
        parse_stream(cin, cerr, cout, cout);
    } catch (const std::exception &e) {
        void *array[50];
        size_t size;

        cerr << "Got " << e.what();
        // get void*'s for all entries on the stack
        size = backtrace(array, 50);

        backtrace_symbols_fd(array, size,
                             STDERR_FILENO);
        exit(1);
    }

    return 0;
}
