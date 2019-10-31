/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
*****************************************************************************/

// ************************************************************************* //
//                              avtScatterPlot.C                             //
// ************************************************************************* //

#include <avtScatterPlot.h>

#include <ScatterAttributes.h>

#include <avtScatterFilter.h>
#include <avtVariableLegend.h>
#include <avtVariablePointGlyphMapper.h>

#include <avtLookupTable.h>
#include <InvalidLimitsException.h>
#include <MapNode.h>

#include <DebugStream.h>

#include <cstring>

#include <string>

// ****************************************************************************
//  Method: avtScatterPlot constructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
//  Modifications:
//
// ****************************************************************************

avtScatterPlot::avtScatterPlot() : avtPlot(), atts()
{
    filter = NULL;
    colorsInitialized = false;
    fgColor[0] = fgColor[1] = fgColor[2] = 0.;

    glyphMapper = new avtVariablePointGlyphMapper();

    varLegend = new avtVariableLegend;
    varLegend->SetTitle("Scatter");

    avtLUT = new avtLookupTable;

    //
    // This is to allow the legend to be reference counted so the behavior can
    // still access it after the plot is deleted.  The legend cannot be
    // reference counted all of the time since we need to know that it is a
    // VariableLegend.
    //
    varLegendRefPtr = varLegend;
}


// ****************************************************************************
//  Method: avtScatterPlot destructor
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
//  Modifications:
//
// ****************************************************************************

avtScatterPlot::~avtScatterPlot()
{
    if (filter != NULL)
    {
        delete filter;
        filter = NULL;
    }
    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }
    if (glyphMapper != NULL)
    {
        delete glyphMapper;
        glyphMapper = NULL;
    }

    //
    // Do not delete the varLegend since it is being held by varLegendRefPtr.
    //
}


// ****************************************************************************
//  Method:  avtScatterPlot::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Nov 2 22:10:12 PST 2004
//
// ****************************************************************************

avtPlot*
avtScatterPlot::Create()
{
    return new avtScatterPlot;
}

// ****************************************************************************
//  Method: avtScatterPlot::SetScaling
//
//  Purpose:
//      Sets the color scaling mode for the scatter plot.
//
//  Arguments:
//      atts    The attributes for this scatter plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtScatterPlot::SetScaling(int mode, double skew)
{
    varLegend->SetLookupTable(avtLUT->GetLookupTable());
    varLegend->SetScaling(mode, skew);

    if (mode == 1)
    {
       glyphMapper->SetLUT(avtLUT->GetLogLookupTable());
    }
    else if (mode == 2)
    {
       avtLUT->SetSkewFactor(skew);
       glyphMapper->SetLUT(avtLUT->GetSkewLookupTable());
    }
    else
    {
       glyphMapper->SetLUT(avtLUT->GetLookupTable());
    }
}

// ****************************************************************************
// Method: avtScatterPlot::SetLimitsMode
//
// Purpose:
//   Sets whether min/max limits are on.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 14:03:12 PST 2004
//
// Modifications:
//
// ****************************************************************************

