/****************************************************************************
 *        Filename: "MMML/MetaExprVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:44:30 2017"
 *         Updated: "2017-10-10 18:41:56 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/MetaExprVisitor.H"
#include "mmml/Instruction.H"
#include "mmml/FunctionRegistry.H"
#include "mmml/error.H"
#include "mmml/TypedArgListVisitor.H"
#include "mmml/casts.H"
#include "mmml/utils.H"
#include "mmml/FuncbodyVisitor.H"

namespace mmml {

using namespace std;
using stringvector = std::vector<std::string>;

antlrcpp::Any
MetaExprVisitor::visitMe_exprparens_rule(MMMLParser::Me_exprparens_ruleContext *ctx)
{
    FuncbodyVisitor fbvisitor(code_ctx->create_block());
    fbvisitor.ltrue = this->ltrue;
    fbvisitor.lfalse = this->lfalse;
    fbvisitor.lout = this->lout;

    Type::const_pointer ret = fbvisitor.visit(ctx->funcbody());

    if (!ret) {
        Report::err(ctx) << "IMPL ERROR NULL PARENS" << endl;
        return Types::int_type;
    }

    *code_ctx << std::move(*fbvisitor.code_ctx);

    return ret;
}

template <class ValidTypePred>
Type::const_pointer generic_bin_op(
    MetaExprVisitor* visitor,
    antlr4::ParserRuleContext* left,
    antlr4::ParserRuleContext* right,
    CodeContext::pointer code_ctx,
    ValidTypePred pred)
{
    auto rcode = code_ctx->create_block();
    auto lcode = code_ctx->create_block();

    MetaExprVisitor
            leftvisitor(lcode),
            rightvisitor(rcode);

    Type::const_pointer ltype = leftvisitor.visit(left);
    if (!ltype)
    {
        Report::err(left) << "IMPL ERROR GOT NULL FROM LEFT ON BIN OP" << endl;
        return Types::int_type;
    }

    Type::const_pointer rtype = rightvisitor.visit(right);
    if (!rtype)
    {
        Report::err(right) << "IMPL ERROR GOT NULL FROM RIGHT ON BIN OP" << endl;
        return Types::int_type;
    }

    // Try to get both to be the same type
    auto coalesced_type = gen_coalesce_code(ltype, lcode, rtype, rcode);

    if (!coalesced_type || !pred(coalesced_type))
    {
        // Required upcasting did not result in expected type

        Report::err(left) << "Could not coerce types "
                                        << ltype->name() << " and " << rtype->name();

        return nullptr;
    }

    *code_ctx << std::move(*lcode)
              << std::move(*rcode);

    return coalesced_type;
}


antlrcpp::Any MetaExprVisitor::visitMe_bool_and_rule(MMMLParser::Me_bool_and_ruleContext *ctx)
{
    MetaExprVisitor leftvisitor(code_ctx->create_block());
    MetaExprVisitor rightvisitor(code_ctx->create_block());

    string _ltrue = this->ltrue;
    string _lfalse = this->lfalse;

    if (_ltrue.length() == 0) {
        // we are not in a boolean context, we must leave the result on the top of the stack
        _ltrue = LabelFactory::make();
        _lfalse = LabelFactory::make();
    }

    string lnext = LabelFactory::make();

    // Visit Left
    leftvisitor.lfalse = _lfalse;
    leftvisitor.ltrue = lnext; // if true, check next
    Type::const_pointer lt = leftvisitor.visit(ctx->l).as<Type::const_pointer>();

    if (!lt->as<BooleanBranchCode>())
    {
        // return is some type, try to convert to bool
        auto lboolt = gen_cast_code(ctx->l,
                                    lt, /* -> */ Types::bool_type,
                                    leftvisitor.code_ctx,
                                    false);

        if (!lboolt)
            Report::err(ctx->l) << "Could not coerce type " << lt->name()
                                << " to bool";

        *leftvisitor.code_ctx
                << Instruction("bz", {leftvisitor.lfalse});
    }

    // Visit Right
    rightvisitor.lfalse = _lfalse; // if false, jump out
    rightvisitor.ltrue = _ltrue; // if true, jump true
    Type::const_pointer rt = rightvisitor.visit(ctx->r).as<Type::const_pointer>();

    if (!rt->as<BooleanBranchCode>()) {
        auto rboolt = gen_cast_code(ctx->r, rt, Types::bool_type, rightvisitor.code_ctx, false);
        if (!rboolt)
            Report::err(ctx->l) << "Could not coerce type " << rt->name()
                                << " to bool";

        *rightvisitor.code_ctx
                << Instruction("bz", {rightvisitor.lfalse});
    }

    *code_ctx << *leftvisitor.code_ctx
              << Instruction().with_label(lnext)
              << *rightvisitor.code_ctx;

    if (!rt->as<BooleanBranchCode>())
        *code_ctx << Instruction("jump", {_ltrue}).with_annot("AND JUMP TRUE");

    if (this->ltrue.length() == 0) {
        auto _lout = LabelFactory::make();
        // not in boolean context, must push true or false
        *code_ctx << Instruction("push", {1}).with_label(_ltrue)
                  << Instruction("jump", {_lout}).with_annot("AND JUMP OUT")
                  << Instruction("push", {0}).with_label(_lfalse)
                  << Instruction().with_label(_lout);
        return Types::bool_type;
    } else {
        // boolean context
        return Types::boolean_branch;
    }
}


