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
#include <vtkVisItPolyDataNormals.h>

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
//  Modifications:
//
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
//
// ****************************************************************************

avtTensorGlyphMapper::avtTensorGlyphMapper(vtkPolyData *g)
{
    glyph = g;
    glyph->Register(NULL);

    colorByMag        = true;
    scale             = 0.2;
    tensorFilter      = NULL;
    normalsFilter     = NULL;
    nTensorFilters    = 0;
    lut = NULL;
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Deleted poly data normals.
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
    if (normalsFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
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
//  Method: avtTensorGlyphMapper::CustomizeMappers
//
//  Purpose:
//      A hook from the base class that allows the variable mapper to force
//      the vtk mappers to be the same as its state.
//
//  Programmer: Hank Childs
//  Creation:   September 23, 2003
//
//  Modifications:
//
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
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
            if (normalsFilter[i] != NULL)
            {
                normalsFilter[i]->SetNormalTypeToCell();
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
//  Modifications:
//
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
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
    if (normalsFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (normalsFilter[i] != NULL)
            {
                normalsFilter[i]->Delete();
            }
        }
        delete [] normalsFilter;
    }

    nTensorFilters     = nDoms;
    tensorFilter       = new vtkTensorGlyph*[nTensorFilters];
    normalsFilter      = new vtkVisItPolyDataNormals*[nTensorFilters];
    for (int i = 0 ; i < nTensorFilters ; i++)
    {
        tensorFilter[i] = NULL;
        normalsFilter[i] = NULL;
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
//  Modifications:
//
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
//
//    Jeremy Meredith, Tue Jun  1 11:24:29 PDT 2004
//    Only do the normals if in 3-space.
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
    if (normalsFilter[dom] == NULL)
    {
        normalsFilter[dom] = vtkVisItPolyDataNormals::New();
    }

    tensorFilter[dom]->SetInput(ds);

    if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 3)
    {
        normalsFilter[dom]->SetInput(tensorFilter[dom]->GetOutput());
        return normalsFilter[dom]->GetOutput();
    }
    else
    {
        return tensorFilter[dom]->GetOutput();
    }
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
//  Modifications:
//
//    Hank Childs, Fri May  7 07:38:31 PDT 2004
//    Reset the default range so the colors update properly.
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

    SetDefaultRange();
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


