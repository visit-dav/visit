/******************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
//                             avtWarpFilter.C                               //
// ************************************************************************* //

#include <avtWarpFilter.h>

#include <vtkCellArray.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>

#include <DebugStream.h>
#include <ImproperUseException.h>

#include <avtDatasetExaminer.h>
#include <avtExtents.h>

#include <string.h>
using std::string;


// ****************************************************************************
//  Method: avtWarpFilter constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2006
//
//  Modifications:
//
// ****************************************************************************

avtWarpFilter::avtWarpFilter()
{
    //OverrideTrueSpatialExtents();
}


// ****************************************************************************
//  Method: avtWarpFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2006
//
//  Modifications:
//
// ****************************************************************************

avtWarpFilter::~avtWarpFilter()
{
}


// ****************************************************************************
//  Method: avtWarpFilter::ExecuteData
//
//  Purpose:
//    Takes in an input dataset and warps it by the point data scalars.
//
//  Arguments:
//      inDS      The input dataset.
//      <unused>  The domain number.
//      <unused>  The label.
//
//  Returns:      The output dataset.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     July 12, 2006
//
//  Modifications:
//    Brad Whitlock, Tue Mar 30 15:33:46 PDT 2010
//    Use avtCurveTransform, if it exists, to transform the curve points.
//
// ****************************************************************************

#include <vtkVisItUtility.h>

vtkDataSet *
avtWarpFilter::ExecuteData(vtkDataSet *inDS, int, string)
{
    if (inDS->GetDataObjectType() != VTK_RECTILINEAR_GRID)
    {
        EXCEPTION1(ImproperUseException, "Expecting RectlinearGrid");
    }
    if (inDS->GetPointData()->GetScalars() == NULL)
    {
        return NULL;
    }

    // Try and reconstitute a curve transform from field data
    vtkDataArray *ct = inDS->GetFieldData()->GetArray("avtCurveTransform");
    vtkMatrix4x4 *t = 0;
    if(ct != 0 && ct->GetNumberOfTuples() == 16)
    {
        t = vtkMatrix4x4::New();
        for(int i = 0; i < 16; ++i)
            t->SetElement(i / 4, i % 4, ct->GetTuple1(i));
        debug5 << "Creating transform from avtCurveTransform" << endl;
    }

    vtkDataArray *xc = vtkRectilinearGrid::SafeDownCast(inDS)->GetXCoordinates();
    vtkDataArray *sc = inDS->GetPointData()->GetScalars();

    int nPts = xc->GetNumberOfTuples();

    vtkPoints *pts = vtkPoints::New();
    pts->SetDataType(xc->GetDataType());
    pts->SetNumberOfPoints(nPts);

    vtkCellArray *verts = vtkCellArray::New();
    vtkCellArray *lines = vtkCellArray::New();
    verts->Allocate(nPts);
    lines->Allocate(nPts-1);
    vtkIdType ptIds[2];
    for (int i = 0; i < nPts; i++)
    {
        if(t == 0)
            pts->SetPoint(i, xc->GetTuple1(i), sc->GetTuple1(i), 0.); 
        else
        {
            float inpoint[4];
            inpoint[0] = xc->GetTuple1(i);
            inpoint[1] = sc->GetTuple1(i);
            inpoint[2] = 0.;
            inpoint[3] = 1.;

            float outpoint[4];
            t->MultiplyPoint(inpoint, outpoint);

            outpoint[0] /= outpoint[3];
            outpoint[1] /= outpoint[3];
            outpoint[2] = 0.; // Force z==0
            pts->SetPoint(i, outpoint);
        }

        ptIds[0] = i; 
        verts->InsertNextCell(1, ptIds);
        if (i < nPts-1)
        {
            ptIds[1] = i+1; 
            lines->InsertNextCell(2, ptIds);
        }
    }

    if(t != 0)
        t->Delete();

    vtkPolyData *polys = vtkPolyData::New();
    polys->SetPoints(pts);
    polys->SetVerts(verts);
    polys->SetLines(lines);

    pts->Delete();
    verts->Delete();
    lines->Delete();

    ManageMemory(polys);
    polys->Delete();
    return polys;
}


// ****************************************************************************
//  Method: avtWarpFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Indicate that the vector are of dimension 0.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 12, 2006
//
//  Modifications:
//    Kathleen Bonnell, Tue Jul  8 13:54:38 PDT 2008
//    Changed topological dimension to 1.
// ****************************************************************************

void
avtWarpFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(1);
    GetOutput()->GetInfo().GetAttributes().SetSpatialDimension(2);
    GetOutput()->GetInfo().GetValidity().SetNormalsAreInappropriate(true);
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
}


// ****************************************************************************
//  Method: avtWarpFilter::ModifyContract
//
//  Purpose:  Create an expression for the magnitude of the requested
//            vector variable, so that the vectors are colored correctly.
// 
//  Programmer: Kathleen Bonnell 
//  Creation:   July 12, 2006 
//
//  Modifications:
// 
// ****************************************************************************

avtContract_p
avtWarpFilter::ModifyContract(avtContract_p contract)
{
    return contract;
}


// ****************************************************************************
//  Method: avtWarpFilter::PostExecute
//
//  Purpose:
//      Cleans up after the execution.  This manages extents.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
// ****************************************************************************

void
avtWarpFilter::PostExecute(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetDesiredSpatialExtents()->Clear();

    double bounds[6];
    avtDataset_p ds = GetTypedOutput();
    avtDatasetExaminer::GetSpatialExtents(ds, bounds);
    outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);
}

