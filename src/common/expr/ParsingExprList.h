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

#ifndef PARSING_EXPR_LIST_H
#define PARSING_EXPR_LIST_H
#include <expr_exports.h>
#include <Expression.h>
#include <ExpressionList.h>
#include <SimpleObserver.h>
#include <VisItParser.h>
#include <string> 

// Forward declarations.
class ExprNode;

// ****************************************************************************
// Class: ParsingExprList
//
// Purpose:
//   This class contains a list of expressions, and the ability to parse them.
//
// Notes:      
//
// Programmer: Sean Ahern
// Creation:   Tue Sep 25 14:17:12 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Fri Aug 15 11:33:25 PDT 2003
//    Added a new form of GetExpressionTree that takes an Expression.
//
//    Jeremy Meredith, Wed Nov 24 12:27:15 PST 2004
//    Refactored and changed parse tree classes around.
//
//    Jeremy Meredith, Tue Dec 14 14:09:51 PST 2004
//    I added code to get the first real (database, non-expression) variable
//    for a given variable (which may or may not be an expression).
//
//    Hank Childs, Mon Jan  8 10:35:38 PST 2007
//    Added GetExpressionTypeFromAVT.
//
//    Brad Whitlock, Tue Jan 20 15:49:56 PST 2009
//    I removed some methods that caused AVT to bleed into this library.
//
// ****************************************************************************

class EXPR_API ParsingExprList : public SimpleObserver
{
public:
    ParsingExprList();
    ParsingExprList(Parser *p);
    ~ParsingExprList();

    static ParsingExprList *Instance();
    static Expression* GetExpression(const char *varname);
    static Expression* GetExpression(std::string varname)
        { return GetExpression(varname.c_str()); }

    ExpressionList *GetList() {return &exprList;}

    virtual void Update(Subject *);

    static ExprNode *GetExpressionTree(Expression*);
    static ExprNode *GetExpressionTree(const char *varname);
    static ExprNode *GetExpressionTree(std::string varname)
        { return GetExpressionTree(varname.c_str()); }

    static std::string GetRealVariable(const std::string &var);

protected:

    Parser* GetParser() { return parser; }

private:
    ExpressionList exprList;
    Parser *parser;

    static ParsingExprList *instance;
};

#endif
