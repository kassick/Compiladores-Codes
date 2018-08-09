/****************************************************************************
 *        Filename: "sparser_ll1.cpp"
 *
 *     Description: Parser para a seguinte gramática de S-expressões
 *
 *                  S -> ( n Slist ) ;
 *
 *                  Slist -> n Slist
 *                        |  S Slist
 *                        |  epsilon
 *                        ;
 *
 *         Version: 1.0
 *         Created: "Mon Jul 31 19:37:20 2017"
 *         Updated: "2017-08-01 09:37:14 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

/*-----------------------------------------------------------------------------
 * Function: print_indent
 *
 * Description: Imprime em algum tipo stream a mensagem msg com o nível de
 *              indent especificado.
 *---------------------------------------------------------------------------*/
template <class Stream>
Stream& print_indent(Stream & out, int indent, const string msg)
{
    for (int i = 0; i < indent; i++)
        out << "    ";
    out << msg << endl;

    return out;
}

/*-----------------------------------------------------------------------------
 * Function: consome_espacos
 *
 * Description: Tira espaços ou quebras de linha de uma entrada in
 *---------------------------------------------------------------------------*/
template <class Stream>
Stream& consome_espacos(Stream & in)
{
    while (1){
        if (in.eof())
            break;

        char c = in.peek();
        if (c == ' ' || c == '\n')
            in.get();
        else
            break;
    }

    return in;
}

// Nível de indent atual (global)
int indent;

void rule_S(istream& in, ostream& out);
void rule_Slist(istream& in, ostream& out);
string tokenName(istream& in);

/*-----------------------------------------------------------------------------
 * Function: tokenName
 *
 * Description: Retorna um token nome ( [a-zA-Z]+ ) da entrada in ou encerra o
 *              programa com erro
 *---------------------------------------------------------------------------*/
string tokenName(istream& in)
{
    string n;
    char c;
    char next_c;

    consome_espacos(in);

    // Acumula letras a-zA-Z em n
    while(!in.eof()) {
        next_c = in.peek();
        if ('A' <= toupper(next_c) && toupper(next_c) <= 'Z') {
            n += next_c;
            in.get();
        } else
            break;
    }

    if (n.length() == 0) {
        throw string("Parse Error: esperava um nome, não encontrou nada");
    }

    return n;
}

/*-----------------------------------------------------------------------------
 * Function: rule_Slist
 *
 * Description: Regra Slist:
                Slist -> n Slist
                       | S Slist
                       | epsilon
                       ;
 *---------------------------------------------------------------------------*/
void rule_Slist(istream& in, ostream& out)
{
    consome_espacos(in);

    // Slist nunca vai ser seguida de fim de arquivo
    if (in.eof()) {
        throw string("Parse Error: esperava um nome ou um abre ou um fecha, encontrou fim de arquivo");
    }

    string n;
    char c = in.peek();

    switch(toupper(c))
    {
        case '(':
            // S Slist

            rule_S(in, out);
            rule_Slist(in, out);

            break;

        case 'A' ... 'Z':
            // n Slist

            n = tokenName(in);
            print_indent(out, indent, n);

            rule_Slist(in, out);

            break;

        case ')':
            // epsilon

            return;

        default:
            throw string("Parse Error: esperava um nome, um abre ou um fecha, encontrou um ``") +
                    c + "´´";
    }
}

/*-----------------------------------------------------------------------------
 * Function: rule_S
 *
 * Description: Regra S
                S -> ( n Slist )
 *---------------------------------------------------------------------------*/
void rule_S(istream& in, ostream& out)
{
    consome_espacos(in);

    char c;
    string n;

    c = in.peek();

    switch(toupper(c)) {
        case '(':                       // ( n Slist )
            c = in.get();               // OPEN

            n = tokenName(in);          // n

            // Antes de começar a parsear
            // a lista de parâmetros, imprime
            // o abre e o nome
            print_indent(out, indent, "( " + n);

            indent++;

            rule_Slist(in, out);             // Slist

            consome_espacos(in);

            if (in.eof()) {
                throw string("Parse Error: esperava fecha ou outra palavra, encontrou fim de arquivo");
            }

            c = in.get();               // CLOSE
            if (c != ')') {
                throw string("Parse Error: esperava um fecha parênteses, encontrou ``") +
                      c + "´´";
            }

            indent--;

            print_indent(out, indent, ")");

            break;

        default:
            throw string("Parse Error: esperava um abre parênteses");
    }

}

string parsestring(string sin)
{
    indent = 0;
    std::stringstream ss(sin), sout;
    try {
        rule_S(ss, sout);
    } catch (const std::string & s) {
        return s;
    }

    consome_espacos(ss);

    if (!ss.eof())
        return "Parênteses desbalanceados";

    return sout.str();
}

extern "C" {
    const char * parse_string_c(const char *s)
    {
        return parsestring(string(s)).c_str();
    }
}

/*-----------------------------------------------------------------------------
 * Function: main
 *
 * Description: lê da entrada ou de um arquivo e tenta interpretar uma regra S
 *---------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    istream *in = &cin;
    fstream fin;

    indent = 0;

    if (argc >= 2) {
        fin.open(argv[1], ios_base::in);
        if (!fin.is_open())
            return -1;

        in = &fin;
    }

    try {
        rule_S(*in, cout);
    } catch (const std::string & s) {
        cerr << s << endl;
    }

    return 0;
}
