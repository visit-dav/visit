#include "ExprGrammar.h"
#include "ExprToken.h"

//
//                     VisIt Expression Grammar
//
//
//  Written April 5, 2002 by Jeremy Meredith
//
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 11:43:51 PST 2004
//    Refactored the parser into a standalone module.  This required
//    telling some tokens what their display name is instead of
//    getting it from a function.
//
//    Jeremy Meredith, Wed Nov 24 14:52:41 PST 2004
//    Removed Expr => List and added Arg => List because we cannot
//    work with lists as expressions just yet.  We can change this
//    back someday.
//
//    Jeremy Meredith, Wed Jun  8 17:04:23 PDT 2005
//    Made symbols all be associated with a dictionary.
//

Dictionary D;

// ------------------------------------------------------------------------
// TERMINALS
// ------------------------------------------------------------------------
Symbol T_Plus          (D, '+');
Symbol T_Mult          (D, '*');
Symbol T_Minus         (D, '-');
Symbol T_Slash         (D, '/');
Symbol T_Exp           (D, '^');
Symbol T_Mod           (D, '%');
Symbol T_LBracket      (D, '[');
Symbol T_RBracket      (D, ']');
Symbol T_LParen        (D, '(');
Symbol T_RParen        (D, ')');
Symbol T_LCarat        (D, '<');
Symbol T_RCarat        (D, '>');
Symbol T_LBrace        (D, '{');
Symbol T_RBrace        (D, '}');
Symbol T_Pound         (D, '#');
Symbol T_Equal         (D, '=');
Symbol T_At            (D, '@');
Symbol T_Comma         (D, ',');
Symbol T_Colon         (D, ':');
Symbol T_Ident         (D, TT_Identifier,   "Identifier");
Symbol T_Integer       (D, TT_IntegerConst, "Integer");
Symbol T_Float         (D, TT_FloatConst,   "Float");
Symbol T_String        (D, TT_StringConst,  "String");
Symbol T_Bool          (D, TT_BoolConst,    "Bool");


// ------------------------------------------------------------------------
// NON-TERMINALS
// ------------------------------------------------------------------------
Symbol Expr            (D,  "Expr"         );
Symbol Constant        (D,  "Constant"     );
Symbol Vector          (D,  "Vector"       );
Symbol List            (D,  "List"         );
Symbol ListElems       (D,  "ListElems"    );
Symbol ListElem        (D,  "ListElem"     );
Symbol Function        (D,  "Function"     );
Symbol Args            (D,  "Args"         );
Symbol Arg             (D,  "Arg"          );
Symbol PathSpec        (D,  "PathSpec"     );
Symbol MultiSlash      (D,  "MultiSlash"   );
Symbol Variable        (D,  "Variable"     );
Symbol Database        (D,  "Database"     );
Symbol DBSpec          (D,  "DBSpec"       );
Symbol PathMachSpec    (D,  "PathMachSpec" );
Symbol MachSpec        (D,  "MachSpec"     );
Symbol TimeSpec        (D,  "TimeSpec"     );


