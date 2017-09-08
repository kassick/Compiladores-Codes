namespace mimimil
{


    using System.Collections.Generic;
    using System;
    using System.Linq;

    using FunctionTable_t = NestedSymbolTable<FunctionEntry>;
    using SymbolsTable_t = NestedSymbolTable<Symbol>;

    // Class to represent a parameter type
    public enum BasicType {
        INT = 1,
        FLOAT = 2,
        STR = 3,
        CHAR = 4,
        BOOL = 5,
        NIL = 6,             // nil value, can be casted to anything
        RECURSE_SELF = -100, // Internal use to solve recursion
        UNDEFINED = -1       // used when we still don't know the result type

    };

    public class Type {
        public BasicType btype;
        public int dim;

        public static Type BASE_INT = new Type(BasicType.INT, 0);
        public static Type BASE_FLOAT = new Type(BasicType.FLOAT, 0);
        public static Type BASE_CHAR = new Type(BasicType.CHAR, 0);
        public static Type BASE_STR = new Type(BasicType.STR, 0);
        public static Type BASE_BOOL = new Type(BasicType.BOOL, 0);
        public static Type BASE_NIL = new Type(BasicType.NIL, -1);
        public static Type BASE_RECURSIVE = new Type(BasicType.RECURSE_SELF, -100);
        public static Type BASE_UNDEFINED = new Type(BasicType.UNDEFINED, -1);

        public Type(BasicType btype, int dim)
        {
            this.btype = btype;
            this.dim = dim;
        }

        public Type(string btstr, int dim)
            : this(TypeUtils.getType(btstr), dim)
        {}

        public Type Clone() {
            return new Type(this.btype, this.dim);
        }

        public override string ToString() {
            string ret;
            switch (this.btype) {
                case BasicType.FLOAT:
                    ret =  "float";
                    break;
                case BasicType.INT:
                    ret =  "int";
                    break;
                case BasicType.STR:
                    ret =  "str";
                    break;
                case BasicType.BOOL:
                    ret =  "bool";
                    break;
                case BasicType.CHAR:
                    ret =  "char";
                    break;
                case BasicType.RECURSE_SELF:
                    return "(recurse)";
                case BasicType.NIL:
                    return "nil";
                default:
                    return "(invalid)";
            }

            int tmpdim = this.dim;

            while ( (tmpdim--) > 0)
                ret = ret + "[]";

            return ret;
        }

        // auto convert to string
        public static implicit operator string(Type t)
        {
            return t.ToString();
        }

        public bool IsValid {
            get {
                return this.btype != BasicType.UNDEFINED;
            }
        }

        public bool IsRecursive {
            get { return this.btype == BasicType.RECURSE_SELF; }
        }

        public bool IsNumeric {
            get {
                return (btype == BasicType.INT ||
                        btype == BasicType.FLOAT);

            }
        }

        public bool IsSequence {
            get {
                return dim > 0;
            }
        }

        public bool IsNil {
            get {
                return this.btype == BasicType.NIL;
            }
        }

        /*-----------------------------------------------------------------------------
         * Function: Operator ==
         * Description:
         *---------------------------------------------------------------------------*/
        public static bool operator ==(Type left, Type right)
        {
            return (left.btype == right.btype && left.dim == right.dim);
        }

        public override bool Equals(Object other)
        {
            if (other is Type)
                return (((Type)this).btype == ((Type)other).btype  &&
                        ((Type)this).dim   == ((Type)other).dim );
            else return false;
        }

        public override int GetHashCode() {
            // stupid hash code
           int hash = 13;
           hash = (hash * 7) + btype.GetHashCode();
           hash = (hash * 7) + dim.GetHashCode();
           return hash;
        }

        /*-----------------------------------------------------------------------------
         * Function: Operator !=
         * Description:
         *---------------------------------------------------------------------------*/
        public static bool operator !=(Type left, Type right) {
            return
                left.btype != right.btype || left.dim != right.dim;
        }

        /*-----------------------------------------------------------------------------
         * Function: Operator >
         * Description:
         *---------------------------------------------------------------------------*/
        public static bool operator >(Type left, Type right)
        {
            if (left.btype != BasicType.UNDEFINED &&
                     right.btype == BasicType.UNDEFINED)
                return true;
                 // !(undefined > concrete)
            if (left.btype == BasicType.UNDEFINED &&
                right.btype != BasicType.UNDEFINED)
                return false;
            // the highes-dimension type has priority
            else if (left.dim > right.dim)
                return true;
            else if (left.dim < right.dim)
                return false;
                // same dimension
            else
                return
                    Array.IndexOf(TypeUtils.TYPE_PRIORITY_ORDER, left.btype) >
                    Array.IndexOf(TypeUtils.TYPE_PRIORITY_ORDER, right.btype);
        }


        /*-----------------------------------------------------------------------------
         * Function: Operator >=
         * Description:
         *---------------------------------------------------------------------------*/
        public static bool operator >=(Type left, Type right)
        {
            return
                left == right ||
                left > right;
        }

        /*-----------------------------------------------------------------------------
         * Function: Operator <
         * Description: Just returns !(< || ==)
         *---------------------------------------------------------------------------*/
        public static bool operator<(Type left, Type right)
        {
            return !(left >= right);
        }

        /*-----------------------------------------------------------------------------
         * Function: Operator <=
         * Description: Returns !(l > r)
         *---------------------------------------------------------------------------*/
        public static bool operator<=(Type left, Type right)
        {
            return !(left > right);
        }


        /*-----------------------------------------------------------------------------
         * Function: OpType
         * Description: Returns the Type of operating this with right with operation op
         *---------------------------------------------------------------------------*/
        public Type OpType(string op, Type right)
        {
            // recursive? -> returns the other
            if (this.IsRecursive)
                return new Type(right.btype, right.dim);
            else if (right.IsRecursive)
                return new Type(this.btype, this.dim);

            // 0-dim? handle basic operands
            if (this.dim == 0 && right.dim == 0) {
                // Basic zero-dimensional types
                switch(op) {
                    case "^":
                        // numeric ^ numeric -> float
                        if (this.IsNumeric && right.IsNumeric)
                            return new Type(BasicType.FLOAT, 0);
                        else
                            // No idea what to do with str power bool
                            // or something else
                            return Type.BASE_UNDEFINED.Clone();

                    case "+":
                    case "-":       // Corner case == one of them is char
                                    // -- can sum/sub with int and returns
                                    // char
                        // char + int -> char
                        // int + char -> char
                        if ((this.btype == BasicType.CHAR && right.btype == BasicType.INT) ||
                            (this.btype == BasicType.INT && right.btype == BasicType.CHAR))
                            return new Type(BasicType.CHAR, 0);
                        else if (this.IsNumeric && right.IsNumeric)
                            // numeric + numeric -> {int, float}
                            return new Type(TypeUtils.mathOpMaybeUpcast(this.btype, right.btype), 0);
                        else
                            return Type.BASE_UNDEFINED.Clone();

                    case "/":
                    case "*":
                        // numeric * numeric -> {int, float}
                        if (this.IsNumeric && right.IsNumeric)
                            return new Type(TypeUtils.mathOpMaybeUpcast(this.btype, right.btype), 0);
                        else
                            return Type.BASE_UNDEFINED.Clone();
                    case "::" :
                        // With 0-dimension strings , :: is string
                        // concat, i.e. str :: str -> str
                        if (this.btype == right.btype && this.btype == BasicType.STR)
                            return new Type(BasicType.STR, 0);
                        else
                            // Invalid op
                            return Type.BASE_UNDEFINED.Clone();

                    default:
                        return new Type( BasicType.UNDEFINED, 0);
                }
            } else /*multi-dim*/
                if (op == "::" &&
                       this.btype == right.btype &&
                       this.dim == right.dim) {
                    // any[]... :: any[]... -> any[]...
                    // as long as types are equal and dimensions are the same
                    return new Type(this.btype, this.dim);
            } else
                    return Type.BASE_UNDEFINED.Clone();
        }

        /*-----------------------------------------------------------------------------
         * Function: getMaybeCoertion -> warn, type
         * Description:
         *---------------------------------------------------------------------------*/
        /*
        public Tuple<bool, Type> CoertionTo(Type target)
        {
            if (! this.IsValid ) return BASE_UNDEFINED.Clone();

            // Any recurstion? coerce to the other
            if (this.IsRecursive)
                return new Tuple<bool, Type>(false,
                                             new Type(target.btype, target.dim));
            else if (target.IsRecursive)
                return new Tuple<bool, Type>(false,
                                             new Type(this.btype, this.dim));

            // anything is a 0-dim bool
            if (target.btype == BasicType.BOOL && target.dim == 0)
                return new Tuple<bool, Type>(false,
                                             new Type(BasicType.BOOL, 0));

            // NIL can be casted to ANYTHING
            if (this.btype == BasicType.NIL)
                return new Tuple<bool, Type>(false,
                                             target.Clone());

            if (this.dim != 0 || target.dim != 0) {
                // Multi-dimensional case
                if (this.dim == target.dim &&
                    this.btype == target.btype)
                    // same dimension, same base type
                    return new Tuple<bool, Type>(false,
                                                 target.Clone());
                else
                    // Can not coerce any list of type A to list of type B
                    return new Typle<bool, Type>(false,
                                                 BASE_UNDEFINED.Clone());
            } else {
                // 0-dim coercion

                // char -> int -> float is ok
                if (this.btype == BasicType.CHAR &&
                    (target.btype == BasicType.INT ||
                     target.btype == BasicType.FLOAT))
                    return new Tuple<bool, Type>(false,
                                                 target.Clone());

                // int -> float is ok
                if (org == BasicType.INT &&
                    target.btype == BasicType.FLOAT)
                    return new Tuple<bool, Type>(false,
                                                 target.Clone());

                // float -> int could be coerced with a warning, but our
                // spec says we shouldn't
                //
            }

            return new Tuple<bool, BasicType, int>(false, BASE_UNDEFINED.Clone());
        }
        */

        // Warn, can
        public TypeUtils.CastEntry Cast(Type target) {

            // Invalid -> anything is error , anything -> invalid, error
            if (! this.IsValid  || ! target.IsValid )
                return null;

            // cast the same to the same is always an implicit cast
            if (this == target) {
                return new TypeUtils.CastEntry(false, true);
            }

            // recurstion? cast to anything valid is ok
            if (this.IsRecursive)
                return new TypeUtils.CastEntry(false, true);

            // anything valid -> bool is ok
            if (target.btype == BasicType.BOOL) {
                return new TypeUtils.CastEntry(false, true);
            }

            // NIL can be casted to ANYTHING
            if (this.btype == BasicType.NIL)
                return new TypeUtils.CastEntry(false, true);

            // cast from/to sequences is not accepted, unless target
            // is bool or this is nil
            if (this.IsSequence || target.IsSequence) {
                if (this == target)
                    return new TypeUtils.CastEntry(false, true);
                else
                    return null;
            }

            // 0-dim casts
            Tuple<BasicType, BasicType> k = Tuple.Create(this.btype, target.btype);
            if (TypeUtils.VALID_CASTS.ContainsKey(k)) {
                var ce = TypeUtils.VALID_CASTS[k];
                return ce;
            }

            return null;
        }
    }

    public static class TypeUtils {

        public static BasicType[] TYPE_PRIORITY_ORDER =
            {   BasicType.RECURSE_SELF,
                BasicType.NIL,
                BasicType.CHAR,
                BasicType.INT,
                BasicType.FLOAT,
                BasicType.STR,
                BasicType.BOOL
            };

        public class CastEntry {
            public bool warn;
            public bool autoCoerce;
            // more

            public CastEntry(bool warn, bool autoCoerce=false) {
                this.warn = warn;
                this.autoCoerce = autoCoerce;
            }
        }

        private static Func<BasicType, BasicType, Tuple<BasicType, BasicType>> tc = Tuple.Create;
        private static Func<bool, CastEntry> ce = (warn) => new CastEntry(warn, false);
        private static Func<bool, CastEntry> ceCoerce = (warn) => new CastEntry(warn, true);

        public static readonly Dictionary<Tuple<BasicType, BasicType>, CastEntry>
            VALID_CASTS = new Dictionary<Tuple<BasicType, BasicType>, CastEntry>
            {
                {tc( BasicType.INT   , BasicType.FLOAT)  , ceCoerce(false)},
                {tc( BasicType.INT   , BasicType.CHAR )  , ce(false)},
                {tc( BasicType.INT   , BasicType.STR)    , ce(false)},
                {tc( BasicType.FLOAT , BasicType.INT )   , ce(true)},
                {tc( BasicType.FLOAT , BasicType.STR)    , ce(false)},
                {tc( BasicType.CHAR  , BasicType.STR)    , ce(false)},
                {tc( BasicType.CHAR  , BasicType.INT)    , ceCoerce(false)},
                {tc( BasicType.CHAR  , BasicType.FLOAT)  , ceCoerce(false)},
                {tc( BasicType.STR   , BasicType.INT)    , ce(false)},
                {tc( BasicType.STR   , BasicType.FLOAT)  , ce(false)},
                {tc( BasicType.INT   , BasicType.BOOL)   , ce(false)},
            } ;

        public static BasicType getType(string type) {
            switch (type) {
                case "float":
                    return BasicType.FLOAT;
                case "int":
                    return BasicType.INT;
                case "str":
                    return BasicType.STR;
                case "bool":
                    return BasicType.BOOL;
                case "char":
                    return BasicType.CHAR;
                default:
                    return BasicType.UNDEFINED;
            }
        }

        public static void typeError(Type left, Type right,
                                     string op,
                                     int line )
        {
            Console.WriteLine("Invalid types for operation: {0} {1} {2} at line {3}",
                              left,
                              op,
                              right,
                              line);
        }

        /*
        public static string getTypeStr(BasicType type, int dim) {

            string ret;
            switch (type) {
                case BasicType.FLOAT:
                    ret =  "float";
                    break;
                case BasicType.INT:
                    ret =  "int";
                    break;
                case BasicType.STR:
                    ret =  "str";
                    break;
                case BasicType.BOOL:
                    ret =  "bool";
                    break;
                case BasicType.CHAR:
                    ret =  "char";
                    break;
                default:
                    return null;
            }

            while ( (dim--) > 0)
                ret = ret + "[]";

            return ret;
        }
        */

        public static BasicType mathOpMaybeUpcast(BasicType left, BasicType right)
        {
            if ((left == BasicType.INT || left == BasicType.FLOAT) &&
                (right == BasicType.INT || right == BasicType.FLOAT))
            {
                // Upcast if any is float
                if (left == BasicType.FLOAT || right == BasicType.FLOAT)
                    return BasicType.FLOAT;
                else
                    return BasicType.INT;
            } else {
                // these math ops are only defined for numerical types
                return BasicType.UNDEFINED;
            }
        }


        /*-----------------------------------------------------------------------------
         * Function: cmp_basic_type: cmp-style comparer
         * Description: Returns 0 if both types are equal.
                        > 0 if type left has priority over type right (left > right)
                        < 0 if type right has priority over type left (left < right)
         *---------------------------------------------------------------------------*/
        /*
        public static int cmp_basic_type(BasicType left, int dim_left,
                                         BasicType right, int dim_right) {
            if (left == right && dim_left == dim_right)
                return 0;
            else if (left != BasicType.UNDEFINED && right == BasicType.UNDEFINED)
                return 1;
            if (left == BasicType.UNDEFINED && right != BasicType.UNDEFINED)
                return -1;
            // the highes-dimension type has priority
            else if (dim_left > dim_right)
                return 1;
            else if (dim_left < dim_right)
                return -1;
            else if (Array.IndexOf(TYPE_PRIORITY_ORDER, left) > Array.IndexOf(TYPE_PRIORITY_ORDER, right))
                return 1;
            else if (Array.IndexOf(TYPE_PRIORITY_ORDER, left) < Array.IndexOf(TYPE_PRIORITY_ORDER, right))
                return -1;

            return -1;
        }
        */

        /*-----------------------------------------------------------------------------
         * Function: getMaybeCoertion -> warn, type, dimension
         * Description:
         *---------------------------------------------------------------------------*/
        /*
        public static Tuple<bool, BasicType, int>
            getMaybeCoertion(BasicType org, int dim_org,
                             BasicType target, int dim_target)
        {
            // anything is a bool
            if (target == BasicType.BOOL)
                return new Tuple<bool, BasicType, int>(false, target, 0);

            // NIL can be passed to ANYTHING
            if (org == BasicType.NIL)
                return new Tuple<bool, BasicType, int>(false, target, 0);

            // no multi-dimensiontal coertion allowed
            if (dim_org != dim_target || dim_org != 0) {
                return new Tuple<bool, BasicType, int>(false, BasicType.UNDEFINED, -1);
            }

            // dumb
            if (org == target)
                return new Tuple<bool, BasicType, int>(false, target, 0);


            // char -> int -> float is ok
            if (org == BasicType.CHAR && (target == BasicType.INT ||
                                target == BasicType.FLOAT))
                return new Tuple<bool, BasicType, int>(false, target, 0);

            // int -> float is ok
            if (org == BasicType.INT && target == BasicType.FLOAT)
                return new Tuple<bool, BasicType, int>(false, target, 0);

            // float -> int could be coerced with a warning, but our
            // spec says we shouldn't
            //

            return new Tuple<bool, BasicType, int>(false, BasicType.UNDEFINED, -1);
        }*/

        /*
        public static List<Type> getOpPossibleResults(Type left, Type right, string op) {
            List<Type> results = new List<Type>();

            if (left.btype == BasicType.RECURSE_SELF ||
                right.btype == BasicType.RECURSE_SELF) {
                // Must handle possible cases

                if (left.dim == 0) {
                    // 0-dim valid ops
                    switch(op) {
                        case "+":
                        case "-":
                            // Only accepts math types and char
                            // may return CHAR, INT, FLOAT

                            // left is recurse? then we may end up with
                            // resulting type INT, FLOAT or CHAR, depending on
                            // the right side. Or maybe recursion as well
                            if ( left.btype == BasicType.RECURSE_SELF ) {
                                results.Add(right);
                                if (right.btype == BasicType.INT) {
                                    // Adding something with a known int?
                                    // Result may be a float or a char
                                    results.Add(new Type(BasicType.FLOAT, 0));
                                    results.Add(new Type(BasicType.CHAR, 0));
                                }
                            } else {
                                // Right is recursive then
                                results.Add(left);
                                if (left.btype == BasicType.INT) {
                                    results.Add(new Type(BasicType.FLOAT, 0));
                                    results.Add(new Type(BasicType.CHAR, 0));
                                }
                            }
                            break;

                        case "*":
                        case "/":
                            // Only accepts math types
                            // may return INT, FLOAT

                            // left is recurse? then we may end up with
                            // resulting type INT, FLOAT or CHAR, depending on
                            // the right side. Or maybe recursion as well
                            if ( left.btype == BasicType.RECURSE_SELF ) {
                                results.Add(right);
                                if (right.btype == BasicType.INT) {
                                    // Adding something with a known int?
                                    // Result may be a float or a char
                                    results.Add(new Type(BasicType.FLOAT, 0));
                                }
                            } else {
                                results.Add(left);
                                if (left.btype == BasicType.INT) {
                                    results.Add(new Type(BasicType.FLOAT, 0));
                                }
                            }

                            break;

                        case "^":
                            // As long as we're working with math types or recursive, it's ok
                            if (left.btype == BasicType.RECURSE_SELF ||
                                left.btype == BasicType.INT ||
                                left.btype == BasicType.FLOAT ||
                                right.btype == BasicType.RECURSE_SELF ||
                                right.btype == BasicType.INT ||
                                right.btype == BasicType.FLOAT)
                            {
                                results.Add(new Type(BasicType.FLOAT, 0));
                            }

                            break;
                    }
                } else if (op == "::") {
                    // Concatenation with an unknown type
                    // Possible results may be the concrete type OR a recursive one
                    if (left.btype == BasicType.RECURSE_SELF) {
                        // defer to right
                        results.Add(right);
                    } else {
                        // defer to left
                        results.Add(left);
                    }
                }
            } else {
                // NO RECURSION HERE
                var optype = getOpType(left.btype, left.dim,
                                       right.btype, right.dim,
                                       op);
                if (optype != BasicType.UNDEFINED) {
                    results.Add(new Type(optype, left.dim));
                }
            }
            return results;
        }
        */

        /*
        public static BasicType getOpType(BasicType left, int dim_left,
                                          BasicType right, int dim_right,
                                          string op )
        {
            if (dim_right == 0 && dim_left == 0) {
                Console.WriteLine("0 dim");
                // Basic zero-dimensional types
                switch(op) {
                    case "^": // For valid types, always returns float
                        if ((left == BasicType.FLOAT || left == BasicType.INT) &&
                            (right == BasicType.FLOAT || right == BasicType.INT))
                            return BasicType.FLOAT;
                        else
                            // No idea what to do with str power bool
                            // or something else
                            return BasicType.UNDEFINED;

                    case "+":
                    case "-":       // Corner case == one of them is char
                                    // -- can sum/sub with int and returns
                                    // char
                        if ((left == BasicType.CHAR && right == BasicType.INT) ||
                            (left == BasicType.INT && right == BasicType.CHAR))
                            return BasicType.CHAR;
                        else
                            return mathOpMaybeUpcast(left, right);

                    case "/":
                    case "*":
                        return mathOpMaybeUpcast(left, right);
                    case "::" :
                        // With 0-dimension strings , :: is string
                        // concat, i.e. str :: str -> str
                        if (left == right && left == BasicType.STR)
                            return BasicType.STR;
                        else
                            // Invalid op
                            return BasicType.UNDEFINED;

                    default:
                        return BasicType.UNDEFINED;

                }

            } else if (op == "::" &&
                       dim_left == dim_right &&
                       left == right) {
                return left;
            } else
                return BasicType.UNDEFINED;
        }
        */

    }

    public struct Symbol {
        public string symbol;
        public Type type;
        public int line;

        public Symbol(string name, Type type, int line) {
            this.symbol = name;
            this.type = type;
            this.line = line;
        }
    }

    // Class to represent a Parameter
    public class Parameter {
        public Type type;
        public string name;
        public int line;

        public Parameter(Type type, string name, int line)
        {
            this.type = type;
            this.name = name;
            this.line = line;
        }
    }

    public class CallParameter {
        public Type type;

        // code associated

        public CallParameter(Type type) {
            this.type = type;

        }
    }

    public class FunctionEntry {
        public string name;
        public List<Parameter> plist;
        public Type retType;
        public int line;
        public bool hasImpl;

        public FunctionEntry(string name,
                             List<Parameter> plist,
                             Type retType,
                             int line,
                             bool hasImpl)
        {
            this.name = name;
            this.plist = plist;
            this.retType = retType;
            this.line = line;
            this.hasImpl = hasImpl;
        }

        public bool MatchesDecl(FunctionEntry other) {
            /// Functions are equal if they have the same name, return
            /// the same type, have the same amount of parameters and
            /// the parameters types are the same

            // Same name?
            if (this.name != other.name) {
                return false;
            }

            // Same parameter count?
            if (this.plist.Count != other.plist.Count)
            {
                return false;
            }

            // All parameters are equal?
            if (this.plist
                .Zip(other.plist, (p1, p2) => new {p1, p2})
                .Any(i => i.p1.type != i.p2.type))
            {
                return false;
            }

            // if other is recursive, it matches
            if (other.retType.IsRecursive) return true;

            // Both return the same
            if (this.retType == other.retType) return true;

            var ce = this.retType.Cast(other.retType);
            if (ce != null && ce.autoCoerce) return true;

            // catch all -- no, they're different
            return false;
        }
    }
}
