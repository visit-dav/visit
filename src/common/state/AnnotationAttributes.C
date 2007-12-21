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

#include <AnnotationAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for AnnotationAttributes::Ticks
//

static const char *Ticks_strings[] = {
"Off", "Bottom", "Left", 
"BottomLeft", "All"};

std::string
AnnotationAttributes::Ticks_ToString(AnnotationAttributes::Ticks t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return Ticks_strings[index];
}

std::string
AnnotationAttributes::Ticks_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return Ticks_strings[index];
}

bool
AnnotationAttributes::Ticks_FromString(const std::string &s, AnnotationAttributes::Ticks &val)
{
    val = AnnotationAttributes::Off;
    for(int i = 0; i < 5; ++i)
    {
        if(s == Ticks_strings[i])
        {
            val = (Ticks)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for AnnotationAttributes::Location
//

static const char *Location_strings[] = {
"Inside", "Outside", "Both"
};

std::string
AnnotationAttributes::Location_ToString(AnnotationAttributes::Location t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Location_strings[index];
}

std::string
AnnotationAttributes::Location_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Location_strings[index];
}

bool
AnnotationAttributes::Location_FromString(const std::string &s, AnnotationAttributes::Location &val)
{
    val = AnnotationAttributes::Inside;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Location_strings[i])
        {
            val = (Location)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for AnnotationAttributes::Axes
//

static const char *Axes_strings[] = {
"ClosestTriad", "FurthestTriad", "OutsideEdges", 
"StaticTriad", "StaticEdges"};

std::string
AnnotationAttributes::Axes_ToString(AnnotationAttributes::Axes t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return Axes_strings[index];
}

std::string
AnnotationAttributes::Axes_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return Axes_strings[index];
}

bool
AnnotationAttributes::Axes_FromString(const std::string &s, AnnotationAttributes::Axes &val)
{
    val = AnnotationAttributes::ClosestTriad;
    for(int i = 0; i < 5; ++i)
    {
        if(s == Axes_strings[i])
        {
            val = (Axes)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for AnnotationAttributes::GradientStyle
//

static const char *GradientStyle_strings[] = {
"TopToBottom", "BottomToTop", "LeftToRight", 
"RightToLeft", "Radial"};

std::string
AnnotationAttributes::GradientStyle_ToString(AnnotationAttributes::GradientStyle t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return GradientStyle_strings[index];
}

std::string
AnnotationAttributes::GradientStyle_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return GradientStyle_strings[index];
}

bool
AnnotationAttributes::GradientStyle_FromString(const std::string &s, AnnotationAttributes::GradientStyle &val)
{
    val = AnnotationAttributes::TopToBottom;
    for(int i = 0; i < 5; ++i)
    {
        if(s == GradientStyle_strings[i])
        {
            val = (GradientStyle)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for AnnotationAttributes::BackgroundMode
//

static const char *BackgroundMode_strings[] = {
"Solid", "Gradient", "Image", 
"ImageSphere"};

std::string
AnnotationAttributes::BackgroundMode_ToString(AnnotationAttributes::BackgroundMode t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return BackgroundMode_strings[index];
}

std::string
AnnotationAttributes::BackgroundMode_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return BackgroundMode_strings[index];
}

bool
AnnotationAttributes::BackgroundMode_FromString(const std::string &s, AnnotationAttributes::BackgroundMode &val)
{
    val = AnnotationAttributes::Solid;
    for(int i = 0; i < 4; ++i)
    {
        if(s == BackgroundMode_strings[i])
        {
            val = (BackgroundMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for AnnotationAttributes::PathExpansionMode
//

static const char *PathExpansionMode_strings[] = {
"File", "Directory", "Full", 
"Smart", "SmartDirectory"};

std::string
AnnotationAttributes::PathExpansionMode_ToString(AnnotationAttributes::PathExpansionMode t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return PathExpansionMode_strings[index];
}

std::string
AnnotationAttributes::PathExpansionMode_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return PathExpansionMode_strings[index];
}

bool
AnnotationAttributes::PathExpansionMode_FromString(const std::string &s, AnnotationAttributes::PathExpansionMode &val)
{
    val = AnnotationAttributes::File;
    for(int i = 0; i < 5; ++i)
    {
        if(s == PathExpansionMode_strings[i])
        {
            val = (PathExpansionMode)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *AnnotationAttributes::TypeMapFormatString = "bbbbbbbbbddddddddddddiiiiissbbssbbbbbbbbbbbbbbbbbddddddddddddddddddiiisssbbbsssbbbiibbaaiaaisbbibii";

// ****************************************************************************
// Method: AnnotationAttributes::AnnotationAttributes
//
// Purpose: 
//   Constructor for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AnnotationAttributes::AnnotationAttributes() : 
    AttributeSubject(AnnotationAttributes::TypeMapFormatString),
    backgroundColor(255, 255, 255), foregroundColor(0, 0, 0), 
    gradientColor1(0, 0, 255), gradientColor2(0, 0, 0)
{
    axesFlag2D = true;
    axesAutoSetTicks2D = true;
    labelAutoSetScaling2D = true;
    xAxisLabels2D = true;
    yAxisLabels2D = true;
    xAxisTitle2D = true;
    yAxisTitle2D = true;
    xGridLines2D = false;
    yGridLines2D = false;
    xMajorTickMinimum2D = 0;
    yMajorTickMinimum2D = 0;
    xMajorTickMaximum2D = 1;
    yMajorTickMaximum2D = 1;
    xMajorTickSpacing2D = 0.2;
    yMajorTickSpacing2D = 0.2;
    xMinorTickSpacing2D = 0.02;
    yMinorTickSpacing2D = 0.02;
    xLabelFontHeight2D = 0.02;
    yLabelFontHeight2D = 0.02;
    xTitleFontHeight2D = 0.02;
    yTitleFontHeight2D = 0.02;
    xLabelScaling2D = 0;
    yLabelScaling2D = 0;
    axesLineWidth2D = 0;
    axesTickLocation2D = Outside;
    axesTicks2D = BottomLeft;
    xAxisUserTitle2D = "X-Axis";
    yAxisUserTitle2D = "Y-Axis";
    xAxisUserTitleFlag2D = false;
    yAxisUserTitleFlag2D = false;
    xAxisUserUnitsFlag2D = false;
    yAxisUserUnitsFlag2D = false;
    axesFlag = true;
    axesAutoSetTicks = true;
    labelAutoSetScaling = true;
    xAxisLabels = true;
    yAxisLabels = true;
    zAxisLabels = true;
    xAxisTitle = true;
    yAxisTitle = true;
    zAxisTitle = true;
    xGridLines = false;
    yGridLines = false;
    zGridLines = false;
    xAxisTicks = true;
    yAxisTicks = true;
    zAxisTicks = true;
    xMajorTickMinimum = 0;
    yMajorTickMinimum = 0;
    zMajorTickMinimum = 0;
    xMajorTickMaximum = 1;
    yMajorTickMaximum = 1;
    zMajorTickMaximum = 1;
    xMajorTickSpacing = 0.2;
    yMajorTickSpacing = 0.2;
    zMajorTickSpacing = 0.2;
    xMinorTickSpacing = 0.02;
    yMinorTickSpacing = 0.02;
    zMinorTickSpacing = 0.02;
    xLabelFontHeight = 0.02;
    yLabelFontHeight = 0.02;
    zLabelFontHeight = 0.02;
    xTitleFontHeight = 0.02;
    yTitleFontHeight = 0.02;
    zTitleFontHeight = 0.02;
    xLabelScaling = 0;
    yLabelScaling = 0;
    zLabelScaling = 0;
    xAxisUserTitle = "X-Axis";
    yAxisUserTitle = "Y-Axis";
    zAxisUserTitle = "Z-Axis";
    xAxisUserTitleFlag = false;
    yAxisUserTitleFlag = false;
    zAxisUserTitleFlag = false;
    xAxisUserUnitsFlag = false;
    yAxisUserUnitsFlag = false;
    zAxisUserUnitsFlag = false;
    axesTickLocation = Inside;
    axesType = ClosestTriad;
    triadFlag = true;
    bboxFlag = true;
    gradientBackgroundStyle = Radial;
    backgroundMode = Solid;
    userInfoFlag = true;
    databaseInfoFlag = true;
    databaseInfoExpansionMode = File;
    legendInfoFlag = true;
    imageRepeatX = 1;
    imageRepeatY = 1;
}

// ****************************************************************************
// Method: AnnotationAttributes::AnnotationAttributes
//
// Purpose: 
//   Copy constructor for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AnnotationAttributes::AnnotationAttributes(const AnnotationAttributes &obj) : 
    AttributeSubject(AnnotationAttributes::TypeMapFormatString)
{
    axesFlag2D = obj.axesFlag2D;
    axesAutoSetTicks2D = obj.axesAutoSetTicks2D;
    labelAutoSetScaling2D = obj.labelAutoSetScaling2D;
    xAxisLabels2D = obj.xAxisLabels2D;
    yAxisLabels2D = obj.yAxisLabels2D;
    xAxisTitle2D = obj.xAxisTitle2D;
    yAxisTitle2D = obj.yAxisTitle2D;
    xGridLines2D = obj.xGridLines2D;
    yGridLines2D = obj.yGridLines2D;
    xMajorTickMinimum2D = obj.xMajorTickMinimum2D;
    yMajorTickMinimum2D = obj.yMajorTickMinimum2D;
    xMajorTickMaximum2D = obj.xMajorTickMaximum2D;
    yMajorTickMaximum2D = obj.yMajorTickMaximum2D;
    xMajorTickSpacing2D = obj.xMajorTickSpacing2D;
    yMajorTickSpacing2D = obj.yMajorTickSpacing2D;
    xMinorTickSpacing2D = obj.xMinorTickSpacing2D;
    yMinorTickSpacing2D = obj.yMinorTickSpacing2D;
    xLabelFontHeight2D = obj.xLabelFontHeight2D;
    yLabelFontHeight2D = obj.yLabelFontHeight2D;
    xTitleFontHeight2D = obj.xTitleFontHeight2D;
    yTitleFontHeight2D = obj.yTitleFontHeight2D;
    xLabelScaling2D = obj.xLabelScaling2D;
    yLabelScaling2D = obj.yLabelScaling2D;
    axesLineWidth2D = obj.axesLineWidth2D;
    axesTickLocation2D = obj.axesTickLocation2D;
    axesTicks2D = obj.axesTicks2D;
    xAxisUserTitle2D = obj.xAxisUserTitle2D;
    yAxisUserTitle2D = obj.yAxisUserTitle2D;
    xAxisUserTitleFlag2D = obj.xAxisUserTitleFlag2D;
    yAxisUserTitleFlag2D = obj.yAxisUserTitleFlag2D;
    xAxisUserUnits2D = obj.xAxisUserUnits2D;
    yAxisUserUnits2D = obj.yAxisUserUnits2D;
    xAxisUserUnitsFlag2D = obj.xAxisUserUnitsFlag2D;
    yAxisUserUnitsFlag2D = obj.yAxisUserUnitsFlag2D;
    axesFlag = obj.axesFlag;
    axesAutoSetTicks = obj.axesAutoSetTicks;
    labelAutoSetScaling = obj.labelAutoSetScaling;
    xAxisLabels = obj.xAxisLabels;
    yAxisLabels = obj.yAxisLabels;
    zAxisLabels = obj.zAxisLabels;
    xAxisTitle = obj.xAxisTitle;
    yAxisTitle = obj.yAxisTitle;
    zAxisTitle = obj.zAxisTitle;
    xGridLines = obj.xGridLines;
    yGridLines = obj.yGridLines;
    zGridLines = obj.zGridLines;
    xAxisTicks = obj.xAxisTicks;
    yAxisTicks = obj.yAxisTicks;
    zAxisTicks = obj.zAxisTicks;
    xMajorTickMinimum = obj.xMajorTickMinimum;
    yMajorTickMinimum = obj.yMajorTickMinimum;
    zMajorTickMinimum = obj.zMajorTickMinimum;
    xMajorTickMaximum = obj.xMajorTickMaximum;
    yMajorTickMaximum = obj.yMajorTickMaximum;
    zMajorTickMaximum = obj.zMajorTickMaximum;
    xMajorTickSpacing = obj.xMajorTickSpacing;
    yMajorTickSpacing = obj.yMajorTickSpacing;
    zMajorTickSpacing = obj.zMajorTickSpacing;
    xMinorTickSpacing = obj.xMinorTickSpacing;
    yMinorTickSpacing = obj.yMinorTickSpacing;
    zMinorTickSpacing = obj.zMinorTickSpacing;
    xLabelFontHeight = obj.xLabelFontHeight;
    yLabelFontHeight = obj.yLabelFontHeight;
    zLabelFontHeight = obj.zLabelFontHeight;
    xTitleFontHeight = obj.xTitleFontHeight;
    yTitleFontHeight = obj.yTitleFontHeight;
    zTitleFontHeight = obj.zTitleFontHeight;
    xLabelScaling = obj.xLabelScaling;
    yLabelScaling = obj.yLabelScaling;
    zLabelScaling = obj.zLabelScaling;
    xAxisUserTitle = obj.xAxisUserTitle;
    yAxisUserTitle = obj.yAxisUserTitle;
    zAxisUserTitle = obj.zAxisUserTitle;
    xAxisUserTitleFlag = obj.xAxisUserTitleFlag;
    yAxisUserTitleFlag = obj.yAxisUserTitleFlag;
    zAxisUserTitleFlag = obj.zAxisUserTitleFlag;
    xAxisUserUnits = obj.xAxisUserUnits;
    yAxisUserUnits = obj.yAxisUserUnits;
    zAxisUserUnits = obj.zAxisUserUnits;
    xAxisUserUnitsFlag = obj.xAxisUserUnitsFlag;
    yAxisUserUnitsFlag = obj.yAxisUserUnitsFlag;
    zAxisUserUnitsFlag = obj.zAxisUserUnitsFlag;
    axesTickLocation = obj.axesTickLocation;
    axesType = obj.axesType;
    triadFlag = obj.triadFlag;
    bboxFlag = obj.bboxFlag;
    backgroundColor = obj.backgroundColor;
    foregroundColor = obj.foregroundColor;
    gradientBackgroundStyle = obj.gradientBackgroundStyle;
    gradientColor1 = obj.gradientColor1;
    gradientColor2 = obj.gradientColor2;
    backgroundMode = obj.backgroundMode;
    backgroundImage = obj.backgroundImage;
    userInfoFlag = obj.userInfoFlag;
    databaseInfoFlag = obj.databaseInfoFlag;
    databaseInfoExpansionMode = obj.databaseInfoExpansionMode;
    legendInfoFlag = obj.legendInfoFlag;
    imageRepeatX = obj.imageRepeatX;
    imageRepeatY = obj.imageRepeatY;

    SelectAll();
}

// ****************************************************************************
// Method: AnnotationAttributes::~AnnotationAttributes
//
// Purpose: 
//   Destructor for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AnnotationAttributes::~AnnotationAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: AnnotationAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AnnotationAttributes& 
AnnotationAttributes::operator = (const AnnotationAttributes &obj)
{
    if (this == &obj) return *this;
    axesFlag2D = obj.axesFlag2D;
    axesAutoSetTicks2D = obj.axesAutoSetTicks2D;
    labelAutoSetScaling2D = obj.labelAutoSetScaling2D;
    xAxisLabels2D = obj.xAxisLabels2D;
    yAxisLabels2D = obj.yAxisLabels2D;
    xAxisTitle2D = obj.xAxisTitle2D;
    yAxisTitle2D = obj.yAxisTitle2D;
    xGridLines2D = obj.xGridLines2D;
    yGridLines2D = obj.yGridLines2D;
    xMajorTickMinimum2D = obj.xMajorTickMinimum2D;
    yMajorTickMinimum2D = obj.yMajorTickMinimum2D;
    xMajorTickMaximum2D = obj.xMajorTickMaximum2D;
    yMajorTickMaximum2D = obj.yMajorTickMaximum2D;
    xMajorTickSpacing2D = obj.xMajorTickSpacing2D;
    yMajorTickSpacing2D = obj.yMajorTickSpacing2D;
    xMinorTickSpacing2D = obj.xMinorTickSpacing2D;
    yMinorTickSpacing2D = obj.yMinorTickSpacing2D;
    xLabelFontHeight2D = obj.xLabelFontHeight2D;
    yLabelFontHeight2D = obj.yLabelFontHeight2D;
    xTitleFontHeight2D = obj.xTitleFontHeight2D;
    yTitleFontHeight2D = obj.yTitleFontHeight2D;
    xLabelScaling2D = obj.xLabelScaling2D;
    yLabelScaling2D = obj.yLabelScaling2D;
    axesLineWidth2D = obj.axesLineWidth2D;
    axesTickLocation2D = obj.axesTickLocation2D;
    axesTicks2D = obj.axesTicks2D;
    xAxisUserTitle2D = obj.xAxisUserTitle2D;
    yAxisUserTitle2D = obj.yAxisUserTitle2D;
    xAxisUserTitleFlag2D = obj.xAxisUserTitleFlag2D;
    yAxisUserTitleFlag2D = obj.yAxisUserTitleFlag2D;
    xAxisUserUnits2D = obj.xAxisUserUnits2D;
    yAxisUserUnits2D = obj.yAxisUserUnits2D;
    xAxisUserUnitsFlag2D = obj.xAxisUserUnitsFlag2D;
    yAxisUserUnitsFlag2D = obj.yAxisUserUnitsFlag2D;
    axesFlag = obj.axesFlag;
    axesAutoSetTicks = obj.axesAutoSetTicks;
    labelAutoSetScaling = obj.labelAutoSetScaling;
    xAxisLabels = obj.xAxisLabels;
    yAxisLabels = obj.yAxisLabels;
    zAxisLabels = obj.zAxisLabels;
    xAxisTitle = obj.xAxisTitle;
    yAxisTitle = obj.yAxisTitle;
    zAxisTitle = obj.zAxisTitle;
    xGridLines = obj.xGridLines;
    yGridLines = obj.yGridLines;
    zGridLines = obj.zGridLines;
    xAxisTicks = obj.xAxisTicks;
    yAxisTicks = obj.yAxisTicks;
    zAxisTicks = obj.zAxisTicks;
    xMajorTickMinimum = obj.xMajorTickMinimum;
    yMajorTickMinimum = obj.yMajorTickMinimum;
    zMajorTickMinimum = obj.zMajorTickMinimum;
    xMajorTickMaximum = obj.xMajorTickMaximum;
    yMajorTickMaximum = obj.yMajorTickMaximum;
    zMajorTickMaximum = obj.zMajorTickMaximum;
    xMajorTickSpacing = obj.xMajorTickSpacing;
    yMajorTickSpacing = obj.yMajorTickSpacing;
    zMajorTickSpacing = obj.zMajorTickSpacing;
    xMinorTickSpacing = obj.xMinorTickSpacing;
    yMinorTickSpacing = obj.yMinorTickSpacing;
    zMinorTickSpacing = obj.zMinorTickSpacing;
    xLabelFontHeight = obj.xLabelFontHeight;
    yLabelFontHeight = obj.yLabelFontHeight;
    zLabelFontHeight = obj.zLabelFontHeight;
    xTitleFontHeight = obj.xTitleFontHeight;
    yTitleFontHeight = obj.yTitleFontHeight;
    zTitleFontHeight = obj.zTitleFontHeight;
    xLabelScaling = obj.xLabelScaling;
    yLabelScaling = obj.yLabelScaling;
    zLabelScaling = obj.zLabelScaling;
    xAxisUserTitle = obj.xAxisUserTitle;
    yAxisUserTitle = obj.yAxisUserTitle;
    zAxisUserTitle = obj.zAxisUserTitle;
    xAxisUserTitleFlag = obj.xAxisUserTitleFlag;
    yAxisUserTitleFlag = obj.yAxisUserTitleFlag;
    zAxisUserTitleFlag = obj.zAxisUserTitleFlag;
    xAxisUserUnits = obj.xAxisUserUnits;
    yAxisUserUnits = obj.yAxisUserUnits;
    zAxisUserUnits = obj.zAxisUserUnits;
    xAxisUserUnitsFlag = obj.xAxisUserUnitsFlag;
    yAxisUserUnitsFlag = obj.yAxisUserUnitsFlag;
    zAxisUserUnitsFlag = obj.zAxisUserUnitsFlag;
    axesTickLocation = obj.axesTickLocation;
    axesType = obj.axesType;
    triadFlag = obj.triadFlag;
    bboxFlag = obj.bboxFlag;
    backgroundColor = obj.backgroundColor;
    foregroundColor = obj.foregroundColor;
    gradientBackgroundStyle = obj.gradientBackgroundStyle;
    gradientColor1 = obj.gradientColor1;
    gradientColor2 = obj.gradientColor2;
    backgroundMode = obj.backgroundMode;
    backgroundImage = obj.backgroundImage;
    userInfoFlag = obj.userInfoFlag;
    databaseInfoFlag = obj.databaseInfoFlag;
    databaseInfoExpansionMode = obj.databaseInfoExpansionMode;
    legendInfoFlag = obj.legendInfoFlag;
    imageRepeatX = obj.imageRepeatX;
    imageRepeatY = obj.imageRepeatY;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: AnnotationAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
AnnotationAttributes::operator == (const AnnotationAttributes &obj) const
{
    // Create the return value
    return ((axesFlag2D == obj.axesFlag2D) &&
            (axesAutoSetTicks2D == obj.axesAutoSetTicks2D) &&
            (labelAutoSetScaling2D == obj.labelAutoSetScaling2D) &&
            (xAxisLabels2D == obj.xAxisLabels2D) &&
            (yAxisLabels2D == obj.yAxisLabels2D) &&
            (xAxisTitle2D == obj.xAxisTitle2D) &&
            (yAxisTitle2D == obj.yAxisTitle2D) &&
            (xGridLines2D == obj.xGridLines2D) &&
            (yGridLines2D == obj.yGridLines2D) &&
            (xMajorTickMinimum2D == obj.xMajorTickMinimum2D) &&
            (yMajorTickMinimum2D == obj.yMajorTickMinimum2D) &&
            (xMajorTickMaximum2D == obj.xMajorTickMaximum2D) &&
            (yMajorTickMaximum2D == obj.yMajorTickMaximum2D) &&
            (xMajorTickSpacing2D == obj.xMajorTickSpacing2D) &&
            (yMajorTickSpacing2D == obj.yMajorTickSpacing2D) &&
            (xMinorTickSpacing2D == obj.xMinorTickSpacing2D) &&
            (yMinorTickSpacing2D == obj.yMinorTickSpacing2D) &&
            (xLabelFontHeight2D == obj.xLabelFontHeight2D) &&
            (yLabelFontHeight2D == obj.yLabelFontHeight2D) &&
            (xTitleFontHeight2D == obj.xTitleFontHeight2D) &&
            (yTitleFontHeight2D == obj.yTitleFontHeight2D) &&
            (xLabelScaling2D == obj.xLabelScaling2D) &&
            (yLabelScaling2D == obj.yLabelScaling2D) &&
            (axesLineWidth2D == obj.axesLineWidth2D) &&
            (axesTickLocation2D == obj.axesTickLocation2D) &&
            (axesTicks2D == obj.axesTicks2D) &&
            (xAxisUserTitle2D == obj.xAxisUserTitle2D) &&
            (yAxisUserTitle2D == obj.yAxisUserTitle2D) &&
            (xAxisUserTitleFlag2D == obj.xAxisUserTitleFlag2D) &&
            (yAxisUserTitleFlag2D == obj.yAxisUserTitleFlag2D) &&
            (xAxisUserUnits2D == obj.xAxisUserUnits2D) &&
            (yAxisUserUnits2D == obj.yAxisUserUnits2D) &&
            (xAxisUserUnitsFlag2D == obj.xAxisUserUnitsFlag2D) &&
            (yAxisUserUnitsFlag2D == obj.yAxisUserUnitsFlag2D) &&
            (axesFlag == obj.axesFlag) &&
            (axesAutoSetTicks == obj.axesAutoSetTicks) &&
            (labelAutoSetScaling == obj.labelAutoSetScaling) &&
            (xAxisLabels == obj.xAxisLabels) &&
            (yAxisLabels == obj.yAxisLabels) &&
            (zAxisLabels == obj.zAxisLabels) &&
            (xAxisTitle == obj.xAxisTitle) &&
            (yAxisTitle == obj.yAxisTitle) &&
            (zAxisTitle == obj.zAxisTitle) &&
            (xGridLines == obj.xGridLines) &&
            (yGridLines == obj.yGridLines) &&
            (zGridLines == obj.zGridLines) &&
            (xAxisTicks == obj.xAxisTicks) &&
            (yAxisTicks == obj.yAxisTicks) &&
            (zAxisTicks == obj.zAxisTicks) &&
            (xMajorTickMinimum == obj.xMajorTickMinimum) &&
            (yMajorTickMinimum == obj.yMajorTickMinimum) &&
            (zMajorTickMinimum == obj.zMajorTickMinimum) &&
            (xMajorTickMaximum == obj.xMajorTickMaximum) &&
            (yMajorTickMaximum == obj.yMajorTickMaximum) &&
            (zMajorTickMaximum == obj.zMajorTickMaximum) &&
            (xMajorTickSpacing == obj.xMajorTickSpacing) &&
            (yMajorTickSpacing == obj.yMajorTickSpacing) &&
            (zMajorTickSpacing == obj.zMajorTickSpacing) &&
            (xMinorTickSpacing == obj.xMinorTickSpacing) &&
            (yMinorTickSpacing == obj.yMinorTickSpacing) &&
            (zMinorTickSpacing == obj.zMinorTickSpacing) &&
            (xLabelFontHeight == obj.xLabelFontHeight) &&
            (yLabelFontHeight == obj.yLabelFontHeight) &&
            (zLabelFontHeight == obj.zLabelFontHeight) &&
            (xTitleFontHeight == obj.xTitleFontHeight) &&
            (yTitleFontHeight == obj.yTitleFontHeight) &&
            (zTitleFontHeight == obj.zTitleFontHeight) &&
            (xLabelScaling == obj.xLabelScaling) &&
            (yLabelScaling == obj.yLabelScaling) &&
            (zLabelScaling == obj.zLabelScaling) &&
            (xAxisUserTitle == obj.xAxisUserTitle) &&
            (yAxisUserTitle == obj.yAxisUserTitle) &&
            (zAxisUserTitle == obj.zAxisUserTitle) &&
            (xAxisUserTitleFlag == obj.xAxisUserTitleFlag) &&
            (yAxisUserTitleFlag == obj.yAxisUserTitleFlag) &&
            (zAxisUserTitleFlag == obj.zAxisUserTitleFlag) &&
            (xAxisUserUnits == obj.xAxisUserUnits) &&
            (yAxisUserUnits == obj.yAxisUserUnits) &&
            (zAxisUserUnits == obj.zAxisUserUnits) &&
            (xAxisUserUnitsFlag == obj.xAxisUserUnitsFlag) &&
            (yAxisUserUnitsFlag == obj.yAxisUserUnitsFlag) &&
            (zAxisUserUnitsFlag == obj.zAxisUserUnitsFlag) &&
            (axesTickLocation == obj.axesTickLocation) &&
            (axesType == obj.axesType) &&
            (triadFlag == obj.triadFlag) &&
            (bboxFlag == obj.bboxFlag) &&
            (backgroundColor == obj.backgroundColor) &&
            (foregroundColor == obj.foregroundColor) &&
            (gradientBackgroundStyle == obj.gradientBackgroundStyle) &&
            (gradientColor1 == obj.gradientColor1) &&
            (gradientColor2 == obj.gradientColor2) &&
            (backgroundMode == obj.backgroundMode) &&
            (backgroundImage == obj.backgroundImage) &&
            (userInfoFlag == obj.userInfoFlag) &&
            (databaseInfoFlag == obj.databaseInfoFlag) &&
            (databaseInfoExpansionMode == obj.databaseInfoExpansionMode) &&
            (legendInfoFlag == obj.legendInfoFlag) &&
            (imageRepeatX == obj.imageRepeatX) &&
            (imageRepeatY == obj.imageRepeatY));
}

// ****************************************************************************
// Method: AnnotationAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
AnnotationAttributes::operator != (const AnnotationAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: AnnotationAttributes::TypeName
//
// Purpose: 
//   Type name method for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

const std::string
AnnotationAttributes::TypeName() const
{
    return "AnnotationAttributes";
}

// ****************************************************************************
// Method: AnnotationAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
AnnotationAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const AnnotationAttributes *tmp = (const AnnotationAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: AnnotationAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
AnnotationAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new AnnotationAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: AnnotationAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
AnnotationAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new AnnotationAttributes(*this);
    else
        retval = new AnnotationAttributes;

    return retval;
}

// ****************************************************************************
// Method: AnnotationAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
AnnotationAttributes::SelectAll()
{
    Select(ID_axesFlag2D,                (void *)&axesFlag2D);
    Select(ID_axesAutoSetTicks2D,        (void *)&axesAutoSetTicks2D);
    Select(ID_labelAutoSetScaling2D,     (void *)&labelAutoSetScaling2D);
    Select(ID_xAxisLabels2D,             (void *)&xAxisLabels2D);
    Select(ID_yAxisLabels2D,             (void *)&yAxisLabels2D);
    Select(ID_xAxisTitle2D,              (void *)&xAxisTitle2D);
    Select(ID_yAxisTitle2D,              (void *)&yAxisTitle2D);
    Select(ID_xGridLines2D,              (void *)&xGridLines2D);
    Select(ID_yGridLines2D,              (void *)&yGridLines2D);
    Select(ID_xMajorTickMinimum2D,       (void *)&xMajorTickMinimum2D);
    Select(ID_yMajorTickMinimum2D,       (void *)&yMajorTickMinimum2D);
    Select(ID_xMajorTickMaximum2D,       (void *)&xMajorTickMaximum2D);
    Select(ID_yMajorTickMaximum2D,       (void *)&yMajorTickMaximum2D);
    Select(ID_xMajorTickSpacing2D,       (void *)&xMajorTickSpacing2D);
    Select(ID_yMajorTickSpacing2D,       (void *)&yMajorTickSpacing2D);
    Select(ID_xMinorTickSpacing2D,       (void *)&xMinorTickSpacing2D);
    Select(ID_yMinorTickSpacing2D,       (void *)&yMinorTickSpacing2D);
    Select(ID_xLabelFontHeight2D,        (void *)&xLabelFontHeight2D);
    Select(ID_yLabelFontHeight2D,        (void *)&yLabelFontHeight2D);
    Select(ID_xTitleFontHeight2D,        (void *)&xTitleFontHeight2D);
    Select(ID_yTitleFontHeight2D,        (void *)&yTitleFontHeight2D);
    Select(ID_xLabelScaling2D,           (void *)&xLabelScaling2D);
    Select(ID_yLabelScaling2D,           (void *)&yLabelScaling2D);
    Select(ID_axesLineWidth2D,           (void *)&axesLineWidth2D);
    Select(ID_axesTickLocation2D,        (void *)&axesTickLocation2D);
    Select(ID_axesTicks2D,               (void *)&axesTicks2D);
    Select(ID_xAxisUserTitle2D,          (void *)&xAxisUserTitle2D);
    Select(ID_yAxisUserTitle2D,          (void *)&yAxisUserTitle2D);
    Select(ID_xAxisUserTitleFlag2D,      (void *)&xAxisUserTitleFlag2D);
    Select(ID_yAxisUserTitleFlag2D,      (void *)&yAxisUserTitleFlag2D);
    Select(ID_xAxisUserUnits2D,          (void *)&xAxisUserUnits2D);
    Select(ID_yAxisUserUnits2D,          (void *)&yAxisUserUnits2D);
    Select(ID_xAxisUserUnitsFlag2D,      (void *)&xAxisUserUnitsFlag2D);
    Select(ID_yAxisUserUnitsFlag2D,      (void *)&yAxisUserUnitsFlag2D);
    Select(ID_axesFlag,                  (void *)&axesFlag);
    Select(ID_axesAutoSetTicks,          (void *)&axesAutoSetTicks);
    Select(ID_labelAutoSetScaling,       (void *)&labelAutoSetScaling);
    Select(ID_xAxisLabels,               (void *)&xAxisLabels);
    Select(ID_yAxisLabels,               (void *)&yAxisLabels);
    Select(ID_zAxisLabels,               (void *)&zAxisLabels);
    Select(ID_xAxisTitle,                (void *)&xAxisTitle);
    Select(ID_yAxisTitle,                (void *)&yAxisTitle);
    Select(ID_zAxisTitle,                (void *)&zAxisTitle);
    Select(ID_xGridLines,                (void *)&xGridLines);
    Select(ID_yGridLines,                (void *)&yGridLines);
    Select(ID_zGridLines,                (void *)&zGridLines);
    Select(ID_xAxisTicks,                (void *)&xAxisTicks);
    Select(ID_yAxisTicks,                (void *)&yAxisTicks);
    Select(ID_zAxisTicks,                (void *)&zAxisTicks);
    Select(ID_xMajorTickMinimum,         (void *)&xMajorTickMinimum);
    Select(ID_yMajorTickMinimum,         (void *)&yMajorTickMinimum);
    Select(ID_zMajorTickMinimum,         (void *)&zMajorTickMinimum);
    Select(ID_xMajorTickMaximum,         (void *)&xMajorTickMaximum);
    Select(ID_yMajorTickMaximum,         (void *)&yMajorTickMaximum);
    Select(ID_zMajorTickMaximum,         (void *)&zMajorTickMaximum);
    Select(ID_xMajorTickSpacing,         (void *)&xMajorTickSpacing);
    Select(ID_yMajorTickSpacing,         (void *)&yMajorTickSpacing);
    Select(ID_zMajorTickSpacing,         (void *)&zMajorTickSpacing);
    Select(ID_xMinorTickSpacing,         (void *)&xMinorTickSpacing);
    Select(ID_yMinorTickSpacing,         (void *)&yMinorTickSpacing);
    Select(ID_zMinorTickSpacing,         (void *)&zMinorTickSpacing);
    Select(ID_xLabelFontHeight,          (void *)&xLabelFontHeight);
    Select(ID_yLabelFontHeight,          (void *)&yLabelFontHeight);
    Select(ID_zLabelFontHeight,          (void *)&zLabelFontHeight);
    Select(ID_xTitleFontHeight,          (void *)&xTitleFontHeight);
    Select(ID_yTitleFontHeight,          (void *)&yTitleFontHeight);
    Select(ID_zTitleFontHeight,          (void *)&zTitleFontHeight);
    Select(ID_xLabelScaling,             (void *)&xLabelScaling);
    Select(ID_yLabelScaling,             (void *)&yLabelScaling);
    Select(ID_zLabelScaling,             (void *)&zLabelScaling);
    Select(ID_xAxisUserTitle,            (void *)&xAxisUserTitle);
    Select(ID_yAxisUserTitle,            (void *)&yAxisUserTitle);
    Select(ID_zAxisUserTitle,            (void *)&zAxisUserTitle);
    Select(ID_xAxisUserTitleFlag,        (void *)&xAxisUserTitleFlag);
    Select(ID_yAxisUserTitleFlag,        (void *)&yAxisUserTitleFlag);
    Select(ID_zAxisUserTitleFlag,        (void *)&zAxisUserTitleFlag);
    Select(ID_xAxisUserUnits,            (void *)&xAxisUserUnits);
    Select(ID_yAxisUserUnits,            (void *)&yAxisUserUnits);
    Select(ID_zAxisUserUnits,            (void *)&zAxisUserUnits);
    Select(ID_xAxisUserUnitsFlag,        (void *)&xAxisUserUnitsFlag);
    Select(ID_yAxisUserUnitsFlag,        (void *)&yAxisUserUnitsFlag);
    Select(ID_zAxisUserUnitsFlag,        (void *)&zAxisUserUnitsFlag);
    Select(ID_axesTickLocation,          (void *)&axesTickLocation);
    Select(ID_axesType,                  (void *)&axesType);
    Select(ID_triadFlag,                 (void *)&triadFlag);
    Select(ID_bboxFlag,                  (void *)&bboxFlag);
    Select(ID_backgroundColor,           (void *)&backgroundColor);
    Select(ID_foregroundColor,           (void *)&foregroundColor);
    Select(ID_gradientBackgroundStyle,   (void *)&gradientBackgroundStyle);
    Select(ID_gradientColor1,            (void *)&gradientColor1);
    Select(ID_gradientColor2,            (void *)&gradientColor2);
    Select(ID_backgroundMode,            (void *)&backgroundMode);
    Select(ID_backgroundImage,           (void *)&backgroundImage);
    Select(ID_userInfoFlag,              (void *)&userInfoFlag);
    Select(ID_databaseInfoFlag,          (void *)&databaseInfoFlag);
    Select(ID_databaseInfoExpansionMode, (void *)&databaseInfoExpansionMode);
    Select(ID_legendInfoFlag,            (void *)&legendInfoFlag);
    Select(ID_imageRepeatX,              (void *)&imageRepeatX);
    Select(ID_imageRepeatY,              (void *)&imageRepeatY);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AnnotationAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
AnnotationAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    AnnotationAttributes defaultObject;
    bool addToParent = false;
    // Create a node for AnnotationAttributes.
    DataNode *node = new DataNode("AnnotationAttributes");

    if(completeSave || !FieldsEqual(ID_axesFlag2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesFlag2D", axesFlag2D));
    }

    if(completeSave || !FieldsEqual(ID_axesAutoSetTicks2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesAutoSetTicks2D", axesAutoSetTicks2D));
    }

    if(completeSave || !FieldsEqual(ID_labelAutoSetScaling2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("labelAutoSetScaling2D", labelAutoSetScaling2D));
    }

    if(completeSave || !FieldsEqual(ID_xAxisLabels2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisLabels2D", xAxisLabels2D));
    }

    if(completeSave || !FieldsEqual(ID_yAxisLabels2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisLabels2D", yAxisLabels2D));
    }

    if(completeSave || !FieldsEqual(ID_xAxisTitle2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisTitle2D", xAxisTitle2D));
    }

    if(completeSave || !FieldsEqual(ID_yAxisTitle2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisTitle2D", yAxisTitle2D));
    }

    if(completeSave || !FieldsEqual(ID_xGridLines2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xGridLines2D", xGridLines2D));
    }

    if(completeSave || !FieldsEqual(ID_yGridLines2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yGridLines2D", yGridLines2D));
    }

    if(completeSave || !FieldsEqual(ID_xMajorTickMinimum2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMajorTickMinimum2D", xMajorTickMinimum2D));
    }

    if(completeSave || !FieldsEqual(ID_yMajorTickMinimum2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMajorTickMinimum2D", yMajorTickMinimum2D));
    }

    if(completeSave || !FieldsEqual(ID_xMajorTickMaximum2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMajorTickMaximum2D", xMajorTickMaximum2D));
    }

    if(completeSave || !FieldsEqual(ID_yMajorTickMaximum2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMajorTickMaximum2D", yMajorTickMaximum2D));
    }

    if(completeSave || !FieldsEqual(ID_xMajorTickSpacing2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMajorTickSpacing2D", xMajorTickSpacing2D));
    }

    if(completeSave || !FieldsEqual(ID_yMajorTickSpacing2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMajorTickSpacing2D", yMajorTickSpacing2D));
    }

    if(completeSave || !FieldsEqual(ID_xMinorTickSpacing2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMinorTickSpacing2D", xMinorTickSpacing2D));
    }

    if(completeSave || !FieldsEqual(ID_yMinorTickSpacing2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMinorTickSpacing2D", yMinorTickSpacing2D));
    }

    if(completeSave || !FieldsEqual(ID_xLabelFontHeight2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xLabelFontHeight2D", xLabelFontHeight2D));
    }

    if(completeSave || !FieldsEqual(ID_yLabelFontHeight2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yLabelFontHeight2D", yLabelFontHeight2D));
    }

    if(completeSave || !FieldsEqual(ID_xTitleFontHeight2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xTitleFontHeight2D", xTitleFontHeight2D));
    }

    if(completeSave || !FieldsEqual(ID_yTitleFontHeight2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yTitleFontHeight2D", yTitleFontHeight2D));
    }

    if(completeSave || !FieldsEqual(ID_xLabelScaling2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xLabelScaling2D", xLabelScaling2D));
    }

    if(completeSave || !FieldsEqual(ID_yLabelScaling2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yLabelScaling2D", yLabelScaling2D));
    }

    if(completeSave || !FieldsEqual(ID_axesLineWidth2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesLineWidth2D", axesLineWidth2D));
    }

    if(completeSave || !FieldsEqual(ID_axesTickLocation2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesTickLocation2D", Location_ToString(axesTickLocation2D)));
    }

    if(completeSave || !FieldsEqual(ID_axesTicks2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesTicks2D", Ticks_ToString(axesTicks2D)));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserTitle2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserTitle2D", xAxisUserTitle2D));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserTitle2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserTitle2D", yAxisUserTitle2D));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserTitleFlag2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserTitleFlag2D", xAxisUserTitleFlag2D));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserTitleFlag2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserTitleFlag2D", yAxisUserTitleFlag2D));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserUnits2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserUnits2D", xAxisUserUnits2D));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserUnits2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserUnits2D", yAxisUserUnits2D));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserUnitsFlag2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserUnitsFlag2D", xAxisUserUnitsFlag2D));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserUnitsFlag2D, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserUnitsFlag2D", yAxisUserUnitsFlag2D));
    }

    if(completeSave || !FieldsEqual(ID_axesFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesFlag", axesFlag));
    }

    if(completeSave || !FieldsEqual(ID_axesAutoSetTicks, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesAutoSetTicks", axesAutoSetTicks));
    }

    if(completeSave || !FieldsEqual(ID_labelAutoSetScaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("labelAutoSetScaling", labelAutoSetScaling));
    }

    if(completeSave || !FieldsEqual(ID_xAxisLabels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisLabels", xAxisLabels));
    }

    if(completeSave || !FieldsEqual(ID_yAxisLabels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisLabels", yAxisLabels));
    }

    if(completeSave || !FieldsEqual(ID_zAxisLabels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zAxisLabels", zAxisLabels));
    }

    if(completeSave || !FieldsEqual(ID_xAxisTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisTitle", xAxisTitle));
    }

    if(completeSave || !FieldsEqual(ID_yAxisTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisTitle", yAxisTitle));
    }

    if(completeSave || !FieldsEqual(ID_zAxisTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zAxisTitle", zAxisTitle));
    }

    if(completeSave || !FieldsEqual(ID_xGridLines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xGridLines", xGridLines));
    }

    if(completeSave || !FieldsEqual(ID_yGridLines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yGridLines", yGridLines));
    }

    if(completeSave || !FieldsEqual(ID_zGridLines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zGridLines", zGridLines));
    }

    if(completeSave || !FieldsEqual(ID_xAxisTicks, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisTicks", xAxisTicks));
    }

    if(completeSave || !FieldsEqual(ID_yAxisTicks, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisTicks", yAxisTicks));
    }

    if(completeSave || !FieldsEqual(ID_zAxisTicks, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zAxisTicks", zAxisTicks));
    }

    if(completeSave || !FieldsEqual(ID_xMajorTickMinimum, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMajorTickMinimum", xMajorTickMinimum));
    }

    if(completeSave || !FieldsEqual(ID_yMajorTickMinimum, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMajorTickMinimum", yMajorTickMinimum));
    }

    if(completeSave || !FieldsEqual(ID_zMajorTickMinimum, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zMajorTickMinimum", zMajorTickMinimum));
    }

    if(completeSave || !FieldsEqual(ID_xMajorTickMaximum, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMajorTickMaximum", xMajorTickMaximum));
    }

    if(completeSave || !FieldsEqual(ID_yMajorTickMaximum, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMajorTickMaximum", yMajorTickMaximum));
    }

    if(completeSave || !FieldsEqual(ID_zMajorTickMaximum, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zMajorTickMaximum", zMajorTickMaximum));
    }

    if(completeSave || !FieldsEqual(ID_xMajorTickSpacing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMajorTickSpacing", xMajorTickSpacing));
    }

    if(completeSave || !FieldsEqual(ID_yMajorTickSpacing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMajorTickSpacing", yMajorTickSpacing));
    }

    if(completeSave || !FieldsEqual(ID_zMajorTickSpacing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zMajorTickSpacing", zMajorTickSpacing));
    }

    if(completeSave || !FieldsEqual(ID_xMinorTickSpacing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xMinorTickSpacing", xMinorTickSpacing));
    }

    if(completeSave || !FieldsEqual(ID_yMinorTickSpacing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yMinorTickSpacing", yMinorTickSpacing));
    }

    if(completeSave || !FieldsEqual(ID_zMinorTickSpacing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zMinorTickSpacing", zMinorTickSpacing));
    }

    if(completeSave || !FieldsEqual(ID_xLabelFontHeight, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xLabelFontHeight", xLabelFontHeight));
    }

    if(completeSave || !FieldsEqual(ID_yLabelFontHeight, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yLabelFontHeight", yLabelFontHeight));
    }

    if(completeSave || !FieldsEqual(ID_zLabelFontHeight, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zLabelFontHeight", zLabelFontHeight));
    }

    if(completeSave || !FieldsEqual(ID_xTitleFontHeight, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xTitleFontHeight", xTitleFontHeight));
    }

    if(completeSave || !FieldsEqual(ID_yTitleFontHeight, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yTitleFontHeight", yTitleFontHeight));
    }

    if(completeSave || !FieldsEqual(ID_zTitleFontHeight, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zTitleFontHeight", zTitleFontHeight));
    }

    if(completeSave || !FieldsEqual(ID_xLabelScaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xLabelScaling", xLabelScaling));
    }

    if(completeSave || !FieldsEqual(ID_yLabelScaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yLabelScaling", yLabelScaling));
    }

    if(completeSave || !FieldsEqual(ID_zLabelScaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zLabelScaling", zLabelScaling));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserTitle", xAxisUserTitle));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserTitle", yAxisUserTitle));
    }

    if(completeSave || !FieldsEqual(ID_zAxisUserTitle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zAxisUserTitle", zAxisUserTitle));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserTitleFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserTitleFlag", xAxisUserTitleFlag));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserTitleFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserTitleFlag", yAxisUserTitleFlag));
    }

    if(completeSave || !FieldsEqual(ID_zAxisUserTitleFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zAxisUserTitleFlag", zAxisUserTitleFlag));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserUnits, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserUnits", xAxisUserUnits));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserUnits, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserUnits", yAxisUserUnits));
    }

    if(completeSave || !FieldsEqual(ID_zAxisUserUnits, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zAxisUserUnits", zAxisUserUnits));
    }

    if(completeSave || !FieldsEqual(ID_xAxisUserUnitsFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("xAxisUserUnitsFlag", xAxisUserUnitsFlag));
    }

    if(completeSave || !FieldsEqual(ID_yAxisUserUnitsFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("yAxisUserUnitsFlag", yAxisUserUnitsFlag));
    }

    if(completeSave || !FieldsEqual(ID_zAxisUserUnitsFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zAxisUserUnitsFlag", zAxisUserUnitsFlag));
    }

    if(completeSave || !FieldsEqual(ID_axesTickLocation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesTickLocation", Location_ToString(axesTickLocation)));
    }

    if(completeSave || !FieldsEqual(ID_axesType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("axesType", Axes_ToString(axesType)));
    }

    if(completeSave || !FieldsEqual(ID_triadFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("triadFlag", triadFlag));
    }

    if(completeSave || !FieldsEqual(ID_bboxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("bboxFlag", bboxFlag));
    }

        DataNode *backgroundColorNode = new DataNode("backgroundColor");
        if(backgroundColor.CreateNode(backgroundColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(backgroundColorNode);
        }
        else
            delete backgroundColorNode;
        DataNode *foregroundColorNode = new DataNode("foregroundColor");
        if(foregroundColor.CreateNode(foregroundColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(foregroundColorNode);
        }
        else
            delete foregroundColorNode;
    if(completeSave || !FieldsEqual(ID_gradientBackgroundStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("gradientBackgroundStyle", GradientStyle_ToString(gradientBackgroundStyle)));
    }

        DataNode *gradientColor1Node = new DataNode("gradientColor1");
        if(gradientColor1.CreateNode(gradientColor1Node, completeSave, true))
        {
            addToParent = true;
            node->AddNode(gradientColor1Node);
        }
        else
            delete gradientColor1Node;
        DataNode *gradientColor2Node = new DataNode("gradientColor2");
        if(gradientColor2.CreateNode(gradientColor2Node, completeSave, true))
        {
            addToParent = true;
            node->AddNode(gradientColor2Node);
        }
        else
            delete gradientColor2Node;
    if(completeSave || !FieldsEqual(ID_backgroundMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("backgroundMode", BackgroundMode_ToString(backgroundMode)));
    }

    if(completeSave || !FieldsEqual(ID_backgroundImage, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("backgroundImage", backgroundImage));
    }

    if(completeSave || !FieldsEqual(ID_userInfoFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("userInfoFlag", userInfoFlag));
    }

    if(completeSave || !FieldsEqual(ID_databaseInfoFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("databaseInfoFlag", databaseInfoFlag));
    }

    if(completeSave || !FieldsEqual(ID_databaseInfoExpansionMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("databaseInfoExpansionMode", PathExpansionMode_ToString(databaseInfoExpansionMode)));
    }

    if(completeSave || !FieldsEqual(ID_legendInfoFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendInfoFlag", legendInfoFlag));
    }

    if(completeSave || !FieldsEqual(ID_imageRepeatX, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("imageRepeatX", imageRepeatX));
    }

    if(completeSave || !FieldsEqual(ID_imageRepeatY, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("imageRepeatY", imageRepeatY));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: AnnotationAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
AnnotationAttributes::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("AnnotationAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("axesFlag2D")) != 0)
        SetAxesFlag2D(node->AsBool());
    if((node = searchNode->GetNode("axesAutoSetTicks2D")) != 0)
        SetAxesAutoSetTicks2D(node->AsBool());
    if((node = searchNode->GetNode("labelAutoSetScaling2D")) != 0)
        SetLabelAutoSetScaling2D(node->AsBool());
    if((node = searchNode->GetNode("xAxisLabels2D")) != 0)
        SetXAxisLabels2D(node->AsBool());
    if((node = searchNode->GetNode("yAxisLabels2D")) != 0)
        SetYAxisLabels2D(node->AsBool());
    if((node = searchNode->GetNode("xAxisTitle2D")) != 0)
        SetXAxisTitle2D(node->AsBool());
    if((node = searchNode->GetNode("yAxisTitle2D")) != 0)
        SetYAxisTitle2D(node->AsBool());
    if((node = searchNode->GetNode("xGridLines2D")) != 0)
        SetXGridLines2D(node->AsBool());
    if((node = searchNode->GetNode("yGridLines2D")) != 0)
        SetYGridLines2D(node->AsBool());
    if((node = searchNode->GetNode("xMajorTickMinimum2D")) != 0)
        SetXMajorTickMinimum2D(node->AsDouble());
    if((node = searchNode->GetNode("yMajorTickMinimum2D")) != 0)
        SetYMajorTickMinimum2D(node->AsDouble());
    if((node = searchNode->GetNode("xMajorTickMaximum2D")) != 0)
        SetXMajorTickMaximum2D(node->AsDouble());
    if((node = searchNode->GetNode("yMajorTickMaximum2D")) != 0)
        SetYMajorTickMaximum2D(node->AsDouble());
    if((node = searchNode->GetNode("xMajorTickSpacing2D")) != 0)
        SetXMajorTickSpacing2D(node->AsDouble());
    if((node = searchNode->GetNode("yMajorTickSpacing2D")) != 0)
        SetYMajorTickSpacing2D(node->AsDouble());
    if((node = searchNode->GetNode("xMinorTickSpacing2D")) != 0)
        SetXMinorTickSpacing2D(node->AsDouble());
    if((node = searchNode->GetNode("yMinorTickSpacing2D")) != 0)
        SetYMinorTickSpacing2D(node->AsDouble());
    if((node = searchNode->GetNode("xLabelFontHeight2D")) != 0)
        SetXLabelFontHeight2D(node->AsDouble());
    if((node = searchNode->GetNode("yLabelFontHeight2D")) != 0)
        SetYLabelFontHeight2D(node->AsDouble());
    if((node = searchNode->GetNode("xTitleFontHeight2D")) != 0)
        SetXTitleFontHeight2D(node->AsDouble());
    if((node = searchNode->GetNode("yTitleFontHeight2D")) != 0)
        SetYTitleFontHeight2D(node->AsDouble());
    if((node = searchNode->GetNode("xLabelScaling2D")) != 0)
        SetXLabelScaling2D(node->AsInt());
    if((node = searchNode->GetNode("yLabelScaling2D")) != 0)
        SetYLabelScaling2D(node->AsInt());
    if((node = searchNode->GetNode("axesLineWidth2D")) != 0)
        SetAxesLineWidth2D(node->AsInt());
    if((node = searchNode->GetNode("axesTickLocation2D")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetAxesTickLocation2D(Location(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Location value;
            if(Location_FromString(node->AsString(), value))
                SetAxesTickLocation2D(value);
        }
    }
    if((node = searchNode->GetNode("axesTicks2D")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetAxesTicks2D(Ticks(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Ticks value;
            if(Ticks_FromString(node->AsString(), value))
                SetAxesTicks2D(value);
        }
    }
    if((node = searchNode->GetNode("xAxisUserTitle2D")) != 0)
        SetXAxisUserTitle2D(node->AsString());
    if((node = searchNode->GetNode("yAxisUserTitle2D")) != 0)
        SetYAxisUserTitle2D(node->AsString());
    if((node = searchNode->GetNode("xAxisUserTitleFlag2D")) != 0)
        SetXAxisUserTitleFlag2D(node->AsBool());
    if((node = searchNode->GetNode("yAxisUserTitleFlag2D")) != 0)
        SetYAxisUserTitleFlag2D(node->AsBool());
    if((node = searchNode->GetNode("xAxisUserUnits2D")) != 0)
        SetXAxisUserUnits2D(node->AsString());
    if((node = searchNode->GetNode("yAxisUserUnits2D")) != 0)
        SetYAxisUserUnits2D(node->AsString());
    if((node = searchNode->GetNode("xAxisUserUnitsFlag2D")) != 0)
        SetXAxisUserUnitsFlag2D(node->AsBool());
    if((node = searchNode->GetNode("yAxisUserUnitsFlag2D")) != 0)
        SetYAxisUserUnitsFlag2D(node->AsBool());
    if((node = searchNode->GetNode("axesFlag")) != 0)
        SetAxesFlag(node->AsBool());
    if((node = searchNode->GetNode("axesAutoSetTicks")) != 0)
        SetAxesAutoSetTicks(node->AsBool());
    if((node = searchNode->GetNode("labelAutoSetScaling")) != 0)
        SetLabelAutoSetScaling(node->AsBool());
    if((node = searchNode->GetNode("xAxisLabels")) != 0)
        SetXAxisLabels(node->AsBool());
    if((node = searchNode->GetNode("yAxisLabels")) != 0)
        SetYAxisLabels(node->AsBool());
    if((node = searchNode->GetNode("zAxisLabels")) != 0)
        SetZAxisLabels(node->AsBool());
    if((node = searchNode->GetNode("xAxisTitle")) != 0)
        SetXAxisTitle(node->AsBool());
    if((node = searchNode->GetNode("yAxisTitle")) != 0)
        SetYAxisTitle(node->AsBool());
    if((node = searchNode->GetNode("zAxisTitle")) != 0)
        SetZAxisTitle(node->AsBool());
    if((node = searchNode->GetNode("xGridLines")) != 0)
        SetXGridLines(node->AsBool());
    if((node = searchNode->GetNode("yGridLines")) != 0)
        SetYGridLines(node->AsBool());
    if((node = searchNode->GetNode("zGridLines")) != 0)
        SetZGridLines(node->AsBool());
    if((node = searchNode->GetNode("xAxisTicks")) != 0)
        SetXAxisTicks(node->AsBool());
    if((node = searchNode->GetNode("yAxisTicks")) != 0)
        SetYAxisTicks(node->AsBool());
    if((node = searchNode->GetNode("zAxisTicks")) != 0)
        SetZAxisTicks(node->AsBool());
    if((node = searchNode->GetNode("xMajorTickMinimum")) != 0)
        SetXMajorTickMinimum(node->AsDouble());
    if((node = searchNode->GetNode("yMajorTickMinimum")) != 0)
        SetYMajorTickMinimum(node->AsDouble());
    if((node = searchNode->GetNode("zMajorTickMinimum")) != 0)
        SetZMajorTickMinimum(node->AsDouble());
    if((node = searchNode->GetNode("xMajorTickMaximum")) != 0)
        SetXMajorTickMaximum(node->AsDouble());
    if((node = searchNode->GetNode("yMajorTickMaximum")) != 0)
        SetYMajorTickMaximum(node->AsDouble());
    if((node = searchNode->GetNode("zMajorTickMaximum")) != 0)
        SetZMajorTickMaximum(node->AsDouble());
    if((node = searchNode->GetNode("xMajorTickSpacing")) != 0)
        SetXMajorTickSpacing(node->AsDouble());
    if((node = searchNode->GetNode("yMajorTickSpacing")) != 0)
        SetYMajorTickSpacing(node->AsDouble());
    if((node = searchNode->GetNode("zMajorTickSpacing")) != 0)
        SetZMajorTickSpacing(node->AsDouble());
    if((node = searchNode->GetNode("xMinorTickSpacing")) != 0)
        SetXMinorTickSpacing(node->AsDouble());
    if((node = searchNode->GetNode("yMinorTickSpacing")) != 0)
        SetYMinorTickSpacing(node->AsDouble());
    if((node = searchNode->GetNode("zMinorTickSpacing")) != 0)
        SetZMinorTickSpacing(node->AsDouble());
    if((node = searchNode->GetNode("xLabelFontHeight")) != 0)
        SetXLabelFontHeight(node->AsDouble());
    if((node = searchNode->GetNode("yLabelFontHeight")) != 0)
        SetYLabelFontHeight(node->AsDouble());
    if((node = searchNode->GetNode("zLabelFontHeight")) != 0)
        SetZLabelFontHeight(node->AsDouble());
    if((node = searchNode->GetNode("xTitleFontHeight")) != 0)
        SetXTitleFontHeight(node->AsDouble());
    if((node = searchNode->GetNode("yTitleFontHeight")) != 0)
        SetYTitleFontHeight(node->AsDouble());
    if((node = searchNode->GetNode("zTitleFontHeight")) != 0)
        SetZTitleFontHeight(node->AsDouble());
    if((node = searchNode->GetNode("xLabelScaling")) != 0)
        SetXLabelScaling(node->AsInt());
    if((node = searchNode->GetNode("yLabelScaling")) != 0)
        SetYLabelScaling(node->AsInt());
    if((node = searchNode->GetNode("zLabelScaling")) != 0)
        SetZLabelScaling(node->AsInt());
    if((node = searchNode->GetNode("xAxisUserTitle")) != 0)
        SetXAxisUserTitle(node->AsString());
    if((node = searchNode->GetNode("yAxisUserTitle")) != 0)
        SetYAxisUserTitle(node->AsString());
    if((node = searchNode->GetNode("zAxisUserTitle")) != 0)
        SetZAxisUserTitle(node->AsString());
    if((node = searchNode->GetNode("xAxisUserTitleFlag")) != 0)
        SetXAxisUserTitleFlag(node->AsBool());
    if((node = searchNode->GetNode("yAxisUserTitleFlag")) != 0)
        SetYAxisUserTitleFlag(node->AsBool());
    if((node = searchNode->GetNode("zAxisUserTitleFlag")) != 0)
        SetZAxisUserTitleFlag(node->AsBool());
    if((node = searchNode->GetNode("xAxisUserUnits")) != 0)
        SetXAxisUserUnits(node->AsString());
    if((node = searchNode->GetNode("yAxisUserUnits")) != 0)
        SetYAxisUserUnits(node->AsString());
    if((node = searchNode->GetNode("zAxisUserUnits")) != 0)
        SetZAxisUserUnits(node->AsString());
    if((node = searchNode->GetNode("xAxisUserUnitsFlag")) != 0)
        SetXAxisUserUnitsFlag(node->AsBool());
    if((node = searchNode->GetNode("yAxisUserUnitsFlag")) != 0)
        SetYAxisUserUnitsFlag(node->AsBool());
    if((node = searchNode->GetNode("zAxisUserUnitsFlag")) != 0)
        SetZAxisUserUnitsFlag(node->AsBool());
    if((node = searchNode->GetNode("axesTickLocation")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetAxesTickLocation(Location(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Location value;
            if(Location_FromString(node->AsString(), value))
                SetAxesTickLocation(value);
        }
    }
    if((node = searchNode->GetNode("axesType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetAxesType(Axes(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Axes value;
            if(Axes_FromString(node->AsString(), value))
                SetAxesType(value);
        }
    }
    if((node = searchNode->GetNode("triadFlag")) != 0)
        SetTriadFlag(node->AsBool());
    if((node = searchNode->GetNode("bboxFlag")) != 0)
        SetBboxFlag(node->AsBool());
    if((node = searchNode->GetNode("backgroundColor")) != 0)
        backgroundColor.SetFromNode(node);
    if((node = searchNode->GetNode("foregroundColor")) != 0)
        foregroundColor.SetFromNode(node);
    if((node = searchNode->GetNode("gradientBackgroundStyle")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetGradientBackgroundStyle(GradientStyle(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            GradientStyle value;
            if(GradientStyle_FromString(node->AsString(), value))
                SetGradientBackgroundStyle(value);
        }
    }
    if((node = searchNode->GetNode("gradientColor1")) != 0)
        gradientColor1.SetFromNode(node);
    if((node = searchNode->GetNode("gradientColor2")) != 0)
        gradientColor2.SetFromNode(node);
    if((node = searchNode->GetNode("backgroundMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetBackgroundMode(BackgroundMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BackgroundMode value;
            if(BackgroundMode_FromString(node->AsString(), value))
                SetBackgroundMode(value);
        }
    }
    if((node = searchNode->GetNode("backgroundImage")) != 0)
        SetBackgroundImage(node->AsString());
    if((node = searchNode->GetNode("userInfoFlag")) != 0)
        SetUserInfoFlag(node->AsBool());
    if((node = searchNode->GetNode("databaseInfoFlag")) != 0)
        SetDatabaseInfoFlag(node->AsBool());
    if((node = searchNode->GetNode("databaseInfoExpansionMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetDatabaseInfoExpansionMode(PathExpansionMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PathExpansionMode value;
            if(PathExpansionMode_FromString(node->AsString(), value))
                SetDatabaseInfoExpansionMode(value);
        }
    }
    if((node = searchNode->GetNode("legendInfoFlag")) != 0)
        SetLegendInfoFlag(node->AsBool());
    if((node = searchNode->GetNode("imageRepeatX")) != 0)
        SetImageRepeatX(node->AsInt());
    if((node = searchNode->GetNode("imageRepeatY")) != 0)
        SetImageRepeatY(node->AsInt());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
AnnotationAttributes::SetAxesFlag2D(bool axesFlag2D_)
{
    axesFlag2D = axesFlag2D_;
    Select(ID_axesFlag2D, (void *)&axesFlag2D);
}

void
AnnotationAttributes::SetAxesAutoSetTicks2D(bool axesAutoSetTicks2D_)
{
    axesAutoSetTicks2D = axesAutoSetTicks2D_;
    Select(ID_axesAutoSetTicks2D, (void *)&axesAutoSetTicks2D);
}

void
AnnotationAttributes::SetLabelAutoSetScaling2D(bool labelAutoSetScaling2D_)
{
    labelAutoSetScaling2D = labelAutoSetScaling2D_;
    Select(ID_labelAutoSetScaling2D, (void *)&labelAutoSetScaling2D);
}

void
AnnotationAttributes::SetXAxisLabels2D(bool xAxisLabels2D_)
{
    xAxisLabels2D = xAxisLabels2D_;
    Select(ID_xAxisLabels2D, (void *)&xAxisLabels2D);
}

void
AnnotationAttributes::SetYAxisLabels2D(bool yAxisLabels2D_)
{
    yAxisLabels2D = yAxisLabels2D_;
    Select(ID_yAxisLabels2D, (void *)&yAxisLabels2D);
}

void
AnnotationAttributes::SetXAxisTitle2D(bool xAxisTitle2D_)
{
    xAxisTitle2D = xAxisTitle2D_;
    Select(ID_xAxisTitle2D, (void *)&xAxisTitle2D);
}

void
AnnotationAttributes::SetYAxisTitle2D(bool yAxisTitle2D_)
{
    yAxisTitle2D = yAxisTitle2D_;
    Select(ID_yAxisTitle2D, (void *)&yAxisTitle2D);
}

void
AnnotationAttributes::SetXGridLines2D(bool xGridLines2D_)
{
    xGridLines2D = xGridLines2D_;
    Select(ID_xGridLines2D, (void *)&xGridLines2D);
}

void
AnnotationAttributes::SetYGridLines2D(bool yGridLines2D_)
{
    yGridLines2D = yGridLines2D_;
    Select(ID_yGridLines2D, (void *)&yGridLines2D);
}

void
AnnotationAttributes::SetXMajorTickMinimum2D(double xMajorTickMinimum2D_)
{
    xMajorTickMinimum2D = xMajorTickMinimum2D_;
    Select(ID_xMajorTickMinimum2D, (void *)&xMajorTickMinimum2D);
}

void
AnnotationAttributes::SetYMajorTickMinimum2D(double yMajorTickMinimum2D_)
{
    yMajorTickMinimum2D = yMajorTickMinimum2D_;
    Select(ID_yMajorTickMinimum2D, (void *)&yMajorTickMinimum2D);
}

void
AnnotationAttributes::SetXMajorTickMaximum2D(double xMajorTickMaximum2D_)
{
    xMajorTickMaximum2D = xMajorTickMaximum2D_;
    Select(ID_xMajorTickMaximum2D, (void *)&xMajorTickMaximum2D);
}

void
AnnotationAttributes::SetYMajorTickMaximum2D(double yMajorTickMaximum2D_)
{
    yMajorTickMaximum2D = yMajorTickMaximum2D_;
    Select(ID_yMajorTickMaximum2D, (void *)&yMajorTickMaximum2D);
}

void
AnnotationAttributes::SetXMajorTickSpacing2D(double xMajorTickSpacing2D_)
{
    xMajorTickSpacing2D = xMajorTickSpacing2D_;
    Select(ID_xMajorTickSpacing2D, (void *)&xMajorTickSpacing2D);
}

void
AnnotationAttributes::SetYMajorTickSpacing2D(double yMajorTickSpacing2D_)
{
    yMajorTickSpacing2D = yMajorTickSpacing2D_;
    Select(ID_yMajorTickSpacing2D, (void *)&yMajorTickSpacing2D);
}

void
AnnotationAttributes::SetXMinorTickSpacing2D(double xMinorTickSpacing2D_)
{
    xMinorTickSpacing2D = xMinorTickSpacing2D_;
    Select(ID_xMinorTickSpacing2D, (void *)&xMinorTickSpacing2D);
}

void
AnnotationAttributes::SetYMinorTickSpacing2D(double yMinorTickSpacing2D_)
{
    yMinorTickSpacing2D = yMinorTickSpacing2D_;
    Select(ID_yMinorTickSpacing2D, (void *)&yMinorTickSpacing2D);
}

void
AnnotationAttributes::SetXLabelFontHeight2D(double xLabelFontHeight2D_)
{
    xLabelFontHeight2D = xLabelFontHeight2D_;
    Select(ID_xLabelFontHeight2D, (void *)&xLabelFontHeight2D);
}

void
AnnotationAttributes::SetYLabelFontHeight2D(double yLabelFontHeight2D_)
{
    yLabelFontHeight2D = yLabelFontHeight2D_;
    Select(ID_yLabelFontHeight2D, (void *)&yLabelFontHeight2D);
}

void
AnnotationAttributes::SetXTitleFontHeight2D(double xTitleFontHeight2D_)
{
    xTitleFontHeight2D = xTitleFontHeight2D_;
    Select(ID_xTitleFontHeight2D, (void *)&xTitleFontHeight2D);
}

void
AnnotationAttributes::SetYTitleFontHeight2D(double yTitleFontHeight2D_)
{
    yTitleFontHeight2D = yTitleFontHeight2D_;
    Select(ID_yTitleFontHeight2D, (void *)&yTitleFontHeight2D);
}

void
AnnotationAttributes::SetXLabelScaling2D(int xLabelScaling2D_)
{
    xLabelScaling2D = xLabelScaling2D_;
    Select(ID_xLabelScaling2D, (void *)&xLabelScaling2D);
}

void
AnnotationAttributes::SetYLabelScaling2D(int yLabelScaling2D_)
{
    yLabelScaling2D = yLabelScaling2D_;
    Select(ID_yLabelScaling2D, (void *)&yLabelScaling2D);
}

void
AnnotationAttributes::SetAxesLineWidth2D(int axesLineWidth2D_)
{
    axesLineWidth2D = axesLineWidth2D_;
    Select(ID_axesLineWidth2D, (void *)&axesLineWidth2D);
}

void
AnnotationAttributes::SetAxesTickLocation2D(AnnotationAttributes::Location axesTickLocation2D_)
{
    axesTickLocation2D = axesTickLocation2D_;
    Select(ID_axesTickLocation2D, (void *)&axesTickLocation2D);
}

void
AnnotationAttributes::SetAxesTicks2D(AnnotationAttributes::Ticks axesTicks2D_)
{
    axesTicks2D = axesTicks2D_;
    Select(ID_axesTicks2D, (void *)&axesTicks2D);
}

void
AnnotationAttributes::SetXAxisUserTitle2D(const std::string &xAxisUserTitle2D_)
{
    xAxisUserTitle2D = xAxisUserTitle2D_;
    Select(ID_xAxisUserTitle2D, (void *)&xAxisUserTitle2D);
}

void
AnnotationAttributes::SetYAxisUserTitle2D(const std::string &yAxisUserTitle2D_)
{
    yAxisUserTitle2D = yAxisUserTitle2D_;
    Select(ID_yAxisUserTitle2D, (void *)&yAxisUserTitle2D);
}

void
AnnotationAttributes::SetXAxisUserTitleFlag2D(bool xAxisUserTitleFlag2D_)
{
    xAxisUserTitleFlag2D = xAxisUserTitleFlag2D_;
    Select(ID_xAxisUserTitleFlag2D, (void *)&xAxisUserTitleFlag2D);
}

void
AnnotationAttributes::SetYAxisUserTitleFlag2D(bool yAxisUserTitleFlag2D_)
{
    yAxisUserTitleFlag2D = yAxisUserTitleFlag2D_;
    Select(ID_yAxisUserTitleFlag2D, (void *)&yAxisUserTitleFlag2D);
}

void
AnnotationAttributes::SetXAxisUserUnits2D(const std::string &xAxisUserUnits2D_)
{
    xAxisUserUnits2D = xAxisUserUnits2D_;
    Select(ID_xAxisUserUnits2D, (void *)&xAxisUserUnits2D);
}

void
AnnotationAttributes::SetYAxisUserUnits2D(const std::string &yAxisUserUnits2D_)
{
    yAxisUserUnits2D = yAxisUserUnits2D_;
    Select(ID_yAxisUserUnits2D, (void *)&yAxisUserUnits2D);
}

void
AnnotationAttributes::SetXAxisUserUnitsFlag2D(bool xAxisUserUnitsFlag2D_)
{
    xAxisUserUnitsFlag2D = xAxisUserUnitsFlag2D_;
    Select(ID_xAxisUserUnitsFlag2D, (void *)&xAxisUserUnitsFlag2D);
}

void
AnnotationAttributes::SetYAxisUserUnitsFlag2D(bool yAxisUserUnitsFlag2D_)
{
    yAxisUserUnitsFlag2D = yAxisUserUnitsFlag2D_;
    Select(ID_yAxisUserUnitsFlag2D, (void *)&yAxisUserUnitsFlag2D);
}

void
AnnotationAttributes::SetAxesFlag(bool axesFlag_)
{
    axesFlag = axesFlag_;
    Select(ID_axesFlag, (void *)&axesFlag);
}

void
AnnotationAttributes::SetAxesAutoSetTicks(bool axesAutoSetTicks_)
{
    axesAutoSetTicks = axesAutoSetTicks_;
    Select(ID_axesAutoSetTicks, (void *)&axesAutoSetTicks);
}

void
AnnotationAttributes::SetLabelAutoSetScaling(bool labelAutoSetScaling_)
{
    labelAutoSetScaling = labelAutoSetScaling_;
    Select(ID_labelAutoSetScaling, (void *)&labelAutoSetScaling);
}

void
AnnotationAttributes::SetXAxisLabels(bool xAxisLabels_)
{
    xAxisLabels = xAxisLabels_;
    Select(ID_xAxisLabels, (void *)&xAxisLabels);
}

void
AnnotationAttributes::SetYAxisLabels(bool yAxisLabels_)
{
    yAxisLabels = yAxisLabels_;
    Select(ID_yAxisLabels, (void *)&yAxisLabels);
}

void
AnnotationAttributes::SetZAxisLabels(bool zAxisLabels_)
{
    zAxisLabels = zAxisLabels_;
    Select(ID_zAxisLabels, (void *)&zAxisLabels);
}

void
AnnotationAttributes::SetXAxisTitle(bool xAxisTitle_)
{
    xAxisTitle = xAxisTitle_;
    Select(ID_xAxisTitle, (void *)&xAxisTitle);
}

void
AnnotationAttributes::SetYAxisTitle(bool yAxisTitle_)
{
    yAxisTitle = yAxisTitle_;
    Select(ID_yAxisTitle, (void *)&yAxisTitle);
}

void
AnnotationAttributes::SetZAxisTitle(bool zAxisTitle_)
{
    zAxisTitle = zAxisTitle_;
    Select(ID_zAxisTitle, (void *)&zAxisTitle);
}

void
AnnotationAttributes::SetXGridLines(bool xGridLines_)
{
    xGridLines = xGridLines_;
    Select(ID_xGridLines, (void *)&xGridLines);
}

void
AnnotationAttributes::SetYGridLines(bool yGridLines_)
{
    yGridLines = yGridLines_;
    Select(ID_yGridLines, (void *)&yGridLines);
}

void
AnnotationAttributes::SetZGridLines(bool zGridLines_)
{
    zGridLines = zGridLines_;
    Select(ID_zGridLines, (void *)&zGridLines);
}

void
AnnotationAttributes::SetXAxisTicks(bool xAxisTicks_)
{
    xAxisTicks = xAxisTicks_;
    Select(ID_xAxisTicks, (void *)&xAxisTicks);
}

void
AnnotationAttributes::SetYAxisTicks(bool yAxisTicks_)
{
    yAxisTicks = yAxisTicks_;
    Select(ID_yAxisTicks, (void *)&yAxisTicks);
}

void
AnnotationAttributes::SetZAxisTicks(bool zAxisTicks_)
{
    zAxisTicks = zAxisTicks_;
    Select(ID_zAxisTicks, (void *)&zAxisTicks);
}

void
AnnotationAttributes::SetXMajorTickMinimum(double xMajorTickMinimum_)
{
    xMajorTickMinimum = xMajorTickMinimum_;
    Select(ID_xMajorTickMinimum, (void *)&xMajorTickMinimum);
}

void
AnnotationAttributes::SetYMajorTickMinimum(double yMajorTickMinimum_)
{
    yMajorTickMinimum = yMajorTickMinimum_;
    Select(ID_yMajorTickMinimum, (void *)&yMajorTickMinimum);
}

void
AnnotationAttributes::SetZMajorTickMinimum(double zMajorTickMinimum_)
{
    zMajorTickMinimum = zMajorTickMinimum_;
    Select(ID_zMajorTickMinimum, (void *)&zMajorTickMinimum);
}

void
AnnotationAttributes::SetXMajorTickMaximum(double xMajorTickMaximum_)
{
    xMajorTickMaximum = xMajorTickMaximum_;
    Select(ID_xMajorTickMaximum, (void *)&xMajorTickMaximum);
}

void
AnnotationAttributes::SetYMajorTickMaximum(double yMajorTickMaximum_)
{
    yMajorTickMaximum = yMajorTickMaximum_;
    Select(ID_yMajorTickMaximum, (void *)&yMajorTickMaximum);
}

void
AnnotationAttributes::SetZMajorTickMaximum(double zMajorTickMaximum_)
{
    zMajorTickMaximum = zMajorTickMaximum_;
    Select(ID_zMajorTickMaximum, (void *)&zMajorTickMaximum);
}

void
AnnotationAttributes::SetXMajorTickSpacing(double xMajorTickSpacing_)
{
    xMajorTickSpacing = xMajorTickSpacing_;
    Select(ID_xMajorTickSpacing, (void *)&xMajorTickSpacing);
}

void
AnnotationAttributes::SetYMajorTickSpacing(double yMajorTickSpacing_)
{
    yMajorTickSpacing = yMajorTickSpacing_;
    Select(ID_yMajorTickSpacing, (void *)&yMajorTickSpacing);
}

void
AnnotationAttributes::SetZMajorTickSpacing(double zMajorTickSpacing_)
{
    zMajorTickSpacing = zMajorTickSpacing_;
    Select(ID_zMajorTickSpacing, (void *)&zMajorTickSpacing);
}

void
AnnotationAttributes::SetXMinorTickSpacing(double xMinorTickSpacing_)
{
    xMinorTickSpacing = xMinorTickSpacing_;
    Select(ID_xMinorTickSpacing, (void *)&xMinorTickSpacing);
}

void
AnnotationAttributes::SetYMinorTickSpacing(double yMinorTickSpacing_)
{
    yMinorTickSpacing = yMinorTickSpacing_;
    Select(ID_yMinorTickSpacing, (void *)&yMinorTickSpacing);
}

void
AnnotationAttributes::SetZMinorTickSpacing(double zMinorTickSpacing_)
{
    zMinorTickSpacing = zMinorTickSpacing_;
    Select(ID_zMinorTickSpacing, (void *)&zMinorTickSpacing);
}

void
AnnotationAttributes::SetXLabelFontHeight(double xLabelFontHeight_)
{
    xLabelFontHeight = xLabelFontHeight_;
    Select(ID_xLabelFontHeight, (void *)&xLabelFontHeight);
}

void
AnnotationAttributes::SetYLabelFontHeight(double yLabelFontHeight_)
{
    yLabelFontHeight = yLabelFontHeight_;
    Select(ID_yLabelFontHeight, (void *)&yLabelFontHeight);
}

void
AnnotationAttributes::SetZLabelFontHeight(double zLabelFontHeight_)
{
    zLabelFontHeight = zLabelFontHeight_;
    Select(ID_zLabelFontHeight, (void *)&zLabelFontHeight);
}

void
AnnotationAttributes::SetXTitleFontHeight(double xTitleFontHeight_)
{
    xTitleFontHeight = xTitleFontHeight_;
    Select(ID_xTitleFontHeight, (void *)&xTitleFontHeight);
}

void
AnnotationAttributes::SetYTitleFontHeight(double yTitleFontHeight_)
{
    yTitleFontHeight = yTitleFontHeight_;
    Select(ID_yTitleFontHeight, (void *)&yTitleFontHeight);
}

void
AnnotationAttributes::SetZTitleFontHeight(double zTitleFontHeight_)
{
    zTitleFontHeight = zTitleFontHeight_;
    Select(ID_zTitleFontHeight, (void *)&zTitleFontHeight);
}

void
AnnotationAttributes::SetXLabelScaling(int xLabelScaling_)
{
    xLabelScaling = xLabelScaling_;
    Select(ID_xLabelScaling, (void *)&xLabelScaling);
}

void
AnnotationAttributes::SetYLabelScaling(int yLabelScaling_)
{
    yLabelScaling = yLabelScaling_;
    Select(ID_yLabelScaling, (void *)&yLabelScaling);
}

void
AnnotationAttributes::SetZLabelScaling(int zLabelScaling_)
{
    zLabelScaling = zLabelScaling_;
    Select(ID_zLabelScaling, (void *)&zLabelScaling);
}

void
AnnotationAttributes::SetXAxisUserTitle(const std::string &xAxisUserTitle_)
{
    xAxisUserTitle = xAxisUserTitle_;
    Select(ID_xAxisUserTitle, (void *)&xAxisUserTitle);
}

void
AnnotationAttributes::SetYAxisUserTitle(const std::string &yAxisUserTitle_)
{
    yAxisUserTitle = yAxisUserTitle_;
    Select(ID_yAxisUserTitle, (void *)&yAxisUserTitle);
}

void
AnnotationAttributes::SetZAxisUserTitle(const std::string &zAxisUserTitle_)
{
    zAxisUserTitle = zAxisUserTitle_;
    Select(ID_zAxisUserTitle, (void *)&zAxisUserTitle);
}

void
AnnotationAttributes::SetXAxisUserTitleFlag(bool xAxisUserTitleFlag_)
{
    xAxisUserTitleFlag = xAxisUserTitleFlag_;
    Select(ID_xAxisUserTitleFlag, (void *)&xAxisUserTitleFlag);
}

void
AnnotationAttributes::SetYAxisUserTitleFlag(bool yAxisUserTitleFlag_)
{
    yAxisUserTitleFlag = yAxisUserTitleFlag_;
    Select(ID_yAxisUserTitleFlag, (void *)&yAxisUserTitleFlag);
}

void
AnnotationAttributes::SetZAxisUserTitleFlag(bool zAxisUserTitleFlag_)
{
    zAxisUserTitleFlag = zAxisUserTitleFlag_;
    Select(ID_zAxisUserTitleFlag, (void *)&zAxisUserTitleFlag);
}

void
AnnotationAttributes::SetXAxisUserUnits(const std::string &xAxisUserUnits_)
{
    xAxisUserUnits = xAxisUserUnits_;
    Select(ID_xAxisUserUnits, (void *)&xAxisUserUnits);
}

void
AnnotationAttributes::SetYAxisUserUnits(const std::string &yAxisUserUnits_)
{
    yAxisUserUnits = yAxisUserUnits_;
    Select(ID_yAxisUserUnits, (void *)&yAxisUserUnits);
}

void
AnnotationAttributes::SetZAxisUserUnits(const std::string &zAxisUserUnits_)
{
    zAxisUserUnits = zAxisUserUnits_;
    Select(ID_zAxisUserUnits, (void *)&zAxisUserUnits);
}

void
AnnotationAttributes::SetXAxisUserUnitsFlag(bool xAxisUserUnitsFlag_)
{
    xAxisUserUnitsFlag = xAxisUserUnitsFlag_;
    Select(ID_xAxisUserUnitsFlag, (void *)&xAxisUserUnitsFlag);
}

void
AnnotationAttributes::SetYAxisUserUnitsFlag(bool yAxisUserUnitsFlag_)
{
    yAxisUserUnitsFlag = yAxisUserUnitsFlag_;
    Select(ID_yAxisUserUnitsFlag, (void *)&yAxisUserUnitsFlag);
}

void
AnnotationAttributes::SetZAxisUserUnitsFlag(bool zAxisUserUnitsFlag_)
{
    zAxisUserUnitsFlag = zAxisUserUnitsFlag_;
    Select(ID_zAxisUserUnitsFlag, (void *)&zAxisUserUnitsFlag);
}

void
AnnotationAttributes::SetAxesTickLocation(AnnotationAttributes::Location axesTickLocation_)
{
    axesTickLocation = axesTickLocation_;
    Select(ID_axesTickLocation, (void *)&axesTickLocation);
}

void
AnnotationAttributes::SetAxesType(AnnotationAttributes::Axes axesType_)
{
    axesType = axesType_;
    Select(ID_axesType, (void *)&axesType);
}

void
AnnotationAttributes::SetTriadFlag(bool triadFlag_)
{
    triadFlag = triadFlag_;
    Select(ID_triadFlag, (void *)&triadFlag);
}

void
AnnotationAttributes::SetBboxFlag(bool bboxFlag_)
{
    bboxFlag = bboxFlag_;
    Select(ID_bboxFlag, (void *)&bboxFlag);
}

void
AnnotationAttributes::SetBackgroundColor(const ColorAttribute &backgroundColor_)
{
    backgroundColor = backgroundColor_;
    Select(ID_backgroundColor, (void *)&backgroundColor);
}

void
AnnotationAttributes::SetForegroundColor(const ColorAttribute &foregroundColor_)
{
    foregroundColor = foregroundColor_;
    Select(ID_foregroundColor, (void *)&foregroundColor);
}

void
AnnotationAttributes::SetGradientBackgroundStyle(AnnotationAttributes::GradientStyle gradientBackgroundStyle_)
{
    gradientBackgroundStyle = gradientBackgroundStyle_;
    Select(ID_gradientBackgroundStyle, (void *)&gradientBackgroundStyle);
}

void
AnnotationAttributes::SetGradientColor1(const ColorAttribute &gradientColor1_)
{
    gradientColor1 = gradientColor1_;
    Select(ID_gradientColor1, (void *)&gradientColor1);
}

void
AnnotationAttributes::SetGradientColor2(const ColorAttribute &gradientColor2_)
{
    gradientColor2 = gradientColor2_;
    Select(ID_gradientColor2, (void *)&gradientColor2);
}

void
AnnotationAttributes::SetBackgroundMode(AnnotationAttributes::BackgroundMode backgroundMode_)
{
    backgroundMode = backgroundMode_;
    Select(ID_backgroundMode, (void *)&backgroundMode);
}

void
AnnotationAttributes::SetBackgroundImage(const std::string &backgroundImage_)
{
    backgroundImage = backgroundImage_;
    Select(ID_backgroundImage, (void *)&backgroundImage);
}

void
AnnotationAttributes::SetUserInfoFlag(bool userInfoFlag_)
{
    userInfoFlag = userInfoFlag_;
    Select(ID_userInfoFlag, (void *)&userInfoFlag);
}

void
AnnotationAttributes::SetDatabaseInfoFlag(bool databaseInfoFlag_)
{
    databaseInfoFlag = databaseInfoFlag_;
    Select(ID_databaseInfoFlag, (void *)&databaseInfoFlag);
}

void
AnnotationAttributes::SetDatabaseInfoExpansionMode(AnnotationAttributes::PathExpansionMode databaseInfoExpansionMode_)
{
    databaseInfoExpansionMode = databaseInfoExpansionMode_;
    Select(ID_databaseInfoExpansionMode, (void *)&databaseInfoExpansionMode);
}

void
AnnotationAttributes::SetLegendInfoFlag(bool legendInfoFlag_)
{
    legendInfoFlag = legendInfoFlag_;
    Select(ID_legendInfoFlag, (void *)&legendInfoFlag);
}

void
AnnotationAttributes::SetImageRepeatX(int imageRepeatX_)
{
    imageRepeatX = imageRepeatX_;
    Select(ID_imageRepeatX, (void *)&imageRepeatX);
}

void
AnnotationAttributes::SetImageRepeatY(int imageRepeatY_)
{
    imageRepeatY = imageRepeatY_;
    Select(ID_imageRepeatY, (void *)&imageRepeatY);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
AnnotationAttributes::GetAxesFlag2D() const
{
    return axesFlag2D;
}

bool
AnnotationAttributes::GetAxesAutoSetTicks2D() const
{
    return axesAutoSetTicks2D;
}

bool
AnnotationAttributes::GetLabelAutoSetScaling2D() const
{
    return labelAutoSetScaling2D;
}

bool
AnnotationAttributes::GetXAxisLabels2D() const
{
    return xAxisLabels2D;
}

bool
AnnotationAttributes::GetYAxisLabels2D() const
{
    return yAxisLabels2D;
}

bool
AnnotationAttributes::GetXAxisTitle2D() const
{
    return xAxisTitle2D;
}

bool
AnnotationAttributes::GetYAxisTitle2D() const
{
    return yAxisTitle2D;
}

bool
AnnotationAttributes::GetXGridLines2D() const
{
    return xGridLines2D;
}

bool
AnnotationAttributes::GetYGridLines2D() const
{
    return yGridLines2D;
}

double
AnnotationAttributes::GetXMajorTickMinimum2D() const
{
    return xMajorTickMinimum2D;
}

double
AnnotationAttributes::GetYMajorTickMinimum2D() const
{
    return yMajorTickMinimum2D;
}

double
AnnotationAttributes::GetXMajorTickMaximum2D() const
{
    return xMajorTickMaximum2D;
}

double
AnnotationAttributes::GetYMajorTickMaximum2D() const
{
    return yMajorTickMaximum2D;
}

double
AnnotationAttributes::GetXMajorTickSpacing2D() const
{
    return xMajorTickSpacing2D;
}

double
AnnotationAttributes::GetYMajorTickSpacing2D() const
{
    return yMajorTickSpacing2D;
}

double
AnnotationAttributes::GetXMinorTickSpacing2D() const
{
    return xMinorTickSpacing2D;
}

double
AnnotationAttributes::GetYMinorTickSpacing2D() const
{
    return yMinorTickSpacing2D;
}

double
AnnotationAttributes::GetXLabelFontHeight2D() const
{
    return xLabelFontHeight2D;
}

double
AnnotationAttributes::GetYLabelFontHeight2D() const
{
    return yLabelFontHeight2D;
}

double
AnnotationAttributes::GetXTitleFontHeight2D() const
{
    return xTitleFontHeight2D;
}

double
AnnotationAttributes::GetYTitleFontHeight2D() const
{
    return yTitleFontHeight2D;
}

int
AnnotationAttributes::GetXLabelScaling2D() const
{
    return xLabelScaling2D;
}

int
AnnotationAttributes::GetYLabelScaling2D() const
{
    return yLabelScaling2D;
}

int
AnnotationAttributes::GetAxesLineWidth2D() const
{
    return axesLineWidth2D;
}

AnnotationAttributes::Location
AnnotationAttributes::GetAxesTickLocation2D() const
{
    return Location(axesTickLocation2D);
}

AnnotationAttributes::Ticks
AnnotationAttributes::GetAxesTicks2D() const
{
    return Ticks(axesTicks2D);
}

const std::string &
AnnotationAttributes::GetXAxisUserTitle2D() const
{
    return xAxisUserTitle2D;
}

std::string &
AnnotationAttributes::GetXAxisUserTitle2D()
{
    return xAxisUserTitle2D;
}

const std::string &
AnnotationAttributes::GetYAxisUserTitle2D() const
{
    return yAxisUserTitle2D;
}

std::string &
AnnotationAttributes::GetYAxisUserTitle2D()
{
    return yAxisUserTitle2D;
}

bool
AnnotationAttributes::GetXAxisUserTitleFlag2D() const
{
    return xAxisUserTitleFlag2D;
}

bool
AnnotationAttributes::GetYAxisUserTitleFlag2D() const
{
    return yAxisUserTitleFlag2D;
}

const std::string &
AnnotationAttributes::GetXAxisUserUnits2D() const
{
    return xAxisUserUnits2D;
}

std::string &
AnnotationAttributes::GetXAxisUserUnits2D()
{
    return xAxisUserUnits2D;
}

const std::string &
AnnotationAttributes::GetYAxisUserUnits2D() const
{
    return yAxisUserUnits2D;
}

std::string &
AnnotationAttributes::GetYAxisUserUnits2D()
{
    return yAxisUserUnits2D;
}

bool
AnnotationAttributes::GetXAxisUserUnitsFlag2D() const
{
    return xAxisUserUnitsFlag2D;
}

bool
AnnotationAttributes::GetYAxisUserUnitsFlag2D() const
{
    return yAxisUserUnitsFlag2D;
}

bool
AnnotationAttributes::GetAxesFlag() const
{
    return axesFlag;
}

bool
AnnotationAttributes::GetAxesAutoSetTicks() const
{
    return axesAutoSetTicks;
}

bool
AnnotationAttributes::GetLabelAutoSetScaling() const
{
    return labelAutoSetScaling;
}

bool
AnnotationAttributes::GetXAxisLabels() const
{
    return xAxisLabels;
}

bool
AnnotationAttributes::GetYAxisLabels() const
{
    return yAxisLabels;
}

bool
AnnotationAttributes::GetZAxisLabels() const
{
    return zAxisLabels;
}

bool
AnnotationAttributes::GetXAxisTitle() const
{
    return xAxisTitle;
}

bool
AnnotationAttributes::GetYAxisTitle() const
{
    return yAxisTitle;
}

bool
AnnotationAttributes::GetZAxisTitle() const
{
    return zAxisTitle;
}

bool
AnnotationAttributes::GetXGridLines() const
{
    return xGridLines;
}

bool
AnnotationAttributes::GetYGridLines() const
{
    return yGridLines;
}

bool
AnnotationAttributes::GetZGridLines() const
{
    return zGridLines;
}

bool
AnnotationAttributes::GetXAxisTicks() const
{
    return xAxisTicks;
}

bool
AnnotationAttributes::GetYAxisTicks() const
{
    return yAxisTicks;
}

bool
AnnotationAttributes::GetZAxisTicks() const
{
    return zAxisTicks;
}

double
AnnotationAttributes::GetXMajorTickMinimum() const
{
    return xMajorTickMinimum;
}

double
AnnotationAttributes::GetYMajorTickMinimum() const
{
    return yMajorTickMinimum;
}

double
AnnotationAttributes::GetZMajorTickMinimum() const
{
    return zMajorTickMinimum;
}

double
AnnotationAttributes::GetXMajorTickMaximum() const
{
    return xMajorTickMaximum;
}

double
AnnotationAttributes::GetYMajorTickMaximum() const
{
    return yMajorTickMaximum;
}

double
AnnotationAttributes::GetZMajorTickMaximum() const
{
    return zMajorTickMaximum;
}

double
AnnotationAttributes::GetXMajorTickSpacing() const
{
    return xMajorTickSpacing;
}

double
AnnotationAttributes::GetYMajorTickSpacing() const
{
    return yMajorTickSpacing;
}

double
AnnotationAttributes::GetZMajorTickSpacing() const
{
    return zMajorTickSpacing;
}

double
AnnotationAttributes::GetXMinorTickSpacing() const
{
    return xMinorTickSpacing;
}

double
AnnotationAttributes::GetYMinorTickSpacing() const
{
    return yMinorTickSpacing;
}

double
AnnotationAttributes::GetZMinorTickSpacing() const
{
    return zMinorTickSpacing;
}

double
AnnotationAttributes::GetXLabelFontHeight() const
{
    return xLabelFontHeight;
}

double
AnnotationAttributes::GetYLabelFontHeight() const
{
    return yLabelFontHeight;
}

double
AnnotationAttributes::GetZLabelFontHeight() const
{
    return zLabelFontHeight;
}

double
AnnotationAttributes::GetXTitleFontHeight() const
{
    return xTitleFontHeight;
}

double
AnnotationAttributes::GetYTitleFontHeight() const
{
    return yTitleFontHeight;
}

double
AnnotationAttributes::GetZTitleFontHeight() const
{
    return zTitleFontHeight;
}

int
AnnotationAttributes::GetXLabelScaling() const
{
    return xLabelScaling;
}

int
AnnotationAttributes::GetYLabelScaling() const
{
    return yLabelScaling;
}

int
AnnotationAttributes::GetZLabelScaling() const
{
    return zLabelScaling;
}

const std::string &
AnnotationAttributes::GetXAxisUserTitle() const
{
    return xAxisUserTitle;
}

std::string &
AnnotationAttributes::GetXAxisUserTitle()
{
    return xAxisUserTitle;
}

const std::string &
AnnotationAttributes::GetYAxisUserTitle() const
{
    return yAxisUserTitle;
}

std::string &
AnnotationAttributes::GetYAxisUserTitle()
{
    return yAxisUserTitle;
}

const std::string &
AnnotationAttributes::GetZAxisUserTitle() const
{
    return zAxisUserTitle;
}

std::string &
AnnotationAttributes::GetZAxisUserTitle()
{
    return zAxisUserTitle;
}

bool
AnnotationAttributes::GetXAxisUserTitleFlag() const
{
    return xAxisUserTitleFlag;
}

bool
AnnotationAttributes::GetYAxisUserTitleFlag() const
{
    return yAxisUserTitleFlag;
}

bool
AnnotationAttributes::GetZAxisUserTitleFlag() const
{
    return zAxisUserTitleFlag;
}

const std::string &
AnnotationAttributes::GetXAxisUserUnits() const
{
    return xAxisUserUnits;
}

std::string &
AnnotationAttributes::GetXAxisUserUnits()
{
    return xAxisUserUnits;
}

const std::string &
AnnotationAttributes::GetYAxisUserUnits() const
{
    return yAxisUserUnits;
}

std::string &
AnnotationAttributes::GetYAxisUserUnits()
{
    return yAxisUserUnits;
}

const std::string &
AnnotationAttributes::GetZAxisUserUnits() const
{
    return zAxisUserUnits;
}

std::string &
AnnotationAttributes::GetZAxisUserUnits()
{
    return zAxisUserUnits;
}

bool
AnnotationAttributes::GetXAxisUserUnitsFlag() const
{
    return xAxisUserUnitsFlag;
}

bool
AnnotationAttributes::GetYAxisUserUnitsFlag() const
{
    return yAxisUserUnitsFlag;
}

bool
AnnotationAttributes::GetZAxisUserUnitsFlag() const
{
    return zAxisUserUnitsFlag;
}

AnnotationAttributes::Location
AnnotationAttributes::GetAxesTickLocation() const
{
    return Location(axesTickLocation);
}

AnnotationAttributes::Axes
AnnotationAttributes::GetAxesType() const
{
    return Axes(axesType);
}

bool
AnnotationAttributes::GetTriadFlag() const
{
    return triadFlag;
}

bool
AnnotationAttributes::GetBboxFlag() const
{
    return bboxFlag;
}

const ColorAttribute &
AnnotationAttributes::GetBackgroundColor() const
{
    return backgroundColor;
}

ColorAttribute &
AnnotationAttributes::GetBackgroundColor()
{
    return backgroundColor;
}

const ColorAttribute &
AnnotationAttributes::GetForegroundColor() const
{
    return foregroundColor;
}

ColorAttribute &
AnnotationAttributes::GetForegroundColor()
{
    return foregroundColor;
}

AnnotationAttributes::GradientStyle
AnnotationAttributes::GetGradientBackgroundStyle() const
{
    return GradientStyle(gradientBackgroundStyle);
}

const ColorAttribute &
AnnotationAttributes::GetGradientColor1() const
{
    return gradientColor1;
}

ColorAttribute &
AnnotationAttributes::GetGradientColor1()
{
    return gradientColor1;
}

const ColorAttribute &
AnnotationAttributes::GetGradientColor2() const
{
    return gradientColor2;
}

ColorAttribute &
AnnotationAttributes::GetGradientColor2()
{
    return gradientColor2;
}

AnnotationAttributes::BackgroundMode
AnnotationAttributes::GetBackgroundMode() const
{
    return BackgroundMode(backgroundMode);
}

const std::string &
AnnotationAttributes::GetBackgroundImage() const
{
    return backgroundImage;
}

std::string &
AnnotationAttributes::GetBackgroundImage()
{
    return backgroundImage;
}

bool
AnnotationAttributes::GetUserInfoFlag() const
{
    return userInfoFlag;
}

bool
AnnotationAttributes::GetDatabaseInfoFlag() const
{
    return databaseInfoFlag;
}

AnnotationAttributes::PathExpansionMode
AnnotationAttributes::GetDatabaseInfoExpansionMode() const
{
    return PathExpansionMode(databaseInfoExpansionMode);
}

bool
AnnotationAttributes::GetLegendInfoFlag() const
{
    return legendInfoFlag;
}

int
AnnotationAttributes::GetImageRepeatX() const
{
    return imageRepeatX;
}

int
AnnotationAttributes::GetImageRepeatY() const
{
    return imageRepeatY;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
AnnotationAttributes::SelectXAxisUserTitle2D()
{
    Select(ID_xAxisUserTitle2D, (void *)&xAxisUserTitle2D);
}

void
AnnotationAttributes::SelectYAxisUserTitle2D()
{
    Select(ID_yAxisUserTitle2D, (void *)&yAxisUserTitle2D);
}

void
AnnotationAttributes::SelectXAxisUserUnits2D()
{
    Select(ID_xAxisUserUnits2D, (void *)&xAxisUserUnits2D);
}

void
AnnotationAttributes::SelectYAxisUserUnits2D()
{
    Select(ID_yAxisUserUnits2D, (void *)&yAxisUserUnits2D);
}

void
AnnotationAttributes::SelectXAxisUserTitle()
{
    Select(ID_xAxisUserTitle, (void *)&xAxisUserTitle);
}

void
AnnotationAttributes::SelectYAxisUserTitle()
{
    Select(ID_yAxisUserTitle, (void *)&yAxisUserTitle);
}

void
AnnotationAttributes::SelectZAxisUserTitle()
{
    Select(ID_zAxisUserTitle, (void *)&zAxisUserTitle);
}

void
AnnotationAttributes::SelectXAxisUserUnits()
{
    Select(ID_xAxisUserUnits, (void *)&xAxisUserUnits);
}

void
AnnotationAttributes::SelectYAxisUserUnits()
{
    Select(ID_yAxisUserUnits, (void *)&yAxisUserUnits);
}

void
AnnotationAttributes::SelectZAxisUserUnits()
{
    Select(ID_zAxisUserUnits, (void *)&zAxisUserUnits);
}

void
AnnotationAttributes::SelectBackgroundColor()
{
    Select(ID_backgroundColor, (void *)&backgroundColor);
}

void
AnnotationAttributes::SelectForegroundColor()
{
    Select(ID_foregroundColor, (void *)&foregroundColor);
}

void
AnnotationAttributes::SelectGradientColor1()
{
    Select(ID_gradientColor1, (void *)&gradientColor1);
}

void
AnnotationAttributes::SelectGradientColor2()
{
    Select(ID_gradientColor2, (void *)&gradientColor2);
}

void
AnnotationAttributes::SelectBackgroundImage()
{
    Select(ID_backgroundImage, (void *)&backgroundImage);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AnnotationAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
AnnotationAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_axesFlag2D:                return "axesFlag2D";
    case ID_axesAutoSetTicks2D:        return "axesAutoSetTicks2D";
    case ID_labelAutoSetScaling2D:     return "labelAutoSetScaling2D";
    case ID_xAxisLabels2D:             return "xAxisLabels2D";
    case ID_yAxisLabels2D:             return "yAxisLabels2D";
    case ID_xAxisTitle2D:              return "xAxisTitle2D";
    case ID_yAxisTitle2D:              return "yAxisTitle2D";
    case ID_xGridLines2D:              return "xGridLines2D";
    case ID_yGridLines2D:              return "yGridLines2D";
    case ID_xMajorTickMinimum2D:       return "xMajorTickMinimum2D";
    case ID_yMajorTickMinimum2D:       return "yMajorTickMinimum2D";
    case ID_xMajorTickMaximum2D:       return "xMajorTickMaximum2D";
    case ID_yMajorTickMaximum2D:       return "yMajorTickMaximum2D";
    case ID_xMajorTickSpacing2D:       return "xMajorTickSpacing2D";
    case ID_yMajorTickSpacing2D:       return "yMajorTickSpacing2D";
    case ID_xMinorTickSpacing2D:       return "xMinorTickSpacing2D";
    case ID_yMinorTickSpacing2D:       return "yMinorTickSpacing2D";
    case ID_xLabelFontHeight2D:        return "xLabelFontHeight2D";
    case ID_yLabelFontHeight2D:        return "yLabelFontHeight2D";
    case ID_xTitleFontHeight2D:        return "xTitleFontHeight2D";
    case ID_yTitleFontHeight2D:        return "yTitleFontHeight2D";
    case ID_xLabelScaling2D:           return "xLabelScaling2D";
    case ID_yLabelScaling2D:           return "yLabelScaling2D";
    case ID_axesLineWidth2D:           return "axesLineWidth2D";
    case ID_axesTickLocation2D:        return "axesTickLocation2D";
    case ID_axesTicks2D:               return "axesTicks2D";
    case ID_xAxisUserTitle2D:          return "xAxisUserTitle2D";
    case ID_yAxisUserTitle2D:          return "yAxisUserTitle2D";
    case ID_xAxisUserTitleFlag2D:      return "xAxisUserTitleFlag2D";
    case ID_yAxisUserTitleFlag2D:      return "yAxisUserTitleFlag2D";
    case ID_xAxisUserUnits2D:          return "xAxisUserUnits2D";
    case ID_yAxisUserUnits2D:          return "yAxisUserUnits2D";
    case ID_xAxisUserUnitsFlag2D:      return "xAxisUserUnitsFlag2D";
    case ID_yAxisUserUnitsFlag2D:      return "yAxisUserUnitsFlag2D";
    case ID_axesFlag:                  return "axesFlag";
    case ID_axesAutoSetTicks:          return "axesAutoSetTicks";
    case ID_labelAutoSetScaling:       return "labelAutoSetScaling";
    case ID_xAxisLabels:               return "xAxisLabels";
    case ID_yAxisLabels:               return "yAxisLabels";
    case ID_zAxisLabels:               return "zAxisLabels";
    case ID_xAxisTitle:                return "xAxisTitle";
    case ID_yAxisTitle:                return "yAxisTitle";
    case ID_zAxisTitle:                return "zAxisTitle";
    case ID_xGridLines:                return "xGridLines";
    case ID_yGridLines:                return "yGridLines";
    case ID_zGridLines:                return "zGridLines";
    case ID_xAxisTicks:                return "xAxisTicks";
    case ID_yAxisTicks:                return "yAxisTicks";
    case ID_zAxisTicks:                return "zAxisTicks";
    case ID_xMajorTickMinimum:         return "xMajorTickMinimum";
    case ID_yMajorTickMinimum:         return "yMajorTickMinimum";
    case ID_zMajorTickMinimum:         return "zMajorTickMinimum";
    case ID_xMajorTickMaximum:         return "xMajorTickMaximum";
    case ID_yMajorTickMaximum:         return "yMajorTickMaximum";
    case ID_zMajorTickMaximum:         return "zMajorTickMaximum";
    case ID_xMajorTickSpacing:         return "xMajorTickSpacing";
    case ID_yMajorTickSpacing:         return "yMajorTickSpacing";
    case ID_zMajorTickSpacing:         return "zMajorTickSpacing";
    case ID_xMinorTickSpacing:         return "xMinorTickSpacing";
    case ID_yMinorTickSpacing:         return "yMinorTickSpacing";
    case ID_zMinorTickSpacing:         return "zMinorTickSpacing";
    case ID_xLabelFontHeight:          return "xLabelFontHeight";
    case ID_yLabelFontHeight:          return "yLabelFontHeight";
    case ID_zLabelFontHeight:          return "zLabelFontHeight";
    case ID_xTitleFontHeight:          return "xTitleFontHeight";
    case ID_yTitleFontHeight:          return "yTitleFontHeight";
    case ID_zTitleFontHeight:          return "zTitleFontHeight";
    case ID_xLabelScaling:             return "xLabelScaling";
    case ID_yLabelScaling:             return "yLabelScaling";
    case ID_zLabelScaling:             return "zLabelScaling";
    case ID_xAxisUserTitle:            return "xAxisUserTitle";
    case ID_yAxisUserTitle:            return "yAxisUserTitle";
    case ID_zAxisUserTitle:            return "zAxisUserTitle";
    case ID_xAxisUserTitleFlag:        return "xAxisUserTitleFlag";
    case ID_yAxisUserTitleFlag:        return "yAxisUserTitleFlag";
    case ID_zAxisUserTitleFlag:        return "zAxisUserTitleFlag";
    case ID_xAxisUserUnits:            return "xAxisUserUnits";
    case ID_yAxisUserUnits:            return "yAxisUserUnits";
    case ID_zAxisUserUnits:            return "zAxisUserUnits";
    case ID_xAxisUserUnitsFlag:        return "xAxisUserUnitsFlag";
    case ID_yAxisUserUnitsFlag:        return "yAxisUserUnitsFlag";
    case ID_zAxisUserUnitsFlag:        return "zAxisUserUnitsFlag";
    case ID_axesTickLocation:          return "axesTickLocation";
    case ID_axesType:                  return "axesType";
    case ID_triadFlag:                 return "triadFlag";
    case ID_bboxFlag:                  return "bboxFlag";
    case ID_backgroundColor:           return "backgroundColor";
    case ID_foregroundColor:           return "foregroundColor";
    case ID_gradientBackgroundStyle:   return "gradientBackgroundStyle";
    case ID_gradientColor1:            return "gradientColor1";
    case ID_gradientColor2:            return "gradientColor2";
    case ID_backgroundMode:            return "backgroundMode";
    case ID_backgroundImage:           return "backgroundImage";
    case ID_userInfoFlag:              return "userInfoFlag";
    case ID_databaseInfoFlag:          return "databaseInfoFlag";
    case ID_databaseInfoExpansionMode: return "databaseInfoExpansionMode";
    case ID_legendInfoFlag:            return "legendInfoFlag";
    case ID_imageRepeatX:              return "imageRepeatX";
    case ID_imageRepeatY:              return "imageRepeatY";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: AnnotationAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
AnnotationAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_axesFlag2D:                return FieldType_bool;
    case ID_axesAutoSetTicks2D:        return FieldType_bool;
    case ID_labelAutoSetScaling2D:     return FieldType_bool;
    case ID_xAxisLabels2D:             return FieldType_bool;
    case ID_yAxisLabels2D:             return FieldType_bool;
    case ID_xAxisTitle2D:              return FieldType_bool;
    case ID_yAxisTitle2D:              return FieldType_bool;
    case ID_xGridLines2D:              return FieldType_bool;
    case ID_yGridLines2D:              return FieldType_bool;
    case ID_xMajorTickMinimum2D:       return FieldType_double;
    case ID_yMajorTickMinimum2D:       return FieldType_double;
    case ID_xMajorTickMaximum2D:       return FieldType_double;
    case ID_yMajorTickMaximum2D:       return FieldType_double;
    case ID_xMajorTickSpacing2D:       return FieldType_double;
    case ID_yMajorTickSpacing2D:       return FieldType_double;
    case ID_xMinorTickSpacing2D:       return FieldType_double;
    case ID_yMinorTickSpacing2D:       return FieldType_double;
    case ID_xLabelFontHeight2D:        return FieldType_double;
    case ID_yLabelFontHeight2D:        return FieldType_double;
    case ID_xTitleFontHeight2D:        return FieldType_double;
    case ID_yTitleFontHeight2D:        return FieldType_double;
    case ID_xLabelScaling2D:           return FieldType_int;
    case ID_yLabelScaling2D:           return FieldType_int;
    case ID_axesLineWidth2D:           return FieldType_linewidth;
    case ID_axesTickLocation2D:        return FieldType_enum;
    case ID_axesTicks2D:               return FieldType_enum;
    case ID_xAxisUserTitle2D:          return FieldType_string;
    case ID_yAxisUserTitle2D:          return FieldType_string;
    case ID_xAxisUserTitleFlag2D:      return FieldType_bool;
    case ID_yAxisUserTitleFlag2D:      return FieldType_bool;
    case ID_xAxisUserUnits2D:          return FieldType_string;
    case ID_yAxisUserUnits2D:          return FieldType_string;
    case ID_xAxisUserUnitsFlag2D:      return FieldType_bool;
    case ID_yAxisUserUnitsFlag2D:      return FieldType_bool;
    case ID_axesFlag:                  return FieldType_bool;
    case ID_axesAutoSetTicks:          return FieldType_bool;
    case ID_labelAutoSetScaling:       return FieldType_bool;
    case ID_xAxisLabels:               return FieldType_bool;
    case ID_yAxisLabels:               return FieldType_bool;
    case ID_zAxisLabels:               return FieldType_bool;
    case ID_xAxisTitle:                return FieldType_bool;
    case ID_yAxisTitle:                return FieldType_bool;
    case ID_zAxisTitle:                return FieldType_bool;
    case ID_xGridLines:                return FieldType_bool;
    case ID_yGridLines:                return FieldType_bool;
    case ID_zGridLines:                return FieldType_bool;
    case ID_xAxisTicks:                return FieldType_bool;
    case ID_yAxisTicks:                return FieldType_bool;
    case ID_zAxisTicks:                return FieldType_bool;
    case ID_xMajorTickMinimum:         return FieldType_double;
    case ID_yMajorTickMinimum:         return FieldType_double;
    case ID_zMajorTickMinimum:         return FieldType_double;
    case ID_xMajorTickMaximum:         return FieldType_double;
    case ID_yMajorTickMaximum:         return FieldType_double;
    case ID_zMajorTickMaximum:         return FieldType_double;
    case ID_xMajorTickSpacing:         return FieldType_double;
    case ID_yMajorTickSpacing:         return FieldType_double;
    case ID_zMajorTickSpacing:         return FieldType_double;
    case ID_xMinorTickSpacing:         return FieldType_double;
    case ID_yMinorTickSpacing:         return FieldType_double;
    case ID_zMinorTickSpacing:         return FieldType_double;
    case ID_xLabelFontHeight:          return FieldType_double;
    case ID_yLabelFontHeight:          return FieldType_double;
    case ID_zLabelFontHeight:          return FieldType_double;
    case ID_xTitleFontHeight:          return FieldType_double;
    case ID_yTitleFontHeight:          return FieldType_double;
    case ID_zTitleFontHeight:          return FieldType_double;
    case ID_xLabelScaling:             return FieldType_int;
    case ID_yLabelScaling:             return FieldType_int;
    case ID_zLabelScaling:             return FieldType_int;
    case ID_xAxisUserTitle:            return FieldType_string;
    case ID_yAxisUserTitle:            return FieldType_string;
    case ID_zAxisUserTitle:            return FieldType_string;
    case ID_xAxisUserTitleFlag:        return FieldType_bool;
    case ID_yAxisUserTitleFlag:        return FieldType_bool;
    case ID_zAxisUserTitleFlag:        return FieldType_bool;
    case ID_xAxisUserUnits:            return FieldType_string;
    case ID_yAxisUserUnits:            return FieldType_string;
    case ID_zAxisUserUnits:            return FieldType_string;
    case ID_xAxisUserUnitsFlag:        return FieldType_bool;
    case ID_yAxisUserUnitsFlag:        return FieldType_bool;
    case ID_zAxisUserUnitsFlag:        return FieldType_bool;
    case ID_axesTickLocation:          return FieldType_enum;
    case ID_axesType:                  return FieldType_enum;
    case ID_triadFlag:                 return FieldType_bool;
    case ID_bboxFlag:                  return FieldType_bool;
    case ID_backgroundColor:           return FieldType_color;
    case ID_foregroundColor:           return FieldType_color;
    case ID_gradientBackgroundStyle:   return FieldType_enum;
    case ID_gradientColor1:            return FieldType_color;
    case ID_gradientColor2:            return FieldType_color;
    case ID_backgroundMode:            return FieldType_enum;
    case ID_backgroundImage:           return FieldType_string;
    case ID_userInfoFlag:              return FieldType_bool;
    case ID_databaseInfoFlag:          return FieldType_bool;
    case ID_databaseInfoExpansionMode: return FieldType_enum;
    case ID_legendInfoFlag:            return FieldType_bool;
    case ID_imageRepeatX:              return FieldType_int;
    case ID_imageRepeatY:              return FieldType_int;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: AnnotationAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

std::string
AnnotationAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_axesFlag2D:                return "bool";
    case ID_axesAutoSetTicks2D:        return "bool";
    case ID_labelAutoSetScaling2D:     return "bool";
    case ID_xAxisLabels2D:             return "bool";
    case ID_yAxisLabels2D:             return "bool";
    case ID_xAxisTitle2D:              return "bool";
    case ID_yAxisTitle2D:              return "bool";
    case ID_xGridLines2D:              return "bool";
    case ID_yGridLines2D:              return "bool";
    case ID_xMajorTickMinimum2D:       return "double";
    case ID_yMajorTickMinimum2D:       return "double";
    case ID_xMajorTickMaximum2D:       return "double";
    case ID_yMajorTickMaximum2D:       return "double";
    case ID_xMajorTickSpacing2D:       return "double";
    case ID_yMajorTickSpacing2D:       return "double";
    case ID_xMinorTickSpacing2D:       return "double";
    case ID_yMinorTickSpacing2D:       return "double";
    case ID_xLabelFontHeight2D:        return "double";
    case ID_yLabelFontHeight2D:        return "double";
    case ID_xTitleFontHeight2D:        return "double";
    case ID_yTitleFontHeight2D:        return "double";
    case ID_xLabelScaling2D:           return "int";
    case ID_yLabelScaling2D:           return "int";
    case ID_axesLineWidth2D:           return "linewidth";
    case ID_axesTickLocation2D:        return "enum";
    case ID_axesTicks2D:               return "enum";
    case ID_xAxisUserTitle2D:          return "string";
    case ID_yAxisUserTitle2D:          return "string";
    case ID_xAxisUserTitleFlag2D:      return "bool";
    case ID_yAxisUserTitleFlag2D:      return "bool";
    case ID_xAxisUserUnits2D:          return "string";
    case ID_yAxisUserUnits2D:          return "string";
    case ID_xAxisUserUnitsFlag2D:      return "bool";
    case ID_yAxisUserUnitsFlag2D:      return "bool";
    case ID_axesFlag:                  return "bool";
    case ID_axesAutoSetTicks:          return "bool";
    case ID_labelAutoSetScaling:       return "bool";
    case ID_xAxisLabels:               return "bool";
    case ID_yAxisLabels:               return "bool";
    case ID_zAxisLabels:               return "bool";
    case ID_xAxisTitle:                return "bool";
    case ID_yAxisTitle:                return "bool";
    case ID_zAxisTitle:                return "bool";
    case ID_xGridLines:                return "bool";
    case ID_yGridLines:                return "bool";
    case ID_zGridLines:                return "bool";
    case ID_xAxisTicks:                return "bool";
    case ID_yAxisTicks:                return "bool";
    case ID_zAxisTicks:                return "bool";
    case ID_xMajorTickMinimum:         return "double";
    case ID_yMajorTickMinimum:         return "double";
    case ID_zMajorTickMinimum:         return "double";
    case ID_xMajorTickMaximum:         return "double";
    case ID_yMajorTickMaximum:         return "double";
    case ID_zMajorTickMaximum:         return "double";
    case ID_xMajorTickSpacing:         return "double";
    case ID_yMajorTickSpacing:         return "double";
    case ID_zMajorTickSpacing:         return "double";
    case ID_xMinorTickSpacing:         return "double";
    case ID_yMinorTickSpacing:         return "double";
    case ID_zMinorTickSpacing:         return "double";
    case ID_xLabelFontHeight:          return "double";
    case ID_yLabelFontHeight:          return "double";
    case ID_zLabelFontHeight:          return "double";
    case ID_xTitleFontHeight:          return "double";
    case ID_yTitleFontHeight:          return "double";
    case ID_zTitleFontHeight:          return "double";
    case ID_xLabelScaling:             return "int";
    case ID_yLabelScaling:             return "int";
    case ID_zLabelScaling:             return "int";
    case ID_xAxisUserTitle:            return "string";
    case ID_yAxisUserTitle:            return "string";
    case ID_zAxisUserTitle:            return "string";
    case ID_xAxisUserTitleFlag:        return "bool";
    case ID_yAxisUserTitleFlag:        return "bool";
    case ID_zAxisUserTitleFlag:        return "bool";
    case ID_xAxisUserUnits:            return "string";
    case ID_yAxisUserUnits:            return "string";
    case ID_zAxisUserUnits:            return "string";
    case ID_xAxisUserUnitsFlag:        return "bool";
    case ID_yAxisUserUnitsFlag:        return "bool";
    case ID_zAxisUserUnitsFlag:        return "bool";
    case ID_axesTickLocation:          return "enum";
    case ID_axesType:                  return "enum";
    case ID_triadFlag:                 return "bool";
    case ID_bboxFlag:                  return "bool";
    case ID_backgroundColor:           return "color";
    case ID_foregroundColor:           return "color";
    case ID_gradientBackgroundStyle:   return "enum";
    case ID_gradientColor1:            return "color";
    case ID_gradientColor2:            return "color";
    case ID_backgroundMode:            return "enum";
    case ID_backgroundImage:           return "string";
    case ID_userInfoFlag:              return "bool";
    case ID_databaseInfoFlag:          return "bool";
    case ID_databaseInfoExpansionMode: return "enum";
    case ID_legendInfoFlag:            return "bool";
    case ID_imageRepeatX:              return "int";
    case ID_imageRepeatY:              return "int";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: AnnotationAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Dec 20 09:40:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
AnnotationAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const AnnotationAttributes &obj = *((const AnnotationAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_axesFlag2D:
        {  // new scope
        retval = (axesFlag2D == obj.axesFlag2D);
        }
        break;
    case ID_axesAutoSetTicks2D:
        {  // new scope
        retval = (axesAutoSetTicks2D == obj.axesAutoSetTicks2D);
        }
        break;
    case ID_labelAutoSetScaling2D:
        {  // new scope
        retval = (labelAutoSetScaling2D == obj.labelAutoSetScaling2D);
        }
        break;
    case ID_xAxisLabels2D:
        {  // new scope
        retval = (xAxisLabels2D == obj.xAxisLabels2D);
        }
        break;
    case ID_yAxisLabels2D:
        {  // new scope
        retval = (yAxisLabels2D == obj.yAxisLabels2D);
        }
        break;
    case ID_xAxisTitle2D:
        {  // new scope
        retval = (xAxisTitle2D == obj.xAxisTitle2D);
        }
        break;
    case ID_yAxisTitle2D:
        {  // new scope
        retval = (yAxisTitle2D == obj.yAxisTitle2D);
        }
        break;
    case ID_xGridLines2D:
        {  // new scope
        retval = (xGridLines2D == obj.xGridLines2D);
        }
        break;
    case ID_yGridLines2D:
        {  // new scope
        retval = (yGridLines2D == obj.yGridLines2D);
        }
        break;
    case ID_xMajorTickMinimum2D:
        {  // new scope
        retval = (xMajorTickMinimum2D == obj.xMajorTickMinimum2D);
        }
        break;
    case ID_yMajorTickMinimum2D:
        {  // new scope
        retval = (yMajorTickMinimum2D == obj.yMajorTickMinimum2D);
        }
        break;
    case ID_xMajorTickMaximum2D:
        {  // new scope
        retval = (xMajorTickMaximum2D == obj.xMajorTickMaximum2D);
        }
        break;
    case ID_yMajorTickMaximum2D:
        {  // new scope
        retval = (yMajorTickMaximum2D == obj.yMajorTickMaximum2D);
        }
        break;
    case ID_xMajorTickSpacing2D:
        {  // new scope
        retval = (xMajorTickSpacing2D == obj.xMajorTickSpacing2D);
        }
        break;
    case ID_yMajorTickSpacing2D:
        {  // new scope
        retval = (yMajorTickSpacing2D == obj.yMajorTickSpacing2D);
        }
        break;
    case ID_xMinorTickSpacing2D:
        {  // new scope
        retval = (xMinorTickSpacing2D == obj.xMinorTickSpacing2D);
        }
        break;
    case ID_yMinorTickSpacing2D:
        {  // new scope
        retval = (yMinorTickSpacing2D == obj.yMinorTickSpacing2D);
        }
        break;
    case ID_xLabelFontHeight2D:
        {  // new scope
        retval = (xLabelFontHeight2D == obj.xLabelFontHeight2D);
        }
        break;
    case ID_yLabelFontHeight2D:
        {  // new scope
        retval = (yLabelFontHeight2D == obj.yLabelFontHeight2D);
        }
        break;
    case ID_xTitleFontHeight2D:
        {  // new scope
        retval = (xTitleFontHeight2D == obj.xTitleFontHeight2D);
        }
        break;
    case ID_yTitleFontHeight2D:
        {  // new scope
        retval = (yTitleFontHeight2D == obj.yTitleFontHeight2D);
        }
        break;
    case ID_xLabelScaling2D:
        {  // new scope
        retval = (xLabelScaling2D == obj.xLabelScaling2D);
        }
        break;
    case ID_yLabelScaling2D:
        {  // new scope
        retval = (yLabelScaling2D == obj.yLabelScaling2D);
        }
        break;
    case ID_axesLineWidth2D:
        {  // new scope
        retval = (axesLineWidth2D == obj.axesLineWidth2D);
        }
        break;
    case ID_axesTickLocation2D:
        {  // new scope
        retval = (axesTickLocation2D == obj.axesTickLocation2D);
        }
        break;
    case ID_axesTicks2D:
        {  // new scope
        retval = (axesTicks2D == obj.axesTicks2D);
        }
        break;
    case ID_xAxisUserTitle2D:
        {  // new scope
        retval = (xAxisUserTitle2D == obj.xAxisUserTitle2D);
        }
        break;
    case ID_yAxisUserTitle2D:
        {  // new scope
        retval = (yAxisUserTitle2D == obj.yAxisUserTitle2D);
        }
        break;
    case ID_xAxisUserTitleFlag2D:
        {  // new scope
        retval = (xAxisUserTitleFlag2D == obj.xAxisUserTitleFlag2D);
        }
        break;
    case ID_yAxisUserTitleFlag2D:
        {  // new scope
        retval = (yAxisUserTitleFlag2D == obj.yAxisUserTitleFlag2D);
        }
        break;
    case ID_xAxisUserUnits2D:
        {  // new scope
        retval = (xAxisUserUnits2D == obj.xAxisUserUnits2D);
        }
        break;
    case ID_yAxisUserUnits2D:
        {  // new scope
        retval = (yAxisUserUnits2D == obj.yAxisUserUnits2D);
        }
        break;
    case ID_xAxisUserUnitsFlag2D:
        {  // new scope
        retval = (xAxisUserUnitsFlag2D == obj.xAxisUserUnitsFlag2D);
        }
        break;
    case ID_yAxisUserUnitsFlag2D:
        {  // new scope
        retval = (yAxisUserUnitsFlag2D == obj.yAxisUserUnitsFlag2D);
        }
        break;
    case ID_axesFlag:
        {  // new scope
        retval = (axesFlag == obj.axesFlag);
        }
        break;
    case ID_axesAutoSetTicks:
        {  // new scope
        retval = (axesAutoSetTicks == obj.axesAutoSetTicks);
        }
        break;
    case ID_labelAutoSetScaling:
        {  // new scope
        retval = (labelAutoSetScaling == obj.labelAutoSetScaling);
        }
        break;
    case ID_xAxisLabels:
        {  // new scope
        retval = (xAxisLabels == obj.xAxisLabels);
        }
        break;
    case ID_yAxisLabels:
        {  // new scope
        retval = (yAxisLabels == obj.yAxisLabels);
        }
        break;
    case ID_zAxisLabels:
        {  // new scope
        retval = (zAxisLabels == obj.zAxisLabels);
        }
        break;
    case ID_xAxisTitle:
        {  // new scope
        retval = (xAxisTitle == obj.xAxisTitle);
        }
        break;
    case ID_yAxisTitle:
        {  // new scope
        retval = (yAxisTitle == obj.yAxisTitle);
        }
        break;
    case ID_zAxisTitle:
        {  // new scope
        retval = (zAxisTitle == obj.zAxisTitle);
        }
        break;
    case ID_xGridLines:
        {  // new scope
        retval = (xGridLines == obj.xGridLines);
        }
        break;
    case ID_yGridLines:
        {  // new scope
        retval = (yGridLines == obj.yGridLines);
        }
        break;
    case ID_zGridLines:
        {  // new scope
        retval = (zGridLines == obj.zGridLines);
        }
        break;
    case ID_xAxisTicks:
        {  // new scope
        retval = (xAxisTicks == obj.xAxisTicks);
        }
        break;
    case ID_yAxisTicks:
        {  // new scope
        retval = (yAxisTicks == obj.yAxisTicks);
        }
        break;
    case ID_zAxisTicks:
        {  // new scope
        retval = (zAxisTicks == obj.zAxisTicks);
        }
        break;
    case ID_xMajorTickMinimum:
        {  // new scope
        retval = (xMajorTickMinimum == obj.xMajorTickMinimum);
        }
        break;
    case ID_yMajorTickMinimum:
        {  // new scope
        retval = (yMajorTickMinimum == obj.yMajorTickMinimum);
        }
        break;
    case ID_zMajorTickMinimum:
        {  // new scope
        retval = (zMajorTickMinimum == obj.zMajorTickMinimum);
        }
        break;
    case ID_xMajorTickMaximum:
        {  // new scope
        retval = (xMajorTickMaximum == obj.xMajorTickMaximum);
        }
        break;
    case ID_yMajorTickMaximum:
        {  // new scope
        retval = (yMajorTickMaximum == obj.yMajorTickMaximum);
        }
        break;
    case ID_zMajorTickMaximum:
        {  // new scope
        retval = (zMajorTickMaximum == obj.zMajorTickMaximum);
        }
        break;
    case ID_xMajorTickSpacing:
        {  // new scope
        retval = (xMajorTickSpacing == obj.xMajorTickSpacing);
        }
        break;
    case ID_yMajorTickSpacing:
        {  // new scope
        retval = (yMajorTickSpacing == obj.yMajorTickSpacing);
        }
        break;
    case ID_zMajorTickSpacing:
        {  // new scope
        retval = (zMajorTickSpacing == obj.zMajorTickSpacing);
        }
        break;
    case ID_xMinorTickSpacing:
        {  // new scope
        retval = (xMinorTickSpacing == obj.xMinorTickSpacing);
        }
        break;
    case ID_yMinorTickSpacing:
        {  // new scope
        retval = (yMinorTickSpacing == obj.yMinorTickSpacing);
        }
        break;
    case ID_zMinorTickSpacing:
        {  // new scope
        retval = (zMinorTickSpacing == obj.zMinorTickSpacing);
        }
        break;
    case ID_xLabelFontHeight:
        {  // new scope
        retval = (xLabelFontHeight == obj.xLabelFontHeight);
        }
        break;
    case ID_yLabelFontHeight:
        {  // new scope
        retval = (yLabelFontHeight == obj.yLabelFontHeight);
        }
        break;
    case ID_zLabelFontHeight:
        {  // new scope
        retval = (zLabelFontHeight == obj.zLabelFontHeight);
        }
        break;
    case ID_xTitleFontHeight:
        {  // new scope
        retval = (xTitleFontHeight == obj.xTitleFontHeight);
        }
        break;
    case ID_yTitleFontHeight:
        {  // new scope
        retval = (yTitleFontHeight == obj.yTitleFontHeight);
        }
        break;
    case ID_zTitleFontHeight:
        {  // new scope
        retval = (zTitleFontHeight == obj.zTitleFontHeight);
        }
        break;
    case ID_xLabelScaling:
        {  // new scope
        retval = (xLabelScaling == obj.xLabelScaling);
        }
        break;
    case ID_yLabelScaling:
        {  // new scope
        retval = (yLabelScaling == obj.yLabelScaling);
        }
        break;
    case ID_zLabelScaling:
        {  // new scope
        retval = (zLabelScaling == obj.zLabelScaling);
        }
        break;
    case ID_xAxisUserTitle:
        {  // new scope
        retval = (xAxisUserTitle == obj.xAxisUserTitle);
        }
        break;
    case ID_yAxisUserTitle:
        {  // new scope
        retval = (yAxisUserTitle == obj.yAxisUserTitle);
        }
        break;
    case ID_zAxisUserTitle:
        {  // new scope
        retval = (zAxisUserTitle == obj.zAxisUserTitle);
        }
        break;
    case ID_xAxisUserTitleFlag:
        {  // new scope
        retval = (xAxisUserTitleFlag == obj.xAxisUserTitleFlag);
        }
        break;
    case ID_yAxisUserTitleFlag:
        {  // new scope
        retval = (yAxisUserTitleFlag == obj.yAxisUserTitleFlag);
        }
        break;
    case ID_zAxisUserTitleFlag:
        {  // new scope
        retval = (zAxisUserTitleFlag == obj.zAxisUserTitleFlag);
        }
        break;
    case ID_xAxisUserUnits:
        {  // new scope
        retval = (xAxisUserUnits == obj.xAxisUserUnits);
        }
        break;
    case ID_yAxisUserUnits:
        {  // new scope
        retval = (yAxisUserUnits == obj.yAxisUserUnits);
        }
        break;
    case ID_zAxisUserUnits:
        {  // new scope
        retval = (zAxisUserUnits == obj.zAxisUserUnits);
        }
        break;
    case ID_xAxisUserUnitsFlag:
        {  // new scope
        retval = (xAxisUserUnitsFlag == obj.xAxisUserUnitsFlag);
        }
        break;
    case ID_yAxisUserUnitsFlag:
        {  // new scope
        retval = (yAxisUserUnitsFlag == obj.yAxisUserUnitsFlag);
        }
        break;
    case ID_zAxisUserUnitsFlag:
        {  // new scope
        retval = (zAxisUserUnitsFlag == obj.zAxisUserUnitsFlag);
        }
        break;
    case ID_axesTickLocation:
        {  // new scope
        retval = (axesTickLocation == obj.axesTickLocation);
        }
        break;
    case ID_axesType:
        {  // new scope
        retval = (axesType == obj.axesType);
        }
        break;
    case ID_triadFlag:
        {  // new scope
        retval = (triadFlag == obj.triadFlag);
        }
        break;
    case ID_bboxFlag:
        {  // new scope
        retval = (bboxFlag == obj.bboxFlag);
        }
        break;
    case ID_backgroundColor:
        {  // new scope
        retval = (backgroundColor == obj.backgroundColor);
        }
        break;
    case ID_foregroundColor:
        {  // new scope
        retval = (foregroundColor == obj.foregroundColor);
        }
        break;
    case ID_gradientBackgroundStyle:
        {  // new scope
        retval = (gradientBackgroundStyle == obj.gradientBackgroundStyle);
        }
        break;
    case ID_gradientColor1:
        {  // new scope
        retval = (gradientColor1 == obj.gradientColor1);
        }
        break;
    case ID_gradientColor2:
        {  // new scope
        retval = (gradientColor2 == obj.gradientColor2);
        }
        break;
    case ID_backgroundMode:
        {  // new scope
        retval = (backgroundMode == obj.backgroundMode);
        }
        break;
    case ID_backgroundImage:
        {  // new scope
        retval = (backgroundImage == obj.backgroundImage);
        }
        break;
    case ID_userInfoFlag:
        {  // new scope
        retval = (userInfoFlag == obj.userInfoFlag);
        }
        break;
    case ID_databaseInfoFlag:
        {  // new scope
        retval = (databaseInfoFlag == obj.databaseInfoFlag);
        }
        break;
    case ID_databaseInfoExpansionMode:
        {  // new scope
        retval = (databaseInfoExpansionMode == obj.databaseInfoExpansionMode);
        }
        break;
    case ID_legendInfoFlag:
        {  // new scope
        retval = (legendInfoFlag == obj.legendInfoFlag);
        }
        break;
    case ID_imageRepeatX:
        {  // new scope
        retval = (imageRepeatX == obj.imageRepeatX);
        }
        break;
    case ID_imageRepeatY:
        {  // new scope
        retval = (imageRepeatY == obj.imageRepeatY);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// Returns a "discernible" background color. If the background mode
// is Solid, the returned value is just the background color. If the
// background mode is Gradient, the returned color is an average of
// the two gradient colors
const ColorAttribute AnnotationAttributes::GetDiscernibleBackgroundColor() const
{
    ColorAttribute retval;
    double bg[4];

    if (GetBackgroundMode() == Solid)
    {
        GetBackgroundColor().GetRgba(bg);
    }
    else
    {
        double gbg1[4], gbg2[4];

        GetGradientColor1().GetRgba(gbg1);
        GetGradientColor2().GetRgba(gbg2);

        bg[0] = (gbg1[0] + gbg2[0]) * 0.5;
        bg[1] = (gbg1[1] + gbg2[1]) * 0.5;
        bg[2] = (gbg1[2] + gbg2[2]) * 0.5;
        bg[3] = (gbg1[3] + gbg2[3]) * 0.5;
    }

    retval.SetRgba2(bg);

    return retval;
}

