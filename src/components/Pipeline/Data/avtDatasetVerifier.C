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
// ****************************************************************************

void
avtDatasetVerifier::VerifyDataset(vtkDataSet *ds, int dom)
{
    int  i, j;
    if (ds->IsA("vtkRectilinearGrid"))
    {
        vtkDataArray *coords = ((vtkRectilinearGrid*)ds)->GetXCoordinates();
        if (coords->GetDataType() != VTK_FLOAT)
        {
            vtkFloatArray *c = vtkFloatArray::New();
            c->SetNumberOfComponents(coords->GetNumberOfComponents());
            c->SetNumberOfTuples(coords->GetNumberOfTuples());
            for (i = 0; i < coords->GetNumberOfTuples(); i++)
               c->SetTuple(i, coords->GetTuple(i));
            ((vtkRectilinearGrid*)ds)->SetXCoordinates(c);
            c->Delete();
        }
        coords = ((vtkRectilinearGrid*)ds)->GetYCoordinates();
        if (coords->GetDataType() != VTK_FLOAT)
        {
            vtkFloatArray *c = vtkFloatArray::New();
            c->SetNumberOfComponents(coords->GetNumberOfComponents());
            c->SetNumberOfTuples(coords->GetNumberOfTuples());
            for (i = 0; i < coords->GetNumberOfTuples(); i++)
               c->SetTuple(i, coords->GetTuple(i));
            ((vtkRectilinearGrid*)ds)->SetYCoordinates(c);
            c->Delete();
        }
        coords = ((vtkRectilinearGrid*)ds)->GetZCoordinates();
        if (coords->GetDataType() != VTK_FLOAT)
        {
            vtkFloatArray *c = vtkFloatArray::New();
            c->SetNumberOfComponents(coords->GetNumberOfComponents());
            c->SetNumberOfTuples(coords->GetNumberOfTuples());
            for (i = 0; i < coords->GetNumberOfTuples(); i++)
               c->SetTuple(i, coords->GetTuple(i));
            ((vtkRectilinearGrid*)ds)->SetZCoordinates(c);
            c->Delete();
        }
    }
    else if (ds->IsA("vtkPointSet"))
    {
        vtkPoints *pts = ((vtkPointSet*)ds)->GetPoints();
        if (pts != NULL && pts->GetDataType() != VTK_FLOAT)
        {
            vtkPoints *p = vtkPoints::New();
            p->SetDataTypeToFloat();
            p->SetNumberOfPoints(pts->GetNumberOfPoints());
            for (i = 0; i < pts->GetNumberOfPoints(); i++)
               p->SetPoint(i, pts->GetPoint(i));
            ((vtkPointSet*)ds)->SetPoints(p);
            p->Delete();
        }
    }

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
            IssueVarMismatchWarning(nscalars, nPts, true, dom);
        }
        if (pt_var->GetDataType() == VTK_UNSIGNED_CHAR &&
            strncmp(pt_var->GetName(), "avt", 3) != 0)
        {
            vtkFloatArray *c = vtkFloatArray::New();
            c->SetNumberOfComponents(pt_var->GetNumberOfComponents());
            c->SetNumberOfTuples(pt_var->GetNumberOfTuples());
            c->SetName(pt_var->GetName());
            for (i = 0; i < pt_var->GetNumberOfTuples(); i++)
               c->SetTuple(i, pt_var->GetTuple(i));
      
            // as long as names match, AddArray will replace. 
            ds->GetPointData()->AddArray(c);
            c->Delete();
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
        if (cell_var->GetDataType() == VTK_UNSIGNED_CHAR &&
            strncmp(cell_var->GetName(), "avt", 3) != 0)
        {
            vtkFloatArray *c = vtkFloatArray::New();
            c->SetNumberOfComponents(cell_var->GetNumberOfComponents());
            c->SetNumberOfTuples(cell_var->GetNumberOfTuples());
            c->SetName(cell_var->GetName());
            for (i = 0; i < cell_var->GetNumberOfTuples(); i++)
               c->SetTuple(i, cell_var->GetTuple(i));
      
            // as long as names match, AddArray will replace. 
            ds->GetCellData()->AddArray(c);
            c->Delete();
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
// ****************************************************************************

void
avtDatasetVerifier::CorrectVarMismatch(vtkDataArray *var,
                                       vtkDataSetAttributes *atts, int destNum)
{
    int i;
    bool isActiveAttribute = false;
    int nComponents = var->GetNumberOfComponents();

    vtkDataArray *newVar = var->NewInstance();
    newVar->SetNumberOfComponents(nComponents);
    newVar->SetNumberOfTuples(destNum);

    const char *name = var->GetName();

    if (name != NULL)
    {
        newVar->SetName(name);
        //
        // Determine if this var is the active Attribute 
        // (Scalar,Vector, Tensor)
        //
        const char *attributeName;
        if (nComponents == 1 && atts->GetScalars())
            attributeName = atts->GetScalars()->GetName();
        else if (nComponents == 3 && atts->GetVectors())
            attributeName = atts->GetVectors()->GetName();
        else if (nComponents == 9 && atts->GetTensors())
            attributeName = atts->GetTensors()->GetName();
        else 
            attributeName = "";
        isActiveAttribute = (strcmp(attributeName, name) == 0); 
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
    // still wanted it.  Set* replaces the currently active Attribute 
    // (Vectors, Scalars), so only do it if the var we are replacing was the 
    // active Attribute, otherwise use AddArray to replace the old var.
    //
    if (!isActiveAttribute)
    {
        atts->AddArray(newVar);
    }
    else if (nComponents == 1)
    {
        atts->SetScalars(newVar);
    }
    else if (nComponents == 3)
    {
        atts->SetVectors(newVar);
    }
    else if (nComponents == 9)
    {
        atts->SetTensors(newVar);
    }
    if (nComponents > 1)
    {
        delete [] defaultTuple; 
    }
    newVar->Delete();
}


