/****************************************************************************
 *        Filename: "MMML/include/mmml/CodeVisitor.H"
 *
 *     Description:
 *
 *         Version: 1.0
 *         Created: "Fri Sep 29 19:45:16 2017"
 *         Updated: "2017-09-30 01:10:44 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/


#pragma once

#include "mmml/TypeRegistry.H"
#include "mmml/CodeContext.H"
#include "MMMLBaseVisitor.h"

namespace mmml {

struct LabelFactory {

    static string make() {
        static int next = 0;

        return "l" + to_string(next++);
    }
};

class  CodeVisitor : public MMMLBaseVisitor {

  public:
    TypeRegistry& type_registry = TypeRegistry::instance();
    Type::const_pointer bool_type = type_registry.find_by_name("bool");
    Type::const_pointer char_type = type_registry.find_by_name("char");
    Type::const_pointer int_type = type_registry.find_by_name("int");
    Type::const_pointer float_type = type_registry.find_by_name("float");
    Type::const_pointer nil_type = type_registry.find_by_name("nil");

    // Static: Set this so we don't have to set it every time we construct
    static ostream *err_stream, *out_stream;

    static ostream& out() {
        if (out_stream)
            return *out_stream;

        return cout;
    }

    static ostream& err() {
        if (err_stream)
            return *err_stream;

        return cerr;
    }

    CodeContext::pointer code_ctx;
    int nerrors = 0;
    int nwarns = 0;

    ostream& warn(antlr4::ParserRuleContext * ctx, const string message = "")
    {
        err() << "Warning at line " << ctx->getStart()->getLine()
              << ", column " << ctx->getStart()->getCharPositionInLine()
              << " : ";
        if (message.length() > 0)
            err() << message << endl;

        nwarns++;

        return err();
    }

    ostream& report_error(antlr4::ParserRuleContext * ctx, const string message = "")
    {
        err() << "Error at line" << ctx->getStart()->getLine()
              << ", column " << ctx->getStart()->getCharPositionInLine()
              << " : ";

        if (message.length() > 0)
            err() << message << endl;

        this->nerrors++;

        return err();
    }

    Type::const_pointer gen_cast_code(antlr4::ParserRuleContext*,
                                      Type::const_pointer source_type,
                                      Type::const_pointer dest_type);

    virtual antlrcpp::Any visitLiteral_float_rule(MMMLParser::Literal_float_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLiteral_decimal_rule(MMMLParser::Literal_decimal_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLiteral_hexadecimal_rule(MMMLParser::Literal_hexadecimal_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLiteral_binary_rule(MMMLParser::Literal_binary_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLiteralstring_rule(MMMLParser::Literalstring_ruleContext *ctx) override ;
    virtual antlrcpp::Any visitLiteral_char_rule(MMMLParser::Literal_char_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLiteralnil_rule(MMMLParser::Literalnil_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitLiteraltrueorfalse_rule(MMMLParser::Literaltrueorfalse_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitSymbol_rule(MMMLParser::Symbol_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitCast_rule(MMMLParser::Cast_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitSeq_create_seq(MMMLParser::Seq_create_seqContext *ctx) override ;

    virtual antlrcpp::Any visitMe_class_get_rule(MMMLParser::Me_class_get_ruleContext *ctx) override ;

    virtual antlrcpp::Any visitMe_class_set_rule(MMMLParser::Me_class_set_ruleContext *ctx) override ;

};

/*
class  __CodeVisitor : public MMMLBaseVisitor {
public:

  virtual antlrcpp::Any visitProgram(MMMLParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecls_one_decl_rule(MMMLParser::Decls_one_decl_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecls_end_rule(MMMLParser::Decls_end_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitProgrammain_rule(MMMLParser::Programmain_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncdef_impl(MMMLParser::Funcdef_implContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncdef_definition(MMMLParser::Funcdef_definitionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecl_custom_type(MMMLParser::Decl_custom_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTyped_arg_list_rule(MMMLParser::Typed_arg_list_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTyped_arg(MMMLParser::Typed_argContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTyped_arg_list_cont_rule(MMMLParser::Typed_arg_list_cont_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTyped_arg_list_end(MMMLParser::Typed_arg_list_endContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCustom_type_decl_rule(MMMLParser::Custom_type_decl_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType_custom_rule(MMMLParser::Type_custom_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType_sequence_rule(MMMLParser::Type_sequence_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType_basictype_rule(MMMLParser::Type_basictype_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitType_tuple_rule(MMMLParser::Type_tuple_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCustom_type_name_rule(MMMLParser::Custom_type_name_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBasic_type(MMMLParser::Basic_typeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFdecl_funcname_rule(MMMLParser::Fdecl_funcname_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFbody_if_rule(MMMLParser::Fbody_if_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFbody_let_rule(MMMLParser::Fbody_let_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFbody_expr_rule(MMMLParser::Fbody_expr_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLetlist_rule(MMMLParser::Letlist_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLetlist_cont_rule(MMMLParser::Letlist_cont_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLetlist_cont_end(MMMLParser::Letlist_cont_endContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLetvarattr_rule(MMMLParser::Letvarattr_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLetvarresult_ignore_rule(MMMLParser::Letvarresult_ignore_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLetunpack_rule(MMMLParser::Letunpack_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTuple_ctor(MMMLParser::Tuple_ctorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitClass_ctor(MMMLParser::Class_ctorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_exprplusminus_rule(MMMLParser::Me_exprplusminus_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_boolneg_rule(MMMLParser::Me_boolneg_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_exprliteral_rule(MMMLParser::Me_exprliteral_ruleContext *ctx) override {
    return visitChildren(ctx);
  }


  virtual antlrcpp::Any visitMe_listconcat_rule(MMMLParser::Me_listconcat_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_list_create_rule(MMMLParser::Me_list_create_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_exprmuldiv_rule(MMMLParser::Me_exprmuldiv_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_exprcast_rule(MMMLParser::Me_exprcast_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_exprparens_rule(MMMLParser::Me_exprparens_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_boolandor_rule(MMMLParser::Me_boolandor_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_class_ctor_rule(MMMLParser::Me_class_ctor_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_tuple_access_rule(MMMLParser::Me_tuple_access_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_exprsymbol_rule(MMMLParser::Me_exprsymbol_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_exprfuncall_rule(MMMLParser::Me_exprfuncall_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_boolgtlt_rule(MMMLParser::Me_boolgtlt_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_tup_create_rule(MMMLParser::Me_tup_create_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_boolnegparens_rule(MMMLParser::Me_boolnegparens_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMe_booleqdiff_rule(MMMLParser::Me_booleqdiff_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncall_rule(MMMLParser::Funcall_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncallparams_rule(MMMLParser::Funcallparams_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncallnoparam_rule(MMMLParser::Funcallnoparam_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncall_params_cont_rule(MMMLParser::Funcall_params_cont_ruleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFuncall_params_end_rule(MMMLParser::Funcall_params_end_ruleContext *ctx) override {
    return visitChildren(ctx);
  }


};

*/


} // end namespace mmml ///////////////////////////////////////////////////////