antlrcpp::Any MetaExprVisitor::visitMe_bool_or_rule(MMMLParser::Me_bool_or_ruleContext *ctx)
{
    MetaExprVisitor leftvisitor(code_ctx->create_block());
    MetaExprVisitor rightvisitor(code_ctx->create_block());

    cerr << "visit me bool or" << endl;

    string _ltrue = this->ltrue;
    string _lfalse = this->lfalse;

    if (_ltrue.length() == 0) {
        // we are not in a boolean context, we must leave the result on the top of the stack
        _ltrue = LabelFactory::make();
        _lfalse = LabelFactory::make();
    }

    string lnext = LabelFactory::make();

    // Visit Left
    leftvisitor.lfalse = lnext; // if false, check next
    leftvisitor.ltrue = _ltrue;
    Type::const_pointer lt = leftvisitor.visit(ctx->l).as<Type::const_pointer>();

    if (!lt->as<BooleanBranchCode>())
    {
        // return is some type, try to convert to bool
        auto lboolt = gen_cast_code(ctx->l, lt, Types::bool_type, leftvisitor.code_ctx, false);

        if (!lboolt)
            Report::err(ctx->l) << "Could not coerce type " << lt->name()
                                << " to bool";

        *leftvisitor.code_ctx << Instruction("bnz", {_ltrue});
    }

    // Visit Right
    rightvisitor.lfalse = _lfalse; // if false, jump out
    rightvisitor.ltrue = _ltrue; // if true, jump true
    Type::const_pointer rt = rightvisitor.visit(ctx->r).as<Type::const_pointer>();

    if (!rt->as<BooleanBranchCode>()) {
        auto rboolt = gen_cast_code(ctx->r, rt, Types::bool_type, rightvisitor.code_ctx, false);
        if (!rboolt)
            Report::err(ctx->l) << "Could not coerce type " << rt->name()
                                << " to bool";

        *rightvisitor.code_ctx << Instruction("bnz", {_ltrue});
    }

    // Synthetize return code
    *code_ctx << *leftvisitor.code_ctx
              << Instruction().with_label(lnext)
              << *rightvisitor.code_ctx;

    if (!rt->as<BooleanBranchCode>())
        *code_ctx << Instruction("jump", {_lfalse}).with_annot("OR JUMP FALSE");

    if (this->ltrue.length() == 0) {
        auto _lout = LabelFactory::make();
        // not in boolean context, must push true or false
        *code_ctx << Instruction("push", {0}).with_label(_lfalse)
                  << Instruction("jump", {_lout}).with_annot("OR JUMP OUT")
                  << Instruction("push", {1}).with_label(_ltrue)
                  << Instruction().with_label(_lout);
        cerr << "returning bool" << endl;
        return Types::bool_type;
    } else {
        // boolean context
        cerr << "returning branch" << endl;
        // *code_ctx << Instruction("jump", {_ltrue});
        return Types::boolean_branch;
    }
}

