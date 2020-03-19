// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Brad Whitlock, Fri Oct 6 11:49:02 PDT 2006
//    I made the symbols be static in the ExprGrammar class so there are no
//    conflicts with Python 2.5.
//
//    Jeremy Meredith, Wed Jul 23 13:29:06 EDT 2008
//    Added a backslash symbol.  We use it both for escaping and as
//    a path separator.
//
// ****************************************************************************
class EXPR_API ExprGrammar : public Grammar
{
  public:
    ExprGrammar();
    ~ExprGrammar();
    bool Initialize();

    // ------------------------------------------------------------------------
    // TERMINALS
    // ------------------------------------------------------------------------
    static Symbol T_Plus;
    static Symbol T_Mult;
    static Symbol T_Minus;
    static Symbol T_Slash;
    static Symbol T_BackSlash;
    static Symbol T_Exp;
    static Symbol T_Mod;
    static Symbol T_LBracket;
    static Symbol T_RBracket;
    static Symbol T_LParen;
    static Symbol T_RParen;
    static Symbol T_LCarat;
    static Symbol T_RCarat;
    static Symbol T_LBrace;
    static Symbol T_RBrace;
    static Symbol T_Pound;
    static Symbol T_Equal;
    static Symbol T_At;
    static Symbol T_Comma;
    static Symbol T_Colon;
    static Symbol T_Ident;
    static Symbol T_Integer;
    static Symbol T_Float;
    static Symbol T_String;
    static Symbol T_Bool;
    static Symbol T_And;

    // ------------------------------------------------------------------------
    // NON-TERMINALS
    // ------------------------------------------------------------------------
    static Symbol Expr;
    static Symbol Constant;
    static Symbol Vector;
    static Symbol List;
    static Symbol ListElems;
    static Symbol ListElem;
    static Symbol Function;
    static Symbol Args;
    static Symbol Arg;
    static Symbol PathSpec;
    static Symbol MultiSlash;
    static Symbol Variable;
    static Symbol Database;
    static Symbol DBSpec;
    static Symbol PathMachSpec;
    static Symbol MachSpec;
    static Symbol TimeSpec;
};

#endif
