/****************************************************************************
 *        Filename: "MMML/TypedArgListVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Tue Oct  3 17:11:46 2017"
 *         Updated: "2017-10-10 17:05:43 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include "mmml/TypedArgListVisitor.H"
#include "mmml/error.H"
#include "mmml/TypeRegistry.H"
#include "mmml/basic_types.H"

namespace mmml {

// TypedArgVisitor
antlrcpp::Any
TypedArgListVisitor::visitTyped_arg_list_rule(MMMLParser::Typed_arg_list_ruleContext *ctx)
{
    Symbol::pointer sym = visit(ctx->typed_arg());

    if (param_names.find(sym->name) != param_names.end())
    {
        Report::err(ctx) << "Duplicate symbol found in parameter list: "
                                       << sym->name
                                       << endl;
    } else {
        result.push_back(sym);
    }

    return visit(ctx->typed_arg_list_cont());
}

antlrcpp::Any
TypedArgListVisitor::visitTyped_arg(MMMLParser::Typed_argContext *ctx)
{

    Type::const_pointer type = this->visit(ctx->type());
    //TypeRegistry::instance().find_by_name(ctx->type()->getText());

    if (!type) {
        Report::err(ctx) << "Unknown type " << ctx->type()->getText()
                                       << endl;
        type = Types::int_type;
    }

    auto sstart = ctx->symbol()->getStart();
    auto sym = make_shared<Symbol>(ctx->symbol()->getText(), type,
                                   sstart->getLine(),
                                   sstart->getCharPositionInLine());

    return sym;
}

antlrcpp::Any
TypedArgListVisitor::visitTyped_arg_list_cont_rule(MMMLParser::Typed_arg_list_cont_ruleContext *ctx)
{
    return visit(ctx->typed_arg_list());
}

antlrcpp::Any
TypedArgListVisitor::visitTyped_arg_list_end(MMMLParser::Typed_arg_list_endContext *ctx)
{
    return this->result;
}

antlrcpp::Any
TypedArgListVisitor::visitType_basictype_rule(MMMLParser::Type_basictype_ruleContext *ctx)
{
    return type_registry.find_by_name(ctx->basic_type()->getText());
}

antlrcpp::Any
TypedArgListVisitor::visitType_tuple_rule(MMMLParser::Type_tuple_ruleContext *ctx)
{
    TupleType::base_types_vector_t types;
    for (auto type : ctx->type()) {
        Type::const_pointer type_ptr = this->visit(type);
        if (!type_ptr)
        {
            Report::err(ctx) << "Could not find type ``"
                             << type->getText()
                             << "´´"
                             << endl;
            continue;
        }

        types.push_back(type_ptr);
    }

    auto tuple_type = make_shared<TupleType>(types);

    return type_registry.add(tuple_type);
}

antlrcpp::Any TypedArgListVisitor::visitType_custom_rule(MMMLParser::Type_custom_ruleContext *ctx)
{
    Type::const_pointer type = type_registry.find_by_name(ctx->custom_type_name()->getText());

    return type;
}

antlrcpp::Any TypedArgListVisitor::visitType_sequence_rule(MMMLParser::Type_sequence_ruleContext *ctx)
{
    Type::const_pointer base = this->visit(ctx->type());
    if (!base) {
        Report::err(ctx) << "Could not find base type ``"
                         << ctx->type()->getText()
                         << "´´ for sequence creation"
                         << endl;
        return Types::int_type;
    }

    auto seq_type = make_shared<SequenceType>(base);

    return type_registry.add(seq_type);
}


}  // mmml
