/****************************************************************************
 *        Filename: "MMML/CodeVisitor.cpp"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:44:30 2017"
 *         Updated: "2017-09-29 21:20:51 kassick"
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
              << Instruction("acreate").with_annot("type(str)");

    return type_registry.add(make_shared<SequenceType>(char_type));
}

antlrcpp::Any CodeVisitor::visitLiteral_char_rule(MMMLParser::Literal_char_ruleContext *ctx)  {
    *code_ctx << Instruction("push", { ctx->getText() })
            .with_annot("type(char)" );
    return char_type;
}

antlrcpp::Any CodeVisitor::visitSymbol_rule(MMMLParser::Symbol_ruleContext *ctx)  {

    auto s = this->code_ctx->symbol_table->find(ctx->getText());
    if (!s) {
        err() << "Unknown symbol " << ctx->getText()
              << " at " << ctx->getStart()->getLine()
              << " : " << ctx->getStart()->getCharPositionInLine()
              << endl;

        *code_ctx << Instruction("push", {0}).with_annot("type(int)");
        return int_type;
    }

    *code_ctx <<
            Instruction("load", {s->pos})
            .with_annot("type(" + type_name(s->type()) + ")");

    return s->type();
}

// Static storage for out_stream and err_stream
ostream* CodeVisitor::out_stream = nullptr ;
ostream* CodeVisitor::err_stream = nullptr ;

} // end namespace mmml ///////////////////////////////////////////////////////
