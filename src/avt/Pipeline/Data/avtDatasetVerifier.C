/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                            avtDatasetVerifier.C                           //
// ************************************************************************* //

#include <avtDatasetVerifier.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDatasetVerifier constructor
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
// ****************************************************************************

avtDatasetVerifier::avtDatasetVerifier()
{
    issuedWarningForVarMismatch   = false;
}


// ****************************************************************************
//  Method: avtDatasetVerifier destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetVerifier::~avtDatasetVerifier()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetVerifier::VerifyDatasets
//
//  Purpose:
//      Verifies that every VTK dataset in the list is valid (like the number
//      of elements in the variable makes sense for the mesh, etc).
//
//  Arguments:
//      nlist   The number of elements in the list.
//      list    The list of datasets.
//      domains The domain number of each dataset.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jan  9 09:40:32 PST 2004
//    Renamed function and added arguments; made routine easily usable by
//    the database as well the terminating dataset source.
//
// ****************************************************************************

void
avtDatasetVerifier::VerifyDatasets(int nlist, vtkDataSet **list, 
                                   std::vector<int> &domains)
{
    for (int i = 0 ; i < nlist ; i++)
    {
        if (list[i] != NULL)
        {
            VerifyDataset(list[i], domains[i]);
        }
    }
}


// ****************************************************************************
//  Method: avtDatasetVerifier::VerifyDataset
//
//  Purpose:
//      Performs the verification of a single VTK datsaet.
//
//  Arguments:
//      ds      A single vtk dataset.
//      dom     The domain number.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Fri Jan  9 09:43:13 PST 2004
//    Iterate over all variables.  Added an argument for the domain number.
//    Also do not issue a warning if the missing values are for ghost zones.
//
//    Hank Childs, Fri Aug 27 15:32:06 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Bonnell, Fri Nov 12 08:22:29 PST 2004
//    Changed args being sent to CorrectVarMismatch, so that the method can
//    handle more var types. 
//
//    Kathleen Bonnell, Tue Feb  1 10:36:59 PST 2005 
//    Convert non-float Points to float.  Convert unsigned char data to float. 
//
//    Hank Childs, Tue Jul  5 16:22:56 PDT 2005
//    Add variable names to warning message ['6368].
//
//    Kathleen Bonnell, Mon Nov 13 17:32:26 PST 2006 
//    Regrab 'pt_var' or 'cell_var' after call to CorrectVarMismatch, as this
//    method invalidates the pointers.
//
//    Hank Childs, Wed Aug 27 14:51:09 PDT 2008
//    No longer worry about type conversion.  That is the transform managers
//    job now.
//
// ****************************************************************************

void
avtDatasetVerifier::VerifyDataset(vtkDataSet *ds, int dom)
{
    int  i, j;

    int nPts   = ds->GetNumberOfPoints();
    int nCells = ds->GetNumberOfCells();

    int nPtVars = ds->GetPointData()->GetNumberOfArrays();
    for (i = 0 ; i < nPtVars ; i++)
    {
        vtkDataArray *pt_var = ds->GetPointData()->GetArray(i);
        int nscalars = pt_var->GetNumberOfTuples();
        if (nscalars != nPts)
        {
            CorrectVarMismatch(pt_var, ds->GetPointData(), nPts); 
            // CorrectVarMismatch invalidates pt_var pointer. Grab it again.
            pt_var = ds->GetPointData()->GetArray(i);
            IssueVarMismatchWarning(nscalars, nPts,true,dom,pt_var->GetName());
        }
    }

    int nCellVars = ds->GetCellData()->GetNumberOfArrays();
    for (i = 0 ; i < nCellVars ; i++)
    {
        vtkDataArray *cell_var = ds->GetCellData()->GetArray(i);
        int nscalars = cell_var->GetNumberOfTuples();
        if (nscalars != nCells)
        {
            CorrectVarMismatch(cell_var, ds->GetCellData(), nCells);
            // CorrectVarMismatch invalidates cell_var pointer. Grab it again.
            cell_var = ds->GetCellData()->GetArray(i);
            bool issueWarning = true;
            vtkUnsignedCharArray *gz = (vtkUnsignedCharArray *)
                                 ds->GetCellData()->GetArray("avtGhostZones");
            if (gz != NULL)
            {
                int ntuples = gz->GetNumberOfTuples();
                int num_real = 0;
                for (j = 0 ; j < ntuples ; j++)
                {
                    if (gz->GetValue(j) == '\0')
                        num_real++;
                }
                if (num_real == nscalars)
                {
                    issueWarning = false;
                    debug1 << "The input file has an invalid number of "
                           << "entries in a zonal variable.  Since the number"
                           << " of entries corresponds to the number of real "
                           << "zones, no warning is being issued." << endl;
                }
            }
            if (issueWarning)
                IssueVarMismatchWarning(nscalars, nCells, false, dom, 
                                        cell_var->GetName());
        }
    }
}