void
avtScatterPlot::SetLimitsMode()
{
    // Get color information.
    std::string colorString;
    int mode = 0;
    double skew = 1.;
    bool minFlag = false, maxFlag = false;
    double minVal, maxVal;
    GetColorInformation(colorString, mode, skew, minFlag, minVal,
        maxFlag, maxVal);

    //
    //  Retrieve the actual range of the data
    //
    double min, max;
    glyphMapper->GetVarRange(min, max);
    double userMin = minFlag ? minVal : min;
    double userMax = maxFlag ? maxVal : max;

#define LM_ORIGINAL_DATA_LIMITS 0
#define LM_CURRENT_PLOT_LIMITS  1
#define LM_USER_LIMITS          2

    int limitsMode = LM_CURRENT_PLOT_LIMITS;
    if (dataExtents.size() == 2)
    {
        limitsMode = LM_USER_LIMITS;
        glyphMapper->SetMin(dataExtents[0]);
        glyphMapper->SetMax(dataExtents[1]);
        userMin = dataExtents[0];
        userMax = dataExtents[1];
    }
    else if (minFlag && maxFlag)
    {
        limitsMode = LM_USER_LIMITS;
        if (userMin >= userMax)
        {
            EXCEPTION1(InvalidLimitsException, false);
        }
        else
        {
            glyphMapper->SetMin(userMin);
            glyphMapper->SetMax(userMax);
        }
    }
    else if (minFlag)
    {
        limitsMode = LM_USER_LIMITS;
        glyphMapper->SetMin(userMin);
        if (userMin > userMax)
        {
            glyphMapper->SetMax(userMin);
        }
        else
        {
            glyphMapper->SetMaxOff();
        }
    }
    else if (maxFlag)
    {
        limitsMode = LM_USER_LIMITS;
        glyphMapper->SetMax(userMax);
        if (userMin > userMax)
        {
            glyphMapper->SetMin(userMax);
        }
        else
        {
            glyphMapper->SetMinOff();
        }
    }
    else
    {
        limitsMode = LM_USER_LIMITS;
        glyphMapper->SetMin(userMin);
        glyphMapper->SetMax(userMax);
    }
    glyphMapper->SetLimitsMode(limitsMode);

    // Set the range in the legend.
    varLegend->SetRange(userMin, userMax);

    //
    // Perform error checking if log scaling is to be used.
    //
    if (mode == 1 && ((minFlag && min <= 0) || (maxFlag && max <= 0.)))
    {
        EXCEPTION1(InvalidLimitsException, true);
    }
    varLegend->SetScaling(mode, skew);

    //
    // set and get the range for the legend's limits text
    //
    varLegend->SetVarRange(min, max);
}

// ****************************************************************************
// Method: avtScatterPlot::GetColorInformation
//
// Purpose:
//   Since the roles of each variable in this plot can be varied, this
//   routine figures out which variable is playing the color role and
//   returns information about it.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 13:57:39 PST 2004
//
// Modifications:
//    Cyrus Harrison, Thu Aug 19 13:12:00 PDT 2010
//    Get var1 from scatter atts.
//
// ****************************************************************************

void
avtScatterPlot::GetColorInformation(std::string &colorString,
    int &mode, double &skew, bool &minFlag, double &minVal,
    bool &maxFlag, double &maxVal)
{
    bool printIt = false;

    if(atts.GetVar1Role() == ScatterAttributes::Color)
    {
        colorString = atts.GetVar1();
        mode = atts.GetVar1Scaling();
        skew = atts.GetVar1SkewFactor();
        minFlag = atts.GetVar1MinFlag();
        minVal = atts.GetVar1Min();
        maxFlag = atts.GetVar1MaxFlag();
        maxVal = atts.GetVar1Max();

        printIt = true;
    }
    else if(atts.GetVar2Role() == ScatterAttributes::Color)
    {
        colorString = atts.GetVar2();
        mode = atts.GetVar2Scaling();
        skew = atts.GetVar2SkewFactor();
        minFlag = atts.GetVar2MinFlag();
        minVal = atts.GetVar2Min();
        maxFlag = atts.GetVar2MaxFlag();
        maxVal = atts.GetVar2Max();

        printIt = true;
    }
    else if(atts.GetVar3Role() == ScatterAttributes::Color)
    {
        colorString = atts.GetVar3();
        mode = atts.GetVar3Scaling();
        skew = atts.GetVar3SkewFactor();
        minFlag = atts.GetVar3MinFlag();
        minVal = atts.GetVar3Min();
        maxFlag = atts.GetVar3MaxFlag();
        maxVal = atts.GetVar3Max();

        printIt = true;
    }
    else if(atts.GetVar4Role() == ScatterAttributes::Color)
    {
        colorString = atts.GetVar4();
        mode = atts.GetVar4Scaling();
        skew = atts.GetVar4SkewFactor();
        minFlag = atts.GetVar4MinFlag();
        minVal = atts.GetVar4Min();
        maxFlag = atts.GetVar4MaxFlag();
        maxVal = atts.GetVar4Max();

        printIt = true;
    }

    if(printIt)
    {
        debug4 << "GetColorInformation: " << endl;
        debug4 << "\tcolorString = " << colorString.c_str() << endl;
        debug4 << "\tmode = " << mode << endl;
        debug4 << "\tskew = " << skew << endl;
        debug4 << "\tminFlag = " << (minFlag?"true":"false") << endl;
        debug4 << "\tminVal = " << minVal << endl;
        debug4 << "\tmaxFlag = " << (maxFlag?"true":"false") << endl;
        debug4 << "\tmaxVal = " << maxVal << endl;
    }
}