// ****************************************************************************
//  Constructor:  ExprGrammar::ExprGrammar
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ExprGrammar::ExprGrammar() : Grammar(D)
{
  // ----------------------------------------------------------------------

  // Start rule
  SetStartSymbol(Expr);

  // ----------------------------------------------------------------------

  // Expression
  AddRule(Rule(0,  Expr)  >>  Expr + T_Plus  + Expr );    // addition
  AddRule(Rule(1,  Expr)  >>  Expr + T_Minus + Expr );    // subtraction
  AddRule(Rule(2,  Expr)  >>  Expr + T_Mult  + Expr );    // multiplication
  AddRule(Rule(3,  Expr)  >>  Expr + T_Slash + Expr );    // division
  AddRule(Rule(4,  Expr)  >>  Expr + T_Exp   + Expr );    // exponentiation
  AddRule(Rule(5,  Expr)  >>  Expr + T_Mod   + Expr );    // modulo
  AddRule(Rule(6,  Expr)  >>  Expr + T_LBracket + T_Integer + T_RBracket );    // index
  AddRule(Rule(7,  Expr)  >>  T_Minus + Expr,  5);        // u-negative, prec=5
  AddRule(Rule(8,  Expr)  >>  T_LParen + Expr + T_RParen );
  AddRule(Rule(9,  Expr)  >>  Constant );
  AddRule(Rule(10, Expr)  >>  Vector   );
  AddRule(Rule(11, Expr)  >>  Function );
  AddRule(Rule(12, Expr)  >>  Variable );
  AddRule(Rule(13, Expr)  >>  Database );
  //AddRule(Rule(14, Expr)  >>  List     );

  // Constant
  AddRule(Rule(0, Constant)  >>  T_Integer );
  AddRule(Rule(1, Constant)  >>  T_Float   );
  AddRule(Rule(2, Constant)  >>  T_String  );
  AddRule(Rule(3, Constant)  >>  T_Bool    );

  // Vector
  AddRule(Rule(0, Vector) >> T_LBrace + Expr + T_Comma + Expr + T_RBrace);
  AddRule(Rule(1, Vector) >> T_LBrace + Expr + T_Comma + Expr + T_Comma + Expr + T_RBrace);

  // List
  AddRule(Rule(0, List)      >> T_LBracket + ListElems + T_RBracket);

  // ListElems
  AddRule(Rule(0, ListElems) >>  ListElems + T_Comma + ListElem);
  AddRule(Rule(1, ListElems) >>  ListElem);

  // ListElem
  AddRule(Rule(0, ListElem)  >>  Expr);
  AddRule(Rule(1, ListElem)  >>  Expr + T_Colon + Expr);
  AddRule(Rule(2, ListElem)  >>  Expr + T_Colon + Expr + T_Colon + Expr);

  // Function
  AddRule(Rule(0, Function)  >>  T_Ident + T_LParen + T_RParen);
  AddRule(Rule(1, Function)  >>  T_Ident + T_LParen + Args + T_RParen);

  // Args
  AddRule(Rule(0, Args)     >> Args + T_Comma + Arg);
  AddRule(Rule(1, Args)     >> Arg);

  // Arg
  AddRule(Rule(0, Arg)      >> Expr);
  AddRule(Rule(1, Arg)      >> T_Ident + T_Equal + Expr);
  AddRule(Rule(2, Arg)      >> List);

  // PathSpec
  AddRule(Rule(0, PathSpec) >> PathSpec + MultiSlash + T_Ident);
  AddRule(Rule(1, PathSpec) >> MultiSlash + T_Ident);
  AddRule(Rule(2, PathSpec) >> T_Ident);

  // MultiSlash
  AddRule(Rule(0, MultiSlash) >> MultiSlash + T_Slash);
  AddRule(Rule(1, MultiSlash) >> T_Slash);

  // Variable
  AddRule(Rule(0, Variable) >> T_Ident );
  AddRule(Rule(1, Variable) >> T_LCarat + PathSpec + T_RCarat );
  AddRule(Rule(2, Variable) >> T_LCarat + DBSpec + T_Colon + PathSpec + T_RCarat );

  // Database
  AddRule(Rule(0, Database) >> T_LCarat + DBSpec + T_Colon + T_RCarat );

  // DBSpec
  AddRule(Rule(0, DBSpec)   >> PathSpec );
  AddRule(Rule(1, DBSpec)   >> PathSpec + MachSpec);
  AddRule(Rule(2, DBSpec)   >> TimeSpec );
  AddRule(Rule(3, DBSpec)   >> PathSpec + TimeSpec );
  AddRule(Rule(4, DBSpec)   >> PathSpec + MachSpec + TimeSpec );

  // MachSpec
  AddRule(Rule(0, MachSpec) >> T_At + T_Ident );

  // TimeSpec
  AddRule(Rule(0, TimeSpec) >> T_LBracket + ListElems + T_RBracket + T_Ident);
  AddRule(Rule(1, TimeSpec) >> T_LBracket + ListElems + T_RBracket);
  AddRule(Rule(2, TimeSpec) >> T_LBracket + T_Pound + ListElems + T_RBracket);

  // ----------------------------------------------------------------------

  // -- associativity --
  SetAssoc(T_Mod,   Grammar::NonAssoc);
  SetAssoc(T_Plus,  Grammar::Left);
  SetAssoc(T_Minus, Grammar::Left);
  SetAssoc(T_Mult,  Grammar::Left);
  SetAssoc(T_Slash, Grammar::Left);
  SetAssoc(T_Exp,   Grammar::Right);

  // -- precedence --
  SetPrec(T_Mod,    1);
  SetPrec(T_Plus,   2);
  SetPrec(T_Minus,  2);
  SetPrec(T_Mult,   3);
  SetPrec(T_Slash,  3);
  SetPrec(T_Exp,    4);
  // Unary minus is 5  -- see rule above
  SetPrec(T_LBracket, 6);
}

// ****************************************************************************
//  Destructor:  ExprGrammar::~ExprGrammar
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ExprGrammar::~ExprGrammar()
{
}
