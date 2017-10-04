/****************************************************************************
 *        Filename: "MMML/mmmlc.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep  8 19:36:14 2017"
 *         Updated: "2017-10-04 17:11:25 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <iostream>

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

using namespace std;

int main(int argc, char *argv[])
{
    cout << "Hello World" << endl;
    return 0;
}
