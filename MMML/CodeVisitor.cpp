/****************************************************************************
 *        Filename: "MMML/CodeVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:44:30 2017"
 *         Updated: "2017-09-30 01:03:10 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/CodeVisitor.H"
#include "mmml/Instruction.H"

namespace mmml {

using namespace std;
using stringvector = std::vector<std::string>;

antlrcpp::Any CodeVisitor::visitLiteral_float_rule(MMMLParser::Literal_float_ruleContext *ctx)
{
    *code_ctx <<
            Instruction("push", { ctx->getText() })
            .with_annot("type(float)" );
    return float_type;
}

antlrcpp::Any CodeVisitor::visitLiteral_decimal_rule(MMMLParser::Literal_decimal_ruleContext *ctx)  {
    *code_ctx <<
            Instruction("push",
                        { ctx->getText() })
            .with_annot("type(int)");
    return int_type;
}

antlrcpp::Any CodeVisitor::visitLiteral_hexadecimal_rule(MMMLParser::Literal_hexadecimal_ruleContext *ctx)  {
    *code_ctx <<
            Instruction("push",
                        { ctx->getText() })
            .with_annot("type(int)");
    return code_ctx;
}

antlrcpp::Any CodeVisitor::visitLiteral_binary_rule(MMMLParser::Literal_binary_ruleContext *ctx)  {
    *code_ctx <<
            Instruction("push",
                        { ctx->getText() })
            .with_annot( "type(int)" );
    return int_type;
}

antlrcpp::Any CodeVisitor::visitLiteralstring_rule(MMMLParser::Literalstring_ruleContext *ctx)  {
    *code_ctx << Instruction("push", { ctx->getText() })
              <<
            Instruction("acreate",
                        {ctx->getText().length() - 2})
            .with_annot("type(str)");

    return type_registry.add(make_shared<SequenceType>(char_type));
}

antlrcpp::Any CodeVisitor::visitLiteral_char_rule(MMMLParser::Literal_char_ruleContext *ctx)  {
    *code_ctx << Instruction("push", { ctx->getText() })
            .with_annot("type(char)" );
    return char_type;
}

antlrcpp::Any CodeVisitor::visitLiteraltrueorfalse_rule(MMMLParser::Literaltrueorfalse_ruleContext *ctx)  {

    *code_ctx <<
            Instruction("push", {( ctx->getText() == "true" ? 1 : 0 ) })
            .with_annot("type(int)");
    return int_type;
}

antlrcpp::Any
CodeVisitor::visitLiteralnil_rule(MMMLParser::Literalnil_ruleContext *ctx) {

  *code_ctx << Instruction("push", {"null"}).with_annot("type(nil)");

  return nil_type;
}

antlrcpp::Any CodeVisitor::visitSymbol_rule(MMMLParser::Symbol_ruleContext *ctx)  {
    auto s = this->code_ctx->symbol_table->find(ctx->getText());
    if (!s) {
        report_error(ctx) << " : Unknown symbol " << ctx->getText()
                          << endl;

        *code_ctx << Instruction("push", {0}).with_annot("type(int)");
        return int_type;
    }

    *code_ctx <<
            Instruction("load", {s->pos})
            .with_annot("type(" + type_name(s->type()) + ")");

    return s->type();
}

// Cast rules
antlrcpp::Any CodeVisitor::visitCast_rule(MMMLParser::Cast_ruleContext *ctx)
{
    auto source_type = visit(ctx->funcbody()).as<Type::const_pointer>();
    auto dest_type = type_registry.find_by_name(ctx->c->getText());

    if (!source_type) {
        report_error(ctx) << "While parsing cast: gor null source type!"
                          << endl;

        goto default_cast_to_int;
    }

    if (!dest_type) {
        report_error(ctx) << "Unknown type ``" << ctx->c->getText() << "''"
                          << " in cast"
                          << endl;
        goto default_cast_to_int;
    }

    return gen_cast_code(ctx, source_type, dest_type);

default_cast_to_int:
    *code_ctx << Instruction("pop")
              << Instruction("push", {0}).with_annot("type(int)");

    return int_type;
}

Type::const_pointer CodeVisitor::gen_cast_code(antlr4::ParserRuleContext * ctx,
                                               Type::const_pointer source_type,
                                               Type::const_pointer dest_type)
{
    // cast to same type?
    if (source_type->equals(dest_type)) {
        // float 1.5
        // Nothing to do
        return source_type;
    }

    // HERE: ain't null, both are basic, source ain't nil
    if (dest_type->equals(bool_type)) {
        // Bool: Anything that ain't zero should be true

        if (source_type->equals(nil_type)) {
            // corner case: bool nil
            // just pop and push 0
            *code_ctx << Instruction("pop")
                      << Instruction("push", {0}).with_annot("type(bool)");
        } else {

            // sequence cast to bool: is it 0-len?
            // get len and then convert it to 0 or 1
            if (source_type->as<SequenceType>()) {

                *code_ctx << Instruction("alen").with_annot("type(bool)");
            }

            // convert top value to 0 or 1
            string lfalse = LabelFactory::make();
            string lcont = LabelFactory::make();

            *code_ctx << Instruction("bz", {lfalse}).with_annot("cast bool")
                      << Instruction("push", {1}).with_annot("type(bool)")
                      << Instruction("jump", {lcont})
                      << Instruction("push", {0}).with_label(lfalse).with_annot("type(bool)")
                      << Instruction("nop", {}).with_label(lcont);
        }

        return bool_type;

    } else if (dest_type->equals(float_type)) {
        // Float -> Float (handled above)
        // Int -> Float
        // Char -> Float
        // Bool -> Float (0 or 1)

        // anything to float, except nil, which is handled above
        *code_ctx << Instruction("cast_d").with_annot("type(float)");

        return float_type;

    } else if (dest_type->equals(int_type)) {

        // Float -> int : WARN
        // Int -> int : (handled above)
        // Char -> int : ok
        // Bool -> int : 0 or 1

        // Downcast Float -> Int:
        if (source_type->equals(float_type)) {
            warn(ctx) << "Casting from float may discard precision" << endl;
        }

        // bool is implemented as int, don't cast if it's a bool from source
        if (!source_type->equals(bool_type))
            *code_ctx << Instruction("cast_i").with_annot("type(int)");

        return int_type;

    } else if (dest_type->equals(char_type)) {
        // Float -> Char : WARN
        // Int -> Char : Downcast, but expected
        // Char -> Char : Same
        // Bool -> Char : 0 or 1

        // Downcast:
        if (source_type->equals(float_type)) {
            warn(ctx) << "Casting from float may discard precision" << endl;
        }

        // Downcast from int is expected, do not check

        *code_ctx << Instruction("cast_c").with_annot("type(char)");
        return int_type;
    }

    err() << "SHOULD NEVER REATH THIS POINT IN GEN_CAST_CODE" << endl;
    return int_type;
}

antlrcpp::Any CodeVisitor::visitSeq_create_seq(MMMLParser::Seq_create_seqContext *ctx)
{
    // Assume base is already in the registry

    auto base_type = visit( ctx->funcbody() ).as<Type::const_pointer>();

    if (!base_type) {
        report_error(ctx) << "IMPL ERROR: Got null type from funcbody" << endl;
        base_type = int_type;
    }

    // Filter this through the registry, de-dup
    auto seq_type = type_registry.add(make_shared<SequenceType>(base_type));

    assert(seq_type);

    *code_ctx <<
            Instruction("acreate", {1})
            .with_annot("type(" + type_name(seq_type) + ")");

    return seq_type;
}

// Static storage for out_stream and err_stream
ostream* CodeVisitor::out_stream = nullptr ;
ostream* CodeVisitor::err_stream = nullptr ;

} // end namespace mmml ///////////////////////////////////////////////////////
