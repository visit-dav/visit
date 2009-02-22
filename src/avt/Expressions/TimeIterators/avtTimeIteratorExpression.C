/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                        avtTimeIteratorExpression.C                        //
// ************************************************************************* //

#include <avtTimeIteratorExpression.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtExprNode.h>
#include <avtExpressionEvaluatorFilter.h>

#include <Expression.h>
#include <ParsingExprList.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <InvalidFilesException.h>

#include <snprintf.h>


// ****************************************************************************
//  Method: avtTimeIteratorExpression constructor
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2009
//
// ****************************************************************************

avtTimeIteratorExpression::avtTimeIteratorExpression()
{
    firstTimeSlice = 0;
    lastTimeSlice = -1;
    timeStride    = 1;
    numTimeSlicesToProcess = -1;
    cmfeType = CONN_CMFE;
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression destructor
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2009
//
// ****************************************************************************

avtTimeIteratorExpression::~avtTimeIteratorExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::ProcessArguments
//
//  Purpose:
//      Parses arguments
//
//  Arguments:
//      args      Expression arguments
//      state     Expression pipeline state
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

void
avtTimeIteratorExpression::ProcessArguments(ArgsExpr *args,
                                            ExprPipelineState *state)
{
    int  i;

    // get the argument list and # of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    // check for call with no args
    if (nargs < NumberOfVariables())
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "Not enough arguments to time iterator expression");
    }

    // Each of these arguments should be variables.
    for (i = 0 ; i < NumberOfVariables() ; i++)
    {
        ArgExpr *arg = (*arguments)[i];
        avtExprNode *tree = dynamic_cast<avtExprNode*>(arg->GetExpr());
        tree->CreateFilters(state);
    }

    vector<int> timecontrols;
    for (i = NumberOfVariables() ; i < nargs ; i++)
    {
        ArgExpr *arg= (*arguments)[i];
        ExprParseTreeNode *tree= arg->GetExpr();
        string argtype = tree->GetTypeName();
        // check for arg passed as integer
        if (argtype == "IntegerConst")
        {
            int t = dynamic_cast<IntegerConstExpr*>(tree)->GetValue();
            timecontrols.push_back(t);
        }
        else if (argtype == "StringConst")
        {
            std::string s = dynamic_cast<StringConstExpr*>(tree)->GetValue();
            if (s == "pos_cmfe")
            {
                if (i == nargs-1)
                {
                    EXCEPTION2(ExpressionException, outputVariableName,
                               "If specify pos_cmfe, then you must follow that"
                               " immediately with a default variable.");
                }
                i++;
                ArgExpr *arg = (*arguments)[i];
                avtExprNode *tree = dynamic_cast<avtExprNode*>(arg->GetExpr());
                tree->CreateFilters(state);
                cmfeType = POS_CMFE;
            }
            else if (s != "conn_cmfe")
            {
                char msg[1024];
                SNPRINTF(msg, 1024, "Could not understand meaning of \"%s\"",
                                    s.c_str());
                EXCEPTION2(ExpressionException, outputVariableName,msg);
            }
        }
    }
    if (timecontrols.size() == 3)
    {
        firstTimeSlice = timecontrols[0];
        lastTimeSlice  = timecontrols[1];
        timeStride     = timecontrols[2];
    }
    else if (timecontrols.size() != 0)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The specification of time must use integers.  "
                   "These integers are dump indices, 0-indexed."
                   "If you specify time controls, there must be three integers"
                   ", start, stop, stride");
    }
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::AddInputVariableName
//
//  Purpose:
//      Adds to the list of input variables
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

void
avtTimeIteratorExpression::AddInputVariableName(const char *v)
{
    if (varnames.size() < NumberOfVariables())
        varnames.push_back(v);
    else
    {
        if (varnames.size() == NumberOfVariables() && cmfeType == POS_CMFE)
        {
            // Need to remove quotes around the variable (if any), since that 
            // won't be parsed well.
            std::string var_wo_quotes = std::string(v);
            for (int j = var_wo_quotes.size()-1 ; j >= 0 ; j--)
                if (var_wo_quotes[j] == '\'')
                    var_wo_quotes.replace(j,1,"");
            varnames.push_back(var_wo_quotes);
        }
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName,
                   "An internal error occurred.");
        }
    }
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::ModifyContract
//
//  Purpose:
//      This method is intended to modify the contract, but we are going to 
//      just make a copy of it for later use.
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

