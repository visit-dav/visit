/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>

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
// Method: avtNamedSelectionExtension::GetContract
//
// Purpose: 
//   Return the contract that we'll use for pipeline execution.
//
// Arguments:
//
// Returns:     
//
// Note:       We turn on zone numbers in the returned contract. If the prior
//             contract did not have zone numbers then we'll need to execute
//             the pipeline.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  6 15:44:42 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

avtContract_p
avtNamedSelectionExtension::GetContract(avtDataObject_p dob, bool &needsUpdate)
{
    avtContract_p c1 = dob->GetContractFromPreviousExecution();
    avtContract_p contract;
    if (c1->GetDataRequest()->NeedZoneNumbers() == false)
    {
        // If we don't have zone numbers, then get them, even if we have
        // to re-execute the whole darn pipeline.
        contract = new avtContract(c1);
        contract->GetDataRequest()->TurnZoneNumbersOn();
        needsUpdate = true;
    }
    else
    {
        contract = c1;
        needsUpdate = false;
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
//   
// ****************************************************************************

void
avtNamedSelectionExtension::GetSelection(avtDataObject_p dob, 
    const SelectionProperties &/*props*/,
    avtNamedSelectionCache &/*cache*/,
    std::vector<int> &doms, std::vector<int> &zones)
{
    const char *mName = "avtNamedSelectionExtension::GetSelection: ";
    bool needsUpdate = false;
    avtContract_p contract = GetContract(dob, needsUpdate);

    if (needsUpdate)
    {
        debug1 << mName << "Must re-execute pipeline to create named selection" << endl;
        dob->Update(contract);
        debug1 << mName << "Done re-executing pipeline to create named selection" << endl;
    }

    avtDataset_p ds;
    CopyTo(ds, dob);
    GetSelectionFromDataset(ds, doms, zones);
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
// ****************************************************************************

void
avtNamedSelectionExtension::GetSelectionFromDataset(avtDataset_p ds, 
    std::vector<int> &doms, std::vector<int> &zones)
{
    const char *mName = "avtNamedSelectionExtension::GetSelectionFromDataTree: ";
    int nleaves = 0;
    avtDataTree_p tree = ds->GetDataTree();
    vtkDataSet **leaves = tree->GetAllLeaves(nleaves);
    unsigned int idx = 0, maxSize = 0;
    for(int pass = 0; pass < 2; ++pass)
    {
        if(pass == 1)
        {
            doms.resize(maxSize);
            zones.resize(maxSize);
        }

        for (int i = 0 ; i < nleaves ; i++)
        {
            if (leaves[i]->GetNumberOfCells() == 0)
                continue;

            vtkDataArray *ocn = leaves[i]->GetCellData()->
                                                GetArray("avtOriginalCellNumbers");
            if (ocn == NULL)
            {
                // Write an error to the logs but don't fail out since we have
                // a collective communication coming up.
                debug5 << mName
                       << "This dataset has no original cell numbers so it cannot "
                          "contribute to the selection." << endl;
            }
            else
            {
                unsigned int *ptr = (unsigned int *) ocn->GetVoidPointer(0);
                if (ptr == NULL)
                {
                    // Write an error to the logs but don't fail out since we have
                    // a collective communication coming up.
                    debug5 << mName
                           << "This dataset has no original cell numbers so it "
                              "cannot contribute to the selection." << endl;
                }
                else
                {
                    unsigned int ncells = leaves[i]->GetNumberOfCells();
                    if(pass == 0)
                        maxSize += leaves[i]->GetNumberOfCells();
                    else
                    {
                        // We have original cell numbers so add them to the selection.
                        for (int j = 0 ; j < ncells ; j++)
                        {
                            doms[idx]  = ptr[2*j];
                            zones[idx] = ptr[2*j+1];
                            idx++;
                        }
                    }
                }
            }
        }
    }
    delete [] leaves;
}
