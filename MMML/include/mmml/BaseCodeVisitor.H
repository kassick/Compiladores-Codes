/****************************************************************************
 *        Filename: "MMML/include/mmml/BaseCodeVisitor.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 18:10:23 2017"
 *         Updated: "2017-10-03 18:34:36 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/CodeContext.H"
#include "MMMLBaseVisitor.h"

namespace mmml {

class BaseCodeVisitor : public MMMLBaseVisitor {

  public:

    // Instance Variables /////////////////////////////////////////////////////
    CodeContext::pointer code_ctx;

    BaseCodeVisitor(CodeContext::pointer code_ctx)
            : code_ctx(code_ctx)
    {}

    BaseCodeVisitor(const BaseCodeVisitor& other)
            : BaseCodeVisitor(other.code_ctx)
    {}

    BaseCodeVisitor() : BaseCodeVisitor(make_shared<CodeContext>())
    {}
};

// static inline
// BaseCodeVisitor& operator<<(BaseCodeVisitor& base, const BaseCodeVisitor& other)
// {
//     if (base.code_ctx &&
//         base.code_ctx->code &&
//         other.code_ctx &&
//         other.code_ctx->code)
//     {
//         *base.code_ctx << *(other.code_ctx);
//     }

//     return base;
// }

// static inline
// BaseCodeVisitor& operator<<(BaseCodeVisitor& base, BaseCodeVisitor&& other)
// {
//     if (base.code_ctx &&
//         base.code_ctx->code &&
//         other.code_ctx &&
//         other.code_ctx->code)
//     {
//         *(base.code_ctx) << std::move(*other.code_ctx);
//     }

//     return base;

// }

// }

// namespace std {

// static inline
// ostream& operator<<(ostream& out, const mmml::BaseCodeVisitor& v)
// {
//     if (v.code_ctx) {
//         if (v.code_ctx->symbol_table) {
//             out << v.code_ctx->symbol_table->to_string()
//                 << endl;
//         }

//         if (v.code_ctx->code)
//             out << *v.code_ctx->code
//                 << endl;
//     }
// }


}
