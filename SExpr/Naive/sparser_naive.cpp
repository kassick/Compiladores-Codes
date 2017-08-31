/****************************************************************************
 *        Filename: "sparser.cpp"
 *
 *     Description: Parser "naive" para a gramática de S-expressões
 *
 *                  S -> ( n Slist ) ;
 *
 *                  Slist -> n Slist | S Slist | epsilon ;
 *
 *                  O parser percorre uma entrada num laço e gerencia o indent e
 *                  nopen conforme ele encontra abre/fecha parênteses
 *
 *         Version: 1.0
 *         Created: "Mon Jul 31 19:37:20 2017"
 *         Updated: "2017-08-01 10:46:44 kassick"
 *
 *          Author: Rodrigo Kassick
 *
 *                    Copyright (C) 2017, Rodrigo Kassick
 ****************************************************************************/

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

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
        cerr << "Parse Error: "
             << "esperava um nome, não encontrou nada"
             << endl;
        exit(1);
    }

    return n;
}

void parse_stream(istream& in, ostream& out)
{
    char c;
    string n;
    int indent = 0;
    int nopen(0);

    // Tem que começar com abre parênteses, não pode ser vazio

    if (consome_espacos(in).eof()) {
        out << "Encontrou fim de arquivo antes do esperado" << endl;
        return;
    }

    if (in.peek() != '(') {
        out << "Esperava um abre parênteses no início da entrada"
             << endl;

        return;
    }

    // Pega o próximo em c, verifica eof
    while (!in.eof())
    {
        c = in.peek();

        if (c == '(') {

            in.get();

            nopen++;

            n = tokenName(in);

            print_indent(out, indent, "( " + n);

            indent++;

        } else if (c == ')') {
            in.get();
            nopen--;
            if (nopen < 0) {
                out << "Erro! Parenteses desbalanceados" << endl;
                return;
            }

            indent--;

            print_indent(out, indent, ")");

        } else if ('A' <= toupper(c) && toupper(c) <= 'Z') {
            n = tokenName(in);
            print_indent(out, indent, n);
        }

        consome_espacos(in);
    }

    if (nopen != 0)
    {
        out << "Erro! Parenteses desbalanceados" << endl;
        return;
    }
}

std::string parsestring(const std::string in )
{
    stringstream sin(in);
    stringstream sout;

    parse_stream(sin, sout);
    return sout.str();
}

extern "C" {
    const char * parse_string_c(const char *s)
    {
        return strdup(parsestring(s).c_str());
    }
}

int main(int argc, char *argv[])
{
    istream *in = &cin;
    fstream fin;

    if (argc >= 2) {
        fin.open(argv[1], ios_base::in);
        if (!fin.is_open())
            return -1;

        in = &fin;
    }

    parse_stream(*in, cout);

    return 0;
}
