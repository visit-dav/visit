// ************************************************************************* //
//                             avtPointGlyphMapper.C                        //
// ************************************************************************* //

#include <avtPointGlyphMapper.h>

#include <vtkActor.h>
#include <vtkVisItGlyph3D.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkVisItPolyDataNormals.h>
#include <BadIndexException.h>
#include <string>
using std::string;


// ****************************************************************************
//  Method: avtPointGlyphMapper constructor
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtPointGlyphMapper::avtPointGlyphMapper()
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
    coloringVarName = "";
}


// ****************************************************************************
//  Method: avtPointGlyphMapper destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004
//
//  Modifications:
//
// ****************************************************************************

avtPointGlyphMapper::~avtPointGlyphMapper()
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
//  Method: avtPointGlyphMapper::CustomizeMappers
//
//  Purpose:
//    A hook from the base class that allows the variable mapper to force
//    the vtk mappers to be the same as its state.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  2 10:18:16 PST 2004
//    avtVariableMapper::CustomizeMappers assumes a valid scalar variable
//    for determining data extents, so don't call it when we aren't coloring
//    by a scalar.  Copied non-data-extents related code from parent class
//    to here.
//
// ****************************************************************************

void
avtPointGlyphMapper::CustomizeMappers(void)
{
    if (colorByScalar)
    {
        avtVariableMapper::CustomizeMappers();
    }
    else     
    {
        if (lighting)
        {
            TurnLightingOn();
        }
        else
        {
            TurnLightingOff();
        }

        SetOpacity(opacity);

        for (int i = 0; i < nMappers; i++)
        {
            if (mappers[i] != NULL)
            {
                mappers[i]->SetLookupTable(lut);
            }
            if (actors[i] != NULL)
            {
                vtkProperty *prop = actors[i]->GetProperty();
                prop->SetLineStipplePattern(LineStyle2StipplePattern(lineStyle));
                prop->SetLineWidth(LineWidth2Int(lineWidth));
            }
        }
    }

    if (glyphFilter != NULL)
    {
        vtkPolyData *glyph = GetGlyphSource();
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetSource(glyph);
                if (dataScaling)
                {
                    glyphFilter[i]->SetScaleModeToScaleByScalar();
                    if (scalingVarName != "")
                        glyphFilter[i]->SelectScalarsForScaling(scalingVarName.c_str());
                }
                else 
                {
                    glyphFilter[i]->SetScaleModeToDataScalingOff();
                }
            }
            if (normalsFilter[i] != NULL)
            {
                normalsFilter[i]->SetNormalTypeToCell();
            }
        }
    }

    if (colorByScalar)
    {
        ColorByScalarOn(coloringVarName);
    }
    else 
    {
        ColorByScalarOff(glyphColor);
    }
         
    SetScale(scale);
}


// ****************************************************************************
//  Method: avtPointGlyphMapper::SetUpFilters
//
//  Purpose:
//    The glyph mapper inserts filters into the VTK pipeline, but can
//    only do so inside another routines (avtMapper::SetUpMappers) loop.
//    This is called before InsertFilters to allow for initialization work.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtPointGlyphMapper::SetUpFilters(int nDoms)
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
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() == 0)
    {
        nGlyphFilters     = nDoms;
        glyphFilter       = new vtkVisItGlyph3D*[nGlyphFilters];
        normalsFilter     = new vtkVisItPolyDataNormals*[nGlyphFilters];
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            glyphFilter[i] = NULL;
            normalsFilter[i] = NULL;
        }
    }
}


// ****************************************************************************
//  Method: avtPointGlyphMapper::InsertFilters
//
//  Purpose:
//    Inserts a glyph filter into the vtk Pipeline.
//
//  Arguments:
//    ds        The upstream dataset.
//    dom       The domain number.
//
//  Returns:      The dataset to be sent downstream.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
avtPointGlyphMapper::InsertFilters(vtkDataSet *ds, int dom)
{
    if (GetInput()->GetInfo().GetAttributes().GetTopologicalDimension() != 0)
        return ds;

    if (dom < 0 || dom >= nGlyphFilters)
    {
        EXCEPTION2(BadIndexException, dom, nGlyphFilters);
    }

    if (glyphFilter[dom] == NULL)
    {
        //
        // We don't have to initialize the filter now, since it will be done
        // in customize mappers later.
        //
        glyphFilter[dom] = vtkVisItGlyph3D::New();
    }
    if (normalsFilter[dom] == NULL)
    {
        normalsFilter[dom] = vtkVisItPolyDataNormals::New();
    }

    glyphFilter[dom]->SetInput(ds);
    glyphFilter[dom]->SetVectorModeToVectorRotationOff();

    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 3)
    {
        normalsFilter[dom]->SetInput(glyphFilter[dom]->GetOutput());
        return normalsFilter[dom]->GetOutput();
    }
    else
    {
        return glyphFilter[dom]->GetOutput();
    }
}


