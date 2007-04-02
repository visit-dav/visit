// ************************************************************************* //
//                           avtExpressionEvaluatorFilter.C                  //
// ************************************************************************* //

#include <algorithm>
#include <vector>

#include <vtkDataArray.h>

#include <avtDatasetExaminer.h>
#include <avtExpressionEvaluatorFilter.h>
#include <avtExpressionFilter.h>
#include <avtIdentityFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTypes.h>

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
// ****************************************************************************

avtExpressionEvaluatorFilter::avtExpressionEvaluatorFilter()
{
    termsrc = NULL;
    currentTimeState = 0;
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
//    Use the timestep retrieved during ExamineSpecification to set the
//    timestep for the filters.
//
//    Hank Childs, Wed Dec 22 10:54:41 PST 2004
//    Consult the 'lastUsedSpec' to come up with the list of secondary 
//    variables that are needed downstream. ['5790]
//
//    Hank Childs, Fri Dec 31 11:47:01 PST 2004
//    Use a cached terminating source rather than one on the stack.
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
        avtPipelineSpecification_p pspec = GetGeneralPipelineSpecification();
        avtDataSpecification_p new_dspec = pspec->GetDataSpecification();
        avtDataSpecification_p old_dspec =lastUsedSpec->GetDataSpecification();
        new_dspec->AddSecondaryVariable(old_dspec->GetVariable());
        const vector<CharStrRef> &vars2nd=old_dspec->GetSecondaryVariables();
        for (int i = 0 ; i < vars2nd.size() ; i++)
        {
            new_dspec->AddSecondaryVariable(*(vars2nd[i]));
        }
        pspec->GetDataSpecification()->SetTimestep(currentTimeState);
        bottom->Update(pspec);
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
//    Hank Childs, Wed Dec 10 14:33:53 PST 2003
//    Added support for expressions of the form A=B.  This involves the 
//    identity function and was causing a crash previously.
//
//    Hank Childs, Sat Dec 13 15:55:31 PST 2003
//    Don't be so quick to declare something a recursive function.
//
//    Kathleen Bonnell, Tue Apr 27 11:34:23 PDT 2004 
//    Added test to determine if new filters need to be created.  If not,
//    don't set the pipelineSpec's DataObject to NULL, and don't create new
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
// ****************************************************************************

avtPipelineSpecification_p
avtExpressionEvaluatorFilter::PerformRestriction(
                                               avtPipelineSpecification_p spec)
{
    pipelineState.Clear();
    int   i;

    avtPipelineSpecification_p rv = spec;

    avtDataSpecification_p ds = spec->GetDataSpecification();
    bool createFilters = true;
    if (*lastUsedSpec != NULL)
    {
        avtDataSpecification_p ds1 = lastUsedSpec->GetDataSpecification();
        createFilters = pipelineState.GetFilters().size() == 0 ||
                        !ds1->VariablesAreTheSame(ds);
    }
    lastUsedSpec = spec;
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

    if (createFilters)
        pipelineState.SetDataObject(NULL);

    int num_recursive_checks = 0;
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
                EXCEPTION1(ImproperUseException, error);
            }
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
            avtExprNode *tree = dynamic_cast<avtExprNode*>
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

        vector<avtExpressionFilter*> &filters = pipelineState.GetFilters();
        avtExpressionFilter *f = NULL;
        if (filters.size() == numFiltersLastTime)
        {
            // The only way we can get here is if we have an expression of 
            // the form "A = B".
            debug1 << "Warning: expression logic identified expression of "
                   << "form A = B." << endl;
            avtIdentityFilter *ident = new avtIdentityFilter();
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
        numFiltersLastTime = filters.size();
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
// ****************************************************************************

void
avtExpressionEvaluatorFilter::Query(PickAttributes *pa)
{
    avtQueryableSource *src = GetInput()->GetQueryableSource();
    src->Query(pa);
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
    const int dom, const char *elType, float pt[3], int &elNum)
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
    const int dom, const int id, const int ts, float c[3], const bool forZone,
    const bool useGlobalId, const char* mn)
{
    string dbVar = ParsingExprList::GetRealVariable(var);
    return GetInput()->GetQueryableSource()->
        QueryCoords(dbVar, dom, id, ts, c, forZone, useGlobalId, mn);
}


// ****************************************************************************
//  Method: avtExpressionEvaluatorFilter::ExamineSpecification
//
//  Purpose:
//    Retrieve the current timestep and save for filters.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 25, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtExpressionEvaluatorFilter::ExamineSpecification(avtPipelineSpecification_p pspec)
{
    currentTimeState = pspec->GetDataSpecification()->GetTimestep();
}

