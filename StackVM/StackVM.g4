grammar StackVM;

start
    :   instruction_line+
        EOF
    ;

instruction_line
    :
        WS*
        (label WS* ':' )?
        WS*
        instruction
        WS*
        NEWLINE
        #instruction
    ;

instruction
    :   'mark'
        #mark
    |   'pop_mark'
        #popMark
    |   'drop_mark'
        #dropMark
    |   'crunch'
        #crunchNoArgs
    |   'crunch' size=intarg
        #crunchBaseIndirect
    |   'crunch' base=intarg size=intarg
        #crunchDirect
    |   'trim'
        #trimIndirect
    |   'trim' base=intarg
        #trimDirect
    |   'swap'
        #swapTopWithNext
    |   'swap' a=intarg
        #swapTopWithArg
    |   'swap' a=intarg b=intarg
        #swap
    |  'dup'
        #dupTop
    |  'acreate'
        #arrayCreate
    |  'acreate' l=intarg
        #arrayCreateArg
    |  'aload'
        #arrayLoad
    |  'alen'
        #arrayLen
    |  'aget'
        #arrayGet
    |  'aget' i=intarg
        #arrayGetArg
    |  'aset'
        #arraySet
    |  'aset' i=intarg
        #arraySetArg
    |  'push' c=chararg
        #pushChar
    |  'puch' i=intarg
        #pushInt
    |  'pushf' f=floatarg
        #pushFloat
    |  'pushd' d=doublearg
        #pushDouble
    |  'push' s=strarg
        #pushString
    |  'push' l=label
        #pushLabel
    |  'push' 'null'
        #pushNull
    |  'push' 'stack_size'
        #pushStackSize
    |  'pop'
        #pop
    |  'store'
        #storeIndirect
    |  'store' i=intarg
        #store
    |  'load'
        #loadIndirect
    |  'load' i=intarg
        #loadIndirect
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
        #pringString
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
    |  'jump' target=label
        #jumpImediate
    |  'bz'
        #branchZero
    |  'bz' target=label
        #branchZeroImediate
    |  'bnz'
        #branchNotZero
    |  'bnz' target=label
        #branchNotZeroImediate
    |  'bneg'
        #branchNegative
    |  'bneg' target=label
        #branchNegativeImediate
    |  'bpos'
        #branchPositive
    |  'bpos' target=label
        #branchPositiveImediate
    |  'push' 'pc'
        #pushPC
    ;

label
    : [a-zA-Z][a-zA-Z0-9]*
    ;

intarg
    : LITERAL_INT | LITERAL_HEX
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

WS : [ \r\t\u000C\n];

NEWLINE : '\r'? '\n'; // dos or windows newlines

COMMENT : '#' ~[\r\n]* '\r'? '\n' -> skip ;

LITERAL_STRING
    :   '"'
        ( ~('"' | [\n\r] ) | '\\' [a-z] )*
        '"'
    ;

LITERAL_CHAR
    :   '\''
        ( '\\' [a-z] | ~('\'') ) // Um escape (\a, \n, \t) ou qualquer coisa que não seja aspas (a, b, ., z, ...)
        '\'' ;
    : '"' (  )
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
