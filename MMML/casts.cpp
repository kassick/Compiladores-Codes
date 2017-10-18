/****************************************************************************
 *        Filename: "MMML/casts.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 17:25:16 2017"
 *         Updated: "2017-10-18 15:29:56 kassick"
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

/*-----------------------------------------------------------------------------
 * Function: gen_coalesce_code
 *
 * Description: Tries to force the lowest type to the highest type
 *
 * Basic Types:
 *       char -> int -> float -> bool
 *
 * Sequence Types:
 *       nil -> T[] -> bool  (for any T)
 *       nil -> bool (false)
 *
 * Anything that can cast to bool:
 *       T -> BooleanBranchCode
 *       Use extra instructions to define where it should beanch to.
 *
 *       gen_coalesce_code(bool_type, lcode, BooleanBranchCode, rcode,
 *                         {Instruction("bz", {"l0"}),
 *                          Instruction("jump", {"l1"})})
 *---------------------------------------------------------------------------*/
Type::const_pointer gen_coalesce_code(
    antlr4::ParserRuleContext * ctx,
    Type::const_pointer ltype, CodeContext::pointer lcode,
    Type::const_pointer rtype, CodeContext::pointer rcode,
    const std::vector<Instruction> &extra_instructions)
{
    if (ltype->equals(rtype))
        return ltype;

    // Dirty Trick: basic_types.cpp register the basic types in the following order:
    // - char          id 0
    // - int           id 1
    // - float         id 2
    // - bool          id 3
    // - nil           id 4
    // - BooleanCast   id MAX_INT
    // ANY SEQUENCE OR TUPLE OR USED DEFINED TYPE HAS HIGHER ID
    // By taking the type with the bigger id, we can upcast without a lot of ifs

    Type::const_pointer from;
    CodeContext::pointer code_ctx;
    Type::const_pointer ret_type;
    Type::const_pointer coalesced = ltype->id() > rtype->id() ? ltype : rtype ;

    // cerr << "Coalescing types " << ltype->name() << " and " << rtype->name() << endl;
    // cerr << "Coalescing types " << ltype->id() << " and " << rtype->id() << endl;
    // cerr << "Coalesced: " << coalesced->name() << endl;

    if (coalesced == ltype) {
        from = rtype;
        code_ctx = rcode;
    } else {
        from = ltype;
        code_ctx = lcode;
    }

    // Recursive vs Anything -- accept anything. On the second pass the casts will be called
    if (from->equals(Types::recursive_type))
        return coalesced;

    // Basic -> Nil :: ERROR
    if (coalesced->as<NilType>())
        return nullptr;

    // Sequence / nil ?
    if (coalesced->as<SequenceType>() && from->as<NilType>()) {

        *code_ctx << Instruction("acreate", {0}).with_annot("type (" + coalesced->name() + ")");
        ret_type = coalesced;

    } else if (coalesced->is_basic() && from->is_basic()) {
        auto r = gen_cast_code(ctx, from, coalesced, code_ctx);
        if (!r) // can't cast?
            return nullptr;

        ret_type = coalesced;

    } else if (coalesced->as<BooleanBranchCode>()) {

        // target is a branch code, cast origin from bool
        auto t = gen_cast_code(ctx, from, Types::bool_type, code_ctx);

        if (!t) // can't cast to bool? bail!
            return nullptr;

        ret_type = coalesced;
    }

    if (ret_type)
        for (const auto& instr : extra_instructions)
            *code_ctx << instr;

    return ret_type;
}

Type::const_pointer gen_cast_code(
    antlr4::ParserRuleContext * ctx,
    Type::const_pointer source_type,
    Type::const_pointer dest_type,
    CodeContext::pointer code_ctx,
    bool sanitize_bool)
{
    if (!source_type || !dest_type)
        return nullptr;

    // cast to same type?
    if (source_type->equals(dest_type)) {
        // float 1.5
        // Nothing to do
        return source_type;
    }

    // nil -> T[] , nil is already an empty sequence, just accept it
    if (dest_type->as<SequenceType>() && source_type->equals(Types::nil_type)) {
        return dest_type;
    }

    if (source_type->as<TupleType>() || source_type->as<ClassType>() ||
        dest_type->as<TupleType>() || dest_type->as<ClassType>())
    {
        // These can't be cast to anything!
        return nullptr;
    }

    // ALMOST anything bool
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

        if (!source_type->is_basic())
            return nullptr;

        // anything to float, except nil, which is handled above
        *code_ctx << Instruction("cast_d").with_annot("type(float)");

        return Types::float_type;

    } else if (dest_type->equals(Types::int_type)) {

        // Float -> int : WARN
        // Int -> int : (handled above)
        // Char -> int : ok
        // Bool -> int : 0 or 1

        if (!source_type->is_basic())
            return nullptr;

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

        if (!source_type->is_basic())
            return nullptr;

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

        if (!source_type->is_basic())
            return nullptr;

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
