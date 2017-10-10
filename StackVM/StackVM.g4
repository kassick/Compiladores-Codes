grammar StackVM;

start
    :   instruction_line+
        EOF
    ;

instruction_line
    :   (label ':' )?
        NEWLINE*
        instruction?
        NEWLINE
        #instructionLine
    |   NEWLINE // linhas em branco
        #ignoreLine
    ;

instruction
    :   'nop'
        #nop
    |   'mark'
        #mark
    |   'mark' keep=non_negative_int_arg
        #markArg
    |   'pop_mark'
        #popMark
    |   'drop_mark'
        #dropMark
    |   'crunch'
        #crunchNoArgs
    |   'crunch' size=non_negative_int_arg
        #crunchBaseIndirect
    |   'crunch' base=non_negative_int_arg size=non_negative_int_arg
        #crunchDirect
    |   'trim'
        #trimIndirect
    |   'trim' base=non_negative_int_arg
        #trimDirect
    |   'swap'
        #swapTopWithNext
    |   'swap' a=non_negative_int_arg
        #swapTopWithArg
    |   'swap' a=non_negative_int_arg b=non_negative_int_arg
        #swap
    |  'dup'
        #dupTop
    |  'acreate'
        #arrayCreate
    |  'acreate' l=non_negative_int_arg
        #arrayCreateArg
    |  'aload'
        #arrayLoad
    |  'alen'
        #arrayLen
    |  'aget'
        #arrayGet
    |  'aget' i=non_negative_int_arg
        #arrayGetArg
    |  'aset'
        #arraySet
    |  'aset' i=non_negative_int_arg
        #arraySetArg
    |  'push' c=chararg
        #pushChar
    |  'push' i=intarg
        #pushInt
    |  'push' d=doublearg
        #pushDouble
    |  'push' s=strarg
        #pushString
    |  'push' l=ref_label
        #pushLabel
    |  'push' 'null'
        #pushNull
    |  'push' 'stack_size'
        #pushStackSize
    |  'pop'
        #pop
    |  'popn' i=non_negative_int_arg
        #popNImediate
    |  'popn'
        #popN
    |  'store'
        #storeIndirect
    |  'store' i=non_negative_int_arg
        #store
    |  'load'
        #loadIndirect
    |  'load' i=non_negative_int_arg
        #load
    |  'readc'
        #readChar
    |  'readi'
        #readInt
    |  'readf'
        #readFloat
    |  'readd'
        #readDouble
    |  'reads'
        #readString
    |  'print'
        #print
    |  'prints'
        #printString
    |  'add'
        #add
    |  'sub'
        #sub
    |  'mul'
        #mult
    |  'div'
        #div
    |  'and'
        #and
    |  'or'
        #or
    |  'not'
        #not
    |  'nullp'
        #nullPredicate
    |  'band'
        #bitwiseAnd
    |  'bor'
        #bitwiseOr
    |  'bnot'
        #bitwiseNot
    |  'jump'
        #jump
    |  'jump' target=ref_label
        #jumpImediate
    |  'bz'
        #branchZero
    |  'bz' target=ref_label
        #branchZeroImediate
    |  'bnz'
        #branchNotZero
    |  'bnz' target=ref_label
        #branchNotZeroImediate
    |  'bneg'
        #branchNegative
    |  'bneg' target=ref_label
        #branchNegativeImediate
    |  'bpos'
        #branchPositive
    |  'bpos' target=ref_label
        #branchPositiveImediate
    |  'push' 'pc'
        #pushPC
    |  'cast_c' #castAsChar
    |  'cast_i' #castAsInt
    |  'cast_d' #castAsDouble
    |  'cast_s' #castAsStr
    |  'exit'
        #exit
    ;

label
    : LITERAL_LABEL
    ;

ref_label
    : LITERAL_LABEL
    ;

intarg
    : LITERAL_INT | LITERAL_HEX
    ;

non_negative_int_arg
    : intarg
    ;

chararg
    : LITERAL_CHAR
    ;

floatarg
    : LITERAL_FLOAT
    ;

doublearg
    : LITERAL_FLOAT
    ;

strarg : LITERAL_STRING ;

// Tokens


NEWLINE : '\r'? '\n'; // dos or windows newlines

SKIP_: (COMMENT | WS) -> skip;

LITERAL_STRING
    :   '"'
        ( ~('"' | [\n\r] ) | '\\' [a-z] )*
        '"'
    ;

LITERAL_LABEL: '_'*[a-zA-Z][a-zA-Z0-9_]*;

LITERAL_CHAR
    :   '\''
        ( '\\' [a-z] | ~('\'') ) // Um escape (\a, \n, \t) ou qualquer coisa que não seja aspas (a, b, ., z, ...)
        '\''
    ;

LITERAL_FLOAT
    :  '-'?
        DEC_DIGIT+
        '.' DEC_DIGIT+([eE][+-]? DEC_DIGIT+)?
    ;

LITERAL_INT
    :  '-'?
        DEC_DIGIT+
    ;

LITERAL_HEX
    :   '0x'
        HEX_DIGIT+
    ;

fragment
HEX_DIGIT : [0-9A-Fa-f];

fragment
DEC_DIGIT : [0-9] ;

fragment
COMMENT : '#' ~[\n\r\f]* ;

fragment
WS : [ \t]+ ;
