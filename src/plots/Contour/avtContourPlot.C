/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtContourPlot.C                             //
// ************************************************************************* //

#include <avtContourPlot.h>

#include <avtColorTables.h>
#include <avtContourFilter.h>
#include <avtLevelsLegend.h>
#include <avtLevelsMapper.h>
#include <avtLookupTable.h>
#include <ContourAttributes.h>
#include <avtFeatureEdgesFilter.h>

#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>

#include <LineAttributes.h>
#include <InvalidColortableException.h>

// ****************************************************************************
//  Method: avtContourPlot constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 15, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Removed data members varMin & varMax, and added legend. 
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001 
//    Added avtLUT.
//
// ****************************************************************************

avtContourPlot::avtContourPlot()
{
    levelsMapper  = new avtLevelsMapper;
    levelsLegend  = new avtLevelsLegend;
    levelsLegend->SetTitle("Contour");
    avtLUT        = new avtLookupTable;
    numLevels     = 0;
    contourFilter = NULL; 

    //
    // This is to allow the legend to reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a
    // LevelsLegend.
    //
    levLegendRefPtr = levelsLegend;
}


// ****************************************************************************
//  Method: avtContourPlot destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 15, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//    Added avtLUT.
//
// ****************************************************************************

avtContourPlot::~avtContourPlot()
{
    if (levelsMapper != NULL)
    {
        delete levelsMapper;
        levelsMapper = NULL;
    }

    if (contourFilter != NULL)
    {
        delete contourFilter;
        contourFilter = NULL;
    }

    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }

    //
    // Do not delete the levelsLegend since it is being held by levLegendRefPtr.
    //
}


// ****************************************************************************
//  Method:  avtContourPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************

avtPlot*
avtContourPlot::Create()
{
    return new avtContourPlot;
}


// ****************************************************************************
//  Method: avtContourPlot::SetAtts
//
//  Purpose:
//      Sets the plot with the attributes.
//
//  Arguments:
//      atts    The attributes for this isocontour plot.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 15, 2001 
//
//  Modifications:
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Revised logic, added call to SetIsoValues if var extents are valid. 
//
//  Modifications:
//    Jeremy Meredith, Fri Mar  2 13:10:02 PST 2001
//    Made this method take a generic AttributeGroup since it is now virtual.
//    Also made some variables const.
//
//    Kathleen Bonnell, Tue Mar 13 11:35:45 PST 2001 
//    Added call to SetLegend. 
//
//    Kathleen Bonnell, Wed Mar 28 17:18:05 PST 2001 
//    Removed references to ContourAttributes::VarMin/Max as they are no
//    longer members. Added min/max arguments to SetIsoValues. 
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Made atts a member so it can be accesses from other methods.
//    Removed call to SetIsoValues as it is called from ApplyOperators and
//    CustomizeBehavior. 
//    
//    Jeremy Meredith, Tue Jun  5 20:33:13 PDT 2001
//    Added code to set a flag if the plot needs recalculation.
//
//    Hank Childs, Sun Jun 17 18:47:57 PDT 2001
//    Pushed code into avtContourFilter.
//
//    Kathleen Bonnell, Mon Jun 25 14:33:59 PDT 2001 
//    Set numLevels based on atts. Set colors in avtLUT and use it
//    to set mapper's. lut.
//
//    Kathleen Bonnell, Mon Sep 24 15:46:23 PDT 2001
//    avtLevelsMapper no longer uses a vtkLookupTable.  Set its colors
//    with ColorAttributeList instead. 
//
//    Brad Whitlock, Fri Nov 22 14:33:50 PST 2002
//    I moved the color code to SetColors.
//
//    Kathleen Bonnell, Mon Sep 29 13:13:32 PDT 2003 
//    Set AntialiasedRenderOrder dependent upon Wireframe mode.
//
//    Kathleen Bonnell, Thu Sep  2 11:44:09 PDT 2004 
//    Ensure that specular properties aren't used in wireframe mode.
//
// ****************************************************************************

void
avtContourPlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const ContourAttributes*)a);
    atts = *(const ContourAttributes*)a;

    if (atts.GetContourMethod() == ContourAttributes::Level)
    {
        numLevels = atts.GetContourNLevels();
    }
    else if (atts.GetContourMethod() == ContourAttributes::Value)
    {
        numLevels = atts.GetContourValue().size();
    }
    else 
    {
        numLevels = atts.GetContourPercent().size();
    }

    SetColors();
    SetLineWidth(atts.GetLineWidth());
    SetLineStyle(atts.GetLineStyle());
    SetLegend(atts.GetLegendFlag());
    if (atts.GetWireframe())
    {
        behavior->SetAntialiasedRenderOrder(ABSOLUTELY_LAST);
        levelsMapper->SetSpecularIsInappropriate(true);
    }
    else 
    {
        behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
        levelsMapper->SetSpecularIsInappropriate(false);
    }
}

