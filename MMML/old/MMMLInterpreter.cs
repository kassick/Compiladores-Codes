
namespace mimimil
{
    using System;
    using System.IO;
    using Antlr4.Runtime;
    using System.Linq;
    // class MMMLTreeVisitor: MMMLBaseVisitor<object>
    // {
    //     private NestedSymbolTable symbols;


    //     public MMMLTreeVisitor() {
    //         symbols = new NestedSymbolTable();
    //     }


    //     public override object VisitLetvarname([NotNull] MMMLParser.LetvarnameContext context)
    //     {
    //         var name = context.GetChild(0).GetText();
    //         Console.WriteLine("Encontrou variavel {0}", name);

    //         return VisitChildren(context);
    //     }

    //     public override object VisitLetexpression([NotNull] MMMLParser.LetexpressionContext context)
    //     {
    //         Console.WriteLine("Novo contexto de Variaveis");
    //         symbols = new NestedSymbolTable(symbols);
    //         foreach (var child in context.children){
    //             Console.WriteLine("Child: " + child.Payload.GetType().ToString());
    //         }

    //         Console.WriteLine("Terminou");
    //         symbols = symbols.Parent;

    //         return VisitChildren(context);
    //     }

    // }


    public class MMMLInterpreter
    {
        public MMMLInterpreter(string filename)
        {
            var sr = new StreamReader(filename);
            var ais = new AntlrInputStream(sr.ReadToEnd());
            var lexer = new MMMLLexer(ais);
            var cts = new CommonTokenStream(lexer);
            var parser = new MMMLParser(cts);
            var prog = parser.program();
            //var visitor = new MMMLTreeVisitor();
            //Console.WriteLine("Olha Soh! {0}", visitor.Visit(prog));
            if (parser.nerrors > 0) {
                Console.WriteLine("\nCompilation Failed ({0} errors)", parser.nerrors);
                return;
            }

            foreach (SymbolEntry<FunctionEntry> entry in parser.functionTable) {
                var plist =
                    from p in entry.symbol.plist
                    select p.name + ": " + p.type.ToString();

                var plist_str = String.Join(", ", plist);

                Console.WriteLine("Function {0}: ({1}) -> {2}",
                                  entry.symbol.name,
                                  plist_str,
                                  entry.symbol.retType);
            }

            //foreach (var entry in parser.functionTable.st)
            Console.WriteLine("The program from {0} is {1}", filename, prog.ToStringTree(parser));
        }
    }
}
