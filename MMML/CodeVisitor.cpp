/****************************************************************************
 *        Filename: "MMML/CodeVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:44:30 2017"
 *         Updated: "2017-10-03 17:14:09 kassick"
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

namespace mmml {

using namespace std;
using stringvector = std::vector<std::string>;

static Symbol::const_pointer mmml_load_symbol(const string name, CodeContext::pointer code_ctx)
{
    auto s = code_ctx->symbol_table->find(name);
    if (s) {
        *code_ctx << Instruction("load", {s->pos}).with_annot("type(" + type_name(s->type()) + ")");
        return s;
    }

    return nullptr;
}

static
Function::pointer visit_function_header(CodeVisitor* visitor,
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

antlrcpp::Any CodeVisitor::visitFuncdef_impl(MMMLParser::Funcdef_implContext *ctx)
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

    // Set implemented BEFORE, so we know that the function is implemented if called recursively
    f->set_implemented(fbody_start->getLine(), fbody_start->getCharPositionInLine());

    // Default: it recurses
    f->rtype = Types::recursive_type;

    ///// Now we visit the function body
    CodeVisitor funcvisitor;
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
    *code_ctx << Instruction("nop").with_label(f->name).with_annot("function " + f->name)
              << std::move(*funcvisitor.code_ctx)
              << Instruction("crunch",
                             {1, funcvisitor.code_ctx->symbol_table->size() - 1})
              << Instruction("swap")
              << Instruction("jump").with_annot("jump to return point");

    return f;
}
antlrcpp::Any CodeVisitor::visitFuncdef_header(MMMLParser::Funcdef_headerContext *ctx)
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
    auto coalesced_type = CodeVisitor::gen_coalesce_code(ltype, lcode, rtype, rcode);

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

Type::const_pointer CodeVisitor::gen_cast_code(antlr4::ParserRuleContext * ctx,
                                               Type::const_pointer source_type,
                                               Type::const_pointer dest_type,
                                               CodeContext::pointer code_ctx)
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
        return Types::int_type;
    }

    Report::err() << "SHOULD NEVER REATH THIS POINT IN GEN_CAST_CODE" << endl;
    return Types::int_type;
}

Type::const_pointer CodeVisitor::gen_coalesce_code(
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


antlrcpp::Any CodeVisitor::visitFbody_if_rule(MMMLParser::Fbody_if_ruleContext *ctx)
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

    cond_type = visit(ctx->cond).as<Type::const_pointer>();
    if (!cond_type) {
        Report::err(ctx) << "IMPL ERROR: COND FUNCBODY RETURNED NULL TYPE"
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop cond")
                  << Instruction("push", {0}).with_annot("type(bool)");

    } else if (!cond_type->equals(Types::bool_type)) {

        // Must cast
        auto new_cond_type = gen_cast_code(ctx, cond_type, Types::bool_type, this->code_ctx);
    }

    string ltrue  = LabelFactory::make();
    string lfalse = LabelFactory::make();
    string lcont  = LabelFactory::make();

    // Visit each side with a different code context

    // visit true
    CodeVisitor truevisitor;
    truevisitor.code_ctx = this->code_ctx->create_block();
    bodytrue_type =  truevisitor.visit(ctx->bodytrue);

    // visit false
    CodeVisitor falsevisitor;
    falsevisitor.code_ctx = this->code_ctx->create_block();
    bodyfalse_type = falsevisitor.visit(ctx->bodyfalse);


    if (!bodytrue_type || !bodyfalse_type) {
        Report::err(ctx) << "IMPL ERROR: BODYTRUE FUNCBODY RETURNED NULL TYPE"
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop true or false")
                  << Instruction("push", {0}).with_annot("type(int)");

        return Types::int_type;
    }

    Type::const_pointer rtype = gen_coalesce_code(bodytrue_type, truevisitor.code_ctx,
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

    *code_ctx << Instruction("bz", {lfalse})
              << Instruction("nop").with_label(ltrue)
              << *truevisitor.code_ctx
              << Instruction("jump", {lcont})
              << Instruction("nop").with_label(lfalse)
              << Instruction("nop").with_label(lcont);


    return rtype;
}

antlrcpp::Any CodeVisitor::visitFbody_let_rule(MMMLParser::Fbody_let_ruleContext *ctx)
{
    /*  let x = 1 + 1,
            y = { 1, 2, "lala" }
        in
            x + (get 1 y)
    */

    // New visitor with a nested name table
    CodeVisitor letvisitor;
    letvisitor.code_ctx = this->code_ctx->create_subcontext();

    // visit letlist, populate the new symbol table
    letvisitor.visit(ctx->letlist());

    Type::const_pointer ltype = letvisitor.visit(ctx->fnested);

    if (!ltype)
    {
        Report::err(ctx) << "IMPL ERROR LET EXPR HAS NIL TYPE" << endl;
        ltype = Types::int_type;
    }

    *this->code_ctx << std::move(*letvisitor.code_ctx);

    return ltype;
}

