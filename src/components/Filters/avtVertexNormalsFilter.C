// ************************************************************************* //
//                          avtVertexNormalsFilter.C                         //
// ************************************************************************* //

#include <avtVertexNormalsFilter.h>

#include <vtkPolyData.h>
#include <vtkVisItPolyDataNormals.h>

#include <avtDataset.h>


// ****************************************************************************
//  Method: avtVertexNormalsFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the VertexNormals filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs 
//  Creation:   December 31, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Jun 20 13:59:01 PDT 2002
//    Be more careful about what sort of poly data gets sent in (no lines).
//
//    Hank Childs, Tue Aug  6 11:11:50 PDT 2002
//    Do not execute if normals have been deemed inappropriate.
//
//    Hank Childs, Sun Aug 18 11:14:20 PDT 2002
//    Do not find normals for disjoint element meshes.
//
//    Hank Childs, Tue Sep 10 15:17:21 PDT 2002
//    Manage the output's memory without the use of data members.
//
//    Jeremy Meredith, Thu Oct 24 17:40:18 PDT 2002
//    Upped the feature angle to 45 degrees.
//
//    Jeremy Meredith, Mon Jan  6 10:44:59 PST 2003
//    Disabled consistency checking because (1) it has bugs, (2) it is slow,
//    and (3) we are careful enough about cell orderings that we don't need it.
//
//    Jeremy Meredith, Wed Aug 13 18:13:14 PDT 2003
//    Made it use the new VisIt poly data normals filter.  Allowed
//    cell normals as well as just point normals.
//
// ****************************************************************************

vtkDataSet *
avtVertexNormalsFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 3 ||
        GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 2)
    {
        return in_ds;
    }

    if (GetInput()->GetInfo().GetValidity().NormalsAreInappropriate())
    {
        return in_ds;
    }

    if (GetInput()->GetInfo().GetValidity().GetDisjointElements() == true)
    {
        return in_ds;
    }

    if (in_ds->GetDataObjectType() != VTK_POLY_DATA)
    {
        return in_ds;
    }

    vtkPolyData *pd = (vtkPolyData *)in_ds;

    if (pd->GetNumberOfPolys() <= 0 && pd->GetNumberOfStrips() <= 0)
    {
        return in_ds;
    }

    vtkVisItPolyDataNormals *normals = vtkVisItPolyDataNormals::New();
    normals->SetInput(pd);
    normals->SetFeatureAngle(45.);
    if (pointNormals)
        normals->SetNormalTypeToPoint();
    else
        normals->SetNormalTypeToCell();
    normals->SetSplitting(true);
    normals->Update();

    vtkPolyData *out_ds = normals->GetOutput();
    ManageMemory(out_ds);
    normals->Delete();

    return out_ds;
}