// ****************************************************************************
//  Method: avtContourPlot::GetDataExtents
//
//  Purpose:
//      Gets the data extents used by the plot.
//
//  Arguments:
//      extents The extents used by the plot.
//
//  Programmer: Eric Brugger
//  Creation:   March 25, 2004 
//
// ****************************************************************************

void
avtContourPlot::GetDataExtents(vector<double> &extents)
{
    double min, max;

    if (dataExtents.size() == 2)
    {
        min = dataExtents[0];
        max = dataExtents[1];
    }
    else
    {
        levelsMapper->GetOriginalDataRange(min, max);

        if (atts.GetMinFlag())
        {
            min = atts.GetMin();
        }
        if (atts.GetMaxFlag())
        {
            max = atts.GetMax();
        }
    }

    extents.push_back(min);
    extents.push_back(max);
}

// ****************************************************************************
// Method: avtContourPlot::SetColorTable
//
// Purpose: 
//   Sets the plot's color table if the color table is the same as that of
//   the plot or we are using the default color table for the plot.
//
// Arguments:
//   ctName : The name of the color table to use.
//
// Returns:    Returns true if the color table is updated.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 3 09:33:47 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
avtContourPlot::SetColorTable(const char *ctName)
{
    bool retval = false;

    if(atts.GetColorType() == ContourAttributes::ColorByColorTable)
    {
        // If the color table is "Default" or is the color table that is being
        // changed, set the colors.
        bool usesCT = (atts.GetColorTableName() == std::string(ctName));
        if (usesCT || atts.GetColorTableName() == "Default")
        {
            SetColors();
            retval = true;
        }
    }

    return retval;
}

// ****************************************************************************
// Method: avtContourPlot::SetColors
//
// Purpose: 
//   Sets the plot's colors.
//
// Programmer: Kathleen Bonnell
// Creation:   Tue Dec 3 09:35:05 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Dec 3 09:35:14 PDT 2002
//   I moved code from SetAtts to create this method then I added the
//   color table coloring case.
//
//   Brad Whitlock, Fri Apr 25 10:35:28 PDT 2003
//   I made it throw an InvalidColortableException.
//
// ****************************************************************************

void
avtContourPlot::SetColors()
{
    if (atts.GetColorType() == ContourAttributes::ColorBySingleColor)
    {
        const ColorAttribute ca = atts.GetSingleColor();
        ColorAttributeList cal; 
        cal.AddColorAttribute(atts.GetSingleColor());
        avtLUT->SetLUTColorsWithOpacity(ca.GetColor(), 1);
        levelsMapper->SetColors(cal);
    }
    else if (atts.GetColorType() == ContourAttributes::ColorByMultipleColors)
    {
        const ColorAttributeList &cal = atts.GetMultiColor();
        unsigned char *colors = new unsigned char[numLevels * 4];
        unsigned char *cptr = colors;
        for(int i = 0; i < numLevels; i++)
        {
            *cptr++ = (char)cal[i].Red();
            *cptr++ = (char)cal[i].Green();
            *cptr++ = (char)cal[i].Blue();
            *cptr++ = (char)cal[i].Alpha();
        }
        avtLUT->SetLUTColorsWithOpacity(colors, numLevels);
        levelsMapper->SetColors(cal);

        // Delete the temp color array.
        delete [] colors;
    }
    else // ColorByColorTable
    {
        ColorAttributeList cal(atts.GetMultiColor());
        unsigned char *colors = new unsigned char[numLevels * 4];
        unsigned char *cptr = colors;
        avtColorTables *ct = avtColorTables::Instance();
        const int opacity = 255;

        //
        // Detect if we're using the default color table or a color table
        // that does not exist anymore.
        //
        string ctName(atts.GetColorTableName());
        if(ctName == "Default")
            ctName = string(ct->GetDefaultDiscreteColorTable());
        else if(!ct->ColorTableExists(ctName.c_str()))
        {
            delete [] colors;
            EXCEPTION1(InvalidColortableException, ctName);
        }

        // 
        // Add a color for each subset name.
        //
        if(ct->IsDiscrete(ctName.c_str()))
        {
            // The CT is discrete, get its color color control points.
            for(int i = 0; i < numLevels; ++i)
            {
                unsigned char rgb[3] = {0,0,0};
                ct->GetControlPointColor(ctName.c_str(), i, rgb);
                *cptr++ = rgb[0];
                *cptr++ = rgb[1];
                *cptr++ = rgb[2];
                *cptr++ = opacity;

                cal[i].SetRgba(rgb[0], rgb[1], rgb[2], opacity);
            }
        }
        else
        {
            // The CT is continuous, sample the CT so we have a unique color
            // for each element.
            unsigned char *rgb = ct->GetSampledColors(ctName.c_str(), numLevels);
            if(rgb)
            {
                for(int i = 0; i < numLevels; ++i)
                {
                     int j = i * 3;
                     *cptr++ = rgb[j];
                     *cptr++ = rgb[j+1];
                     *cptr++ = rgb[j+2];
                     *cptr++ = opacity;

                     cal[i].SetRgba(rgb[j], rgb[j+1], rgb[j+2], opacity);
                }
                delete [] rgb;
            }
        }
        avtLUT->SetLUTColorsWithOpacity(colors, numLevels);
        levelsMapper->SetColors(cal);

        delete [] colors;
    }
}

