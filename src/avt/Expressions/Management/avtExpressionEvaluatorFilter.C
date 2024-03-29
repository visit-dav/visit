// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtExpressionEvaluatorFilter.C                  //
// ************************************************************************* //

#include <algorithm>
#include <vector>

#include <vtkDataArray.h>

#include <avtDatasetExaminer.h>
#include <avtDataBinning.h>
#include <avtDataBinningFunctionInfo.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtExpressionFilter.h>
#include <ExpressionException.h>
#include <avtIdentityExpression.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTypes.h>
#include <avtExpressionTypeConversions.h>

#include <ParsingExprList.h>
#include <PickAttributes.h>
#include <PickVarInfo.h>

#include <avtExprNode.h>
#include <Expression.h>
#include <ParsingExprList.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <NoInputException.h>
#include <TimingsManager.h>

using std::vector;
using std::set;
using std::string;
using std::find;


GetDataBinningCallback  avtExpressionEvaluatorFilter::getDataBinningCallback = NULL;
void                  *avtExpressionEvaluatorFilter::getDataBinningCallbackArgs = NULL;


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Dec 31 11:47:01 PST 2004
//    Initialize termsrc.
//
//    Hank Childs, Mon Mar 23 11:02:55 CDT 2009
//    Initialize onDemandProcessing.
//
// ****************************************************************************

