/****************************************************************************
 *        Filename: "MMML/CodeVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:44:30 2017"
 *         Updated: "2017-10-04 14:30:30 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/CodeVisitor.H"
#include "mmml/Instruction.H"
#include "mmml/FunctionRegistry.H"
#include "mmml/error.H"
#include "mmml/TypedArgListVisitor.H"
#include "mmml/casts.H"
#include "mmml/utils.H"

namespace mmml {

using namespace std;
using stringvector = std::vector<std::string>;

template <class ValidTypePred>
Type::const_pointer generic_bin_op(CodeVisitor* visitor,
                                   antlr4::ParserRuleContext* left,
                                   antlr4::ParserRuleContext* right,
                                   CodeContext::pointer code_ctx,
                                   ValidTypePred pred)
{
    CodeVisitor leftvisitor, rightvisitor;

    auto rcode = code_ctx->create_block();
    auto lcode = code_ctx->create_block();

    leftvisitor.code_ctx = lcode;
    rightvisitor.code_ctx = rcode;

    Type::const_pointer ltype = leftvisitor.visit(left);
    if (!ltype)
    {
        Report::err(left) << "IMPL ERROR GOT NULL FROM LEFT ON BIN OP" << endl;
        *code_ctx << Instruction("pop").with_annot("pop left");
        return Types::int_type;
    }

    Type::const_pointer rtype = rightvisitor.visit(right);
    if (!rtype)
    {
        Report::err(right) << "IMPL ERROR GOT NULL FROM RIGHT ON BIN OP" << endl;
        *code_ctx << Instruction("pop").with_annot("pop right");
        return Types::int_type;
    }

    // Try to get both to be the same type
    auto coalesced_type = gen_coalesce_code(ltype, lcode, rtype, rcode);

    if (!coalesced_type || !pred(coalesced_type))
    {
        // Required upcasting did not result in expected type

        Report::err(left) << "Could not coerce types "
                                        << ltype->name() << " and " << rtype->name();

        *code_ctx << Instruction("pop").with_annot("pop right");
        *code_ctx << Instruction("pop").with_annot("pop left");

        return nullptr;
    }

    *code_ctx << *lcode << *rcode;

    return coalesced_type;
}




antlrcpp::Any CodeVisitor::visitLiteral_float_rule(MMMLParser::Literal_float_ruleContext *ctx)
{
    *code_ctx <<
            Instruction("push", { ctx->getText() })
            .with_annot("type(float)" );
    return Types::float_type;
}

antlrcpp::Any CodeVisitor::visitLiteral_decimal_rule(MMMLParser::Literal_decimal_ruleContext *ctx)  {
    *code_ctx <<
            Instruction("push",
                        { ctx->getText() })
            .with_annot("type(int)");
    return Types::int_type;
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
    return Types::int_type;
}

antlrcpp::Any CodeVisitor::visitLiteralstring_rule(MMMLParser::Literalstring_ruleContext *ctx)  {
    *code_ctx << Instruction("push", { ctx->getText() })
              <<
            Instruction("acreate",
                        {ctx->getText().length() - 2})
            .with_annot("type(str)");

    return type_registry.add(make_shared<SequenceType>(Types::char_type));
}

antlrcpp::Any CodeVisitor::visitLiteral_char_rule(MMMLParser::Literal_char_ruleContext *ctx)  {
    *code_ctx << Instruction("push", { ctx->getText() })
            .with_annot("type(char)" );
    return Types::char_type;
}

antlrcpp::Any CodeVisitor::visitLiteraltrueorfalse_rule(MMMLParser::Literaltrueorfalse_ruleContext *ctx)  {

    *code_ctx <<
            Instruction("push", {( ctx->getText() == "true" ? 1 : 0 ) })
            .with_annot("type(int)");
    return Types::int_type;
}

antlrcpp::Any
CodeVisitor::visitLiteralnil_rule(MMMLParser::Literalnil_ruleContext *ctx) {

  // *code_ctx << Instruction("push", {"null"}).with_annot("type(nil)");

    return Types::nil_type;
}

antlrcpp::Any CodeVisitor::visitMe_exprsymbol_rule(MMMLParser::Me_exprsymbol_ruleContext *ctx)  {
    auto s = mmml_load_symbol(ctx->symbol()->getText(), code_ctx);
    if (!s) {
        Report::err(ctx) << " : Unknown symbol " << ctx->symbol()->getText()
                          << endl;

        *code_ctx << Instruction("push", {0}).with_annot("type(int)");
        return Types::int_type;
    }

    // mmml_load_symbol inserts load n in the ctx parameter
    return s->type();
}

antlrcpp::Any CodeVisitor::visitMe_boolneg_rule(MMMLParser::Me_boolneg_ruleContext *ctx) {
    // ! sym

    auto s = mmml_load_symbol(ctx->symbol()->getText(), code_ctx);
    if (!s) {
        Report::err(ctx) << " : Unknown symbol " << ctx->getText()
                          << endl;

        *code_ctx << Instruction("push", {0}).with_annot("type(bool)");
        return Types::bool_type;
    }

    // coerce it to bool
    auto coerced_bool_type = gen_cast_code(ctx,
                                           s->type(), Types::bool_type,
                                           code_ctx);

    if (!coerced_bool_type || !coerced_bool_type->equals(Types::bool_type))
    {
        Report::err(ctx) << "Can not coerce type " << s->type()->name()
                          << " to bool";

        *code_ctx << Instruction("pop").with_annot("drop invalid bool cast");
    }

    return Types::bool_type;
}

antlrcpp::Any CodeVisitor::visitMe_boolnegparens_rule(MMMLParser::Me_boolnegparens_ruleContext *ctx)
{
    Type::const_pointer ftype = visit(ctx->funcbody());

    if (!ftype) {
        Report::err(ctx, "IMPL ERROR ON BOOLNEGPARENS");
        *code_ctx << Instruction("pop").with_annot("drop invalid bool cast");
        return Types::bool_type;
    }

    // coerce it to bool
    auto coerced_bool_type = gen_cast_code(ctx,
                                           ftype, Types::bool_type,
                                           code_ctx);

    if (!coerced_bool_type || !coerced_bool_type->equals(Types::bool_type))
    {
        Report::err(ctx) << "Can not coerce type " << ftype->name()
                          << " to bool";

        *code_ctx << Instruction("pop").with_annot("drop invalid bool cast");
    }

    return Types::bool_type;
}


antlrcpp::Any CodeVisitor::visitMe_listconcat_rule(MMMLParser::Me_listconcat_ruleContext *ctx)
{
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<SequenceType>();
                                });

    if (!rtype)
        return type_registry.add(make_shared<SequenceType>(Types::int_type));

    *code_ctx
            << Instruction("mark", {2})
            .with_annot("stack: {2: c, d} {2: a, b} |  # right left")
            << Instruction("push", {0})
            .with_annot("stack: 0 {2: c, d} {2: a, b} |  #int right left")
            << Instruction("swap", {0})
            .with_annot("stack: {2: a, b} {2: c, d} 0 | left right @len")
            << Instruction("aload", {})
            .with_annot("stack: 2 b a {2: c, d} 0 | llen l9 l8 ... r0 right len ")
            << Instruction("store", {0})
            .with_annot("stack: b a {2: c, d} 2 | l9 l8 ... l0 right len")
            << Instruction("load", {1})
            .with_annot("stack: {2: c, d} b a {2: c, d} 2 | right l9 ... l0 right len")
            << Instruction("aload", {})
            .with_annot("stack: 2 d c b a {2: c, d} 2 | rlen r9 .. r0 l9 .. l0 right len")
            << Instruction("load", {0})
            .with_annot("stack: 2 2 d c b a {2: c, d} 2  |")
            << Instruction("add", {})
            .with_annot("stack: 4 d c b a {2: c, d} 2  |")
            << Instruction("acreate", {})
            .with_annot("stack : {4 a b c d} {2: c d} 2 |")
            << Instruction("crunch", {0, 2})
            .with_annot("stack : {4 a b c d} |")
            << Instruction("drop_mark", {})
            .with_annot("type(" + rtype->name() + ")");

    return rtype;
}

antlrcpp::Any CodeVisitor::visitMe_exprmuldiv_rule(MMMLParser::Me_exprmuldiv_ruleContext *ctx)
{
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->is_basic();
                                });

    if (!rtype)
        return Types::int_type;

    *code_ctx
            << Instruction(ctx->op->getText() == "/" ? "div" : "mul").with_annot("type(" + rtype->name() + ")");

    return rtype;
}

antlrcpp::Any CodeVisitor::visitMe_exprplusminus_rule(MMMLParser::Me_exprplusminus_ruleContext *ctx)
{
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->is_basic();
                                });

    if (!rtype)
        return Types::int_type;

    *code_ctx
            << Instruction(ctx->op->getText() == "+" ? "add" : "sub").with_annot("type(" + rtype->name() + ")");

    return rtype;
}


// Cast rules
antlrcpp::Any CodeVisitor::visitCast_rule(MMMLParser::Cast_ruleContext *ctx)
{
    auto source_type = visit(ctx->funcbody()).as<Type::const_pointer>();
    auto dest_type = type_registry.find_by_name(ctx->c->getText());

    if (!source_type) {
        Report::err(ctx) << "While parsing cast: gor null source type!"
                          << endl;

        goto default_cast_to_int;
    }

    if (!dest_type) {
        Report::err(ctx) << "Unknown type ``" << ctx->c->getText() << "''"
                          << " in cast"
                          << endl;
        goto default_cast_to_int;
    }

    return gen_cast_code(ctx, source_type, dest_type, code_ctx);

default_cast_to_int:
    *code_ctx << Instruction("pop")
              << Instruction("push", {0}).with_annot("type(int)");

    return Types::int_type;
}



antlrcpp::Any CodeVisitor::visitSeq_create_seq(MMMLParser::Seq_create_seqContext *ctx)
{
    // Assume base is already in the registry

    auto base_type = visit( ctx->funcbody() ).as<Type::const_pointer>();

    if (!base_type) {
        Report::err(ctx) << "IMPL ERROR: Got null type from funcbody" << endl;
        base_type = Types::int_type;
    }

    // Filter this through the registry, de-dup
    auto seq_type = type_registry.add(make_shared<SequenceType>(base_type));

    assert(seq_type);

    *code_ctx <<
            Instruction("acreate", {1})
            .with_annot("type(" + type_name(seq_type) + ")");

    return seq_type;
}
antlrcpp::Any CodeVisitor::visitMe_class_get_rule(MMMLParser::Me_class_get_ruleContext *ctx)
{
    Type::const_pointer field_type;
    ClassType::const_pointer class_type;
    int pos;

    auto funcbody_type = visit(ctx->funcbody()).as<Type::const_pointer>();
    // top is tuple, get specific position

    if (!funcbody_type) {
        Report::err(ctx) << "IMPL ERROR god null from funcbody" << endl;
        goto default_return_int;
    }

    class_type = funcbody_type->as<ClassType>();
    if (!class_type) {
        Report::err(ctx) << "Using class accessor method get on type ``"
                          << funcbody_type->name() << "''"
                          << endl;
        goto default_return_int;
    }

    pos = class_type->get_pos(ctx->name->getText());
    field_type = class_type->get_type(ctx->name->getText());

    if (!field_type || pos < 0) {
        Report::err(ctx) << "Could not find field named ``"
                          << ctx->name->getText()
                          << "'' in class type "
                          << class_type->name()
                          << endl;
        goto default_return_int;
    }

    *code_ctx << Instruction("aget", {pos}).with_annot("type(" + field_type->name() + ")");

    return field_type;


default_return_int:
    *code_ctx << Instruction("pop").with_annot("Panic!")
              << Instruction("push", {0}).with_annot("type(int)");
    return Types::int_type;
}

antlrcpp::Any CodeVisitor::visitMe_class_set_rule(MMMLParser::Me_class_set_ruleContext *ctx)
{
    Type::const_pointer field_type, cl_funcbody_type, val_funcbody_type;
    ClassType::const_pointer class_type;
    int pos;

    cl_funcbody_type = visit(ctx->cl).as<Type::const_pointer>();

    if (!cl_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR god null from cl funcbody" << endl;

        *code_ctx << Instruction("pop").with_annot("drop class")
                  << Instruction("push", {0}).with_annot("type(int)");

        return Types::int_type;
    }

    val_funcbody_type = visit(ctx->val).as<Type::const_pointer>();
    if (!val_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR god null from val funcbody" << endl;
        *code_ctx << Instruction("pop").with_annot("drop val");
        return cl_funcbody_type;
    }

    class_type = cl_funcbody_type->as<ClassType>();
    if (!class_type) {
        Report::err(ctx) << "Using class accessor method set on type ``"
                          << cl_funcbody_type->name() << "''"
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop val");
        return cl_funcbody_type;
    }

    pos = class_type->get_pos(ctx->name->getText());
    field_type = class_type->get_type(ctx->name->getText());

    if (!field_type || pos < 0) {
        Report::err(ctx) << "Could not find field named ``"
                          << ctx->name->getText()
                          << "'' in class type "
                          << class_type->name()
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop val");
        return cl_funcbody_type;
    }

    *code_ctx << Instruction("aset", {pos}).with_annot("type(" + class_type->name() + ")");

    return class_type;
}

antlrcpp::Any CodeVisitor::visitMe_tuple_set_rule(MMMLParser::Me_tuple_set_ruleContext *ctx)
{
    Type::const_pointer field_type, tup_funcbody_type, val_funcbody_type;
    TupleType::const_pointer tuple_type;
    int pos;

    tup_funcbody_type = visit(ctx->tup).as<Type::const_pointer>();
    // top is tuple, get specific position

    if (!tup_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR got null from tup funcbody" << endl;
        *code_ctx << Instruction("pop").with_annot("drop tup")
                  << Instruction("push", {0}).with_annot("type(int)");
        return Types::int_type;
    }

    tuple_type = tup_funcbody_type->as<TupleType>();
    if (!tuple_type) {
        Report::err(ctx) << "Using tuple accessor method get on type ``"
                          << tup_funcbody_type->name() << "''"
                          << endl;
        return tup_funcbody_type;
    }

    val_funcbody_type = visit(ctx->val).as<Type::const_pointer>();
    if (!val_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR got null from val funcbody" << endl;
        *code_ctx << Instruction("pop").with_annot("drop val");
        return tup_funcbody_type;
    }

    pos = stoi(ctx->pos->getText());
    field_type = tuple_type->get_nth_type(pos);

    if (!field_type) {
        Report::err(ctx) << "Access to invalid position " << pos
                          << " on tuple of type"
                          << tuple_type->name()
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop val");
        return tup_funcbody_type;
    }

    *code_ctx << Instruction("aset", {pos}).with_annot("type(" + tuple_type->name() + ")");

    return field_type;
}

antlrcpp::Any CodeVisitor::visitMe_tuple_get_rule(MMMLParser::Me_tuple_get_ruleContext *ctx)
{
    Type::const_pointer field_type;
    TupleType::const_pointer tuple_type;
    int pos;

    auto funcbody_type = visit(ctx->funcbody()).as<Type::const_pointer>();
    // top is tuple, get specific position

    if (!funcbody_type) {
        Report::err(ctx) << "IMPL ERROR god null from funcbody" << endl;
        goto default_return_int;
    }

    tuple_type = funcbody_type->as<TupleType>();
    if (!tuple_type) {
        Report::err(ctx) << "Using tuple accessor method get on type ``"
                          << funcbody_type->name() << "''"
                          << endl;
        goto default_return_int;
    }

    pos = stoi(ctx->pos->getText());
    field_type = tuple_type->get_nth_type(pos);

    if (!field_type) {
        Report::err(ctx) << "Access to invalid position " << pos
                          << " on tuple of type"
                          << tuple_type->name()
                          << endl;
        goto default_return_int;
    }

    *code_ctx << Instruction("aget", {pos}).with_annot("type(" + field_type->name() + ")");

    return field_type;

default_return_int:
    *code_ctx << Instruction("pop").with_annot("Panic!")
              << Instruction("push", {0}).with_annot("type(int)");
    return Types::int_type;
}

antlrcpp::Any CodeVisitor::visitTuple_ctor(MMMLParser::Tuple_ctorContext *ctx)
{
    TupleType::base_types_vector_t types;
    Type::pointer _tup;
    TupleType::const_pointer tuple;

    int i = 0;

    for(auto funcbody: ctx->funcbody())
    {
        auto type = visit(funcbody);
        if (!type) {
            Report::err(funcbody) << "IMPL ERROR: GOT NULL FROM TUPLE CTOR FUNCBODY #" << i;
            goto default_return_int;
        }

        types.push_back(type);
    }

    _tup = make_shared<TupleType>(types);
    tuple = type_registry.add(_tup)->as<TupleType>();

    if (!tuple) {
        Report::err(ctx) << "Could not create tuple type";
        goto default_return_int;
    }

    return tuple;

default_return_int:
    while (i-- > 0)
        *code_ctx << Instruction("drop").with_annot("drop " + to_string(i));
    *code_ctx << Instruction("push", {0}).with_annot("type(int)");

    return Types::int_type;

}




} // end namespace mmml ///////////////////////////////////////////////////////
