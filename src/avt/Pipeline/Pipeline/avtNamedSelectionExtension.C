/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
#include <avtNamedSelectionExtension.h>

#include <avtDataset.h>
#include <avtParallel.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <DebugStream.h>

avtNamedSelectionCacheItem::avtNamedSelectionCacheItem() : properties()
{
}

avtNamedSelectionCacheItem::~avtNamedSelectionCacheItem()
{
}


avtNamedSelectionExtension::avtNamedSelectionExtension()
{
}

avtNamedSelectionExtension::~avtNamedSelectionExtension()
{
}

// ****************************************************************************
// Method: avtNamedSelectionExtension::ModifyContract
//
// Purpose: 
//   Return the contract that we'll use for pipeline execution.
//
// Arguments:
//
// Returns:     
//
// Note:       We add different requests to the input contract and return a
//             changed copy of it.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  6 15:44:42 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 10:00:39 PDT 2011
//   I added support for different id methods.
//
// ****************************************************************************

avtContract_p
avtNamedSelectionExtension::ModifyContract(avtContract_p c0, 
    const SelectionProperties &props, bool &needsUpdate) const
{
    avtContract_p contract;
    
    if(props.GetIdVariableType() == SelectionProperties::UseZoneIDForID)
    {
        if (c0->GetDataRequest()->NeedZoneNumbers() == false)
        {
            // If we don't have zone numbers, then get them, even if we have
            // to re-execute the whole darn pipeline.
            contract = new avtContract(c0);
            contract->GetDataRequest()->TurnZoneNumbersOn();
            needsUpdate = true;
        }
        else
        {
            contract = c0;
            needsUpdate = false;
        }
    }
    else if(props.GetIdVariableType() == SelectionProperties::UseGlobalZoneIDForID)
    {
        if (c0->GetDataRequest()->NeedGlobalZoneNumbers() == false)
        {
            // If we don't have global zone numbers, then get them, even if we have
            // to re-execute the whole darn pipeline.
            contract = new avtContract(c0);
            contract->GetDataRequest()->TurnGlobalZoneNumbersOn();
            needsUpdate = true;
        }
        else
        {
            contract = c0;
            needsUpdate = false;
        } 
    }
    else if(props.GetIdVariableType() == SelectionProperties::UseVariableForID)
    {
        // Make sure that we request the id variable.
        if(c0->GetDataRequest()->HasSecondaryVariable(props.GetIdVariable().c_str()))
        {
            contract = c0;
            needsUpdate = false;
        }
        else
        {
            contract = new avtContract(c0);
            contract->GetDataRequest()->AddSecondaryVariable(props.GetIdVariable().c_str());
            needsUpdate = true;
        }
    }

    return contract;
}

// ****************************************************************************
// Method: avtNamedSelectionExtension::GetSelection
//
// Purpose: 
//   Return the selection as a set of domain and zone numbers.
//
// Arguments:
//   dob : The input to the selection's pipeline.
//   props : The selection properties (ignored for default case)
//   cache : The selection manager's cache (ignored)
//   doms  : The return vector of domains.
//   zones : The return vector of cells.
//
// Returns:    The cells that make up the selection are returned in doms/zones
//             vectors.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  6 16:02:40 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Oct 28 09:44:54 PDT 2011
//   I changed the code so it creates an avtNamedSelection object.
//
// ****************************************************************************

avtNamedSelection *
avtNamedSelectionExtension::GetSelection(avtDataObject_p dob, 
    const SelectionProperties &props,
    avtNamedSelectionCache &/*cache*/)
{
    const char *mName = "avtNamedSelectionExtension::GetSelection: ";
    avtContract_p c0 = dob->GetContractFromPreviousExecution();
    bool needsUpdate = false;
    avtContract_p contract = ModifyContract(c0, props, needsUpdate);

    if (needsUpdate)
    {
        debug1 << mName << "Must re-execute pipeline to create named selection" << endl;
        dob->Update(contract);
        debug1 << mName << "Done re-executing pipeline to create named selection" << endl;
    }

    // Get this processor's contribution
    avtDataset_p ds;
    CopyTo(ds, dob);
    avtNamedSelection *ns = GetSelectionFromDataset(ds, props);

    TRY
    {
        // Make sure all processors have the same selection.
        ns->Globalize();
    }
    CATCH(VisItException)
    {
        delete ns;
        RETHROW;
    }
    ENDTRY

    return ns;
}

