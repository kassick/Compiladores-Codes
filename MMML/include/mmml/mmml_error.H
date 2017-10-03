/****************************************************************************
 *        Filename: "MMML/include/mmml/mmml_error.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 16:34:31 2017"
 *         Updated: "2017-10-03 16:39:51 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include <ostream>
#include <string>

namespace mmml
{

struct Types {
    static Type::const_pointer recursive_type;
    static Type::const_pointer bool_type;
    static Type::const_pointer char_type;
    static Type::const_pointer int_type;
    static Type::const_pointer float_type;
    static Type::const_pointer nil_type;
};

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

        err() << "Error at line" << ctx->getStart()->getLine()
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

    ostream& info(antlr4::ParserRuleContext* ctx, const string message="")
    {
        if (!ctx)
            return out();

        out() << "Into: at line" << ctx->getStart()->getLine()
              << ", column " << ctx->getStart()->getCharPositionInLine()
              << " : ";

        if (message.length() > 0)
            out() << message << endl;

        return out();
    }
};


} // end namespace mmml ///////////////////////////////////////////////////////
