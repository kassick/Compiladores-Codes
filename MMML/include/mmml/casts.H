/****************************************************************************
 *        Filename: "MMML/casts.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 17:23:52 2017"
 *         Updated: "2017-10-05 01:06:05 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/Type.H"
#include "mmml/CodeContext.H"
#include "antlr4-runtime.h"

namespace mmml {

Type::const_pointer gen_cast_code(antlr4::ParserRuleContext*,
                                  Type::const_pointer source_type,
                                  Type::const_pointer dest_type,
                                  CodeContext::pointer code_ctx,
                                  bool sanitize_bool = false);

Type::const_pointer gen_coalesce_code(
    Type::const_pointer, CodeContext::pointer,
    Type::const_pointer, CodeContext::pointer );

}
