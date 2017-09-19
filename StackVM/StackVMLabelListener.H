/****************************************************************************
 *        Filename: "StackVM/StackVMLabelListener.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Mon Sep 18 18:08:15 2017"
 *         Updated: "2017-09-19 02:19:48 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "StackVMBaseListener.h"
#include "vm.H"

namespace StackVM {


struct LabelListener : public StackVMBaseListener
{
    int cur_line;
    int errors;
    ostream & out;
    VM & vm;

    LabelListener(VM& vm_, ostream & out_) :
            vm(vm_),
            out(out_),
            cur_line(0),
            errors(0)
    {}

    virtual
    void enterInstructionLine(StackVMParser::InstructionLineContext * ctx)
            override
    {
        if (ctx->label())
        {
            string label = ctx->label()->getText();
            if (vm.label_map.count(label) != 0)
            {
                out << "ERROR: "
                    << "Label ``" << label << "''"
                    << " in line " << ctx->label()->start->getLine()
                    << " has already been used"
                    << endl;
                errors++;
            }

            vm.label_map[label] = cur_line;
        }
    }

    virtual
    void exitInstructionLine(StackVMParser::InstructionLineContext * /*ctx*/)
            override
    {
        cur_line++;
    }
};

}
