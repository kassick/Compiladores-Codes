/****************************************************************************
 *        Filename: "MMML/include/mmml/mmml_error.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 16:34:31 2017"
 *         Updated: "2017-10-18 14:27:47 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <iostream>
#include <string>
#include "antlr4-runtime.h"
#include "BaseErrorListener.h"
#include "Token.h"

namespace mmml
{

using namespace std;
using namespace antlr4;


// struct Types {
//     static Type::const_pointer recursive_type;
//     static Type::const_pointer bool_type;
//     static Type::const_pointer char_type;
//     static Type::const_pointer int_type;
//     static Type::const_pointer float_type;
//     static Type::const_pointer nil_type;
// };

struct Report
{
    // Static: Set this so we don't have to set it every time we construct
    static ostream *err_stream, *out_stream;

    static int nerrors;
    static int nwarns;

    static ostream& out() {
        if (out_stream)
            return *out_stream;

        return cout;
    }

    static ostream& err() {
        if (err_stream)
            return *err_stream;

        return cerr;
    }

    static
    ostream& warn(antlr4::ParserRuleContext * ctx, const string message = "")
    {
        if (!ctx)
            return err();

        err() << "Warning at line " << ctx->getStart()->getLine()
              << ", column " << ctx->getStart()->getCharPositionInLine()
              << " : ";
        if (message.length() > 0)
            err() << message << endl;

        nwarns++;

        return err();
    }

    static
    ostream& err(antlr4::ParserRuleContext * ctx, const string message = "")
    {
        if (!ctx)
            return err();

        err() << "Error at line " << ctx->getStart()->getLine()
              << ", column " << ctx->getStart()->getCharPositionInLine()
              << " : ";

        if (message.length() > 0)
            err() << message << endl;

        nerrors++;

        return err();
    }

    ostream& out(antlr4::ParserRuleContext* ctx, const string message="")
    {
        if (!ctx)
            return out();

        out() << "At line" << ctx->getStart()->getLine()
              << ", column " << ctx->getStart()->getCharPositionInLine()
              << " : ";

        if (message.length() > 0)
            out() << message << endl;

        return out();
    }

    static
    ostream& info(antlr4::ParserRuleContext* ctx, const string message="")
    {
        if (!ctx)
            return err();

        err() << "Info: at line " << ctx->getStart()->getLine()
              << ", column " << ctx->getStart()->getCharPositionInLine()
              << " : ";

        if (message.length() > 0)
            out() << message << endl;

        return err();
    }
};

struct ErrorListener : public antlr4::BaseErrorListener {

    std::ostream & out;

    ErrorListener(std::ostream & out_) : out(out_) {}

    virtual void syntaxError(Recognizer *recognizer, Token * offendingSymbol, size_t line, size_t charPositionInLine,
                             const std::string &msg, std::exception_ptr e) override
    {
        out << "Syntax error on token " << offendingSymbol->toString()
            << " at line" << line << ", column" << charPositionInLine
            << " : " << msg
            << std::endl;
        Report::nerrors++;
    }

    // virtual void reportAmbiguity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact,
    //                              const antlrcpp::BitSet &ambigAlts, atn::ATNConfigSet *configs) override;

    // virtual void reportAttemptingFullContext(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
    //                                          const antlrcpp::BitSet &conflictingAlts, atn::ATNConfigSet *configs) override;

    // virtual void reportContextSensitivity(Parser *recognizer, const dfa::DFA &dfa, size_t startIndex, size_t stopIndex,
    //                                       size_t prediction, atn::ATNConfigSet *configs) override;
};


} // end namespace mmml ///////////////////////////////////////////////////////
