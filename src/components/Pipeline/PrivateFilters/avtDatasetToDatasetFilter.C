// ************************************************************************* //
//                         avtDatasetToDatasetFilter.C                       //
// ************************************************************************* //

#include <avtDatasetToDatasetFilter.h>

#include <avtCommonDataFunctions.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDatasetToDatasetFilter constructor
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov  2 16:32:00 PST 2001
//    Initialized removeActiveVariableWhenDone.
//
// ****************************************************************************

avtDatasetToDatasetFilter::avtDatasetToDatasetFilter()
{
    pipelineVariable = NULL;
    activeVariable   = NULL;
    switchVariables  = false;
    removeActiveVariableWhenDone = false;
}


// ****************************************************************************
//  Method: avtDatasetToDatasetFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Jun 28 13:23:13 PST 2002
//    Fixed a memory leak.
//
// ****************************************************************************

avtDatasetToDatasetFilter::~avtDatasetToDatasetFilter()
{
    if (pipelineVariable != NULL)
    {
        delete [] pipelineVariable;
        pipelineVariable = NULL;
    }
    if (activeVariable != NULL)
    {
        delete [] activeVariable;
        activeVariable = NULL;
    }
    for (int i=0;i<secondaryVarList.size();i++)
    {
        char *tmp = (char *)secondaryVarList[i];
        delete [] tmp;
    }
}


// ****************************************************************************
//  Method: avtDatasetToDatasetFilter::PreExecute
//
//  Purpose:
//      If we need to switch the active variable, then this will do that here
//      right before the execution takes place.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
// ****************************************************************************

void
avtDatasetToDatasetFilter::PreExecute(void)
{
    if (switchVariables)
    {
        InputSetActiveVariable(activeVariable);
    }
}


// ****************************************************************************
//  Method: avtDatasetToDatasetFilter::PostExecute
//
//  Purpose:
//      If we switched the active variable, then switch it back here since
//      execution is now done.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov  2 16:33:00 PST 2001
//    If appropriate, remove the active variable when done.
//
//    Sean Ahern, Wed Jun 26 18:46:27 PDT 2002
//    When appropriate, remove the secondary variables when done.
//
//    Hank Childs, Wed Feb 25 14:51:00 PST 2004
//    Remove the variables from the data attributes when done.
//
// ****************************************************************************

