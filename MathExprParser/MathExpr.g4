grammar MathExpr;

WS : [ \r\t\u000C\n]+ -> skip;

start : expr EOF ;

expr
    : '(' expr ')'
        #exprParensRule
    |   l=expr op=TOK_DIV_OR_MUL r=expr
        #exprDivOrMulRule
    |   l=expr op=TOK_PLUS_OR_MINUS r=expr
        #exprPlusOrMinusRule
    |   number
        #exprNumberRule
    ;

number
    :   FLOAT       #numberfloatRule
    |   DECIMAL     #numberdecimalRule
    |   HEXADECIMAL #numberhexadecimalRule
    |   BINARY      #numberbinaryRule
    ;

TOK_DIV_OR_MUL: ('/'|'*');
TOK_PLUS_OR_MINUS: ('+'|'-');
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
