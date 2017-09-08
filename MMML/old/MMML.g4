grammar MMML;

@parser::namespace { mimimil }
@lexer::namespace { mimimil }

@header {
#pragma warning disable 0105 // duplicated using statement
#pragma warning disable 3021 // no need for cls-compliant

using System;
using System.Linq;
using System.Collections.Generic;

using FunctionTable_t = NestedSymbolTable<FunctionEntry>;

using SymbolsTable_t = NestedSymbolTable<Symbol>;

}

@parser::members {

   // Number of errors -- If > 0 , do not generate code
   public int nerrors = 0;

   // The current symbol table
   public SymbolsTable_t currentSymbols = null;

   // The function table
   public FunctionTable_t functionTable = new FunctionTable_t();

   public bool supressMessages = false;

   public const int ERROR = 0;
   public const int WARN = 1;
   public const int INFO = 2;

   public void message(int level, string fmt, params object[] p)
   {
       if (supressMessages) return;
       string head;
       switch(level) {
          case ERROR: head = "ERROR"; break;
          case WARN:  head = "WARNING"; break;
          case INFO:  head = "INFO"; break;
          default:    head = "????"; break;
       }

       Console.WriteLine(head + ": " + fmt, p);
   }
}

options {
   language=CSharp_v4_5;
}


/*
Programa: Declarações de funções e uma função main SEMPRE

def fun x = x + 1

def main =
  let x = read_int
  in
     print concat "Resultado" (string (fun x))
*/

WS : [ \r\t\u000C\n]+ -> channel(HIDDEN) ;

COMMENT : '//' ~('\n'|'\r')* '\r'? '\n' -> channel(HIDDEN);

program
@after {
    Console.WriteLine("Errors: {0}", nerrors);

}
    : fdecls maindecl
    ;

fdecls
    : fdecl fdecls                                   #fdecls_one_decl_rule
    |                                                #fdecls_end_rule
    ;

maindecl
returns [Type retType, SymbolsTable_t symbols]
@init {
    currentSymbols = new SymbolsTable_t();
    $symbols = currentSymbols;
}
@after {
    currentSymbols = null;
}
    :
        'def' 'main' '='
        {
            // Main has no parameters and thus no symbols to start with

            // Main is defined as a UNDEFINED function so we make sure
            // it can never recurse. Functions without parameter
            // should never ever recurse

            var mainFe = new FunctionEntry("main",
                                           new List<Parameter>(), // no parameters for main
                                           Type.BASE_UNDEFINED.Clone(),
                                           $start.Line,
                                           true /* impl is always true for main */);

            if (functionTable.lookup("main") != null) {
                message(ERROR, "Redefinition of function {0} at line {1}",
                                  "main",
                                  $start.Line);
                nerrors++;
            } else {
                functionTable.store("main", mainFe);
            }
        }

        funcbody
        {
            // Syntethize the return type and symbols from it's function body
            $retType = $funcbody.ftype;

            if ($retType.IsRecursive) {
                message(ERROR, "Recursive function never returns at line {0}",
                        $start.Line);
                nerrors++;

                $retType = Type.BASE_INT.Clone();
            }

            // Update return type of main
            mainFe.retType = $retType;
        }

                                                          #programmain_rule
    ;

