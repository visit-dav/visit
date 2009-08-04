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
//                             avtPointGlypher.C                             //
// ************************************************************************* //

#include <avtPointGlypher.h>

#include <vtkVisItGlyph3D.h>
#include <vtkPolyData.h>
#include <vtkVisItPolyDataNormals.h>
#include <BadIndexException.h>


using std::string;


// ****************************************************************************
//  Method: avtPointGlypher constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 19, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 09:24:15 PST 2004
//    Added scalingVarDim and spatialDim.
//
// ****************************************************************************

avtPointGlypher::avtPointGlypher()
{
    glyph2D = NULL;
    glyph3D = NULL;
    glyphType = 0;
    colorByScalar = true;
    SetUpGlyph();

    scale             = 0.2;
    glyphFilter       = 0;
    normalsFilter     = NULL;
    nGlyphFilters     = 0;
    scalingVarName = "";
    scalingVarDim = 1;
    coloringVarName = "";
    spatialDim = 3;
}


// ****************************************************************************
//  Method: avtPointGlypher destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPointGlypher::~avtPointGlypher()
{
    ClearGlyphs();
    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->Delete();
            }
        }
        delete [] glyphFilter;
    }
    if (normalsFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (normalsFilter[i] != NULL)
            {
                normalsFilter[i]->Delete();
            }
        }
        delete [] normalsFilter;
    }
}


// ****************************************************************************
//  Method: avtPointGlypher::CustomizeGlyphs
//
//  Purpose:
//    Sets the proper source for the glyphFilters, based on the spatialDim. 
//
//  Arguments:
//    spatDim   The spatial dimension of the points being glyphed.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 08:49:04 PST 2004
//    Renamed from CustomizeMappers, as this is no longer derived from 
//    avtMapper.  Added spatial dimension argument.  Removed avtMapper
//    specific code.
//
// ****************************************************************************

void
avtPointGlypher::CustomizeGlyphs(int spatDim)
{
    spatialDim = spatDim;
    if (glyphFilter != NULL)
    {
        vtkPolyData *glyph = GetGlyphSource();
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetSource(glyph);
            }
            if (normalsFilter[i] != NULL)
            {
                normalsFilter[i]->SetNormalTypeToCell();
            }
        }
    }
    if (dataScaling)
    {
        DataScalingOn(scalingVarName, scalingVarDim);
    }
    else 
    {
        DataScalingOff(); 
    }

    if (colorByScalar)
    {
        ColorByScalarOn(coloringVarName);
    }
    else 
    {
        ColorByScalarOff();
    }
         
    SetScale(scale);
}


// ****************************************************************************
//  Method: avtPointGlypher::SetUpGlyphs
//
//  Purpose:
//    Creates glyphFilters.
//
//  Arguments:
//    nGlyphs   The number of glyph filters to be created.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 08:49:04 PST 2004
//    Renamed method, so it doesn't collide with avtMapper methods. 
//
// ****************************************************************************

