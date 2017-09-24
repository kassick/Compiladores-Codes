/****************************************************************************
 *        Filename: "StackVM/StackVMErrorListener.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 20 15:54:55 2017"
 *         Updated: "2017-09-24 19:33:02 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "antlr4-runtime.h"
#include "BaseErrorListener.h"
#include "Token.h"
#include <iostream>

using namespace antlr4;
using namespace antlrcpp;

struct MathErrorListener : public antlr4::BaseErrorListener {

    std::ostream & out;

    MathErrorListener(std::ostream & out_) : out(out_) {}

    virtual void syntaxError(Recognizer *recognizer, Token * offendingSymbol, size_t line, size_t charPositionInLine,
                             const std::string &msg, std::exception_ptr e) override
    {
        out << "Syntax error at token " << offendingSymbol->toString()
            << " at line" << line << ", column" << charPositionInLine
            << " : " << msg
            << std::endl;
    }

    // virtual void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact,
    //                              const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs) override;

    // virtual void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
    //                                          const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs) override;

    // virtual void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
    //                                       size_t prediction, atn::ATNConfigSet *configs) override;
};