// ****************************************************************************
// Method: avtScatterPlot::SetAtts
//
// Purpose:
//   This method is called when we set the plot's attributes.
//
// Arguments:
//   a : The new set of plot attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 13:56:29 PST 2004
//
// Modifications:
//    Brad Whitlock, Wed Jul 20 13:26:13 PST 2005
//    I made the pointSize in the atts be used for to set the point size for
//    points, which is not the same as what's used for Box, Axis, Icosahedra.
//
//    Kathleen Bonnell, Mon Jan 17 18:13:11 MST 2011
//    Consider InvertColorTable flag when setting updateColors.
//
//    Brad Whitlock, Mon Jan  7 17:00:39 PST 2013
//    I added some new glyph types.
//
// ****************************************************************************

void
avtScatterPlot::SetAtts(const AttributeGroup *a)
{
    const ScatterAttributes *newAtts = (const ScatterAttributes *)a;

    // See if the colors will need to be updated.
    bool updateColors = (!colorsInitialized) ||
         (atts.GetColorTableName() != newAtts->GetColorTableName()) ||
         (atts.GetInvertColorTable() != newAtts->GetInvertColorTable());

    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const ScatterAttributes*)a);
    atts = *newAtts;

    // Set whether the legend is on or off.
    SetLegend(atts.GetLegendFlag());
    SetVarName(varname);

    // Update the plot's colors if needed.
    if(updateColors || atts.GetColorTableName() == "Default")
    {
        colorsInitialized = true;
        SetColorTable(atts.GetColorTableName().c_str());
    }

    glyphMapper->SetScale(atts.GetPointSize());
    glyphMapper->DataScalingOff();

    glyphMapper->SetGlyphType(atts.GetPointType());

    // Get color information.
    std::string colorString;
    int mode = 0;
    double skew = 1.;
    bool minFlag = false, maxFlag = false;
    double minVal, maxVal;
    GetColorInformation(colorString, mode, skew, minFlag, minVal,
        maxFlag, maxVal);

    if(atts.GetColorType() == ScatterAttributes::ColorByForegroundColor)
    {
        varLegend->SetColorBarVisibility(0);
        varLegend->SetVarRangeVisibility(0);
        glyphMapper->ColorBySingleColor(fgColor);
    }
    else if(atts.GetColorType() == ScatterAttributes::ColorBySingleColor)
    {
        varLegend->SetColorBarVisibility(0);
        varLegend->SetVarRangeVisibility(0);
        glyphMapper->ColorBySingleColor(atts.GetSingleColor().GetColor());
    }
    else //if(atts.GetColorType() == ScatterAttributes::ColorByColorTable)
    {
        glyphMapper->ColorByScalarOn(colorString);
        varLegend->SetColorBarVisibility(1);
        varLegend->SetVarRangeVisibility(1);
    }

    SetScaling(mode, skew);
    SetLimitsMode();
    SetPointGlyphSize();
}


