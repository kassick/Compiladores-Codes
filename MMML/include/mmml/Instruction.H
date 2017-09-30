/****************************************************************************
 *        Filename: "MMML/include/mmml/Instruction.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 17:01:22 2017"
 *         Updated: "2017-09-29 21:00:55 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <cstring>

namespace std {

// string has no to_string defined ... affe
static
string to_string(const string s)
{
    return s;
}

}

namespace mmml
{


namespace __priv {
    constexpr size_t constLength(const char* str)
    {
        return (*str == 0) ? 0 : constLength(str + 1) + 1;
    }
}

// 0         1         2         3         4         5         6         7         8
// 012345678901234567890123456789012345678901234567890123456789012345678901234567890
// label      : instr arg arg arg                    # annot
constexpr const char * MMML_EMPTY_LABEL = "          ";
constexpr const int MMML_LABEL_LEN = __priv::constLength(MMML_EMPTY_LABEL);
constexpr const int MMML_ANNOT_START = 50;
constexpr const int MMML_INSTR_START = 13;
constexpr const int MMML_INSTR_LEN = 50 - 13 - 1 /*mandatory space*/;

using namespace std;
struct Instruction
{
    string label;
    string op;
    vector<string> args;
    string annot;

    Instruction(const Instruction& other) :
            label(other.label),
            op(other.op),
            args(other.args),
            annot(other.annot)
    {}

    Instruction(Instruction&& other) :
            label(std::move(other.label)),
            op(std::move(other.op)),
            args(std::move(other.args)),
            annot(std::move(other.annot))
    {}

    template <class T>
    Instruction(string op_, initializer_list<T> args_)
            : op(op_)
    {
        auto b = begin(args_);
        auto e = end(args_);

        args.resize(args_.size());

        auto it = this->args.begin();

        while(b != e) {
            *it = std::to_string( *b );
            it++;
            b++;
        }
    }

    template <class T>
    Instruction(string op_, T args_)
            : op(op_)
    {
        auto b = begin(args_);
        auto e = end(args_);

        args.resize(args_.size());

        auto it = this->args.begin();

        while(b != e) {
            *it = std::to_string( *b );
            it++;
            b++;
        }
    }

    Instruction(string op, string annot="")
            : Instruction(op, std::vector<string>())
    {}

    Instruction& with_label(const string l) {
        this->label = l;
        return *this;
    }

    Instruction& with_annot(const string a) {
        this->annot = a;
        return *this;
    }

    string assembly() const {
        stringstream code;

        if (this->label.length() == 0)
            code << MMML_EMPTY_LABEL;
        else {
            code << this->label;
            for (int i = this->label.length(); i < MMML_LABEL_LEN; i++)
                code << " ";
        }

        code << " : "
             << this->op;

        int instr_missing_spaces = MMML_INSTR_LEN - this->op.length();;
        for(const auto& arg : this->args)
        {
            code << " " << arg;
            instr_missing_spaces -= arg.length() + 1;
        }

        if (this->annot.length() > 0)
        {
            while(instr_missing_spaces-- > 0)
            {
                code << " ";
            }

            code << " # " << this->annot;
        }

        return code.str();
    }
};

} // end namespace mmml ///////////////////////////////////////////////////////