// Load Symbol ////////////////////////////////////////////////////////////////
antlrcpp::Any MetaExprVisitor::visitMe_exprsymbol_rule(MMMLParser::Me_exprsymbol_ruleContext *ctx)
{
    cerr << "visit expr symbol" << endl;
    auto s = mmml_load_symbol(ctx->symbol()->getText(), code_ctx);
    if (!s) {
        Report::err(ctx) << " : Unknown symbol " << ctx->symbol()->getText()
                         << endl;

        return Types::int_type;
    }

    // mmml_load_symbol inserts load n in the ctx parameter
    return s->type();
}

// Literals ///////////////////////////////////////////////////////////////////
antlrcpp::Any
MetaExprVisitor::visitMe_exprliteral_rule(MMMLParser::Me_exprliteral_ruleContext *ctx)
{
    cerr << "visit literal" << endl;
    return visit(ctx->literal());
}

antlrcpp::Any MetaExprVisitor::visitLiteral_float_rule(MMMLParser::Literal_float_ruleContext *ctx)
{
    *code_ctx <<
            Instruction("push", { ctx->getText() })
            .with_annot("type(float)" );
    return Types::float_type;
}

antlrcpp::Any MetaExprVisitor::visitLiteral_decimal_rule(MMMLParser::Literal_decimal_ruleContext *ctx)  {
    *code_ctx <<
            Instruction("push",
                        { ctx->getText() })
            .with_annot("type(int)");
    return Types::int_type;
}

antlrcpp::Any MetaExprVisitor::visitLiteral_hexadecimal_rule(MMMLParser::Literal_hexadecimal_ruleContext *ctx)  {
    *code_ctx <<
            Instruction("push",
                        { ctx->getText() })
            .with_annot("type(int)");
    return code_ctx;
}

antlrcpp::Any MetaExprVisitor::visitLiteral_binary_rule(MMMLParser::Literal_binary_ruleContext *ctx)  {
    *code_ctx <<
            Instruction("push",
                        { ctx->getText() })
            .with_annot( "type(int)" );
    return Types::int_type;
}

antlrcpp::Any MetaExprVisitor::visitLiteralstring_rule(MMMLParser::Literalstring_ruleContext *ctx)  {
    *code_ctx << Instruction("push", { ctx->getText() })
              <<
            Instruction("acreate",
                        {ctx->getText().length() - 2})
            .with_annot("type(str)");

    return type_registry.add(make_shared<SequenceType>(Types::char_type));
}

antlrcpp::Any MetaExprVisitor::visitLiteral_char_rule(MMMLParser::Literal_char_ruleContext *ctx)  {
    *code_ctx << Instruction("push", { ctx->getText() })
            .with_annot("type(char)" );
    return Types::char_type;
}

antlrcpp::Any MetaExprVisitor::visitLiteraltrueorfalse_rule(MMMLParser::Literaltrueorfalse_ruleContext *ctx)  {

    *code_ctx <<
            Instruction("push", {( ctx->getText() == "true" ? 1 : 0 ) })
            .with_annot("type(bool)");
    return Types::bool_type;
}

