/****************************************************************************
 *        Filename: "MMML/ToplevelVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Wed Oct  4 10:09:35 2017"
 *         Updated: "2017-10-04 14:14:08 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/ToplevelVisitor.H"
#include "mmml/FunctionRegistry.H"
#include "mmml/TypedArgListVisitor.H"
#include "mmml/error.H"
#include "mmml/utils.H"

namespace mmml
{

antlrcpp::Any ToplevelVisitor::visitProgrammain_rule(MMMLParser::Programmain_ruleContext *ctx)
{
    auto funcStart = ctx->getStart();
    auto fbody_start = ctx->funcbody()->getStart();

    ///// Now we visit the function body
    ToplevelVisitor funcvisitor;
    funcvisitor.code_ctx = make_shared<CodeContext>();
    auto st = funcvisitor.code_ctx->symbol_table;

    // return point always on 0
    st->add(make_shared<Symbol>("@ret_point", Types::int_type,
                                funcStart->getLine(), funcStart->getCharPositionInLine()));

    Type::const_pointer fb_ret = funcvisitor.visit(ctx->funcbody());

    // Now merge the function call:
    *code_ctx << Instruction("nop").with_label("start").with_annot("main function")
              << std::move(*funcvisitor.code_ctx)
              << Instruction("print")
              << Instruction("exit");


    return nullptr;
}

static
Function::pointer visit_function_header(ToplevelVisitor* visitor,
                                        antlr4::ParserRuleContext* ctx,
                                        const string name, MMMLParser::Typed_arg_listContext* args)
{
    auto f = FunctionRegistry::instance().find(name);

    if (f)
        return f;

    std::vector<Symbol::const_pointer> plist = TypedArgListVisitor().visit(args);

    auto nf = make_shared<Function>(name,
                                    plist,
                                    ctx->getStart()->getLine(),
                                    ctx->getStart()->getCharPositionInLine());

    auto rf = FunctionRegistry::instance().add(nf);

    if (nf != rf)
    {
        Report::err(ctx) << "IMPL ERROR REGISTRY GOT NULL IN ADD" << endl;
    }

    return rf;
}

antlrcpp::Any ToplevelVisitor::visitFuncdef_impl(MMMLParser::Funcdef_implContext *ctx)
{
    auto funcStart = ctx->getStart();
    auto fbody_start = ctx->funcbody()->getStart();

    auto f = visit_function_header(this,
                                   ctx,
                                   ctx->functionname()->getText(),
                                   ctx->typed_arg_list());

    if (!f) {
        Report::err(ctx) << "IMPL ERROR ON FUNCDEF_IMPL"
                          << endl;
        return f;
    }

    if (f->implemented)
    {
        // oops, redefining previously defined function is an error
        Report::err(ctx) << "Re-implementation of function " << f->name
                          << " previously implemented in line " << f->impl_line
                          << ", column " << f->impl_col
                          << endl;
        return f;
    }

    if (!f->is_sane()) {
        Report::err(ctx) << "Defined function is not sane (repeated args or unknown types)"
                          << endl;
        return f;
    }

    // Now prepare for code generation
    auto out_label = LabelFactory::make();

    // Set implemented BEFORE, so we know that the function is implemented if called recursively
    f->set_implemented(
        LabelFactory::make(),
        fbody_start->getLine(),
        fbody_start->getCharPositionInLine());

    // Default: it recurses
    f->rtype = Types::recursive_type;

    ///// Now we visit the function body
    // FuncbodyVisitor
    ToplevelVisitor funcvisitor;
    // funcvisitor.out_label = out_label;
    funcvisitor.code_ctx = make_shared<CodeContext>();
    auto st = funcvisitor.code_ctx->symbol_table;

    // return point always on 0
    st->add(make_shared<Symbol>("@ret_point", Types::int_type,
                                funcStart->getLine(), funcStart->getCharPositionInLine()));

    // args start at 1
    for (const auto & arg: f->args)
    {
        auto s = make_shared<Symbol>(arg->name, arg->type(),
                                     funcStart->getLine(),
                                     funcStart->getCharPositionInLine());
        st->add(s);
    }

    // this one will be ignored later
    auto nested_to_drop = st->make_nested();
    funcvisitor.code_ctx->symbol_table = nested_to_drop;

    Type::const_pointer fb_ret = funcvisitor.visit(ctx->funcbody());

    if (!fb_ret) {
        Report::err(ctx) << "IMPL ERROR GOT NULL FROM FUCBODY IN FUNC IMPL" << endl;
        f->rtype = Types::int_type;

        return f;
    }

    if (fb_ret->equals(Types::recursive_type))
    {
        Report::err(ctx) << "Function " << f->name
                          << "always recurses into itself. Can not resolve recursion"
                          << endl;
        f->rtype = Types::int_type;
    }

    // Now drop the temporary symbol table and visit again to resolve symbols
    funcvisitor.code_ctx->symbol_table = st;
    fb_ret = funcvisitor.visit(ctx->funcbody());

    /*
      Callee cleanup: Remove all symbols from stack, leave only return and ret_point

      Stack:
      4 : retval
      3 : arg3
      2 : arg2
      1 : arg1
      0 : retpoint
      ------------
      -1 : ...

      crunch 1 symbol_table.size() - 1

      1 : retval
      0 : retpoint
      ------------
      -1 : ...

      swap

      1 : retpoint
      0 : retval
      ------------
      -1 : ...

      drop_mark

      51 : retpoint
      50 : retval
      49 : ...

      jump
     */

    // Now merge the function call:
    *code_ctx <<
            Instruction("nop")
            .with_label(f->label)
            .with_annot("function " + f->name)
              <<
            std::move(*funcvisitor.code_ctx)
              <<
            Instruction("crunch",
                        {1, funcvisitor.code_ctx->symbol_table->size() - 1})
            .with_label(out_label)
            .with_annot("return")
              <<
            Instruction("swap")
              <<
            Instruction("jump").with_annot("jump to return point");

    return f;
}