// ****************************************************************************
//  Method: avtScatterPlot::SetVarName
//
//  Purpose:
//      Sets the variable name in the legend.
//
//  Arguments:
//      name      The name of the variable.
//
//  Programmer:   Brad Whitlock
//  Creation:     Tue Nov 2 22:10:12 PST 2004
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  1 16:07:40 PST 2005
//    Often, "varname" is passed in as the argument to this method, causing
//    reading of freed memory.  I had it no-op the dangerous section in this
//    case (which is the expected behavior).
//
//    Cyrus Harrison, Thu Aug 19 13:12:00 PDT 2010
//    Get var1 from scatter atts.
//
// ****************************************************************************

void
avtScatterPlot::SetVarName(const char *name)
{
    if (name != varname)
    {
        if (varname != NULL && name != NULL)
        {
            delete [] varname;
            varname = NULL;
        }
        if (name != NULL)
        {
            varname = new char[strlen(name)+1];
            strcpy(varname, name);
        }
    }

    std::string v;
    std::string var1name = atts.GetVar1();
    std::string default_var("default");
    const char *s[] = {0,0,0,0,0};
    s[int(atts.GetVar1Role())] = var1name.c_str();
    s[int(atts.GetVar2Role())] = (atts.GetVar2() == default_var) ? var1name.c_str() : atts.GetVar2().c_str();
    s[int(atts.GetVar3Role())] = (atts.GetVar3() == default_var) ? var1name.c_str() : atts.GetVar3().c_str();
    s[int(atts.GetVar4Role())] = (atts.GetVar4() == default_var) ? var1name.c_str() : atts.GetVar4().c_str();
    for(int i = 0, count = 0; i < 4; ++i)
    {
        if(s[i] != 0)
        {
            if(count++ > 0)
                v += "         ";
            v += std::string(s[i]);
            bool addComma = false;
            for(int j = i+1; j < 4; ++j)
            {
                if(s[j] != 0)
                {
                    addComma = true;
                    break;
                }
            }

            if(addComma)
                v += ",\n";
        }
    }

    varLegend->SetVarName(v.c_str());
}


// ****************************************************************************
//  Method: avtScatterPlot::SetLegend
//
//  Purpose:
//      Turns the legend on or off.
//
//  Arguments:
//      legendOn     true if the legend should be turned on, false otherwise.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
// ****************************************************************************

void
avtScatterPlot::SetLegend(bool legendOn)
{
    if (legendOn)
    {
        varLegend->LegendOn();
    }
    else
    {
        varLegend->LegendOff();
    }
}

// ****************************************************************************
// Method: avtScatterPlot::SetColorTable
//
// Purpose:
//   This method is called when the color table is changed.
//
// Arguments:
//   ctName : The name of the color table that was changed.
//
// Returns:    True if the specified color table was being used; False otherwise.
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 13:55:32 PST 2004
//
// Modifications:
//    Kathleen Bonnell, Mon Jan 17 18:13:11 MST 2011
//    Retrieve invertColorTable flag and pass to avtLUT.
//
// ****************************************************************************

bool
avtScatterPlot::SetColorTable(const char *ctName)
{
    bool namesMatch = (atts.GetColorTableName() == std::string(ctName));
    bool invert = atts.GetInvertColorTable();

    if (atts.GetColorTableName() == "Default")
        return avtLUT->SetColorTable(NULL, namesMatch, false, invert);
    else
        return avtLUT->SetColorTable(ctName, namesMatch, false, invert);
}

// ****************************************************************************
//  Method: avtScatterPlot::GetMapper
//
//  Purpose:
//      Gets the var mapper as its base class (avtMapper) for our base
//      class (avtPlot).
//
//  Returns:    The mapper for this plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
// ****************************************************************************

avtMapperBase *
avtScatterPlot::GetMapper(void)
{
    return glyphMapper;
}