antlrcpp::Any
MetaExprVisitor::visitLiteralnil_rule(MMMLParser::Literalnil_ruleContext *ctx) {

  // *code_ctx << Instruction("push", {"null"}).with_annot("type(nil)");

    return Types::nil_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_boolneg_rule(MMMLParser::Me_boolneg_ruleContext *ctx)
{
    cerr << "visit bool neg" << endl;
    // ! sym

    auto s = mmml_load_symbol(ctx->symbol()->getText(), code_ctx);
    if (!s) {
        Report::err(ctx) << " : Unknown symbol " << ctx->getText()
                          << endl;

        return Types::bool_type;
    }

    // coerce it to bool
    bool bool_ctx = this->ltrue.length() > 0;
    auto coerced_bool_type = gen_cast_code(ctx,
                                           s->type(), Types::bool_type,
                                           code_ctx,
                                           /*normalize=*/ !bool_ctx);

    if (!coerced_bool_type || !coerced_bool_type->equals(Types::bool_type))
    {
        Report::err(ctx) << "Can not coerce type " << s->type()->name()
                          << " to bool";

        return Types::bool_type;
    }

    if (bool_ctx)
    {
        // must jump to true or go to false
        *code_ctx << Instruction("bz", {this->ltrue}).with_annot("branch to true")
                  << Instruction("jump", {this->lfalse}).with_annot("branch to false");
        return Types::boolean_branch;
    }

    // 0 - 1    ->  -1  (true)
    // 1 - 1    ->   0 (false)
    auto _lfalse = LabelFactory::make();
    auto _lout = LabelFactory::make();

    *code_ctx << Instruction("bz", {_lfalse}).with_annot("Convert to false")
              << Instruction("push", {0}).with_annot("type(bool)")
              << Instruction("jump", {_lout})
              << Instruction("push", {1}).with_annot("type(bool)").with_label(_lfalse)
              << Instruction().with_label(_lout);

    return Types::bool_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_boolnegparens_rule(MMMLParser::Me_boolnegparens_ruleContext *ctx)
{
    cerr << "visit bool neg parens" << endl;
    FuncbodyVisitor fbvisitor(code_ctx->create_block());
    fbvisitor.ltrue = this->lfalse;
    fbvisitor.lfalse = this->ltrue;
    fbvisitor.lout = this->lout;

    Type::const_pointer ftype = fbvisitor.visit(ctx->funcbody());

    if (!ftype) {
        Report::err(ctx, "IMPL ERROR ON BOOLNEGPARENS");
        return Types::bool_type;
    }

    *code_ctx << std::move(*fbvisitor.code_ctx);

    return ftype;
}

antlrcpp::Any MetaExprVisitor::visitMe_listconcat_rule(MMMLParser::Me_listconcat_ruleContext *ctx)
{
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<SequenceType>();
                                });


    // invalid types in join, but recover as if
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

antlrcpp::Any MetaExprVisitor::visitMe_exprmuldiv_rule(MMMLParser::Me_exprmuldiv_ruleContext *ctx)
{
    // Can multiply any basic type (char, int, float) with any other. Will accept recursive type as "valid" -- any type is higher than recursive, so rtype should be the non-recursive
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<RecursiveType>() || (c->is_basic() && !c->as<BoolType>());
                                });

    if (!rtype)
        return Types::int_type;

    *code_ctx
            << Instruction(ctx->op->getText() == "/" ? "div" : "mul").with_annot("type(" + rtype->name() + ")");

    return rtype;
}

antlrcpp::Any MetaExprVisitor::visitMe_exprplusminus_rule(MMMLParser::Me_exprplusminus_ruleContext *ctx)
{
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<RecursiveType>() || (c->is_basic() && !c->as<BoolType>());
                                });

    if (!rtype)
        return Types::int_type;

    *code_ctx
            << Instruction(ctx->op->getText() == "+" ? "add" : "sub").with_annot("type(" + rtype->name() + ")");

    return rtype;
}

// Relational
antlrcpp::Any
MetaExprVisitor::visitMe_boolgtlt_rule(MMMLParser::Me_boolgtlt_ruleContext *ctx)
{
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<RecursiveType>() || (c->is_basic() && !c->as<BoolType>());
                                });

    if (!rtype)
        return Types::int_type;

    string _ltrue, _lfalse;
    bool boolean_ctx = this->ltrue.length() > 0;
    if (boolean_ctx) {
        _ltrue = this->ltrue;
        _lfalse = this->lfalse;
    } else {
        _ltrue = LabelFactory::make();
        _lfalse = LabelFactory::make();
    }

    // cmp_instr jumps to false or falls through
    // cmp_instr _lfalse
    // jump _ltrue
    *code_ctx << Instruction("sub").with_annot("cmp");

    auto cmp_txt = ctx->TOK_CMP_GT_LT()->getText();
    if (cmp_txt == "<=")
        // x <= y <=> x-y <= 0 <=> !(x-y > 0)
        *code_ctx << Instruction("bpos", {_lfalse})
                  << Instruction("jump", {_ltrue});
    else if (cmp_txt == "<")
        // x < y <=> x-y < 0
        *code_ctx << Instruction("bneg", {_ltrue})
                  << Instruction("jump", {_lfalse});
    else if (cmp_txt == ">=")
        // x >= y  <=>  x-y >= 0  <=> !(x-y < 0)
        *code_ctx << Instruction("bneg", {_lfalse})
                  << Instruction("jump", {_ltrue});
    else if (cmp_txt == ">")
        // x > y <=> x-y > 0
        *code_ctx << Instruction("bpos", {_ltrue})
                  << Instruction("jump", {_lfalse});
    else
        throw logic_error("UNHANDLED COMPARE");

    if (boolean_ctx)
        return Types::boolean_branch;

    auto _lout = LabelFactory::make();
    *code_ctx << Instruction("push", {1}).with_label(_ltrue)
              << Instruction("jump", {_lout})
              << Instruction("push", {0}).with_label(_lfalse)
              << Instruction().with_label(_lout);

    return Types::bool_type;
}

