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
    : decls maindecl EOF
    ;

// Declarações de Funções
// def f1 a : int, c : int = a + b
// def f2 a : int, c : int = (float a) / b
decls
    : decl decls  #decls_one_decl_rule
    | /*empty*/   #decls_end_rule
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
decl
    :   'def' functionname typed_arg_list '=' funcbody
        #funcdef_impl
    |   'def' functionname typed_arg_list '->' type
        #funcdef_definition
    |   custom_type_decl
        #decl_custom_type
    ;

// Lista de Parâmetros:
//   a : int      , b : int, c : int
// |----------| |--------------------|
//    param             cont

typed_arg_list
    :   typed_arg typed_arg_list_cont
        #typed_arg_list_rule
    ;

typed_arg
    :   symbol ':' type
    ;

typed_arg_list_cont
    :   ',' typed_arg_list #typed_arg_list_cont_rule
    |   /*vazio*/          #typed_arg_list_end
    ;

// class MeuTipo = a : int, b : float, c : char[], d : {int, int}
custom_type_decl
    :   'class' custom_type_name '=' typed_arg_list
        #custom_type_decl_rule
    ;

// Tipo
// int , char, etc.
// int[], char[], etc
// int[], char[][], etc.
type
    :   basic_type               #type_basictype_rule
    |   '{' type (',' type)* '}' #type_tuple_rule
    |   custom_type_name         #type_custom_rule
    |   type '[]'                #type_sequence_rule
    ;

custom_type_name
    :   symbol                 #custom_type_name_rule
    ;

// // Tipos Básicos da Linguagem
basic_type
    :   'char'
    |   'int'
    |   'bool'
    |   'float'
    ;

// Nome de Função
functionname
    : TOK_ID                                 #fdecl_funcname_rule
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
tuple_ctor
    :   '{' funcbody (',' funcbody)* '}'
    ;

class_ctor
    : 'make'
        name=symbol tuple_ctor
    ;

metaexpr
    : '(' funcbody ')'                           #me_exprparens_rule     // Anything in parenthesis -- if, let, funcion call, etc
    | tuple_ctor                                 #me_tup_create_rule     // tuple creation
    | class_ctor                                 #me_class_ctor_rule     // create a class from
    | sequence_expr                              #me_list_create_rule    // creates a list [x]
    | TOK_NEG symbol                             #me_boolneg_rule        // Negate a variable
    | TOK_NEG '(' funcbody ')'                   #me_boolnegparens_rule  // or anything in between ( )
    | l=metaexpr op=TOK_CONCAT r=metaexpr        #me_listconcat_rule     // Sequence concatenation
    | l=metaexpr op=TOK_DIV_OR_MUL r=metaexpr    #me_exprmuldiv_rule     // Div and Mult are equal
    | l=metaexpr op=TOK_PLUS_OR_MINUS r=metaexpr #me_exprplusminus_rule  // Sum and Sub are equal
    | metaexpr TOK_CMP_GT_LT metaexpr            #me_boolgtlt_rule       // < <= >= > are equal
    | metaexpr TOK_CMP_EQ_DIFF metaexpr          #me_booleqdiff_rule     // == and != are egual
    | metaexpr TOK_BOOL_AND_OR metaexpr          #me_boolandor_rule      // &&   and  ||  are equal
    | 'get' pos=DECIMAL funcbody                 #me_tuple_access_rule   // get 0 funcTup
    | 'set' pos=DECIMAL funcbody                 #me_tuple_access_rule   // get 0 funcTup
    | 'get' name=symbol funcbody                 #me_class_get_rule      // get campo
    | 'set' name=symbol cl=funcbody val=funcbody #me_class_set_rule      // get campo
    | symbol                                     #me_exprsymbol_rule     // a single symbol
    | literal                                    #me_exprliteral_rule    // literal value
    | funcall                                    #me_exprfuncall_rule    // a funcion call
    | cast                                       #me_exprcast_rule       // cast a type to other
    ;

// Criação de sequência:
// [a + b]
sequence_expr
  : '[' funcbody ']'                               #seq_create_seq
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
  : c=basic_type funcbody #cast_rule
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
        '\''
    ;

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
