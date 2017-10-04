/****************************************************************************
 *        Filename: "MMML/utils.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 17:33:29 2017"
 *         Updated: "2017-10-04 17:29:23 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/CodeContext.H"
#include "antlr4-runtime.h"

namespace mmml
{

struct LabelFactory {

    static string make() {
        static int next = 0;

        return "l" + to_string(next++);
    }
};

struct SymbolFactory {

    static string make() {
        static int next = 0;

        return "@s" + to_string(next++);
    }
};

static inline
Symbol::const_pointer mmml_load_symbol(const string name, CodeContext::pointer code_ctx)
{
    auto s = code_ctx->symbol_table->find(name);
    if (s) {
        *code_ctx << Instruction("load", {s->pos}).with_annot("type(" + type_name(s->type()) + ")");
        return s;
    }

    return nullptr;
}

}
