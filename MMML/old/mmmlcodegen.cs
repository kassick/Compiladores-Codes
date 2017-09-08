namespace mimimil
{
    using System;
    using System.Linq;
    using System.Collections.Generic;

    // NotNull attribute
    using Antlr4.Runtime.Misc;
    using CodeNode = Code.Node;

    namespace Code {
        public class Utils {
            private static int nextLabel = 0;

            public static string NextL() {
                string r = "L"+ nextLabel.ToString();
                nextLabel++;
                return r;
            }

            public static string typeHeader(Type t)
            {
                switch (t.btype) {
                    case BasicType.BOOL: return "z";
                    case BasicType.INT: return "i";
                    case BasicType.FLOAT: return "d";
                    case BasicType.STR: return "a";
                    default:
                        throw new Exception("Error in code gen: Unknown type " + t);
                }
            }
        }

        public class Node {
            //public readonly string label;
            public virtual string Code {
                get { return ""; }
            }

            public Node Chain(Node other)
            {
                return new ChainedNode(this, other);
            }

            public Node Label(string lbl) {
                return new ChainedNode(this, new LabelNode(lbl));
            }

            public Node ChainStr(string literalCode) {
                return new ChainedNode(this, new StringNode(literalCode));
            }
        }

        public class StringNode  : Node{
            public string code;

            public StringNode(string code) {
                this.code = code;
            }

            public override string Code {
                get {
                    return "\t " + this.code + "\n";
                }
            }
        }

        public class LabelNode  : Node{
            public string label;

            public LabelNode(string label) {
                this.label = label;
            }

            public override string Code {
                get {
                    return this.label + ":" + "\n";
                }
            }
        }

        public class ChainedNode : Node {
            public Node first, second;

            public ChainedNode(Node first, Node second) {
                this.first = first;
                this.second  = second;
            }

            public override string Code {
                get {
                    return
                        ( first != null ? first.Code + "\n" : "" ) +
                        (second != null ? second.Code + "\n" : "");
                }
            }
        }

        public class IfNode : Node {
            public Node
                cond,
                trueBranch,
                falseBranch;

            public override string Code {
                get {
                    var ltrue = Utils.NextL();
                    var lfalse = Utils.NextL();
                    var lend = Utils.NextL();

                    // Stack cond eval has <nullguard> <value>
                    // if <nullguard> == 0, no value, false
                    // pops <value>
                    // continue

                    return
                        cond
                        .ChainStr("ifeq " + lfalse)
                        .ChainStr("pop")
                        .Label(ltrue)
                        .Chain(trueBranch)
                        .ChainStr("goto " + lend)
                        .Label(lfalse)
                        .Chain(falseBranch)
                        .Label(lend)
                        .Code;
                }
            }
        }

        public class LoadSym : Node {
            public int stPos;
            public Type type;

            public override string Code {
                get {
                    int valuePos = 2 * stPos;
                    int nullGuardPos = 2 * stPos + 1;

                    if (type.IsSequence)
                        return "NOT IMPLEMENTED";

                    string typeHeader = Utils.typeHeader(type);

                    // Loads into stack the value and the guard
                    // The guard remains on the top

                    return (new StringNode(typeHeader + "load " + valuePos))
                        .ChainStr("iload " + nullGuardPos)
                        .Code;
                }
            }
        }

        public class StoreSym : Node {
            public int stPos;
            public Type type;

            public override string Code {
                get {
                    int valuePos = 2 * stPos;
                    int nullGuardPos = 2 * stPos + 1;

                    if (type.IsSequence)
                        return "NOT IMPLEMENTED";

                    string typeHeader = Utils.typeHeader(type);

                    // Stack contains guard on top and value below
                    // Stores first the guard and then the value
                    return
                        (new StringNode("istore " + nullGuardPos))
                        .ChainStr(typeHeader + "store " + valuePos)
                        .Code;
                }
            }
        }

        public class LoadConst : Node {
            public Type type;
            public bool nil;
            public string value;

            private static string defaultTypeValue(BasicType t) {
                switch (t) {
                    case BasicType.INT: return "0";
                    case BasicType.FLOAT: return "0.0";
                    case BasicType.STR: return "\"\"";
                    case BasicType.BOOL: return "0";
                    case BasicType.NIL: return "0";
                    default:
                        throw new Exception("Invalid type " + t.ToString());
                }
            }

            public override string Code {
                get {
                    string lc = "";
                    string v = this.value;

                    if (type.IsNil)
                        nil = true;

                    if (nil)
                        v = defaultTypeValue(type.btype);

                    switch (type.btype) {
                        case BasicType.INT:
                            lc = "sipush "; break;
                        case BasicType.FLOAT:
                            lc = "ldc2_w "; break;
                        case BasicType.BOOL:
                            lc = "iconst_";
                            v = "0";
                            break;
                        case BasicType.STR:
                            lc = "ldc "; break;

                        default:
                            if (type.IsSequence) {
                                throw new Exception ("Sequences not implemented");
                            }
                            lc = ""; break;
                    }

                    // sipush 10     (literal 10)
                    // iconst_0      (null guard == 0)
                    return
                        (new StringNode(lc + v))
                        .ChainStr("iconst_" + (nil ? 0 : 1))
                        .Code;
                }
            }
        }

        public class Cast : Node {
            public Type src, dst;

            private Node parseStringTo(Type dst)
            {
                // TODO
                return new StringNode( "" );
            }

            private Node convertToString(Type src)
            {
                // TODO
                return new StringNode( "" );
            }

            public override string Code {
                get {
                    // anything -> bool
                    if (dst.btype == BasicType.BOOL) {
                        // Cast something to bool ,
                        // keep the guard, substitute the value for 0
                        //                                    <guard> <value>
                        return
                            "\t swap\n"     +              // <value> <guard>
                            "\t pop\n"      +              // <guard>
                            "\t iconst_0\n" +              // 0 <guard>
                            "\t swap\n"     ;              // <guard> 0
                    }

                    Node castNode = null;
                    if (src.btype == BasicType.STR) {
                        // parse from string
                        castNode = parseStringTo(dst);
                    } else if (dst.btype == BasicType.STR) {
                        //  convertToString
                        castNode = convertToString(src);
                    } else {
                        // i2d , d2i , i2c, c2i
                        // Numerical casts
                        var sh = Utils.typeHeader(src);
                        var dh = Utils.typeHeader(dst);

                        castNode =
                            new StringNode(sh + "2" + dh)
                            .ChainStr("iconst_1");
                    }

                    var lNotNil = Utils.NextL();
                    var lNil = Utils.NextL();
                    var lEnd = Utils.NextL();

                    // Stack cond eval has <nullguard> <value>
                    // if <nullguard> == 0, no value.
                    // We must just load the default value for the destination type
                    // otherwise, we must execute the cast

                    // (float nil)
                    // <guard> <value>
                    //  0        <i>          ifeq lfalse
                    // <i>                    pop
                    //                        push <default>
                    //           0.0          push 0 <is nil>
                    // 0         0.0          goto lfalse

                    // (float  not-nil)
                    //
                    //           <i>     lfalse:
                    //                        i2d
                    //           <f>          push 1 <not nil>
                    // 1         <f>

                    return
                        new StringNode("ifeq " + lNil)
                        .Label(lNotNil)
                        .Chain(castNode)
                        .ChainStr("goto " + lEnd )
                        .Label(lNil)
                        .ChainStr("pop")
                        .Chain(new LoadConst { type = dst, nil = true, value = "" })
                        .Label(lEnd)
                        .Code;
                }
            }
        }

        public class Call : Node
        {
            public FunctionEntry fe;
            public List<Node> paramsCode;

            public override string Code {
                get {
                    var pcode_str_list =
                        from p in paramsCode
                        select p.Code;
                    var params_code = String.Join("\n", pcode_str_list);
                    return params_code + "\n" +
                        "\t CALL " + fe.name + "\n";
                }
            }
        }

        public class SumOp : Node
        {
            Type typeLeft, typeRight, typedst;
            Node codeLeft, codeRight;

            public override string Code {
                get {
                    /*
                    Node castleft, castRight;
                    if (typedst.btype == BasicType.FLOAT) {
                        if (typeLeft.btype != BasicType.FLOAT)
                            return "";
                    } */


                    return "";
                }
            }

        }
    }


    public class CodeGenVisitor : MMMLBaseVisitor<CodeNode>
    {
	/// <summary>
	/// Visit a parse tree produced by the <c>symbol_rule</c>
	/// labeled alternative in <see cref="MMMLParser.symbol"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{Result}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSymbol_rule([NotNull] MMMLParser.Symbol_ruleContext context) {
            //var ifn = new Code.IfNode {cond = null , trueBranch = null, falseBranch = null};
            return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>funcall_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcall"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncall_rule([NotNull] MMMLParser.Funcall_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>basictype_rule</c>
	/// labeled alternative in <see cref="MMMLParser.type"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitBasictype_rule([NotNull] MMMLParser.Basictype_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>sequencetype_rule</c>
	/// labeled alternative in <see cref="MMMLParser.type"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSequencetype_rule([NotNull] MMMLParser.Sequencetype_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>letlist_cont_end</c>
	/// labeled alternative in <see cref="MMMLParser.letlist_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetlist_cont_end([NotNull] MMMLParser.Letlist_cont_endContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>letlist_cont_rule</c>
	/// labeled alternative in <see cref="MMMLParser.letlist_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetlist_cont_rule([NotNull] MMMLParser.Letlist_cont_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>literal_char_rule</c>
	/// labeled alternative in <see cref="MMMLParser.literal"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLiteral_char_rule([NotNull] MMMLParser.Literal_char_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>literalnumber_rule</c>
	/// labeled alternative in <see cref="MMMLParser.literal"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLiteralnumber_rule([NotNull] MMMLParser.Literalnumber_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>literaltrueorfalse_rule</c>
	/// labeled alternative in <see cref="MMMLParser.literal"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLiteraltrueorfalse_rule([NotNull] MMMLParser.Literaltrueorfalse_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>literalnil_rule</c>
	/// labeled alternative in <see cref="MMMLParser.literal"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLiteralnil_rule([NotNull] MMMLParser.Literalnil_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>literalstring_rule</c>
	/// labeled alternative in <see cref="MMMLParser.literal"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLiteralstring_rule([NotNull] MMMLParser.Literalstring_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>cast_rule</c>
	/// labeled alternative in <see cref="MMMLParser.cast"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitCast_rule([NotNull] MMMLParser.Cast_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>funcall_params_cont_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcall_params_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncall_params_cont_rule([NotNull] MMMLParser.Funcall_params_cont_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>funcall_params_end_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcall_params_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncall_params_end_rule([NotNull] MMMLParser.Funcall_params_end_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>numberf loat_rule</c>
	/// labeled alternative in <see cref="MMMLParser.number"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitNumberfloat_rule([NotNull] MMMLParser.Numberfloat_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>numberdecimal_rule</c>
	/// labeled alternative in <see cref="MMMLParser.number"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitNumberdecimal_rule([NotNull] MMMLParser.Numberdecimal_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>numberhexadecimal_rule</c>
	/// labeled alternative in <see cref="MMMLParser.number"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitNumberhexadecimal_rule([NotNull] MMMLParser.Numberhexadecimal_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>numberbinary_rule</c>
	/// labeled alternative in <see cref="MMMLParser.number"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitNumberbinary_rule([NotNull] MMMLParser.Numberbinary_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprplusminus_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprplusminus_rule([NotNull] MMMLParser.Me_exprplusminus_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_boolneg_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_boolneg_rule([NotNull] MMMLParser.Me_boolneg_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprliteral_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprliteral_rule([NotNull] MMMLParser.Me_exprliteral_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_listconcat_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_listconcat_rule([NotNull] MMMLParser.Me_listconcat_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_list_create_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_list_create_rule([NotNull] MMMLParser.Me_list_create_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprmuldiv_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprmuldiv_rule([NotNull] MMMLParser.Me_exprmuldiv_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprcast_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprcast_rule([NotNull] MMMLParser.Me_exprcast_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprparens_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprparens_rule([NotNull] MMMLParser.Me_exprparens_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_boolandor_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_boolandor_rule([NotNull] MMMLParser.Me_boolandor_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_boolgtlt_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_boolgtlt_rule([NotNull] MMMLParser.Me_boolgtlt_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprsymbol_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprsymbol_rule([NotNull] MMMLParser.Me_exprsymbol_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprfuncall_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprfuncall_rule([NotNull] MMMLParser.Me_exprfuncall_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_boolnegparens_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_boolnegparens_rule([NotNull] MMMLParser.Me_boolnegparens_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_exprpower_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_exprpower_rule([NotNull] MMMLParser.Me_exprpower_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>me_booleqdiff_rule</c>
	/// labeled alternative in <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMe_booleqdiff_rule([NotNull] MMMLParser.Me_booleqdiff_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdecl_funcname_rule</c>
	/// labeled alternative in <see cref="MMMLParser.functionname"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdecl_funcname_rule([NotNull] MMMLParser.Fdecl_funcname_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>funcdef_rule</c>
	/// labeled alternative in <see cref="MMMLParser.fdecl"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncdef_rule([NotNull] MMMLParser.Funcdef_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>funcdef_definition</c>
	/// labeled alternative in <see cref="MMMLParser.fdecl"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncdef_definition([NotNull] MMMLParser.Funcdef_definitionContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdecls_end_rule</c>
	/// labeled alternative in <see cref="MMMLParser.fdecls"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdecls_end_rule([NotNull] MMMLParser.Fdecls_end_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdecls_one_decl_rule</c>
	/// labeled alternative in <see cref="MMMLParser.fdecls"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdecls_one_decl_rule([NotNull] MMMLParser.Fdecls_one_decl_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fbody_let_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcbody"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFbody_let_rule([NotNull] MMMLParser.Fbody_let_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fbody_if_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcbody"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFbody_if_rule([NotNull] MMMLParser.Fbody_if_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fbody_expr_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcbody"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFbody_expr_rule([NotNull] MMMLParser.Fbody_expr_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>programmain_rule</c>
	/// labeled alternative in <see cref="MMMLParser.maindecl"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitProgrammain_rule([NotNull] MMMLParser.Programmain_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>se_create_seq</c>
	/// labeled alternative in <see cref="MMMLParser.sequence_expr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSe_create_seq([NotNull] MMMLParser.Se_create_seqContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>sequencetype_sequence_rule</c>
	/// labeled alternative in <see cref="MMMLParser.sequence_type"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSequencetype_sequence_rule([NotNull] MMMLParser.Sequencetype_sequence_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>sequencetype_basetype_rule</c>
	/// labeled alternative in <see cref="MMMLParser.sequence_type"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSequencetype_basetype_rule([NotNull] MMMLParser.Sequencetype_basetype_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdeclparams_end_rule</c>
	/// labeled alternative in <see cref="MMMLParser.fdeclparams_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdeclparams_end_rule([NotNull] MMMLParser.Fdeclparams_end_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdeclparams_cont_rule</c>
	/// labeled alternative in <see cref="MMMLParser.fdeclparams_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdeclparams_cont_rule([NotNull] MMMLParser.Fdeclparams_cont_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>letlist_rule</c>
	/// labeled alternative in <see cref="MMMLParser.letlist"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetlist_rule([NotNull] MMMLParser.Letlist_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdeclparams_no_params</c>
	/// labeled alternative in <see cref="MMMLParser.fdeclparams"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdeclparams_no_params([NotNull] MMMLParser.Fdeclparams_no_paramsContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdeclparams_one_param_rule</c>
	/// labeled alternative in <see cref="MMMLParser.fdeclparams"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdeclparams_one_param_rule([NotNull] MMMLParser.Fdeclparams_one_param_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>funcallparams_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcall_params"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncallparams_rule([NotNull] MMMLParser.Funcallparams_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>funcallnoparam_rule</c>
	/// labeled alternative in <see cref="MMMLParser.funcall_params"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncallnoparam_rule([NotNull] MMMLParser.Funcallnoparam_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>fdecl_param_rule</c>
	/// labeled alternative in <see cref="MMMLParser.fdeclparam"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdecl_param_rule([NotNull] MMMLParser.Fdecl_param_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>letunpack_rule</c>
	/// labeled alternative in <see cref="MMMLParser.letvarexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetunpack_rule([NotNull] MMMLParser.Letunpack_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>letvarresult_ignore_rule</c>
	/// labeled alternative in <see cref="MMMLParser.letvarexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetvarresult_ignore_rule([NotNull] MMMLParser.Letvarresult_ignore_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by the <c>letvarattr_rule</c>
	/// labeled alternative in <see cref="MMMLParser.letvarexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetvarattr_rule([NotNull] MMMLParser.Letvarattr_ruleContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.program"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitProgram([NotNull] MMMLParser.ProgramContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.fdecls"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdecls([NotNull] MMMLParser.FdeclsContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.maindecl"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMaindecl([NotNull] MMMLParser.MaindeclContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.fdecl"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdecl([NotNull] MMMLParser.FdeclContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.fdeclparams"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdeclparams([NotNull] MMMLParser.FdeclparamsContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.fdeclparams_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdeclparams_cont([NotNull] MMMLParser.Fdeclparams_contContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.fdeclparam"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFdeclparam([NotNull] MMMLParser.FdeclparamContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.functionname"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFunctionname([NotNull] MMMLParser.FunctionnameContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.type"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitType([NotNull] MMMLParser.TypeContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.basic_type"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitBasic_type([NotNull] MMMLParser.Basic_typeContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.sequence_type"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSequence_type([NotNull] MMMLParser.Sequence_typeContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.funcbody"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncbody([NotNull] MMMLParser.FuncbodyContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.letlist"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetlist([NotNull] MMMLParser.LetlistContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.letlist_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetlist_cont([NotNull] MMMLParser.Letlist_contContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.letvarexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLetvarexpr([NotNull] MMMLParser.LetvarexprContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.metaexpr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitMetaexpr([NotNull] MMMLParser.MetaexprContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.sequence_expr"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSequence_expr([NotNull] MMMLParser.Sequence_exprContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.funcall"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncall([NotNull] MMMLParser.FuncallContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.cast"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitCast([NotNull] MMMLParser.CastContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.funcall_params"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncall_params([NotNull] MMMLParser.Funcall_paramsContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.funcall_params_cont"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitFuncall_params_cont([NotNull] MMMLParser.Funcall_params_contContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.literal"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitLiteral([NotNull] MMMLParser.LiteralContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.strlit"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitStrlit([NotNull] MMMLParser.StrlitContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.charlit"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitCharlit([NotNull] MMMLParser.CharlitContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.number"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitNumber([NotNull] MMMLParser.NumberContext context) { return VisitChildren(context); }

	/// <summary>
	/// Visit a parse tree produced by <see cref="MMMLParser.symbol"/>.
	/// <para>
	/// The default implementation returns the result of calling <see cref="AbstractParseTreeVisitor{CodeNode}.VisitChildren(IRuleNode)"/>
	/// on <paramref name="context"/>.
	/// </para>
	/// </summary>
	/// <param name="context">The parse tree.</param>
	/// <return>The visitor result.</return>
	public override CodeNode VisitSymbol([NotNull] MMMLParser.SymbolContext context) { return VisitChildren(context); }

    }
}
