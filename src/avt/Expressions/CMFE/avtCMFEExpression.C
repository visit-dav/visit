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

// ************************************************************************* //
//                             avtCMFEExpression.C                           //
// ************************************************************************* //

#include <avtCMFEExpression.h>

#include <math.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedIntArray.h>

#include <avtExprNode.h>
#include <ExprToken.h>
#include <ParsingExprList.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtMetaData.h>
#include <avtOriginatingSource.h>

#include <DebugStream.h>
#include <ExpressionException.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


// ****************************************************************************
//  Method: avtCMFEExpression constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Sep  9 10:46:17 PDT 2005
//    Initialized varDim.
//
//    Hank Childs, Thu Jan  5 15:36:14 PST 2006
//    Initialized isNodal.
//
//    Hank Childs, Sun Mar 22 14:13:16 CDT 2009
//    Initialized onDemandProcessing.
//
// ****************************************************************************

avtCMFEExpression::avtCMFEExpression()
{
    varDim = 1;;
    isNodal = false;
    onDemandProcessing = false;
}


// ****************************************************************************
//  Method: avtCMFEExpression destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

avtCMFEExpression::~avtCMFEExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtCMFEExpression::AddInputVariableName
//
//  Purpose:
//      Adds an input variable's name.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2006
//
// ****************************************************************************

void
avtCMFEExpression::AddInputVariableName(const char *vname)
{
    avtExpressionFilter::AddInputVariableName(vname);
    varnames.push_back(vname);
}


// ****************************************************************************
//  Method: avtCMFEExpression::PreExecute
//
//  Purpose:
//      Reset the "issuedWarning" flag.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
// ****************************************************************************

void
avtCMFEExpression::PreExecute(void)
{
    issuedWarning = false;
    avtExpressionFilter::PreExecute();
}


// ****************************************************************************
//  Method: avtCMFEExpression::ProcessArguments
//
//  Purpose:
//      Tells the first argument to go generate itself.  The second argument
//      is the database to compare with.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Sep  9 09:38:08 PDT 2005
//    Add suport for expressions in first argument.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Add support for implied database names.
//
//    Hank Childs, Thu Jan  5 16:33:39 PST 2006
//    Add support for a third variable to set up default values.
//
// ****************************************************************************

