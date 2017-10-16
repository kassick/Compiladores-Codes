grammar SExpr;
@header
{
#include <iostream>
#include <string>

}

@parser::members {
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

sexpr[int indent]
returns [int max_level]
@init
{
    $max_level = $indent;
}
    :  '(' n=NAME
        {
            print_indent($indent, std::string("( ") + $n.text);
        }
        slist[$indent + 1]
        ')'
        {
            print_indent($indent, std::string(")"));
            if ($slist.max_level > $max_level)
                $max_level = $slist.max_level;
        }
    ;

slist[int indent]
returns [int max_level]
@init
{
    $max_level = $indent;
}
    :   NAME
        {
            print_indent($indent, $NAME.text);
        }
        slist[$indent]
        {
            if ($slist.max_level > $max_level)
                $max_level = $slist.max_level;
        }
    |   sexpr[$indent]
        {
            if ($sexpr.max_level > $max_level)
                $max_level = $sexpr.max_level;
        }
        slist[$indent]
        {
            if ($slist.max_level > $max_level)
                $max_level = $slist.max_level;
        }
    |
    ;

WS : (' ' | '\t' | '\n')+ -> channel(HIDDEN);
NAME : [a-z]+ ;
