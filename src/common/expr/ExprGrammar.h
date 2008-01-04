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