avtExpressionEvaluatorFilter::avtExpressionEvaluatorFilter()
{
    termsrc = NULL;
    currentTimeState = 0;
    onDemandProcessing = false;
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Dec 31 11:47:01 PST 2004
//    Delete termsrc.
//
// ****************************************************************************

avtExpressionEvaluatorFilter::~avtExpressionEvaluatorFilter()
{
    if (termsrc != NULL)
        delete termsrc;
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::Execute
//
//  Purpose:
//      Hook up the pipeline in the pipelineState to the network.
//
//  Programmer:   Sean Ahern
//  Creation:     Wed Jan 22 10:38:51 PST 2003
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 28 08:01:45 PDT 2004
//    Use the timestep retrieved during ExamineContract to set the
//    timestep for the filters.
//
//    Hank Childs, Wed Dec 22 10:54:41 PST 2004
//    Consult the 'lastUsedSpec' to come up with the list of secondary 
//    variables that are needed downstream. ['5790]
//
//    Hank Childs, Fri Dec 31 11:47:01 PST 2004
//    Use a cached terminating source rather than one on the stack.
//
//    Hank Childs, Tue Aug 16 16:53:20 PDT 2005
//    Added called to VerifyVariableTypes ['6485]
//
//    Hank Childs, Fri Sep 23 09:50:49 PDT 2005
//    Directly use 'lastUsedSpec's data specification, since it has the
//    best description of the data we want.
//
//    Hank Childs, Mon Jan 14 20:47:54 PST 2008
//    Make sure no singleton constants escape out of the EEF.
//
//    Hank Childs, Mon Sep 15 16:30:50 PST 2008
//    Deleted termsrc to free up memory for the next pipeline operations.
//
//    Hank Childs, Mon Mar 23 11:02:55 CDT 2009
//    Set contract with whether we are doing "onDemandProcessing".
//
//    Hank Childs, Wed Dec 22 12:56:36 PST 2010
//    More passing along of contract data members to honor streaming.
//
// ****************************************************************************

void
avtExpressionEvaluatorFilter::Execute(void)
{
    // Start the timer going
    int  timingIndex = visitTimer->StartTimer();

    // Get my input and output for hooking up to the database.
    avtDataObject_p dObj = GetInput();

    // Make sure our version is the "latest and greatest".
    termsrc->ResetTree(GetInputDataTree());
    termsrc->GetOutput()->GetInfo().Copy(GetInput()->GetInfo());
    avtDataObject_p data = termsrc->GetOutput(); 

    // Do we have anything to do?  Did we apply any expression filters?
    if (pipelineState.GetFilters().size() != 0)
    {
        // Hook the input and output up to the pipeline in pipelineState.
        vector<avtExpressionFilter*> &filters = pipelineState.GetFilters();
        avtExpressionFilter *top =    filters.front();
        avtExpressionFilter *bottom = filters.back();

        top->SetInput(data);
        //
        // Make sure that the DataSpec being used has the timestep needed.
        //
        avtContract_p contract = GetGeneralContract();
        avtDataRequest_p new_dataRequest = 
                new avtDataRequest(lastUsedSpec->GetDataRequest());
        new_dataRequest->SetTimestep(currentTimeState);
        contract = new avtContract(contract, new_dataRequest);
        contract->SetCalculateVariableExtentsList(
                       lastUsedSpec->GetCalculateVariableExtentsList());
        contract->SetCalculateMeshExtents(lastUsedSpec->ShouldCalculateMeshExtents());
        contract->SetOnDemandStreaming(onDemandProcessing);
        contract->UseLoadBalancing(lastUsedSpec->ShouldUseLoadBalancing());
        contract->SetReplicateSingleDomainOnAllProcessors(replicateSingleDomainOnAllProcessors);
        bottom->Update(contract);
        GetOutput()->Copy(*(bottom->GetOutput()));
    } else {
        GetOutput()->Copy(*dObj);
    }

    // Make sure no singleton constants escape from the EEF.  This should
    // be essentially a no-op for the case where there are no singletons.
    bool success = true;
    GetDataTree()->Traverse(CExpandSingletonConstants, NULL, success);

    VerifyVariableTypes();

    // Delete the termsrc to free up resources
    if(termsrc != 0)
    {
        delete termsrc;
        termsrc = 0;
    }

    // Stop the timer
    visitTimer->StopTimer(timingIndex, "Expression Evaluator Filter");
    visitTimer->DumpTimings();
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::VerifyVariableTypes
//
//  Purpose:
//      Verify that the variables created are of the same type they were
//      declared to be.
//
//  Programmer: Hank Childs
//  Creation:   August 16, 2005
//
//  Modifications:
//
//    Hank Childs, Fri Aug 19 09:20:24 PDT 2005
//    Switch ordering of arguments in error message.
//
//    Hank Childs, Fri Sep  9 10:27:45 PDT 2005
//    Don't bother checking against 'unknown', since it probably comes from
//    a CMFE expression.
//
//    Hank Childs, Fri Jun  9 14:34:50 PDT 2006
//    Add default to switch statement.
//
//    Kathleen Bonnell, Wed Aug  2 17:54:47 PDT 2006
//    Support CurveMeshVar expressions. 
//
//    Hank Childs, Mon Jan  8 10:57:33 PST 2007
//    Use the ParsingExprList method to map AVT variable types to expression
//    types.  (We essentially had two versions of the same routine.)
//
//    Brad Whitlock, Tue Jan 20 15:57:01 PST 2009
//    I changed the name of a type conversion function.
//
//    Rob Sisneros, Sun Aug 29 20:13:10 CDT 2010
//    Add a check for variables from operators.
//
// ****************************************************************************

void
avtExpressionEvaluatorFilter::VerifyVariableTypes(void)
{
    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
    int nvars = atts.GetNumberOfVariables();
    for (int i = 0 ; i < nvars ; i++)
    {
        const std::string &varname = atts.GetVariableName(i);
        avtVarType vt = atts.GetVariableType(varname.c_str());
        Expression const *exp = ParsingExprList::GetExpression(varname.c_str());
        if (exp == NULL)
            continue;
        Expression::ExprType et = exp->GetType();
        if (et == Expression::Mesh || et == Expression::Material ||
            et == Expression::Species || et == Expression::Unknown)
            continue;
        avtVarType et_as_avt = ExprType_To_avtVarType(et);

        // consider a ScalarVar to be equivalent to a Curve Expression.
        if (vt != et_as_avt &&
           (!(vt == AVT_SCALAR_VAR && et_as_avt == AVT_CURVE)))
        {
            char msg[1024];
            sprintf(msg, "The expression variable \"%s\" was declared to be of"
                         " type %s, but is actually of type %s.  Please "
                         "confirm that the variable was declared correctly.  "
                         "Contact us via https://visit-help.llnl.gov if you believe "
                         "that the variable has been declared correctly.",
                         varname.c_str(),avtVarTypeToString(et_as_avt).c_str(),
                         avtVarTypeToString(vt).c_str());
            EXCEPTION1(VisItException, msg);
        }
        // If the expression is from an operator, and has gotten this far,
        // something is wrong.
        if(exp->GetFromOperator())
        { 
            char msg[1024];
            sprintf(msg, "The expression variable \"%s\" was declared as an operator"
                      " expression.  This variable must be removed from the contract"
                      " and created by the operator.", varname.c_str());
            EXCEPTION1(VisItException, msg);
        }
    }
}


// ****************************************************************************
//  Method: avtExpressionEvaluationFilter::AdditionalPipelineFilters
//
//  Purpose:
//      Gets the number of additional stages we will execute on in the pipeline
//      when evaluating the expressions.
//
//  Programmer: Sean Ahern
//  Creation:   January 22, 2003 (?)
//
// ****************************************************************************

int
avtExpressionEvaluatorFilter::AdditionalPipelineFilters(void)
{
    return (int)pipelineState.GetFilters().size();
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::ModifyContract
//
//  Purpose:
//      Determines if any of the variables are expressions.  If so, remove
//      the variable name (because it will only confuse the database), and
//      put it on the list of variables to generate.  Walk through the
//      expressions to determine what the base list of variables should be.
//      Put that base list back on the data specification.
//
//      Tells the expression trees to create the pipeline filters that the
//      expressions require to work.  Their state is managed by the
//      pipelineState variable.
//
//  Programmer: Sean Ahern
//  Creation:   Thu Nov 21 15:17:05 PST 2002
//
//  Modifications:
//
//    Hank Childs, Mon Jul 21 15:35:48 PDT 2003
//    Make sure that the primary variable stays the same.
//
//    Sean Ahern, Wed Jul 23 16:53:07 PDT 2003
//    Reorganized the code slightly to reduce code duplication.
//
//    Brad Whitlock, Wed Aug 27 14:08:36 PST 2003
//    Made it work on Windows.
//
//    Hank Childs, Fri Oct 24 14:31:33 PDT 2003
//    Allow expressions that the EEF will use to also perform a restriction.
//    Also fixed lines longer than 79 characters.
//
//    Hank Childs, Wed Dec 10 14:33:53 PST 2003
//    Added support for expressions of the form A=B.  This involves the 
//    identity function and was causing a crash previously.
//
//    Hank Childs, Sat Dec 13 15:55:31 PST 2003
//    Don't be so quick to declare something a recursive function.
//
//    Kathleen Bonnell, Tue Apr 27 11:34:23 PDT 2004 
//    Added test to determine if new filters need to be created.  If not,
//    don't set the contract's DataObject to NULL, and don't create new
//    filters.  (QueryOverTime will re-use these filters for each time step).
//
//    Jeremy Meredith, Wed Nov 24 12:25:10 PST 2004
//    Renamed EngineExprNode to avtExprNode because of a refactoring.
//
//    Hank Childs, Fri Dec 31 11:50:07 PST 2004
//    Maintain our own cached version of the terminating source.
//
//    Hank Childs, Thu Jan  6 11:08:56 PST 2005
//    Beef up logic to insert identity filters.
//
//    Jeremy Meredith, Mon Jun 13 15:51:50 PDT 2005
//    Delete the parse trees when we're done with them.  This fixes leaks.
//
//    Hank Childs, Sun Feb 19 10:03:12 PST 2006
//    Add support for DDFs.  Also correct order of perform restriction calls.
//
//    Cyrus Harrison, Tue Jul  3 11:24:33 PDT 2007
//    Changed to reflect return of vector from GetVarLeaves()
//
//    Hank Childs, Mon Dec 10 17:49:02 PST 2007
//    Keep track of variables that will be generated.  This is needed to
//    force recalculation in rare cases.
//
//    Jeremy Meredith, Fri Oct 17 17:05:20 EDT 2008
//    In the case of a recursive expressions, return an expression error, not
//    an improper use error.  (The engine will treat ImproperUseExceptions
//    as fatal and exit.)
//
//    Hank Childs, Sat Aug 21 14:02:28 PDT 2010
//    Rename DDFs to DataBinnings.
//
// ****************************************************************************

avtContract_p
avtExpressionEvaluatorFilter::ModifyContract(avtContract_p spec)
{
    pipelineState.Clear();

    avtContract_p rv = spec;

    avtDataRequest_p ds = spec->GetDataRequest();
    bool createFilters = true;
    if (*lastUsedSpec != NULL)
    {
        avtDataRequest_p ds1 = lastUsedSpec->GetDataRequest();
        createFilters = pipelineState.GetFilters().size() == 0 ||
                        !ds1->VariablesAreTheSame(ds);
    }
    lastUsedSpec = spec;
    avtDataRequest_p newds;

    // We need to test if any of the primary or secondary variables are
    // expressions.  We keep a list "candidates" of variables whose type we
    // don't know, a list "real_list" of real variables, and a list
    // "expr_list" of expression variables.  For each expression variable,
    // we parse it down to real variables.  When we're done, we take the
    // expressions and create the necessary filters to make them happen.
    // We also take the real variables and make a new data specification
    // that tells the database to load those.
    //
    // NOTE: Sets might not be the best way to keep these lists.  I need
    // deletion of duplicates and fast insertion.  A post-processed vector
    // might be better.
    set<string> candidates;
    set<string> real_list;
    vector<string> expr_list;

    // Insert all of the variables onto the candidates list.
    candidates.insert(ds->GetVariable());
    const vector<CharStrRef> &sv = ds->GetSecondaryVariables();
    for (size_t i = 0; i != sv.size(); i++)
    {
        const char *str = *(sv[i]);
        candidates.insert(str);
    }

    // Walk through the candidates, processing the results into the real
    // list and the expression list.  When the variables are found, turn the
    // parsed expression into a list of filters.  These filters are hooked
    // together, then put on a list in pipelineState for use in Execute().
    debug4 << "EEF::ModifyContract: Checking candidates" << endl;

    if (createFilters)
        pipelineState.SetDataObject(NULL);

    int num_recursive_checks = 0;
    while (!candidates.empty())
    {
        std::set<string>::iterator front = candidates.begin();
        string var = *front;
        candidates.erase(front);
        debug4 << "EEF::ModifyContract:     candidate: " << var.c_str() 
               << endl;

        // Have we seen this before?
        std::vector<string>::iterator search;
        search = find(expr_list.begin(), expr_list.end(), var);
        if (search != expr_list.end())
        {
            // We were asked about an expression that we have seen before.  This
            // can happen under normal conditions.  For example, we see an expr
            // that is a vector.  Later on, we get a magnitude expression that
            // references the original vector.  That would get us into this loop.
            // And its not recursive.  That said, this loop is entered when we
            // get a recursive expression.  So, if we come in 1000 times, we
            // probably have a recursive expression.
            num_recursive_checks++;
            if (num_recursive_checks >= 1000)
            {
                // We've seen this expression already.
                char error[] = "Recursive expression.";
                debug1 << error << endl;
                EXCEPTION2(ExpressionException, var, "it was recursive");
            }
        }

        // Check if this is an expression, a real variable, or a data binning.
        Expression const *exp = ParsingExprList::GetExpression(var);
        avtDataBinning *db = NULL;
        if (getDataBinningCallback != NULL)
            db = getDataBinningCallback(getDataBinningCallbackArgs, var.c_str());

        if (exp == NULL && db == NULL)
        {
            debug4 << "EEF::ModifyContract:     not an expression" << endl;
            // Not an expression.  Put the name on the real list.
            real_list.insert(var);
        } 
        else if (db != NULL)
        {
            debug4 << "EEF::ModifyContract:     Data Binning.  Roots:" << endl;
            avtDataBinningFunctionInfo *info = db->GetFunctionInfo();
            int nVars = info->GetDomainNumberOfTuples();
            for (int i = 0 ; i < nVars ; i++)
            {
                std::string name = info->GetDomainTupleName(i);
                debug4 << "EEF::ModifyContract:         " << name.c_str() << endl;
                candidates.insert(name);
            }
            std::string name = info->GetCodomainName();
            debug4 << "EEF::ModifyContract:         " << name.c_str() << endl;
            candidates.insert(name);
        }
        else  // (expr != NULL)
        {
            debug4 << "EEF::ModifyContract:     expression.  Roots:" 
                   << endl;
            // Expression.  Put the name on the expr list.  Find the base
            // variables of the expression and put them on the candidate
            // list.
            expr_list.push_back(var);
            ExprNode *tree = ParsingExprList::GetExpressionTree(var);
            avtExprNode *avttree = dynamic_cast<avtExprNode*>(tree);
            vector<string> roots = avttree->GetVarLeaves();
            delete tree;

            std::vector<string>::iterator itr = roots.begin();
            for ( itr = roots.begin(); itr != roots.end(); ++itr)
            {
                debug4 << "EEF::ModifyContract:         " << itr->c_str()
                       << endl;
                candidates.insert(*itr);
            }
        }
    }

    // See what expressions have to be generated.  If it is not the same list as
    // the last time we executed, then set the "modified" bit to true, forcing this
    // filter to re-execute.  If we don't set this to true, we are depending on the
    // output from the database to be different from previous execution.  This isn't
    // always the case, esp. if we are making expressions based on the mesh.
    if (expr_list_fromLastTime.size() != expr_list.size())
        modified = true;
    else
    {
        for (size_t i = 0 ; i < expr_list.size() ; i++)
            if (expr_list[i] != expr_list_fromLastTime[i])
                modified = true;
    }
    expr_list_fromLastTime = expr_list;

    // Take the list of expressions and make the filters for them.
    int numFiltersLastTime = 0;
    while (createFilters && !expr_list.empty())
    {
        std::vector<string>::iterator back = expr_list.end() - 1;
        string var = *back;
        expr_list.erase(back);
       
        // Get the expression tree again.  (We could save trees between the
        // first and second sections of the code.  It wouldn't save much
        // time, but would be cleaner.)
        avtExprNode *tree = dynamic_cast<avtExprNode*>
                                     (ParsingExprList::GetExpressionTree(var));

        // Create the filters that the tree uses.  Put them into the
        // filters stack in pipelineState.
        tree->CreateFilters(&pipelineState);
        delete tree;

        vector<avtExpressionFilter*> &filters = pipelineState.GetFilters();
        avtExpressionFilter *f = NULL;
        if (filters.size() == (size_t)numFiltersLastTime)
        {
            // The only way we can get here is if we have an expression of 
            // the form "A = B".
            debug1 << "Warning: expression logic identified expression of "
                   << "form A = B." << endl;
            avtIdentityExpression *ident = new avtIdentityExpression();
            string inputName = pipelineState.PopName();
            ident->AddInputVariableName(inputName.c_str());
            ident->SetOutputVariableName(var.c_str());
            ident->SetInput(pipelineState.GetDataObject());
            pipelineState.SetDataObject(ident->GetOutput());
            pipelineState.AddFilter(ident);
            f = ident;
        }
        else
        {
            f = filters.back();
        }
        f->SetOutputVariableName(var.c_str());
        numFiltersLastTime = (int)filters.size();
    }

    // Make sure we have real variables to pass to the database.
    if (real_list.empty())
    {
        // We don't have any real variables left!  Not sure this can
        // actually happen, but it's good to check for in any case.
        char error[] = "After parsing, expression has no real variables.";
        debug1 << error << endl;
        EXCEPTION1(ImproperUseException, error);
    }

    //
    // Now that we know the real variables, re-set up the data specification.
    //

    // Check if the original variable is in our "real" list.
    std::set<string>::iterator it;
    bool haveActiveVariable = false;
    for (it = real_list.begin() ; it != real_list.end() ; it++)
    {
        if (*it == ds->GetVariable())
            haveActiveVariable = true;
    }

    // Set up the data spec.  Note: This data spec is used in two places.
    // The first is to tell the expressions what secondary variables are 
    // needed, what the pipeline variable is, etc, as well as let them
    // modify the pipeline specification.  The second use is to send
    // the resulting pipeline spec up to the database.
    //
    // For the first, it is important the pipeline variable be the variable
    // we are going to use later in the pipeline.  This means that we will
    // use expression variables.
    //
    // But if we send the expression variables up to the database, it won't
    // know about the variables we are referring to.  So that's a bad idea.
    // So we should remove the expression variables before they get up to the
    // database.
    //
    // So start off by creating a pipeline specification that has the
    // expression variable for expression filters.  Then, after they have
    // modified the pipeline specification, swap out the variable for a real
    // variable.

    it = real_list.begin();
    newds = new avtDataRequest(ds);
    newds->RemoveAllSecondaryVariables();
    for ( ; it != real_list.end() ; it++)
        if (*it != ds->GetVariable())
            newds->AddSecondaryVariable((*it).c_str());
    rv = new avtContract(spec, newds);

    //
    // Set up the input of the terminating source.  This is because some of
    // the filters may need to be able to go upstream to do the perform
    // restriction.
    //
    vector<avtExpressionFilter *> &filters = pipelineState.GetFilters();
    if (termsrc == NULL)
    {
        termsrc = new avtSourceFromAVTDataset(GetTypedInput());
    }
    if (pipelineState.GetFilters().size() != 0)
    {
        avtExpressionFilter *top =    filters.front();
        top->SetInput(termsrc->GetOutput());
    }

    // Do these in the order the pipeline would.
    for (int i = (int)filters.size()-1 ; i >= 0 ; i--)
    {
        rv = filters[i]->ModifyContract(rv);
    }

    // Here's the part where we swap out the active variable for a real one
    // in the database.  See extended comment above for more details.
    if (!haveActiveVariable)
    {
        it = real_list.begin();
        newds = new avtDataRequest(rv->GetDataRequest(), 
                                         (*it).c_str());
        newds->SetOriginalVariable(ds->GetVariable());
        rv = new avtContract(rv, newds);
    }

    return rv;
}

// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::ReleaseData
//
//  Purpose:
//      Releases the data associated sub-filters of the EEF.
//
//  Programmer: Hank Childs
//  Creation:   November 17, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Apr 22 14:42:38 PDT 2004
//    Moved code to new pipelineState method.
//  
//    Hank Childs, Mon Feb  7 16:41:36 PST 2005
//    Fix memory leak.
//
// ****************************************************************************

void
avtExpressionEvaluatorFilter::ReleaseData(void)
{
    avtDatasetToDatasetFilter::ReleaseData();
    pipelineState.ReleaseData();
    if (termsrc != NULL)
    {
        delete termsrc;
        termsrc = NULL;
    }
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::RegisterGetDataBinningCallback
//
//  Purpose:
//      Registers a callback that allows us to get data binnings.
//
//  Programmer:  Hank Childs
//  Creation:    February 19, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:02:28 PDT 2010
//    Rename method (DDFs to DataBinnings).
//
// ****************************************************************************

void
avtExpressionEvaluatorFilter::RegisterGetDataBinningCallback(GetDataBinningCallback gdc,
                                                     void *args)
{
    getDataBinningCallback     = gdc;
    getDataBinningCallbackArgs = args;
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::CanApplyToDirectDatabaseQOT
//
//  Purpose:
//      Determine if we can apply our filters to a direct database query 
//      over time. There are only a subset of filters that can handle this.
//
//  Returns:
//      true if the filters can be applied to a direct database QOT.
//      Otherwise, false.
//
//  Programmer: Alister Maguire 
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

bool
avtExpressionEvaluatorFilter::CanApplyToDirectDatabaseQOT(void)
{
    if (pipelineState.GetFilters().size() != 0)
    {
        vector<avtExpressionFilter*> &filters = pipelineState.GetFilters();

        for (vector<avtExpressionFilter *>::const_iterator it = filters.begin();
             it < filters.end(); ++it)
        {
            if ((*it) != NULL && !(*it)->CanApplyToDirectDatabaseQOT())
            {
                return false;
            }
        }
    }

    return true;
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::Query
//
//  Purpose:
//      This does part of the work for a pick and query.  Since the input
//      dataset does not contain all the information about the original data
//      (material information, etc), this will relay the request to the
//      queryable source upstream.  When the query comes back, it will 
//      determine which zone and domain we are talking about and add the 
//      information about the expressions.
//
//  Programmer: Hank Childs
//  Creation:   July 29, 2003
//
//  Modifications:
//
//    Hank Childs, Tue Aug 19 20:47:26 PDT 2003
//    Cause an update if there are expression variables that are not sitting
//    in our output.
//
//    Hank Childs, Mon Sep 22 13:32:51 PDT 2003
//    Tell each variable what type it is.
//
//    Hank Childs, Wed Oct  8 15:23:13 PDT 2003
//    Make room for each expression variable (was previously clobbering
//    existing vars -- whoops).
//
//    Kathleen Bonnell, Thu Nov 20 15:06:49 PST 2003 
//    Allow for index == 0 when testing whether or not to add the pickVarInfo.
//
//    Kathleen Bonnell, Fri May 28 18:31:15 PDT 2004 
//    Account for pick type DomainZone.
//
//    Kathleen Bonnell, Wed May 11 17:07:25 PDT 2005 
//    Removed code that parsed and processed missing expression vars, 
//    this hasn't been necessary for a long while, since avtPickQuery started
//    using an 'Update' so that information that gets retrieved always
//    reflect the current SILRestriction. (about March, 2004).
//
//    Kathleen Bonnell, Thu Jun 30 10:59:50 PDT 2005 
//    Re-add the parsing code.  In some way I don't understand, this code
//    affects the avtDataAttributes that Pick uses to query from. 
// 
//    Kathleen Bonnell, Tue Jul  5 15:28:27 PDT 2005
//    Removed (again) the parsing code.  Found the real fix for the problem
//    instead of using this code.
//    
// ****************************************************************************

void
avtExpressionEvaluatorFilter::Query(PickAttributes *pa)
{
    GetInput()->GetQueryableSource()->Query(pa);
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::FindElementForPoint
//
//  Purpose:
//      Passes this request on to the queryable source. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   November 13, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar  7 10:19:17 PST 2005
//    Ensure a database variable is being used.
//
// ****************************************************************************

bool
avtExpressionEvaluatorFilter::FindElementForPoint(const char *var, const int ts,
    const int dom, const char *elType, double pt[3], int &elNum)
{
    string dbVar = ParsingExprList::GetRealVariable(var);
    return GetInput()->GetQueryableSource()->
        FindElementForPoint(dbVar.c_str(), ts, dom, elType, pt, elNum);
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::GetDomainName
//
//  Purpose:
//      Passes this request on to the queryable source. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2003 
//
//  Modifications:
//    Kathleen Bonnell, Mon Mar  7 10:19:17 PST 2005
//    Ensure a database variable is being used.
//
// ****************************************************************************

void
avtExpressionEvaluatorFilter::GetDomainName(const std::string &var, const int ts,
    const int dom, std::string &domName)
{
    string dbVar = ParsingExprList::GetRealVariable(var);
    GetInput()->GetQueryableSource()->GetDomainName(dbVar, ts, dom, domName);
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::QueryCoords
//
//  Purpose:
//      Passes this request on to the queryable source. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 25, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jun 10 18:29:08 PDT 2004
//    Renamed from QueryZoneCenter to QueryCoords, added bool arg.
//
//   Kathleen Bonnell, Thu Dec 16 17:11:19 PST 2004 
//   Added another bool arg. 
//
//   Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//   Added const char* arg. 
//
//    Kathleen Bonnell, Mon Mar  7 10:19:17 PST 2005
//    Ensure a database variable is being used.
//
// ****************************************************************************

bool
avtExpressionEvaluatorFilter::QueryCoords(const std::string &var, 
    const int dom, const int id, const int ts, double c[3], const bool forZone,
    const bool useGlobalId, const char* mn)
{
    string dbVar = ParsingExprList::GetRealVariable(var);
    return GetInput()->GetQueryableSource()->
        QueryCoords(dbVar, dom, id, ts, c, forZone, useGlobalId, mn);
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::ExamineContract
//
//  Purpose:
//    Retrieve the current timestep and save for filters.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 25, 2004 
//
//  Modifications:
//
//    Hank Childs, Mon Mar 23 11:02:55 CDT 2009
//    Examine whether we are doing on demand processing.
//
// ****************************************************************************

void
avtExpressionEvaluatorFilter::ExamineContract(avtContract_p contract)
{
    currentTimeState = contract->GetDataRequest()->GetTimestep();
    onDemandProcessing = contract->DoingOnDemandStreaming();
    replicateSingleDomainOnAllProcessors = contract->ReplicateSingleDomainOnAllProcessors();
}

// ****************************************************************************
//  Method:  avtExpressionEvaluatorFilter::FilterUnderstandsTransformedRectMesh
//
//  Purpose:
//    If this filter returns true, this means that it correctly deals
//    with rectilinear grids having an implied transform set in the
//    data attributes.  It can do this conditionally if desired.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
bool
avtExpressionEvaluatorFilter::FilterUnderstandsTransformedRectMesh()
{
    // If all of the created filters can understand a transformed
    // rectilinear mesh, then return true.
    vector<avtExpressionFilter*> &filters = pipelineState.GetFilters();
    for (size_t i=0; i<filters.size(); i++)
    {
        if (!filters[i]->FilterUnderstandsTransformedRectMesh())
            return false;
    }

    return true;
}