avtContract_p
avtTimeIteratorExpression::ModifyContract(avtContract_p c)
{
    avtContract_p c2 = avtExpressionFilter::ModifyContract(c);
    executionContract = new avtContract(c2);
    return c2;
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::Execute
//
//  Purpose:
//      Iterates over time and hands results over to derived types who create
//      the final output.
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

void
avtTimeIteratorExpression::Execute(void)
{
    FinalizeTimeLoop();
    avtContract_p contract = ConstructContractWithVarnames();

    // Store off the original expression list.
    ParsingExprList *pel = ParsingExprList::Instance();
    ExpressionList orig_list = *(pel->GetList());

    InitializeOutput();

    std::string db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
    ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
    if (*dbp == NULL)
        EXCEPTION1(InvalidFilesException, db.c_str());

    // The first EEF already set up its expressions ... we need a new one
    // to set up filters for the CMFE expressions.
    avtExpressionEvaluatorFilter myeef;
    myeef.SetInput(GetInput());
    for (int i = 0 ; i < numTimeSlicesToProcess ; i++)
    {
        int timeSlice = firstTimeSlice + i*timeStride;
        if (timeSlice > actualLastTimeSlice)
            timeSlice = actualLastTimeSlice;
        debug1 << "Time iterating expression working on time slice " 
               << timeSlice << endl;
cerr << "WORKING ON TIME " << timeSlice << endl;
        UpdateExpressions(timeSlice);
        // won't re-execute without setting modified to true, because
        // it doesn't check to see if expression definitions change.
        myeef.ReleaseData();
        myeef.GetOutput()->Update(contract);

        avtDatabaseMetaData *md = dbp->GetMetaData(timeSlice, false,
                                                   false, false);
        currentCycle = md->GetCycles()[timeSlice];
        currentTime  = md->GetTimes()[timeSlice];
        ProcessDataTree(myeef.GetTypedOutput()->GetDataTree(), i);
        debug1 << "Time iterating expression done working on time slice " 
               << timeSlice << endl;
    }

    // Get the upstream filters back the way they are supposed to be.
    GetInput()->Update(executionContract);

    FinalizeOutput();

    // Restore the original expression list ... i.e. undo the temporary
    // expressions we put in.
    *(pel->GetList()) = orig_list;
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::UpdateExpressions
//
//  Purpose:
//      Creates expressions that will read the current time slice.
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

void
avtTimeIteratorExpression::UpdateExpressions(int ts)
{
    ParsingExprList *pel = ParsingExprList::Instance();
    ExpressionList new_list = *(pel->GetList());

    int nvars = varnames.size();
    if (cmfeType == POS_CMFE)
        nvars--;
    for (int i = 0 ; i < nvars ; i++)
    {
        // No new expression has to be made in this case.
        if (VariableComesFromCurrentTime(i))
            continue;

        std::string meshname = 
                           GetInput()->GetInfo().GetAttributes().GetMeshname();
        char expr_defn[1024];
        if (cmfeType == CONN_CMFE)
        {
            SNPRINTF(expr_defn, 1024, "conn_cmfe(<[%d]i:%s>, %s)", ts,
                                        varnames[i].c_str(), meshname.c_str());
        }
        else
        {
            int defVarIndex = varnames.size()-1;
            SNPRINTF(expr_defn, 1024, "pos_cmfe(<[%d]i:%s>, %s, %s)", ts,
                                        varnames[i].c_str(), meshname.c_str(),
                                        varnames[defVarIndex].c_str());
        }

        std::string exp_name = GetInternalVarname(i);
        
        bool alreadyInList = false;
cerr << "DEFINITION = " << expr_defn << endl;
        for (int j = 0 ; j < new_list.GetNumExpressions() ; j++)
        {
            if (new_list[j].GetName() == exp_name)
            {
                alreadyInList = true;
                new_list[j].SetDefinition(expr_defn);
            }
        }
        if (!alreadyInList)
        {
            Expression exp;
            exp.SetName(exp_name);
            exp.SetDefinition(expr_defn);
            exp.SetType(Expression::Unknown);
            new_list.AddExpressions(exp);
        }
    }

    *(pel->GetList()) = new_list;
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::GetInternalVarname
//
//  Purpose:
//      We use our own internal names for the variables.  This method generates
//      those names.
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

std::string
avtTimeIteratorExpression::GetInternalVarname(int v)
{
    if (VariableComesFromCurrentTime(v))
        return varnames[v];

    char name[1024];
    SNPRINTF(name, 1024, "_avt_ti_%d", v);
    return std::string(name);
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::ConstructContractWithVarnames
//
//  Purpose:
//      The current contract has all sorts of variables we don't need.  We
//      only need the variables indicated by the "varnames" array.  Make
//      sure those are the only ones read.
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

avtContract_p
avtTimeIteratorExpression::ConstructContractWithVarnames(void)
{
    avtDataRequest_p old_dr = executionContract->GetDataRequest();
    std::string varname = GetInternalVarname(0);
    avtDataRequest_p new_dr = new avtDataRequest(old_dr, varname.c_str());
    new_dr->RemoveAllSecondaryVariables();
    for (int i = 1 ; i < NumberOfVariables() ; i++)
    {
        std::string varname = GetInternalVarname(i);
        new_dr->AddSecondaryVariable(varname.c_str());
    }

    avtContract_p rv = new avtContract(executionContract, new_dr);
    return rv;
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::FinalizeTimeLoop
//
//  Purpose:
//      See how many time slices there are and make sure we don't walk off the
//      end of the array, etc.
//
//  Notes:      This routine is taken almost verbatim from Kathleen Bonnell's
//              implementation in avtTimeLoopFilter (only variable names are
//              changed).  I wish I could have found a refactoring that would
//              allow me to use her routine directly, but I couldn't.  --HRC
//
//  Programmer:   Hank Childs
//  Creation:     February 14, 2009
//
// ****************************************************************************

void
avtTimeIteratorExpression::FinalizeTimeLoop()
{
    int numStates = GetInput()->GetInfo().GetAttributes().GetNumStates();
    if (firstTimeSlice < 0)
    {
        firstTimeSlice = 0;
    }
    if (lastTimeSlice < 0)
    {
        lastTimeSlice = numStates - 1;
    }
    if (timeStride < 0)
    {
        timeStride = 1;
    }
    if (firstTimeSlice >= lastTimeSlice)
    {
        std::string msg("Start time must be smaller than end time for " );
        msg += GetType();
        msg += ".\n";
        EXCEPTION1(ImproperUseException, msg);
    }

    numTimeSlicesToProcess = (lastTimeSlice-firstTimeSlice)/timeStride+1;

    if (lastTimeSlice >= numStates)
    {
        std::string msg(GetType());
        msg += ":  Clamping end time to number of available timesteps.";
        avtCallback::IssueWarning(msg.c_str());
    }

    //
    // Ensure that the specified lastTimeSlice is included,
    // regardless of the timeStride.
    //
    actualLastTimeSlice = firstTimeSlice + (numTimeSlicesToProcess-1)*timeStride;
    if (actualLastTimeSlice < lastTimeSlice)
    {
        numTimeSlicesToProcess++;
        actualLastTimeSlice = lastTimeSlice;
    }
}


// ****************************************************************************
//  Method: avtTimeIteratorExpression::IsPointVariable
//
//  Purpose:
//      Determines if we have a point variable or a cell variable.  Does this
//      by looking at centering of inputs.  If they are mixed, then it declares
//      them to be cell variables.
//
//  Programmer: Hank Childs
//  Creation:   February 16, 2009
//
// ****************************************************************************

bool
avtTimeIteratorExpression::IsPointVariable(void)
{
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    bool hasNodal = false;
    bool hasZonal = false;
    for (int i = 0 ; i < varnames.size() ; i++)
    {
        if (!atts.ValidVariable(varnames[i]))
            return avtExpressionFilter::IsPointVariable();

        if (atts.GetCentering(varnames[i].c_str()) == AVT_ZONECENT)
            hasZonal = true;
        else
            hasNodal = true;
    }

    if (hasZonal && hasNodal)
        return false;

    return hasNodal;
}


