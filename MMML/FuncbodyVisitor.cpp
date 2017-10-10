/****************************************************************************
 *        Filename: "MMML/ToplevelVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Oct  4 10:09:35 2017"
 *         Updated: "2017-10-10 15:28:40 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/FuncbodyVisitor.H"
#include "mmml/FunctionRegistry.H"
#include "mmml/TypedArgListVisitor.H"
#include "mmml/MetaExprVisitor.H"
#include "mmml/error.H"
#include "mmml/utils.H"
#include "mmml/casts.H"

namespace mmml
{

antlrcpp::Any FuncbodyVisitor::visitFbody_expr_rule(MMMLParser::Fbody_expr_ruleContext *ctx)
{
    MetaExprVisitor mev(code_ctx->create_block());
    mev.lout = lout;
    mev.ltrue = ltrue;
    mev.lfalse = lfalse;

    auto ret = mev.visit(ctx->metaexpr());
    cerr << (ret.is<Type::const_pointer>() ? "not null" : "null")
         << endl;
    Type::const_pointer t = ret;
    *code_ctx << *mev.code_ctx;

    return t;
}

antlrcpp::Any FuncbodyVisitor::visitFbody_if_rule(MMMLParser::Fbody_if_ruleContext *ctx)
{
    /*
                   load 0      # a
                   load 1      # b
                   sub         # type(int)
                   bz lfalse
      ltrue      : nop
                   push "igual"
                   jump lcont
      lfalse     : nop
                   push "diferente"
      lcont      : prints

    */


    Type::const_pointer cond_type, bodytrue_type, bodyfalse_type;

    string _laftercond = LabelFactory::make();
    string _ltrue  = LabelFactory::make();
    string _lfalse = LabelFactory::make();
    string _lout;

    if (this->lout.length() > 0)
        _lout = this->lout;
    else
        _lout = LabelFactory::make();

    FuncbodyVisitor boolvisitor(this->code_ctx->create_block());
    boolvisitor.lout = _laftercond;
    boolvisitor.ltrue = _ltrue;
    boolvisitor.lfalse = _lfalse;

    cond_type = boolvisitor.visit(ctx->cond).as<Type::const_pointer>();
    if (!cond_type) {
        Report::err(ctx) << "IMPL ERROR: COND FUNCBODY RETURNED NULL TYPE"
                         << endl;

        return Types::int_type;
    }

    if (!cond_type->as<BooleanBranchCode>()) {
        // funcbody wasn't evaluated to boolean context
        // Must cast to bool
        // If it's bool, gen_cast_code just returns

        // Must cast
        auto new_cond_type =
                gen_cast_code(ctx,
                              cond_type, Types::bool_type,
                              boolvisitor.code_ctx, false);

        *boolvisitor.code_ctx
                << Instruction("bz", {_lfalse}).with_annot("branch if")
                << Instruction("jump", {_ltrue}).with_label(_laftercond).with_annot("IF COND JUMP NOT BBRANCH");

    }

    // visit true
    FuncbodyVisitor truevisitor(this->code_ctx->create_block());
    truevisitor.lout = this->lout;
    truevisitor.ltrue = this->ltrue;
    truevisitor.lfalse = this->lfalse;
    bodytrue_type =  truevisitor.visit(ctx->bodytrue);

    // visit false
    FuncbodyVisitor falsevisitor(this->code_ctx->create_block());
    falsevisitor.lout = this->lout;
    falsevisitor.ltrue = this->ltrue;
    falsevisitor.lfalse = this->lfalse;
    bodyfalse_type = falsevisitor.visit(ctx->bodyfalse);

    if (!bodytrue_type || !bodyfalse_type) {
        Report::err(ctx) << "IMPL ERROR: BODYTRUE FUNCBODY RETURNED NULL TYPE"
                          << endl;
        return Types::int_type;
    }

    // Now calculate the return type
    Type::const_pointer rtype;

    // If any side was a branch code (because this if is in bool context), we
    // must coalesce both to a branch code. The one that wasn't, must convert to
    // bool and add a bz to false and a jump to true.

    if (bodytrue_type->as<BooleanBranchCode>() ||
        bodytrue_type->as<BooleanBranchCode>())
        rtype = gen_coalesce_code (
                    bodytrue_type, truevisitor.code_ctx,
                    bodyfalse_type, falsevisitor.code_ctx,
                    {
                        Instruction("bz", {this->lfalse}),
                                Instruction("jump", {this->ltrue}).with_annot("Coalesce jump")
                    }
                 );
    else
        rtype = gen_coalesce_code(bodytrue_type, truevisitor.code_ctx,
                                  bodyfalse_type, falsevisitor.code_ctx);

    if (!rtype) {
        Report::err(ctx) << "Could not coalesce types " << bodytrue_type
                          << " and " << bodyfalse_type
                          << " . Maybe a cast?"
                          << endl;

        return Types::int_type;
    }

    *code_ctx
            << *boolvisitor.code_ctx
            << Instruction().with_label(_ltrue).with_annot("true branch")
            << *truevisitor.code_ctx;

    // If we are in boolean context, the bodies already do the jump
    if (!rtype->as<BooleanBranchCode>())
        *code_ctx << Instruction("jump", {_lout}).with_annot("OUT OF TRUE BRANCH");

    *code_ctx << Instruction().with_label(_lfalse).with_annot("false branch")
              << *falsevisitor.code_ctx;

    if (!rtype->as<BooleanBranchCode>())
        *code_ctx << Instruction("jump", {_lout}).with_annot("OUT OF FALSE BRANCH");

    if (this->lout.length() == 0)
        *code_ctx << Instruction().with_label(_lout);
    else
        cerr << "Not adding out jump for if " << ctx->getText() << " it already has label at " << this->lout << endl;

    return rtype;
}