// ****************************************************************************
//  Method: avtScatterPlot::ApplyOperators
//
//  Purpose:
//    Performs the implied operators for a scatter plot.
//
//  Arguments:
//    input     The input data object.
//
//  Returns:    The data object after the scatter plot applies (same data object).
//
//  Programmer: Brad Whitlock
//  Creation:   March  21, 2001
//
//  Modifications:
//    Cyrus Harrison, Thu Aug 19 13:12:00 PDT 2010
//    Support var1 in scatter atts.
//
// ****************************************************************************

avtDataObject_p
avtScatterPlot::ApplyOperators(avtDataObject_p input)
{
//    debug4 << "avtScatterPlot::ApplyOperators" << endl;
    avtDataObject_p dob = input;

    //
    // Turn the variables into an unstructured point mesh.
    //
    if (filter != NULL)
        delete filter;
    // if we haven't set the actual var name yet, do so
    if(atts.GetVar1() == "default")
        atts.SetVar1(varname);
    filter = new avtScatterFilter(atts);
    filter->SetInput(dob);
    dob = filter->GetOutput();

    return dob;
}


// ****************************************************************************
//  Method: avtScatterPlot::ApplyRenderingTransformation
//
//  Purpose:
//    Performs the rendering transformation for a scatter plot.
//
//  Arguments:
//    input     The input data object.
//
//  Returns:    The data object after the scatter plot applies (same data object).
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
//  Modifications:
//
// ****************************************************************************

avtDataObject_p
avtScatterPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
//    debug4 << "avtScatterPlot::ApplyRenderingTransformation" << endl;
    return input;
}


// ****************************************************************************
//  Method: avtScatterPlot::CustomizeBehavior
//
//  Purpose:
//      Customizes the behavior of the output.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
//  Modifications:
//    Brad Whitlock, Thu Jul 21 16:28:00 PST 2005
//    Added code to set the point glyph size.
//
// ****************************************************************************

void
avtScatterPlot::CustomizeBehavior(void)
{
    SetLimitsMode();
    SetPointGlyphSize();

    behavior->SetLegend(varLegendRefPtr);
    behavior->SetShiftFactor(0.5);
    behavior->SetRenderOrder(DOES_NOT_MATTER);
}


// ****************************************************************************
//  Method: avtScatterPlot::TargetTopologicalDimension
//
//  Purpose:
//      Determines what the target topological dimension should be.
//
//  Returns:    The target topological dimension for the scatter plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
//  Modifications:
//
// ****************************************************************************

int
avtScatterPlot::TargetTopologicalDimension(void)
{
    return 0;
}


// ****************************************************************************
//  Method: avtScatterPlot::SetForegroundColor
//
//  Purpose:
//    Sets the foreground color.
//
//  Returns:    True if using this color will require the plot to be redrawn.
//
//  Programmer: Brad Whitlock
//  Creation:   September 26, 2001
//
//  Modifications:
//
// ****************************************************************************

bool
avtScatterPlot::SetForegroundColor(const double *fg)
{
    bool retval = false;

    if(atts.GetColorType() == ScatterAttributes::ColorByForegroundColor)
    {
       if (fgColor[0] != fg[0] || fgColor[1] != fg[1] || fgColor[2] != fg[2])
       {
           retval = true;
       }
    }
    fgColor[0] = fg[0];
    fgColor[1] = fg[1];
    fgColor[2] = fg[2];

    if(retval)
       glyphMapper->ColorBySingleColor(fgColor);

    return retval;
}

// ****************************************************************************
// Method: avtScatterPlot::SetPointGlyphSize
//
// Purpose:
//   Sets the point glyph size into the mapper.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 21 15:24:25 PST 2005
//
// Modifications:
//   Brad Whitlock, Thu Aug 25 10:10:45 PDT 2005
//   Added sphere points.
//
// ****************************************************************************

void
avtScatterPlot::SetPointGlyphSize()
{
    // Size used for points when using a point glyph.
    if(atts.GetPointType() == Point)
        glyphMapper->SetPointSize(atts.GetPointSizePixels());
}