void
avtCMFEExpression::ProcessArguments(ArgsExpr *args, 
                                          ExprPipelineState *state)
{
    // Check the number of arguments
    std::vector<ArgExpr*> *arguments = args->GetArgs();
    int nargs = arguments->size();

    int targetArgs = 2;
    const char *mismatchMsg = 
                   "The database comparison expression expects two "
                   "arguments: the database variable and a mesh to sample "
                   "onto.";
    if (HasDefaultVariable())
    {
        targetArgs = 3;
        mismatchMsg = 
                   "The database comparison expression expects three "
                   "arguments: the database variable, a mesh to sample "
                   "onto, and a value or constant to assign to parts of"
                   " the mesh that are not overlapping.";
    }

    // Check if there's a second argument.
    if (nargs != targetArgs)
    {
        EXCEPTION2(ExpressionException, outputVariableName, mismatchMsg);
    }

    // Tell the second argument (the mesh) to create its filters.
    ArgExpr *secondarg = (*arguments)[1];
    avtExprNode *secondTree = dynamic_cast<avtExprNode*>(secondarg->GetExpr());
    secondTree->CreateFilters(state);

    if (targetArgs == 3)
    {
        // Tell the third argument (the default var) to create its filters.
        ArgExpr *thirdarg = (*arguments)[2];
        avtExprNode *thirdTree=dynamic_cast<avtExprNode*>(thirdarg->GetExpr());
        thirdTree->CreateFilters(state);
    }

    // Pull off the first argument and see if it's a string or a list.
    ArgExpr *firstarg = (*arguments)[0];
    argument_expression = firstarg->GetText();
    ExprParseTreeNode *firstTree = firstarg->GetExpr();
    bool oldVal = VarExpr::GetVarLeavesRequiresCurrentDB();
    VarExpr::SetGetVarLeavesRequiresCurrentDB(false);
    std::set<ExprParseTreeNode *> vars = firstTree->GetVarLeafNodes();
    VarExpr::SetGetVarLeavesRequiresCurrentDB(oldVal);
    if (vars.empty())
    {
        EXCEPTION2(ExpressionException, outputVariableName, 
                   "The database comparison expression does not contain "
                   "any real variables.");
    }
    std::set<ExprParseTreeNode *>::iterator i = vars.begin();
    if ((*i)->GetTypeName() != "Var")
    {
        // There should be no way to get to this line.
        EXCEPTION2(ExpressionException, outputVariableName,
                   "VisIt was not able to locate a real variable in your "
                   "database comparison expression.");
    }

    VarExpr *var_expr = dynamic_cast<VarExpr *>(*i);
    var  = var_expr->GetVar()->GetFullpath();

    DBExpr *db_expr = var_expr->GetDB();
    if (db_expr == NULL)
    {
        EXCEPTION2(ExpressionException, outputVariableName,
                   "The first argument of the database comparison "
                   "expression must be a database.");
    }
    db = "";
    if (db_expr->GetFile() != NULL)
        db = db_expr->GetFile()->GetFullpath();

    TimeExpr *time_expr = db_expr->GetTime();
    if (time_expr == NULL)
        timeType = TimeExpr::Unknown;
    else
    {
        timeType = time_expr->GetType();
        isDelta = time_expr->GetIsDelta();
        ListExpr *l_expr = time_expr->GetList();
        std::vector<ListElemExpr *> *l_elems = l_expr->GetElems();
        if (l_elems == NULL || l_elems->size() < 1)
            EXCEPTION2(ExpressionException, outputVariableName,
                       "No times were specified.");
        if (l_elems->size() > 1)
            EXCEPTION2(ExpressionException, outputVariableName,
                       "Only one time can be specified.");
        ListElemExpr *the_one = (*l_elems)[0];
        ExprNode *cons = the_one->GetBeg();
        bool negate = false;
        if (cons->GetTypeName() == "Unary")
        {
            UnaryExpr *unary = dynamic_cast<UnaryExpr*>(cons);
            cons = unary->GetExpr();
            negate = true;
        }
        if (timeType == TimeExpr::Cycle || timeType == TimeExpr::Index)
        {
            if (cons->GetTypeName() != "IntegerConst")
            {
                EXCEPTION2(ExpressionException, outputVariableName,
                       "The type of time you have specified requires an "
                       "integer argument.");
            }
            else
            {
                ConstExpr *c_cons = dynamic_cast<ConstExpr*>(cons);
                IntegerConstExpr *i_cons =
                                   dynamic_cast<IntegerConstExpr*>(c_cons);
                int val = i_cons->GetValue();
                if (timeType == TimeExpr::Cycle)
                    cycle = (negate ? -val : val);
                else
                    timeIndex = (negate ? -val : val);
            }
        }
        else if (timeType == TimeExpr::Time)
        {
            if (cons->GetTypeName() != "FloatConst")
            {
                EXCEPTION2(ExpressionException, outputVariableName,
                       "The type of time you have specified requires a "
                       "floating point argument.");
            }
            else
            {
                dtime = dynamic_cast<FloatConstExpr*>(cons)->GetValue();
                dtime = (negate ? -dtime : dtime);
            }
        }
        else
        {
            EXCEPTION2(ExpressionException, outputVariableName,
                   "a time was specified for a "
                   "database, but that time could not be parsed."
                   "Try adding the \'c\', \'i\', or \'t\' qualifiers.");
        }
    }
}