// ****************************************************************************
//  Method: avtDatasetVerifier::IssueVarMismatchWarning
//
//  Purpose:
//      Issues a warning that variables are missing.
//
//  Arguments:
//      nVars    The number of values we got.
//      nUnits   The number we should have gotten.
//      isPoint  true if it is a ptvar, false otherwise.
//      dom      The domain number.
//
//  Programmer:  Hank Childs
//  Creation:    October 18, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 16 10:02:10 PST 2003
//    Improve clarity of warning.
//
//    Hank Childs, Fri Jan  9 09:43:13 PST 2004
//    Added argument dom.
//
//    Hank Childs, Tue Jul  5 16:18:02 PDT 2005
//    Added variable name. ['6368]
//
// ****************************************************************************

void
avtDatasetVerifier::IssueVarMismatchWarning(int nVars, int nUnits,bool isPoint,
                                            int dom, const char *varname)
{
    if (issuedWarningForVarMismatch)
    {
        return;
    }

    const char *vname = (varname != NULL ? varname : "<unnamed>");
    const char *unit_string = (isPoint ? "nodal" : "zonal");
    const char *action = ((nVars < nUnits)
                          ? "Extra 0.'s were added"
                          : "Some values were removed");

    char msg[1024];
    sprintf(msg, "In domain %d, your %s variable \"%s\" has %d values, but it "
                 "should have %d.  %s to ensure VisIt runs smoothly.",
                 dom, unit_string, vname, nVars, nUnits, action);
    avtCallback::IssueWarning(msg);

    issuedWarningForVarMismatch = true;
}


// ****************************************************************************
//  Method: avtDatasetVerifier::CorrectVarMismatch
//
//  Purpose:
//      Corrects a variable mismatch.  Assigns 0 to the end.
//
//  Arguments:
//      var      The original variable.
//      atts     The point-data or cell-data associated with the dataset,
//               where the new variable needs to be placed.
//      destNum  The number of values needed in the destination variable array.
//
//  Programmer:  Hank Childs
//  Creation:    October 18, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Nov 13 11:39:52 PST 2001
//    Do not register the new scalars until we are entirely done with the
//    old scalar, to prevent it from being freed while we still want to access
//    it.
//
//    Hank Childs, Tue Mar 19 17:41:51 PST 2002
//    Pass on the name of the variable.
//
//    Kathleen Bonnell, Thu Mar 21 10:59:44 PST 2002 
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray and 
//    vtkFloatArray instead.
//
//    Kathleen Bonnell, Fri Nov 12 08:22:29 PST 2004
//    Changed args.  Reworked method to handle more than just Scalar vars.
//
//    Kathleen Bonnell, Mon Nov 13 17:32:26 PST 2006 
//    Removed tests for 'isActiveAttribute'.  Always use 'AddArray' instead
//    of 'SetScalars' etc, because 'AddArray' overwrites the array in-place
//    preserving the order of the arrays in vtkDataSetAttributes, whereas 
//    'SetScalars' and the like DOES NOT.  This is important due to the
//    fact that this method is called from within a for-loop that is looping
//    over all the arrays in 'atts' by index number.  Want to preserve the
//    original ordering!  AddArray also does not destroy active attributes.
//
//    Jeremy Meredith, Thu Aug  7 14:45:00 EDT 2008
//    Removed unused var.
//
// ****************************************************************************

void
avtDatasetVerifier::CorrectVarMismatch(vtkDataArray *var,
                                       vtkDataSetAttributes *atts, int destNum)
{
    int i;
    int nComponents = var->GetNumberOfComponents();

    vtkDataArray *newVar = var->NewInstance();
    newVar->SetNumberOfComponents(nComponents);
    newVar->SetNumberOfTuples(destNum);

    const char *name = var->GetName();

    if (name != NULL)
    {
        newVar->SetName(name);
    }

    //
    // Create a default tuple to be used in the case when our new
    // variable requires more values than are contained in the original var.
    //
    float *defaultTuple = NULL;
    if (nComponents > 1)
    {
        defaultTuple = new float[nComponents];
        for (i = 0; i < nComponents; i++)
            defaultTuple[i] = 0.f; 
    } 
                 
    int   origNum = var->GetNumberOfTuples();
    for (i = 0 ; i < destNum ; i++)
    {
        if (i < origNum)
        {
            newVar->SetTuple(i, var->GetTuple(i));
        }
        else if (nComponents == 1)
        {
            newVar->SetTuple1(i, 0.f);
        }
        else 
        {
            newVar->SetTuple(i, defaultTuple);
        }
    }

    //
    // Now make the new var be the official var.  We couldn't do this
    // before since registering it may have freed the original var and we
    // still wanted it.  AddArray replaces the old var, preserving the
    // order of arrays as well as the 'active' attributes.
    //
    atts->AddArray(newVar);

    if (nComponents > 1)
    {
        delete [] defaultTuple; 
    }
    newVar->Delete();
}