antlrcpp::Any FuncbodyVisitor::visitFbody_let_rule(MMMLParser::Fbody_let_ruleContext *ctx)
{
    /*  let x = 1 + 1,
            y = { 1, 2, "lala" }
        in
            x + (get 1 y)
    */

    auto exe_label = LabelFactory::make();

    cerr << "before new ctx: " << code_ctx->symbol_table->to_string() << endl;

    auto new_context = this->code_ctx->create_subcontext();

    cerr << "Before visiting decls: " << new_context->symbol_table->to_string() << endl;

    FuncbodyVisitor decls_visitor(new_context);

    // visit letlist, populate the new symbol table
    decls_visitor.visit(ctx->letlist());

    // Evaluate body with the symbol table
    FuncbodyVisitor exe_visitor(new_context);
    //exe_visitor.lout = this->lout;
    exe_visitor.lin = exe_label;

    Type::const_pointer ltype = exe_visitor.visit(ctx->fnested);

    if (!ltype)
    {
        Report::err(ctx) << "IMPL ERROR LET EXPR HAS NIL TYPE" << endl;
        ltype = Types::int_type;
    }

    *this->code_ctx
            << std::move(*new_context);

    return ltype;
}

// antlrcpp::Any FuncbodyVisitor::visitLetlist_rule(MMMLParser::Letlist_ruleContext *ctx)
// {
//     auto lnext = LabelFactory::make();

//     FuncbodyVisitor let_attr_visitor(this->code_ctx);
//     let_attr_visitor.lout = lnext;

//     let_attr_visitor.visit(ctx->letvarexpr());

//     FuncbodyVisitor let_attr_cont_visitor(this->code_ctx);
//     let_attr_cont_visitor.lin = lnext;

//     let_attr_cont_visitor.visit(ctx->letlist_cont());

//     return nullptr;
// }

antlrcpp::Any FuncbodyVisitor::visitLetvarattr_rule(MMMLParser::Letvarattr_ruleContext *ctx)
{
    auto lcont = LabelFactory::make();

    FuncbodyVisitor fbvisitor(this->code_ctx->create_block());
    //fbvisitor.lout = lcont;

    // let x = funcbody
    Type::const_pointer symbol_type = fbvisitor.visit(ctx->funcbody());

    if (!symbol_type) {
        Report::err(ctx) << "IMPL ERROR GOT NULL FROM FUNCBODY IN letvarattr"
                          << endl;
        symbol_type = Types::int_type;
    }

    if (code_ctx->symbol_table->find_local(ctx->sym->getText())) {
        // name clash on same table
        Report::err(ctx) << "Re-defining symbol ``" << ctx->sym->getText() << "''"
                          << " on same context in an error"
                          << endl;

        return code_ctx->symbol_table->offset;
    }

    auto sym = make_shared<Symbol>(ctx->sym->getText(),
                                   symbol_type,
                                   ctx->sym->getStart()->getLine(),
                                   ctx->sym->getStart()->getCharPositionInLine());

    code_ctx->symbol_table->add(sym);

    *code_ctx
            << *fbvisitor.code_ctx
            << Instruction()
            .with_label(lcont)
            .with_annot("store at " + to_string(sym->pos) +
                        " type(" + symbol_type->name() + ")");

    cerr << "After " << ctx->getText() << ":" << endl;
    cerr << code_ctx->symbol_table->to_string() << endl;

    return code_ctx->symbol_table->offset;
}