antlrcpp::Any ToplevelVisitor::visitFuncdef_header(MMMLParser::Funcdef_headerContext *ctx)
{
    auto funcStart = ctx->getStart();

    auto f = visit_function_header(this,
                                   ctx,
                                   ctx->functionname()->getText(),
                                   ctx->typed_arg_list());

    if (!f) {
        Report::err(ctx) << "IMPL ERROR ON FUNCDEF_IMPL"
                          << endl;
        return f;
    }

    if (f->implemented)
    {
        // oops, redefining previously defined function is an error
        Report::err(ctx) << "Header-definition of function "
                          << f->name
                          << "appears after it's implementation on line "
                          << f->impl_line << ", column " << f->impl_col
                          << endl;
        return f;
    }

    if (!f->is_sane()) {
        Report::err(ctx) << "Defined function is not sane (repeated args or unknown types)"
                          << endl;
        return f;
    }

    f->rtype = type_registry.find_by_name(ctx->type()->getText());
    if (!f->rtype) {
        Report::err(ctx) << "Unknown type in function " << f->name
                          << endl;

        f->rtype = Types::int_type;
    }

    return f;
}


antlrcpp::Any ToplevelVisitor::visitCustom_type_decl_rule(MMMLParser::Custom_type_decl_ruleContext *ctx)
{
    auto name = ctx->custom_type_name()->getText();
    auto oldtype = type_registry.find_by_name(name);

    std::vector<Symbol::const_pointer> plist = TypedArgListVisitor().visit(ctx->typed_arg_list());

    if (oldtype) {
        Report::err(ctx) << "Type ``" << name
                         << " has already been defined"
                         << endl;
    }

    auto newclass = make_shared<ClassType>(name, plist);

    type_registry.add(newclass);

    return nullptr;
}

} // end namespace mmmml //////////////////////////////////////////////////////