// ****************************************************************************
//  Method: avtPointGlyphMapper::SetScale
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
avtPointGlyphMapper::SetScale(float s)
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
//    type    The new type. (valid values 0 .. 3 )
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 19, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtPointGlyphMapper::SetGlyphType(const int type)
{
    if (type < 0 || type > 3) 
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
//  Method: avtPointGlyphMapper::SetUpGlyph
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
//
// ****************************************************************************

void
avtPointGlyphMapper::SetUpGlyph(void)
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
    else if (glyphType == 3)  // POINT
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
//  Method: avtPointGlyphMapper::ClearGlyphs 
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
avtPointGlyphMapper::ClearGlyphs()
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
//  Method: avtPointGlyphMapper::GetGlyphSource 
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
avtPointGlyphMapper::DataScalingOff(void)
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
//  Method: avtPointGlyphMapper::DataScalingOn 
//
//  Purpose:
//    Retrieves the correct glyph source for the input's spatial dimension.
//
//  Arguments:
//    sname     The name of the scalars to be used for scaling the glyphs.
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 19, 2004 
//
//  Modifications:
//
// ****************************************************************************


void
avtPointGlyphMapper::DataScalingOn(const string &sname)
{
    dataScaling = true;
    scalingVarName = sname;

    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetScaleModeToScaleByScalar();
                if (scalingVarName != "")
                    glyphFilter[i]->SelectScalarsForScaling(scalingVarName.c_str());
            }
        }
    }
}


// ****************************************************************************
//  Method: avtPointGlyphMapper::GetGlyphSource 
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
//
// ****************************************************************************

vtkPolyData *
avtPointGlyphMapper::GetGlyphSource(void)
{
    if (*(GetInput()) != NULL)
    {
        if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
            return glyph2D;
        else 
            return glyph3D;
    }
    return glyph3D;

}


// ****************************************************************************
//  Method: avtPointGlyphMapper::ColorByScalarOn
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
avtPointGlyphMapper::ColorByScalarOn(const string &sn)
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
//  Method: avtPointGlyphMapper::ColorByScalarOff
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
//    Kathleen Bonnell, Tue Nov  2 10:18:16 PST 2004
//    Change argument type from unsigned char to float.
//
// ****************************************************************************

void
avtPointGlyphMapper::ColorByScalarOff(const float col[3])
{
    glyphColor[0] = col[0];
    glyphColor[1] = col[1];
    glyphColor[2] = col[2];
    colorByScalar = false;
  
    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                vtkProperty *prop = actors[i]->GetProperty();
                prop->SetColor(glyphColor);
            }
        }
    }

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
//  Method: avtPointGlyphMapper::ColorByScalarOff
//
//  Purpose:
//    Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     November 2, 2004 
//
// ****************************************************************************

void
avtPointGlyphMapper::ColorByScalarOff(const unsigned char col[3])
{
    float fc[3];
    fc[0] = (float)col[0] / 255.;
    fc[1] = (float)col[1] / 255.;
    fc[2] = (float)col[2] / 255.;
    ColorByScalarOff(fc);
}


// ****************************************************************************
//  Method: avtPointGlyphMapper::ColorByScalarOff
//
//  Purpose:
//    Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Bonnell
//  Creation:     November 2, 2004 
//
// ****************************************************************************

void
avtPointGlyphMapper::ColorByScalarOff(const double col[3])
{
    float fc[3];
    fc[0] = (float)col[0];
    fc[1] = (float)col[1];
    fc[2] = (float)col[2];
    ColorByScalarOff(fc);
}

