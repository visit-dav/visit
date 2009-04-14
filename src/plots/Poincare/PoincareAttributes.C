/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <PoincareAttributes.h>
#include <DataNode.h>
#include <PlaneAttributes.h>
#include <PointAttributes.h>
#include <Line.h>

//
// Enum conversion methods for PoincareAttributes::SourceType
//

static const char *SourceType_strings[] = {
"SpecifiedPoint", "SpecifiedLine", "SpecifiedPlane"
};

std::string
PoincareAttributes::SourceType_ToString(PoincareAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return SourceType_strings[index];
}

std::string
PoincareAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return SourceType_strings[index];
}

bool
PoincareAttributes::SourceType_FromString(const std::string &s, PoincareAttributes::SourceType &val)
{
    val = PoincareAttributes::SpecifiedPoint;
    for(int i = 0; i < 3; ++i)
    {
        if(s == SourceType_strings[i])
        {
            val = (SourceType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::TerminationType
//

static const char *TerminationType_strings[] = {
"Distance", "Time", "Step"
};

std::string
PoincareAttributes::TerminationType_ToString(PoincareAttributes::TerminationType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return TerminationType_strings[index];
}

std::string
PoincareAttributes::TerminationType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return TerminationType_strings[index];
}

bool
PoincareAttributes::TerminationType_FromString(const std::string &s, PoincareAttributes::TerminationType &val)
{
    val = PoincareAttributes::Distance;
    for(int i = 0; i < 3; ++i)
    {
        if(s == TerminationType_strings[i])
        {
            val = (TerminationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::ColorStyleType
//

static const char *ColorStyleType_strings[] = {
"OriginalValue", "InputOrder", "PointIndex", 
"Plane", "ToroidalWindingOrder", "ToroidalWindingPointOrder", 
"ToroidalWindings", "PoloidalWindings", "SafetyFactor"
};

std::string
PoincareAttributes::ColorStyleType_ToString(PoincareAttributes::ColorStyleType t)
{
    int index = int(t);
    if(index < 0 || index >= 9) index = 0;
    return ColorStyleType_strings[index];
}

std::string
PoincareAttributes::ColorStyleType_ToString(int t)
{
    int index = (t < 0 || t >= 9) ? 0 : t;
    return ColorStyleType_strings[index];
}

bool
PoincareAttributes::ColorStyleType_FromString(const std::string &s, PoincareAttributes::ColorStyleType &val)
{
    val = PoincareAttributes::OriginalValue;
    for(int i = 0; i < 9; ++i)
    {
        if(s == ColorStyleType_strings[i])
        {
            val = (ColorStyleType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::ShowMeshType
//

static const char *ShowMeshType_strings[] = {
"Curves", "Surfaces"};

std::string
PoincareAttributes::ShowMeshType_ToString(PoincareAttributes::ShowMeshType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ShowMeshType_strings[index];
}

std::string
PoincareAttributes::ShowMeshType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ShowMeshType_strings[index];
}

bool
PoincareAttributes::ShowMeshType_FromString(const std::string &s, PoincareAttributes::ShowMeshType &val)
{
    val = PoincareAttributes::Curves;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ShowMeshType_strings[i])
        {
            val = (ShowMeshType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::OverlapType
//

static const char *OverlapType_strings[] = {
"Raw", "Remove", "Merge", 
"Smooth"};

std::string
PoincareAttributes::OverlapType_ToString(PoincareAttributes::OverlapType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return OverlapType_strings[index];
}

std::string
PoincareAttributes::OverlapType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return OverlapType_strings[index];
}

bool
PoincareAttributes::OverlapType_FromString(const std::string &s, PoincareAttributes::OverlapType &val)
{
    val = PoincareAttributes::Raw;
    for(int i = 0; i < 4; ++i)
    {
        if(s == OverlapType_strings[i])
        {
            val = (OverlapType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::IntegrationType
//

static const char *IntegrationType_strings[] = {
"DormandPrince", "AdamsBashforth"};

std::string
PoincareAttributes::IntegrationType_ToString(PoincareAttributes::IntegrationType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return IntegrationType_strings[index];
}

std::string
PoincareAttributes::IntegrationType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return IntegrationType_strings[index];
}

bool
PoincareAttributes::IntegrationType_FromString(const std::string &s, PoincareAttributes::IntegrationType &val)
{
    val = PoincareAttributes::DormandPrince;
    for(int i = 0; i < 2; ++i)
    {
        if(s == IntegrationType_strings[i])
        {
            val = (IntegrationType)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *PoincareAttributes::TypeMapFormatString = "iddDDDDDDdisabbddiibbiiiidiibi";

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Constructor for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes() : 
    AttributeSubject(PoincareAttributes::TypeMapFormatString),
    colorTableName("Default"), singleColor(0, 0, 0)
{
    sourceType = SpecifiedPoint;
    maxStepLength = 0.1;
    termination = 10;
    pointSource[0] = 0;
    pointSource[1] = 0;
    pointSource[2] = 0;
    lineStart[0] = 0;
    lineStart[1] = 0;
    lineStart[2] = 0;
    lineEnd[0] = 1;
    lineEnd[1] = 0;
    lineEnd[2] = 0;
    planeOrigin[0] = 0;
    planeOrigin[1] = 0;
    planeOrigin[2] = 0;
    planeNormal[0] = 0;
    planeNormal[1] = 0;
    planeNormal[2] = 1;
    planeUpAxis[0] = 0;
    planeUpAxis[1] = 1;
    planeUpAxis[2] = 0;
    planeRadius = 1;
    pointDensity = 1;
    legendFlag = true;
    lightingFlag = true;
    relTol = 0.0001;
    absTol = 1e-05;
    terminationType = Step;
    integrationType = AdamsBashforth;
    showStreamlines = false;
    showPoints = false;
    NumberPlanes = 1;
    ColorStyle = SafetyFactor;
    MaxToroidalWinding = 30;
    OverrideToroidalWinding = 0;
    HitRate = 0.9;
    ShowCurves = Curves;
    AdjustPlane = -1;
    ShowIslands = false;
    Overlaps = Remove;
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Copy constructor for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes(const PoincareAttributes &obj) : 
    AttributeSubject(PoincareAttributes::TypeMapFormatString)
{
    sourceType = obj.sourceType;
    maxStepLength = obj.maxStepLength;
    termination = obj.termination;
    pointSource[0] = obj.pointSource[0];
    pointSource[1] = obj.pointSource[1];
    pointSource[2] = obj.pointSource[2];

    lineStart[0] = obj.lineStart[0];
    lineStart[1] = obj.lineStart[1];
    lineStart[2] = obj.lineStart[2];

    lineEnd[0] = obj.lineEnd[0];
    lineEnd[1] = obj.lineEnd[1];
    lineEnd[2] = obj.lineEnd[2];

    planeOrigin[0] = obj.planeOrigin[0];
    planeOrigin[1] = obj.planeOrigin[1];
    planeOrigin[2] = obj.planeOrigin[2];

    planeNormal[0] = obj.planeNormal[0];
    planeNormal[1] = obj.planeNormal[1];
    planeNormal[2] = obj.planeNormal[2];

    planeUpAxis[0] = obj.planeUpAxis[0];
    planeUpAxis[1] = obj.planeUpAxis[1];
    planeUpAxis[2] = obj.planeUpAxis[2];

    planeRadius = obj.planeRadius;
    pointDensity = obj.pointDensity;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    relTol = obj.relTol;
    absTol = obj.absTol;
    terminationType = obj.terminationType;
    integrationType = obj.integrationType;
    showStreamlines = obj.showStreamlines;
    showPoints = obj.showPoints;
    NumberPlanes = obj.NumberPlanes;
    ColorStyle = obj.ColorStyle;
    MaxToroidalWinding = obj.MaxToroidalWinding;
    OverrideToroidalWinding = obj.OverrideToroidalWinding;
    HitRate = obj.HitRate;
    ShowCurves = obj.ShowCurves;
    AdjustPlane = obj.AdjustPlane;
    ShowIslands = obj.ShowIslands;
    Overlaps = obj.Overlaps;

    SelectAll();
}

// ****************************************************************************
// Method: PoincareAttributes::~PoincareAttributes
//
// Purpose: 
//   Destructor for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::~PoincareAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: PoincareAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes& 
PoincareAttributes::operator = (const PoincareAttributes &obj)
{
    if (this == &obj) return *this;
    sourceType = obj.sourceType;
    maxStepLength = obj.maxStepLength;
    termination = obj.termination;
    pointSource[0] = obj.pointSource[0];
    pointSource[1] = obj.pointSource[1];
    pointSource[2] = obj.pointSource[2];

    lineStart[0] = obj.lineStart[0];
    lineStart[1] = obj.lineStart[1];
    lineStart[2] = obj.lineStart[2];

    lineEnd[0] = obj.lineEnd[0];
    lineEnd[1] = obj.lineEnd[1];
    lineEnd[2] = obj.lineEnd[2];

    planeOrigin[0] = obj.planeOrigin[0];
    planeOrigin[1] = obj.planeOrigin[1];
    planeOrigin[2] = obj.planeOrigin[2];

    planeNormal[0] = obj.planeNormal[0];
    planeNormal[1] = obj.planeNormal[1];
    planeNormal[2] = obj.planeNormal[2];

    planeUpAxis[0] = obj.planeUpAxis[0];
    planeUpAxis[1] = obj.planeUpAxis[1];
    planeUpAxis[2] = obj.planeUpAxis[2];

    planeRadius = obj.planeRadius;
    pointDensity = obj.pointDensity;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    relTol = obj.relTol;
    absTol = obj.absTol;
    terminationType = obj.terminationType;
    integrationType = obj.integrationType;
    showStreamlines = obj.showStreamlines;
    showPoints = obj.showPoints;
    NumberPlanes = obj.NumberPlanes;
    ColorStyle = obj.ColorStyle;
    MaxToroidalWinding = obj.MaxToroidalWinding;
    OverrideToroidalWinding = obj.OverrideToroidalWinding;
    HitRate = obj.HitRate;
    ShowCurves = obj.ShowCurves;
    AdjustPlane = obj.AdjustPlane;
    ShowIslands = obj.ShowIslands;
    Overlaps = obj.Overlaps;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: PoincareAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the PoincareAttributes class.
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
PoincareAttributes::operator == (const PoincareAttributes &obj) const
{
    // Compare the pointSource arrays.
    bool pointSource_equal = true;
    for(int i = 0; i < 3 && pointSource_equal; ++i)
        pointSource_equal = (pointSource[i] == obj.pointSource[i]);

    // Compare the lineStart arrays.
    bool lineStart_equal = true;
    for(int i = 0; i < 3 && lineStart_equal; ++i)
        lineStart_equal = (lineStart[i] == obj.lineStart[i]);

    // Compare the lineEnd arrays.
    bool lineEnd_equal = true;
    for(int i = 0; i < 3 && lineEnd_equal; ++i)
        lineEnd_equal = (lineEnd[i] == obj.lineEnd[i]);

    // Compare the planeOrigin arrays.
    bool planeOrigin_equal = true;
    for(int i = 0; i < 3 && planeOrigin_equal; ++i)
        planeOrigin_equal = (planeOrigin[i] == obj.planeOrigin[i]);

    // Compare the planeNormal arrays.
    bool planeNormal_equal = true;
    for(int i = 0; i < 3 && planeNormal_equal; ++i)
        planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

    // Compare the planeUpAxis arrays.
    bool planeUpAxis_equal = true;
    for(int i = 0; i < 3 && planeUpAxis_equal; ++i)
        planeUpAxis_equal = (planeUpAxis[i] == obj.planeUpAxis[i]);

    // Create the return value
    return ((sourceType == obj.sourceType) &&
            (maxStepLength == obj.maxStepLength) &&
            (termination == obj.termination) &&
            pointSource_equal &&
            lineStart_equal &&
            lineEnd_equal &&
            planeOrigin_equal &&
            planeNormal_equal &&
            planeUpAxis_equal &&
            (planeRadius == obj.planeRadius) &&
            (pointDensity == obj.pointDensity) &&
            (colorTableName == obj.colorTableName) &&
            (singleColor == obj.singleColor) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (relTol == obj.relTol) &&
            (absTol == obj.absTol) &&
            (terminationType == obj.terminationType) &&
            (integrationType == obj.integrationType) &&
            (showStreamlines == obj.showStreamlines) &&
            (showPoints == obj.showPoints) &&
            (NumberPlanes == obj.NumberPlanes) &&
            (ColorStyle == obj.ColorStyle) &&
            (MaxToroidalWinding == obj.MaxToroidalWinding) &&
            (OverrideToroidalWinding == obj.OverrideToroidalWinding) &&
            (HitRate == obj.HitRate) &&
            (ShowCurves == obj.ShowCurves) &&
            (AdjustPlane == obj.AdjustPlane) &&
            (ShowIslands == obj.ShowIslands) &&
            (Overlaps == obj.Overlaps));
}

// ****************************************************************************
// Method: PoincareAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the PoincareAttributes class.
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
PoincareAttributes::operator != (const PoincareAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: PoincareAttributes::TypeName
//
// Purpose: 
//   Type name method for the PoincareAttributes class.
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
PoincareAttributes::TypeName() const
{
    return "PoincareAttributes";
}

// ****************************************************************************
// Method: PoincareAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the PoincareAttributes class.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//    Jeremy Meredith, Wed Apr  8 16:48:05 EDT 2009
//    Initial steps to unification with streamline attributes.
// ****************************************************************************

bool
PoincareAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;
    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const PoincareAttributes *tmp = (const PoincareAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    else if(atts->TypeName() == "PointAttributes")
    {
        const PointAttributes *p = (PointAttributes *)atts;
        SetPointSource(p->GetPoint());
        retval = true;
    }
    else if(atts->TypeName() == "Line")
    {
        const Line *line = (const Line *)atts;
        SetLineStart(line->GetPoint1());
        SetLineEnd(line->GetPoint2());
        retval = true;
    }
    else if(atts->TypeName() == "PlaneAttributes")
    {
        const PlaneAttributes *plane = (const PlaneAttributes *)atts;
        SetPlaneOrigin(plane->GetOrigin());
        SetPlaneNormal(plane->GetNormal());
        SetPlaneUpAxis(plane->GetUpAxis());
        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: PoincareAttributes::CreateCompatible
//
// Purpose: 
//   Creates a new state object of the desired type.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//    Jeremy Meredith, Wed Apr  8 16:48:05 EDT 2009
//    Initial steps to unification with streamline attributes.
// ****************************************************************************

AttributeSubject *
PoincareAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if (TypeName() == tname)
    {
        retval = new PoincareAttributes(*this);
    }
    else if (tname == "PointAttributes")
    {
        PointAttributes *p = new PointAttributes;
        p->SetPoint(GetPointSource());
        retval = p;
    }
    else if (tname == "Line")
    {
        Line *l = new Line;
        l->SetPoint1(GetLineStart());
        l->SetPoint2(GetLineEnd());
        retval = l;
    }
    else if (tname == "PlaneAttributes")
    {
        PlaneAttributes *p = new PlaneAttributes;
        p->SetOrigin(GetPlaneOrigin());
        p->SetNormal(GetPlaneNormal());
        p->SetUpAxis(GetPlaneUpAxis());
        retval = p;
    }
    return retval;
}

// ****************************************************************************
// Method: PoincareAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the PoincareAttributes class.
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
PoincareAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new PoincareAttributes(*this);
    else
        retval = new PoincareAttributes;

    return retval;
}

// ****************************************************************************
// Method: PoincareAttributes::SelectAll
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
PoincareAttributes::SelectAll()
{
    Select(ID_sourceType,              (void *)&sourceType);
    Select(ID_maxStepLength,           (void *)&maxStepLength);
    Select(ID_termination,             (void *)&termination);
    Select(ID_pointSource,             (void *)pointSource, 3);
    Select(ID_lineStart,               (void *)lineStart, 3);
    Select(ID_lineEnd,                 (void *)lineEnd, 3);
    Select(ID_planeOrigin,             (void *)planeOrigin, 3);
    Select(ID_planeNormal,             (void *)planeNormal, 3);
    Select(ID_planeUpAxis,             (void *)planeUpAxis, 3);
    Select(ID_planeRadius,             (void *)&planeRadius);
    Select(ID_pointDensity,            (void *)&pointDensity);
    Select(ID_colorTableName,          (void *)&colorTableName);
    Select(ID_singleColor,             (void *)&singleColor);
    Select(ID_legendFlag,              (void *)&legendFlag);
    Select(ID_lightingFlag,            (void *)&lightingFlag);
    Select(ID_relTol,                  (void *)&relTol);
    Select(ID_absTol,                  (void *)&absTol);
    Select(ID_terminationType,         (void *)&terminationType);
    Select(ID_integrationType,         (void *)&integrationType);
    Select(ID_showStreamlines,         (void *)&showStreamlines);
    Select(ID_showPoints,              (void *)&showPoints);
    Select(ID_NumberPlanes,            (void *)&NumberPlanes);
    Select(ID_ColorStyle,              (void *)&ColorStyle);
    Select(ID_MaxToroidalWinding,      (void *)&MaxToroidalWinding);
    Select(ID_OverrideToroidalWinding, (void *)&OverrideToroidalWinding);
    Select(ID_HitRate,                 (void *)&HitRate);
    Select(ID_ShowCurves,              (void *)&ShowCurves);
    Select(ID_AdjustPlane,             (void *)&AdjustPlane);
    Select(ID_ShowIslands,             (void *)&ShowIslands);
    Select(ID_Overlaps,                (void *)&Overlaps);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PoincareAttributes::CreateNode
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
PoincareAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    PoincareAttributes defaultObject;
    bool addToParent = false;
    // Create a node for PoincareAttributes.
    DataNode *node = new DataNode("PoincareAttributes");

    if(completeSave || !FieldsEqual(ID_sourceType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sourceType", SourceType_ToString(sourceType)));
    }

    if(completeSave || !FieldsEqual(ID_maxStepLength, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxStepLength", maxStepLength));
    }

    if(completeSave || !FieldsEqual(ID_termination, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("termination", termination));
    }

    if(completeSave || !FieldsEqual(ID_pointSource, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSource", pointSource, 3));
    }

    if(completeSave || !FieldsEqual(ID_lineStart, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineStart", lineStart, 3));
    }

    if(completeSave || !FieldsEqual(ID_lineEnd, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineEnd", lineEnd, 3));
    }

    if(completeSave || !FieldsEqual(ID_planeOrigin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("planeOrigin", planeOrigin, 3));
    }

    if(completeSave || !FieldsEqual(ID_planeNormal, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("planeNormal", planeNormal, 3));
    }

    if(completeSave || !FieldsEqual(ID_planeUpAxis, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("planeUpAxis", planeUpAxis, 3));
    }

    if(completeSave || !FieldsEqual(ID_planeRadius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("planeRadius", planeRadius));
    }

    if(completeSave || !FieldsEqual(ID_pointDensity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointDensity", pointDensity));
    }

    if(completeSave || !FieldsEqual(ID_colorTableName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

        DataNode *singleColorNode = new DataNode("singleColor");
        if(singleColor.CreateNode(singleColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(singleColorNode);
        }
        else
            delete singleColorNode;
    if(completeSave || !FieldsEqual(ID_legendFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
    }

    if(completeSave || !FieldsEqual(ID_lightingFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lightingFlag", lightingFlag));
    }

    if(completeSave || !FieldsEqual(ID_relTol, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("relTol", relTol));
    }

    if(completeSave || !FieldsEqual(ID_absTol, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("absTol", absTol));
    }

    if(completeSave || !FieldsEqual(ID_terminationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminationType", TerminationType_ToString(terminationType)));
    }

    if(completeSave || !FieldsEqual(ID_integrationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationType", IntegrationType_ToString(integrationType)));
    }

    if(completeSave || !FieldsEqual(ID_showStreamlines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showStreamlines", showStreamlines));
    }

    if(completeSave || !FieldsEqual(ID_showPoints, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showPoints", showPoints));
    }

    if(completeSave || !FieldsEqual(ID_NumberPlanes, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("NumberPlanes", NumberPlanes));
    }

    if(completeSave || !FieldsEqual(ID_ColorStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ColorStyle", ColorStyleType_ToString(ColorStyle)));
    }

    if(completeSave || !FieldsEqual(ID_MaxToroidalWinding, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("MaxToroidalWinding", MaxToroidalWinding));
    }

    if(completeSave || !FieldsEqual(ID_OverrideToroidalWinding, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OverrideToroidalWinding", OverrideToroidalWinding));
    }

    if(completeSave || !FieldsEqual(ID_HitRate, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("HitRate", HitRate));
    }

    if(completeSave || !FieldsEqual(ID_ShowCurves, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ShowCurves", ShowMeshType_ToString(ShowCurves)));
    }

    if(completeSave || !FieldsEqual(ID_AdjustPlane, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("AdjustPlane", AdjustPlane));
    }

    if(completeSave || !FieldsEqual(ID_ShowIslands, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ShowIslands", ShowIslands));
    }

    if(completeSave || !FieldsEqual(ID_Overlaps, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("Overlaps", OverlapType_ToString(Overlaps)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: PoincareAttributes::SetFromNode
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
PoincareAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("PoincareAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("sourceType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetSourceType(SourceType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SourceType value;
            if(SourceType_FromString(node->AsString(), value))
                SetSourceType(value);
        }
    }
    if((node = searchNode->GetNode("maxStepLength")) != 0)
        SetMaxStepLength(node->AsDouble());
    if((node = searchNode->GetNode("termination")) != 0)
        SetTermination(node->AsDouble());
    if((node = searchNode->GetNode("pointSource")) != 0)
        SetPointSource(node->AsDoubleArray());
    if((node = searchNode->GetNode("lineStart")) != 0)
        SetLineStart(node->AsDoubleArray());
    if((node = searchNode->GetNode("lineEnd")) != 0)
        SetLineEnd(node->AsDoubleArray());
    if((node = searchNode->GetNode("planeOrigin")) != 0)
        SetPlaneOrigin(node->AsDoubleArray());
    if((node = searchNode->GetNode("planeNormal")) != 0)
        SetPlaneNormal(node->AsDoubleArray());
    if((node = searchNode->GetNode("planeUpAxis")) != 0)
        SetPlaneUpAxis(node->AsDoubleArray());
    if((node = searchNode->GetNode("planeRadius")) != 0)
        SetPlaneRadius(node->AsDouble());
    if((node = searchNode->GetNode("pointDensity")) != 0)
        SetPointDensity(node->AsInt());
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("relTol")) != 0)
        SetRelTol(node->AsDouble());
    if((node = searchNode->GetNode("absTol")) != 0)
        SetAbsTol(node->AsDouble());
    if((node = searchNode->GetNode("terminationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetTerminationType(TerminationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TerminationType value;
            if(TerminationType_FromString(node->AsString(), value))
                SetTerminationType(value);
        }
    }
    if((node = searchNode->GetNode("integrationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetIntegrationType(IntegrationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationType value;
            if(IntegrationType_FromString(node->AsString(), value))
                SetIntegrationType(value);
        }
    }
    if((node = searchNode->GetNode("showStreamlines")) != 0)
        SetShowStreamlines(node->AsBool());
    if((node = searchNode->GetNode("showPoints")) != 0)
        SetShowPoints(node->AsBool());
    if((node = searchNode->GetNode("NumberPlanes")) != 0)
        SetNumberPlanes(node->AsInt());
    if((node = searchNode->GetNode("ColorStyle")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 9)
                SetColorStyle(ColorStyleType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ColorStyleType value;
            if(ColorStyleType_FromString(node->AsString(), value))
                SetColorStyle(value);
        }
    }
    if((node = searchNode->GetNode("MaxToroidalWinding")) != 0)
        SetMaxToroidalWinding(node->AsInt());
    if((node = searchNode->GetNode("OverrideToroidalWinding")) != 0)
        SetOverrideToroidalWinding(node->AsInt());
    if((node = searchNode->GetNode("HitRate")) != 0)
        SetHitRate(node->AsDouble());
    if((node = searchNode->GetNode("ShowCurves")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetShowCurves(ShowMeshType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ShowMeshType value;
            if(ShowMeshType_FromString(node->AsString(), value))
                SetShowCurves(value);
        }
    }
    if((node = searchNode->GetNode("AdjustPlane")) != 0)
        SetAdjustPlane(node->AsInt());
    if((node = searchNode->GetNode("ShowIslands")) != 0)
        SetShowIslands(node->AsBool());
    if((node = searchNode->GetNode("Overlaps")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetOverlaps(OverlapType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OverlapType value;
            if(OverlapType_FromString(node->AsString(), value))
                SetOverlaps(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
PoincareAttributes::SetSourceType(PoincareAttributes::SourceType sourceType_)
{
    sourceType = sourceType_;
    Select(ID_sourceType, (void *)&sourceType);
}

void
PoincareAttributes::SetMaxStepLength(double maxStepLength_)
{
    maxStepLength = maxStepLength_;
    Select(ID_maxStepLength, (void *)&maxStepLength);
}

void
PoincareAttributes::SetTermination(double termination_)
{
    termination = termination_;
    Select(ID_termination, (void *)&termination);
}

void
PoincareAttributes::SetPointSource(const double *pointSource_)
{
    pointSource[0] = pointSource_[0];
    pointSource[1] = pointSource_[1];
    pointSource[2] = pointSource_[2];
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
PoincareAttributes::SetLineStart(const double *lineStart_)
{
    lineStart[0] = lineStart_[0];
    lineStart[1] = lineStart_[1];
    lineStart[2] = lineStart_[2];
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
PoincareAttributes::SetLineEnd(const double *lineEnd_)
{
    lineEnd[0] = lineEnd_[0];
    lineEnd[1] = lineEnd_[1];
    lineEnd[2] = lineEnd_[2];
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
PoincareAttributes::SetPlaneOrigin(const double *planeOrigin_)
{
    planeOrigin[0] = planeOrigin_[0];
    planeOrigin[1] = planeOrigin_[1];
    planeOrigin[2] = planeOrigin_[2];
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
PoincareAttributes::SetPlaneNormal(const double *planeNormal_)
{
    planeNormal[0] = planeNormal_[0];
    planeNormal[1] = planeNormal_[1];
    planeNormal[2] = planeNormal_[2];
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
PoincareAttributes::SetPlaneUpAxis(const double *planeUpAxis_)
{
    planeUpAxis[0] = planeUpAxis_[0];
    planeUpAxis[1] = planeUpAxis_[1];
    planeUpAxis[2] = planeUpAxis_[2];
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
PoincareAttributes::SetPlaneRadius(double planeRadius_)
{
    planeRadius = planeRadius_;
    Select(ID_planeRadius, (void *)&planeRadius);
}

void
PoincareAttributes::SetPointDensity(int pointDensity_)
{
    pointDensity = pointDensity_;
    Select(ID_pointDensity, (void *)&pointDensity);
}

void
PoincareAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PoincareAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
PoincareAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
PoincareAttributes::SetLightingFlag(bool lightingFlag_)
{
    lightingFlag = lightingFlag_;
    Select(ID_lightingFlag, (void *)&lightingFlag);
}

void
PoincareAttributes::SetRelTol(double relTol_)
{
    relTol = relTol_;
    Select(ID_relTol, (void *)&relTol);
}

void
PoincareAttributes::SetAbsTol(double absTol_)
{
    absTol = absTol_;
    Select(ID_absTol, (void *)&absTol);
}

void
PoincareAttributes::SetTerminationType(PoincareAttributes::TerminationType terminationType_)
{
    terminationType = terminationType_;
    Select(ID_terminationType, (void *)&terminationType);
}

void
PoincareAttributes::SetIntegrationType(PoincareAttributes::IntegrationType integrationType_)
{
    integrationType = integrationType_;
    Select(ID_integrationType, (void *)&integrationType);
}

void
PoincareAttributes::SetShowStreamlines(bool showStreamlines_)
{
    showStreamlines = showStreamlines_;
    Select(ID_showStreamlines, (void *)&showStreamlines);
}

void
PoincareAttributes::SetShowPoints(bool showPoints_)
{
    showPoints = showPoints_;
    Select(ID_showPoints, (void *)&showPoints);
}

void
PoincareAttributes::SetNumberPlanes(int NumberPlanes_)
{
    NumberPlanes = NumberPlanes_;
    Select(ID_NumberPlanes, (void *)&NumberPlanes);
}

void
PoincareAttributes::SetColorStyle(PoincareAttributes::ColorStyleType ColorStyle_)
{
    ColorStyle = ColorStyle_;
    Select(ID_ColorStyle, (void *)&ColorStyle);
}

void
PoincareAttributes::SetMaxToroidalWinding(int MaxToroidalWinding_)
{
    MaxToroidalWinding = MaxToroidalWinding_;
    Select(ID_MaxToroidalWinding, (void *)&MaxToroidalWinding);
}

void
PoincareAttributes::SetOverrideToroidalWinding(int OverrideToroidalWinding_)
{
    OverrideToroidalWinding = OverrideToroidalWinding_;
    Select(ID_OverrideToroidalWinding, (void *)&OverrideToroidalWinding);
}

void
PoincareAttributes::SetHitRate(double HitRate_)
{
    HitRate = HitRate_;
    Select(ID_HitRate, (void *)&HitRate);
}

void
PoincareAttributes::SetShowCurves(PoincareAttributes::ShowMeshType ShowCurves_)
{
    ShowCurves = ShowCurves_;
    Select(ID_ShowCurves, (void *)&ShowCurves);
}

void
PoincareAttributes::SetAdjustPlane(int AdjustPlane_)
{
    AdjustPlane = AdjustPlane_;
    Select(ID_AdjustPlane, (void *)&AdjustPlane);
}

void
PoincareAttributes::SetShowIslands(bool ShowIslands_)
{
    ShowIslands = ShowIslands_;
    Select(ID_ShowIslands, (void *)&ShowIslands);
}

void
PoincareAttributes::SetOverlaps(PoincareAttributes::OverlapType Overlaps_)
{
    Overlaps = Overlaps_;
    Select(ID_Overlaps, (void *)&Overlaps);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

PoincareAttributes::SourceType
PoincareAttributes::GetSourceType() const
{
    return SourceType(sourceType);
}

double
PoincareAttributes::GetMaxStepLength() const
{
    return maxStepLength;
}

double
PoincareAttributes::GetTermination() const
{
    return termination;
}

const double *
PoincareAttributes::GetPointSource() const
{
    return pointSource;
}

double *
PoincareAttributes::GetPointSource()
{
    return pointSource;
}

const double *
PoincareAttributes::GetLineStart() const
{
    return lineStart;
}

double *
PoincareAttributes::GetLineStart()
{
    return lineStart;
}

const double *
PoincareAttributes::GetLineEnd() const
{
    return lineEnd;
}

double *
PoincareAttributes::GetLineEnd()
{
    return lineEnd;
}

const double *
PoincareAttributes::GetPlaneOrigin() const
{
    return planeOrigin;
}

double *
PoincareAttributes::GetPlaneOrigin()
{
    return planeOrigin;
}

const double *
PoincareAttributes::GetPlaneNormal() const
{
    return planeNormal;
}

double *
PoincareAttributes::GetPlaneNormal()
{
    return planeNormal;
}

const double *
PoincareAttributes::GetPlaneUpAxis() const
{
    return planeUpAxis;
}

double *
PoincareAttributes::GetPlaneUpAxis()
{
    return planeUpAxis;
}

double
PoincareAttributes::GetPlaneRadius() const
{
    return planeRadius;
}

int
PoincareAttributes::GetPointDensity() const
{
    return pointDensity;
}

const std::string &
PoincareAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
PoincareAttributes::GetColorTableName()
{
    return colorTableName;
}

const ColorAttribute &
PoincareAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
PoincareAttributes::GetSingleColor()
{
    return singleColor;
}

bool
PoincareAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
PoincareAttributes::GetLightingFlag() const
{
    return lightingFlag;
}

double
PoincareAttributes::GetRelTol() const
{
    return relTol;
}

double
PoincareAttributes::GetAbsTol() const
{
    return absTol;
}

PoincareAttributes::TerminationType
PoincareAttributes::GetTerminationType() const
{
    return TerminationType(terminationType);
}

PoincareAttributes::IntegrationType
PoincareAttributes::GetIntegrationType() const
{
    return IntegrationType(integrationType);
}

bool
PoincareAttributes::GetShowStreamlines() const
{
    return showStreamlines;
}

bool
PoincareAttributes::GetShowPoints() const
{
    return showPoints;
}

int
PoincareAttributes::GetNumberPlanes() const
{
    return NumberPlanes;
}

PoincareAttributes::ColorStyleType
PoincareAttributes::GetColorStyle() const
{
    return ColorStyleType(ColorStyle);
}

int
PoincareAttributes::GetMaxToroidalWinding() const
{
    return MaxToroidalWinding;
}

int
PoincareAttributes::GetOverrideToroidalWinding() const
{
    return OverrideToroidalWinding;
}

double
PoincareAttributes::GetHitRate() const
{
    return HitRate;
}

PoincareAttributes::ShowMeshType
PoincareAttributes::GetShowCurves() const
{
    return ShowMeshType(ShowCurves);
}

int
PoincareAttributes::GetAdjustPlane() const
{
    return AdjustPlane;
}

bool
PoincareAttributes::GetShowIslands() const
{
    return ShowIslands;
}

PoincareAttributes::OverlapType
PoincareAttributes::GetOverlaps() const
{
    return OverlapType(Overlaps);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
PoincareAttributes::SelectPointSource()
{
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
PoincareAttributes::SelectLineStart()
{
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
PoincareAttributes::SelectLineEnd()
{
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
PoincareAttributes::SelectPlaneOrigin()
{
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
PoincareAttributes::SelectPlaneNormal()
{
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
PoincareAttributes::SelectPlaneUpAxis()
{
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
PoincareAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PoincareAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PoincareAttributes::GetFieldName
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
PoincareAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_sourceType:              return "sourceType";
    case ID_maxStepLength:           return "maxStepLength";
    case ID_termination:             return "termination";
    case ID_pointSource:             return "pointSource";
    case ID_lineStart:               return "lineStart";
    case ID_lineEnd:                 return "lineEnd";
    case ID_planeOrigin:             return "planeOrigin";
    case ID_planeNormal:             return "planeNormal";
    case ID_planeUpAxis:             return "planeUpAxis";
    case ID_planeRadius:             return "planeRadius";
    case ID_pointDensity:            return "pointDensity";
    case ID_colorTableName:          return "colorTableName";
    case ID_singleColor:             return "singleColor";
    case ID_legendFlag:              return "legendFlag";
    case ID_lightingFlag:            return "lightingFlag";
    case ID_relTol:                  return "relTol";
    case ID_absTol:                  return "absTol";
    case ID_terminationType:         return "terminationType";
    case ID_integrationType:         return "integrationType";
    case ID_showStreamlines:         return "showStreamlines";
    case ID_showPoints:              return "showPoints";
    case ID_NumberPlanes:            return "NumberPlanes";
    case ID_ColorStyle:              return "ColorStyle";
    case ID_MaxToroidalWinding:      return "MaxToroidalWinding";
    case ID_OverrideToroidalWinding: return "OverrideToroidalWinding";
    case ID_HitRate:                 return "HitRate";
    case ID_ShowCurves:              return "ShowCurves";
    case ID_AdjustPlane:             return "AdjustPlane";
    case ID_ShowIslands:             return "ShowIslands";
    case ID_Overlaps:                return "Overlaps";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PoincareAttributes::GetFieldType
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
PoincareAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_sourceType:              return FieldType_enum;
    case ID_maxStepLength:           return FieldType_double;
    case ID_termination:             return FieldType_double;
    case ID_pointSource:             return FieldType_doubleArray;
    case ID_lineStart:               return FieldType_doubleArray;
    case ID_lineEnd:                 return FieldType_doubleArray;
    case ID_planeOrigin:             return FieldType_doubleArray;
    case ID_planeNormal:             return FieldType_doubleArray;
    case ID_planeUpAxis:             return FieldType_doubleArray;
    case ID_planeRadius:             return FieldType_double;
    case ID_pointDensity:            return FieldType_int;
    case ID_colorTableName:          return FieldType_colortable;
    case ID_singleColor:             return FieldType_color;
    case ID_legendFlag:              return FieldType_bool;
    case ID_lightingFlag:            return FieldType_bool;
    case ID_relTol:                  return FieldType_double;
    case ID_absTol:                  return FieldType_double;
    case ID_terminationType:         return FieldType_enum;
    case ID_integrationType:         return FieldType_enum;
    case ID_showStreamlines:         return FieldType_bool;
    case ID_showPoints:              return FieldType_bool;
    case ID_NumberPlanes:            return FieldType_int;
    case ID_ColorStyle:              return FieldType_enum;
    case ID_MaxToroidalWinding:      return FieldType_int;
    case ID_OverrideToroidalWinding: return FieldType_int;
    case ID_HitRate:                 return FieldType_double;
    case ID_ShowCurves:              return FieldType_enum;
    case ID_AdjustPlane:             return FieldType_int;
    case ID_ShowIslands:             return FieldType_bool;
    case ID_Overlaps:                return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: PoincareAttributes::GetFieldTypeName
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
PoincareAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_sourceType:              return "enum";
    case ID_maxStepLength:           return "double";
    case ID_termination:             return "double";
    case ID_pointSource:             return "doubleArray";
    case ID_lineStart:               return "doubleArray";
    case ID_lineEnd:                 return "doubleArray";
    case ID_planeOrigin:             return "doubleArray";
    case ID_planeNormal:             return "doubleArray";
    case ID_planeUpAxis:             return "doubleArray";
    case ID_planeRadius:             return "double";
    case ID_pointDensity:            return "int";
    case ID_colorTableName:          return "colortable";
    case ID_singleColor:             return "color";
    case ID_legendFlag:              return "bool";
    case ID_lightingFlag:            return "bool";
    case ID_relTol:                  return "double";
    case ID_absTol:                  return "double";
    case ID_terminationType:         return "enum";
    case ID_integrationType:         return "enum";
    case ID_showStreamlines:         return "bool";
    case ID_showPoints:              return "bool";
    case ID_NumberPlanes:            return "int";
    case ID_ColorStyle:              return "enum";
    case ID_MaxToroidalWinding:      return "int";
    case ID_OverrideToroidalWinding: return "int";
    case ID_HitRate:                 return "double";
    case ID_ShowCurves:              return "enum";
    case ID_AdjustPlane:             return "int";
    case ID_ShowIslands:             return "bool";
    case ID_Overlaps:                return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PoincareAttributes::FieldsEqual
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
PoincareAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const PoincareAttributes &obj = *((const PoincareAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_sourceType:
        {  // new scope
        retval = (sourceType == obj.sourceType);
        }
        break;
    case ID_maxStepLength:
        {  // new scope
        retval = (maxStepLength == obj.maxStepLength);
        }
        break;
    case ID_termination:
        {  // new scope
        retval = (termination == obj.termination);
        }
        break;
    case ID_pointSource:
        {  // new scope
        // Compare the pointSource arrays.
        bool pointSource_equal = true;
        for(int i = 0; i < 3 && pointSource_equal; ++i)
            pointSource_equal = (pointSource[i] == obj.pointSource[i]);

        retval = pointSource_equal;
        }
        break;
    case ID_lineStart:
        {  // new scope
        // Compare the lineStart arrays.
        bool lineStart_equal = true;
        for(int i = 0; i < 3 && lineStart_equal; ++i)
            lineStart_equal = (lineStart[i] == obj.lineStart[i]);

        retval = lineStart_equal;
        }
        break;
    case ID_lineEnd:
        {  // new scope
        // Compare the lineEnd arrays.
        bool lineEnd_equal = true;
        for(int i = 0; i < 3 && lineEnd_equal; ++i)
            lineEnd_equal = (lineEnd[i] == obj.lineEnd[i]);

        retval = lineEnd_equal;
        }
        break;
    case ID_planeOrigin:
        {  // new scope
        // Compare the planeOrigin arrays.
        bool planeOrigin_equal = true;
        for(int i = 0; i < 3 && planeOrigin_equal; ++i)
            planeOrigin_equal = (planeOrigin[i] == obj.planeOrigin[i]);

        retval = planeOrigin_equal;
        }
        break;
    case ID_planeNormal:
        {  // new scope
        // Compare the planeNormal arrays.
        bool planeNormal_equal = true;
        for(int i = 0; i < 3 && planeNormal_equal; ++i)
            planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

        retval = planeNormal_equal;
        }
        break;
    case ID_planeUpAxis:
        {  // new scope
        // Compare the planeUpAxis arrays.
        bool planeUpAxis_equal = true;
        for(int i = 0; i < 3 && planeUpAxis_equal; ++i)
            planeUpAxis_equal = (planeUpAxis[i] == obj.planeUpAxis[i]);

        retval = planeUpAxis_equal;
        }
        break;
    case ID_planeRadius:
        {  // new scope
        retval = (planeRadius == obj.planeRadius);
        }
        break;
    case ID_pointDensity:
        {  // new scope
        retval = (pointDensity == obj.pointDensity);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_singleColor:
        {  // new scope
        retval = (singleColor == obj.singleColor);
        }
        break;
    case ID_legendFlag:
        {  // new scope
        retval = (legendFlag == obj.legendFlag);
        }
        break;
    case ID_lightingFlag:
        {  // new scope
        retval = (lightingFlag == obj.lightingFlag);
        }
        break;
    case ID_relTol:
        {  // new scope
        retval = (relTol == obj.relTol);
        }
        break;
    case ID_absTol:
        {  // new scope
        retval = (absTol == obj.absTol);
        }
        break;
    case ID_terminationType:
        {  // new scope
        retval = (terminationType == obj.terminationType);
        }
        break;
    case ID_integrationType:
        {  // new scope
        retval = (integrationType == obj.integrationType);
        }
        break;
    case ID_showStreamlines:
        {  // new scope
        retval = (showStreamlines == obj.showStreamlines);
        }
        break;
    case ID_showPoints:
        {  // new scope
        retval = (showPoints == obj.showPoints);
        }
        break;
    case ID_NumberPlanes:
        {  // new scope
        retval = (NumberPlanes == obj.NumberPlanes);
        }
        break;
    case ID_ColorStyle:
        {  // new scope
        retval = (ColorStyle == obj.ColorStyle);
        }
        break;
    case ID_MaxToroidalWinding:
        {  // new scope
        retval = (MaxToroidalWinding == obj.MaxToroidalWinding);
        }
        break;
    case ID_OverrideToroidalWinding:
        {  // new scope
        retval = (OverrideToroidalWinding == obj.OverrideToroidalWinding);
        }
        break;
    case ID_HitRate:
        {  // new scope
        retval = (HitRate == obj.HitRate);
        }
        break;
    case ID_ShowCurves:
        {  // new scope
        retval = (ShowCurves == obj.ShowCurves);
        }
        break;
    case ID_AdjustPlane:
        {  // new scope
        retval = (AdjustPlane == obj.AdjustPlane);
        }
        break;
    case ID_ShowIslands:
        {  // new scope
        retval = (ShowIslands == obj.ShowIslands);
        }
        break;
    case ID_Overlaps:
        {  // new scope
        retval = (Overlaps == obj.Overlaps);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

