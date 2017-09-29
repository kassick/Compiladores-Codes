/****************************************************************************
 *        Filename: "MMML/include/mmml/BasicBlock.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 17:21:32 2017"
 *         Updated: "2017-09-29 18:14:07 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <vector>
#include <iostream>

#include "mmml/Instruction.H"

namespace mmml {

using namespace std;

struct InstructionBlock
{
    vector<Instruction> instructions;

    InstructionBlock() {}

    InstructionBlock(const InstructionBlock& other)
            : instructions(other.instructions)
    {}

    InstructionBlock(InstructionBlock&& other)
            : instructions(std::move(other.instructions))
    {}


    // Join-Copy ctor
    InstructionBlock(const InstructionBlock& other1, const InstructionBlock& other2)
    {
        // pre-allocate
        this->instructions.reserve(other1.instructions.size() + other2.instructions.size());

        std::copy(other1.instructions.cbegin(),
                  other2.instructions.cend(),
                  std::back_inserter(this->instructions));
    }

    // Join-Move ctor
    InstructionBlock(InstructionBlock&& other1, InstructionBlock&& other2)
    {
        // pre-allocate
        this->instructions.reserve(other1.instructions.size() + other2.instructions.size());

        std::move(other1.instructions.begin(),
                  other2.instructions.end(),
                  std::back_inserter(this->instructions));
    }

    friend ostream& operator<<(ostream& out, const InstructionBlock& ib);
    friend InstructionBlock& operator<<(InstructionBlock& ib, const InstructionBlock& other);
    friend InstructionBlock& operator<<(InstructionBlock& ib, InstructionBlock&& other);
};


ostream& operator<<(ostream& out, const InstructionBlock& ib)
{
    for(const auto instr: ib.instructions)
        out << instr.assembly() << endl;

    return out;
}

InstructionBlock& operator<<(InstructionBlock& ib, const InstructionBlock& other)
{
    // pre-allocate the right size
    ib.instructions.reserve(ib.instructions.size() + other.instructions.size());

    // cal std::copy [ begin(other), end(other) ) into back of original instructions
    std::copy(other.instructions.begin(), other.instructions.end(),
              std::back_inserter(ib.instructions));

    return ib;
}

InstructionBlock& operator<<(InstructionBlock& ib, InstructionBlock&& other)
{

    // pre-allocate the right size
    ib.instructions.reserve(ib.instructions.size() + other.instructions.size());

    // cal std::move [ begin(other), end(other) ) into back of original instructions
    std::move(other.instructions.begin(), other.instructions.end(),
              std::back_inserter(ib.instructions));

    return ib;
}

} // end namespace mmml ///////////////////////////////////////////////////////
