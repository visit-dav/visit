// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtFeatureEdgesFilter.C                          //
// ************************************************************************* //

#include <avtFeatureEdgesFilter.h>

#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkVisItFeatureEdges.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

#include <avtDataset.h>

#include <DebugStream.h>
#include <VisItException.h>
#include <vtkVisItUtility.h>


using     std::string;


// ****************************************************************************
//  Method: avtFeatureEdgesFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFeatureEdgesFilter::avtFeatureEdgesFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtFeatureEdgesFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtFeatureEdgesFilter::~avtFeatureEdgesFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtFeatureEdgesFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the FeatureEdges filter.
//
//  Arguments:
//      inDR       The input data representation.
//
//  Returns:       The output data representation.
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
//    Hank Childs, Tue Aug 31 08:40:44 PDT 2004
//    The VTK feature edges filter removes edges that have "vtkGhostLevels".
//    But we now use "avtGhostZones" to indicate this.  So we need to remove
//    the ghost zones ourselves to do what used to be done from within the
//    feature edges filter.
//
//    Hank Childs, Thu Mar 31 11:16:23 PST 2005
//    Fix problem with single cell case ['6105].
//
//    Kathleen Biagas, Tue Aug 21 16:50:07 MST 2012
//    Preserve coordinate type.
//
//    Kathleen Biagas, Mon Jan 28 10:52:35 PST 2013
//    Call Update on filter not data object.
//
//    Eric Brugger, Mon Jul 21 11:41:48 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
//    Eric Brugger, Fri Sep 26 08:35:00 PDT 2014
//    I modified the routine to return a NULL in the case where it previously
//    returned an avtDataRepresentation with a NULL vtkDataSet.
//
// ****************************************************************************

avtDataRepresentation *
avtFeatureEdgesFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
    {
        // We only work on surface data
        EXCEPTION1(VisItException, "avtFeatureEdgesFilter::ExecuteDataTree "
                                   "-- Did not get polydata");
    }

    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 1)
    {
        return inDR;
    }

    vtkDataSet *outDS = NULL;

    if (inDS->GetNumberOfCells() == 1)
    {
        //
        // VTK doesn't do a good job with 1-cell feature edges, so just do it
        // ourselves.
        //

        vtkCell *cell = inDS->GetCell(0);
        vtkPolyData  *output = vtkPolyData::New();
        vtkCellData  *inCD   = inDS->GetCellData();
        vtkPointData *inPD   = inDS->GetPointData();
        vtkCellData  *outCD  = output->GetCellData();
        vtkPointData *outPD  = output->GetPointData();
        
        vtkIdList *ids = cell->GetPointIds();
        vtkIdType npts = ids->GetNumberOfIds();
        vtkPoints *pts = vtkVisItUtility::NewPoints(inDS);
        pts->SetNumberOfPoints(npts);
        outPD->CopyAllocate(inPD, npts);
        std::vector<int> origId(npts, -1);
        for (vtkIdType i = 0 ; i < npts ; i++)
        {
             origId[i] = ids->GetId(i);
             outPD->CopyData(inPD, ids->GetId(i), i);
             double pt[3];
             inDS->GetPoint(ids->GetId(i), pt);
             pts->SetPoint(i, pt);
        }
        vtkIdType ncells = cell->GetNumberOfEdges();
        outCD->CopyAllocate(inCD, ncells);
        vtkCellArray *lines = vtkCellArray::New();
        lines->Allocate(ncells*(2+1));
        for (vtkIdType i = 0 ; i < ncells ; i++)
        {
             outCD->CopyData(inCD, vtkIdType(0), i);
             vtkCell *edge = cell->GetEdge(i);
             vtkIdList *edge_ids = edge->GetPointIds();
             vtkIdType line[2];
             int origId0 = edge_ids->GetId(0);
             int origId1 = edge_ids->GetId(1);
             int newId0 = 0, newId1 = 0;
             for (vtkIdType j = 0 ; j < npts ; j++)
             {
                 if (origId0 == origId[j])
                     newId0 = j;
                 if (origId1 == origId[j])
                     newId1 = j;
             }
             line[0] = newId0;
             line[1] = newId1;
             lines->InsertNextCell(2, line);
        }

        output->SetPoints(pts);
        pts->Delete();
        output->SetLines(lines);
        lines->Delete();

        outDS = output;
    }
    else
    {
        //
        // Set up and apply the filter
        //
        vtkVisItFeatureEdges *featureEdgesFilter = vtkVisItFeatureEdges::New();
        featureEdgesFilter->SetInputData((vtkPolyData*)inDS);
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
        featureEdgesFilter->Update();

        vtkDataSet *output = featureEdgesFilter->GetOutput();

        if (output->GetNumberOfCells() > 0)
        {
            outDS = output;
            outDS->Register(NULL);
        }
        featureEdgesFilter->Delete();
    }

    if (outDS == NULL)
        return NULL;

    avtDataRepresentation *outDR = new avtDataRepresentation(outDS,
        inDR->GetDomain(), inDR->GetLabel());

    outDS->Delete();

    return outDR;
}


// ****************************************************************************
//  Method: avtFeatureEdgesFilter::UpdateDataObjectInfo
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
avtFeatureEdgesFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


