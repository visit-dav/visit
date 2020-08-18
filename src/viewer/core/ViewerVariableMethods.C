// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerVariableMethods.h>

#include <ViewerFileServerInterface.h>
#include <ViewerMessaging.h>
#include <ViewerText.h>

#include <avtDatabaseMetaData.h>
#include <avtExpressionTypeConversions.h>
#include <DebugStream.h>
#include <VisItEnv.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
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
    Expression const *exp = expressionList[var.c_str()];

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
    Expression const *exp = expressionList[var.c_str()];
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
    ExpressionList const *exprList = ParsingExprList::Instance()->GetList();

    //
    // Create a new expression list that contains all of the expressions
    // from the main expression list that are not expressions that come
    // from databases.
    //
    for(int i = 0; i < exprList->GetNumExpressions(); ++i)
    {
        const Expression &expr = exprList->GetExpressions(i);
        if(!expr.GetFromDB() && !expr.GetFromOperator())
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
    if(!host.empty() && !db.empty())
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
// Method: ViewerVariableMethods::GetOperatorCreatedExpressions
//
// Purpose: 
//   Gets the operator-created expressions for the specified database.
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
//
//   Mark C. Miller, Thu Jun  8 15:03:27 PDT 2017
//   Disable if speculative expression generation (SEG) is disabled.
// ****************************************************************************

void
ViewerVariableMethods::GetOperatorCreatedExpressions(ExpressionList &newList,
    const std::string &host, const std::string &db, int state)
{
    // Store all of the specified database's expressions in the 
    // new list.
    if(!host.empty() && !db.empty())
    {
        const avtDatabaseMetaData *md = 0;
        if(state == ViewerFileServerInterface::ANY_STATE)
            md = GetViewerFileServer()->GetMetaData(host, db);
        else
            md = GetViewerFileServer()->GetMetaDataForState(host, db, state);

        if (!md) return;

        if (md->ShouldDisableSEG(VisItEnv::exists(md->GetSEGEnvVarName())))
            return;

        // Start with the user's expressions and this database's expressions.
        ExpressionList userAndDB;
        GetUserExpressions(userAndDB);
        for (int i = 0 ; i < md->GetNumberOfExpressions(); ++i)
            userAndDB.AddExpressions(*(md->GetExpression(i)));

        // We'll make operator-created expressions for all user and db expressions.
        // Note that we only append the operator-created expressions to the return.
        avtDatabaseMetaData md2 = *md;
        md2.GetExprList() = userAndDB;
        for(int j = 0; j < GetOperatorPluginManager()->GetNEnabledPlugins(); j++)
        {
            std::string id(GetOperatorPluginManager()->GetEnabledID(j));
            CommonOperatorPluginInfo *ComInfo = GetOperatorPluginManager()->GetCommonPluginInfo(id);
            ExpressionList const *fromOperators = ComInfo->GetCreatedExpressions(&md2);
            if(fromOperators != NULL)
            {
                for(int k = 0; k < fromOperators->GetNumExpressions(); k++)
                    newList.AddExpressions(fromOperators->GetExpressions(k));
                delete fromOperators;
            }
        }
    }
}

// ****************************************************************************
// Method: ViewerVariableMethods::GetAllExpressions
//
// Purpose: 
//   Gets user-defined expressions, the expressions for the specified
//   database, and the operator-created expressions .
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
    // Store all of the expressions in the new list.
    GetUserExpressions(newList);
    GetDatabaseExpressions(newList, host, db, state);
    GetOperatorCreatedExpressions(newList, host, db, state);
}