// ****************************************************************************
//  Method:  avtScatterPlot::Equivalent
//
//  Purpose:
//    Returns true if changes will not require recalculation.
//
//  Arguments:
//    a          the atts to compare
//
//  Programmer:  Brad Whitlock
//  Creation:    Tue Nov 2 22:10:12 PST 2004
//
//  Modification:
//
// ****************************************************************************

bool
avtScatterPlot::Equivalent(const AttributeGroup *a)
{
    return !(atts.ChangesRequireRecalculation(*(const ScatterAttributes*)a));
}


// ****************************************************************************
//  Method: avtScatterPlot::ReleaseData
//
//  Purpose:
//      Release the problem sized data associated with this plot.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 2 22:10:12 PST 2004
//
// ****************************************************************************

void
avtScatterPlot::ReleaseData(void)
{
    avtPlot::ReleaseData();

    if (filter != NULL)
        filter->ReleaseData();
}

// ****************************************************************************
// Method: avtScatterPlot::EnhanceSpecification
//
// Purpose:
//   Enhance the data specification so all of the required variables are
//   read from the database.
//
// Arguments:
//   spec : The input data specification.
//
// Returns:    A modified data specification.
//
// Note:       The filter has similar code in it to add the variables that
//             it wants to the specification but I found it to be unreliable.
//             Though this is the same code, having it here works. If the
//             scatter plot filter ever moves into the filters library then
//             the filter should really be the one enhancing the data
//             specification.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 13:52:11 PST 2004
//
// Modifications:
//   Cyrus Harrison,Thu Aug 19 08:58:26 PDT 2010
//   Expliclity request var1 if necessary.
//
//   Hank Childs, Sun Aug 29 20:05:11 CDT 2010
//   Calculate the extents of the color variable.
//
// ****************************************************************************

avtContract_p
avtScatterPlot::EnhanceSpecification(avtContract_p contract_in)
{
    avtContract_p rv = contract_in;
    avtDataRequest_p datareq_in = contract_in->GetDataRequest();

    std::string var1(atts.GetVar1());
    std::string var2(atts.GetVar2());
    std::string var3(atts.GetVar3());
    std::string var4(atts.GetVar4());
    bool addVar1 = false, addVar2 = false, addVar3 = false, addVar4 = false;

    // if var1 isn't set, read from contract (supports cli use)
    if(var1 == "default")
    {
        var1 = datareq_in->GetVariable();
        atts.SetVar1(var1);
    }

    // if var1 one isn't the primary var in our contract
    // we will need to create a new datarequest.
    if ( var1 != datareq_in->GetVariable())
    {
        addVar1 = true;
    }

    //
    // Find out if we REALLY need to add the secondary variables.
    //

    if (var2 != "default" &&
        var2 != var1 &&
        !datareq_in->HasSecondaryVariable(var2.c_str()))
    {
        addVar2 = true;
    }

    if (var3 != "default" &&
        var3 != var1 &&
        !datareq_in->HasSecondaryVariable(var3.c_str()))
    {
        addVar3 = true;
    }

    if (var4 != "default" &&
        var4 != var1 &&
        !datareq_in->HasSecondaryVariable(var4.c_str()))
    {
        addVar4 = true;
    }

    if(addVar1 || addVar2 || addVar3 || addVar4)
    {
        if(addVar1)
        {
            // to change the primary var we need to create a new data request
            avtDataRequest_p datareq_out = new avtDataRequest(contract_in->GetDataRequest(),
                                                              var1.c_str());
            rv = new avtContract(contract_in, datareq_out);
        }
        else
        {
            rv = new avtContract(contract_in);
        }

        if(addVar2)
            rv->GetDataRequest()->AddSecondaryVariable(var2.c_str());
        if(addVar3)
            rv->GetDataRequest()->AddSecondaryVariable(var3.c_str());
        if(addVar4)
            rv->GetDataRequest()->AddSecondaryVariable(var4.c_str());
    }

    if (atts.GetVar1Role() == ScatterAttributes::Color)
        rv->SetCalculateVariableExtents(var1, true);
    if (atts.GetVar2Role() == ScatterAttributes::Color)
        rv->SetCalculateVariableExtents(var2, true);
    if (atts.GetVar3Role() == ScatterAttributes::Color)
        rv->SetCalculateVariableExtents(var3, true);
    if (atts.GetVar4Role() == ScatterAttributes::Color)
        rv->SetCalculateVariableExtents(var4, true);

    return rv;
}


