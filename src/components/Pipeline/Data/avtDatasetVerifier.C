// ************************************************************************* //
//                            avtDatasetVerifier.C                           //
// ************************************************************************* //

#include <avtDatasetVerifier.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

#include <avtCallback.h>


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
//  Method: avtDatasetVerifier::VerifyDataTree
//
//  Purpose:
//      Verifies that every VTK dataset in the tree is valid (like the number
//      of elements in the variable makes sense for the mesh, etc).
//
//  Arguments:
//      tree    The tree to verify.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
// ****************************************************************************

void
avtDatasetVerifier::VerifyDataTree(avtDataTree_p &tree)
{
    int nLeaves;
    vtkDataSet **ds = tree->GetAllLeaves(nLeaves);

    for (int i = 0 ; i < nLeaves ; i++)
    {
        VerifyDataset(ds[i]);
    }

    delete [] ds;
}


// ****************************************************************************
//  Method: avtDatasetVerifier::VerifyDataset
//
//  Purpose:
//      Performs the verification of a single VTK datsaet.
//
//  Arguments:
//      ds      A single vtk dataset.
//
//  Programmer: Hank Childs
//  Creation:   October 18, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

void
avtDatasetVerifier::VerifyDataset(vtkDataSet *ds)
{
    int nPts   = ds->GetNumberOfPoints();
    int nCells = ds->GetNumberOfCells();

    vtkDataArray *pt_var   = ds->GetPointData()->GetScalars();
    vtkDataArray *cell_var = ds->GetCellData()->GetScalars();

    if (pt_var != NULL)
    {
        int nScalars = pt_var->GetNumberOfTuples();
        if (nScalars != nPts)
        {
            CorrectVarMismatch(ds, pt_var, true);
            IssueVarMismatchWarning(nScalars, nPts, true);
        }
    }

    if (cell_var != NULL)
    {
        int nScalars = cell_var->GetNumberOfTuples();
        if (nScalars != nCells)
        {
            CorrectVarMismatch(ds, cell_var, false);
            IssueVarMismatchWarning(nScalars, nCells, false);
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
//
//  Programmer:  Hank Childs
//  Creation:    October 18, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 16 10:02:10 PST 2003
//    Improve clarity of warning.
//
// ****************************************************************************

void
avtDatasetVerifier::IssueVarMismatchWarning(int nVars, int nUnits,bool isPoint)
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
    sprintf(msg, "Your %s variable has %d values, but it should have %d."
                 "%s to ensure VisIt runs smoothly.",
                 unit_string, nVars, nUnits, action);
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


