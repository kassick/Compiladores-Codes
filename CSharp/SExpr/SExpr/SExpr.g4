grammar SExpr ;

@header { using System ; }

main: sexpr EOF ;

sexpr
	@before { Console.WriteLine("SExpr: "); }
	: { Console.WriteLine("Regra 1"); }
	  LPAREN NAME slist RPAREN
	  { Console.WriteLine("Fim Regra 2"); }
	;

slist
	@before { Console.WriteLine("SList: "); }
	: { Console.WriteLine("Regra 1: "); }
	  NAME slist 
	| { Console.WriteLine("Regra 2: "); }
	  sexpr slist
	| { Console.WriteLine("Vazio"); }
	;

NAME : [a-zA-Z] ;
LPAREN : '(' ;
RPAREN : ')' ;
WS : (' ' | '\t' | '\n')+ -> skip;