void
avtDatasetToDatasetFilter::PostExecute(void)
{
    if (switchVariables)
    {
        debug5 << GetType() << ": Setting output variable to be " 
               << pipelineVariable << " after execution." << endl;
        OutputSetActiveVariable(pipelineVariable);
        if (removeActiveVariableWhenDone)
        {
            debug5 << GetType() << ": Removing variable " << activeVariable
                   << " after execution." << endl;
            avtDataTree_p tree = GetDataTree();
            bool    success;
            tree->Traverse(CRemoveVariable, (void *)activeVariable, success);
            GetOutput()->GetInfo().GetAttributes(). 
                                                RemoveVariable(activeVariable);
        }
    }

    // Iterate through the secondary variables, removing them when needed.
    for (int i = 0; i < removeSecondaryVariable.size(); i++)
    {
        if (removeSecondaryVariable[i] == true)
        {
            debug5 << GetType() << ": Removing secondary variable "
                   << secondaryVarList[i] << " after execution." << endl;
            avtDataTree_p tree = GetDataTree();
            bool    success;
            tree->Traverse(CRemoveVariable, (void *)secondaryVarList[i],
                           success);
            GetOutput()->GetInfo().GetAttributes(). 
                                           RemoveVariable(secondaryVarList[i]);
        } else
        {
            debug5 << GetType() << ": Leaving secondary variable "
                   << secondaryVarList[i] << " after execution." << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtDatasetToDatasetFilter::ExamineSpecification
//
//  Purpose:
//      If we need to switch the active variable, then we need to know what
//      to switch it back to when we are done.  This is the only chance we
//      have to determine the variable name of the normal, "pipeline" variable.
//
//  Arguments:
//      s       A pipeline specification
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//
//    Hank Childs, Tue May 28 11:48:54 PDT 2002
//    Always capture the pipeline variable.
//
//    Sean Ahern, Tue Jun 25 19:13:16 PDT 2002
//    Added support for secondary variables.
//
// ****************************************************************************

void
avtDatasetToDatasetFilter::ExamineSpecification(avtPipelineSpecification_p s)
{
    avtDataSpecification_p ds = s->GetDataSpecification();

    // 
    // We need to know what the pipeline variable is so we can switch it
    // back when we are done.  It's also a nice thing to keep track of.
    // 
    if (pipelineVariable != NULL)
        delete[]pipelineVariable;

    const char *var = ds->GetVariable();
    pipelineVariable = new char[strlen(var) + 1];
    strcpy(pipelineVariable, var);

    const char *me = GetType();

    if (switchVariables)
    {
        // 
        // Determine if the pipeline already knows if it needs the
        // active variable.
        // 
        bool    haveVariable = false;
        if (strcmp(activeVariable, pipelineVariable) == 0)
        {
            haveVariable = true;
        }
        const vector<CharStrRef> &var2nd = ds->GetSecondaryVariables();
        for (int i = 0; i < var2nd.size(); i++)
        {
            const char *v2 = *(var2nd[i]);
            if (strcmp(v2, activeVariable) == 0)
            {
                haveVariable = true;
            }
        }

        // 
        // Tell the pipeline about our active variable (if necessary) and
        // decide if we should remove the active variable when we are done.
        // 
        if (!haveVariable)
        {
            ds->AddSecondaryVariable(activeVariable);
            removeActiveVariableWhenDone = true;
            debug5 << GetType() << ": ExamineSpecification: Setting primary "
                   << "variable " << activeVariable 
                   << " to be removed at PostExecute." << endl;
        } else
        {
            removeActiveVariableWhenDone = false;
            debug5 << GetType() << ": ExamineSpecification: Leaving primary "
                   << "variable " << activeVariable 
                   << " where it is at PostExecute." << endl;
        }
    }

    // Iterate through the secondary variables.
    // Determine if the pipline already knows about each of them.
    for (int i = 0; i < secondaryVarList.size(); i++)
    {
        bool    haveVariable = false;

        if (strcmp(pipelineVariable, secondaryVarList[i]) == 0)
        {
            haveVariable = true;
        }

        const vector<CharStrRef> &var2nd = ds->GetSecondaryVariables();
        for (int j = 0; j < var2nd.size(); j++)
        {
            const char *v2 = *(var2nd[j]);
            if (strcmp(v2, secondaryVarList[i]) == 0)
                haveVariable = true;
        }

        if (!haveVariable)
        {
            ds->AddSecondaryVariable(secondaryVarList[i]);
            removeSecondaryVariable[i] = true;
            debug5 << GetType() << ": ExamineSpecification: Setting secondary "
                   << "variable " << secondaryVarList[i] 
                   << " to be removed at PostExecute." << endl;
        } else
        {
            removeSecondaryVariable[i] = false;
            debug5 << GetType() << ": ExamineSpecification: Leaving secondary "
                   << "variable " << secondaryVarList[i] 
                   << " where it is at PostExecute." << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtDatasetToDatasetFilter::SetActiveVariable
//
//  Purpose:
//      Sets an active variable for the execution of this filter.  This is
//      only used when the active variable is not the one for the pipeline.
//      The actual setting takes place later, this only indicates that we
//      would like for it to be set.
//
//  Arguments:
//      varname    The name of the active variable.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2001
//
// ****************************************************************************

void
avtDatasetToDatasetFilter::SetActiveVariable(const char *varname)
{
    switchVariables = true;
    const char *me = GetType();
    if (activeVariable != NULL)
    {
        delete [] activeVariable;
    }
    activeVariable = new char[strlen(varname)+1];
    strcpy(activeVariable, varname);
}


// ****************************************************************************
//  Method: avtDatasetToDatasetFile::AddSecondaryVariable
//
//  Purpose:
//      Used by derived types to declare interest in an additional (ie
//      secondary) variable.  This class will then make sure this variable
//      is fetched properly.
//
//  Programmer: Hank Childs          <Added Header>
//  Creation:   September 22, 2003   <Header Creation Date>
//
// ****************************************************************************

void
avtDatasetToDatasetFilter::AddSecondaryVariable(const char *var)
{
    // Are we already asking for this in the activeVariable?
    if (strcmp(activeVariable, var) == 0)
        return;

    // Search through the existing secondary variables and see if it's
    // already there.
    for (int i=0;i<secondaryVarList.size();i++)
    {
        if (strcmp(secondaryVarList[i],var) == 0)
            return;
    }

    // Not found - add it to the list.
    const char *tvar = strdup(var);
    secondaryVarList.push_back(tvar);
    removeSecondaryVariable.push_back(false); // Doesn't matter what
                                              // value - but have to
                                              // allocate the memory for
                                              // later.

}


