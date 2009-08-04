/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
*****************************************************************************/

// ************************************************************************* //
//                           avtPointToGlyphFilter.C                         //
// ************************************************************************* //

#include <avtPointToGlyphFilter.h>

#include <math.h>

#include <vtkCellArray.h>
#include <vtkGeometryFilter.h>
#include <vtkGlyph3D.h>
#include <vtkIdTypeArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataRelevantPointsFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridRelevantPointsFilter.h>

#include <avtDataset.h>

#include <ImproperUseException.h>


// ****************************************************************************
//  Method: avtPointToGlyphFilter constructor
//
//  Programmer: Hank Childs 
//  Creation:   June 22, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Aug 21 22:06:27 PDT 2003
//    Added support for more glyph types.
//
// ****************************************************************************

avtPointToGlyphFilter::avtPointToGlyphFilter()
{
    pointSize         = 0.05;
    glyphType         = 0;
    scaleVar          = "default";
    scaleByVarEnabled = false;
    
    SetUpGlyph();
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
//  Method: avtPointToGlyphFilter::SetUpGlyph
//
//  Purpose:
//      Sets up glyph based on glyphType.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
//  Modifications:
//    Jeremy Meredith, Tue May  4 13:20:01 PDT 2004
//    Added support for unglyphed (i.e. GL_POINT) glyph types.
//
// ****************************************************************************

void
avtPointToGlyphFilter::SetUpGlyph(void)
{
    if (glyphType == 0)  // BOX
    {
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
    else if (glyphType == 1) // AXIS
    {
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(12);
        pts->SetPoint(0, 0., -0.5, -0.5);
        pts->SetPoint(1, 0., -0.5, +0.5);
        pts->SetPoint(2, 0., +0.5, +0.5);
        pts->SetPoint(3, 0., +0.5, -0.5);
        pts->SetPoint(4, -0.5, 0., -0.5);
        pts->SetPoint(5, -0.5, 0., +0.5);
        pts->SetPoint(6, +0.5, 0., +0.5);
        pts->SetPoint(7, +0.5, 0., -0.5);
        pts->SetPoint(8, -0.5, -0.5, 0.);
        pts->SetPoint(9, -0.5, +0.5, 0.);
        pts->SetPoint(10, +0.5, +0.5, 0.);
        pts->SetPoint(11, +0.5, -0.5, 0.);

        glyph3D = vtkPolyData::New();
        glyph3D->SetPoints(pts);
        pts->Delete();

        glyph3D->Allocate(15);
        vtkIdType ids[3][4] = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, 
                                { 8, 9, 10, 11 } };
        glyph3D->InsertNextCell(VTK_QUAD, 4, ids[0]);
        glyph3D->InsertNextCell(VTK_QUAD, 4, ids[1]);
        glyph3D->InsertNextCell(VTK_QUAD, 4, ids[2]);

        vtkPoints *pts2D = vtkPoints::New();
        pts2D->SetNumberOfPoints(4);
        pts2D->SetPoint(0, -0.5, 0., 0.);
        pts2D->SetPoint(1, +0.5, 0., 0.);
        pts2D->SetPoint(2, 0., -0.5, 0.);
        pts2D->SetPoint(3, 0., +0.5, 0.);

        glyph2D = vtkPolyData::New();
        glyph2D->SetPoints(pts2D);
        pts2D->Delete();

        glyph2D->Allocate(6);
        vtkIdType line1[2] = { 0, 1 };
        vtkIdType line2[2] = { 2, 3 };
        glyph2D->InsertNextCell(VTK_LINE, 2, line1);
        glyph2D->InsertNextCell(VTK_LINE, 2, line2);
    }
    else if (glyphType == 2) // ICOSAHEDRON
    {
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(60);
        pts->SetPoint(0, 0, 0.5, 0);
        pts->SetPoint(1, -0.43787, 0.223375, 0.091508);
        pts->SetPoint(2, -0.0483832, 0.223697, 0.444543);
        pts->SetPoint(3, 0, 0.5, 0);
        pts->SetPoint(4, -0.0483832, 0.223697, 0.444543);
        pts->SetPoint(5, 0.407933, 0.223515, 0.183389);
        pts->SetPoint(6, 0, 0.5, 0);
        pts->SetPoint(7, 0.407933, 0.223515, 0.183389);
        pts->SetPoint(8, 0.300453, 0.223706, -0.331186);
        pts->SetPoint(9, 0, 0.5, 0);
        pts->SetPoint(10, 0.300453, 0.223706, -0.331186);
        pts->SetPoint(11, -0.222361, 0.223742, -0.387937);
        pts->SetPoint(12, 0, 0.5, 0);
        pts->SetPoint(13, -0.222361, 0.223742, -0.387937);
        pts->SetPoint(14, -0.43787, 0.223375, 0.091508);
        pts->SetPoint(15, 0, -0.5, 0);
        pts->SetPoint(16, 0.222411, -0.223548, 0.38802);
        pts->SetPoint(17, -0.300216, -0.223689, 0.331411);
        pts->SetPoint(18, 0, -0.5, 0);
        pts->SetPoint(19, -0.300216, -0.223689, 0.331411);
        pts->SetPoint(20, -0.407838, -0.223735, -0.183333);
        pts->SetPoint(21, 0, -0.5, 0);
        pts->SetPoint(22, -0.407838, -0.223735, -0.183333);
        pts->SetPoint(23, 0.0482061, -0.223357, -0.444733);
        pts->SetPoint(24, 0, -0.5, 0);
        pts->SetPoint(25, 0.0482061, -0.223357, -0.444733);
        pts->SetPoint(26, 0.437665, -0.223704, -0.091682);
        pts->SetPoint(27, 0, -0.5, 0);
        pts->SetPoint(28, 0.437665, -0.223704, -0.091682);
        pts->SetPoint(29, 0.222411, -0.223548, 0.38802);
        pts->SetPoint(30, -0.300216, -0.223689, 0.331411);
        pts->SetPoint(31, 0.222411, -0.223548, 0.38802);
        pts->SetPoint(32, -0.0483832, 0.223697, 0.444543);
        pts->SetPoint(33, -0.407838, -0.223735, -0.183333);
        pts->SetPoint(34, -0.300216, -0.223689, 0.331411);
        pts->SetPoint(35, -0.43787, 0.223375, 0.091508);
        pts->SetPoint(36, 0.0482061, -0.223357, -0.444733);
        pts->SetPoint(37, -0.407838, -0.223735, -0.183333);
        pts->SetPoint(38, -0.222361, 0.223742, -0.387937);
        pts->SetPoint(39, 0.437665, -0.223704, -0.091682);
        pts->SetPoint(40, 0.0482061, -0.223357, -0.444733);
        pts->SetPoint(41, 0.300453, 0.223706, -0.331186);
        pts->SetPoint(42, 0.222411, -0.223548, 0.38802);
        pts->SetPoint(43, 0.437665, -0.223704, -0.091682);
        pts->SetPoint(44, 0.407933, 0.223515, 0.183389);
        pts->SetPoint(45, -0.0483832, 0.223697, 0.444543);
        pts->SetPoint(46, -0.43787, 0.223375, 0.091508);
        pts->SetPoint(47, -0.300216, -0.223689, 0.331411);
        pts->SetPoint(48, -0.43787, 0.223375, 0.091508);
        pts->SetPoint(49, -0.222361, 0.223742, -0.387937);
        pts->SetPoint(50, -0.407838, -0.223735, -0.183333);
        pts->SetPoint(51, -0.222361, 0.223742, -0.387937);
        pts->SetPoint(52, 0.300453, 0.223706, -0.331186);
        pts->SetPoint(53, 0.0482061, -0.223357, -0.444733);
        pts->SetPoint(54, 0.300453, 0.223706, -0.331186);
        pts->SetPoint(55, 0.407933, 0.223515, 0.183389);
        pts->SetPoint(56, 0.437665, -0.223704, -0.091682);
        pts->SetPoint(57, 0.407933, 0.223515, 0.183389);
        pts->SetPoint(58, -0.0483832, 0.223697, 0.444543);
        pts->SetPoint(59, 0.222411, -0.223548, 0.38802);

        glyph3D = vtkPolyData::New();
        glyph3D->SetPoints(pts);
        pts->Delete();

        glyph3D->Allocate(80);
        int i;
        for (i = 0 ; i < 20 ; i++)
        {
            vtkIdType ids[3] = { 3*i, 3*i+1, 3*i+2 };
            glyph3D->InsertNextCell(VTK_TRIANGLE, 3, ids);
        }

        vtkPoints *pts2D = vtkPoints::New();
        pts2D->SetNumberOfPoints(13);
        pts2D->SetPoint(0, 0., 0., 0.);
        for (i = 0 ; i < 12 ; i++)
        {
#if defined(_WIN32) && !defined(M_PI)
#define M_PI 3.14159
#endif
             float rad = ((float) i) / 12. * 2. * M_PI;
             pts2D->SetPoint(i+1, cos(rad)/2., sin(rad)/2., 0.);
        }

        glyph2D = vtkPolyData::New();
        glyph2D->SetPoints(pts2D);
        pts2D->Delete();

        glyph2D->Allocate(48);
        for (i = 0 ; i < 12 ; i++)
        {
            int pt2 = (i+2 >= 13 ? 1 : i+2);
            vtkIdType tri[3] = { 0, i+1, pt2 };
            glyph2D->InsertNextCell(VTK_TRIANGLE, 3, tri);
        }
    }
    else if (glyphType == 3) // POINT
    {
        // Do nothing; we're not going to glyph them!
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::SetGlyphType
//
//  Purpose:
//      Sets the glyph type.
//
//  Programmer: Hank Childs
//  Creation:   August 21, 2003
//
// ****************************************************************************

void
avtPointToGlyphFilter::SetGlyphType(int gt)
{
    glyphType = gt;
    SetUpGlyph();
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
//    Hank Childs, Sat Dec 13 21:34:05 PST 2003
//    Do a better job of handling nodal and zonal point data.
//
//    Jeremy Meredith, Tue May  4 12:25:07 PDT 2004
//    Added support for un-glyphed point meshes (glyph type 3).
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
    // If there is no glypher, then just copy the points into vtkPolyData
    //
    if (glyphType == 3)
    {
        // vtkGeometryFilter should work just fine for this.
        vtkGeometryFilter *geom = vtkGeometryFilter::New();
        geom->SetInput(ds);
        vtkPolyData *output = geom->GetOutput();
        geom->Update();
        ManageMemory(output);
        pdrp->Delete();
        ugrp->Delete();
        geom->Delete();
        return output;
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
    int i;
    for (i = 0 ; i < npts ; i++)
    {
        out_pd->CopyData(in_pd, ids->GetValue(i), i);
    }
    for (i = 0 ; i < out_pd->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = out_pd->GetArray(i);
        output->GetPointData()->AddArray(arr);
    }
    output->GetPointData()->RemoveArray("InputPointIds");
    out_pd->Delete();
    ids->Delete();

    ManageMemory(output);
    glyphFilter->Delete();
    pdrp->Delete();
    ugrp->Delete();
    return output;
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::UpdateDataObjectInfo
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
//    Hank Childs, Thu Aug 21 22:53:30 PDT 2003
//    Allow for icosahedrons to get shaded.
//
//    Jeremy Meredith, Tue May  4 12:31:47 PDT 2004
//    Don't change topological dimension from zero if we are not glyphing.
//
//    Hank Childs, Wed May  5 16:05:41 PDT 2004
//    Allow for all glyphs to get shaded.
//
// ****************************************************************************

void
avtPointToGlyphFilter::UpdateDataObjectInfo(void)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        if (glyphType != 3)
            GetOutput()->GetInfo().GetAttributes().SetTopologicalDimension(2);
        GetOutput()->GetInfo().GetValidity().
                                    SetWireframeRenderingIsInappropriate(true);
    }
}


