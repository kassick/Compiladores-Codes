grammar MMML;

@parser::namespace { mimimil }
@lexer::namespace { mimimil }

/*
Programa: Declarações de funções e uma função main SEMPRE

def fun x = x + 1

def main =
  let x = read_int
  in
     print concat "Resultado" (string (fun x))
*/

WS : [ \r\t\u000C\n]+ -> skip;

COMMENT : '//' ~('\n'|'\r')* '\r'? '\n' -> channel(HIDDEN);

program
    : fdecls maindecl
    ;

fdecls
    : fdecl fdecls  #fdecls_one_decl_rule
    | /*empty*/     #fdecls_end_rule
    ;

maindecl
  : 'def' 'main' '=' funcbody #programmain_rule
  ;

fdecl
    :  'def' functionname fdeclparams '=' funcbody #funcdef_rule
    |  'def' functionname fdeclparams '->' type    #funcdef_definition
    ;

fdeclparams
  :   fdeclparam fdeclparams_cont #fdeclparams_one_param_rule
  |                               #fdeclparams_no_params
  ;

fdeclparams_cont
  : ',' fdeclparam fdeclparams_cont #fdeclparams_cont_rule
  |                                 #fdeclparams_end_rule
  ;

fdeclparam
  : symbol ':' type #fdecl_param_rule
  ;

functionname
    : TOK_ID                                 #fdecl_funcname_rule
    ;

type
    : basic_type    #basictype_rule
    | sequence_type #sequencetype_rule
    ;

basic_type
    : 'int'
    | 'bool'
    | 'str'
    | 'float'
    ;

sequence_type
  :   basic_type '[]'      #sequencetype_basetype_rule
  |   s=sequence_type '[]' #sequencetype_sequence_rule
  ;

funcbody
  :   'if' cond=funcbody 'then' bodytrue=funcbody 'else' bodyfalse=funcbody #fbody_if_rule
  |   'let' letlist 'in' fnested=funcbody                                   #fbody_let_rule
  |   metaexpr                                                              #fbody_expr_rule
  ;

letlist
  : letvarexpr  letlist_cont  #letlist_rule
  ;

letlist_cont
  :   ',' letvarexpr letlist_cont #letlist_cont_rule
  |   /*empty*/                   #letlist_cont_end ;

letvarexpr
  :   sym=symbol '=' funcbody          #letvarattr_rule
  |    '_'    '=' funcbody             #letvarresult_ignore_rule
  |    symbol '::' symbol '=' funcbody #letunpack_rule
  ;

metaexpr
    : '(' funcbody ')'                           #me_exprparens_rule     // Anything in parenthesis -- if, let, funcion call, etc
    | sequence_expr                              #me_list_create_rule    // creates a list [x]
    | TOK_NEG symbol                             #me_boolneg_rule        // Negate a variable
    | TOK_NEG '(' funcbody ')'                   #me_boolnegparens_rule  // or anything in between ( )
    | l=metaexpr op=TOK_POWER r=metaexpr         #me_exprpower_rule      // Exponentiation
    | l=metaexpr op=TOK_CONCAT r=metaexpr        #me_listconcat_rule     // Sequence concatenation
    | l=metaexpr op=TOK_DIV_OR_MUL r=metaexpr    #me_exprmuldiv_rule     // Div and Mult are equal
    | l=metaexpr op=TOK_PLUS_OR_MINUS r=metaexpr #me_exprplusminus_rule  // Sum and Sub are equal
    | metaexpr TOK_CMP_GT_LT metaexpr            #me_boolgtlt_rule       // < <= >= > are equal
    | metaexpr TOK_CMP_EQ_DIFF metaexpr          #me_booleqdiff_rule     // == and != are egual
    | metaexpr TOK_BOOL_AND_OR metaexpr          #me_boolandor_rule      // &&   and  ||  are equal
    | symbol                                     #me_exprsymbol_rule     // a single symbol
    | literal                                    #me_exprliteral_rule    // literal value
    | funcall                                    #me_exprfuncall_rule    // a funcion call
    | cast                                       #me_exprcast_rule       // cast a type to other
    ;

sequence_expr
  : '[' funcbody ']'                               #se_create_seq
  ;

funcall
  : symbol funcall_params #funcall_rule
  ;

cast
  : c=type funcbody #cast_rule
  ;

funcall_params
  :   metaexpr funcall_params_cont #funcallparams_rule
  |   '_'                          #funcallnoparam_rule
  ;

funcall_params_cont
  :   metaexpr funcall_params_cont #funcall_params_cont_rule
  |   /*empty*/                    #funcall_params_end_rule
  ;

literal
  :   'nil'              #literalnil_rule
  |   ('true' | 'false') #literaltrueorfalse_rule
  |   number             #literalnumber_rule
  |   strlit             #literalstring_rule
  |   charlit            #literal_char_rule
  ;

strlit: TOK_STR_LIT ;

charlit : TOK_CHAR_LIT ;

number
    :   FLOAT       #numberfloat_rule
    |   DECIMAL     #numberdecimal_rule
    |   HEXADECIMAL #numberhexadecimal_rule
    |   BINARY      #numberbinary_rule
    ;

symbol
    : TOK_ID     #symbol_rule
    ;

// id: begins with a letter, follows letters, numbers or underscore
TOK_ID: [a-zA-Z]([a-zA-Z0-9_]*);
TOK_CONCAT: '::' ;
TOK_NEG: '!';
TOK_POWER: '^' ;
TOK_DIV_OR_MUL: ('/'|'*');
TOK_PLUS_OR_MINUS: ('+'|'-');
TOK_CMP_GT_LT: ('<='|'>='|'<'|'>');
TOK_CMP_EQ_DIFF: ('=='|'!=');
TOK_BOOL_AND_OR: ('&&'|'||');
TOK_REL_OP : ('>'|'<'|'=='|'>='|'<=') ;

// TOK_STR_LIT
// : '"' (~[\"\\\r\n] | '\\' (. | EOF))* '"'
// ;

TOK_STR_LIT
    :   '"'
        ( ~('"' | [\n\r] ) | '\\' [a-z] )*
        '"'
    ;

TOK_CHAR_LIT
    :   '\''
        ( '\\' [a-z] | ~('\'') ) // Um escape (\a, \n, \t) ou qualquer coisa que não seja aspas (a, b, ., z, ...)
        '\'' ;

FLOAT : '-'? DEC_DIGIT+ '.' DEC_DIGIT+([eE][+-]? DEC_DIGIT+)? ;

DECIMAL : '-'? DEC_DIGIT+ ;

HEXADECIMAL : '0' 'x' HEX_DIGIT+ ;

BINARY : BIN_DIGIT+ 'b' ; // Sequencia de digitos seguida de b  10100b

fragment
BIN_DIGIT : [01];

fragment
HEX_DIGIT : [0-9A-Fa-f];

fragment
DEC_DIGIT : [0-9] ;
