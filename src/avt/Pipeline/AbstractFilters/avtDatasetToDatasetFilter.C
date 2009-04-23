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
//                         avtDatasetToDatasetFilter.C                       //
// ************************************************************************* //

#include <cstdlib>
#include <cstring>

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
//    Kathleen Bonnell, Fri Jan  7 14:24:37 PST 2005 
//    Changed 'delete [] tmp' to 'free(tmp)' as the elements of the
//    secondaryVarList are created with strdup which uses malloc. 
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
        free(tmp);
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
//  Modifications:
//
//    Hank Childs, Tue Feb  1 16:56:42 PST 2005
//    Make sure that if we haven't asked for an "active variable", that we
//    get the "pipeline variable".
//
//    Jeremy Meredith, Thu Feb 15 11:55:03 EST 2007
//    Call inherited PreExecute before everything else.
//
// ****************************************************************************

void
avtDatasetToDatasetFilter::PreExecute(void)
{
    avtDatasetToDataObjectFilter::PreExecute();

    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();

    if (switchVariables)
    {
        InputSetActiveVariable(activeVariable);
    }
    else if (atts.ValidActiveVariable() &&
             atts.GetVariableName() != pipelineVariable)
    {
        InputSetActiveVariable(pipelineVariable);
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
//    Hank Childs, Tue Jul 27 08:56:44 PDT 2004
//    Call base class' PostExecute.
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

    avtDatasetToDataObjectFilter::PostExecute();
    avtDataObjectToDatasetFilter::PostExecute();
}


// ****************************************************************************
//  Method: avtDatasetToDatasetFilter::ExamineContract
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
avtDatasetToDatasetFilter::ExamineContract(avtContract_p s)
{
    avtDataRequest_p ds = s->GetDataRequest();

    // 
    // We need to know what the pipeline variable is so we can switch it
    // back when we are done.  It's also a nice thing to keep track of.
    // 
    if (pipelineVariable != NULL)
        delete[]pipelineVariable;

    const char *var = ds->GetVariable();
    pipelineVariable = new char[strlen(var) + 1];
    strcpy(pipelineVariable, var);

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
            debug5 << GetType() << ": ExamineContract: Setting primary "
                   << "variable " << activeVariable 
                   << " to be removed at PostExecute." << endl;
        } else
        {
            removeActiveVariableWhenDone = false;
            debug5 << GetType() << ": ExamineContract: Leaving primary "
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
            debug5 << GetType() << ": ExamineContract: Setting secondary "
                   << "variable " << secondaryVarList[i] 
                   << " to be removed at PostExecute." << endl;
        } else
        {
            removeSecondaryVariable[i] = false;
            debug5 << GetType() << ": ExamineContract: Leaving secondary "
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