// ****************************************************************************
//  Method: avtCMFEExpression::Execute
//
//  Purpose:
//      Gets the second mesh to compare onto and calls the virtual method that
//      does the actual cross mesh field evaluation.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Sep  1 11:23:32 PDT 2005
//    Add handling for time.
//
//    Hank Childs, Thu Sep  8 13:50:17 PDT 2005
//    Add handling for expressions.
//
//    Hank Childs, Fri Oct  7 08:31:30 PDT 2005
//    Add support for implied database names.  Also use the same SIL
//    restriction if the derived type thinks we should.
//
//    Hank Childs, Thu Jan  5 15:36:14 PST 2006
//    Add better support for variable centering.
//
//    Hank Childs, Thu Apr 10 16:10:33 PDT 2008
//    Make sure that consistent ghost levels are requested.
//
//    Hank Childs, Sun Feb 22 10:46:41 PST 2009
//    Allow for expressions to be the "CMFE var" 
//    (example: <conn_cmfe([1]id:p+1> ... p+1 is an expression)
//
//    Hank Childs, Sun Mar 22 14:13:16 CDT 2009
//    Add support for on demand streaming.
//
//    Hank Childs, Tue May  5 14:15:45 PDT 2009
//    Fix parsing problem with names that have subdirectories.
//
//    Mark C. Miller, Wed Jun 17 14:23:25 PDT 2009
//    Replaced CATCH(...) with CATCHALL
// ****************************************************************************

void
avtCMFEExpression::Execute()
{
    if (db == "")
        db = GetInput()->GetInfo().GetAttributes().GetFullDBName();
    ref_ptr<avtDatabase> dbp = avtCallback::GetDatabase(db, 0, NULL);
    if (*dbp == NULL)
        EXCEPTION1(InvalidFilesException, db.c_str());

    int actualTimestep = GetTimestate(dbp);

    ParsingExprList *pel = ParsingExprList::Instance();
    ExpressionList original_list = *(pel->GetList());
    ExpressionList new_list = original_list;
    Expression exp2;
    exp2.SetName("_avt_internal_cmfe_expr");
    std::string var_wo_quotes = var;
    for (int i = var_wo_quotes.size()-1 ; i >= 0 ; i--)
        if (var_wo_quotes[i] == '\'')
            var_wo_quotes.replace(i,1,"");
    std::string final_var = "<" + var_wo_quotes + ">";

    exp2.SetDefinition(final_var);
    exp2.SetType(Expression::Unknown);
    new_list.AddExpressions(exp2);
    *(pel->GetList()) = new_list;

    std::string dbvar = pel->GetRealVariable("_avt_internal_cmfe_expr");

    avtDataObject_p dob = dbp->GetOutput(dbvar.c_str(), actualTimestep);
    if (*dob == NULL)
        EXCEPTION1(InvalidVariableException, var.c_str());
    if (strcmp(dob->GetType(), "avtDataset") != 0)
        EXCEPTION1(InvalidVariableException, var.c_str());

    // We now need to create an expression that corresponds to the first
    // argument.  See extended comment below in section #2.
    std::string expr_var = "_avt_cmfe_expression_";

    avtDataRequest_p ds = new avtDataRequest( dob->GetOriginatingSource()
                                ->GetGeneralContract()->GetDataRequest(),
                                expr_var.c_str());
    avtContract_p spec = new avtContract(ds, 1);
    if (UseIdenticalSIL())
    {
        // This will only work for conn_cmfe.
        ds = new avtDataRequest(ds, firstDBSIL);
        spec = new avtContract(spec, ds);
    }
    spec->GetDataRequest()->SetTimestep(actualTimestep);
    spec->GetDataRequest()->SetDesiredGhostDataType(ghostNeeds);
    spec->SetOnDemandStreaming(onDemandProcessing);
    for (int i = 0 ; i < dataSels.size() ; i++)
        spec->GetDataRequest()->AddDataSelectionRefPtr(dataSels[i]);

    avtExpressionEvaluatorFilter *eef = new avtExpressionEvaluatorFilter;
    eef->SetInput(dob);
    dob = eef->GetOutput();

    //
    // When we cause the pipeline to update, it is going to cause the
    // EEF to execute.  We need to do two things to make sure it works
    // correctly:
    // 1) The first pass through (which led to this CMFE filter being
    //    instantiated), we ignored the first argument to this filter, which
    //    had the database and database variable listed.  We need to tell
    //    the EEF *not* to ignore those variables this time.  This is done
    //    through the GetVarLeavesRequiresCurrentDB call.
    // 2) The first argument to the CMFE can possibly be an expression 
    //    itself, for example log(pressure), assuming pressure is a database
    //    variable (actually probably log(file1:pressure)).  So we need to
    //    tell the pipeline to generate this variable.  But we can't ask for
    //    log(pressure) ... that's not a valid variable.  So we have to create
    //    a new expression like _dummy_expr = log(pressure).  Then we can
    //    ask the pipeline to create _dummy_expr.  So we need to take the
    //    list of expressions and add our new expression.  This is very
    //    similar to what the Macro expressions do.
    //
    bool oldVal = VarExpr::GetVarLeavesRequiresCurrentDB();
    VarExpr::SetGetVarLeavesRequiresCurrentDB(false);
    Expression exp;
    exp.SetName(expr_var.c_str());
    std::string replace_string = ":" + var;
    int pos = argument_expression.find(replace_string)+1;
    std::string new_defn = argument_expression;
    new_defn.replace(pos, var.size(), "_avt_internal_cmfe_expr");
    exp.SetDefinition(new_defn);
    exp.SetType(Expression::Unknown);
    new_list.AddExpressions(exp);
    *(pel->GetList()) = new_list;

    TRY
    {
        dob->Update(spec);
    }
    CATCHALL
    {
        //
        // Now switch back #1 and #2
        //
        VarExpr::SetGetVarLeavesRequiresCurrentDB(oldVal);
        *(pel->GetList()) = original_list;

        char msg[1024];
        sprintf(msg, "Unable to evaluate expression \"%s\" for doing "
                     "database comparison.", argument_expression.c_str());
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }
    ENDTRY;

    //
    // Now switch back #1 and #2
    //
    VarExpr::SetGetVarLeavesRequiresCurrentDB(oldVal);
    *(pel->GetList()) = original_list;

    delete eef;
    avtDataset_p dsp;
    CopyTo(dsp, dob);
    varDim = 
        dsp->GetInfo().GetAttributes().GetVariableDimension(expr_var.c_str());
    isNodal = (dsp->GetInfo().GetAttributes().GetCentering(expr_var.c_str())
               == AVT_NODECENT);

    avtDataTree_p output = PerformCMFE(GetInputDataTree(), dsp->GetDataTree(),
                                       expr_var, outputVariableName);
    SetOutputDataTree(output);

    //
    // Reset the variable information to match the newly CMFE'd variable.
    //
    avtDataAttributes &inputAtts = dob->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    SetExpressionAttributes(inputAtts, outAtts);
}


