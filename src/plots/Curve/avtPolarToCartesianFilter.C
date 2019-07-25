// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                  avtPolarToCartesianFilter.C                              //
// ************************************************************************* //

#include <avtPolarToCartesianFilter.h>
#include <avtOriginatingSource.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <string.h>
using std::string;


// ****************************************************************************
//  Method: avtPolarToCartesianFilter constructor
//
//  Programmer: Kathleen Biagas
//  Creation:   September 11, 2013
//
//  Modifications:
//
// ****************************************************************************

avtPolarToCartesianFilter::avtPolarToCartesianFilter()
{
    swapCoords = false;
    useDegrees = false;
}


// ****************************************************************************
//  Method: avtPolarToCartesianFilter destructor
//
//  Programmer: Kathleen Biagas
//  Creation:   September 11, 2013
//
//  Modifications:
//
// ****************************************************************************

avtPolarToCartesianFilter::~avtPolarToCartesianFilter()
{
}


// ****************************************************************************
//  Method: avtPolarToCartesianFilter::ExecuteData
//
//  Purpose:
//    Takes in an input dataset and converts the polar coordinates to 
//    cartesian.   
//
//  Arguments:
//      inDR      The input data representation.
//
//  Returns:      The output data representation.
//
//  Programmer:   Kathleen Biagas
//  Creation:     September 11, 2013
//
//  Modifications:
//    Eric Brugger, Tue Aug 19 10:03:39 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

avtDataRepresentation *
avtPolarToCartesianFilter::ExecuteData(avtDataRepresentation *inDR)
{
    //
    // Get the VTK data set.
    //
    vtkDataSet *inDS = inDR->GetDataVTK();

    if (inDS->GetDataObjectType() != VTK_POLY_DATA)
        EXCEPTION1(ImproperUseException, "Expecting PolyData");

    vtkPolyData *inPoly = vtkPolyData::SafeDownCast(inDS);
    vtkPolyData *outPoly = inPoly->NewInstance();
    outPoly->ShallowCopy(inPoly);
    vtkPoints *inPts = inPoly->GetPoints();
    vtkIdType nPts = inPts->GetNumberOfPoints();
    vtkPoints *outPts = inPts->NewInstance();
    outPts->SetNumberOfPoints(nPts);
    double newPt[3] = {0., 0., 0.};
    double toRadians = useDegrees ? M_PI/180.0 : 1.0;

    // set it up for R_Theta order, but change if necessary.
    int r_index = 0;
    int theta_index = 1;
    if (swapCoords)
    {
        r_index = 1;
        theta_index = 0;
    } 
    for (vtkIdType i = 0; i < nPts; ++i)
    {
        double *ip = inPts->GetPoint(i);
        newPt[0] = ip[r_index] * cos(ip[theta_index] *toRadians);
        newPt[1] = ip[r_index] * sin(ip[theta_index] *toRadians);
        outPts->SetPoint(i, newPt);
    }
    outPoly->SetPoints(outPts); 
    outPts->Delete();

    avtDataRepresentation *outDR = new avtDataRepresentation(outPoly,
        inDR->GetDomain(), inDR->GetLabel());

    outPoly->Delete();

    return outDR;
}


// ****************************************************************************
//  Method: avtPolarToCartesianFilter::UpdateDataObjectInfo
//
//  Purpose:
//
//
//  Programmer: Kathleen Biagas
//  Creation:   September 11, 2013
//
//  Modifications:
//
// ****************************************************************************

void
avtPolarToCartesianFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetAttributes().SetSpatialDimension(2);
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


// ****************************************************************************
//  Method: avtPolarToCartesianFilter::ModifyContract
//
//  Purpose:  
// 
//  Programmer: Kathleen Biagas 
//  Creation:   September 11, 2013
//
//  Modifications:
// 
// ****************************************************************************

avtContract_p
avtPolarToCartesianFilter::ModifyContract(avtContract_p contract)
{
    return contract;
}


// ****************************************************************************
//  Method: avtPolarToCartesianFilter::PostExecute
//
//  Purpose:
//      Cleans up after the execution.  This manages extents.
//
//  Programmer: Kathleen Biagas 
//  Creation:   September 11, 2013
//
//  Modifications:
//
// ****************************************************************************

void
avtPolarToCartesianFilter::PostExecute(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetDesiredSpatialExtents()->Clear();

    double bounds[6];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(ds, bounds);
    outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);
}

