#include "ExprGrammar.h"

// ------------------------------------------------------------------------
// TERMINALS
// ------------------------------------------------------------------------
Symbol T_Plus          ('+');
Symbol T_Mult          ('*');
Symbol T_Minus         ('-');
Symbol T_Slash         ('/');
Symbol T_Exp           ('^');
Symbol T_Mod           ('%');
Symbol T_LBracket      ('[');
Symbol T_RBracket      (']');
Symbol T_LParen        ('(');
Symbol T_RParen        (')');
Symbol T_LCarat        ('<');
Symbol T_RCarat        ('>');
Symbol T_LBrace        ('{');
Symbol T_RBrace        ('}');
Symbol T_Pound         ('#');
Symbol T_Equal         ('=');
Symbol T_At            ('@');
Symbol T_Comma         (',');
Symbol T_Colon         (':');
Symbol T_Ident         (TT_Identifier);
Symbol T_Integer       (TT_IntegerConst);
Symbol T_Float         (TT_FloatConst);
Symbol T_String        (TT_StringConst);
Symbol T_Bool          (TT_BoolConst);


// ------------------------------------------------------------------------
// NON-TERMINALS
// ------------------------------------------------------------------------
Symbol Expr            ( "Expr"         );
Symbol Constant        ( "Constant"     );
Symbol Vector          ( "Vector"       );
Symbol List            ( "List"         );
Symbol ListElems       ( "ListElems"    );
Symbol ListElem        ( "ListElem"     );
Symbol Function        ( "Function"     );
Symbol Args            ( "Args"         );
Symbol Arg             ( "Arg"          );
Symbol PathSpec        ( "PathSpec"     );
Symbol MultiSlash      ( "MultiSlash"   );
Symbol Variable        ( "Variable"     );
Symbol Database        ( "Database"     );
Symbol DBSpec          ( "DBSpec"       );
Symbol PathMachSpec    ( "PathMachSpec" );
Symbol MachSpec        ( "MachSpec"     );
Symbol TimeSpec        ( "TimeSpec"     );


// ****************************************************************************
//  Constructor:  ExprGrammar::ExprGrammar
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
// ****************************************************************************
ExprGrammar::ExprGrammar() : Grammar()
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
  AddRule(Rule(11, Expr)  >>  List     );
  AddRule(Rule(12, Expr)  >>  Function );
  AddRule(Rule(13, Expr)  >>  Variable );
  AddRule(Rule(14, Expr)  >>  Database );

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
