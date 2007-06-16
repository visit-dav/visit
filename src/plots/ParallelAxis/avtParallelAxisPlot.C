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
//                              avtParallelAxisPlot.C                        //
// ************************************************************************* //

#include <avtParallelAxisPlot.h>
#include <avtParallelAxisFilter.h>


#include <ColorAttribute.h>

#include <avtColorTables.h>
#include <avtLevelsMapper.h>
#include <avtLookupTable.h>

#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtParallelAxisPlot
//
//  Purpose: Constructor for the avtParallelAxisPlot class.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Mar 19 11:30:16 EDT 2007
//    Added background color for fading with context.
//   
// ****************************************************************************

avtParallelAxisPlot::avtParallelAxisPlot()
{
    levelsMapper  = new avtLevelsMapper;
    avtLUT        = new avtLookupTable;
    parAxisFilter = NULL;
    bgColor[0] = bgColor[1] = bgColor[2] = 1.0;  // white
}


// ****************************************************************************
//  Method: ~avtParallelAxisPlot
//
//  Purpose: Destructor for the avtParallelAxisPlot class.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

avtParallelAxisPlot::~avtParallelAxisPlot()
{
    if (levelsMapper != NULL)
    {
        delete levelsMapper;
        levelsMapper = NULL;
    }

    if (parAxisFilter != NULL)
    {
        delete parAxisFilter;
        parAxisFilter = NULL;
    }

    if (avtLUT != NULL)
    {
        delete avtLUT;
        avtLUT = NULL;
    }
}


// ****************************************************************************
//  Method:  avtParallelAxisPlot::Create
//
//  Purpose: Calls the constructor.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