antlrcpp::Any FuncbodyVisitor::visitLetvarresult_ignore_rule(MMMLParser::Letvarresult_ignore_ruleContext *ctx)
{
    // let _  = funcbody
    auto lcont = LabelFactory::make();

    FuncbodyVisitor fbvisitor(code_ctx->create_block());
    //fbvisitor.lout = lcont;

    Type::const_pointer symbol_type = fbvisitor.visit(ctx->funcbody());

    if (!symbol_type) {
        Report::err(ctx) << "IMPL ERROR GOT NULL FROM FUNCBODY IN letvarattr"
                          << endl;
        symbol_type = Types::int_type;
    }

    *code_ctx <<
            std::move(*fbvisitor.code_ctx)
              <<
            Instruction("pop",{})
            .with_label(lcont)
            .with_annot("drop _");

    return code_ctx->symbol_table->offset;
}

antlrcpp::Any FuncbodyVisitor::visitLetunpack_rule(MMMLParser::Letunpack_ruleContext *ctx)
{
    // let h :: t = funcbody
    /*
      aload "abc"
      # stack is 3 c b a
      push -1
      # stack is -1 3 c b a
      add
      # stack is 2 c b a
      acreate
      # stack is {b c} a
      store @tail
      store @head
     */

    auto lcont = LabelFactory::make();

    FuncbodyVisitor fbvisitor(code_ctx->create_block());
    //fbvisitor.lout = lcont;

    Type::const_pointer list_type = fbvisitor.visit(ctx->funcbody());

    if (!list_type) {
        Report::err(ctx) << "IMPL ERROR GOT NULL TYPE FROM UNPACK FUNCBODY" << endl;
        return code_ctx->symbol_table->offset;
    }

    if (!list_type->as<SequenceType>()) {
        // recover
        Report::err(ctx) << "Unpack rule expects a list on it's right side" << endl;
        list_type = type_registry.add(make_shared<SequenceType>(list_type));
    }

    auto previous_tail = code_ctx->symbol_table->find_local(ctx->tail->getText());
    if (previous_tail) {
        Report::err(ctx) << "Re-definition of symbol ``" << previous_tail->name
                          << " shadows previously defined in line "
                          << previous_tail->line << ", column" << previous_tail->col
                          << endl;
    } else {
        auto sym = make_shared<Symbol>(ctx->tail->getText(), list_type,
                                       ctx->tail->getStart()->getLine(),
                                       ctx->tail->getStart()->getCharPositionInLine());

        code_ctx->symbol_table->add(sym);

        *fbvisitor.code_ctx <<
                Instruction("store", {sym->pos})
                .with_label(lcont)
                .with_annot("type(" + list_type->name() + ")");
    }

    auto previous_head = code_ctx->symbol_table->find_local(ctx->head->getText());
    if (previous_head) {
        Report::err(ctx) << "Re-definition of symbol ``" << previous_head->name
                          << " shadows previously defined in line "
                          << previous_head->line << ", column" << previous_head->col
                          << endl;
    } else {
        auto decayed_type = list_type->as<SequenceType>()->decayed_type.lock();
        if (!decayed_type)
        {
            Report::err(ctx) << "IMPL ERROR DECAYED LIST TYPE IS NULL" << endl;
            decayed_type = Types::int_type;
        }

        auto sym = make_shared<Symbol>(ctx->tail->getText(), decayed_type,
                                       ctx->head->getStart()->getLine(),
                                       ctx->head->getStart()->getCharPositionInLine());

        code_ctx->symbol_table->add(sym);

        *fbvisitor.code_ctx <<
                Instruction("store", {sym->pos})
                .with_annot("type(" + decayed_type->name() + ")");
    }

    return code_ctx->symbol_table->offset;
}

} // end namespace mmmml //////////////////////////////////////////////////////
