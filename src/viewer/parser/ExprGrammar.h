#ifndef EXPRGRAMMAR_H
#define EXPRGRAMMAR_H
#include <viewer_parser_exports.h>

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
// ****************************************************************************
class VIEWER_PARSER_API ExprGrammar : public Grammar
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
extern VIEWER_PARSER_API Symbol Expr;
extern VIEWER_PARSER_API Symbol Constant;
extern VIEWER_PARSER_API Symbol Vector;
extern VIEWER_PARSER_API Symbol List;
extern VIEWER_PARSER_API Symbol ListElems;
extern VIEWER_PARSER_API Symbol ListElem;
extern VIEWER_PARSER_API Symbol Function;
extern VIEWER_PARSER_API Symbol Args;
extern VIEWER_PARSER_API Symbol Arg;
extern VIEWER_PARSER_API Symbol PathSpec;
extern VIEWER_PARSER_API Symbol MultiSlash;
extern VIEWER_PARSER_API Symbol Variable;
extern VIEWER_PARSER_API Symbol Database;
extern VIEWER_PARSER_API Symbol DBSpec;
extern VIEWER_PARSER_API Symbol PathMachSpec;
extern VIEWER_PARSER_API Symbol MachSpec;
extern VIEWER_PARSER_API Symbol TimeSpec;

#endif