antlrcpp::Any
MetaExprVisitor::visitMe_booleqdiff_rule(MMMLParser::Me_booleqdiff_ruleContext *ctx)
{
    auto rtype = generic_bin_op(this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<RecursiveType>() || (c->is_basic() && !c->as<BoolType>());
                                });

    if (!rtype)
        return Types::int_type;

    string _ltrue, _lfalse;
    bool boolean_ctx = this->ltrue.length() > 0;
    if (boolean_ctx) {
        _ltrue = this->ltrue;
        _lfalse = this->lfalse;
    } else {
        _ltrue = LabelFactory::make();
        _lfalse = LabelFactory::make();
    }

    // cmp_instr jumps to false or falls through
    // cmp_instr _lfalse
    // jump _ltrue
    *code_ctx << Instruction("sub").with_annot("cmp");

    auto cmp_txt = ctx->TOK_CMP_EQ_DIFF()->getText();
    if (cmp_txt == "==")
        *code_ctx << Instruction("bz", {_ltrue})
                  << Instruction("jump", {_lfalse});
    else if (cmp_txt == "!=")
        *code_ctx << Instruction("bnz", {_ltrue})
                  << Instruction("jump", {_lfalse});

    if (boolean_ctx)
        return Types::boolean_branch;

    auto _lout = LabelFactory::make();
    *code_ctx << Instruction("push", {1}).with_label(_ltrue)
              << Instruction("jump", {_lout})
              << Instruction("push", {0}).with_label(_lfalse)
              << Instruction().with_label(_lout);

    return Types::bool_type;
}
// Cast ///////////////////////////////////////////////////////////////////////
antlrcpp::Any MetaExprVisitor::visitMe_exprcast_rule(MMMLParser::Me_exprcast_ruleContext *ctx)
{
    return visit(ctx->cast());
}

antlrcpp::Any MetaExprVisitor::visitCast_rule(MMMLParser::Cast_ruleContext *ctx)
{
    auto source_type = visit(ctx->funcbody()).as<Type::const_pointer>();
    auto dest_type = type_registry.find_by_name(ctx->c->getText());

    if (!source_type) {
        Report::err(ctx) << "While parsing cast: gor null source type!"
                          << endl;

        return Types::int_type;
    }

    if (!dest_type) {
        Report::err(ctx) << "Unknown type ``" << ctx->c->getText() << "''"
                          << " in cast"
                          << endl;
        return Types::int_type;
    }

    return gen_cast_code(ctx, source_type, dest_type, code_ctx);
}

antlrcpp::Any
MetaExprVisitor::visitMe_list_create_rule(MMMLParser::Me_list_create_ruleContext *ctx)
{
    return this->visit(ctx->sequence_expr());
}

