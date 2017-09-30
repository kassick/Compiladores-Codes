/****************************************************************************
 *        Filename: "MMML/include/mmml/CodeContext.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:33:27 2017"
 *         Updated: "2017-09-29 21:10:11 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/NestedSymbolTable.H"
#include "mmml/InstructionBlock.H"
#include "mmml/basic_types.H"

namespace mmml {

struct CodeContext
{
    typedef shared_ptr<CodeContext> pointer;
    typedef shared_ptr<const CodeContext> const_pointer;
    typedef weak_ptr<CodeContext> weak_pointer;
    typedef weak_ptr<const CodeContext> const_weak_pointer;

    int nerrors = 0;

    NestedSymbolTable::pointer symbol_table;
    InstructionBlock::pointer code;
    Type::const_pointer rtype;

    CodeContext() {
        symbol_table = make_shared<NestedSymbolTable>();
        code = nullptr;
        rtype = nullptr;
    }

    CodeContext(NestedSymbolTable::pointer symbols) {
        symbol_table = symbols;
        code = nullptr;
        rtype = nullptr;
    }

    CodeContext::pointer create_context() {
        return make_shared<CodeContext>(this->symbol_table);
    }
};

} // end namespace mmmml //////////////////////////////////////////////////////

static inline
mmml::CodeContext& operator<<(mmml::CodeContext& ctx, const mmml::Instruction& i) {
    *(ctx.code) << i;
    return ctx;
}