// ****************************************************************************
//  Method: avtCMFEExpression::GetTimestate
//
//  Purpose:
//      Determines what the correct time state is, using the current time
//      state, as well as specifications in the expression.
//
//  Programmer: Hank Childs
//  Creation:   September 1, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Jan 12 11:34:43 PST 2006
//    Post a special error message for time derivatives, since they will
//    cause the error message from this routine to execute most frequently.
//
//    Hank Childs, Tue Jun 13 13:10:11 PDT 2006
//    Fix a typo in warning message.
//
// ****************************************************************************

int
avtCMFEExpression::GetTimestate(ref_ptr<avtDatabase> dbp)
{
    avtDatabaseMetaData *md = NULL;
    int actualTimestep = 0;
    if (timeType == TimeExpr::Index)
    {
        if (isDelta)
            actualTimestep = firstDBTime+timeIndex;
        else
            actualTimestep = timeIndex;
    }
    else if (timeType == TimeExpr::Cycle)
    {
        md = dbp->GetMetaData(0, false, true);
        if (md->GetCycles().size() == 0 || !md->AreAllCyclesAccurateAndValid())
        {
            avtCallback::IssueWarning("VisIt cannot choose a time state "
                 "for comparing databases based on a cycle, because the "
                 "cycles are not believed to be accurate.  Using the first "
                 "time state instead.");
        }
        else
        {
            int c = (isDelta ? md->GetCycles()[firstDBTime] + cycle : cycle);
            int closest      = 0;
            int closest_dist = abs(c-md->GetCycles()[0]);
            for (int i = 0 ; i < md->GetCycles().size() ; i++)
            {
                int dist = abs(c-md->GetCycles()[i]);
                if (dist < closest_dist)
                {
                    closest      = i;
                    closest_dist = dist;
                }
            }
            actualTimestep = closest;
        }
    }
    else if (timeType == TimeExpr::Time)
    {
        md = dbp->GetMetaData(0, true, false);
        if (md->GetTimes().size() == 0 || !md->AreAllTimesAccurateAndValid())
        {
            avtCallback::IssueWarning("VisIt cannot choose a time state "
                 "for comparing databases based on a time, because the "
                 "times are not believed to be accurate.  Using the first "
                 "time state instead.");
        }
        else
        {
            float c = (isDelta ? md->GetTimes()[firstDBTime] + dtime : dtime);
            int   closest      = 0;
            float closest_dist = fabs(c-md->GetTimes()[0]);
            for (int i = 0 ; i < md->GetTimes().size() ; i++)
            {
                float dist = fabs(c-md->GetTimes()[i]);
                if (dist < closest_dist)
                {
                    closest      = i;
                    closest_dist = dist;
                }
            }
            actualTimestep = closest;
        }
    }
    else
        return 0;
    

    if (actualTimestep > 0 && md == NULL)
    {
        md = dbp->GetMetaData(0);
    }

    if (actualTimestep < 0)
    {
        actualTimestep = 0;
        if (isDelta && firstDBTime == 0 && timeIndex == -1)
            avtCallback::IssueWarning("VisIt uses the current time state and "
               "the previous time state when doing a time derivative.  Because"
               " you are at the first time state, there is no previous time "
               "state to difference with.  So the resulting plot will have no "
               "differences.  This feature will work correctly, however, for "
               "the rest of the time states.");
        else
            avtCallback::IssueWarning("You have instructed VisIt to use a "
               "non-existent time state when comparing databases.  VisIt "
               "is using the first time state in its place.");
    }
    if (actualTimestep > 0 && actualTimestep > md->GetNumStates())
    {
        actualTimestep = md->GetNumStates()-1;
        avtCallback::IssueWarning("You have instructed VisIt to use a "
               "non-existent time state when comparing databases.  VisIt "
               "is using the last time state in its place.");
    }

    return actualTimestep;
}


