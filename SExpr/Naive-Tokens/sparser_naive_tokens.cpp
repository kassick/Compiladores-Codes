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
#include "SExprLLexer.h"

using namespace std;
using namespace sexpr_parser;
using namespace antlr4;

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

void parse_stream(istream& in, ostream& out)
{
    char c;
    string n;
    int indent = 0;
    int nopen(0);

    ANTLRInputStream ain(in);
    SExprLLexer lexer(&ain);

    // for (const auto ch: lexer.getChannelNames())
    //         out << "Canal " << ch;

    auto token = lexer.nextToken();
    while(token)
    {
        if (token->getChannel() == SExprLLexer::HIDDEN) {
            token = lexer.nextToken();
            continue;
        }

        // cerr << "Token " << token->getType() << " : ``" << token->getText() << "´´"
        //      << " no canal " << token->getChannel()
        //      << endl;

        switch(token->getType()) {
            case SExprLLexer::TOK_OPEN:
                {
                    nopen++;

                    auto token_name = lexer.nextToken();
                    while (token_name && token_name->getChannel() == SExprLLexer::HIDDEN)
                        token_name = lexer.nextToken();

                    if (!token_name || token_name->getType() == SExprLLexer::EOF) {
                        out << "ERRO: Esperava um nome, encontrou EOF" << endl;
                        return;
                    } else if (token_name->getType() != SExprLLexer::TOK_NAME) {
                        out << "ERRO: Esperava um nome, encontrou outra coisa na linha "
                            << token_name->getLine()
                            << ", coluna " << token_name->getCharPositionInLine()
                            << endl;
                        return;
                    }

                    print_indent(out, indent, "( " + token_name->getText());

                    indent++;
                }
                break;
            case SExprLLexer::TOK_CLOSE:
                nopen--;
                if (nopen < 0) {
                    out << "Erro! Parenteses desbalanceados" << endl;
                    return;
                }

                indent--;

                print_indent(out, indent, ")");
                break;

            case SExprLLexer::TOK_NAME:
                print_indent(out, indent, token->getText());
                break;

            case SExprLLexer::EOF:
                if (nopen != 0)
                {
                    out << "Erro! Parenteses desbalanceados" << endl;
                }

                return;

            default:
                out << "ERRO: "
                    << "Esperava um abre, um fecha ou um nome, encontrou um "
                    << token->getType()
                    << endl;
                return;
        }

        token = lexer.nextToken();
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
