/****************************************************************************
 *        Filename: "MMML/MetaExprVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:44:30 2017"
 *         Updated: "2017-10-23 15:25:38 kassick"
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
#include "mmml/FunctionCallArgListVisitor.H"

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
    const string& op,
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
    auto coalesced_type = gen_coalesce_code(left, ltype, lcode, rtype, rcode);

    if (!coalesced_type || !pred(coalesced_type))
    {
        // Required upcasting did not result in expected type

        Report::err(left) << "Could not coerce types "
                          << ltype->name()
                          << " and "
                          << rtype->name()
                          << " for operation ``" << op << "''"
                          << endl;

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

    string _lout;
    string _ltrue = this->ltrue;
    string _lfalse = this->lfalse;
    string lnext = LabelFactory::make();

    if (_ltrue.length() == 0) {
        // we are not in a boolean context, we must leave the result on the top of the stack
        _lout = LabelFactory::make();
        _lfalse = LabelFactory::make();
        _ltrue = LabelFactory::make();
    }


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
                                << " to bool"
                                << endl;

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
                                << " to bool"
                                << endl;

        *rightvisitor.code_ctx
                << Instruction("bz", {rightvisitor.lfalse});
    }

    *code_ctx
            << std::move(*leftvisitor.code_ctx)
            << Instruction().with_label(lnext).with_annot("and: next test")
            << std::move(*rightvisitor.code_ctx);

    if (!rt->as<BooleanBranchCode>())
        *code_ctx << Instruction("jump", {_ltrue}).with_annot("AND JUMP TRUE");

    if (this->ltrue.length() == 0) {
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

    string _lout;
    string _ltrue = this->ltrue;
    string _lfalse = this->lfalse;
    string lnext = LabelFactory::make();

    if (_ltrue.length() == 0) {
        // we are not in a boolean context, we must leave the result on the top of the stack
        _lout = LabelFactory::make();
        _ltrue = LabelFactory::make();
        _lfalse = LabelFactory::make();
    }


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
                                << " to bool"
                                << endl;

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
                                << " to bool"
                                << endl;

        *rightvisitor.code_ctx << Instruction("bnz", {_ltrue});
    }

    // Synthetize return code
    *code_ctx << std::move(*leftvisitor.code_ctx)
              << Instruction().with_label(lnext).with_annot("or: test next")
              << std::move(*rightvisitor.code_ctx);

    if (!rt->as<BooleanBranchCode>())
        *code_ctx << Instruction("jump", {_lfalse}).with_annot("OR JUMP FALSE");

    if (this->ltrue.length() == 0) {
        // not in boolean context, must push true or false
        *code_ctx << Instruction("push", {0}).with_label(_lfalse)
                  << Instruction("jump", {_lout}).with_annot("OR JUMP OUT")
                  << Instruction("push", {1}).with_label(_ltrue)
                  << Instruction().with_label(_lout);
        return Types::bool_type;
    } else {
        // boolean context
        // *code_ctx << Instruction("jump", {_ltrue});
        return Types::boolean_branch;
    }
}

// Load Symbol ////////////////////////////////////////////////////////////////
antlrcpp::Any MetaExprVisitor::visitMe_exprsymbol_rule(MMMLParser::Me_exprsymbol_ruleContext *ctx)
{
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
    return Types::int_type;
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
              << Instruction("acreate").with_annot("type(str)");

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
    *code_ctx << Instruction("acreate", {0}).with_annot("type(nil)");

    return Types::nil_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_boolneg_rule(MMMLParser::Me_boolneg_ruleContext *ctx)
{
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
                          << " to bool"
                         << endl;

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
    auto _lout = LabelFactory::make();
    auto _lfalse = LabelFactory::make();

    *code_ctx << Instruction("bz", {_lfalse}).with_annot("Convert to false")
              << Instruction("push", {0}).with_annot("type(bool)")
              << Instruction("jump", {_lout})
              << Instruction("push", {1}).with_annot("type(bool)").with_label(_lfalse)
              << Instruction().with_label(_lout);

    return Types::bool_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_boolnegparens_rule(MMMLParser::Me_boolnegparens_ruleContext *ctx)
{
    string _ltrue, _lfalse, _lout, _lend;

    bool boolean_ctx = this->ltrue.length() > 0;

    if (boolean_ctx) {
        _lfalse = this->lfalse;
        _ltrue = this->ltrue;
    } else {
        _lend = LabelFactory::make();
        _lout = LabelFactory::make();
        _lfalse = LabelFactory::make();
        _ltrue = LabelFactory::make();
    }

    FuncbodyVisitor fbvisitor(code_ctx->create_block());
    fbvisitor.ltrue = _lfalse;
    fbvisitor.lfalse = _ltrue;
    fbvisitor.lout = _lout;

    Type::const_pointer ftype = fbvisitor.visit(ctx->funcbody());

    if (!ftype) {
        Report::err(ctx, "IMPL ERROR ON BOOLNEGPARENS");

        return Types::bool_type;
    }

    // May need to cast to bool
    if (!ftype->as<BooleanBranchCode>()) {
        auto cast_type =
                gen_cast_code(ctx,
                              ftype, /*->*/ Types::bool_type,
                              fbvisitor.code_ctx,
                              false);
        if (!cast_type)
        {
            Report::err(ctx) << "Can't cast ``" << ftype->name() << "´´"
                             << " to boolean"
                             << endl;
            return Types::bool_type;
        }

        ftype = cast_type;
    }

    Type::const_pointer ret_type;

    if (boolean_ctx)
    {
        // If it's not branch code, it's a boolean, but we already have our targets
        if (!ftype->as<BooleanBranchCode>())
        {
            *fbvisitor.code_ctx
                    << Instruction("bz", {_ltrue}).with_annot("!false => true")
                    << Instruction("jump", {_lfalse}).with_annot("!true => false");
        }

        ret_type = Types::boolean_branch;

    } else {
        // Not boolean context

        *fbvisitor.code_ctx
                << Instruction("bz", {_ltrue}).with_annot("BBB").with_label(_lout)
                << Instruction("push", {0}).with_label(_lfalse)
                << Instruction("jump", {_lend})
                << Instruction("push", {1}).with_label(_ltrue)
                << Instruction().with_label(_lend);

        ret_type = Types::bool_type;
    }

    *code_ctx << std::move(*fbvisitor.code_ctx);

    return ret_type;
}

