// ************************************************************************* //
//                             avtTensorGlyphMapper.C                        //
// ************************************************************************* //

#include <avtTensorGlyphMapper.h>

#include <vtkActor.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkTensorGlyph.h>

#include <BadIndexException.h>



// ****************************************************************************
//  Method: avtTensorGlyphMapper constructor
//
//  Arguments:
//      g       The glyph this mapper should use.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

avtTensorGlyphMapper::avtTensorGlyphMapper(vtkPolyData *g)
{
    glyph = g;
    glyph->Register(NULL);

    colorByMag        = true;
    scale             = 0.2;
    tensorFilter      = NULL;
    nTensorFilters    = 0;
    lut = NULL;
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

avtTensorGlyphMapper::~avtTensorGlyphMapper()
{
    if (glyph != NULL)
    {
        glyph->Delete();
        glyph = NULL;
    }

    if (tensorFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (tensorFilter[i] != NULL)
            {
                tensorFilter[i]->Delete();
            }
        }
        delete [] tensorFilter;
    }
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the variable mapper to force
//      the vtk mappers to be the same as its state.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

void
avtTensorGlyphMapper::CustomizeMappers(void)
{
    if (tensorFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (tensorFilter[i] != NULL)
            {
                tensorFilter[i]->SetSource(glyph);
                tensorFilter[i]->SetScaling(1);
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
        }
    }

    SetScale(scale);
    SetDefaultRange();
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetUpFilters
//
//  Purpose:
//      The glyph mapper inserts filters into the VTK pipeline, but can
//      only do so inside another routines (avtMapper::SetUpMappers) loop.
//      This is called before InsertFilters to allow for initialization work.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetUpFilters(int nDoms)
{
    if (tensorFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (tensorFilter[i] != NULL)
            {
                tensorFilter[i]->Delete();
            }
        }
        delete [] tensorFilter;
    }

    nTensorFilters     = nDoms;
    tensorFilter       = new vtkTensorGlyph*[nTensorFilters];
    for (int i = 0 ; i < nTensorFilters ; i++)
    {
        tensorFilter[i] = NULL;
    }
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::InsertFilters
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
//  Creation:   September 23, 2003
//
// ****************************************************************************

vtkDataSet *
avtTensorGlyphMapper::InsertFilters(vtkDataSet *ds, int dom)
{
    if (dom < 0 || dom >= nTensorFilters)
    {
        EXCEPTION2(BadIndexException, dom, nTensorFilters);
    }

    if (tensorFilter[dom] == NULL)
    {
        //
        // We don't have to initialize the filter now, since it will be done
        // in customize mappers later.
        //
        tensorFilter[dom] = vtkTensorGlyph::New();
    }

    tensorFilter[dom]->SetInput(ds);

    return tensorFilter[dom]->GetOutput();
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetScale
//
//  Purpose:
//      Sets the scale of each glyph.
//
//  Arguments:
//      s        The new scale.
//
//  Programmer:  Hank Childs 
//  Creation:    September 23, 2003 
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetScale(float s)
{
    scale = s;
    if (tensorFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (tensorFilter[i] != NULL)
            {
                tensorFilter[i]->SetScaleFactor(scale);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::ColorByMagOn
//
//  Purpose:
//      Tells the glyph mapper to color by the magnitude.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
// ****************************************************************************

void
avtTensorGlyphMapper::ColorByMagOn(void)
{
    colorByMag = true;

    if (tensorFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (tensorFilter[i] != NULL)
            {
                tensorFilter[i]->SetColorModeToEigenvalues();
            }
        }
    }
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::ColorByMagOff
//
//  Purpose:
//      Tells the glyph mapper to color all of the glyphs the same color.
//
//  Arguments:
//      col       The new color.
//
//  Programmer:   Hank Childs
//  Creation:     September 23, 2003
//
// ****************************************************************************

void
avtTensorGlyphMapper::ColorByMagOff(const unsigned char col[3])
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

    if (tensorFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (tensorFilter[i] != NULL)
            {
                tensorFilter[i]->SetColorModeToScalars();
            }
        }
    }
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetLookupTable
//
//  Purpose: 
//      Replaces the current lookup table. 
//
//  Arguments:
//   LUT       The new lookup table to be used. 
//
//  Programmer: Hank Childs 
//  Creation:   September 23, 2003
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetLookupTable(vtkLookupTable *LUT)
{
    if (lut == LUT)
    {
        // no need to change anything
        return;
    }
    lut = LUT;

    // Replace each mapper's lut.
    for (int i = 0 ; i < nMappers ; i++)
    {
        if (actors[i] != NULL)
        {
            mappers[i]->SetLookupTable(lut);
        }
    } 
}


