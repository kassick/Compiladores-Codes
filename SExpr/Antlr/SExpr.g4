grammar SExpr;
@header
{
#include <iostream>
#include <string>

}

@parser::members {
                 int indent = 0;
                 std::ostream * out;
/*-----------------------------------------------------------------------------
 * Function: print_indent
 *
 * Description: Imprime em algum tipo stream a mensagem msg com o nível de
 *              indent especificado.
 *---------------------------------------------------------------------------*/
std::ostream& print_indent(int indent, const std::string msg)
{
    for (int i = 0; i < indent; i++)
        *out << "    ";
    *out << msg << std::endl;

    return *out;
}
                 }

sexpr : '(' n=NAME
         {
            print_indent(indent, std::string("( ") + $n.text);
            indent++;
         }
         slist ')'
         {
            indent--;
            print_indent(indent, std::string(")"));
         }
         ;

slist : NAME
        {
            print_indent(indent, $NAME.text);
        }
        slist
      | sexpr slist
      |
      ;

WS : (' ' | '\t' | '\n')+ -> channel(HIDDEN);
NAME : [a-z]+ ;