antlrcpp::Any CodeVisitor::visitLetvarattr_rule(MMMLParser::Letvarattr_ruleContext *ctx)
{
    // let x = funcbody
    Type::const_pointer symbol_type = visit(ctx->funcbody());

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
        *code_ctx << Instruction("pop",{}).with_annot("drop " + ctx->sym->getText());
    } else {
        auto sym = make_shared<Symbol>(ctx->sym->getText(),
                                       symbol_type,
                                       ctx->sym->getStart()->getLine(), ctx->sym->getStart()->getCharPositionInLine());

        code_ctx->symbol_table->add(sym);

        *code_ctx << Instruction("store", {sym->pos}).with_annot("type(" + symbol_type->name() + ")");
    }

    return code_ctx->symbol_table->offset;
}

antlrcpp::Any CodeVisitor::visitLetvarresult_ignore_rule(MMMLParser::Letvarresult_ignore_ruleContext *ctx)
{
    // let _  = funcbody
    Type::const_pointer symbol_type = visit(ctx->funcbody());

    if (!symbol_type) {
        Report::err(ctx) << "IMPL ERROR GOT NULL FROM FUNCBODY IN letvarattr"
                          << endl;
        symbol_type = Types::int_type;
    }

    *code_ctx << Instruction("pop",{}).with_annot("drop _");

    return code_ctx->symbol_table->offset;
}

antlrcpp::Any CodeVisitor::visitLetunpack_rule(MMMLParser::Letunpack_ruleContext *ctx)
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
     */

    Type::const_pointer list_type = visit(ctx->funcbody());
    if (!list_type) {
        Report::err(ctx) << "IMPL ERROR GOT NULL TYPE FROM UNPACK FUNCBODY" << endl;
        *code_ctx << Instruction("pop").with_annot("drop unpack");
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
        *code_ctx << Instruction("pop").with_annot("drop tail");
    } else {
        auto sym = make_shared<Symbol>(ctx->tail->getText(), list_type,
                                       ctx->tail->getStart()->getLine(),
                                       ctx->tail->getStart()->getCharPositionInLine());

        code_ctx->symbol_table->add(sym);

        *code_ctx << Instruction("store", {sym->pos}).with_annot("type(" + list_type->name() + ")");
    }

    auto previous_head = code_ctx->symbol_table->find_local(ctx->head->getText());
    if (previous_head) {
        Report::err(ctx) << "Re-definition of symbol ``" << previous_head->name
                          << " shadows previously defined in line "
                          << previous_head->line << ", column" << previous_head->col
                          << endl;
        *code_ctx << Instruction("pop").with_annot("drop head");
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

        *code_ctx << Instruction("store", {sym->pos}).with_annot("type(" + decayed_type->name() + ")");
    }

    return code_ctx->symbol_table->offset;
}


} // end namespace mmml ///////////////////////////////////////////////////////
