/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <CurveAttributes.h>
#include <DataNode.h>
#include <Line.h>

//
// Enum conversion methods for CurveAttributes::CurveColor
//

static const char *CurveColor_strings[] = {
"Cycle", "Custom"};

std::string
CurveAttributes::CurveColor_ToString(CurveAttributes::CurveColor t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return CurveColor_strings[index];
}

std::string
CurveAttributes::CurveColor_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return CurveColor_strings[index];
}

bool
CurveAttributes::CurveColor_FromString(const std::string &s, CurveAttributes::CurveColor &val)
{
    val = CurveAttributes::Cycle;
    for(int i = 0; i < 2; ++i)
    {
        if(s == CurveColor_strings[i])
        {
            val = (CurveColor)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for CurveAttributes::FillMode
//

static const char *FillMode_strings[] = {
"Static", "Dynamic"};

std::string
CurveAttributes::FillMode_ToString(CurveAttributes::FillMode t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return FillMode_strings[index];
}

std::string
CurveAttributes::FillMode_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return FillMode_strings[index];
}

bool
CurveAttributes::FillMode_FromString(const std::string &s, CurveAttributes::FillMode &val)
{
    val = CurveAttributes::Static;
    for(int i = 0; i < 2; ++i)
    {
        if(s == FillMode_strings[i])
        {
            val = (FillMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for CurveAttributes::SymbolTypes
//

static const char *SymbolTypes_strings[] = {
"Point", "TriangleUp", "TriangleDown", 
"Square", "Circle", "Plus", 
"X"};

std::string
CurveAttributes::SymbolTypes_ToString(CurveAttributes::SymbolTypes t)
{
    int index = int(t);
    if(index < 0 || index >= 7) index = 0;
    return SymbolTypes_strings[index];
}

std::string
CurveAttributes::SymbolTypes_ToString(int t)
{
    int index = (t < 0 || t >= 7) ? 0 : t;
    return SymbolTypes_strings[index];
}

bool
CurveAttributes::SymbolTypes_FromString(const std::string &s, CurveAttributes::SymbolTypes &val)
{
    val = CurveAttributes::Point;
    for(int i = 0; i < 7; ++i)
    {
        if(s == SymbolTypes_strings[i])
        {
            val = (SymbolTypes)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for CurveAttributes::CurveFillMode
//

static const char *CurveFillMode_strings[] = {
"NoFill", "Solid", "HorizontalGradient", 
"VerticalGradient"};

std::string
CurveAttributes::CurveFillMode_ToString(CurveAttributes::CurveFillMode t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return CurveFillMode_strings[index];
}

std::string
CurveAttributes::CurveFillMode_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return CurveFillMode_strings[index];
}

bool
CurveAttributes::CurveFillMode_FromString(const std::string &s, CurveAttributes::CurveFillMode &val)
{
    val = CurveAttributes::NoFill;
    for(int i = 0; i < 4; ++i)
    {
        if(s == CurveFillMode_strings[i])
        {
            val = (CurveFillMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for CurveAttributes::PolarCoordinateOrder
//

static const char *PolarCoordinateOrder_strings[] = {
"R_Theta", "Theta_R"};

std::string
CurveAttributes::PolarCoordinateOrder_ToString(CurveAttributes::PolarCoordinateOrder t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return PolarCoordinateOrder_strings[index];
}

std::string
CurveAttributes::PolarCoordinateOrder_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return PolarCoordinateOrder_strings[index];
}

bool
CurveAttributes::PolarCoordinateOrder_FromString(const std::string &s, CurveAttributes::PolarCoordinateOrder &val)
{
    val = CurveAttributes::R_Theta;
    for(int i = 0; i < 2; ++i)
    {
        if(s == PolarCoordinateOrder_strings[i])
        {
            val = (PolarCoordinateOrder)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for CurveAttributes::AngleUnits
//

static const char *AngleUnits_strings[] = {
"Radians", "Degrees"};

std::string
CurveAttributes::AngleUnits_ToString(CurveAttributes::AngleUnits t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return AngleUnits_strings[index];
}

std::string
CurveAttributes::AngleUnits_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return AngleUnits_strings[index];
}

bool
CurveAttributes::AngleUnits_FromString(const std::string &s, CurveAttributes::AngleUnits &val)
{
    val = CurveAttributes::Radians;
    for(int i = 0; i < 2; ++i)
    {
        if(s == AngleUnits_strings[i])
        {
            val = (AngleUnits)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: CurveAttributes::CurveAttributes
//
// Purpose: 
//   Init utility for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void CurveAttributes::Init()
{
    showLines = true;
    lineStyle = 0;
    lineWidth = 0;
    showPoints = false;
    symbol = Point;
    pointSize = 5;
    pointFillMode = Static;
    pointStride = 1;
    symbolDensity = 50;
    curveColorSource = Cycle;
    showLegend = true;
    showLabels = true;
    doBallTimeCue = false;
    timeCueBallSize = 0.01;
    doLineTimeCue = false;
    lineTimeCueWidth = 0;
    doCropTimeCue = false;
    timeForTimeCue = 0;
    fillMode = NoFill;
    polarToCartesian = false;
    polarCoordinateOrder = R_Theta;
    angleUnits = Radians;

    CurveAttributes::SelectAll();
}

// ****************************************************************************
// Method: CurveAttributes::CurveAttributes
//
// Purpose: 
//   Copy utility for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void CurveAttributes::Copy(const CurveAttributes &obj)
{
    showLines = obj.showLines;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    showPoints = obj.showPoints;
    symbol = obj.symbol;
    pointSize = obj.pointSize;
    pointFillMode = obj.pointFillMode;
    pointStride = obj.pointStride;
    symbolDensity = obj.symbolDensity;
    curveColorSource = obj.curveColorSource;
    curveColor = obj.curveColor;
    showLegend = obj.showLegend;
    showLabels = obj.showLabels;
    designator = obj.designator;
    doBallTimeCue = obj.doBallTimeCue;
    ballTimeCueColor = obj.ballTimeCueColor;
    timeCueBallSize = obj.timeCueBallSize;
    doLineTimeCue = obj.doLineTimeCue;
    lineTimeCueColor = obj.lineTimeCueColor;
    lineTimeCueWidth = obj.lineTimeCueWidth;
    doCropTimeCue = obj.doCropTimeCue;
    timeForTimeCue = obj.timeForTimeCue;
    fillMode = obj.fillMode;
    fillColor1 = obj.fillColor1;
    fillColor2 = obj.fillColor2;
    polarToCartesian = obj.polarToCartesian;
    polarCoordinateOrder = obj.polarCoordinateOrder;
    angleUnits = obj.angleUnits;

    CurveAttributes::SelectAll();
}

// Type map format string
const char *CurveAttributes::TypeMapFormatString = CURVEATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t CurveAttributes::TmfsStruct = {CURVEATTRIBUTES_TMFS};


// ****************************************************************************
// Method: CurveAttributes::CurveAttributes
//
// Purpose: 
//   Default constructor for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

CurveAttributes::CurveAttributes() : 
    AttributeSubject(CurveAttributes::TypeMapFormatString),
    curveColor(0, 0, 0), ballTimeCueColor(0, 0, 0), 
    lineTimeCueColor(0, 0, 0), fillColor1(255, 0, 0), 
    fillColor2(255, 100, 100)
{
    CurveAttributes::Init();
}

// ****************************************************************************
// Method: CurveAttributes::CurveAttributes
//
// Purpose: 
//   Constructor for the derived classes of CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

CurveAttributes::CurveAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs),
    curveColor(0, 0, 0), ballTimeCueColor(0, 0, 0), 
    lineTimeCueColor(0, 0, 0), fillColor1(255, 0, 0), 
    fillColor2(255, 100, 100)
{
    CurveAttributes::Init();
}

// ****************************************************************************
// Method: CurveAttributes::CurveAttributes
//
// Purpose: 
//   Copy constructor for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

CurveAttributes::CurveAttributes(const CurveAttributes &obj) : 
    AttributeSubject(CurveAttributes::TypeMapFormatString)
{
    CurveAttributes::Copy(obj);
}

// ****************************************************************************
// Method: CurveAttributes::CurveAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

CurveAttributes::CurveAttributes(const CurveAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    CurveAttributes::Copy(obj);
}

// ****************************************************************************
// Method: CurveAttributes::~CurveAttributes
//
// Purpose: 
//   Destructor for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

CurveAttributes::~CurveAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: CurveAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

CurveAttributes& 
CurveAttributes::operator = (const CurveAttributes &obj)
{
    if (this == &obj) return *this;

    CurveAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: CurveAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
CurveAttributes::operator == (const CurveAttributes &obj) const
{
    // Create the return value
    return ((showLines == obj.showLines) &&
            (lineStyle == obj.lineStyle) &&
            (lineWidth == obj.lineWidth) &&
            (showPoints == obj.showPoints) &&
            (symbol == obj.symbol) &&
            (pointSize == obj.pointSize) &&
            (pointFillMode == obj.pointFillMode) &&
            (pointStride == obj.pointStride) &&
            (symbolDensity == obj.symbolDensity) &&
            (curveColorSource == obj.curveColorSource) &&
            (curveColor == obj.curveColor) &&
            (showLegend == obj.showLegend) &&
            (showLabels == obj.showLabels) &&
            (designator == obj.designator) &&
            (doBallTimeCue == obj.doBallTimeCue) &&
            (ballTimeCueColor == obj.ballTimeCueColor) &&
            (timeCueBallSize == obj.timeCueBallSize) &&
            (doLineTimeCue == obj.doLineTimeCue) &&
            (lineTimeCueColor == obj.lineTimeCueColor) &&
            (lineTimeCueWidth == obj.lineTimeCueWidth) &&
            (doCropTimeCue == obj.doCropTimeCue) &&
            (timeForTimeCue == obj.timeForTimeCue) &&
            (fillMode == obj.fillMode) &&
            (fillColor1 == obj.fillColor1) &&
            (fillColor2 == obj.fillColor2) &&
            (polarToCartesian == obj.polarToCartesian) &&
            (polarCoordinateOrder == obj.polarCoordinateOrder) &&
            (angleUnits == obj.angleUnits));
}

// ****************************************************************************
// Method: CurveAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
CurveAttributes::operator != (const CurveAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: CurveAttributes::TypeName
//
// Purpose: 
//   Type name method for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

const std::string
CurveAttributes::TypeName() const
{
    return "CurveAttributes";
}

bool
CurveAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const CurveAttributes *tmp = (const CurveAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    else if(atts->TypeName() == "Line")
    {
        const Line *tmp = (const Line*)atts;
        SetDesignator(tmp->GetDesignator());
        SetCurveColor(tmp->GetColor());
        retval = true;
    }

    return retval;
}

AttributeSubject *
CurveAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;

    if(TypeName() == tname)
    {
        retval = new CurveAttributes(*this);
    }
    else if(tname == "Line")
    {
        Line *line = new Line;
        line->SetDesignator(GetDesignator());
        line->SetColor(GetCurveColor());
        line->SetLineStyle(GetLineStyle());
        line->SetLineWidth(GetLineWidth());
        retval = line;
    }

    return retval;
}

// ****************************************************************************
// Method: CurveAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the CurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
CurveAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new CurveAttributes(*this);
    else
        retval = new CurveAttributes;

    return retval;
}

// ****************************************************************************
// Method: CurveAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
CurveAttributes::SelectAll()
{
    Select(ID_showLines,            (void *)&showLines);
    Select(ID_lineStyle,            (void *)&lineStyle);
    Select(ID_lineWidth,            (void *)&lineWidth);
    Select(ID_showPoints,           (void *)&showPoints);
    Select(ID_symbol,               (void *)&symbol);
    Select(ID_pointSize,            (void *)&pointSize);
    Select(ID_pointFillMode,        (void *)&pointFillMode);
    Select(ID_pointStride,          (void *)&pointStride);
    Select(ID_symbolDensity,        (void *)&symbolDensity);
    Select(ID_curveColorSource,     (void *)&curveColorSource);
    Select(ID_curveColor,           (void *)&curveColor);
    Select(ID_showLegend,           (void *)&showLegend);
    Select(ID_showLabels,           (void *)&showLabels);
    Select(ID_designator,           (void *)&designator);
    Select(ID_doBallTimeCue,        (void *)&doBallTimeCue);
    Select(ID_ballTimeCueColor,     (void *)&ballTimeCueColor);
    Select(ID_timeCueBallSize,      (void *)&timeCueBallSize);
    Select(ID_doLineTimeCue,        (void *)&doLineTimeCue);
    Select(ID_lineTimeCueColor,     (void *)&lineTimeCueColor);
    Select(ID_lineTimeCueWidth,     (void *)&lineTimeCueWidth);
    Select(ID_doCropTimeCue,        (void *)&doCropTimeCue);
    Select(ID_timeForTimeCue,       (void *)&timeForTimeCue);
    Select(ID_fillMode,             (void *)&fillMode);
    Select(ID_fillColor1,           (void *)&fillColor1);
    Select(ID_fillColor2,           (void *)&fillColor2);
    Select(ID_polarToCartesian,     (void *)&polarToCartesian);
    Select(ID_polarCoordinateOrder, (void *)&polarCoordinateOrder);
    Select(ID_angleUnits,           (void *)&angleUnits);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CurveAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
CurveAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    CurveAttributes defaultObject;
    bool addToParent = false;
    // Create a node for CurveAttributes.
    DataNode *node = new DataNode("CurveAttributes");

    if(completeSave || !FieldsEqual(ID_showLines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showLines", showLines));
    }

    if(completeSave || !FieldsEqual(ID_lineStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineStyle", lineStyle));
    }

    if(completeSave || !FieldsEqual(ID_lineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineWidth", lineWidth));
    }

    if(completeSave || !FieldsEqual(ID_showPoints, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showPoints", showPoints));
    }

    if(completeSave || !FieldsEqual(ID_symbol, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("symbol", SymbolTypes_ToString(symbol)));
    }

    if(completeSave || !FieldsEqual(ID_pointSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSize", pointSize));
    }

    if(completeSave || !FieldsEqual(ID_pointFillMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointFillMode", FillMode_ToString(pointFillMode)));
    }

    if(completeSave || !FieldsEqual(ID_pointStride, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointStride", pointStride));
    }

    if(completeSave || !FieldsEqual(ID_symbolDensity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("symbolDensity", symbolDensity));
    }

    if(completeSave || !FieldsEqual(ID_curveColorSource, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("curveColorSource", CurveColor_ToString(curveColorSource)));
    }

        DataNode *curveColorNode = new DataNode("curveColor");
        if(curveColor.CreateNode(curveColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(curveColorNode);
        }
        else
            delete curveColorNode;
    if(completeSave || !FieldsEqual(ID_showLegend, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showLegend", showLegend));
    }

    if(completeSave || !FieldsEqual(ID_showLabels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showLabels", showLabels));
    }

    if(completeSave || !FieldsEqual(ID_designator, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("designator", designator));
    }

    if(completeSave || !FieldsEqual(ID_doBallTimeCue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doBallTimeCue", doBallTimeCue));
    }

        DataNode *ballTimeCueColorNode = new DataNode("ballTimeCueColor");
        if(ballTimeCueColor.CreateNode(ballTimeCueColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(ballTimeCueColorNode);
        }
        else
            delete ballTimeCueColorNode;
    if(completeSave || !FieldsEqual(ID_timeCueBallSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeCueBallSize", timeCueBallSize));
    }

    if(completeSave || !FieldsEqual(ID_doLineTimeCue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doLineTimeCue", doLineTimeCue));
    }

        DataNode *lineTimeCueColorNode = new DataNode("lineTimeCueColor");
        if(lineTimeCueColor.CreateNode(lineTimeCueColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(lineTimeCueColorNode);
        }
        else
            delete lineTimeCueColorNode;
    if(completeSave || !FieldsEqual(ID_lineTimeCueWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineTimeCueWidth", lineTimeCueWidth));
    }

    if(completeSave || !FieldsEqual(ID_doCropTimeCue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doCropTimeCue", doCropTimeCue));
    }

    if(completeSave || !FieldsEqual(ID_timeForTimeCue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeForTimeCue", timeForTimeCue));
    }

    if(completeSave || !FieldsEqual(ID_fillMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("fillMode", CurveFillMode_ToString(fillMode)));
    }

        DataNode *fillColor1Node = new DataNode("fillColor1");
        if(fillColor1.CreateNode(fillColor1Node, completeSave, true))
        {
            addToParent = true;
            node->AddNode(fillColor1Node);
        }
        else
            delete fillColor1Node;
        DataNode *fillColor2Node = new DataNode("fillColor2");
        if(fillColor2.CreateNode(fillColor2Node, completeSave, true))
        {
            addToParent = true;
            node->AddNode(fillColor2Node);
        }
        else
            delete fillColor2Node;
    if(completeSave || !FieldsEqual(ID_polarToCartesian, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("polarToCartesian", polarToCartesian));
    }

    if(completeSave || !FieldsEqual(ID_polarCoordinateOrder, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("polarCoordinateOrder", PolarCoordinateOrder_ToString(polarCoordinateOrder)));
    }

    if(completeSave || !FieldsEqual(ID_angleUnits, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("angleUnits", AngleUnits_ToString(angleUnits)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: CurveAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
CurveAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("CurveAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("showLines")) != 0)
        SetShowLines(node->AsBool());
    if((node = searchNode->GetNode("lineStyle")) != 0)
        SetLineStyle(node->AsInt());
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("showPoints")) != 0)
        SetShowPoints(node->AsBool());
    if((node = searchNode->GetNode("symbol")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 7)
                SetSymbol(SymbolTypes(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SymbolTypes value;
            if(SymbolTypes_FromString(node->AsString(), value))
                SetSymbol(value);
        }
    }
    if((node = searchNode->GetNode("pointSize")) != 0)
        SetPointSize(node->AsDouble());
    if((node = searchNode->GetNode("pointFillMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetPointFillMode(FillMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            FillMode value;
            if(FillMode_FromString(node->AsString(), value))
                SetPointFillMode(value);
        }
    }
    if((node = searchNode->GetNode("pointStride")) != 0)
        SetPointStride(node->AsInt());
    if((node = searchNode->GetNode("symbolDensity")) != 0)
        SetSymbolDensity(node->AsInt());
    if((node = searchNode->GetNode("curveColorSource")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetCurveColorSource(CurveColor(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CurveColor value;
            if(CurveColor_FromString(node->AsString(), value))
                SetCurveColorSource(value);
        }
    }
    if((node = searchNode->GetNode("curveColor")) != 0)
        curveColor.SetFromNode(node);
    if((node = searchNode->GetNode("showLegend")) != 0)
        SetShowLegend(node->AsBool());
    if((node = searchNode->GetNode("showLabels")) != 0)
        SetShowLabels(node->AsBool());
    if((node = searchNode->GetNode("designator")) != 0)
        SetDesignator(node->AsString());
    if((node = searchNode->GetNode("doBallTimeCue")) != 0)
        SetDoBallTimeCue(node->AsBool());
    if((node = searchNode->GetNode("ballTimeCueColor")) != 0)
        ballTimeCueColor.SetFromNode(node);
    if((node = searchNode->GetNode("timeCueBallSize")) != 0)
        SetTimeCueBallSize(node->AsDouble());
    if((node = searchNode->GetNode("doLineTimeCue")) != 0)
        SetDoLineTimeCue(node->AsBool());
    if((node = searchNode->GetNode("lineTimeCueColor")) != 0)
        lineTimeCueColor.SetFromNode(node);
    if((node = searchNode->GetNode("lineTimeCueWidth")) != 0)
        SetLineTimeCueWidth(node->AsInt());
    if((node = searchNode->GetNode("doCropTimeCue")) != 0)
        SetDoCropTimeCue(node->AsBool());
    if((node = searchNode->GetNode("timeForTimeCue")) != 0)
        SetTimeForTimeCue(node->AsDouble());
    if((node = searchNode->GetNode("fillMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetFillMode(CurveFillMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CurveFillMode value;
            if(CurveFillMode_FromString(node->AsString(), value))
                SetFillMode(value);
        }
    }
    if((node = searchNode->GetNode("fillColor1")) != 0)
        fillColor1.SetFromNode(node);
    if((node = searchNode->GetNode("fillColor2")) != 0)
        fillColor2.SetFromNode(node);
    if((node = searchNode->GetNode("polarToCartesian")) != 0)
        SetPolarToCartesian(node->AsBool());
    if((node = searchNode->GetNode("polarCoordinateOrder")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetPolarCoordinateOrder(PolarCoordinateOrder(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PolarCoordinateOrder value;
            if(PolarCoordinateOrder_FromString(node->AsString(), value))
                SetPolarCoordinateOrder(value);
        }
    }
    if((node = searchNode->GetNode("angleUnits")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetAngleUnits(AngleUnits(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            AngleUnits value;
            if(AngleUnits_FromString(node->AsString(), value))
                SetAngleUnits(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
CurveAttributes::SetShowLines(bool showLines_)
{
    showLines = showLines_;
    Select(ID_showLines, (void *)&showLines);
}

void
CurveAttributes::SetLineStyle(int lineStyle_)
{
    lineStyle = lineStyle_;
    Select(ID_lineStyle, (void *)&lineStyle);
}

void
CurveAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
CurveAttributes::SetShowPoints(bool showPoints_)
{
    showPoints = showPoints_;
    Select(ID_showPoints, (void *)&showPoints);
}

void
CurveAttributes::SetSymbol(CurveAttributes::SymbolTypes symbol_)
{
    symbol = symbol_;
    Select(ID_symbol, (void *)&symbol);
}

void
CurveAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(ID_pointSize, (void *)&pointSize);
}

void
CurveAttributes::SetPointFillMode(CurveAttributes::FillMode pointFillMode_)
{
    pointFillMode = pointFillMode_;
    Select(ID_pointFillMode, (void *)&pointFillMode);
}

void
CurveAttributes::SetPointStride(int pointStride_)
{
    pointStride = pointStride_;
    Select(ID_pointStride, (void *)&pointStride);
}

void
CurveAttributes::SetSymbolDensity(int symbolDensity_)
{
    symbolDensity = symbolDensity_;
    Select(ID_symbolDensity, (void *)&symbolDensity);
}

void
CurveAttributes::SetCurveColorSource(CurveAttributes::CurveColor curveColorSource_)
{
    curveColorSource = curveColorSource_;
    Select(ID_curveColorSource, (void *)&curveColorSource);
}

void
CurveAttributes::SetCurveColor(const ColorAttribute &curveColor_)
{
    curveColor = curveColor_;
    Select(ID_curveColor, (void *)&curveColor);
}

void
CurveAttributes::SetShowLegend(bool showLegend_)
{
    showLegend = showLegend_;
    Select(ID_showLegend, (void *)&showLegend);
}

void
CurveAttributes::SetShowLabels(bool showLabels_)
{
    showLabels = showLabels_;
    Select(ID_showLabels, (void *)&showLabels);
}

void
CurveAttributes::SetDesignator(const std::string &designator_)
{
    designator = designator_;
    Select(ID_designator, (void *)&designator);
}

void
CurveAttributes::SetDoBallTimeCue(bool doBallTimeCue_)
{
    doBallTimeCue = doBallTimeCue_;
    Select(ID_doBallTimeCue, (void *)&doBallTimeCue);
}

void
CurveAttributes::SetBallTimeCueColor(const ColorAttribute &ballTimeCueColor_)
{
    ballTimeCueColor = ballTimeCueColor_;
    Select(ID_ballTimeCueColor, (void *)&ballTimeCueColor);
}

void
CurveAttributes::SetTimeCueBallSize(double timeCueBallSize_)
{
    timeCueBallSize = timeCueBallSize_;
    Select(ID_timeCueBallSize, (void *)&timeCueBallSize);
}

void
CurveAttributes::SetDoLineTimeCue(bool doLineTimeCue_)
{
    doLineTimeCue = doLineTimeCue_;
    Select(ID_doLineTimeCue, (void *)&doLineTimeCue);
}

void
CurveAttributes::SetLineTimeCueColor(const ColorAttribute &lineTimeCueColor_)
{
    lineTimeCueColor = lineTimeCueColor_;
    Select(ID_lineTimeCueColor, (void *)&lineTimeCueColor);
}

void
CurveAttributes::SetLineTimeCueWidth(int lineTimeCueWidth_)
{
    lineTimeCueWidth = lineTimeCueWidth_;
    Select(ID_lineTimeCueWidth, (void *)&lineTimeCueWidth);
}

void
CurveAttributes::SetDoCropTimeCue(bool doCropTimeCue_)
{
    doCropTimeCue = doCropTimeCue_;
    Select(ID_doCropTimeCue, (void *)&doCropTimeCue);
}

void
CurveAttributes::SetTimeForTimeCue(double timeForTimeCue_)
{
    timeForTimeCue = timeForTimeCue_;
    Select(ID_timeForTimeCue, (void *)&timeForTimeCue);
}

void
CurveAttributes::SetFillMode(CurveAttributes::CurveFillMode fillMode_)
{
    fillMode = fillMode_;
    Select(ID_fillMode, (void *)&fillMode);
}

void
CurveAttributes::SetFillColor1(const ColorAttribute &fillColor1_)
{
    fillColor1 = fillColor1_;
    Select(ID_fillColor1, (void *)&fillColor1);
}

void
CurveAttributes::SetFillColor2(const ColorAttribute &fillColor2_)
{
    fillColor2 = fillColor2_;
    Select(ID_fillColor2, (void *)&fillColor2);
}

void
CurveAttributes::SetPolarToCartesian(bool polarToCartesian_)
{
    polarToCartesian = polarToCartesian_;
    Select(ID_polarToCartesian, (void *)&polarToCartesian);
}

void
CurveAttributes::SetPolarCoordinateOrder(CurveAttributes::PolarCoordinateOrder polarCoordinateOrder_)
{
    polarCoordinateOrder = polarCoordinateOrder_;
    Select(ID_polarCoordinateOrder, (void *)&polarCoordinateOrder);
}

void
CurveAttributes::SetAngleUnits(CurveAttributes::AngleUnits angleUnits_)
{
    angleUnits = angleUnits_;
    Select(ID_angleUnits, (void *)&angleUnits);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
CurveAttributes::GetShowLines() const
{
    return showLines;
}

int
CurveAttributes::GetLineStyle() const
{
    return lineStyle;
}

int
CurveAttributes::GetLineWidth() const
{
    return lineWidth;
}

bool
CurveAttributes::GetShowPoints() const
{
    return showPoints;
}

CurveAttributes::SymbolTypes
CurveAttributes::GetSymbol() const
{
    return SymbolTypes(symbol);
}

double
CurveAttributes::GetPointSize() const
{
    return pointSize;
}

CurveAttributes::FillMode
CurveAttributes::GetPointFillMode() const
{
    return FillMode(pointFillMode);
}

int
CurveAttributes::GetPointStride() const
{
    return pointStride;
}

int
CurveAttributes::GetSymbolDensity() const
{
    return symbolDensity;
}

CurveAttributes::CurveColor
CurveAttributes::GetCurveColorSource() const
{
    return CurveColor(curveColorSource);
}

const ColorAttribute &
CurveAttributes::GetCurveColor() const
{
    return curveColor;
}

ColorAttribute &
CurveAttributes::GetCurveColor()
{
    return curveColor;
}

bool
CurveAttributes::GetShowLegend() const
{
    return showLegend;
}

bool
CurveAttributes::GetShowLabels() const
{
    return showLabels;
}

const std::string &
CurveAttributes::GetDesignator() const
{
    return designator;
}

std::string &
CurveAttributes::GetDesignator()
{
    return designator;
}

bool
CurveAttributes::GetDoBallTimeCue() const
{
    return doBallTimeCue;
}

const ColorAttribute &
CurveAttributes::GetBallTimeCueColor() const
{
    return ballTimeCueColor;
}

ColorAttribute &
CurveAttributes::GetBallTimeCueColor()
{
    return ballTimeCueColor;
}

double
CurveAttributes::GetTimeCueBallSize() const
{
    return timeCueBallSize;
}

bool
CurveAttributes::GetDoLineTimeCue() const
{
    return doLineTimeCue;
}

const ColorAttribute &
CurveAttributes::GetLineTimeCueColor() const
{
    return lineTimeCueColor;
}

ColorAttribute &
CurveAttributes::GetLineTimeCueColor()
{
    return lineTimeCueColor;
}

int
CurveAttributes::GetLineTimeCueWidth() const
{
    return lineTimeCueWidth;
}

bool
CurveAttributes::GetDoCropTimeCue() const
{
    return doCropTimeCue;
}

double
CurveAttributes::GetTimeForTimeCue() const
{
    return timeForTimeCue;
}

CurveAttributes::CurveFillMode
CurveAttributes::GetFillMode() const
{
    return CurveFillMode(fillMode);
}

const ColorAttribute &
CurveAttributes::GetFillColor1() const
{
    return fillColor1;
}

ColorAttribute &
CurveAttributes::GetFillColor1()
{
    return fillColor1;
}

const ColorAttribute &
CurveAttributes::GetFillColor2() const
{
    return fillColor2;
}

ColorAttribute &
CurveAttributes::GetFillColor2()
{
    return fillColor2;
}

bool
CurveAttributes::GetPolarToCartesian() const
{
    return polarToCartesian;
}

CurveAttributes::PolarCoordinateOrder
CurveAttributes::GetPolarCoordinateOrder() const
{
    return PolarCoordinateOrder(polarCoordinateOrder);
}

CurveAttributes::AngleUnits
CurveAttributes::GetAngleUnits() const
{
    return AngleUnits(angleUnits);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
CurveAttributes::SelectCurveColor()
{
    Select(ID_curveColor, (void *)&curveColor);
}

void
CurveAttributes::SelectDesignator()
{
    Select(ID_designator, (void *)&designator);
}

void
CurveAttributes::SelectBallTimeCueColor()
{
    Select(ID_ballTimeCueColor, (void *)&ballTimeCueColor);
}

void
CurveAttributes::SelectLineTimeCueColor()
{
    Select(ID_lineTimeCueColor, (void *)&lineTimeCueColor);
}

void
CurveAttributes::SelectFillColor1()
{
    Select(ID_fillColor1, (void *)&fillColor1);
}

void
CurveAttributes::SelectFillColor2()
{
    Select(ID_fillColor2, (void *)&fillColor2);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CurveAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
CurveAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_showLines:            return "showLines";
    case ID_lineStyle:            return "lineStyle";
    case ID_lineWidth:            return "lineWidth";
    case ID_showPoints:           return "showPoints";
    case ID_symbol:               return "symbol";
    case ID_pointSize:            return "pointSize";
    case ID_pointFillMode:        return "pointFillMode";
    case ID_pointStride:          return "pointStride";
    case ID_symbolDensity:        return "symbolDensity";
    case ID_curveColorSource:     return "curveColorSource";
    case ID_curveColor:           return "curveColor";
    case ID_showLegend:           return "showLegend";
    case ID_showLabels:           return "showLabels";
    case ID_designator:           return "designator";
    case ID_doBallTimeCue:        return "doBallTimeCue";
    case ID_ballTimeCueColor:     return "ballTimeCueColor";
    case ID_timeCueBallSize:      return "timeCueBallSize";
    case ID_doLineTimeCue:        return "doLineTimeCue";
    case ID_lineTimeCueColor:     return "lineTimeCueColor";
    case ID_lineTimeCueWidth:     return "lineTimeCueWidth";
    case ID_doCropTimeCue:        return "doCropTimeCue";
    case ID_timeForTimeCue:       return "timeForTimeCue";
    case ID_fillMode:             return "fillMode";
    case ID_fillColor1:           return "fillColor1";
    case ID_fillColor2:           return "fillColor2";
    case ID_polarToCartesian:     return "polarToCartesian";
    case ID_polarCoordinateOrder: return "polarCoordinateOrder";
    case ID_angleUnits:           return "angleUnits";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: CurveAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
CurveAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_showLines:            return FieldType_bool;
    case ID_lineStyle:            return FieldType_linestyle;
    case ID_lineWidth:            return FieldType_linewidth;
    case ID_showPoints:           return FieldType_bool;
    case ID_symbol:               return FieldType_enum;
    case ID_pointSize:            return FieldType_double;
    case ID_pointFillMode:        return FieldType_enum;
    case ID_pointStride:          return FieldType_int;
    case ID_symbolDensity:        return FieldType_int;
    case ID_curveColorSource:     return FieldType_enum;
    case ID_curveColor:           return FieldType_color;
    case ID_showLegend:           return FieldType_bool;
    case ID_showLabels:           return FieldType_bool;
    case ID_designator:           return FieldType_string;
    case ID_doBallTimeCue:        return FieldType_bool;
    case ID_ballTimeCueColor:     return FieldType_color;
    case ID_timeCueBallSize:      return FieldType_double;
    case ID_doLineTimeCue:        return FieldType_bool;
    case ID_lineTimeCueColor:     return FieldType_color;
    case ID_lineTimeCueWidth:     return FieldType_int;
    case ID_doCropTimeCue:        return FieldType_bool;
    case ID_timeForTimeCue:       return FieldType_double;
    case ID_fillMode:             return FieldType_enum;
    case ID_fillColor1:           return FieldType_color;
    case ID_fillColor2:           return FieldType_color;
    case ID_polarToCartesian:     return FieldType_bool;
    case ID_polarCoordinateOrder: return FieldType_enum;
    case ID_angleUnits:           return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: CurveAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
CurveAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_showLines:            return "bool";
    case ID_lineStyle:            return "linestyle";
    case ID_lineWidth:            return "linewidth";
    case ID_showPoints:           return "bool";
    case ID_symbol:               return "enum";
    case ID_pointSize:            return "double";
    case ID_pointFillMode:        return "enum";
    case ID_pointStride:          return "int";
    case ID_symbolDensity:        return "int";
    case ID_curveColorSource:     return "enum";
    case ID_curveColor:           return "color";
    case ID_showLegend:           return "bool";
    case ID_showLabels:           return "bool";
    case ID_designator:           return "string";
    case ID_doBallTimeCue:        return "bool";
    case ID_ballTimeCueColor:     return "color";
    case ID_timeCueBallSize:      return "double";
    case ID_doLineTimeCue:        return "bool";
    case ID_lineTimeCueColor:     return "color";
    case ID_lineTimeCueWidth:     return "int";
    case ID_doCropTimeCue:        return "bool";
    case ID_timeForTimeCue:       return "double";
    case ID_fillMode:             return "enum";
    case ID_fillColor1:           return "color";
    case ID_fillColor2:           return "color";
    case ID_polarToCartesian:     return "bool";
    case ID_polarCoordinateOrder: return "enum";
    case ID_angleUnits:           return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: CurveAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
CurveAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const CurveAttributes &obj = *((const CurveAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_showLines:
        {  // new scope
        retval = (showLines == obj.showLines);
        }
        break;
    case ID_lineStyle:
        {  // new scope
        retval = (lineStyle == obj.lineStyle);
        }
        break;
    case ID_lineWidth:
        {  // new scope
        retval = (lineWidth == obj.lineWidth);
        }
        break;
    case ID_showPoints:
        {  // new scope
        retval = (showPoints == obj.showPoints);
        }
        break;
    case ID_symbol:
        {  // new scope
        retval = (symbol == obj.symbol);
        }
        break;
    case ID_pointSize:
        {  // new scope
        retval = (pointSize == obj.pointSize);
        }
        break;
    case ID_pointFillMode:
        {  // new scope
        retval = (pointFillMode == obj.pointFillMode);
        }
        break;
    case ID_pointStride:
        {  // new scope
        retval = (pointStride == obj.pointStride);
        }
        break;
    case ID_symbolDensity:
        {  // new scope
        retval = (symbolDensity == obj.symbolDensity);
        }
        break;
    case ID_curveColorSource:
        {  // new scope
        retval = (curveColorSource == obj.curveColorSource);
        }
        break;
    case ID_curveColor:
        {  // new scope
        retval = (curveColor == obj.curveColor);
        }
        break;
    case ID_showLegend:
        {  // new scope
        retval = (showLegend == obj.showLegend);
        }
        break;
    case ID_showLabels:
        {  // new scope
        retval = (showLabels == obj.showLabels);
        }
        break;
    case ID_designator:
        {  // new scope
        retval = (designator == obj.designator);
        }
        break;
    case ID_doBallTimeCue:
        {  // new scope
        retval = (doBallTimeCue == obj.doBallTimeCue);
        }
        break;
    case ID_ballTimeCueColor:
        {  // new scope
        retval = (ballTimeCueColor == obj.ballTimeCueColor);
        }
        break;
    case ID_timeCueBallSize:
        {  // new scope
        retval = (timeCueBallSize == obj.timeCueBallSize);
        }
        break;
    case ID_doLineTimeCue:
        {  // new scope
        retval = (doLineTimeCue == obj.doLineTimeCue);
        }
        break;
    case ID_lineTimeCueColor:
        {  // new scope
        retval = (lineTimeCueColor == obj.lineTimeCueColor);
        }
        break;
    case ID_lineTimeCueWidth:
        {  // new scope
        retval = (lineTimeCueWidth == obj.lineTimeCueWidth);
        }
        break;
    case ID_doCropTimeCue:
        {  // new scope
        retval = (doCropTimeCue == obj.doCropTimeCue);
        }
        break;
    case ID_timeForTimeCue:
        {  // new scope
        retval = (timeForTimeCue == obj.timeForTimeCue);
        }
        break;
    case ID_fillMode:
        {  // new scope
        retval = (fillMode == obj.fillMode);
        }
        break;
    case ID_fillColor1:
        {  // new scope
        retval = (fillColor1 == obj.fillColor1);
        }
        break;
    case ID_fillColor2:
        {  // new scope
        retval = (fillColor2 == obj.fillColor2);
        }
        break;
    case ID_polarToCartesian:
        {  // new scope
        retval = (polarToCartesian == obj.polarToCartesian);
        }
        break;
    case ID_polarCoordinateOrder:
        {  // new scope
        retval = (polarCoordinateOrder == obj.polarCoordinateOrder);
        }
        break;
    case ID_angleUnits:
        {  // new scope
        retval = (angleUnits == obj.angleUnits);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
//
// Modifications:
//    Kathleen Biagas, Wed Sep 11 17:16:26 PDT 2013
//    Return true if converting coordinates.
//
// ****************************************************************************

bool
CurveAttributes::ChangesRequireRecalculation(const CurveAttributes &obj) const
{
    if (polarToCartesian != obj.polarToCartesian) return true;
    if (angleUnits != obj.angleUnits && obj.polarToCartesian) return true;
    if (polarCoordinateOrder != obj.polarCoordinateOrder && obj.polarToCartesian) return true;
    return false;
}

// ****************************************************************************
// Method: CurveAttributes::ProcessOldVersions
//
// Purpose: 
//   This method creates modifies a DataNode representation of the object
//   so it conforms to the newest representation of the object, which can
//   can be read back in.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2010
//
// Modifications:
//
// ****************************************************************************

void
CurveAttributes::ProcessOldVersions(DataNode *parentNode,
                                         const char *configVersion)
{
    if (parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("CurveAttributes");
    if (searchNode == 0)
        return;

    if (VersionLessThan(configVersion, "2.1.0"))
    {
        DataNode *k = 0;
        if (( k = searchNode->GetNode("renderMode")) != 0)
        {
            std::string mode = k->AsString();
            searchNode->RemoveNode(k, true);
            if (mode == "RenderAsLines") // asLines
            {
                searchNode->AddNode(new DataNode("showLines", true));
                searchNode->AddNode(new DataNode("pointFillMode", FillMode_ToString(CurveAttributes::Static)));
            }
            else
            {
                searchNode->AddNode(new DataNode("showLines", false));
                searchNode->AddNode(new DataNode("pointFillMode", FillMode_ToString(CurveAttributes::Dynamic)));
            }
        }
    }
}

