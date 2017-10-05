/****************************************************************************
 *        Filename: "MMML/include/mmml/BasicBlock.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 17:21:32 2017"
 *         Updated: "2017-10-05 12:15:51 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <vector>
#include <iostream>
#include <memory>

#include "mmml/Instruction.H"

namespace mmml {
struct InstructionBlock;
}

namespace std {
ostream& operator<<(ostream& out, const mmml::InstructionBlock& ib);
}

namespace mmml {


using namespace std;

struct InstructionBlock
{
    typedef shared_ptr<InstructionBlock> pointer;
    typedef shared_ptr<const InstructionBlock> const_pointer;
    typedef weak_ptr<InstructionBlock> weak_pointer;
    typedef weak_ptr<const InstructionBlock> const_weak_pointer;

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

    friend ostream& std::operator<<(ostream& out, const InstructionBlock& ib);
    friend InstructionBlock& operator<<(InstructionBlock& ib, const InstructionBlock& other);
    friend InstructionBlock& operator<<(InstructionBlock& ib, InstructionBlock&& other);
    friend InstructionBlock& operator<<(InstructionBlock& ib, const Instruction& inst);
    friend InstructionBlock& operator<<(InstructionBlock& ib, Instruction&& inst);
};


} // end namespace mmml ///////////////////////////////////////////////////////