// ****************************************************************************
//  Method: avtContourPlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 13, 2001 
//
// ****************************************************************************

void
avtContourPlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        levelsLegend->LegendOn();
    }
    else
    {
        levelsLegend->LegendOff();
    }
}


// ****************************************************************************
//  Method: avtContourPlot::SetLineWidth
//
//  Purpose:
//      Sets the line width.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 15, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use LineAttributes to ensure proper format gets sent to vtk.
//
// ****************************************************************************

void
avtContourPlot::SetLineWidth(int lw)
{
    levelsMapper->SetLineWidth(Int2LineWidth(lw));
}


// ****************************************************************************
//  Method: avtContourPlot::SetLineStyle
//
//  Purpose:
//      Sets the line style.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 15, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Sat Aug 18 18:09:04 PDT 2001
//    Use LineAttributes to ensure proper format gets sent to vtk.
//
// ****************************************************************************

void
avtContourPlot::SetLineStyle(int ls)
{
    levelsMapper->SetLineStyle(Int2LineStyle(ls));
}


// ****************************************************************************
//  Method: avtContourPlot::GetMapper
//
//  Purpose:
//      Gets the levels mapper as its base class (avtMapper) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 15, 2001
//
// ****************************************************************************

avtMapper *
avtContourPlot::GetMapper(void)
{
    return levelsMapper;
}


// ****************************************************************************
//  Method: avtContourPlot::ApplyOperators
//
//  Purpose:
//      Performs the implied operators for an isocontour plot, namely,
//      an avtContourFilter. 
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the contour filter is applied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   February 15, 2001
//
//  Notes:  This method determines the isoValues to send to the filter
//          based on parameters set by method SetAtts.
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Added call to method SetIsoValues. Removed logic involving varMin
//    and varMax.
//
//    Hank Childs, Sun Mar 25 12:28:25 PST 2001
//    Fixed spot where data attributes were being used incorrectly.
//
//    Kathleen Bonnell, Wed Mar 28 17:18:05 PST 2001 
//    Added arguments in call to SetIsoValues.  
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Removed check for empty isoValues before call to SetIsoValues,
//    as the logic is part of the SetIsoValues method itself. 
//
//    Hank Childs, Fri Jun 15 09:14:24 PDT 2001
//    Changed signature for more general data objects.
//
//    Jeremy Meredith, Wed Mar 13 11:16:25 PST 2002
//    Added a wireframe mode.
//
//    Kathleen Bonnell, Mon Jun 24 15:09:37 PDT 2002  
//    Fix potential memory leak with contourFilter. 
//
//    Kathleen Bonnell, Tue Oct 22 08:33:26 PDT 2002 
//    Moved feature edges filter to ApplyRenderingTransformation, so that
//    output of this method would be a queryable object. 
//
//    Eric Brugger, Thu Mar 25 16:42:45 PST 2004
//    I added code to use the data extents from the base class if set.
//
// ****************************************************************************