// ****************************************************************************
//  Method: avtCMFEExpression::ExamineContract
//
//  Purpose:
//      Captures what the current database time state is, in case that is
//      needed for a delta.
//
//  Programmer: Hank Childs
//  Creation:   September 1, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Oct  7 10:33:20 PDT 2005
//    Cache the SIL restriction as well.
//
//    Hank Childs, Thu Apr 10 16:10:33 PDT 2008
//    Make sure that consistent ghost levels are requested.
//
//    Hank Childs, Tue Mar 24 13:18:10 CDT 2009
//    Store the data selections so we can put them in the new contract.
//
// ****************************************************************************

void
avtCMFEExpression::ExamineContract(avtContract_p spec)
{
    avtExpressionFilter::ExamineContract(spec);

    firstDBTime = spec->GetDataRequest()->GetTimestep();
    firstDBSIL  = spec->GetDataRequest()->GetRestriction();
    ghostNeeds  = spec->GetDataRequest()->GetDesiredGhostDataType();
    const std::vector<avtDataSelection_p> ds = spec->GetDataRequest()->GetAllDataSelections();
    int numSels = ds.size();
    dataSels.clear();
    for (int i = 0 ; i < numSels ; i++)
        dataSels.push_back(spec->GetDataRequest()->GetDataSelection(i));
    onDemandProcessing = spec->DoingOnDemandStreaming();
}


