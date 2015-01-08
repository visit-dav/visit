/******************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
******************************************************************************/

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