void
avtPointGlypher::SetUpGlyphs(int nGlyphs)
{
    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->Delete();
            }
        }
        delete [] glyphFilter;
    }
    if (normalsFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (normalsFilter[i] != NULL)
            {
                normalsFilter[i]->Delete();
            }
        }
        delete [] normalsFilter;
    }
    nGlyphFilters     = nGlyphs;
    glyphFilter       = new vtkVisItGlyph3D*[nGlyphFilters];
    normalsFilter     = new vtkVisItPolyDataNormals*[nGlyphFilters];
    for (int i = 0 ; i < nGlyphFilters ; i++)
    {
        glyphFilter[i] = NULL;
        normalsFilter[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtPointGlypher::InsertGlyphs
//
//  Purpose:
//    Inserts a glyph filter into the vtk Pipeline.
//
//  Arguments:
//    ds          The upstream dataset.
//    whichGlyph  Which glyph filter should be used. 
//    spatDim     The spatial dimension. 
//
//  Returns:      The dataset to be sent downstream.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//    Hank Childs, Wed Nov 10 11:27:23 PST 2004
//    Do not glyphs points when our glyph type is "point".
//
//    Kathleen Bonnell, Fri Nov 12 08:49:04 PST 2004 
//    Added spatDim argument, since this is no longer derived from avtMapper,
//    and so 'GetInput' no longer is available. 
//
//    Brad Whitlock, Thu Aug 25 10:24:49 PDT 2005
//    Added support for sphere points.
//
// ****************************************************************************

vtkDataSet *
avtPointGlypher::InsertGlyphs(vtkDataSet *ds, int whichGlyph, int spatDim)
{
    if (glyphType == 3 || glyphType == 4) // Point, Sphere
        return ds;

    if (whichGlyph < 0 || whichGlyph >= nGlyphFilters)
    {
        EXCEPTION2(BadIndexException, whichGlyph, nGlyphFilters);
    }

    spatialDim = spatDim;

    if (glyphFilter[whichGlyph] == NULL)
    {
        //
        // We don't have to initialize the filter now, since it will be done
        // in customize mappers later.
        //
        glyphFilter[whichGlyph] = vtkVisItGlyph3D::New();
    }
    if (normalsFilter[whichGlyph] == NULL)
    {
        normalsFilter[whichGlyph] = vtkVisItPolyDataNormals::New();
    }

    glyphFilter[whichGlyph]->SetInput(ds);
    glyphFilter[whichGlyph]->SetVectorModeToVectorRotationOff();

    if (spatialDim == 3)
    {
        normalsFilter[whichGlyph]->SetInput(glyphFilter[whichGlyph]->GetOutput());
        return normalsFilter[whichGlyph]->GetOutput();
    }
    else
    {
        return glyphFilter[whichGlyph]->GetOutput();
    }
}


// ****************************************************************************
//  Method: avtPointGlypher::SetScale
//
//  Purpose:
//    Sets the scale of each glyph.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    August 19, 2004 
//
// ****************************************************************************

void
avtPointGlypher::SetScale(double s)
{
    scale = s;
    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetScaleFactor(scale);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtPointToGlyphFilter::SetGlyphType
//
//  Purpose:
//    Sets Glyph type, makes sure the glyph filters know about the new type. 
//
//  Arguments:
//    type    The new type. (valid values 0 .. 4 )
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 19, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Aug 25 10:25:14 PDT 2005
//    Added support for sphere points.
//
// ****************************************************************************

void
avtPointGlypher::SetGlyphType(const int type)
{
    if (type < 0 || type > 4) 
        return; 

    if (glyphType != type)
    {
        glyphType = type;
        SetUpGlyph();
        if (glyphFilter != NULL)
        {
            vtkPolyData *glyph = GetGlyphSource();
            for (int i = 0 ; i < nGlyphFilters ; i++)
            {
                if (glyphFilter[i] != NULL)
                {
                    glyphFilter[i]->SetSource(glyph);
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtPointGlypher::SetUpGlyph
//
//  Purpose:
//    Sets up glyph based on glyphType.
//
//  Notes:
//    Taken practially verbatim from avtPointGlyphFilter.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Aug 25 10:23:23 PDT 2005
//    Added support for sphere glyphs.
//
// ****************************************************************************

void
avtPointGlypher::SetUpGlyph(void)
{
    //
    // Free any memory associated with the old glyphs
    //
    ClearGlyphs();

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
             double rad = ((double) i) / 12. * 2. * M_PI;
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
    else if (glyphType == 3 || // POINT
             glyphType == 4)   // SPHERE (textured onto a point)
    {
        vtkPoints *pts = vtkPoints::New();
        pts->SetNumberOfPoints(1);
        pts->SetPoint(0, 0, 0, 0);
        vtkIdType ids[1] ={0};
        glyph3D = vtkPolyData::New();
        glyph3D->SetPoints(pts);

        glyph3D->Allocate(1);
        glyph3D->InsertNextCell(VTK_VERTEX, 1, ids);

        glyph2D = vtkPolyData::New();
        glyph2D->SetPoints(pts);
        pts->Delete();

        glyph2D->Allocate(1);
        glyph2D->InsertNextCell(VTK_VERTEX, 1, ids);
    }
}

// ****************************************************************************
//  Method: avtPointGlypher::ClearGlyphs 
//
//  Purpose:
//    Deletes the polydata associated with the glyph sources. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointGlypher::ClearGlyphs()
{
    if (glyph2D != NULL)
    {
        glyph2D->Delete();
        glyph2D = NULL;
    }
    if (glyph3D != NULL)
    {
        glyph3D->Delete();
        glyph3D = NULL;
    }
}

// ****************************************************************************
//  Method: avtPointGlypher::DataScalingOff 
//
//  Purpose:
//    Retrieves the correct glyph source for the input's spatial dimension.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointGlypher::DataScalingOff(void)
{
    dataScaling = false;
    scalingVarName = "";

    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetScaleModeToDataScalingOff();
            }
        }
    }
}

// ****************************************************************************
//  Method: avtPointGlypher::DataScalingOn 
//
//  Purpose:
//    Turns on the appropriate type of data scaling based on the dimension
//    of the variable to be used in scaling. 
//
//  Arguments:
//    sname     The name of the scalars to be used for scaling the glyphs.
//    varDim    The dimension of the var to be used for scaling.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 09:24:15 PST 2004
//    Added varDim argument so that data scaling can be done by other
//    than just scalar vars.  
//
// ****************************************************************************


void
avtPointGlypher::DataScalingOn(const string &sname, int varDim)
{
    dataScaling = true;
    scalingVarName = sname;
    scalingVarDim = varDim;
    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                if (scalingVarDim  < 3)
                {
                    glyphFilter[i]->SetScaleModeToScaleByScalar();
                    glyphFilter[i]->SelectScalarsForScaling(scalingVarName.c_str());
                }
                else if (scalingVarDim == 3)
                {
                    glyphFilter[i]->SetScaleModeToScaleByVector();
                    glyphFilter[i]->SelectVectorsForScaling(scalingVarName.c_str());
                }
                else 
                { 
                    // will use the first three components to scale each 
                    // coord direction.
                    glyphFilter[i]->SetScaleModeToScaleByVectorComponents();
                    glyphFilter[i]->SelectVectorsForScaling(scalingVarName.c_str());
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtPointGlypher::GetGlyphSource 
//
//  Purpose:
//    Retrieves the correct glyph source for the input's spatial dimension.
//
//  Returns:
//    A 2d glyph source (polydata) or a 3d glyphsource.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 09:24:15 PST 2004
//    Use spatialDim data member, as this class is no longer derived from
//    an avtMapper and has no access to pipeline input data.
//
// ****************************************************************************

vtkPolyData *
avtPointGlypher::GetGlyphSource()
{
    if (spatialDim == 2)
        return glyph2D;
    else 
        return glyph3D;
}


// ****************************************************************************
//  Method: avtPointGlypher::ColorByScalarOn
//
//  Purpose:
//    Tells the glyph mapper to color by the given scalar.
//
//  Arguments:
//    sn        The name of the scalar var the glyph should use for coloring.
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointGlypher::ColorByScalarOn(const string &sn)
{
    colorByScalar = true;
    coloringVarName = sn;
    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetColorModeToColorByScalar();
                glyphFilter[i]->SelectScalarsForColoring(coloringVarName.c_str());
            }
        }
    }
}


// ****************************************************************************
//  Method: avtPointGlypher::ColorByScalarOff
//
//  Purpose:
//    Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     August 19, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Fri Nov 12 09:30:38 PST 2004
//    Removed vtkActor specific code, as this class is no longer derived from
//    avtMapper and does not have access to vtkActors.
//
// ****************************************************************************

void
avtPointGlypher::ColorByScalarOff()
{
    colorByScalar = false;
    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetColorMode(0);
            }
        }
    }
}

// ****************************************************************************
// Method: avtPointGlypher::ProtectedSetFullFrameScaling
//
// Purpose: 
//   Sets fullframe scaling in the glyphers.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 26 13:57:21 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtPointGlypher::ProtectedSetFullFrameScaling(bool useScale, const double *s)
{
    bool retval = false;

    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                if(glyphFilter[i]->SetFullFrameScaling(useScale?1:0, s))
                    retval = true;
            }
        }
    }

    return retval;
}