avtDataObject_p
avtContourPlot::ApplyOperators(avtDataObject_p input)
{
    //
    // Very clumsy.  I should really go back and make this be a formal part of
    // the ContourAttributes and just get this sub-part.
    //
    ContourOpAttributes catts;
    catts.SetContourNLevels(atts.GetContourNLevels());
    catts.SetContourValue(atts.GetContourValue());
    catts.SetContourPercent(atts.GetContourPercent());
    catts.SetContourMethod(ContourOpAttributes::ContourMethod(
        atts.GetContourMethod()));
    if (dataExtents.size() == 2)
    {
        catts.SetMinFlag(true);
        catts.SetMaxFlag(true);
        catts.SetMin(dataExtents[0]);
        catts.SetMax(dataExtents[1]);
    }
    else
    {
        catts.SetMinFlag(atts.GetMinFlag());
        catts.SetMaxFlag(atts.GetMaxFlag());
        catts.SetMin(atts.GetMin());
        catts.SetMax(atts.GetMax());
    }
    catts.SetScaling(ContourOpAttributes::ContourScaling(atts.GetScaling()));

    if (contourFilter != NULL)
    {
        delete contourFilter;
    }
    contourFilter = new avtContourFilter(catts);
    contourFilter->SetInput(input);

    return contourFilter->GetOutput();
}

// ****************************************************************************
//  Method: avtContourPlot::ApplyOperators
//
//  Purpose:
//      Performs the rendering transformation for an isocontour plot, namely,
//      an avtFeatureEdgesFilter. 
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the feature edges is applied.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 22, 2002 
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtContourPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    if (atts.GetWireframe())
    {
        avtFeatureEdgesFilter *edgeFilter = new avtFeatureEdgesFilter();
        edgeFilter->SetInput(input);
        return edgeFilter->GetOutput();
    }
    return input;
}


// ****************************************************************************
//  Method: avtContourPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior of the output.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   February  15, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Mar  2 11:34:46 PST 2001
//    Added Legend
//
//    Hank Childs, Mon Mar 12 16:59:34 PST 2001
//    Added shift factor.
//
//    Kathleen Bonnell, Wed Mar 28 17:18:05 PST 2001 
//    Added arguments in call to SetIsoValues. Changed Call to levelsMapper
//    Get DataRange method to reflect new name.
//
//    Kathleen Bonnell, Tue Apr  3 08:56:47 PDT 2001 
//    Change call to retrieve data range.  The correct behavior is to retrieve
//    the original data extents, as reflected in the new method call.  Added
//    call to set the legend's VarRange so that limit text will reflect real
//    var extents, even when artificial limits are set.
//    
//    Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001 
//    Use avtLUT to set legend's lut.  Use better test for whether or
//    not to display the legend.  Use information in atts to set
//    more reasonable legend labels.  Still needs work!
//
//    Kathleen Bonnell, Sat Sep 22 12:13:57 PDT 2001 
//    Removed logic for setting legend's labels.  Now contained in 
//    CustomizeMapper. 
//    
//    Eric Brugger, Wed Jul 16 10:27:29 PDT 2003
//    Modified to work with the new way legends are managed.
//
// ****************************************************************************

void
avtContourPlot::CustomizeBehavior(void)
{
    // 
    //  Need to get the data range from mapper, and 
    //  recreate the isoValues so they can be sent to the legend.
    //
    double min, max;
    levelsMapper->GetOriginalDataRange(min, max);

    //
    // Legend limits text should be the original data extents. 
    //
    levelsLegend->SetVarRange(min, max);

    levelsLegend->SetLookupTable(avtLUT->GetLookupTable());

    if (numLevels == 0 || min == max)
    {
        levelsLegend->SetColorBarVisibility(0);
        levelsLegend->SetMessage("Constant, no levels");
    }
    else
    { 
        levelsLegend->SetColorBarVisibility(1);
        levelsLegend->SetMessage(NULL);
    }

    behavior->SetLegend(levLegendRefPtr);
    behavior->SetShiftFactor(0.5);
}


// ****************************************************************************
//  Method: avtContourPlot::CustomizeMapper
//
//  Purpose:
//    Use the info to set the isolevels in the legend.  
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 22, 2001 
//
//  Modifications:
//    Eric Brugger, Wed Jul 16 10:27:29 PDT 2003
//    Modified to work with the new way legends are managed.
//
// ****************************************************************************

void
avtContourPlot::CustomizeMapper(avtDataObjectInformation &info)
{
    vector<string> isoValues;
    info.GetAttributes().GetLabels(isoValues);

    if (!isoValues.empty())
    {
        levelsLegend->SetLevels(isoValues);
        levelsLegend->SetColorBarVisibility(1);
        levelsLegend->SetMessage(NULL);
    }
    else
    { 
        levelsLegend->SetColorBarVisibility(0);
        levelsLegend->SetMessage("Unable to compute levels");
    }
}


// ****************************************************************************
//  Method: avtContourPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2002
//
// ****************************************************************************

void
avtContourPlot::ReleaseData(void)
{
    avtSurfaceDataPlot::ReleaseData();

    if (contourFilter != NULL)
    {
        contourFilter->ReleaseData();
    }
}