// ****************************************************************************
// Method: avtNamedSelectionExtension::GetIdVariable
//
// Purpose: 
//   Return the name of the id variable that we'll use for the selection.
//
// Arguments:
//   props : The selection properties.
//
// Returns:    The name of the id variable that we'll use when accessing cell data.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 28 13:28:51 PDT 2011
//
// Modifications:
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Support for location named selections.
//   
// ****************************************************************************

std::string
avtNamedSelectionExtension::GetIdVariable(const SelectionProperties &props)
{
    std::string idName;
    if(props.GetIdVariableType() == SelectionProperties::UseZoneIDForID)
        idName = "avtOriginalCellNumbers";
    else if(props.GetIdVariableType() == SelectionProperties::UseLocationsForID)
        idName = "";
    else if(props.GetIdVariableType() == SelectionProperties::UseGlobalZoneIDForID)
        idName = "avtGlobalZoneNumbers";
    else if(props.GetIdVariableType() == SelectionProperties::UseVariableForID)
        idName = props.GetIdVariable();
    return idName;
}

// ****************************************************************************
// Method: avtNamedSelectionExtension::GetSelectionFromDataset
//
// Purpose: 
//   Convert the data in the data tree into a selection stored in int vectors.
//
// Arguments:
//   ds    : The dataset.
//   doms  : The return vector containing the domains.
//   zones : The return vector containing the zones.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  6 15:45:54 PDT 2011
//
// Modifications:
//   Brad Whitlock, Tue Sep  6 15:54:01 PDT 2011
//   I moved it from the named selection manager and changed it to a 2 pass
//   scheme to remove successive calls to resize.
//
//   Brad Whitlock, Fri Oct 28 10:48:34 PDT 2011
//   I rewrote it.
//
//    Dave Pugmire, Thu Mar 15 10:55:22 EDT 2012
//    Support for location named selections.
//
// ****************************************************************************

avtNamedSelection *
avtNamedSelectionExtension::GetSelectionFromDataset(avtDataset_p ds, 
                                                    const SelectionProperties &props)
{
    const char *mName = "avtNamedSelectionExtension::GetSelectionFromDataTree: ";

    avtNamedSelection *ns = NULL;
    std::string idName;
    if(props.GetIdVariableType() == SelectionProperties::UseZoneIDForID)
        ns = new avtZoneIdNamedSelection(props.GetName());
    else if(props.GetIdVariableType() == SelectionProperties::UseGlobalZoneIDForID)
        ns = new avtFloatingPointIdNamedSelection(props.GetName());
    else if(props.GetIdVariableType() == SelectionProperties::UseVariableForID)
        ns = new avtFloatingPointIdNamedSelection(props.GetName());
    else if(props.GetIdVariableType() == SelectionProperties::UseLocationsForID)
        ns = new avtLocationsNamedSelection(props.GetName());
    ns->SetIdVariable(GetIdVariable(props));

    int nleaves = 0;
    avtDataTree_p tree = ds->GetDataTree();
    vtkDataSet **leaves = tree->GetAllLeaves(nleaves);
    unsigned int maxSize = 0;

    for(int pass = 0; pass < 2; ++pass)
    {
        // pass 0 calculated size, now allocate space.
        if (pass == 1)
            ns->Allocate(maxSize);
            
        for (int i = 0 ; i < nleaves ; i++)
        {
            if (pass == 0)
            {
                if (ns->CheckValid(leaves[i]))
                {
                    if (ns->GetType() == avtNamedSelection::LOCATIONS)
                        maxSize += leaves[i]->GetNumberOfPoints();
                    else
                        maxSize += leaves[i]->GetNumberOfCells();
                }
                else
                {
                    // Write an error to the logs but don't fail out since we have
                    // a collective communication coming up.
                    debug5 << mName
                           << "This dataset does not have the id variable "
                           << ns->GetIdVariable() << " so it cannot contribute "
                           << "to the selection." << endl;
                }
            }
            else // pass == 1
            {
                ns->Append(leaves[i]);
            }
        }
    }
    delete [] leaves;

    return ns;
}
