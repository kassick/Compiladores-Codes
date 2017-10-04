/****************************************************************************
 *        Filename: "MMML/include/mmml/CodeContext.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:33:27 2017"
 *         Updated: "2017-10-04 16:52:38 kassick"
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

    NestedSymbolTable::pointer symbol_table;
    InstructionBlock::pointer code;

    CodeContext(NestedSymbolTable::pointer symbols, InstructionBlock::pointer code)
            : symbol_table(symbols),
              code(code)
    {}

    CodeContext(NestedSymbolTable::pointer symbols)
            : CodeContext(symbols, make_shared<InstructionBlock>())
    {}

    CodeContext()
            : CodeContext(make_shared<NestedSymbolTable>(),
                          make_shared<InstructionBlock>())
    {}



    CodeContext::pointer create_block() {
        return make_shared<CodeContext>(this->symbol_table);
    }

    CodeContext::pointer create_subcontext() {
        return make_shared<CodeContext>(this->symbol_table->make_nested());
    }
};

} // end namespace mmmml //////////////////////////////////////////////////////

static inline
mmml::CodeContext& operator<<(mmml::CodeContext& ctx, const mmml::Instruction& i) {
    *(ctx.code) << i;
    return ctx;
}

static inline
mmml::CodeContext& operator<<(mmml::CodeContext& ctx, const mmml::CodeContext& other) {
    *(ctx.code) << *(other.code);
    return ctx;
}

static inline
mmml::CodeContext& operator<<(mmml::CodeContext& ctx, const mmml::CodeContext&& other) {
    *(ctx.code) << std::move(*(other.code));
    return ctx;
}
