/****************************************************************************
 *        Filename: "stackvm.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 13 11:00:28 2017"
 *         Updated: "2017-09-19 03:58:08 kassick"
 *
 *          Author:
 *
 *                    Copyright (C) 2017,
 ****************************************************************************/

#include <iostream>
#include <stack>
#include <vector>

#include "vm.H"
#include "instructions.H"
#include "StackVMParser.h"
#include "StackVMLexer.h"
#include "StackVMLabelListener.H"
#include "StackVMInstructionListener.H"

using namespace std;
using namespace antlr4;
using namespace StackVM;

VM& parse_labels(tree::ParseTree & tree, VM& vm, ostream& out )
{
    StackVM::LabelListener label_listener(vm, out);

    tree::ParseTreeWalker::DEFAULT.walk(&label_listener, &tree);

    if (label_listener.errors > 0) {
        vm.ok_to_go = false;
        out << endl
            << label_listener.errors
            << " errors. Stopping."
            << endl;
    } else {
        // listener saw n lines, prepare the vector
        vm.instructions.resize(label_listener.cur_line);
    }

    for (const auto &kv: vm.label_map)
    {
        out << "found label "
            << kv.first
            << " at " << kv.second
            << endl;
    }

    return vm;
}

VM& parse_full(tree::ParseTree& tree, VM& vm, ostream& out )
{
    if (!vm.ok_to_go)
        return vm;

    StackVM::InstructionListener instr_listener(vm, out);
    tree::ParseTreeWalker::DEFAULT.walk(&instr_listener, &tree);

    return vm;
}

VM& parse_stream(istream& in, ostream& out, VM& vm)
{
    ANTLRInputStream ain(in);

    StackVMLexer lexer(&ain);

    CommonTokenStream tokens(&lexer);
    tokens.fill();

    StackVMParser parser(&tokens);

    tree::ParseTree* tree = parser.start();

    return parse_full(*tree,
                      parse_labels(*tree, vm, out),
                      out);
}

std::string parsestring(std::string s) {
    std::stringstream sin(s);
    std::stringstream out;

    VM vm(sin, out);

    parse_stream(sin, out, vm);

    if (!vm.ok_to_go) {
        out << "Can not run" << endl;
    } else {
        out << "going to run now" << endl;
        out << vm.to_string();
    }

    return out.str();
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
    istream * in = &cin;

    if (argc > 1) {
        fstream * fh = new fstream(argv[1], ios_base::in);
        in = fh;
    }

    VM vm(*in, sout);
    parse_stream(*in, sout, vm);

    if (!vm.ok_to_go) {
        sout << "Can not run" << endl;
    } else {
        sout << "going to run now" << endl;
        sout << vm.to_string();
    }

    cout << sout.str();

    return 0;
}
