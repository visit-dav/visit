// ************************************************************************* //
//                            avtDatasetVerifier.C                           //
// ************************************************************************* //

#include <avtDatasetVerifier.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
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
            CorrectVarMismatch(ds, pt_var, true);
            IssueVarMismatchWarning(nscalars, nPts, true, dom);
        }
    }

    int nCellVars = ds->GetCellData()->GetNumberOfArrays();
    for (i = 0 ; i < nCellVars ; i++)
    {
        vtkDataArray *cell_var = ds->GetCellData()->GetArray(i);
        int nscalars = cell_var->GetNumberOfTuples();
        if (nscalars != nCells)
        {
            CorrectVarMismatch(ds, cell_var, false);
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
                IssueVarMismatchWarning(nscalars, nCells, false, dom);
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
// ****************************************************************************

void
avtDatasetVerifier::IssueVarMismatchWarning(int nVars, int nUnits,
                                            bool isPoint, int dom)
{
    if (issuedWarningForVarMismatch)
    {
        return;
    }

    const char *unit_string = (isPoint ? "nodal" : "zonal");
    const char *action = ((nVars < nUnits)
                          ? "Extra 0.'s were added"
                          : "Some values were removed");

    char msg[1024];
    sprintf(msg, "In domain %d, your %s variable has %d values, but it should "
                 "have %d.  %s to ensure VisIt runs smoothly.",
                 dom, unit_string, nVars, nUnits, action);
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
//      ds       The dataset.
//      scalar   The original scalar variable.
//      isPoint  true if it is a ptvar, false otherwise.
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
// ****************************************************************************

void
avtDatasetVerifier::CorrectVarMismatch(vtkDataSet *ds, vtkDataArray *scalar,
                                       bool isPoint)
{
    int destNum = 0;
    vtkFloatArray *newScalar = vtkFloatArray::New();

    if (isPoint)
    {
        destNum = ds->GetNumberOfPoints();
    }
    else
    {
        destNum = ds->GetNumberOfCells();
    }
    newScalar->SetNumberOfTuples(destNum);

    const char *name = scalar->GetName();
    if (name != NULL)
    {
        newScalar->SetName(name);
    }

    int   origNum = scalar->GetNumberOfTuples();
    for (int i = 0 ; i < destNum ; i++)
    {
        if (i < origNum)
        {
            newScalar->SetValue(i, scalar->GetTuple1(i));
        }
        else
        {
            newScalar->SetValue(i, 0.);
        }
    }

    //
    // Now make the new scalar be the official scalar.  We couldn't do this
    // before since registering it may have freed the original scalar and we
    // still wanted it.
    //
    if (isPoint)
    {
        ds->GetPointData()->SetScalars(newScalar);
    }
    else
    {
        ds->GetCellData()->SetScalars(newScalar);
    }
    newScalar->Delete();
}


