// ************************************************************************* //
//                          avtFeatureEdgesFilter.C                          //
// ************************************************************************* //

#include <avtFeatureEdgesFilter.h>

#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFeatureEdges.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>


using     std::string;


// ****************************************************************************
//  Method: avtFeatureEdgesFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the FeatureEdges filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//      <unused>   The label.
//
//  Returns:       The output unstructured grid.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 12, 2001
//
//  Modifications:
//    Jeremy Meredith, Wed Mar 13 15:18:09 PST 2002
//    Made it only use feature-angle detection on spatially 3D meshes.
//
//    Hank Childs, Tue Sep 10 15:43:53 PDT 2002
//    Renamed to ExecuteData and handled memory issues entirely from within
//    routine.
//
//    Hank Childs, Wed Oct 15 21:19:00 PDT 2003
//    Improved for the single cell case -- VTK feature edges was handling
//    poorly.
//
// ****************************************************************************

vtkDataSet *
avtFeatureEdgesFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        // We only work on surface data
        EXCEPTION1(VisItException, "avtFeatureEdgesFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
    {
        return inDS;
    }

    vtkDataSet *outDS = NULL;

    if (inDS->GetNumberOfCells() == 1)
    {
        //
        // VTK doesn't do a good job with 1-cell feature edges, so just do it
        // ourselves.
        //
        int  i;

        vtkCell *cell = inDS->GetCell(0);
        vtkPolyData  *output = vtkPolyData::New();
        vtkCellData  *inCD   = inDS->GetCellData();
        vtkPointData *inPD   = inDS->GetPointData();
        vtkCellData  *outCD  = output->GetCellData();
        vtkPointData *outPD  = output->GetPointData();
        
        vtkIdList *ids = cell->GetPointIds();
        int npts = ids->GetNumberOfIds();
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(npts);
        outPD->CopyAllocate(inPD, npts);
        for (i = 0 ; i < npts ; i++)
        {
             outPD->CopyData(inPD, ids->GetId(i), i);
             float pt[3];
             inDS->GetPoint(ids->GetId(i), pt);
             pts->SetPoint(i, pt);
        }
        int ncells = cell->GetNumberOfEdges();
        outCD->CopyAllocate(inCD, ncells);
        vtkCellArray *lines = vtkCellArray::New();
        lines->Allocate(ncells*(2+1));
        for (i = 0 ; i < ncells ; i++)
        {
             outCD->CopyData(inCD, 0, i);
             vtkCell *edge = cell->GetEdge(i);
             vtkIdList *edge_ids = edge->GetPointIds();
             int line[2];
             line[0] = edge_ids->GetId(0);
             line[1] = edge_ids->GetId(1);
             lines->InsertNextCell(2, line);
        }

        output->SetPoints(pts);
        pts->Delete();
        output->SetLines(lines);
        lines->Delete();

        ManageMemory(output);
        output->Delete();
        outDS = output;
    }
    else
    {
        //
        // Set up and apply the filter
        //
        vtkFeatureEdges *featureEdgesFilter = vtkFeatureEdges::New();
        featureEdgesFilter->SetInput((vtkPolyData*)inDS);
        vtkPolyData *newDS = vtkPolyData::New();
        featureEdgesFilter->SetOutput(newDS);

        featureEdgesFilter->BoundaryEdgesOn();
        if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 3)
        {
            featureEdgesFilter->FeatureEdgesOn();
            featureEdgesFilter->SetFeatureAngle(60.0);
        }
        else
        {
            featureEdgesFilter->FeatureEdgesOff();
        }
        featureEdgesFilter->NonManifoldEdgesOff();
        featureEdgesFilter->ManifoldEdgesOff();
        featureEdgesFilter->ColoringOff();

        newDS->Update();

        if (newDS->GetNumberOfCells() > 0)
        {
            outDS = newDS;
        }
        ManageMemory(outDS);
        newDS->Delete();
        featureEdgesFilter->Delete();
    }

    return outDS;
}


// ****************************************************************************
//  Method: avtFeatureEdgesFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicate that this invalidates the zone numberings.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 12, 2001
//
//  Modifications:
// ****************************************************************************

void
avtFeatureEdgesFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


