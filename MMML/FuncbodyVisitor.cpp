/****************************************************************************
 *        Filename: "MMML/ToplevelVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Oct  4 10:09:35 2017"
 *         Updated: "2017-10-04 16:52:32 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/FuncbodyVisitor.H"
#include "mmml/FunctionRegistry.H"
#include "mmml/TypedArgListVisitor.H"
#include "mmml/error.H"
#include "mmml/utils.H"
#include "mmml/casts.H"

namespace mmml
{


antlrcpp::Any FuncbodyVisitor::visitFbody_expr_rule(MMMLParser::Fbody_expr_ruleContext *ctx)
{
    // MetaExprVisitor mev(code_context->create_block());
    // mev.lout = lout;
    // mev.lin = lin;
    // mev.ltrue = ltrue;
    // mev.lfalse = lfalse;

    // return mev.visit(ctx->metaexpr());

    return nullptr;
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

    make_entry_label();

    Type::const_pointer cond_type, bodytrue_type, bodyfalse_type;

    string laftercond = LabelFactory::make();
    string ltrue  = LabelFactory::make();
    string lfalse = LabelFactory::make();

    FuncbodyVisitor boolvisitor(this->code_ctx->create_block());
    boolvisitor.lout = laftercond;
    boolvisitor.ltrue = ltrue;
    boolvisitor.lfalse = lfalse;

    cond_type = boolvisitor.visit(ctx->cond).as<Type::const_pointer>();

    *boolvisitor.code_ctx << Instruction("nop").with_label(laftercond);

    if (!cond_type) {
        Report::err(ctx) << "IMPL ERROR: COND FUNCBODY RETURNED NULL TYPE"
                          << endl;
        *boolvisitor.code_ctx <<
                Instruction("pop").with_annot("drop cond")
                               <<
                Instruction("push", {0}).with_annot("type(bool)");

    } else if (!cond_type->equals(Types::bool_type)) {

        // Must cast
        auto new_cond_type =
                gen_cast_code(ctx,
                              cond_type, Types::bool_type,
                              boolvisitor.code_ctx);
    }

    // Visit each side with a different code context

    // visit true
    FuncbodyVisitor truevisitor(this->code_ctx->create_block());
    truevisitor.lin = ltrue;
    truevisitor.lout = this->lout;
    bodytrue_type =  truevisitor.visit(ctx->bodytrue);

    // visit false
    FuncbodyVisitor falsevisitor(this->code_ctx->create_block());
    falsevisitor.lin = lfalse;
    falsevisitor.lout = this->lout;
    bodyfalse_type = falsevisitor.visit(ctx->bodyfalse);

    if (!bodytrue_type || !bodyfalse_type) {
        Report::err(ctx) << "IMPL ERROR: BODYTRUE FUNCBODY RETURNED NULL TYPE"
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop true or false")
                  << Instruction("push", {0}).with_annot("type(int)");

        return Types::int_type;
    }

    Type::const_pointer rtype =
            gen_coalesce_code(bodytrue_type, truevisitor.code_ctx,
                              bodyfalse_type, falsevisitor.code_ctx);

    if (!rtype) {
        Report::err(ctx) << "Could not coalesce types " << bodytrue_type
                          << " and " << bodyfalse_type
                          << " . Maybe a cast?"
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop true or false")
                  << Instruction("push", {0}).with_annot("type(int)");

        return Types::int_type;
    }

    *code_ctx
            << *boolvisitor.code_ctx
            << *truevisitor.code_ctx
            << *falsevisitor.code_ctx;

    make_out_jump();

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

    FuncbodyVisitor decls_visitor(this->code_ctx->create_subcontext());

    // visit letlist, populate the new symbol table
    decls_visitor.visit(ctx->letlist());

    FuncbodyVisitor exe_visitor(this->code_ctx->create_block());
    exe_visitor.lout = this->lout;
    exe_visitor.lin = exe_label;

    Type::const_pointer ltype = exe_visitor.visit(ctx->fnested);

    if (!ltype)
    {
        Report::err(ctx) << "IMPL ERROR LET EXPR HAS NIL TYPE" << endl;
        ltype = Types::int_type;
    }

    *this->code_ctx
            << std::move(*decls_visitor.code_ctx)
            << std::move(*exe_visitor.code_ctx);

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
    fbvisitor.lout = lcont;

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
            << Instruction("store", {sym->pos})
            .with_label(lcont)
            .with_annot("type(" + symbol_type->name() + ")");

    return code_ctx->symbol_table->offset;
}

antlrcpp::Any FuncbodyVisitor::visitLetvarresult_ignore_rule(MMMLParser::Letvarresult_ignore_ruleContext *ctx)
{
    // let _  = funcbody
    auto lcont = LabelFactory::make();

    FuncbodyVisitor fbvisitor(code_ctx->create_block());
    fbvisitor.lout = lcont;

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
    fbvisitor.lout = lcont;

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
