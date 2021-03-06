/****************************************************************************
 *        Filename: "StackVM/StackVMErrorListener.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Sep 20 15:54:55 2017"
 *         Updated: "2017-10-11 00:23:58 kassick"
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

namespace StackVM {

using namespace antlr4;
using namespace antlrcpp;

struct ErrorListener : public antlr4::BaseErrorListener {

    std::ostream & out;
    bool has_errors = false;

    ErrorListener(std::ostream & out_) : out(out_) {}

    virtual void syntaxError(Recognizer *recognizer, Token * offendingSymbol, size_t line, size_t charPositionInLine,
                             const std::string &msg, std::exception_ptr e) override
    {
        out << "Syntax error on token " << offendingSymbol->toString()
            << " at line" << line << ", column" << charPositionInLine
            << " : " << msg
            << std::endl;
        has_errors = true;
    }

    // virtual void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact,
    //                              const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs) override;

    // virtual void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
    //                                          const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs) override;

    // virtual void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
    //                                       size_t prediction, atn::ATNConfigSet *configs) override;
};


}
