// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtVectorGlyphMapper.C                        //
// ************************************************************************* //

#include <avtVectorGlyphMapper.h>

#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkDataSetMapper.h>
#include <vtkLookupTable.h>
#include <vtkProperty.h>
#include <vtkVisItGlyph3D.h>

#include <avtExtents.h>

#include <BadIndexException.h>

#include <string>


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
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
//
//    Kathleen Bonnell, Mon Aug  9 13:54:42 PDT 2004 
//    Initialize colorByScalar.
//
//    Eric Brugger, Tue Nov 23 12:28:20 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004 
//    Initialize setMin, setMax and limitsMode.
//
//    Kathleen Biagas, Web Feb 6 19:38:27 PST 2013
//    Changed arg to vtkAlgorithmOutput for pipeline connections with VTK v 6.
//
//    Kathleen Biagas, Thu Feb 7 12:58:49 PST 2013
//    We don't want to own glyph, so don't up the ref count.
//
//    Alister Maguire, Mon Apr 27 13:20:17 PDT 2020
//    Removed normalsFilter as the required deep copy within seems to cause
//    issues with picking, and ommitting the filter doesn't change the
//    rendering in any noticable way.
//
// ****************************************************************************

avtVectorGlyphMapper::avtVectorGlyphMapper(vtkAlgorithmOutput *g)
{
    glyph = g;

    lineWidth         = LW_0;
    colorByMag        = true;
    colorByScalar     = false;
    scale             = 0.2;
    scaleByMagnitude  = true;
    autoScale         = true;
    glyphFilter       = 0;
    nGlyphFilters     = 0;
    lut = NULL;
    setMin = setMax = false;
    limitsMode = 0;  // use original data extents
}

// ****************************************************************************
//  Method: avtVectorGlyphMapper destructor
//
//  Programmer: Hank Childs
//  Creation:   March 23, 2001
//
//  Modifications:
//
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Deleted poly data normals.
//
//    Kathleen Biagas, Thu Feb 7 12:58:49 PST 2013
//    We don't own glyph, so don't delete it here.
//
// ****************************************************************************

avtVectorGlyphMapper::~avtVectorGlyphMapper()
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
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
//
//    Kathleen Bonnell, Mon Aug  9 13:54:42 PDT 2004 
//    Test for flag colorByScalar. 
//
//    Eric Brugger, Tue Nov 23 12:28:20 PST 2004
//    Added scaleByMagnitude.
//
//    Kathleen Bonnell, Wed Dec 22 16:42:35 PST 2004 
//    Added code for setting min and max.
//
//    Hank Childs, Fri Sep 14 09:56:07 PDT 2007
//    Tell the glyph filter to treat 2D vectors as 2D.
//
// ****************************************************************************

void
avtVectorGlyphMapper::CustomizeMappers(void)
{
    if (setMin)
    {
        SetMin(min);
    }
    else
    {
        SetMinOff();
    }

    if (setMax)
    {
        SetMax(max);
    }
    else
    {
        SetMaxOff();
    }

    //
    // It is probable that the mappers defaults did not actually get set, so
    // explicitly do that here.
    //
    SetMappersMinMax();

    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetSourceConnection(glyph);
                if (scaleByMagnitude)
                    glyphFilter[i]->SetScaleModeToScaleByVector();
                else
                    glyphFilter[i]->SetScaleModeToDataScalingOff();

                if (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() == 2)
                    glyphFilter[i]->SetTreatVectorsAs2D(1);
            }
        }
    }
         
    if (colorByMag)
    {
        ColorByMagOn();
    }
    else if (colorByScalar)
    {
        ColorByScalarOn(scalarName);
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
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
//
//    Kathleen Bonnell, Tue Oct 12 16:18:37 PDT 2004
//    Use VisIt's version of vtkGlyph3D. 
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

    nGlyphFilters = nDoms;
    glyphFilter   = new vtkVisItGlyph3D*[nGlyphFilters];
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
//    Hank Childs, Wed May  5 14:19:54 PDT 2004
//    Added poly data normals.
//
//    Jeremy Meredith, Tue Jun  1 11:24:29 PDT 2004
//    Only do the normals if in 3-space.
//
//    Kathleen Bonnell, Tue Oct 12 16:18:37 PDT 2004
//    Use VisIt's version of vtkGlyph3D. 
//
//    Kathleen Biagas, Wed Feb 6 19:38:27 PDT 2013
//    Changed signature of InsertFilters.
//
//    Alister Maguire, Mon Apr 27 13:20:17 PDT 2020
//    Removed normalsFilter as the required deep copy within seems to cause
//    issues with picking, and ommitting the filter doesn't change the
//    rendering in any noticable way.
//
// ****************************************************************************

vtkAlgorithmOutput *
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
        glyphFilter[dom] = vtkVisItGlyph3D::New();
    }

    glyphFilter[dom]->SetInputData(ds);
    return glyphFilter[dom]->GetOutputPort();
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
//  Modifications:
//    Eric Brugger, Tue Nov 23 12:28:20 PST 2004
//    Added scaleByMagnitude and autoScale.
//
//    Hank Childs, Thu Aug 26 13:47:30 PDT 2010
//    Change extents names.
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetScale(double s)
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
//  Method: avtVectorGlyphMapper::SetScaleByMagnitude
//
//  Purpose:
//      Sets the scale by magnitude mode.
//
//  Arguments:
//      val      The new scale by magnitude mode.
//
//  Programmer:  Eric Brugger
//  Creation:    November 23, 2004 
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetScaleByMagnitude(bool val)
{
    scaleByMagnitude = val;

    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                if (scaleByMagnitude)
                    glyphFilter[i]->SetScaleModeToScaleByVector();
                else
                    glyphFilter[i]->SetScaleModeToDataScalingOff();
            }
        }
    }
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetAutoScale
//
//  Purpose:
//      Sets the auto scale mode.
//
//  Arguments:
//      val      The new auto scale mode.
//
//  Programmer:  Eric Brugger
//  Creation:    November 23, 2004 
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetAutoScale(bool val)
{
    autoScale = val;
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
//  Modifications:
//    Kathleen Bonnell, Mon Aug  9 13:54:42 PDT 2004
//    Set colorByScalar to false.
//
//    Kathleen Bonnell, Mon Jul 25 08:34:49 PDT 2005 
//    Turn on mappers' scalar visibility. 
//
// ****************************************************************************

void
avtVectorGlyphMapper::ColorByMagOn(void)
{
    colorByMag = true;
    colorByScalar = false;


    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                mappers[i]->ScalarVisibilityOn();
            }
        }
    }


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
//  Method: avtVectorGlyphMapper::ColorByScalarOn
//
//  Purpose:
//      Tells the glyph mapper to color by the named scalar variable.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 6, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 25 08:34:49 PDT 2005 
//    Turn on mappers' scalar visibility. 
//
// ****************************************************************************

