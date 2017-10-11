/****************************************************************************
 *        Filename: "stackvm.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 13 11:00:28 2017"
 *         Updated: "2017-10-11 00:48:35 kassick"
 *
 *          Author:
 *
 *                    Copyright (C) 2017,
 ****************************************************************************/

#include <iostream>
#include <stack>
#include <vector>
#include <map>

#include "vm.H"
#include "instructions.H"
#include "StackVMParser.h"
#include "StackVMLexer.h"
#include "StackVMLabelListener.H"
#include "StackVMInstructionListener.H"
#include "StackVMErrorListener.H"

#include "docopt/docopt.h"

using namespace std;
using namespace antlr4;
using namespace StackVM;

static const char USAGE[] =
        R"(StackVM

    Usage:
      stackvm [--interactive] [--show-start] [--show-end] [--quiet] [FILE]

    Options:
      -h --help         Show this screen.
      --interactive     Runs in interactive mode, step by step
      --quiet           Do not present prompts for input/output
      --show-start      Show code and stack before execution
      --show-end        Show code and stack after execution
)";

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

    // for (const auto &kv: vm.label_map)
    // {
    //     out << "found label "
    //         << kv.first
    //         << " at " << kv.second
    //         << endl;
    // }

    return vm;
}

VM& parse_full(tree::ParseTree& tree, VM& vm, ostream& out )
{
    if (!vm.ok_to_go)
        return vm;

    StackVM::InstructionListener instr_listener(vm, out);
    tree::ParseTreeWalker::DEFAULT.walk(&instr_listener, &tree);

    if (instr_listener.errors > 0)
        vm.ok_to_go = false;

    return vm;
}

VM& parse_stream(istream& in, ostream& out, VM& vm)
{
    try {
        ANTLRInputStream ain(in);

        StackVMLexer lexer(&ain);

        CommonTokenStream tokens(&lexer);
        tokens.fill();

        StackVMParser parser(&tokens);

        ErrorListener errl(out);

        parser.addErrorListener(&errl);

        tree::ParseTree* tree = parser.start();

        if (errl.has_errors)
            return vm;

        return parse_full(*tree,
                          parse_labels(*tree, vm, out),
                          out);
    } catch (exception& e) {
        out << "Error during parse : " << e.what() << endl;
        vm.ok_to_go = false;
        return vm;
    }
}

std::string parsestring(std::string s, std::string input) {

    std::stringstream code_stream(s, ios_base::in);
    std::stringstream input_stream(input, ios_base::in);
    std::stringstream out(ios_base::out);

    VM vm(input_stream, out);
    vm.interactive = false;
    vm.step_by_step = false;

    parse_stream(code_stream, out, vm);

    if (!vm.ok_to_go) {
        out << "Can not run" << endl;
    } else {
        if (vm.set_pc_to("start") < 0)
            vm.set_pc_to(0);

        out << "Initial state:" << endl;
        out << vm.to_string();
        out << endl;

        vm.run();

        out << endl << endl
            << "Finished "
            << endl << endl;
        out << vm.to_string() << endl;
    }
    return out.str();
}

extern "C" {
    const char * parse_string_c(const char *code, const char * input)
    {
        return strdup(parsestring(string(code), string(input)).c_str());
    }
}

int main(int argc, char *argv[])
{
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
                                                               { argv + 1, argv + argc },
                                                               true,               // show help if requested
                                                               "StackVM 1.0");     // version string
    istream * in = &cin;

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

    // New vm that will interact with stdin and stdout
    VM vm(cin, cout);
    vm.step_by_step = args["--interactive"].asBool();
    vm.interactive = !args["--quiet"].asBool();
    vm.instruction_limit = -1;

    // Parse stream from stdin or a file
    parse_stream(*in, cout, vm);

    if (!vm.ok_to_go) {
        cout << "Can not run" << endl;
    } else {
        if (vm.set_pc_to("start") < 0)
            vm.set_pc_to(0);

        if (args["--show-start"].asBool())
        {
            cout << "Running: " << endl;
            cout << vm.to_string() << endl << endl;
        }

        // Run with a display window size of 10
        vm.run(10);

        if (args["--show-end"].asBool())
        {
            cout << endl << endl
                 << "Finished " << endl;
            cout << vm.to_string();
        }
    }

    return 0;
}
