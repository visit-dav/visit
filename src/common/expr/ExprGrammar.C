/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
//    Brad Whitlock, Fri Oct 6 11:49:02 PDT 2006
//    I made the symbols be static in the ExprGrammar class so there are no
//    conflicts with Python 2.5.
//

Dictionary D;

// ------------------------------------------------------------------------
// TERMINALS
// ------------------------------------------------------------------------
Symbol ExprGrammar::T_Plus          (D, '+');
Symbol ExprGrammar::T_Mult          (D, '*');
Symbol ExprGrammar::T_Minus         (D, '-');
Symbol ExprGrammar::T_Slash         (D, '/');
Symbol ExprGrammar::T_Exp           (D, '^');
Symbol ExprGrammar::T_Mod           (D, '%');
Symbol ExprGrammar::T_LBracket      (D, '[');
Symbol ExprGrammar::T_RBracket      (D, ']');
Symbol ExprGrammar::T_LParen        (D, '(');
Symbol ExprGrammar::T_RParen        (D, ')');
Symbol ExprGrammar::T_LCarat        (D, '<');
Symbol ExprGrammar::T_RCarat        (D, '>');
Symbol ExprGrammar::T_LBrace        (D, '{');
Symbol ExprGrammar::T_RBrace        (D, '}');
Symbol ExprGrammar::T_Pound         (D, '#');
Symbol ExprGrammar::T_Equal         (D, '=');
Symbol ExprGrammar::T_At            (D, '@');
Symbol ExprGrammar::T_Comma         (D, ',');
Symbol ExprGrammar::T_Colon         (D, ':');
Symbol ExprGrammar::T_Ident         (D, TT_Identifier,   "Identifier");
Symbol ExprGrammar::T_Integer       (D, TT_IntegerConst, "Integer");
Symbol ExprGrammar::T_Float         (D, TT_FloatConst,   "Float");
Symbol ExprGrammar::T_String        (D, TT_StringConst,  "String");
Symbol ExprGrammar::T_Bool          (D, TT_BoolConst,    "Bool");


// ------------------------------------------------------------------------
// NON-TERMINALS
// ------------------------------------------------------------------------
Symbol ExprGrammar::Expr            (D,  "Expr"         );
Symbol ExprGrammar::Constant        (D,  "Constant"     );
Symbol ExprGrammar::Vector          (D,  "Vector"       );
Symbol ExprGrammar::List            (D,  "List"         );
Symbol ExprGrammar::ListElems       (D,  "ListElems"    );
Symbol ExprGrammar::ListElem        (D,  "ListElem"     );
Symbol ExprGrammar::Function        (D,  "Function"     );
Symbol ExprGrammar::Args            (D,  "Args"         );
Symbol ExprGrammar::Arg             (D,  "Arg"          );
Symbol ExprGrammar::PathSpec        (D,  "PathSpec"     );
Symbol ExprGrammar::MultiSlash      (D,  "MultiSlash"   );
Symbol ExprGrammar::Variable        (D,  "Variable"     );
Symbol ExprGrammar::Database        (D,  "Database"     );
Symbol ExprGrammar::DBSpec          (D,  "DBSpec"       );
Symbol ExprGrammar::PathMachSpec    (D,  "PathMachSpec" );
Symbol ExprGrammar::MachSpec        (D,  "MachSpec"     );
Symbol ExprGrammar::TimeSpec        (D,  "TimeSpec"     );


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