fdecl
returns [Type retType, List<Parameter> plist, SymbolsTable_t symbols = null]
locals [FunctionEntry orgFe = null, FuncbodyContext fbagain]
@init {
    currentSymbols = new SymbolsTable_t();
    $symbols = currentSymbols;
}
@after {
    currentSymbols = null;
}
    :   'def' functionname fdeclparams '='
        {
            // New symbols-table for the parameters
            $plist = $fdeclparams.plist;

            // Each parameter is a symbol
            foreach (var i in $plist) {
                if (currentSymbols.lookup(i.name) != null) {
                    message(ERROR,
                            "In function {0}, parameter {1} "+
                            "shadows another parameter with the same name",
                            $functionname.text, i.name);
                    nerrors++;
                }
                var sym = new Symbol(i.name, i.type, i.line);
                currentSymbols.store(i.name, sym);
            }

            // If function was not previously declared, declare it now
            // with type recursive
            if (functionTable.lookup($functionname.text) == null) {
                $orgFe = new FunctionEntry($functionname.text,
                                           $plist,
                                           Type.BASE_RECURSIVE.Clone(),
                                           $start.Line, false);
                functionTable.store($functionname.text, $orgFe);
            }

            // Mark the position in the input stream, as we'll have to
            // return here to re-evaluate the body

            var funcBodyBeginMark = _input.Mark();
            var funcBodyBeginIndex = _input.Index;

            // Shhhhh while we just peek at the function body
            supressMessages = true;
        }

        funcbody
        {
            // Ok, you can warn/err again now
            supressMessages = false;

            $retType = $funcbody.ftype;

            if ($retType.IsRecursive) {
                message(ERROR, "Recursive function never returns at line {0}",
                        $functionname.start.Line);
                nerrors++;

                $retType = Type.BASE_INT.Clone();
            }

            var newFe = new FunctionEntry($functionname.text,
                                          $plist,
                                          $retType,
                                          $start.Line,
                                          true /* has impl now */);
            // Store in the table
            $orgFe = null;
            var ftentry = functionTable.lookup( $functionname.text );
            if (ftentry != null) {
                $orgFe = ftentry.symbol;
                if ($orgFe.hasImpl) {
                    message(ERROR, "Redefinition of function {0} at line {1}",
                            $functionname.text,
                            $functionname.start.Line);
                    nerrors++;
                } else if (!newFe.MatchesDecl($orgFe)) {
                    message(ERROR, "Implementation of function {0} at line {1} " +
                            "does not match previous defitition at line {2}",
                            $functionname.text, $start.Line,
                            $orgFe.line);
                    nerrors++;
                } else {
                    if ($orgFe.retType > newFe.retType) {
                        // strictly greater: we'll need to cast
                        Console.WriteLine("Cast {0} -> {1}", newFe.retType, $orgFe.retType);
                        newFe.retType = $orgFe.retType;
                    }
                    functionTable.store($functionname.text, newFe);
                }
            }

            // Now backtrack, and eval the expression again
            _input.Seek(funcBodyBeginIndex);
            _input.Release(funcBodyBeginMark);

            // Discard symbols generated during type deduction
            // $funcbody.symbols may be currentSymbols, but it's ok as
            // a table isn't nested within itself by definition
            $funcbody.symbols.Discard();

            $fbagain = funcbody();

            if ($fbagain.ftype != $funcbody.ftype)
            {
                message(ERROR, "Function {0} should return {1}, " +
                        "but it's body evaluates to ${2}",
                        $functionname.text,
                        $funcbody.ftype,
                        $fbagain.ftype);
                nerrors++;
            }

            //Console.WriteLine("Current symbol table has {0} size and {1} symbols total nested",
            //currentSymbols.Count, currentSymbols.NestedSize);
        }
                                                     #funcdef_rule
    |  'def' functionname fdeclparams '->' type
        {
            $retType = new Type($type.basetype, $type.dimension);
            $plist = $fdeclparams.plist;

            $orgFe = new FunctionEntry($functionname.text,
                                       $plist,
                                       $retType,
                                       $functionname.start.Line,
                                       false /*no impl*/);
            // Store in the table
            if (functionTable.lookup($functionname.text) != null) {
                message(ERROR, "Redefinition of function {0} at line {1}",
                                  $functionname.text,
                                  $functionname.start.Line);
                nerrors++;
            } else {
                functionTable.store($functionname.text,
                                    $orgFe);
            }
        }
                                                          #funcdef_definition
    ;

