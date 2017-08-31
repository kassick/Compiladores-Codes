grammar SExpr;
@header
{
#include <iostream>
#include <string>

    using namespace std;
}

sexpr : '(' n=NAME
         {
            cerr << "A" << endl;
         }
         slist ')'
         {
            cerr << "B" << endl;
         }
         ;

slist : NAME
        {
            cerr << "C" << endl;
        }
        childList=slist
        #slistName
      | sexpr { cerr << "D" << endl;} childList=slist { cerr << "E" << endl; }
        #slistSexpr
      |
        #slistEmpty
      ;

WS : (' ' | '\t' | '\n')+ -> channel(HIDDEN);
NAME : [a-z]+ ;
