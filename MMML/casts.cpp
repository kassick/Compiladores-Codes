/****************************************************************************
 *        Filename: "MMML/casts.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 17:25:16 2017"
 *         Updated: "2017-10-05 01:49:24 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/casts.H"
#include "mmml/basic_types.H"
#include "mmml/error.H"
#include "mmml/utils.H"

namespace mmml {

Type::const_pointer gen_coalesce_code(
    Type::const_pointer ltype, CodeContext::pointer lcode,
    Type::const_pointer rtype, CodeContext::pointer rcode)
{
    if (ltype->equals(rtype))
        return ltype;

    // Basic Types:
    // char -> int
    // char -> float
    // char -> bool
    // int -> float
    // int -> bool
    // float -> bool
    // Special cases:
    // nil -> any[]

    // Dirty Trick: basic_types.cpp register the basic types in the following order:
    // - char          id 0
    // - int           id 1
    // - float         id 2
    // - bool          id 3
    // - nil           id 4
    // ANY SEQUENCE OR TUPLE OR USED DEFINED TYPE HAS HIGHER ID
    // By taking the type with the bigger id, we can upcast without a lot of ifs

    Type::const_pointer coalesced = ltype->id() > rtype->id() ? ltype : rtype ;
    Type::const_pointer from;
    CodeContext::pointer code_ctx;

    if (coalesced == ltype) {
        from = rtype;
        code_ctx = rcode;
    } else {
        from = ltype;
        code_ctx = lcode;
    }

    // Basic -> Nil :: ERROR
    if (coalesced->as<NilType>())
        return nullptr;

    // Sequence / nil ?
    if (coalesced->as<SequenceType>() && from->as<NilType>()) {

        *code_ctx << Instruction("acreate", {0}).with_annot("type (" + coalesced->name() + ")");

    } else if (coalesced->is_basic() && from->is_basic())
        return gen_cast_code(nullptr, from, coalesced, code_ctx);

    return nullptr;
}


Type::const_pointer gen_cast_code(
    antlr4::ParserRuleContext * ctx,
    Type::const_pointer source_type,
    Type::const_pointer dest_type,
    CodeContext::pointer code_ctx,
    bool sanitize_bool)
{
    // cast to same type?
    if (source_type->equals(dest_type)) {
        // float 1.5
        // Nothing to do
        return source_type;
    }

    // HERE: ain't null, both are basic, source ain't nil
    if (dest_type->equals(Types::bool_type)) {
        // Bool: Anything that ain't zero should be true

        if (source_type->equals(Types::nil_type)) {
            // corner case: bool nil
            // just pop and push 0
            *code_ctx << Instruction("pop")
                      << Instruction("push", {0}).with_annot("type(bool)");

            return Types::bool_type;
        }

        // sequence cast to bool: is it 0-len?
        // get len and then convert it to 0 or 1
        if (source_type->as<SequenceType>()) {

            *code_ctx << Instruction("alen").with_annot("type(bool)");

        } else if (!source_type->is_basic()) {
            // not sequence, not basic, error
            return nullptr;
        }

        // Sequence or basic: value on top of stack. Should we sanitize?
        if (sanitize_bool)
        {
            // convert top value to 0 or 1
            string lfalse = LabelFactory::make();
            string lcont = LabelFactory::make();

            *code_ctx << Instruction("bz", {lfalse}).with_annot("cast bool")
                      << Instruction("push", {1}).with_annot("type(bool)")
                      << Instruction("jump", {lcont})
                      << Instruction("push", {0}).with_label(lfalse).with_annot("type(bool)")
                      << Instruction("nop", {}).with_label(lcont);
        }

        return Types::bool_type;

    } else if (dest_type->equals(Types::float_type)) {
        // Float -> Float (handled above)
        // Int -> Float
        // Char -> Float
        // Bool -> Float (0 or 1)

        // anything to float, except nil, which is handled above
        *code_ctx << Instruction("cast_d").with_annot("type(float)");

        return Types::float_type;

    } else if (dest_type->equals(Types::int_type)) {

        // Float -> int : WARN
        // Int -> int : (handled above)
        // Char -> int : ok
        // Bool -> int : 0 or 1

        // Downcast Float -> Int:
        if (source_type->equals(Types::float_type)) {
            Report::warn(ctx) << "Casting from float may discard precision" << endl;
        }

        // bool is implemented as int, don't cast if it's a bool from source
        if (!source_type->equals(Types::bool_type))
            *code_ctx << Instruction("cast_i").with_annot("type(int)");

        return Types::int_type;

    } else if (dest_type->equals(Types::char_type)) {
        // Float -> Char : WARN
        // Int -> Char : Downcast, but expected
        // Char -> Char : Same
        // Bool -> Char : 0 or 1

        // Downcast:
        if (source_type->equals(Types::float_type)) {
            Report::warn(ctx) << "Casting from float may discard precision" << endl;
        }

        // Downcast from int is expected, do not check

        *code_ctx << Instruction("cast_c").with_annot("type(char)");
        return Types::char_type;

    } else if (source_type->equals(Types::bool_type)) {
        // convert top value to 0 or 1
        string lfalse = LabelFactory::make();
        string lcont = LabelFactory::make();

        *code_ctx << Instruction("bz", {lfalse}).with_annot("cast bool")
                  << Instruction("push", {1}).with_annot("type(bool)")
                  << Instruction("jump", {lcont})
                  << Instruction("push", {0}).with_label(lfalse).with_annot("type(bool)")
                  << Instruction("nop", {}).with_label(lcont);

        if (dest_type->equals(Types::char_type))
        {
            *code_ctx << Instruction("cast_c").with_annot("type(char)");
            return Types::char_type;
        }
        else if (dest_type->equals(Types::int_type))
        {
            *code_ctx << Instruction("cast_i").with_annot("type(int)");
            return Types::int_type;
        }
        else if (dest_type->equals(Types::float_type))
        {
            *code_ctx << Instruction("cast_d").with_annot("type(float)");
            return Types::float_type;
        }
        else {
            Report::err(ctx) << "Could not cast boolean to non-primitive type" << endl;
            return nullptr;
        }
    }

    return nullptr;
}


} // end namespace mmml; //////////////////////////////////////////////////////
