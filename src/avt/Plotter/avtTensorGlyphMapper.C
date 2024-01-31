// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtTensorGlyphMapper.C                        //
// ************************************************************************* //

#include <avtTensorGlyphMapper.h>

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkVisItTensorGlyph.h>

#include <avtExtents.h>

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
//    Eric Brugger, Wed Nov 24 13:04:09 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Kathleen Biagas, Thu Feb 7 08:45:03 PST 2013
//    Changed arg to vtkAlgorithmOutput for pipeline connections with VTK v 6.
//
//    Kathleen Biagas, Thu Feb 7 12:58:49 PST 2013
//    We don't want to own glyph, so don't up the ref count.
//
//    Kathleen Biagas, Thu Mar 14 13:04:18 PDT 2013
//    Remove normalsFilter.
//
// ****************************************************************************

avtTensorGlyphMapper::avtTensorGlyphMapper(vtkAlgorithmOutput *g)
{
    glyph = g;

    colorByMag        = true;
    scale             = 0.2;
    scaleByMagnitude  = true;
    autoScale         = true;
    tensorFilter      = NULL;
    nTensorFilters    = 0;
    lut = NULL;
    setMin = setMax = false;
    limitsMode = 0;  // use original data extents
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
//    Kathleen Biagas, Thu Feb 7 12:58:49 PST 2013
//    We don't own glyph, so don't delete it here.
//
//    Kathleen Biagas, Thu Mar 14 13:04:18 PDT 2013
//    Remove normalsFilter.
//
// ****************************************************************************

avtTensorGlyphMapper::~avtTensorGlyphMapper()
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
//    Eric Brugger, Wed Nov 24 13:04:09 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Kathleen Biagas, Thu Feb 7 08:46:37 PST 2013
//    Set the SourceConnection to the tensorFilter, not SourceData.
//
//    Kathleen Biagas, Thu Mar 14 13:04:18 PDT 2013
//    Remove normalsFilter.
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
                tensorFilter[i]->SetSourceConnection(glyph);
                if (scaleByMagnitude)
                    tensorFilter[i]->SetScaling(1);
                else
                    tensorFilter[i]->SetScaling(0);
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
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005 
//    Use VisIt version of TensorGlyph, so that original cell and node
//    arrays can be copied through. 
//
//    Kathleen Biagas, Thu Mar 14 13:04:18 PDT 2013
//    Remove normalsFilter.
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
    tensorFilter       = new vtkVisItTensorGlyph*[nTensorFilters];
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
//  Modifications:
//
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
//
//    Jeremy Meredith, Tue Jun  1 11:24:29 PDT 2004
//    Only do the normals if in 3-space.
//
//    Kathleen Bonnell, Tue Aug 30 15:11:01 PDT 2005 
//    Use VisIt version of TensorGlyph, so that original cell and node
//    arrays can be copied through. 
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PDT 2013
//    Changed signature of InsertFilters.
//
//    Kathleen Biagas, Thu Mar 14 13:04:18 PDT 2013
//    Remove normalsFilter.
//
// ****************************************************************************

vtkAlgorithmOutput *
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
        tensorFilter[dom] = vtkVisItTensorGlyph::New();
    }

    tensorFilter[dom]->SetInputData(ds);
    return tensorFilter[dom]->GetOutputPort();
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
//    Eric Brugger, Wed Nov 24 13:04:09 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetScale(double s)
{
    scale = s;

    //
    // If auto scale is enable, then set the scale based on the spatial
    // extents and possibly the data extents.
    //
    if (autoScale)
    {
        avtDataset_p input = GetTypedInput();
        if (*input != 0)
        {
            avtDataAttributes &atts=input->GetInfo().GetAttributes();
            avtExtents *extents = atts.GetOriginalSpatialExtents();
            int nDims = extents->GetDimension();
            double exts[6];
            extents->CopyTo(exts);
            double dist = 0.;
            int i;
            for (i = 0; i < nDims; i++)
            {
                dist += (exts[2*i+1] - exts[2*i]) * (exts[2*i+1] - exts[2*i]);
            }
            dist = sqrt(dist);

            extents = atts.GetOriginalDataExtents();
            extents->CopyTo(exts);

            if (scaleByMagnitude)
                scale = (scale * dist * 0.2) / exts[1];
            else
                scale = scale * dist * 0.2;
        }
    }

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
//  Method: avtTensorGlyphMapper::SetScaleByMagnitude
//
//  Purpose:
//      Sets the scale by magnitude mode.
//
//  Arguments:
//      val      The new scale by magnitude mode.
//
//  Programmer:  Eric Brugger
//  Creation:    November 24, 2004
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetScaleByMagnitude(bool val)
{
    scaleByMagnitude = val;

    if (tensorFilter != NULL)
    {
        for (int i = 0 ; i < nTensorFilters ; i++)
        {
            if (tensorFilter[i] != NULL)
            {
                if (scaleByMagnitude)
                    tensorFilter[i]->SetScaling(1);
                else
                    tensorFilter[i]->SetScaling(0);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetAutoScale
//
//  Purpose:
//      Sets the auto scale mode.
//
//  Arguments:
//      val      The new auto scale mode.
//
//  Programmer:  Eric Brugger
//  Creation:    November 24, 2004
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetAutoScale(bool val)
{
    autoScale = val;
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
                double r = ((double) glyphColor[0]) / 255.;
                double g = ((double) glyphColor[1]) / 255.;
                double b = ((double) glyphColor[2]) / 255.;
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

// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetMin
//
//  Purpose:
//      Sets the plotter's scalar min.
//
//  Arguments:
//      minArg      The new minimum.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004 
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetMin(double minArg)
{
    if (setMin == true && min == minArg)
    {
        return;
    }

    min    = minArg;
    setMin = true;

    SetMappersMinMax();
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetMinOff
//
//  Purpose:
//      Sets the bounds for the scalar's min to be off.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004 
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetMinOff(void)
{
    if (setMin == false)
    {
        return;
    }
    setMin = false;

    SetMappersMinMax();
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetMax
//
//  Purpose:
//      Sets the plotter's scalar max.
//
//  Arguments:
//      maxArg      The new maximum.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004 
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetMax(double maxArg)
{
    if (setMax == true && max == maxArg)
    {
        return;
    }

    max    = maxArg;
    setMax = true;

    SetMappersMinMax();
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetMaxOff
//
//  Purpose:
//      Sets the bounds for the scalar's max to be off.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004 
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetMaxOff(void)
{
    if (setMax == false)
    {
        return;
    }
    setMax = false;

    SetMappersMinMax();
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetMappersMinMax
//
//  Purpose:
//      Sets the mappers min/max.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetMappersMinMax(void)
{
    if (mappers == NULL)
    {
        //
        // This happens when SetMin is called before the mappers are
        // initialized.
        //
        return;
    }

    double mmin = 0.;
    double mmax = 0.;

    if (limitsMode == 1 ) // use current plot extents
    {
        GetCurrentRange(mmin, mmax);
    }
    else  // use either original data extents or user-specified limits
    {
        GetRange(mmin, mmax);
    }

    for (int i = 0 ; i < nMappers ; i++)
    {
        if (mappers[i] != NULL)
        {
            mappers[i]->SetScalarRange(mmin, mmax);
        }
    }
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::GetRange
//
//  Purpose:
//      Gets the range of the variable, taking into account artificial limits.
//
//  Arguments:
//      rmin    The minimum in the range.
//      rmax    The maximum in the range.
//
//  Returns:    True if the extents were found, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004
//
//  Modifications:
//
// ****************************************************************************

bool
avtTensorGlyphMapper::GetRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmax = 1.;
        return false;
    }

    double de[2];
    bool gotExtents = avtMapper::GetRange(de[0], de[1]);

    rmin = (setMin ? min : de[0]);
    rmax = (setMax ? max : de[1]);

    return gotExtents;
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::GetCurrentRange
//
//  Purpose:
//      Gets the current range of the variable. 
//
//  Arguments:
//      rmin          The minimum in the range.
//      rmax          The maximum in the range.
//
//  Returns:    True if the extents were found, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004 
//
// ****************************************************************************

bool
avtTensorGlyphMapper::GetCurrentRange(double &rmin, double &rmax)
{
    if (mappers == NULL)
    {
        //
        // We have been asked for the range before the input has been set.
        //
        rmin = 0.;
        rmax = 1.;
        return false;
    }

    double de[2];
    bool rv = avtMapper::GetCurrentRange(de[0], de[1]);

    rmin = (setMin ? min : de[0]);
    rmax = (setMax ? max : de[1]);

    return rv;
}


// ****************************************************************************
//  Method: avtTensorGlyphMapper::SetLimitsMode
//
//  Purpose:
//    Sets the limits mode, which specifies which type of limits to use.
//
//  Arguments:
//    lm        The new limits mode.
//                0: Use Original Data limits
//                1: Use Current Plot limits
//                2: Use user-specified limits. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 22, 2004
//
// ****************************************************************************

void
avtTensorGlyphMapper::SetLimitsMode(const int lm)
{
    if (lm == limitsMode)
    {
        return;
    }

    limitsMode = lm;

    SetMappersMinMax();
}