fdeclparams
returns [List<Parameter> plist, SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
    $plist = new List<Parameter>();
}
@after {
     /*foreach (var p in $plist) {
        //Console.WriteLine("Parametro: {0} -- tipo {1}", p.name, p.type );
       } */
}
    :   fdeclparam
        {
            var type = new Type($fdeclparam.ptype, $fdeclparam.pdimension);
            if (! type.IsValid )
            {
                message(ERROR, "Invalid type ``{0}´´ at {1}",
                                  $fdeclparam.ptype,
                                  $fdeclparam.start.Line);
                nerrors++;
                // Error recovery
                type = Type.BASE_INT.Clone();
            }

            var param = new Parameter(type, $fdeclparam.pname, $fdeclparam.start.Line);

            $plist.Add(param);
        }
        fdeclparams_cont[$plist]

                                                     #fdeclparams_one_param_rule
    |                                                #fdeclparams_no_params
    ;

fdeclparams_cont[List<Parameter> plist]
returns[SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : ',' fdeclparam
        {
            var type = new Type($fdeclparam.ptype, $fdeclparam.pdimension);
            if (! type.IsValid )
            {
                message(ERROR, "Invalid type ``{0}´´ at {1}",
                                  $fdeclparam.ptype,
                                  $fdeclparam.start.Line);
                nerrors++;
                // Error recovery
                type = Type.BASE_INT.Clone();
            }

            var param = new Parameter(type, $fdeclparam.pname, $fdeclparam.start.Line);

            $plist.Add(param);
        }
        fdeclparams_cont[$plist]
                                                     #fdeclparams_cont_rule
    |                                                #fdeclparams_end_rule
    ;

fdeclparam
returns [string pname, string ptype, int pdimension, SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : symbol ':' type
        {
            $pname = $symbol.text;
            $ptype = $type.basetype;
            $pdimension = $type.dimension;
        }
        #fdecl_param_rule
    ;

functionname: TOK_ID                                 #fdecl_funcname_rule
    ;

type
returns [string basetype, int dimension, SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : basic_type
      {
          $dimension = 0;
          $basetype = $basic_type.text;
      }
                                                    #basictype_rule
    | sequence_type
      {
          $dimension = $sequence_type.dimension;
          $basetype = $sequence_type.basetype;
      }
                                                    #sequencetype_rule
    ;

basic_type
    : 'int'
    | 'bool'
    | 'str'
    | 'float'
    ;

sequence_type
returns [int dimension=0, String basetype, SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    :   basic_type '[]'
        {
            $dimension = 1;
            $basetype = $basic_type.text;
        }

                                                     #sequencetype_basetype_rule
    |   s=sequence_type '[]'
        {
            $dimension = $s.dimension + 1;
            $basetype = $s.basetype;
        }
                                                     #sequencetype_sequence_rule
    ;

funcbody
returns [Type ftype, SymbolsTable_t symbols]
@init {
    $ftype = Type.BASE_UNDEFINED.Clone();
}
@after {}
    :   'if'
        {
            $symbols = currentSymbols;
            Type bodytrue_type, bodyfalse_type;
            TypeUtils.CastEntry ce = null;
        }

        cond=funcbody

        {
            // tuple is <warn, type>
            var ce_cond = $cond.ftype.Cast(Type.BASE_BOOL);
            if (ce_cond == null)
            {
                message(ERROR, "Could not convert expression " +
                                  "of type {0} to bool on line {1}",
                                  $cond.ftype, $cond.start.Line);
                nerrors ++;
            } else {
                if (!ce_cond.autoCoerce) {
                    // should never happen, as anything -> bool is ok
                    message(ERROR, "Could not implictly convert expression" +
                                      " of type {0} to bool on line {1}",
                                      $cond.ftype,
                                      $cond.start.Line);
                    nerrors ++;

                    // error recovery: keep on
                }

                if (ce_cond.warn) {
                    // Should never happen, anything is implictly converted to bool
                    message(WARN, "May loose precision on implicit " +
                                      "convertion from {0} to bool in line {1}",
                                      $cond.ftype, $cond.start.Line);
                }
            }
        }

        'then' bodytrue=funcbody
        {
            bodytrue_type = $bodytrue.ftype;
        }
        'else' bodyfalse=funcbody
        {
            bodyfalse_type = $bodyfalse.ftype;

            // Now see the resulting type
            if (bodytrue_type >= bodyfalse_type)
            {
                // cast bodyfalse -> bodytrue
                ce = bodyfalse_type.Cast(bodytrue_type);
                if (ce == null) {
                    message(ERROR, "Can not coerce type " +
                                      "{0} at line {1} to {2}",
                                      bodyfalse_type,
                                      $bodyfalse.start.Line,
                                      bodytrue_type);
                    nerrors++;
                } else {
                    // Can cast, but if it's not implicit, do not coerce
                    if (!ce.autoCoerce) {
                        // Can not cast or it's not an implicit coertion
                        message(ERROR, "Can not implictly coerce " +
                                          "from type {0} at line {1} to {2}",
                                          bodyfalse_type,
                                          $bodyfalse.start.Line,
                                          bodytrue_type);
                        nerrors++;
                        // Error recovery: pretend the cast was ok
                    }

                    if (ce.warn) {
                        message(WARN, "May lose precision on " +
                                          "implicit convertion " +
                                          "from {0} to {1} in line {2}",
                                          bodyfalse_type,
                                          bodytrue_type,
                                          $bodyfalse.start.Line);
                    }
                }

                $ftype = bodytrue_type;
            } else {
                // cast bodytrue -> bodyfalse
                ce = bodytrue_type.Cast(bodyfalse_type);
                if (ce == null) {
                    message(ERROR, "Can not coerce type " +
                                      "{0} at line {1} to {2}",
                                      bodytrue_type,
                                      $bodytrue.start.Line,
                                      bodyfalse_type);
                    nerrors++;
                } else {
                    if (!ce.autoCoerce) {
                        // Can cast, but it's not implicit
                        message(ERROR, "Can not implictly coerce " +
                                          "from type {0} at line {1} to {2}",
                                          bodytrue_type,
                                          $bodytrue.start.Line,
                                          bodyfalse_type);
                        nerrors++;
                        // Error recovery: pretend the cast was ok
                    }

                    if (ce.warn) {
                        message(WARN, "May lose precision on " +
                                          "implicit convertion " +
                                          "from {0} to {1} in line {2}",
                                          bodytrue_type,
                                          bodyfalse_type,
                                          $bodytrue.start.Line);
                    }
                }

                $ftype = bodyfalse_type;
            }
        }
                                                          #fbody_if_rule
    |   'let'
        {
            currentSymbols =  new SymbolsTable_t(currentSymbols);
            $symbols = currentSymbols;
        }
        letlist
        'in'
        fnested=funcbody
        {
            $ftype = $fnested.ftype;
            currentSymbols = currentSymbols.Parent;
        }
                                                          #fbody_let_rule
    |   {
            // Keep on the current table
            $symbols = currentSymbols;
        }
        metaexpr
        {
            $ftype = $metaexpr.etype;
        }
                                                          #fbody_expr_rule
    ;

letlist
returns [SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : letvarexpr  letlist_cont
                                                          #letlist_rule
    ;

letlist_cont
returns [SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    :   ',' letvarexpr
        letlist_cont
                                                          #letlist_cont_rule
    |                                                     #letlist_cont_end
    ;

letvarexpr
returns [SymbolsTable_t symbols, int offset]
@init {
    $symbols = currentSymbols;
}
    :   sym=symbol '=' funcbody
        {
            // Check if we have the symbol on same level
            var sameLevelSym = currentSymbols.lookup($sym.text, 1);
            if (sameLevelSym != null && sameLevelSym.symbol.type != $funcbody.ftype)
            {
                // Redeclaration on same level with different types is supported?
                message(ERROR, "Can not redefine variable {0}" +
                        " with different type on the same level" +
                        "on line {1}",
                        $sym.text, $start.Line);
                nerrors++;
            }
            // Check if we'd shadow someone on any upper level
            var oldSym = currentSymbols.lookup($sym.text);
            if (oldSym != null) {
                message(WARN, "Declaration of {0} in line {1} "+
                        "shadows previous declaration (line {2}) ",
                        $sym.text, $sym.start.Line,
                        oldSym.symbol.line);
            }

            var newSym = new Symbol($sym.text,
                                    $funcbody.ftype,
                                    $sym.start.Line);

            // TODO pass type size as parameter!
            $offset = currentSymbols.store($sym.text, newSym);
        }
        #letvarattr_rule
    |    '_'    '=' funcbody                         #letvarresult_ignore_rule
    |    symbol '::' symbol '=' funcbody             #letunpack_rule
    ;

metaexpr
returns [Type etype, SymbolsTable_t symbols]
@init {
    $etype = Type.BASE_UNDEFINED.Clone();
    $symbols = currentSymbols;
}
@after {
    //Console.WriteLine("Expression type is {0}", $etype);
}
    :   // Anything in parenthesis -- if, let, funcion call, etc
        '(' funcbody ')'
        {
            $etype = $funcbody.ftype;
            //Console.WriteLine("\t\tTipo de PARENS: {0} ", $etype);
        }
                                                          #me_exprparens_rule

    | sequence_expr                                  #me_list_create_rule    // creates a list [x]
    | TOK_NEG symbol
        {
            var sym = currentSymbols.lookup($symbol.text);
            if (sym == null) {
                message(ERROR, "Unknown symbol ``{0}'' at line {1}",
                                  $symbol.text,
                                  $symbol.start.Line);
                nerrors++;
            }

            $etype = Type.BASE_BOOL.Clone();
        }
                                                          #me_boolneg_rule        // Negate a variable
    | TOK_NEG '(' funcbody ')'
        {
            $etype = Type.BASE_BOOL.Clone();
        }
        #me_boolnegparens_rule  //        or anything in between ( )

    | l=metaexpr op=TOK_POWER r=metaexpr
        {
            $etype = $l.etype.OpType($op.text, $r.etype);

            if (!$etype.IsValid)
            {
                TypeUtils.typeError($l.etype, $r.etype, $op.text, $metaexpr.start.Line);
                nerrors ++;
                $etype = Type.BASE_INT.Clone();
            }
        }

                                                          #me_exprpower_rule      // Exponentiation
    | l=metaexpr op=TOK_CONCAT r=metaexpr
        {
            //Console.WriteLine("On concat: {0}, {1}", $l.etype, $r.etype);

            $etype = $l.etype.OpType($op.text, $r.etype);

            //Console.WriteLine("after op: {0}", $etype);

            if (!$etype.IsValid)
            {
                TypeUtils.typeError($l.etype, $r.etype, $op.text, $metaexpr.start.Line);
                nerrors ++;
                $etype = Type.BASE_INT.Clone();
            }
        }
                                                          #me_listconcat_rule     // Sequence concatenation
    | l=metaexpr op=TOK_DIV_OR_MUL r=metaexpr
        {
            $etype = $l.etype.OpType($op.text, $r.etype);

            //Console.WriteLine("after op: {0}", $etype);

            if (!$etype.IsValid)
            {
                TypeUtils.typeError($l.etype, $r.etype, $op.text, $metaexpr.start.Line);
                nerrors ++;
                $etype = Type.BASE_INT.Clone();
            }
        }
                                                          #me_exprmuldiv_rule     // Div and Mult are equal
    | l=metaexpr op=TOK_PLUS_OR_MINUS r=metaexpr
        {
            $etype = $l.etype.OpType($op.text, $r.etype);

            //Console.WriteLine("after op: {0}", $etype);

            if (!$etype.IsValid)
            {
                TypeUtils.typeError($l.etype, $r.etype, $op.text, $metaexpr.start.Line);
                nerrors ++;
                $etype = Type.BASE_INT.Clone();
            }
        }
                                                          #me_exprplusminus_rule  // Sum and Sub are equal
    | metaexpr TOK_CMP_GT_LT metaexpr
        {
            $etype = Type.BASE_BOOL.Clone();
        }
                                                          #me_boolgtlt_rule       // < <= >= > are equal
    | metaexpr TOK_CMP_EQ_DIFF metaexpr
        {
            $etype = Type.BASE_BOOL.Clone();
        }
                                                          #me_booleqdiff_rule     // == and != are egual
    | metaexpr TOK_BOOL_AND_OR metaexpr
        {
            $etype = Type.BASE_BOOL.Clone();
        }
        #me_boolandor_rule      // &&   and  ||  are equal
    | symbol
        {
            var sym = currentSymbols.lookup($symbol.text);
            if (sym == null) {
                message(ERROR, "Unknown symbol ``{0}'' at line {1}",
                                  $symbol.text,
                                  $symbol.start.Line);
                nerrors++;

                // Error recovery: default to int
                $etype = Type.BASE_INT.Clone();
            } else {
                $etype = sym.symbol.type;
                //Console.WriteLine("Encontrou Símbolo {0} tipo {1} em {2}",
                                  //$symbol.text,
                                  //$etype,
                                  //sym.offset);
            }
        }
        #me_exprsymbol_rule     // a single symbol
    | literal
        {
            $etype = $literal.ltype;
        }
        #me_exprliteral_rule    // literal value
    | funcall
        {
            $etype = $funcall.fctype;
        }
        #me_exprfuncall_rule    // a funcion call
    | cast
        {
            $etype = $cast.ctype;
        }
        #me_exprcast_rule       // cast a type to other
    ;

sequence_expr
returns [Type stype, SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : '[' funcbody ']'                               #se_create_seq
    ;

funcall
returns [Type fctype, SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : symbol funcall_params
      {
          var funcentry = functionTable.lookup($symbol.text);
          if (funcentry == null) {
              message(ERROR, "Could not find definition for function {0} called from line {1}",
                      $symbol.text,
                      $symbol.start.Line);
              nerrors++;

              // Error recovery
              $fctype = Type.BASE_INT.Clone();
          } else {
              $fctype = funcentry.symbol.retType;

              var plist = funcentry.symbol.plist;
              var clist = $funcall_params.clist;

              if (plist.Count != clist.Count) {
                  message(ERROR, "Wrong number of parameters for "+
                          "function {0}: expected {1}, got {2}",
                          $symbol.text,
                          plist.Count,
                          clist.Count);
                  nerrors++;
              } else {
                  int i = 1;
                  var zippedParams = plist.Zip(clist,
                                               (p, c) => new { PType = p.type, CType = c.type, Pos = i++ });
                  foreach (var pc in zippedParams) {
                      // See the cast entry from Call Type fo Parameter Type
                      var ce = pc.CType.Cast(pc.PType);
                      if (ce == null) {
                          message(ERROR, "In call to {0}, line {1}: " +
                                  "Invalid type for parameter {0}. " +
                                  "Expected {2}, got {3}",
                                  $symbol.text, $start.Line,
                                  pc.Pos, pc.PType, pc.CType);
                          nerrors++;
                      } else if (!ce.autoCoerce){
                          message(ERROR, "In call to {0}, line {1}: " +
                                  "No implicit convertion from {2} " +
                                  "to {3} on parameter {4}",
                                  $symbol.text, $start.Line,
                                  pc.CType, pc.PType, pc.Pos);
                          nerrors++;
                      } else if (ce.warn) {
                          message(WARN, "In call to {0}, line {1}: " +
                                  "May lose precision in conversion from {2}" +
                                  " to {3} on parameter {4}",
                                  $symbol.text, $start.Line,
                                  pc.CType, pc.PType, pc.Pos);
                      }
                  }
              }
          }
      }
      #funcall_rule
    ;

cast
returns [Type ctype, SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : c=type funcbody
        {
            $ctype = new Type($c.text, 0);

            // Look up a cast entry for an explicit cast from funcbody
            // to the 0-dim type in c
            var ce = $funcbody.ftype.Cast(new Type($c.text, 0));

            if (ce == null) {
                message(ERROR, "Can not cast from {0} to {1} at line {2}",
                                  $funcbody.ftype, $c.text, $c.start.Line);
                nerrors++;

                // Error recovery: pretend the cast worked
            } else if (ce.warn) {
                message(WARN,
                        "Cast from {0} to {1} at line {2} may lose precision",
                        $funcbody.ftype, $ctype, $start.Line);
            }
        }
                                                        #cast_rule
    ;

funcall_params
returns [List<CallParameter> clist, SymbolsTable_t symbols]
@init {
    $clist = new List<CallParameter>();
    $symbols = currentSymbols;
}
    :   metaexpr
        {
            $clist.Add(new CallParameter($metaexpr.etype));
        }
        funcall_params_cont[$clist]

                                                          #funcallparams_rule
    |   '_'
        {
            // leave the list empty
        }
                                                          #funcallnoparam_rule
    ;

funcall_params_cont[List<CallParameter> clist]
returns[SymbolsTable_t symbols]
@init {
    $symbols = currentSymbols;
}
    : metaexpr
        {
           $clist.Add(new CallParameter($metaexpr.etype));
        }

        funcall_params_cont[$clist]
                                                          #funcall_params_cont_rule
    |   {
            // do not add
        }                                                 #funcall_params_end_rule
    ;

literal
    returns [Type ltype]
    @init {
       $ltype = Type.BASE_UNDEFINED.Clone() ;
    }
    :   'nil'
        {
            $ltype = Type.BASE_NIL.Clone();
        }
                                                          #literalnil_rule
    |   ('true' | 'false')
        {
            $ltype = Type.BASE_BOOL.Clone();
        }
                                                        #literaltrueorfalse_rule
    |   number
        {
            $ltype = new Type($number.ntype, 0);
        }
                                                        #literalnumber_rule
    |   strlit
        {
            $ltype = Type.BASE_STR.Clone();
        }
                                                        #literalstring_rule
    |   charlit
        {
            $ltype = Type.BASE_CHAR.Clone();
        }
                                                        #literal_char_rule
    ;

strlit: TOK_STR_LIT
    ;

charlit
    : TOK_CHAR_LIT
    ;

number
    returns [BasicType ntype]
    :   FLOAT       { $ntype = BasicType.FLOAT; }
                                                          #numberfloat_rule
    |   DECIMAL     { $ntype = BasicType.INT; }
                                                          #numberdecimal_rule
    |   HEXADECIMAL { $ntype = BasicType.INT; }
                                                          #numberhexadecimal_rule
    |   BINARY      { $ntype = BasicType.INT; }
                                                          #numberbinary_rule
                ;

symbol: TOK_ID                                          #symbol_rule
    ;


// id: begins with a letter, follows letters, numbers or underscore
TOK_ID: [a-zA-Z]([a-zA-Z0-9_]*);

TOK_CONCAT: '::' ;
TOK_NEG: '!';
TOK_POWER: '^' ;
TOK_DIV_OR_MUL: ('/'|'*');
TOK_PLUS_OR_MINUS: ('+'|'-');
TOK_CMP_GT_LT: ('<='|'>='|'<'|'>');
TOK_CMP_EQ_DIFF: ('=='|'!=');
TOK_BOOL_AND_OR: ('&&'|'||');

TOK_REL_OP : ('>'|'<'|'=='|'>='|'<=') ;

TOK_STR_LIT
  : '"' (~[\"\\\r\n] | '\\' (. | EOF))* '"'
  ;


TOK_CHAR_LIT
    : '\'' (~[\'\n\r\\] | '\\' (. | EOF)) '\''
    ;

FLOAT : '-'? DEC_DIGIT+ '.' DEC_DIGIT+([eE][\+-]? DEC_DIGIT+)? ;

DECIMAL : '-'? DEC_DIGIT+ ;

HEXADECIMAL : '0' 'x' HEX_DIGIT+ ;

BINARY : BIN_DIGIT+ 'b' ; // Sequencia de digitos seguida de b  10100b

fragment
BIN_DIGIT : [01];

fragment
HEX_DIGIT : [0-9A-Fa-f];

fragment
DEC_DIGIT : [0-9] ;
