grammar SExpr;
@header
{
#include <iostream>
#include <string>

    using namespace std;
}

sexpr : '(' n=NAME
         slist ')'
         ;

slist : NAME
        childList=slist
        #slistName
      | sexpr childList=slist
        #slistSexpr
      |
        #slistEmpty
      ;

WS : (' ' | '\t' | '\n')+ -> channel(HIDDEN);
NAME : [a-z]+ ;
