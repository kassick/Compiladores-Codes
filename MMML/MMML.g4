grammar MMML;

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

// Código de Programa µmML
program
    : fdecls maindecl EOF
    ;

// Declarações de Funções
// def f1 a : int, c : int = a + b
// def f2 a : int, c : int = (float a) / b
fdecls
    : fdecl fdecls  #fdecls_one_decl_rule
    | /*empty*/     #fdecls_end_rule
    ;

// Declaração da função principal
// def main = 1 + 1
maindecl
  : 'def' 'main' '=' funcbody #programmain_rule
  ;

// Declaração de uma função
// Header de Função:
// def f2 a : int, b : int -> float
// Implementação de Função:
// def f2 a : int, b : int = a / (float b)
fdecl
    :  'def' functionname fdeclparams '=' funcbody #funcdef_rule
    |  'def' functionname fdeclparams '->' type    #funcdef_definition
    ;

// Lista de Parâmetros:
//   a : int      , b : int, c : int
// |----------| |--------------------|
//    param             cont
fdeclparams
  :   fdeclparam fdeclparams_cont #fdeclparams_one_param_rule
  |                               #fdeclparams_no_params
  ;

// Continuacao da Lista de Parâmetros
//   b : int          , c : int
// |----------| |--------------------|
//    param             cont
fdeclparams_cont
  : ',' fdeclparam fdeclparams_cont #fdeclparams_cont_rule
  |  /*vazio*/                      #fdeclparams_end_rule
  ;

// Declaração de Parâmetro
//    a        :      int
// |-------|      |---------|
//  symbol          type
fdeclparam
  : symbol ':' type #fdecl_param_rule
  ;

// Nome de Função
functionname
    : TOK_ID                                 #fdecl_funcname_rule
    ;

// Tipo
// int , char, etc.
// int[], char[], etc
// int[], char[][], etc.
type
    : basic_type    #basictype_rule
    | sequence_type #sequencetype_rule
    ;

// Tipos Básicos da Linguagem
basic_type
    :   'char'
    |   'int'
    |   'bool'
    |   'str'
    |   'float'
    ;

// Tipos Sequência:
// Sequência de um tipo base: int[], char[], etc.
// Sequência de um tipo sequência: int[][], etc
sequence_type
  :   basic_type '[]'      #sequencetype_basetype_rule
  |   s=sequence_type '[]' #sequencetype_sequence_rule
  ;

// Corpo de Função:
// if x == y then x else x + y
// ou
// let x = 1 + 2, y = 3+1 in (f x y)
// ou
// 1 + 2 + 3
funcbody
  :   'if' cond=funcbody 'then' bodytrue=funcbody 'else' bodyfalse=funcbody #fbody_if_rule
  |   'let' letlist 'in' fnested=funcbody                                   #fbody_let_rule
  |   metaexpr                                                              #fbody_expr_rule
  ;

// Lista de declarações
//   x = 1        , y = 2
// |--------|  |----------|
//  expr           cont
letlist
  : letvarexpr  letlist_cont  #letlist_rule
  ;

letlist_cont
  :   ',' letvarexpr letlist_cont #letlist_cont_rule
  |   /*empty*/                   #letlist_cont_end ;

// Atribuição:
// x = 1 + 2
// ou
// _ = 1 + 2
// ou
// x::rest = l
letvarexpr
  :   sym=symbol '=' funcbody          #letvarattr_rule
  |    '_'    '=' funcbody             #letvarresult_ignore_rule
  |    symbol '::' symbol '=' funcbody #letunpack_rule
  ;

// Meta Expressão:
// Booleanas
// a && b
// a || b
// !a
// Concatenação de listas
// a :: b
// Criação de lista
// [a]
// Matemáticas
// a / b
// a + b
// Relacionais
// a <= b
// a >= b
// Símbolo
// a
// Literais
// 1
// 2.4
// 0xafbe
// 1010b
// Chamada de Função
// f a b
// Cast
// int a
metaexpr
    : '(' funcbody ')'                           #me_exprparens_rule     // Anything in parenthesis -- if, let, funcion call, etc
    | sequence_expr                              #me_list_create_rule    // creates a list [x]
    | TOK_NEG symbol                             #me_boolneg_rule        // Negate a variable
    | TOK_NEG '(' funcbody ')'                   #me_boolnegparens_rule  // or anything in between ( )
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

// Criação de sequência:
// [a + b]
sequence_expr
  : '[' funcbody ']'                               #se_create_seq
  ;

// Chamada de função
// f a b
funcall
  : symbol funcall_params #funcall_rule
  ;

// Parâmetros de Função
//    a + b        c d
// |-------|   |-------|
//   expr         cont
funcall_params
    :   metaexpr funcall_params_cont #funcallparams_rule
    |   '_'                          #funcallnoparam_rule
    ;

// Continuação dos Parâmetros
//   c       d
// |----|  |-------|
//  expr    cont
funcall_params_cont
    :   metaexpr funcall_params_cont #funcall_params_cont_rule
    |   /*empty*/                    #funcall_params_end_rule
    ;

// Cast
// int b
// char 65
cast
  : c=type funcbody #cast_rule
  ;

literal
    :   'nil'              #literalnil_rule
    |   ('true' | 'false') #literaltrueorfalse_rule
    |   FLOAT              #literal_float_rule
    |   DECIMAL            #literal_decimal_rule
    |   HEXADECIMAL        #literal_hexadecimal_rule
    |   BINARY             #literal_binary_rule
    |   TOK_STR_LIT        #literalstring_rule
    |   TOK_CHAR_LIT       #literal_char_rule
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
    :   '"' // open string
        ( ~('"' | '\n' | '\r' ) | '\\' [a-z"] )*
        '"' // close string
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
