// ************************************************************************* //
//                             avtVectorGlyphMapper.C                        //
// ************************************************************************* //

#include <avtVectorGlyphMapper.h>

#include <vtkActor.h>
#include <vtkGlyph3D.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <BadIndexException.h>



// ****************************************************************************
//  Method: avtVectorGlyphMapper constructor
//
//  Arguments:
//      g       The glyph this mapper should use.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Jun 25 12:45:06 PDT 2001
//    Intialize stipplePattern.
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001 
//    Use enum types from LineAtttributes to ensure proper pattern
//    is sent down to vtk. 
//
//    Kathleen Bonnell, Mon Aug 20 18:19:25 PDT 2001
//    Removed member glyphFilterStride, it is no longer needed.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Initialize lut. 
//
//    Kathleen Bonnell, Tue Dec  3 16:10:40 PST 2002  
//    Re-initialize lineWidth, based on new LineAttributes. 
//
// ****************************************************************************

avtVectorGlyphMapper::avtVectorGlyphMapper(vtkPolyData *g)
{
    glyph = g;
    glyph->Register(NULL);

    lineWidth         = LW_0;
    lineStyle         = SOLID; 
    colorByMag        = true;
    scale             = 0.2;
    glyphFilter       = 0;
    nGlyphFilters     = 0;
    lut = NULL;
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

avtVectorGlyphMapper::~avtVectorGlyphMapper()
{
    if (glyph != NULL)
    {
        glyph->Delete();
        glyph = NULL;
    }

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
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the variable mapper to force
//      the vtk mappers to be the same as its state.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Jun 25 12:45:06 PDT 2001
//    Set actors' line stipple pattern.
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
//    Kathleen Bonnell, Wed Aug 29 16:44:31 PDT 2001 
//    Set mappers lookup table with member lut. 
//    
// ****************************************************************************

void
avtVectorGlyphMapper::CustomizeMappers(void)
{
    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetSource(glyph);
                glyphFilter[i]->SetScaleModeToScaleByVector();
            }
        }
    }
         
    if (colorByMag)
    {
        ColorByMagOn();
    }
    else
    {
        ColorByMagOff(glyphColor);
    }

    for (int i = 0; i < nMappers; i++)
    {
        if (actors[i] != NULL) 
        {
            mappers[i]->SetLookupTable(lut);
            vtkProperty *prop = actors[i]->GetProperty();
            prop->SetLineStipplePattern(LineStyle2StipplePattern(lineStyle));
            prop->SetLineWidth(LineWidth2Int(lineWidth));
        }
    }

    SetScale(scale);
    SetDefaultRange();
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetUpFilters
//
//  Purpose:
//      The glyph mapper inserts filters into the VTK pipeline, but can
//      only do so inside another routines (avtMapper::SetUpMappers) loop.
//      This is called before InsertFilters to allow for initialization work.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Aug 20 18:19:25 PDT 2001
//    Removed unsupported parameter nRenModes, and logic associated with
//    it, including glyphFilterStride..
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetUpFilters(int nDoms)
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

    nGlyphFilters     = nDoms;
    glyphFilter       = new vtkGlyph3D*[nGlyphFilters];
    for (int i = 0 ; i < nGlyphFilters ; i++)
    {
        glyphFilter[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::InsertFilters
//
//  Purpose:
//      Inserts a glyph filter into the vtk Pipeline.
//
//  Arguments:
//      ds        The upstream dataset.
//      dom       The domain number.
//      mode      The rendering mode.
//
//  Returns:      The dataset to be sent downstream.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//  
//    Kathleen Bonnell, Mon Aug 20 18:19:25 PDT 2001
//    Removed unsupported paramter 'mode' and logic associated with it.
//
// ****************************************************************************

vtkDataSet *
avtVectorGlyphMapper::InsertFilters(vtkDataSet *ds, int dom)
{
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
        glyphFilter[dom] = vtkGlyph3D::New();
    }

    glyphFilter[dom]->SetInput(ds);

    return glyphFilter[dom]->GetOutput();
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetLineWidth
//
//  Purpose:
//      Sets the line width of for all the actors of plot.
//
//  Arguments:
//      w        The new line width
//
//  Programmer:  Hank Childs 
//  Creation:    March 23, 2001 
//
//  Modifications:
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use enum types from LineAttributes.h to ensure proper
//    line width and style are sent down to vtk.
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetLineWidth(_LineWidth lw)
{
    lineWidth = lw; 
    if ( actors == NULL )
    {
        // this occurs when this method called before input is set.
        return;
    }

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetLineWidth(LineWidth2Int(lineWidth));
        }
    }
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetLineStyle
//
//  Purpose:
//      Sets the line style of for all the actors of plot.
//
//  Arguments:
//      ls       The new line style.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 25, 2001 
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetLineStyle(_LineStyle ls)
{
    lineStyle = ls;
    if ( actors == NULL )
    {
        // this occurs when this method called before input is set.
        return;
    }

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            actors[i]->GetProperty()->SetLineStipplePattern(LineStyle2StipplePattern(lineStyle));
        }
    }
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetScale
//
//  Purpose:
//      Sets the scale of each glyph.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Hank Childs 
//  Creation:    March 23, 2001 
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetScale(float s)
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
//  Method: avtVectorGlyphMapper::ColorByMagOn
//
//  Purpose:
//      Tells the glyph mapper to color by the magnitude.
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
// ****************************************************************************

void
avtVectorGlyphMapper::ColorByMagOn(void)
{
    colorByMag = true;

    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetColorModeToColorByVector();
            }
        }
    }
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::ColorByMagOff
//
//  Purpose:
//      Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//      col       The new color.
//
//  Programmer:   Hank Childs
//  Creation:     March 23, 2001
//
// ****************************************************************************

void
avtVectorGlyphMapper::ColorByMagOff(const unsigned char col[3])
{
    glyphColor[0] = col[0];
    glyphColor[1] = col[1];
    glyphColor[2] = col[2];
    colorByMag = false;
  
    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                vtkProperty *prop = actors[i]->GetProperty();
                float r = ((float) glyphColor[0]) / 255.;
                float g = ((float) glyphColor[1]) / 255.;
                float b = ((float) glyphColor[2]) / 255.;
                prop->SetColor(r, g, b);
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
// Method: avtVectorGlyphMapper::SetLookupTable
//
// Purpose: 
//   Replaces the current lookup table. 
//
// Arguments:
//   LUT       The new lookup table to be used. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 29, 2001 
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetLookupTable(vtkLookupTable *LUT)
{
    if (lut == LUT)
    {
        // no need to change anything
        return;
    }
    lut = LUT;

    // Replace each mapper's lut.
    for(int i = 0; i < nMappers; ++i)
    {
        if(actors[i] != NULL)
        {
            mappers[i]->SetLookupTable(lut);
        }
    } 
}


