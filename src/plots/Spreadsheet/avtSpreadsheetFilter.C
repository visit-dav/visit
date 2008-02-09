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
//                              avtSpreadsheetFilter.C                       //
// ************************************************************************* //

#include <avtSpreadsheetFilter.h>

#include <vtkDataSet.h>
#include <string>

#include <InvalidSetException.h>
#include <InvalidVariableException.h>

#include <avtSILRestrictionTraverser.h>
#include <DebugStream.h>

// ****************************************************************************
//  Method: avtSpreadsheetFilter constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtSpreadsheetFilter::avtSpreadsheetFilter()
{
}

// ****************************************************************************
//  Method: avtSpreadsheetFilter destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

avtSpreadsheetFilter::~avtSpreadsheetFilter()
{
}

// ****************************************************************************
// Method: avtSpreadsheetFilter::SetAtts
//
// Purpose: 
//   Set the attributes used for the filter.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 17:17:33 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtSpreadsheetFilter::SetAtts(const SpreadsheetAttributes &a)
{
    atts = a;
}

// ****************************************************************************
// Method: avtSpreadsheetFilter::ModifyContract
//
// Purpose: 
//   Turns off all domains except for the specified subset.
//
// Arguments:
//   spec : The pipeline specification.
//
// Returns:    A new pipeline specification.
//
// Note:       This filter overrides any SIL selection that has taken place.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 17:16:40 PST 2007
//
// Modifications:
//   Brad Whitlock, Wed Mar 28 18:25:33 PST 2007
//   Force MIR to be off.
//
//   Gunther H. Weber, Mon Feb  4 14:41:06 PST 2008
//   Use doubles if they are a file's native precision
//
// ****************************************************************************

avtContract_p
avtSpreadsheetFilter::ModifyContract(avtContract_p spec)
{
    const char *mName = "avtSpreadsheetFilter::ModifyContract: ";

    avtContract_p rv = new avtContract(spec);
    std::string firstName;
    avtSILRestriction_p silr = spec->GetDataRequest()->GetRestriction();
    avtSILSet_p current = silr->GetSILSet(silr->GetTopSet());
    const std::vector<int> &mapsOut = current->GetMapsOut();
    int nSets = 1;
    int setId = -1, firstSetId = -1;
    for(int j = 0; j < mapsOut.size() && setId==-1; ++j)
    {
        int cIndex = mapsOut[j];
        avtSILCollection_p collection = silr->GetSILCollection(cIndex);
        if(*collection != NULL && collection->GetRole() == SIL_DOMAIN) 
        {
            const std::vector<int> &setIds = collection->GetSubsetList();
            nSets = setIds.size();
            for(int si = 0; si < setIds.size() && setId==-1; ++si)
            {
                if(firstSetId == -1)
                    firstSetId = setIds[si];
                if(silr->GetSILSet(setIds[si])->GetName() == 
                   atts.GetSubsetName())
                {
                    setId = setIds[si];
                }
            }
            break;
        }
    }

    //
    // If we've identified which set to use, do something with it now.
    //
    silr = rv->GetDataRequest()->GetRestriction();
    if(setId != -1)
    {
        debug1 << mName << "The set " << atts.GetSubsetName()
               << " was okay. Its setid = " << setId << endl;

        // The set name that was provided is okay.
        silr->TurnOffAll();
        silr->TurnOnSet(setId);
    }
    else
    {
        debug1 << mName << "The set " << atts.GetSubsetName()
               << " was not okay. ";

        // The set name that was provided is not okay.
        if(nSets > 1)
        {
            // Use the first set.
            if(firstSetId != -1)
            {
                debug1 << "One domain so turn on all sets" << endl;
                silr->TurnOffAll();
                silr->TurnOnSet(firstSetId);
            }
            else
            {
                EXCEPTION1(InvalidSetException, atts.GetSubsetName().c_str());
            }
        }
        else
        {
            debug1 << "One domain so turn on all sets" << endl;

            // Turn on all sets.
            silr->TurnOnAll();
        }
    }

    // Force material interface reconstruction to be off.
    rv->GetDataRequest()->ForceMaterialInterfaceReconstructionOff();
    rv->GetDataRequest()->SetNeedMixedVariableReconstruction(false);

    // Force native precision
    rv->GetDataRequest()->SetNeedNativePrecision(true);

    // Add double to the permissible types
    vector<int> adTypes;
    adTypes.push_back(VTK_FLOAT);
    adTypes.push_back(VTK_DOUBLE);
    rv->GetDataRequest()->UpdateAdmissibleDataTypes(adTypes);

    return rv;
}

// ****************************************************************************
//  Method: avtSpreadsheetFilter::ExecuteData
//
//  Purpose:
//      Does the actual VTK code to modify the dataset.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Feb 6 12:08:42 PDT 2007
//
// ****************************************************************************

void
avtSpreadsheetFilter::Execute(void)
{
    avtDataObject_p input = GetInput();
    GetOutput()->Copy(*input);
}

