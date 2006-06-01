/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
