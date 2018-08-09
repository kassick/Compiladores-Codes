# S-Expression parsers

This is a collection of S-Expression parsers written in C++. They were developed for the Compilers course at Uniritter (2016 and 2017) and serve to demonstrate some parsing techniques as presented in class.

Available versions:

- [Naive](./SExpr/Naive/sparser_naive.cpp): One naive implementation. It's behaviour is similar to a LL(1) parser, as it peeks the input to see what kind of input it should see next.

- [Recursive Descendent LL(1)](./SExpr/LL1/sparser_ll1.cpp): This parser uses the recursive descent approach with 1 character look-ahead. It's a hand-coded parser, doesn't use a parsing table.

- [Naive with Tokens](./SExpr/Naive-Tokens/sparser_naive_tokens.cpp): A naive implementation using antlr as a tokenizer. This code showcases a first use of antlr for code generation.

- [ANTLR ALL(*) with Action](./SExpr/Antlr/sparser_antlr.cpp): An implementation with antlr as a parser. Actions are embedded in the grammar rules.

- [ANTLR ALL(*) with a Listener](./SExpr/AntlrListener/sparser_antlr_listener.cpp): An implementation with antlr as a parser. No actions are embedded inside the grammar. This code showcases the Listener pattern.

- [ANTLR ALL(*) with a Visitor](./SExpr/AntlrVisitor/sparser_antlr_visitor.cpp): An implementation with antlr as a parser. No actions are embedded inside the grammar. This code showcases the Visitor pattern.

- [ANTLR ALL(*) with Rule Argument Attributes](./SExpr/Antlr-Attributes/sparser_antlr_attrs.cpp): An implementation with antlr as a parser. It uses rule attributes to avoid using "global" variables such as `indent`
