grammar SExprL;

ignore: 'ignore';

TOK_OPEN: '(' ;
TOK_CLOSE: ')';
TOK_NAME: [a-zA-Z]+;
WS: (' ' | '\t' | '\n')+ -> skip; //channel(HIDDEN);
