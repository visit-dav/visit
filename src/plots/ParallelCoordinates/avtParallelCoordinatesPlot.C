/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
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
//                              avtParallelCoordinatesPlot.C                        //
// ************************************************************************* //

#include <avtParallelCoordinatesPlot.h>
#include <avtParallelCoordinatesFilter.h>


#include <ColorAttribute.h>

#include <avtColorTables.h>
#include <avtLevelsMapper.h>
#include <avtLookupTable.h>

#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot
//
//  Purpose: Constructor for the avtParallelCoordinatesPlot class.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

avtParallelCoordinatesPlot::avtParallelCoordinatesPlot()
{
    levelsMapper  = new avtLevelsMapper;
    avtLUT        = new avtLookupTable;
    parAxisFilter = NULL;
    bgColor[0] = bgColor[1] = bgColor[2] = 1.0;  // white
}


// ****************************************************************************
//  Method: ~avtParallelCoordinatesPlot
//
//  Purpose: Destructor for the avtParallelCoordinatesPlot class.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

avtParallelCoordinatesPlot::~avtParallelCoordinatesPlot()
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
//  Method:  avtParallelCoordinatesPlot::Create
//
//  Purpose: Calls the constructor.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

avtPlot*
avtParallelCoordinatesPlot::Create()
{
    return new avtParallelCoordinatesPlot;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::SetAtts
//
//  Purpose: Sets attributes in the plot to specified values.
//
//  Arguments:
//      atts    The attribute values for this ParallelCoordinates plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelCoordinatesPlot::SetAtts(const AttributeGroup *a)
{
    needsRecalculation =
        atts.ChangesRequireRecalculation(*(const ParallelCoordinatesAttributes*)a);
    atts = *(const ParallelCoordinatesAttributes*)a;
    
    SetColors();

    behavior->SetRenderOrder(DOES_NOT_MATTER);
    behavior->SetAntialiasedRenderOrder(DOES_NOT_MATTER);
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::SetColors
//
//  Purpose: Sets RGB components of colors used in the plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelCoordinatesPlot::SetColors()
{
    int redID, red, green, blue;
    int numColorEntries = 4 * (1+PCP_CTX_BRIGHTNESS_LEVELS);
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

    avtLUT->SetLUTColorsWithOpacity(plotColors, 1+PCP_CTX_BRIGHTNESS_LEVELS);
    levelsMapper->SetColors(colorAttList);

    delete [] plotColors;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::GetMapper
//
//  Purpose: Gets the levels mapper as its base class (avtMapper) for the
//           plot's class (avtPlot).
//
//  Returns: The mapper for this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

avtMapper *
avtParallelCoordinatesPlot::GetMapper(void)
{
    return levelsMapper;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::ApplyOperators
//
//  Purpose: Applies the implied operators for a ParallelCoordinates plot, namely,
//           an avtParallelCoordinatesFilter.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The data object after the ParallelCoordinates filter is applied.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

avtDataObject_p
avtParallelCoordinatesPlot::ApplyOperators(avtDataObject_p input)
{
    if (parAxisFilter != NULL)
    {
        delete parAxisFilter;
        parAxisFilter = NULL;
    }

    parAxisFilter = new avtParallelCoordinatesFilter(atts);

    parAxisFilter->SetInput(input);

    return parAxisFilter->GetOutput();
}

// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::ApplyRenderingTransformation
//
//  Purpose: Performs the rendering transformation for a ParallelCoordinates plot,
//           namely, no transformations at all.
//
//  Arguments:
//      input   The input data object.
//
//  Returns:    The input data object.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

avtDataObject_p
avtParallelCoordinatesPlot::ApplyRenderingTransformation(avtDataObject_p input)
{
    return input;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::CustomizeBehavior
//
//  Purpose: Customizes the behavior of the output.  
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelCoordinatesPlot::CustomizeBehavior(void)
{
    behavior->GetInfo().GetAttributes().SetWindowMode(WINMODE_AXISARRAY);

    behavior->SetShiftFactor(0.0);
    behavior->SetLegend(NULL);
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::CustomizeMapper
//
//  Purpose: (Currently just a place holder).
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelCoordinatesPlot::CustomizeMapper(avtDataObjectInformation &info)
{
//  May need to do something here in the future.
    return;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::EnhanceSpecification
//
//  Purpose: Make sure that all axis variables are specified as secondary
//           variables except those which are components of the pipeline
//           variable.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 17:51:12 EST 2008
//    Exit early if we had an array variable.
//
//    Jeremy Meredith, Fri Feb 15 13:16:46 EST 2008
//    Renamed orderedAxisNames to scalarAxisNames to distinguish these
//    as names of actual scalars instead of just display names.
//
// ****************************************************************************

avtContract_p
avtParallelCoordinatesPlot::EnhanceSpecification(avtContract_p in_spec)
{
    if (atts.GetScalarAxisNames().size() == 0)
    {
        // nothing to do; this means we have an array variable
        return in_spec;
    }

    if (!atts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAP/ES/1: ParallelCoordinates plot attributes are "
               << "inconsistent." << endl;
        return in_spec;
    }

    stringVector curAxisVarNames = atts.GetScalarAxisNames();
    stringVector needSecondaryVars;
    const char *inPipelineVar = in_spec->GetDataRequest()->GetVariable();
    std::string outPipelineVar(inPipelineVar);
    std::string axisVarName;
    int axisNum;

    avtContract_p outSpec;

    for (axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
    {
        if (curAxisVarNames[axisNum] == outPipelineVar) break;
    }
    
    if (axisNum < curAxisVarNames.size())
    {
        outSpec = new avtContract(in_spec);
    }
    else
    {
        outPipelineVar = curAxisVarNames[0];
        
        avtDataRequest_p newDataSpec = new avtDataRequest(
            in_spec->GetDataRequest(), outPipelineVar.c_str());
        outSpec = new avtContract(in_spec, newDataSpec);
    }

    for (axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
    {
        if ((axisVarName = curAxisVarNames[axisNum]) != outPipelineVar)
        {
            needSecondaryVars.push_back(axisVarName);
        }
    }
        
    const std::vector<CharStrRef> curSecondaryVars =
        in_spec->GetDataRequest()->GetSecondaryVariables();
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
          outSpec->GetDataRequest()->AddSecondaryVariable(needSecondaryVar);
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
          outSpec->GetDataRequest()->RemoveSecondaryVariable(curSecondaryVar);
        }
    }

    return outSpec;
}

// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::ReleaseData
//
//  Purpose: Release the problem-sized data associated with this plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//   
// ****************************************************************************

void
avtParallelCoordinatesPlot::ReleaseData(void)
{
    avtSurfaceDataPlot::ReleaseData();

    if (parAxisFilter != NULL) parAxisFilter->ReleaseData();
}


// ****************************************************************************
//  Method: avtParallelCoordinatesPlot::SetBackgroundColor
//
//  Purpose:
//    Sets the background color.
//
//  Returns:    True if using this color will require the plot to be redrawn.
//
//  Programmer: Jeremy Meredith
//  Creation:  January 31, 2008
//
//  Note: original implementation from Mark Blair's parallel axis plot
//
//  Modifications:
//
// ****************************************************************************

bool
avtParallelCoordinatesPlot::SetBackgroundColor(const double *bg)
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
