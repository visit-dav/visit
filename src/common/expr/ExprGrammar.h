#ifndef EXPRGRAMMAR_H
#define EXPRGRAMMAR_H
#include <expr_exports.h>

#include "Grammar.h"

// ****************************************************************************
//  Class:  ExprGrammar
//
//  Purpose:
//    Implementation of a Grammar for the VisIt expression language.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2002
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 24 11:45:56 PST 2004
//    Refactored the expression-specific stuff to its own library.
//
// ****************************************************************************
class EXPR_API ExprGrammar : public Grammar
{
  public:
    ExprGrammar();
    ~ExprGrammar();
    bool Initialize();
};

// ------------------------------------------------------------------------
// TERMINALS
// ------------------------------------------------------------------------
extern Symbol T_Plus;
extern Symbol T_Mult;
extern Symbol T_Minus;
extern Symbol T_Slash;
extern Symbol T_Exp;
extern Symbol T_Mod;
extern Symbol T_LBracket;
extern Symbol T_RBracket;
extern Symbol T_LParen;
extern Symbol T_RParen;
extern Symbol T_LCarat;
extern Symbol T_RCarat;
extern Symbol T_LBrace;
extern Symbol T_RBrace;
extern Symbol T_Pound;
extern Symbol T_Equal;
extern Symbol T_At;
extern Symbol T_Comma;
extern Symbol T_Colon;
extern Symbol T_Ident;
extern Symbol T_Integer;
extern Symbol T_Float;
extern Symbol T_String;
extern Symbol T_Bool;


// ------------------------------------------------------------------------
// NON-TERMINALS
// ------------------------------------------------------------------------
extern EXPR_API Symbol Expr;
extern EXPR_API Symbol Constant;
extern EXPR_API Symbol Vector;
extern EXPR_API Symbol List;
extern EXPR_API Symbol ListElems;
extern EXPR_API Symbol ListElem;
extern EXPR_API Symbol Function;
extern EXPR_API Symbol Args;
extern EXPR_API Symbol Arg;
extern EXPR_API Symbol PathSpec;
extern EXPR_API Symbol MultiSlash;
extern EXPR_API Symbol Variable;
extern EXPR_API Symbol Database;
extern EXPR_API Symbol DBSpec;
extern EXPR_API Symbol PathMachSpec;
extern EXPR_API Symbol MachSpec;
extern EXPR_API Symbol TimeSpec;

#endif