avtPlot*
avtParallelAxisPlot::Create()
{
    return new avtParallelAxisPlot;
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::SetAtts
//
//  Purpose: Sets attributes in the plot to specified values.
//
//  Arguments:
//      atts    The attribute values for this ParallelAxis plot.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelAxisPlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const ParallelAxisAttributes*)a);
    atts = *(const ParallelAxisAttributes*)a;
    
    SetColors();

    behavior->SetRenderOrder(DOES_NOT_MATTER);
    behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::SetColors
//
//  Purpose: Sets RGB components of colors used in the plot.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
//    Jeremy Meredith, Fri Mar 16 17:47:02 EDT 2007
//    Added colors for the "Context" portion of the plot.  Alas, these
//    must come first so the Context is drawn behind the other data curve
//    lines and annotations.
//
//    Jeremy Meredith, Mon Mar 19 11:28:57 EDT 2007
//    Fade context colors nicely into background color at low end.
//
// ****************************************************************************

void
avtParallelAxisPlot::SetColors()
{
    int redID, red, green, blue;
    int numColorEntries = 4 * (4+PCP_CTX_BRIGHTNESS_LEVELS);
    unsigned char *plotColors = new unsigned char[numColorEntries];

    ColorAttribute colorAtt;
    ColorAttributeList colorAttList;

    for (redID = 0; redID < numColorEntries; redID += 4)
    {
        switch (redID)
        {
          case PCP_CTX_BRIGHTNESS_LEVELS*4 + 0:
            red   = atts.GetLinesColor().Red();
            green = atts.GetLinesColor().Green();
            blue  = atts.GetLinesColor().Blue();
            break;
          case PCP_CTX_BRIGHTNESS_LEVELS*4 + 4:
            red   = (PCP_DEFAULT_AXIS_COLOR >> 24) & 0xff;
            green = (PCP_DEFAULT_AXIS_COLOR >> 16) & 0xff;
            blue  = (PCP_DEFAULT_AXIS_COLOR >>  8) & 0xff;
            break;
          case PCP_CTX_BRIGHTNESS_LEVELS*4 + 8:
            red   = (PCP_DEFAULT_AXIS_TITLE_COLOR >> 24) & 0xff;
            green = (PCP_DEFAULT_AXIS_TITLE_COLOR >> 16) & 0xff;
            blue  = (PCP_DEFAULT_AXIS_TITLE_COLOR >>  8) & 0xff;
            break;
          case PCP_CTX_BRIGHTNESS_LEVELS*4 +12:
            red   = (PCP_DEFAULT_RANGE_BOUND_COLOR >> 24) & 0xff;
            green = (PCP_DEFAULT_RANGE_BOUND_COLOR >> 16) & 0xff;
            blue  = (PCP_DEFAULT_RANGE_BOUND_COLOR >>  8) & 0xff;
            break;
          default:
            {
            float scale = ((redID)/4.)/float(PCP_CTX_BRIGHTNESS_LEVELS);
            int bgred   = int(bgColor[0]*255);
            int bggreen = int(bgColor[1]*255);
            int bgblue  = int(bgColor[2]*255);
            int hired   = atts.GetContextColor().Red();
            int higreen = atts.GetContextColor().Green();
            int hiblue  = atts.GetContextColor().Blue();            
            red   = int(scale*hired   + (1.-scale)*bgred);
            green = int(scale*higreen + (1.-scale)*bggreen);
            blue  = int(scale*hiblue  + (1.-scale)*bgblue);
            }
            break;
        }

        colorAtt.SetRgba(red, green, blue, 255);
        colorAttList.AddColors(colorAtt);

        plotColors[redID  ] = (unsigned char)red;
        plotColors[redID+1] = (unsigned char)green;
        plotColors[redID+2] = (unsigned char)blue;
        plotColors[redID+3] = 255;
    }

    avtLUT->SetLUTColorsWithOpacity(plotColors, 4+PCP_CTX_BRIGHTNESS_LEVELS);
    levelsMapper->SetColors(colorAttList);

    delete [] plotColors;
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::GetMapper
//
//  Purpose: Gets the levels mapper as its base class (avtMapper) for the
//           plot's class (avtPlot).
//
//  Returns: The mapper for this plot.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

avtMapper *
avtParallelAxisPlot::GetMapper(void)
{
    return levelsMapper;
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::ApplyOperators
//
//  Purpose: Applies the implied operators for a ParallelAxis plot, namely,
//           an avtParallelAxisFilter.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the ParallelAxis filter is applied.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

avtDataObject_p
avtParallelAxisPlot::ApplyOperators(avtDataObject_p input)
{
    if (parAxisFilter != NULL)
    {
        delete parAxisFilter;
        parAxisFilter = NULL;
    }

    parAxisFilter = new avtParallelAxisFilter(atts);

    parAxisFilter->SetInput(input);

    return parAxisFilter->GetOutput();
}

// ****************************************************************************
//  Method: avtParallelAxisPlot::ApplyRenderingTransformation
//
//  Purpose: Performs the rendering transformation for a ParallelAxis plot,
//           namely, no transformations at all.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The input data object.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

avtDataObject_p
avtParallelAxisPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::CustomizeBehavior
//
//  Purpose: Customizes the behavior of the output.  
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelAxisPlot::CustomizeBehavior(void)
{
    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_CURVE);

    behavior->SetShiftFactor(0.0);
    behavior->SetLegend(NULL);
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::CustomizeMapper
//
//  Purpose: (Currently just a place holder).
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelAxisPlot::CustomizeMapper(avtDataObjectInformation &info)
{
//  May need to do something here in the future.
    return;
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::EnhanceSpecification
//
//  Purpose: Make sure that all axis variables are specified as secondary
//           variables except those which are components of the pipeline
//           variable.
//
//  Programmer: Mark Blair
//  Creation:   Fri Apr 14 18:49:00 PDT 2006
//
//  Modifications:
//
//      Mark Blair, Wed Aug 16 16:46:00 PDT 2006
//      Added check for attribute consistency.
//
// ****************************************************************************

avtPipelineSpecification_p
avtParallelAxisPlot::EnhanceSpecification(avtPipelineSpecification_p in_spec)
{
    if (!atts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAP/ES/1: ParallelAxis plot attributes are inconsistent."
               << endl;
        return in_spec;
    }

    stringVector curAxisVarNames = atts.GetOrderedAxisNames();
    stringVector needSecondaryVars;
    const char *inPipelineVar = in_spec->GetDataSpecification()->GetVariable();
    std::string outPipelineVar(inPipelineVar);
    std::string axisVarName;
    int axisNum;

    avtPipelineSpecification_p outSpec;

    for (axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
    {
        if (curAxisVarNames[axisNum] == outPipelineVar) break;
    }
    
    if (axisNum < curAxisVarNames.size())
    {
        outSpec = new avtPipelineSpecification(in_spec);
    }
    else
    {
        outPipelineVar = curAxisVarNames[0];
        
        avtDataSpecification_p newDataSpec = new avtDataSpecification(
            in_spec->GetDataSpecification(), outPipelineVar.c_str());
        outSpec = new avtPipelineSpecification(in_spec, newDataSpec);
    }

    for (axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
    {
        if ((axisVarName = curAxisVarNames[axisNum]) != outPipelineVar)
        {
            needSecondaryVars.push_back(axisVarName);
        }
    }
        
    const std::vector<CharStrRef> curSecondaryVars =
        in_spec->GetDataSpecification()->GetSecondaryVariables();
    int needSecVNum, curSecVNum;
    const char *needSecondaryVar;
    const char *curSecondaryVar;

    for (needSecVNum = 0; needSecVNum < needSecondaryVars.size(); needSecVNum++)
    {
        needSecondaryVar = needSecondaryVars[needSecVNum].c_str();

        for (curSecVNum = 0; curSecVNum < curSecondaryVars.size(); curSecVNum++)
        {
            if (strcmp(*curSecondaryVars[curSecVNum], needSecondaryVar) == 0)
            {
                break;
            }
        }

        if (curSecVNum >= curSecondaryVars.size())
        {
          outSpec->GetDataSpecification()->AddSecondaryVariable(needSecondaryVar);
        }
    }

    for (curSecVNum = 0; curSecVNum < curSecondaryVars.size(); curSecVNum++ ) {
        curSecondaryVar = *curSecondaryVars[curSecVNum];

        for (needSecVNum = 0; needSecVNum < needSecondaryVars.size(); needSecVNum++)
        {
            if (strcmp(needSecondaryVars[needSecVNum].c_str(),curSecondaryVar) == 0)
            {
                break;
            }
        }

        if (needSecVNum >= needSecondaryVars.size())
        {
          outSpec->GetDataSpecification()->RemoveSecondaryVariable(curSecondaryVar);
        }
    }

    return outSpec;
}

// ****************************************************************************
//  Method: avtParallelAxisPlot::ReleaseData
//
//  Purpose: Release the problem-sized data associated with this plot.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelAxisPlot::ReleaseData(void)
{
    avtSurfaceDataPlot::ReleaseData();

    if (parAxisFilter != NULL) parAxisFilter->ReleaseData();
}


// ****************************************************************************
//  Method: avtParallelAxisPlot::SetBackgroundColor
//
//  Purpose:
//    Sets the background color.
//
//  Returns:    True if using this color will require the plot to be redrawn.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2007
//
//  Modifications:
//
// ****************************************************************************

bool
avtParallelAxisPlot::SetBackgroundColor(const double *bg)
{
    if (bgColor[0] == bg[0] && bgColor[1] == bg[1] && bgColor[2] == bg[2])
    {
        return false;
    }

    bgColor[0] = bg[0];
    bgColor[1] = bg[1];
    bgColor[2] = bg[2];
    SetColors();

    return true;
}