antlrcpp::Any MetaExprVisitor::visitMe_listconcat_rule(MMMLParser::Me_listconcat_ruleContext *ctx)
{
    auto rtype = generic_bin_op("::",
                                this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<SequenceType>() || c->as<NilType>();
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
    // corner case: mod must be int/int
    if (ctx->op->getText() == "%")
    {
        Type::const_pointer left_type = this->visit(ctx->l);

        if (!left_type) {
            Report::err(ctx->l) << "IMPL ERROR GOT NIL ON MOD LEFT" << endl;
            return Types::int_type ;
        }

        auto left_int_type = gen_cast_code (ctx->l, left_type, Types::int_type,
                                            this->code_ctx);

        if (!left_int_type) {
            Report::err(ctx->l) << "Can not convert type ``" << left_type->name() << "''"
                                << " to int in call to operator mod"
                                << endl;
            // fall through, let it try to find more errors
        }

        Type::const_pointer right_type = this->visit(ctx->r);

        if (!right_type) {
            Report::err(ctx->l) << "IMPL ERROR GOT NIL ON MOD RIGHT" << endl;
            return Types::int_type ;
        }

        auto right_int_type = gen_cast_code (ctx->r, right_type, Types::int_type,
                                             this->code_ctx);

        if (!right_int_type) {
            Report::err(ctx->l) << "Can not convert type ``" << right_type->name() << "''"
                                << " to int in call to operator mod"
                                << endl;
        }

        *code_ctx << Instruction("mod").with_annot("type(int)");

        return Types::int_type;
    }

    // Can multiply any basic type (char, int, float) with any other. Will accept recursive type as "valid" -- any type is higher than recursive, so rtype should be the non-recursive
    auto rtype = generic_bin_op(ctx->op->getText(),
                                this,
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
    auto rtype = generic_bin_op(ctx->op->getText(),
                                this,
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
    auto rtype = generic_bin_op(ctx->TOK_CMP_GT_LT()->getText(),
                                this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<RecursiveType>() || (c->is_basic() && !c->as<BoolType>());
                                });

    if (!rtype)
        return Types::bool_type;

    bool boolean_ctx = this->ltrue.length() > 0;

    string _lout = LabelFactory::make();
    string _ltrue, _lfalse;
    if (boolean_ctx) {
        _lfalse = this->lfalse;
        _ltrue = this->ltrue;
    } else {
        _lfalse = LabelFactory::make();
        _ltrue = LabelFactory::make();
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

    *code_ctx << Instruction("push", {1}).with_label(_ltrue)
              << Instruction("jump", {_lout})
              << Instruction("push", {0}).with_label(_lfalse)
              << Instruction().with_label(_lout);

    return Types::bool_type;
}

antlrcpp::Any
MetaExprVisitor::visitMe_booleqdiff_rule(MMMLParser::Me_booleqdiff_ruleContext *ctx)
{
    auto rtype = generic_bin_op(ctx->TOK_CMP_EQ_DIFF()->getText(),
                                this,
                                ctx->l, ctx->r,
                                code_ctx,
                                [](Type::const_pointer c) {
                                    return c->as<RecursiveType>() || (c->is_basic() && !c->as<BoolType>());
                                });

    if (!rtype)
        return Types::int_type;

    bool boolean_ctx = this->ltrue.length() > 0;

    string _lout = LabelFactory::make();
    string _ltrue, _lfalse;

    if (boolean_ctx) {
        _ltrue = this->ltrue;
        _lfalse = this->lfalse;
    } else {
        _lfalse = LabelFactory::make();
        _ltrue = LabelFactory::make();
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
    FuncbodyVisitor fbvisitor(code_ctx);
    auto source_type = fbvisitor.visit(ctx->funcbody()).as<Type::const_pointer>();
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

    auto ret = gen_cast_code(ctx, source_type, dest_type, code_ctx);
    if (!ret) {
        Report::err(ctx) << "Could not cast type ``" << source_type->name() << "''"
                         << " to ``" << dest_type->name() << "''"
                         << endl;
    }

    return  ret;
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
                         << ctx->name->getText()
                         << endl;

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
        Report::err(ctx) << "Using class accessor method set on type ``"
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
            Report::err(funcbody) << "IMPL ERROR: GOT NULL FROM TUPLE CTOR FUNCBODY #" << i << endl;

            return Types::int_type;
        }

        types.push_back(type);
    }

    _tup = make_shared<TupleType>(types);
    tuple = type_registry.add(_tup)->as<TupleType>();

    if (!tuple) {
        Report::err(ctx) << "Could not create tuple type"
                         << endl;
        return Types::int_type;
    }

    *code_ctx << Instruction("acreate", {tuple->base_types.size()})
            .with_annot("type(" + tuple->name() + ")");

    return tuple->as<Type>();
}


// Function call //////////////////////////////////////////////////////////////

antlrcpp::Any
MetaExprVisitor::visitFuncall_rule(MMMLParser::Funcall_ruleContext *ctx)
{
    FunctionCallArgListVisitor fcvisitor(this->code_ctx);

    int nargs = fcvisitor.visit(ctx->funcall_params());

    const string fname = ctx->symbol()->getText();

    auto & args = fcvisitor.args;

    if (nargs == 0)
    {
        // MMML does not support 0-args functions, EXCEPT THE SPECIAL ONES
        if (fname == "read_char")
        {
            *code_ctx << Instruction("readc");
            return Types::char_type;
        }
        else if (fname == "read_int")
        {
            *code_ctx << Instruction("readi");
            return Types::int_type;
        }
        else if (fname == "read_float")
        {
            *code_ctx << Instruction("readd");
            return Types::float_type;
        }
        else if (fname == "read_string")
        {
            *code_ctx << Instruction("reads")
                      << Instruction("acreate");
            auto strtype = make_shared<SequenceType>(Types::char_type);
            return type_registry.add(strtype);
        }

        // else
        Report::err(ctx) << "Calling invalid 0-sized function ``"
                             << fname
                             << "´´"
                             << endl;
        return Types::int_type;
    }

    if (fname == "print") {
        // Special function print
        if (nargs != 1)
        {
            Report::err(ctx) << "Call to function ``print´´ must always have 1 parameter" << endl;
            return Types::int_type;
        }

        SequenceType::const_pointer seq = fcvisitor.args[0].first->as<SequenceType>();
        if (seq) {
            auto base_ptr = seq->base_type.lock();
            if (base_ptr && base_ptr->equals(Types::char_type) && seq->dim == 1)
            {
                *code_ctx << std::move(*fcvisitor.args[0].second)
                          << Instruction("aload")
                          << Instruction("prints");

            } else {
                Report::err(ctx) << "Function print can not print sequence type" << endl;
            }

        } else if (fcvisitor.args[0].first->is_basic()) {
            *code_ctx << std::move(*fcvisitor.args[0].second)
                      << Instruction("print");
        } else {
            Report::err(ctx) << "Function print can not print type " << fcvisitor.args[0].first->name()
                             << endl;
        }

        return Types::int_type;
    }

    if (fname == "str") {
        SequenceType::const_pointer seq = fcvisitor.args[0].first->as<SequenceType>();
        if (seq) {
            auto base_ptr = seq->base_type.lock();
            if (base_ptr && base_ptr->equals(Types::char_type) && seq->dim == 1)
            {
                *code_ctx << std::move(*fcvisitor.args[0].second)
                          << Instruction("nop");
            } else {
                Report::err(ctx) << "Function str can not convert sequence type" << endl;
            }
        } else if (fcvisitor.args[0].first->is_basic()) {
            *code_ctx << std::move(*fcvisitor.args[0].second)
                      << Instruction("cast_s")
                      << Instruction("acreate");
        } else {
            Report::err(ctx) << "Function str can not convert type " << fcvisitor.args[0].first->name()
                             << endl;
        }

        auto strtype = make_shared<SequenceType>(Types::char_type);
        return type_registry.add(strtype);
    }

    if (fname == "length")
    {
        if (nargs != 1)
        {
            Report::err(ctx) << "Call to function ``length'' must always have 1 parameter" << endl;
            return Types::int_type;
        }

        // Must be sequence or nil
        SequenceType::const_pointer seq = fcvisitor.args[0].first->as<SequenceType>();
        if (seq || args[0].first->as<NilType>()) {
            *code_ctx << std::move(*fcvisitor.args[0].second)
                      << Instruction("alen");
        } else {
            Report::err(ctx) << "Function ``length'' expects a sequence type, got "
                             << "``" << args[0].first->name() << "''"
                             << endl;
        }

        return Types::int_type;
    }

    if (fname == "nth")
    {
        if (nargs != 2) {
            Report::err(ctx) << "Call to function ``nth'' must have two parameters: sequence and position"
                             << endl;
            return Types::int_type;
        }

        auto & arg_seq = args[0];
        auto & arg_pos = args[1];

        if (!arg_seq.first->as<SequenceType>()) {
            Report::err(ctx) << "Arg 0 of nth expects sequence, got "
                             << "``" << arg_seq.first->name() << "''"
                             << endl;
            return Types::int_type;
        }

        auto coerced_type = gen_cast_code(ctx,
                                          arg_pos.first, /* -> */ Types::int_type,
                                          /*code_ctx=*/ arg_pos.second,
                                          false);

        if (!coerced_type) {
            Report::err(ctx) << "Can not coerce arg 1 of nth, type ``"
                             << arg_pos.first->name()
                             << "''"
                             << " to int"
                             << endl;
            return Types::int_type;
        }

        auto base_type = arg_seq.first->as<SequenceType>()->decayed_type.lock();
        if (!base_type)
        {
            Report::err(ctx) << "IMPL ERROR NULL DECAYED FOR TYPE " << arg_seq.first->name() << endl;
            return Types::int_type;
        }

        // push 1
        // push 2
        // acreate    // top: {1 2}
        // load 0
        // push 1
        // sub
        // aget      // top: x-1 {1 2}
        *code_ctx << std::move(*arg_seq.second)
                  << std::move(*arg_pos.second)
                  << Instruction("aget")
                .with_annot("type(" + base_type->name() + ")");

        return base_type;
    }

    if (fname == "let_nth")
    {
        if (nargs != 3) {
            Report::err(ctx) << "Call to function ``nth'' must have three parameters: sequence, position and value"
                             << endl;
            return type_registry.add(make_shared<SequenceType>(Types::int_type));
        }

        auto & arg_seq = args[0];
        auto & arg_pos = args[1];
        auto & arg_value = args[2];

        if (!arg_seq.first->as<SequenceType>()) {
            Report::err(ctx) << "Arg 0 of let_nth expects sequence, got "
                             << "``" << arg_seq.first->name() << "''"
                             << endl;
            return Types::int_type;
        }

        auto base_type = arg_seq.first->as<SequenceType>()->decayed_type.lock();
        if (!base_type)
        {
            Report::err(ctx) << "IMPL ERROR NULL DECAYED FOR TYPE " << arg_seq.first->name() << endl;
            return Types::int_type;
        }

        auto coerced_pos_type = gen_cast_code(ctx,
                                              arg_pos.first, /* -> */ Types::int_type,
                                              arg_pos.second, false);
        if (!coerced_pos_type) {
            Report::err(ctx) << "Can not coerce arg 1 of let_nth, type ``"
                             << arg_pos.first->name()
                             << "''"
                             << " to int"
                             << endl;
            //return type_registry.add(make_shared<SequenceType>(Types::int_type));
            return Types::int_type;
        }

        // Can coerce value to base type of sequence?
        auto coerced_val_type = gen_cast_code(ctx,
                                              arg_value.first, /* -> */ base_type,
                                              arg_value.second);
        if (!coerced_val_type)
        {
            Report::err(ctx) << "Could not coerce arg 2 of let_nth, type ``"
                             << arg_value.first->name()
                             << "''"
                             << " to type ``"
                             << base_type->name()
                             << "''"
                             << endl;
            return arg_seq.first;
        }



        // push 1
        // push 2
        // acreate    // top: {1 2}
        // load 0
        // push 1
        // sub       // top: x-1 {1 2}
        // push 100  // top: 100 x-1 {1 2}
        // aset      //
        *code_ctx << std::move(*arg_seq.second)
                  << std::move(*arg_pos.second)
                  << std::move(*arg_value.second)
                  << Instruction("aset")
                .with_annot("type(" + arg_seq.first->name() + ")");

        return arg_seq.first;
    }

    auto function = function_registry.find(ctx->symbol()->getText());
    if (!function)
    {
        Report::err(ctx) << "Calling unknown function ``"
                         << ctx->symbol()->getText()
                         << "´´"
                         << endl;
        return Types::int_type;
    }


    if (nargs != function->args.size())
    {
        Report::err(ctx) << "Invalid number of arguments for function ``"
                         << function->name
                         << "´´. Got " << nargs
                         << ", expected " << function->args.size()
                         << endl;
        return function->rtype;
    }

    auto call_ctx = this->code_ctx->create_block();

    for (int i = 0; i < fcvisitor.args.size(); i++)
    {
        auto cast_type = gen_cast_code(ctx,
                                       fcvisitor.args[i].first,
                                       function->args[i]->type(),
                                       fcvisitor.args[i].second,
                                       true);

        if (!cast_type)
        {
            Report::err(ctx) << "Could not coerce type "
                             << fcvisitor.args[i].first->name()
                             << " to " << function->args[i]->type()->name()
                             << " in function call"
                             << endl;
            return function->rtype;
        }

        *call_ctx << std::move(*fcvisitor.args[i].second);
    }

    auto ret_label = LabelFactory::make();

    *call_ctx ;

    *code_ctx <<
            Instruction("push", {ret_label})
            .with_annot("@ret_point for " + function->name)
              <<
            std::move(*call_ctx)
              <<
            Instruction("mark", {nargs + 1})
            .with_annot("keep params for " + function->name)
              <<
            Instruction("jump", {function->label})
            .with_annot("call " + function->name)
              <<
            Instruction("nop").with_label(ret_label);

    return function->rtype;
}

} // end namespace mmml ///////////////////////////////////////////////////////
