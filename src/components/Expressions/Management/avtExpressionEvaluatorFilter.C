// ************************************************************************* //
//                           avtExpressionEvaluatorFilter.C                  //
// ************************************************************************* //

#include <algorithm>
#include <vector>

#include <vtkDataArray.h>

#include <avtDatasetExaminer.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtExpressionFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTypes.h>

#include <PickAttributes.h>
#include <PickVarInfo.h>

#include <EngineExprNode.h>
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
//
// ****************************************************************************
void
avtExpressionEvaluatorFilter::Execute(void)
{
    // Start the timer going
    int  timingIndex = visitTimer->StartTimer();

    // Get my input and output for hooking up to the database.
    avtDataObject_p dObj = GetInput();

    avtDataset_p ds; 
    CopyTo(ds, dObj);
    avtSourceFromAVTDataset termsrc(ds);
    avtDataObject_p data = termsrc.GetOutput(); 

    // Do we have anything to do?  Did we apply any expression filters?
    if (pipelineState.GetFilters().size() != 0)
    {
        // Hook the input and output up to the pipeline in pipelineState.
        vector<avtExpressionFilter*> &filters = pipelineState.GetFilters();
        avtExpressionFilter *top =    filters.front();
        avtExpressionFilter *bottom = filters.back();

        top->SetInput(data);
        bottom->Update(GetGeneralPipelineSpecification());
        GetOutput()->Copy(*(bottom->GetOutput()));
    } else {
        GetOutput()->Copy(*dObj);
    }

    // Stop the timer
    visitTimer->StopTimer(timingIndex, "Expression Evaluator Filter");
    visitTimer->DumpTimings();
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
    return pipelineState.GetFilters().size();
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::PerformRestriction
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
// ****************************************************************************

avtPipelineSpecification_p
avtExpressionEvaluatorFilter::PerformRestriction(
                                               avtPipelineSpecification_p spec)
{
    int   i;

    avtPipelineSpecification_p rv = spec;
    lastUsedSpec = spec;
    avtDataSpecification_p ds = spec->GetDataSpecification();
    avtDataSpecification_p newds;

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
    for (i = 0; i != sv.size(); i++)
    {
        const char *str = *(sv[i]);
        candidates.insert(str);
    }

    // Walk through the candidates, processing the results into the real
    // list and the expression list.  When the variables are found, turn the
    // parsed expression into a list of filters.  These filters are hooked
    // together, then put on a list in pipelineState for use in Execute().
    debug5 << "EEF::PerformRestriction: Checking candidates" << endl;
    pipelineState.SetDataObject(NULL);
    while (!candidates.empty())
    {
        std::set<string>::iterator front = candidates.begin();
        string var = *front;
        candidates.erase(front);
        debug5 << "EEF::PerformRestriction:     candidate: " << var.c_str() 
               << endl;

        // Have we seen this before?
        std::vector<string>::iterator search;
        search = find(expr_list.begin(), expr_list.end(), var);
        if (search != expr_list.end())
        {
            // We've seen this expression already.
            char error[] = "Recursive expression.";
            debug1 << error << endl;
            EXCEPTION1(ImproperUseException, error);
        }

        // Check if this is an expression or a real variable.
        Expression *exp = ParsingExprList::GetExpression(var);
        if (exp == NULL)
        {
            debug5 << "EEF::PerformRestriction:     not an expression" << endl;
            // Not an expression.  Put the name on the real list.
            real_list.insert(var);
        } else {
            debug5 << "EEF::PerformRestriction:     expression.  Roots:" 
                   << endl;
            // Expression.  Put the name on the expr list.  Find the base
            // variables of the expression and put them on the candidate
            // list.
            expr_list.push_back(var);
            EngineExprNode *tree = dynamic_cast<EngineExprNode*>
                                    (ParsingExprList::GetExpressionTree(var));
            set<string> roots = tree->GetVarLeaves();
            while (!roots.empty())
            {
                std::set<string>::iterator front = roots.begin();
                debug5 << "EEF::PerformRestriction:         " << front->c_str()
                       << endl;
                candidates.insert(*front);
                roots.erase(front);
            }
        }
    }

    // Take the list of expressions and make the filters for them.
    while (!expr_list.empty())
    {
        std::vector<string>::iterator back = expr_list.end() - 1;
        string var = *back;
        expr_list.erase(back);
        
        // Get the expression tree again.  (We could save trees between the
        // first and second sections of the code.  It wouldn't save much
        // time, but would be cleaner.)
        EngineExprNode *tree = dynamic_cast<EngineExprNode*>
                                     (ParsingExprList::GetExpressionTree(var));

        // Create the filters that the tree uses.  Put them into the
        // filters stack in pipelineState.
        tree->CreateFilters(&pipelineState);

        vector<avtExpressionFilter*> &filters = pipelineState.GetFilters();
        avtExpressionFilter *bottom = filters.back();
        bottom->SetOutputVariableName(var.c_str());
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

    // Set up the data spec.
    it = real_list.begin();
    newds = new avtDataSpecification(ds);
    if (!haveActiveVariable)
        newds->SetDBVariable((*it).c_str());

    newds->RemoveAllSecondaryVariables();
    for ( ; it != real_list.end() ; it++)
        if (*it != ds->GetVariable())
            newds->AddSecondaryVariable((*it).c_str());

    rv = new avtPipelineSpecification(spec, newds);

    vector<avtExpressionFilter *> &filters = pipelineState.GetFilters();
    for (i = 0 ; i < filters.size() ; i++)
    {
        rv = filters[i]->PerformRestriction(rv);
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
// ****************************************************************************

void
avtExpressionEvaluatorFilter::ReleaseData(void)
{
    avtDatasetToDatasetFilter::ReleaseData();
    vector<avtExpressionFilter *> &filters = pipelineState.GetFilters();
    for (int i = 0 ; i < filters.size() ; i++)
    {
        filters[i]->ReleaseData();
    }
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
// ****************************************************************************

void
avtExpressionEvaluatorFilter::Query(PickAttributes *pa)
{
    int   i, j, k;

    //
    // Sanity check.
    //
    if (*(GetInput()) == NULL)
        EXCEPTION0(NoInputException);

    //
    // We need to identify if there are expression variables that have been
    // requested for the pick that are missing.  If so, we will have to
    // re-execute.
    //
    const stringVector &orig_vars = pa->GetVariables();
    stringVector expr_vars;
    std::vector<int> indices;
    for (i = 0 ; i < orig_vars.size() ; i++)
    {
        Expression *exp = ParsingExprList::GetExpression(orig_vars[i]);
        if (exp != NULL)
        {
            expr_vars.push_back(orig_vars[i]);
            indices.push_back(i);
        }
    }
    stringVector unmatched_vars;
    if (expr_vars.size() > 0)
    {
        avtDataset_p output = GetTypedOutput();
        VarList vl;
        avtDatasetExaminer::GetVariableList(output, vl);
        for (i = 0 ; i < expr_vars.size() ; i++)
        {
            bool foundMatch = false;
            for (j = 0 ; j < vl.nvars ; j++)
            {
                if (expr_vars[i] == vl.varnames[j])
                    foundMatch = true;
            }
            if (!foundMatch)
            {
                unmatched_vars.push_back(expr_vars[i]);
            }
        }
        modified = true;
    }
    if (unmatched_vars.size() > 0)
    {
        for (i = 0 ; i < unmatched_vars.size() ; i++)
            lastUsedSpec->GetDataSpecification()->AddSecondaryVariable(
                                                    unmatched_vars[i].c_str());

        //
        // Force the update.
        //
        GetOutput()->Update(lastUsedSpec);
    }

    //
    // Start by going to the new queryable source upstream (most likely the
    // terminating source corresponding to the database) and ask it to pick
    // with our new pick attributes.
    //
    avtQueryableSource *src = GetInput()->GetQueryableSource();
    src->Query(pa);

    //
    // Now iterate over the expressions and add where possible.
    //
    if (expr_vars.size() > 0)
    {
        bool canUseNativeArray = 
                      GetInput()->GetInfo().GetValidity().GetZonesPreserved();

        const intVector &incidentElements = pa->GetIncidentElements();
        int element = pa->GetElementNumber();
        int domain  = pa->GetDomain();
        bool zonePick = pa->GetPickType() == PickAttributes::Zone;

        avtDataset_p output = GetTypedOutput();
        for (i = 0 ; i < expr_vars.size() ; i++)
        {
            avtCentering cent = AVT_UNKNOWN_CENT;
            vtkDataArray *arr = avtDatasetExaminer::GetArray(output,
                                           expr_vars[i].c_str(), domain, cent);
            if (arr == NULL)
            {
                continue;
            }

            PickVarInfo varInfo;
            varInfo.SetVariableName(expr_vars[i]);
            int ncomps = arr->GetNumberOfComponents();
            if (ncomps == 1)
                varInfo.SetVariableType("scalar");
            else if (ncomps == 3)
                varInfo.SetVariableType("vector");
            else if (ncomps == 9)
                varInfo.SetVariableType("tensor");

            bool zoneCent = (cent == AVT_ZONECENT);
            varInfo.SetCentering(zoneCent ? PickVarInfo::Zonal
                                          : PickVarInfo::Nodal);
            std::vector<double> vals;
            std::vector<std::string> names;
            char temp[1024];
            if (canUseNativeArray)
            {
                if (zoneCent != zonePick)
                {
                    for (j = 0 ; j < incidentElements.size() ; j++)
                    {
                        sprintf(temp, "(%d)", incidentElements[j]);
                        names.push_back(temp);
                        float mag = 0.;
                        for (k = 0 ; k < arr->GetNumberOfComponents() ; k++)
                        {
                            float val=arr->GetComponent(incidentElements[j],k);
                            mag += val*val;
                            vals.push_back(val);
                        }
                        mag = sqrt(mag);
                        vals.push_back(mag);
                    }
                }
                else
                {
                    // the info we're after is associated with element
                    sprintf(temp, "(%d)", element);
                    names.push_back(temp);
                    float mag = 0.;
                    for (k = 0 ; k < arr->GetNumberOfComponents() ; k++)
                    {
                        float val = arr->GetComponent(element, k);
                        mag += val*val;
                        vals.push_back(val);
                    }
                    mag = sqrt(mag);
                    vals.push_back(mag);
                }
            }
            if (!vals.empty())
            {
                varInfo.SetNames(names);
                varInfo.SetValues(vals);
                int index = -1;
                for (int ii = 0 ; ii < pa->GetNumPickVarInfos() ; ii++)
                {
                    PickVarInfo &vi = pa->GetPickVarInfo(ii);
                    if (vi.GetVariableName() == varInfo.GetVariableName())
                    {
                        index = ii;
                        break;
                    }
                }
                if (index >= 0)
                {
                    PickVarInfo &varInfo2 = pa->GetPickVarInfo(index);
                    varInfo2 = varInfo;
                }
                else
                {
                    pa->AddPickVarInfo(varInfo);
                }
            }
        }
    }
}


