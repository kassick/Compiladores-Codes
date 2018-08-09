using System;
using Teste;
using Antlr4.Runtime;

namespace SExpr
{
    class MainClass
    {
        public static void Main (string[] args)
        {
            AntlrInputStream ain = new AntlrInputStream ("(a b cd ef (ged))");
            SExprLexer lexer = new SExprLexer(ain);
            BufferedTokenStream tokens = new BufferedTokenStream (lexer);
            tokens.Fill ();
            SExprParser parser = new SExprParser (tokens);
            Console.WriteLine (parser.main ());
        }
    }
}
