// ************************************************************************* //
//                           avtPointToGlyphFilter.C                         //
// ************************************************************************* //

#include <avtPointToGlyphFilter.h>

#include <vtkGlyph3D.h>
#include <vtkPolyData.h>
#include <vtkPolyDataRelevantPointsFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>

#include <avtDataset.h>


// ****************************************************************************
//  Method: avtPointToGlyphFilter constructor
//
//  Programmer: Hank Childs 
//  Creation:   June 22, 2002
//
// ****************************************************************************

avtPointToGlyphFilter::avtPointToGlyphFilter()
{
    pointSize         = 0.05;
    scaleVar          = "default";
    scaleByVarEnabled = false;
    
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(8);
    pts->SetPoint(0, -0.5, -0.5, -0.5);
    pts->SetPoint(1, +0.5, -0.5, -0.5);
    pts->SetPoint(2, +0.5, +0.5, -0.5);
    pts->SetPoint(3, -0.5, +0.5, -0.5);
    pts->SetPoint(4, -0.5, -0.5, +0.5);
    pts->SetPoint(5, +0.5, -0.5, +0.5);
    pts->SetPoint(6, +0.5, +0.5, +0.5);
    pts->SetPoint(7, -0.5, +0.5, +0.5);

    glyph3D = vtkPolyData::New();
    glyph3D->SetPoints(pts);
    pts->Delete();

    glyph3D->Allocate(24);
    vtkIdType ids[6][4] = { { 0, 1, 2, 3 }, { 1, 5, 6, 2 }, { 2, 6, 7, 3 },
                            { 3, 7, 4, 0 }, { 0, 4, 5, 1 }, { 4, 7, 6, 5 } };
    glyph3D->InsertNextCell(VTK_QUAD, 4, ids[0]);
    glyph3D->InsertNextCell(VTK_QUAD, 4, ids[1]);
    glyph3D->InsertNextCell(VTK_QUAD, 4, ids[2]);
    glyph3D->InsertNextCell(VTK_QUAD, 4, ids[3]);
    glyph3D->InsertNextCell(VTK_QUAD, 4, ids[4]);
    glyph3D->InsertNextCell(VTK_QUAD, 4, ids[5]);

    vtkPoints *pts2D = vtkPoints::New();
    pts2D->SetNumberOfPoints(4);
    pts2D->SetPoint(0, -0.5, -0.5, 0.);
    pts2D->SetPoint(1, +0.5, -0.5, 0.);
    pts2D->SetPoint(2, +0.5, +0.5, 0.);
    pts2D->SetPoint(3, -0.5, +0.5, 0.);

    glyph2D = vtkPolyData::New();
    glyph2D->SetPoints(pts2D);
    pts2D->Delete();

    glyph2D->Allocate(4);
    vtkIdType ids2D[4] = { 0, 1, 2, 3};
    glyph2D->InsertNextCell(VTK_QUAD, 4, ids2D);
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter destructor
//
//  Programmer: Hank Childs
//  Creation:   June 22, 2002
//
// ****************************************************************************

avtPointToGlyphFilter::~avtPointToGlyphFilter()
{
    if (glyph3D != NULL)
    {
        glyph3D->Delete();
        glyph3D = NULL;
    }
    if (glyph2D != NULL)
    {
        glyph2D->Delete();
        glyph2D = NULL;
    }
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::SetPointSize
//
//  Purpose:
//      Sets the point size.
//
//  Programmer: Hank Childs
//  Creation:   June 23, 2002
//
// ****************************************************************************

void
avtPointToGlyphFilter::SetPointSize(double p)
{
    pointSize = p;
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::SetScaleVariable
//
//  Purpose:
//      Sets the variable to scale point size by.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 19, 2002
//
// ****************************************************************************

void
avtPointToGlyphFilter::SetScaleVariable(const std::string &s)
{
    scaleVar = s;
    if (scaleByVarEnabled && scaleVar != "default")
        SetActiveVariable(scaleVar.c_str());
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::SetScaleByVariableEnabled
//
//  Purpose:
//      Sets whether or not to scale points by a variable.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 19, 2002
//
// ****************************************************************************

void
avtPointToGlyphFilter::SetScaleByVariableEnabled(bool s)
{
    scaleByVarEnabled = s;
    if (scaleByVarEnabled && scaleVar != "default")
        SetActiveVariable(scaleVar.c_str());
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the PointToGlyph filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output dataset.
//
//  Programmer: Hank Childs 
//  Creation:   June 22, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Sep 10 19:47:03 PDT 2002
//    Cleaned up memory management.
//
//    Hank Childs, Thu Oct 31 07:28:45 PST 2002
//    Make sure that we only glyph the relevant points.
//
//    Jeremy Meredith, Fri Dec 20 11:24:35 PST 2002
//    Added code to support scaling by a variable.
//    Added code from Hank Childs to tell it how to copy other variables.
//
//    Hank Childs, Wed Mar 26 13:56:53 PST 2003
//    Add early detection for the case where there are 0 input cells.
//
// ****************************************************************************

vtkDataSet *
avtPointToGlyphFilter::ExecuteData(vtkDataSet *in_ds, int, std::string)
{
    if (in_ds == NULL || in_ds->GetNumberOfCells() <= 0)
    {
        return NULL;
    }
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 0)
    {
        return in_ds;
    }

    vtkPolyData *glyph = NULL;
    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
    {
        glyph = glyph2D;
    }
    else
    {
        glyph = glyph3D;
    }

    //
    // The glyph filter will glyph every input point, regardless of whether or
    // not it is being used.  If we have poly data or an unstructured grid,
    // we likely have unused points.
    //
    vtkPolyDataRelevantPointsFilter *pdrp =
                                        vtkPolyDataRelevantPointsFilter::New();
    vtkUnstructuredGridRelevantPointsFilter *ugrp =
                                vtkUnstructuredGridRelevantPointsFilter::New();
    vtkDataSet *ds = in_ds;
    int dstype = ds->GetDataObjectType();
    if (dstype == VTK_POLY_DATA)
    {
        pdrp->SetInput((vtkPolyData *) ds);
        ds = pdrp->GetOutput();
    }
    else if (dstype == VTK_UNSTRUCTURED_GRID)
    {
        ugrp->SetInput((vtkUnstructuredGrid *) ds);
        ds = ugrp->GetOutput();
    }

    //
    // Now construct the glypher.
    //
    vtkGlyph3D *glyphFilter = vtkGlyph3D::New();
    if (!scaleByVarEnabled)
    {
        glyphFilter->SetScaleModeToDataScalingOff();
    }
    else
    {
        glyphFilter->SetScaleModeToScaleByScalar();
    }
    glyphFilter->SetScaleFactor(pointSize);
    glyphFilter->SetVectorModeToVectorRotationOff();
    glyphFilter->SetIndexModeToOff();

    glyphFilter->SetSource(glyph);
    glyphFilter->SetInput(ds);
    glyphFilter->SetGeneratePointIds(1);
    glyphFilter->Update();

    vtkDataSet *output = glyphFilter->GetOutput();

    //
    // The VTK glyphing routine does not copy point data.  Copy that over
    // manually.
    //
    vtkIdTypeArray *ids = (vtkIdTypeArray *)
                         output->GetPointData()->GetArray("InputPointIds");
    ids->Register(NULL);
    vtkPointData *out_pd = vtkPointData::New();
    vtkPointData *in_pd  = ds->GetPointData();
    int npts = output->GetNumberOfPoints();
    out_pd->CopyAllocate(in_pd, npts);
    for (int i = 0 ; i < npts ; i++)
    {
        out_pd->CopyData(in_pd, ids->GetValue(i), i);
    }
    output->GetPointData()->ShallowCopy(out_pd);
    out_pd->Delete();
    ids->Delete();

    ManageMemory(output);
    glyphFilter->Delete();
    pdrp->Delete();
    ugrp->Delete();
    return output;
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Tell our output that it now has a topological dimension of 2.
//
//  Programmer: Hank Childs
//  Creation:   June 23, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Mar 17 13:01:47 PST 2003 
//    Do not take normals downstream, since our glyphs are cubes.
//
// ****************************************************************************

void
avtPointToGlyphFilter::RefashionDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(2);
        GetOutput()->GetInfo().GetValidity().
                                    SetWireframeRenderingIsInappropriate(true);
        GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true); 
    }
}