// ****************************************************************************
//  Method: avtScatterPlot::GetExtraInfoForPick
//
//  Purpose:
//    Overide default settings for extraInfoForPick.
//
//  Programmer: Kathleen Biagas
//  Creation:   February 29, 2012
//
// ****************************************************************************

const MapNode &
avtScatterPlot::GetExtraInfoForPick()
{
    stringVector addVars;
    std::string xvar, yvar, zvar;

    ScatterAttributes::VariableRole varRole = atts.GetVar1Role();
    std::string var1Name(atts.GetVar1());
    if (varRole != ScatterAttributes::None)
    {
        addVars.push_back(var1Name);
        if (varRole == ScatterAttributes::Coordinate0)
            xvar = var1Name;
        else if (varRole == ScatterAttributes::Coordinate1)
            yvar = var1Name;
        else if (varRole == ScatterAttributes::Coordinate2)
            zvar = var1Name;
    }

    varRole = atts.GetVar2Role();
    if (varRole != ScatterAttributes::None)
    {
        std::string var2Name(atts.GetVar2());
        if (var2Name == "default")
            var2Name = var1Name;
        else
            addVars.push_back(var2Name);

        if (varRole == ScatterAttributes::Coordinate0)
            xvar = var2Name;
        else if (varRole == ScatterAttributes::Coordinate1)
            yvar = var2Name;
        else if (varRole == ScatterAttributes::Coordinate2)
            zvar = var2Name;
    }

    varRole = atts.GetVar3Role();
    if (varRole != ScatterAttributes::None)
    {
        std::string var3Name(atts.GetVar3());
        if (var3Name == "default")
            var3Name = var1Name;
        else
            addVars.push_back(var3Name);

        if (varRole == ScatterAttributes::Coordinate0)
            xvar = var3Name;
        else if (varRole == ScatterAttributes::Coordinate1)
            yvar = var3Name;
        else if (varRole == ScatterAttributes::Coordinate2)
            zvar = var3Name;
    }

    varRole = atts.GetVar4Role();
    if (varRole != ScatterAttributes::None)
    {
        std::string var4Name(atts.GetVar4());
        if (var4Name == "default")
            var4Name = var1Name;
        else
            addVars.push_back(var4Name);

        if (varRole == ScatterAttributes::Coordinate0)
            xvar = var4Name;
        else if (varRole == ScatterAttributes::Coordinate1)
            yvar = var4Name;
        else if (varRole == ScatterAttributes::Coordinate2)
            zvar = var4Name;
    }

    std::string pointString("<" + xvar + ", " + yvar);
    if (!zvar.empty())
        pointString.append(", " + zvar);
    pointString.append(">");

    // control how pick is performed
    if (!addVars.empty())
        extraPickInfo["additionalVars"] = addVars;
    extraPickInfo["glyphPickAlways"] = true;

    // control how pick output is diplayed
    extraPickInfo["showMeshName"] = false;
    extraPickInfo["showIncidentElements"] = false;
    extraPickInfo["pointString"] = pointString;

    return extraPickInfo;
}


// ****************************************************************************
//  Method: avtScatterPlot::PlotHasBeenGlyphed
//
//  Purpose:
//    Returns whether or not this plot has had point glyphs applied.
//
//  Programmer: Kathleen Biagas
//  Creation:   October 31, 2019 
//
// ****************************************************************************

bool
avtScatterPlot::PlotHasBeenGlyphed()
{
    return (atts.GetPointType() != Point && atts.GetPointType() != Sphere);
}