antlrcpp::Any MetaExprVisitor::visitSeq_create_seq(MMMLParser::Seq_create_seqContext *ctx)
{
    // Assume base is already in the registry

    FuncbodyVisitor fbvisitor(this->code_ctx);
    auto base_type = fbvisitor.visit( ctx->funcbody() ).as<Type::const_pointer>();

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

antlrcpp::Any
MetaExprVisitor::visitMe_class_ctor_rule(MMMLParser::Me_class_ctor_ruleContext *ctx)
{
    return visit(ctx->class_ctor());
}

antlrcpp::Any
MetaExprVisitor::visitClass_ctor(MMMLParser::Class_ctorContext *ctx)
{
    TupleType::const_pointer tuple;

    Type::const_pointer class_type =
            type_registry.find_by_name(ctx->name->getText());

    // Check if it's a known class
    if (!class_type) {
        Report::err(ctx) << "Trying to create unknown class "
                         << ctx->name->getText();

        return Types::int_type;

    } else if (!class_type->as<ClassType>()) {
        Report::err(ctx) << "Given name is not a class name" << endl;

        return Types::int_type;

    }

    // Check the creation tuple
    Type::const_pointer _tuple = this->visit(ctx->tuple_ctor());
    if (!_tuple) {
        Report::err(ctx) << "IMPL ERROR GOT NULL TUP TYPE IN CLASS MAKE" << endl;
        return Types::int_type;
    }

    tuple = _tuple->as<TupleType>();
    if (!tuple) {
        Report::err(ctx) << "Trying to create a class with something not a tuple"
                         << endl;
        return Types::int_type;
    }

    if (!class_type->equal_storage(tuple)) {
        Report::err(ctx) << "Trying to create class " << class_type->name()
                         << " with incompatible tuple"
                         << endl;
        return Types::int_type;
    }

    return class_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_class_get_rule(MMMLParser::Me_class_get_ruleContext *ctx)
{
    Type::const_pointer field_type;
    ClassType::const_pointer class_type;
    int pos;

    FuncbodyVisitor fbvisitor(this->code_ctx);
    Type::const_pointer funcbody_type =
            fbvisitor.visit(ctx->funcbody());

    // top shoud be class
    if (!funcbody_type) {
        Report::err(ctx) << "IMPL ERROR got null from funcbody" << endl;

        return Types::int_type;
    }

    class_type = funcbody_type->as<ClassType>();
    if (!class_type) {
        Report::err(ctx) << "Using class accessor method get on type ``"
                         << funcbody_type->name() << "''"
                         << endl;
        return Types::int_type;
    }

    pos = class_type->get_pos(ctx->name->getText());
    if (pos < 0) {
        Report::err(ctx) << "Trying to get unknown field ``"
                         << ctx->name->getText()
                         << "´´ in class "
                         << class_type->name()
                         << endl;
        return class_type->as<Type>();
    }

    field_type = class_type->get_type(ctx->name->getText());
    if (!field_type)
    {
        Report::err(ctx) << "IMPL ERROR FIELD TYPE HAS NULL" << endl;
        field_type = Types::int_type;
    }

    *code_ctx << Instruction("aget", {pos}).with_annot("type(" + field_type->name() + ")");

    return field_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_class_set_rule(MMMLParser::Me_class_set_ruleContext *ctx)
{
    Type::const_pointer field_type, cl_funcbody_type, val_funcbody_type;
    ClassType::const_pointer class_type;
    int pos;

    FuncbodyVisitor fbvisitor(this->code_ctx);
    cl_funcbody_type = fbvisitor.visit(ctx->cl);

    if (!cl_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR got null from tup funcbody" << endl;
        return Types::int_type;
    }

    class_type = cl_funcbody_type->as<ClassType>();
    if (!class_type) {
        Report::err(ctx) << "Using tuple accessor method get on type ``"
                          << cl_funcbody_type->name() << "''"
                          << endl;
        return Types::int_type;
    }

    pos = class_type->get_pos(ctx->name->getText());
    if (pos < 0) {
        Report::err(ctx) << "Trying to set unknown field ``"
                         << ctx->name->getText()
                         << "´´ in class "
                         << class_type->name()
                         << endl;
        return cl_funcbody_type;
    }

    field_type = class_type->get_type(ctx->name->getText());
    if (!field_type)
    {
        Report::err(ctx) << "IMPL ERROR FIELD TYPE HAS NULL" << endl;
        field_type = Types::int_type;
    }

    val_funcbody_type = fbvisitor.visit(ctx->val);
    if (!val_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR got null from val funcbody" << endl;
        return cl_funcbody_type;
    }

    if (!gen_cast_code(ctx,
                       val_funcbody_type, /* -> */ field_type,
                       code_ctx, true))
    {
        Report::err(ctx) << "Can not cast type " << val_funcbody_type->name()
                         << " to " << field_type->name()
                         << " on tuple set"
                         << endl;
        return cl_funcbody_type;
    }

    *code_ctx
            << Instruction("aset", {pos})
            .with_annot("type(" + class_type->name() + ")");

    return cl_funcbody_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_tuple_set_rule(MMMLParser::Me_tuple_set_ruleContext *ctx)
{
    Type::const_pointer field_type, tup_funcbody_type, val_funcbody_type;
    TupleType::const_pointer tuple_type;
    int pos;

    FuncbodyVisitor fbvisitor(this->code_ctx);
    tup_funcbody_type = fbvisitor.visit(ctx->tup).as<Type::const_pointer>();
    // top is tuple, get specific position

    if (!tup_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR got null from tup funcbody" << endl;
        return Types::int_type;
    }

    tuple_type = tup_funcbody_type->as<TupleType>();
    if (!tuple_type) {
        Report::err(ctx) << "Using tuple accessor method get on type ``"
                          << tup_funcbody_type->name() << "''"
                          << endl;
        return tup_funcbody_type;
    }

    val_funcbody_type = fbvisitor.visit(ctx->val);
    if (!val_funcbody_type) {
        Report::err(ctx) << "IMPL ERROR got null from val funcbody" << endl;
        return tup_funcbody_type;
    }

    pos = stoi(ctx->pos->getText());
    field_type = tuple_type->get_nth_type(pos);

    if (!field_type) {
        Report::err(ctx) << "Access to invalid position " << pos
                          << " on tuple of type"
                          << tuple_type->name()
                          << endl;
        return tup_funcbody_type;
    }

    if (!gen_cast_code(ctx,
                  val_funcbody_type, /* -> */ field_type,
                       code_ctx, true))
    {
        Report::err(ctx) << "Can not cast type " << val_funcbody_type->name()
                         << " to " << field_type->name()
                         << " on tuple set"
                         << endl;
        return tup_funcbody_type->as<Type>();
    }

    *code_ctx << Instruction("aset", {pos}).with_annot("type(" + tuple_type->name() + ")");

    return tup_funcbody_type->as<Type>();
}

antlrcpp::Any MetaExprVisitor::visitMe_tuple_get_rule(MMMLParser::Me_tuple_get_ruleContext *ctx)
{
    Type::const_pointer field_type;
    TupleType::const_pointer tuple_type;
    int pos;

    FuncbodyVisitor fbvisitor(this->code_ctx);
    auto funcbody_type = fbvisitor.visit(ctx->funcbody()).as<Type::const_pointer>();
    // top is tuple, get specific position

    if (!funcbody_type) {
        Report::err(ctx) << "IMPL ERROR god null from funcbody" << endl;

        return Types::int_type;
    }

    tuple_type = funcbody_type->as<TupleType>();
    if (!tuple_type) {
        Report::err(ctx) << "Using tuple accessor method get on type ``"
                          << funcbody_type->name() << "''"
                          << endl;
        return Types::int_type;
    }

    pos = stoi(ctx->pos->getText());
    field_type = tuple_type->get_nth_type(pos);

    if (!field_type) {
        Report::err(ctx) << "Access to invalid position " << pos
                          << " on tuple of type"
                          << tuple_type->name()
                          << endl;
        return Types::int_type;
    }

    *code_ctx << Instruction("aget", {pos}).with_annot("type(" + field_type->name() + ")");

    return field_type;
}

antlrcpp::Any
MetaExprVisitor::visitMe_tup_create_rule(MMMLParser::Me_tup_create_ruleContext *ctx)
{
    return visit(ctx->tuple_ctor());
}

antlrcpp::Any MetaExprVisitor::visitTuple_ctor(MMMLParser::Tuple_ctorContext *ctx)
{
    TupleType::base_types_vector_t types;
    Type::pointer _tup;
    TupleType::const_pointer tuple;

    int i = 0;

    for(auto funcbody: ctx->funcbody())
    {
        FuncbodyVisitor fbvisitor(this->code_ctx);
        Type::const_pointer type = fbvisitor.visit(funcbody);
        if (!type) {
            Report::err(funcbody) << "IMPL ERROR: GOT NULL FROM TUPLE CTOR FUNCBODY #" << i;

            return Types::int_type;
        }

        types.push_back(type);
    }

    _tup = make_shared<TupleType>(types);
    tuple = type_registry.add(_tup)->as<TupleType>();

    if (!tuple) {
        Report::err(ctx) << "Could not create tuple type";
        return Types::int_type;
    }

    *code_ctx << Instruction("acreate", {tuple->base_types.size()})
            .with_annot("type(" + tuple->name() + ")");

    return tuple->as<Type>();
}




} // end namespace mmml ///////////////////////////////////////////////////////
