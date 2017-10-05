/****************************************************************************
 *        Filename: "MMML/InstructionBlock.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 21:02:33 2017"
 *         Updated: "2017-10-05 13:19:25 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/InstructionBlock.H"

using namespace mmml;
using namespace std;

namespace std {
    ostream& operator<<(ostream& out, const InstructionBlock& ib)
    {
        for(const auto instr: ib.instructions)
            out << instr.assembly() << endl;

        return out;
    }
}

namespace mmml {
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

        other.instructions.resize(0);

        return ib;
    }

    InstructionBlock& operator<<(InstructionBlock& ib, const Instruction& inst)
    {
        ib.instructions.emplace_back(inst);
        return ib;
    }

    InstructionBlock& operator<<(InstructionBlock& ib, Instruction&& inst)
    {
        ib.instructions.emplace_back(std::move(inst));
        return ib;
    }

}