void
avtVectorGlyphMapper::ColorByScalarOn(const std::string &sn)
{
    colorByMag = false;
    colorByScalar = true;
    scalarName = sn;


    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                mappers[i]->ScalarVisibilityOn();
            }
        }
    }

    if (glyphFilter != NULL)
    {
        for (int i = 0 ; i < nGlyphFilters ; i++)
        {
            if (glyphFilter[i] != NULL)
            {
                glyphFilter[i]->SetColorModeToColorByScalar();
                glyphFilter[i]->SelectInputScalars(scalarName.c_str());
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
//  Modifications:
//    Kathleen Bonnell, Mon Aug  9 13:54:42 PDT 2004
//    Set colorByScalar to false.
//
//    Kathleen Bonnell, Mon Jul 25 08:34:49 PDT 2005 
//    Turn Mappers' scalar visibility off,  set color mode for the glyph 
//    filter to 'SingleColor' to ensure that no unnecessary scalars are
//    created.
//
// ****************************************************************************

void
avtVectorGlyphMapper::ColorByMagOff(const unsigned char col[3])
{
    glyphColor[0] = col[0];
    glyphColor[1] = col[1];
    glyphColor[2] = col[2];
    colorByMag = false;
    colorByScalar = false;
  
    if (actors != NULL)
    {
        for (int i = 0 ; i < nMappers ; i++)
        {
            if (actors[i] != NULL)
            {
                mappers[i]->ScalarVisibilityOff();
                vtkProperty *prop = actors[i]->GetProperty();
                double r = ((double) glyphColor[0]) / 255.;
                double g = ((double) glyphColor[1]) / 255.;
                double b = ((double) glyphColor[2]) / 255.;
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
               // Will ensure no new scalars are created accidentally.
                glyphFilter[i]->SetColorModeToColorBySingleColor();
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


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetMin
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
avtVectorGlyphMapper::SetMin(double minArg)
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
//  Method: avtVectorGlyphMapper::SetMinOff
//
//  Purpose:
//      Sets the bounds for the scalar's min to be off.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004 
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetMinOff(void)
{
    if (setMin == false)
    {
        return;
    }
    setMin = false;

    SetMappersMinMax();
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetMax
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
avtVectorGlyphMapper::SetMax(double maxArg)
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
//  Method: avtVectorGlyphMapper::SetMaxOff
//
//  Purpose:
//      Sets the bounds for the scalar's max to be off.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 22, 2004 
//
// ****************************************************************************

void
avtVectorGlyphMapper::SetMaxOff(void)
{
    if (setMax == false)
    {
        return;
    }
    setMax = false;

    SetMappersMinMax();
}


// ****************************************************************************
//  Method: avtVectorGlyphMapper::SetMappersMinMax
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
avtVectorGlyphMapper::SetMappersMinMax(void)
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
//  Method: avtVectorGlyphMapper::GetRange
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
avtVectorGlyphMapper::GetRange(double &rmin, double &rmax)
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
//  Method: avtVectorGlyphMapper::GetCurrentRange
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
avtVectorGlyphMapper::GetCurrentRange(double &rmin, double &rmax)
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
//  Method: avtVectorGlyphMapper::SetLimitsMode
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
avtVectorGlyphMapper::SetLimitsMode(const int lm)
{
    if (lm == limitsMode)
    {
        return;
    }

    limitsMode = lm;

    SetMappersMinMax();
}


// ****************************************************************************
// Method: avtVectorGlyphMapper::SetFullFrameScaling
//
// Purpose: 
//   Sets a fullframe scale factor that can be used by the mapper to compensate
//   for the stretching that fullframe mode performs on geometry.
//
// Arguments:
//   useScale : True if the scale is used.
//   s        : The fullframe scale vector.
//
// Returns:    True if any vtk mappers use the scale.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 24 13:55:54 PST 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtVectorGlyphMapper::SetFullFrameScaling(bool useScale, const double *s)
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
