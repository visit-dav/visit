/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <ViewerVariableMethods.h>

#include <ViewerFileServerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerText.h>

#include <avtDatabaseMetaData.h>
#include <avtExpressionTypeConversions.h>
#include <DebugStream.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <ParsingExprList.h>
#include <VisItException.h>

// ****************************************************************************
// Method: ViewerVariableMethods::ViewerVariableMethods
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 16:48:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerVariableMethods::ViewerVariableMethods()
{
}

// ****************************************************************************
// Method: ViewerVariableMethods::~ViewerVariableMethods
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 16:48:45 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerVariableMethods::~ViewerVariableMethods()
{
}

// ****************************************************************************
// Method: ViewerVariableMethods::DetermineVarType
//
// Purpose: 
//   Determines the AVT variable type for the specified variable.
//
// Arguments:
//   host  : The host where the file resides.
//   db    : The database.
//   var   : The variable that we want.
//   state : The state at which we want information about the variable.
//
// Returns:    
//
// Note:       This code was moved out of ViewerPlot and ViewerQueryManager
//             since the code in those two classes was identical. Normally
//             code like this would not be here but it seems like a fairly
//             convenient place since we have no base class for viewer objects.
//
//             We must pass the state because of variables that can exist
//             at some time states and not at others.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 26 10:18:45 PDT 2004
//
// Modifications:
//   Brad Whitlock, Fri Feb 18 10:00:35 PDT 2005
//   Changed the code so it uses an expression list that is correct for the
//   specified database instead of just assuming that ParsingExprList will
//   contain the right expression list. Note that someday when we have support
//   for expressions from multiple databases, and ParsingExprList contains
//   the list of expressions from all open sources then it will be okay to
//   use ParsingExprList again.
//
//   Brad Whitlock, Tue Apr 29 15:04:13 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

avtVarType
ViewerVariableMethods::DetermineVarType(const std::string &host,
    const std::string &db, const std::string &var, int state)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check if the variable is an expression.
    ExpressionList expressionList;
    GetAllExpressions(expressionList, host, db, state);
    Expression *exp = expressionList[var.c_str()];

    if (exp != NULL)
    {
        retval = ExprType_To_avtVarType(exp->GetType());
    }
    else
    {
        const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaDataForState(host, db, state);
        if (md != 0)
        {
            // 
            // Get the type for the variable.
            //
            TRY
            {
                avtDatabaseMetaData *ncmd = const_cast<avtDatabaseMetaData *>(md);
                retval = ncmd->DetermineVarType(var);
            }
            CATCH(VisItException)
            {
                GetViewerMessaging()->Error(
                    TR("VisIt was unable to determine the variable type for "
                       "%1:%2's %3 variable.").
                    arg(host).
                    arg(db).
                    arg(var));

                debug1 << "ViewerVariableMethods::DetermineVarType: Caught an "
                          "exception!" << endl;
                retval = AVT_UNKNOWN_TYPE;
            }
            ENDTRY
        }
    }

    return retval;
}


// ****************************************************************************
// Method: ViewerVariableMethods::DetermineRealVarType
//
// Purpose: 
//   Determines the AVT variable type for the 'real' variable behind
//   the specified variable.
//
// Arguments:
//   host  : The host where the file resides.
//   db    : The database.
//   var   : The variable that we want.
//   state : The state at which we want information about the variable.
//
// Returns:    
//
// Programmer: Kathleen Bonnell 
// Creation:   July 5, 2005 
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 15:05:55 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

avtVarType
ViewerVariableMethods::DetermineRealVarType(const std::string &host,
    const std::string &db, const std::string &var, int state)
{
    avtVarType retval = AVT_UNKNOWN_TYPE;

    // Check if the variable is an expression.
    ExpressionList expressionList;
    GetAllExpressions(expressionList, host, db, state);
    Expression *exp = expressionList[var.c_str()];
    std::string realVar = var;
    if (exp != NULL)
    {
        std::string realVar = ParsingExprList::GetRealVariable(var);
        if (realVar != var)
            exp = NULL;
    }
    if (exp != NULL)
    {
        retval = ExprType_To_avtVarType(exp->GetType());
    }
    else
    {
        const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaDataForState(host, db, state);
        if (md != 0)
        {
            // 
            // Get the type for the variable.
            //
            TRY
            {
                avtDatabaseMetaData *ncmd = const_cast<avtDatabaseMetaData *>(md);
                retval = ncmd->DetermineVarType(realVar);
            }
            CATCH(VisItException)
            {
                GetViewerMessaging()->Error(
                    TR("VisIt was unable to determine the real variable type "
                       "for %1:%2's %3 variable.").
                    arg(host).
                    arg(db).
                    arg(var));

                debug1 << "ViewerVariableMethods::DetermineVarType: Caught an "
                          "exception!" << endl;
                retval = AVT_UNKNOWN_TYPE;
            }
            ENDTRY
        }
    }

    return retval;
}


// ****************************************************************************
// Method: ViewerVariableMethods::GetUserExpressions
//
// Purpose: 
//   Gets the expressions that were defined by the user.
//
// Arguments:
//   newList : The return list for the user-defined expressions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 09:44:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerVariableMethods::GetUserExpressions(ExpressionList &newList)
{
    ExpressionList *exprList = ParsingExprList::Instance()->GetList();

    //
    // Create a new expression list that contains all of the expressions
    // from the main expression list that are not expressions that come
    // from databases.
    //
    for(int i = 0; i < exprList->GetNumExpressions(); ++i)
    {
        const Expression &expr = exprList->GetExpressions(i);
        if(!expr.GetFromDB())
            newList.AddExpressions(expr);
    }
}

// ****************************************************************************
// Method: ViewerVariableMethods::GetDatabaseExpressions
//
// Purpose: 
//   Gets the database expressions for the specified database.
//
// Arguments:
//   newList : The return list for the database expressions.
//   host    : The host where the database is located.
//   db      : The database name.
//   state   : The database time state.
//
// Returns:    The list of database expressions.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 09:45:27 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Aug 19 11:06:39 PDT 2011
//   If ANY_STATE was passed for the state then call a different metadata
//   function.
//
// ****************************************************************************

void
ViewerVariableMethods::GetDatabaseExpressions(ExpressionList &newList,
    const std::string &host, const std::string &db, int state)
{
    // Store all of the specified database's expressions in the 
    // new list.
    if(host.size() > 0 && db.size() > 0)
    {
        const avtDatabaseMetaData *md = 0;
        if(state == ViewerFileServerInterface::ANY_STATE)
            md = GetViewerFileServer()->GetMetaData(host, db);
        else
            md = GetViewerFileServer()->GetMetaDataForState(host, db, state);
        if (md != 0)
        {
            // Add the expressions for the database.
            for (int j = 0 ; j < md->GetNumberOfExpressions(); ++j)
                newList.AddExpressions(*(md->GetExpression(j)));
        }
    }
}

// ****************************************************************************
// Method: ViewerVariableMethods::GetAllExpressions
//
// Purpose: 
//   Gets user-defined expressions and the expressions for the specified
//   database.
//
// Arguments:
//   newList : The return list for the database expressions.
//   host    : The host where the database is located.
//   db      : The database name.
//   state   : The database time state.
//
// Returns:    A list of expressions.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 18 09:46:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerVariableMethods::GetAllExpressions(ExpressionList &newList,
    const std::string &host, const std::string &db, int state)
{
    // Store all of the expressions that are not database expressions
    // in the new list.
    GetUserExpressions(newList);
    GetDatabaseExpressions(newList, host, db, state);
}
