/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
"SpecifiedPoint", "SpecifiedLine"};

std::string
PoincareAttributes::SourceType_ToString(PoincareAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SourceType_strings[index];
}

std::string
PoincareAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SourceType_strings[index];
}

bool
PoincareAttributes::SourceType_FromString(const std::string &s, PoincareAttributes::SourceType &val)
{
    val = PoincareAttributes::SpecifiedPoint;
    for(int i = 0; i < 2; ++i)
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
// Enum conversion methods for PoincareAttributes::IntegrationType
//

static const char *IntegrationType_strings[] = {
"DormandPrince", "AdamsBashforth", "M3DC1Integrator"
};

std::string
PoincareAttributes::IntegrationType_ToString(PoincareAttributes::IntegrationType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return IntegrationType_strings[index];
}

std::string
PoincareAttributes::IntegrationType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return IntegrationType_strings[index];
}

bool
PoincareAttributes::IntegrationType_FromString(const std::string &s, PoincareAttributes::IntegrationType &val)
{
    val = PoincareAttributes::DormandPrince;
    for(int i = 0; i < 3; ++i)
    {
        if(s == IntegrationType_strings[i])
        {
            val = (IntegrationType)i;
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
// Enum conversion methods for PoincareAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"ColorBySingleColor", "ColorByColorTable"};

std::string
PoincareAttributes::ColoringMethod_ToString(PoincareAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ColoringMethod_strings[index];
}

std::string
PoincareAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
PoincareAttributes::ColoringMethod_FromString(const std::string &s, PoincareAttributes::ColoringMethod &val)
{
    val = PoincareAttributes::ColorBySingleColor;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ColoringMethod_strings[i])
        {
            val = (ColoringMethod)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::ColorBy
//

static const char *ColorBy_strings[] = {
"OriginalValue", "InputOrder", "PointIndex", 
"Plane", "WindingOrder", "WindingPointOrder", 
"ToroidalWindings", "PoloidalWindings", "SafetyFactor", 
"Confidence", "RidgelineVariance"};

std::string
PoincareAttributes::ColorBy_ToString(PoincareAttributes::ColorBy t)
{
    int index = int(t);
    if(index < 0 || index >= 11) index = 0;
    return ColorBy_strings[index];
}

std::string
PoincareAttributes::ColorBy_ToString(int t)
{
    int index = (t < 0 || t >= 11) ? 0 : t;
    return ColorBy_strings[index];
}

bool
PoincareAttributes::ColorBy_FromString(const std::string &s, PoincareAttributes::ColorBy &val)
{
    val = PoincareAttributes::OriginalValue;
    for(int i = 0; i < 11; ++i)
    {
        if(s == ColorBy_strings[i])
        {
            val = (ColorBy)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Init utility for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PoincareAttributes::Init()
{
    minPunctures = 10;
    maxPunctures = 100;
    sourceType = SpecifiedPoint;
    pointSource[0] = 0;
    pointSource[1] = 0;
    pointSource[2] = 0;
    lineStart[0] = 0;
    lineStart[1] = 0;
    lineStart[2] = 0;
    lineEnd[0] = 1;
    lineEnd[1] = 0;
    lineEnd[2] = 0;
    pointDensity = 1;
    integrationType = AdamsBashforth;
    maxStepLength = 0.1;
    relTol = 0.0001;
    absTol = 1e-05;
    maxToroidalWinding = 30;
    overrideToroidalWinding = 0;
    hitRate = 0.9;
    adjustPlane = -1;
    overlaps = Remove;
    showCurves = Curves;
    numberPlanes = 1;
    min = 0;
    max = 0;
    minFlag = false;
    maxFlag = false;
    colorType = ColorBySingleColor;
    colorBy = SafetyFactor;
    showIslands = false;
    showLines = true;
    showPoints = false;
    verboseFlag = true;
    legendFlag = true;
    lightingFlag = true;

    PoincareAttributes::SelectAll();
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Copy utility for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PoincareAttributes::Copy(const PoincareAttributes &obj)
{
    minPunctures = obj.minPunctures;
    maxPunctures = obj.maxPunctures;
    sourceType = obj.sourceType;
    pointSource[0] = obj.pointSource[0];
    pointSource[1] = obj.pointSource[1];
    pointSource[2] = obj.pointSource[2];

    lineStart[0] = obj.lineStart[0];
    lineStart[1] = obj.lineStart[1];
    lineStart[2] = obj.lineStart[2];

    lineEnd[0] = obj.lineEnd[0];
    lineEnd[1] = obj.lineEnd[1];
    lineEnd[2] = obj.lineEnd[2];

    pointDensity = obj.pointDensity;
    integrationType = obj.integrationType;
    maxStepLength = obj.maxStepLength;
    relTol = obj.relTol;
    absTol = obj.absTol;
    maxToroidalWinding = obj.maxToroidalWinding;
    overrideToroidalWinding = obj.overrideToroidalWinding;
    hitRate = obj.hitRate;
    adjustPlane = obj.adjustPlane;
    overlaps = obj.overlaps;
    showCurves = obj.showCurves;
    numberPlanes = obj.numberPlanes;
    min = obj.min;
    max = obj.max;
    minFlag = obj.minFlag;
    maxFlag = obj.maxFlag;
    colorType = obj.colorType;
    singleColor = obj.singleColor;
    colorTableName = obj.colorTableName;
    colorBy = obj.colorBy;
    showIslands = obj.showIslands;
    showLines = obj.showLines;
    showPoints = obj.showPoints;
    verboseFlag = obj.verboseFlag;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;

    PoincareAttributes::SelectAll();
}

// Type map format string
const char *PoincareAttributes::TypeMapFormatString = POINCAREATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t PoincareAttributes::TmfsStruct = {POINCAREATTRIBUTES_TMFS};


// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Default constructor for the PoincareAttributes class.
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
    singleColor(0, 0, 0), colorTableName("Default")
{
    PoincareAttributes::Init();
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Constructor for the derived classes of PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs),
    singleColor(0, 0, 0), colorTableName("Default")
{
    PoincareAttributes::Init();
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
    PoincareAttributes::Copy(obj);
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes(const PoincareAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    PoincareAttributes::Copy(obj);
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

    PoincareAttributes::Copy(obj);

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

    // Create the return value
    return ((minPunctures == obj.minPunctures) &&
            (maxPunctures == obj.maxPunctures) &&
            (sourceType == obj.sourceType) &&
            pointSource_equal &&
            lineStart_equal &&
            lineEnd_equal &&
            (pointDensity == obj.pointDensity) &&
            (integrationType == obj.integrationType) &&
            (maxStepLength == obj.maxStepLength) &&
            (relTol == obj.relTol) &&
            (absTol == obj.absTol) &&
            (maxToroidalWinding == obj.maxToroidalWinding) &&
            (overrideToroidalWinding == obj.overrideToroidalWinding) &&
            (hitRate == obj.hitRate) &&
            (adjustPlane == obj.adjustPlane) &&
            (overlaps == obj.overlaps) &&
            (showCurves == obj.showCurves) &&
            (numberPlanes == obj.numberPlanes) &&
            (min == obj.min) &&
            (max == obj.max) &&
            (minFlag == obj.minFlag) &&
            (maxFlag == obj.maxFlag) &&
            (colorType == obj.colorType) &&
            (singleColor == obj.singleColor) &&
            (colorTableName == obj.colorTableName) &&
            (colorBy == obj.colorBy) &&
            (showIslands == obj.showIslands) &&
            (showLines == obj.showLines) &&
            (showPoints == obj.showPoints) &&
            (verboseFlag == obj.verboseFlag) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag));
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
    Select(ID_minPunctures,            (void *)&minPunctures);
    Select(ID_maxPunctures,            (void *)&maxPunctures);
    Select(ID_sourceType,              (void *)&sourceType);
    Select(ID_pointSource,             (void *)pointSource, 3);
    Select(ID_lineStart,               (void *)lineStart, 3);
    Select(ID_lineEnd,                 (void *)lineEnd, 3);
    Select(ID_pointDensity,            (void *)&pointDensity);
    Select(ID_integrationType,         (void *)&integrationType);
    Select(ID_maxStepLength,           (void *)&maxStepLength);
    Select(ID_relTol,                  (void *)&relTol);
    Select(ID_absTol,                  (void *)&absTol);
    Select(ID_maxToroidalWinding,      (void *)&maxToroidalWinding);
    Select(ID_overrideToroidalWinding, (void *)&overrideToroidalWinding);
    Select(ID_hitRate,                 (void *)&hitRate);
    Select(ID_adjustPlane,             (void *)&adjustPlane);
    Select(ID_overlaps,                (void *)&overlaps);
    Select(ID_showCurves,              (void *)&showCurves);
    Select(ID_numberPlanes,            (void *)&numberPlanes);
    Select(ID_min,                     (void *)&min);
    Select(ID_max,                     (void *)&max);
    Select(ID_minFlag,                 (void *)&minFlag);
    Select(ID_maxFlag,                 (void *)&maxFlag);
    Select(ID_colorType,               (void *)&colorType);
    Select(ID_singleColor,             (void *)&singleColor);
    Select(ID_colorTableName,          (void *)&colorTableName);
    Select(ID_colorBy,                 (void *)&colorBy);
    Select(ID_showIslands,             (void *)&showIslands);
    Select(ID_showLines,               (void *)&showLines);
    Select(ID_showPoints,              (void *)&showPoints);
    Select(ID_verboseFlag,             (void *)&verboseFlag);
    Select(ID_legendFlag,              (void *)&legendFlag);
    Select(ID_lightingFlag,            (void *)&lightingFlag);
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

    if(completeSave || !FieldsEqual(ID_minPunctures, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minPunctures", minPunctures));
    }

    if(completeSave || !FieldsEqual(ID_maxPunctures, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxPunctures", maxPunctures));
    }

    if(completeSave || !FieldsEqual(ID_sourceType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sourceType", SourceType_ToString(sourceType)));
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

    if(completeSave || !FieldsEqual(ID_pointDensity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointDensity", pointDensity));
    }

    if(completeSave || !FieldsEqual(ID_integrationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationType", IntegrationType_ToString(integrationType)));
    }

    if(completeSave || !FieldsEqual(ID_maxStepLength, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxStepLength", maxStepLength));
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

    if(completeSave || !FieldsEqual(ID_maxToroidalWinding, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxToroidalWinding", maxToroidalWinding));
    }

    if(completeSave || !FieldsEqual(ID_overrideToroidalWinding, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("overrideToroidalWinding", overrideToroidalWinding));
    }

    if(completeSave || !FieldsEqual(ID_hitRate, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("hitRate", hitRate));
    }

    if(completeSave || !FieldsEqual(ID_adjustPlane, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("adjustPlane", adjustPlane));
    }

    if(completeSave || !FieldsEqual(ID_overlaps, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("overlaps", OverlapType_ToString(overlaps)));
    }

    if(completeSave || !FieldsEqual(ID_showCurves, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showCurves", ShowMeshType_ToString(showCurves)));
    }

    if(completeSave || !FieldsEqual(ID_numberPlanes, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numberPlanes", numberPlanes));
    }

    if(completeSave || !FieldsEqual(ID_min, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("min", min));
    }

    if(completeSave || !FieldsEqual(ID_max, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("max", max));
    }

    if(completeSave || !FieldsEqual(ID_minFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minFlag", minFlag));
    }

    if(completeSave || !FieldsEqual(ID_maxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxFlag", maxFlag));
    }

    if(completeSave || !FieldsEqual(ID_colorType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorType", ColoringMethod_ToString(colorType)));
    }

        DataNode *singleColorNode = new DataNode("singleColor");
        if(singleColor.CreateNode(singleColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(singleColorNode);
        }
        else
            delete singleColorNode;
    if(completeSave || !FieldsEqual(ID_colorTableName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

    if(completeSave || !FieldsEqual(ID_colorBy, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorBy", ColorBy_ToString(colorBy)));
    }

    if(completeSave || !FieldsEqual(ID_showIslands, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showIslands", showIslands));
    }

    if(completeSave || !FieldsEqual(ID_showLines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showLines", showLines));
    }

    if(completeSave || !FieldsEqual(ID_showPoints, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showPoints", showPoints));
    }

    if(completeSave || !FieldsEqual(ID_verboseFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("verboseFlag", verboseFlag));
    }

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
    if((node = searchNode->GetNode("minPunctures")) != 0)
        SetMinPunctures(node->AsDouble());
    if((node = searchNode->GetNode("maxPunctures")) != 0)
        SetMaxPunctures(node->AsDouble());
    if((node = searchNode->GetNode("sourceType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetSourceType(SourceType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SourceType value;
            if(SourceType_FromString(node->AsString(), value))
                SetSourceType(value);
        }
    }
    if((node = searchNode->GetNode("pointSource")) != 0)
        SetPointSource(node->AsDoubleArray());
    if((node = searchNode->GetNode("lineStart")) != 0)
        SetLineStart(node->AsDoubleArray());
    if((node = searchNode->GetNode("lineEnd")) != 0)
        SetLineEnd(node->AsDoubleArray());
    if((node = searchNode->GetNode("pointDensity")) != 0)
        SetPointDensity(node->AsInt());
    if((node = searchNode->GetNode("integrationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetIntegrationType(IntegrationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationType value;
            if(IntegrationType_FromString(node->AsString(), value))
                SetIntegrationType(value);
        }
    }
    if((node = searchNode->GetNode("maxStepLength")) != 0)
        SetMaxStepLength(node->AsDouble());
    if((node = searchNode->GetNode("relTol")) != 0)
        SetRelTol(node->AsDouble());
    if((node = searchNode->GetNode("absTol")) != 0)
        SetAbsTol(node->AsDouble());
    if((node = searchNode->GetNode("maxToroidalWinding")) != 0)
        SetMaxToroidalWinding(node->AsInt());
    if((node = searchNode->GetNode("overrideToroidalWinding")) != 0)
        SetOverrideToroidalWinding(node->AsInt());
    if((node = searchNode->GetNode("hitRate")) != 0)
        SetHitRate(node->AsDouble());
    if((node = searchNode->GetNode("adjustPlane")) != 0)
        SetAdjustPlane(node->AsInt());
    if((node = searchNode->GetNode("overlaps")) != 0)
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
    if((node = searchNode->GetNode("showCurves")) != 0)
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
    if((node = searchNode->GetNode("numberPlanes")) != 0)
        SetNumberPlanes(node->AsInt());
    if((node = searchNode->GetNode("min")) != 0)
        SetMin(node->AsDouble());
    if((node = searchNode->GetNode("max")) != 0)
        SetMax(node->AsDouble());
    if((node = searchNode->GetNode("minFlag")) != 0)
        SetMinFlag(node->AsBool());
    if((node = searchNode->GetNode("maxFlag")) != 0)
        SetMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("colorType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetColorType(ColoringMethod(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ColoringMethod value;
            if(ColoringMethod_FromString(node->AsString(), value))
                SetColorType(value);
        }
    }
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("colorBy")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 11)
                SetColorBy(ColorBy(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ColorBy value;
            if(ColorBy_FromString(node->AsString(), value))
                SetColorBy(value);
        }
    }
    if((node = searchNode->GetNode("showIslands")) != 0)
        SetShowIslands(node->AsBool());
    if((node = searchNode->GetNode("showLines")) != 0)
        SetShowLines(node->AsBool());
    if((node = searchNode->GetNode("showPoints")) != 0)
        SetShowPoints(node->AsBool());
    if((node = searchNode->GetNode("verboseFlag")) != 0)
        SetVerboseFlag(node->AsBool());
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
PoincareAttributes::SetMinPunctures(double minPunctures_)
{
    minPunctures = minPunctures_;
    Select(ID_minPunctures, (void *)&minPunctures);
}

void
PoincareAttributes::SetMaxPunctures(double maxPunctures_)
{
    maxPunctures = maxPunctures_;
    Select(ID_maxPunctures, (void *)&maxPunctures);
}

void
PoincareAttributes::SetSourceType(PoincareAttributes::SourceType sourceType_)
{
    sourceType = sourceType_;
    Select(ID_sourceType, (void *)&sourceType);
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
PoincareAttributes::SetPointDensity(int pointDensity_)
{
    pointDensity = pointDensity_;
    Select(ID_pointDensity, (void *)&pointDensity);
}

void
PoincareAttributes::SetIntegrationType(PoincareAttributes::IntegrationType integrationType_)
{
    integrationType = integrationType_;
    Select(ID_integrationType, (void *)&integrationType);
}

void
PoincareAttributes::SetMaxStepLength(double maxStepLength_)
{
    maxStepLength = maxStepLength_;
    Select(ID_maxStepLength, (void *)&maxStepLength);
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
PoincareAttributes::SetMaxToroidalWinding(int maxToroidalWinding_)
{
    maxToroidalWinding = maxToroidalWinding_;
    Select(ID_maxToroidalWinding, (void *)&maxToroidalWinding);
}

void
PoincareAttributes::SetOverrideToroidalWinding(int overrideToroidalWinding_)
{
    overrideToroidalWinding = overrideToroidalWinding_;
    Select(ID_overrideToroidalWinding, (void *)&overrideToroidalWinding);
}

void
PoincareAttributes::SetHitRate(double hitRate_)
{
    hitRate = hitRate_;
    Select(ID_hitRate, (void *)&hitRate);
}

void
PoincareAttributes::SetAdjustPlane(int adjustPlane_)
{
    adjustPlane = adjustPlane_;
    Select(ID_adjustPlane, (void *)&adjustPlane);
}

void
PoincareAttributes::SetOverlaps(PoincareAttributes::OverlapType overlaps_)
{
    overlaps = overlaps_;
    Select(ID_overlaps, (void *)&overlaps);
}

void
PoincareAttributes::SetShowCurves(PoincareAttributes::ShowMeshType showCurves_)
{
    showCurves = showCurves_;
    Select(ID_showCurves, (void *)&showCurves);
}

void
PoincareAttributes::SetNumberPlanes(int numberPlanes_)
{
    numberPlanes = numberPlanes_;
    Select(ID_numberPlanes, (void *)&numberPlanes);
}

void
PoincareAttributes::SetMin(double min_)
{
    min = min_;
    Select(ID_min, (void *)&min);
}

void
PoincareAttributes::SetMax(double max_)
{
    max = max_;
    Select(ID_max, (void *)&max);
}

void
PoincareAttributes::SetMinFlag(bool minFlag_)
{
    minFlag = minFlag_;
    Select(ID_minFlag, (void *)&minFlag);
}

void
PoincareAttributes::SetMaxFlag(bool maxFlag_)
{
    maxFlag = maxFlag_;
    Select(ID_maxFlag, (void *)&maxFlag);
}

void
PoincareAttributes::SetColorType(PoincareAttributes::ColoringMethod colorType_)
{
    colorType = colorType_;
    Select(ID_colorType, (void *)&colorType);
}

void
PoincareAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
PoincareAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PoincareAttributes::SetColorBy(PoincareAttributes::ColorBy colorBy_)
{
    colorBy = colorBy_;
    Select(ID_colorBy, (void *)&colorBy);
}

void
PoincareAttributes::SetShowIslands(bool showIslands_)
{
    showIslands = showIslands_;
    Select(ID_showIslands, (void *)&showIslands);
}

void
PoincareAttributes::SetShowLines(bool showLines_)
{
    showLines = showLines_;
    Select(ID_showLines, (void *)&showLines);
}

void
PoincareAttributes::SetShowPoints(bool showPoints_)
{
    showPoints = showPoints_;
    Select(ID_showPoints, (void *)&showPoints);
}

void
PoincareAttributes::SetVerboseFlag(bool verboseFlag_)
{
    verboseFlag = verboseFlag_;
    Select(ID_verboseFlag, (void *)&verboseFlag);
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

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

double
PoincareAttributes::GetMinPunctures() const
{
    return minPunctures;
}

double
PoincareAttributes::GetMaxPunctures() const
{
    return maxPunctures;
}

PoincareAttributes::SourceType
PoincareAttributes::GetSourceType() const
{
    return SourceType(sourceType);
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

int
PoincareAttributes::GetPointDensity() const
{
    return pointDensity;
}

PoincareAttributes::IntegrationType
PoincareAttributes::GetIntegrationType() const
{
    return IntegrationType(integrationType);
}

double
PoincareAttributes::GetMaxStepLength() const
{
    return maxStepLength;
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

int
PoincareAttributes::GetMaxToroidalWinding() const
{
    return maxToroidalWinding;
}

int
PoincareAttributes::GetOverrideToroidalWinding() const
{
    return overrideToroidalWinding;
}

double
PoincareAttributes::GetHitRate() const
{
    return hitRate;
}

int
PoincareAttributes::GetAdjustPlane() const
{
    return adjustPlane;
}

PoincareAttributes::OverlapType
PoincareAttributes::GetOverlaps() const
{
    return OverlapType(overlaps);
}

PoincareAttributes::ShowMeshType
PoincareAttributes::GetShowCurves() const
{
    return ShowMeshType(showCurves);
}

int
PoincareAttributes::GetNumberPlanes() const
{
    return numberPlanes;
}

double
PoincareAttributes::GetMin() const
{
    return min;
}

double
PoincareAttributes::GetMax() const
{
    return max;
}

bool
PoincareAttributes::GetMinFlag() const
{
    return minFlag;
}

bool
PoincareAttributes::GetMaxFlag() const
{
    return maxFlag;
}

PoincareAttributes::ColoringMethod
PoincareAttributes::GetColorType() const
{
    return ColoringMethod(colorType);
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

PoincareAttributes::ColorBy
PoincareAttributes::GetColorBy() const
{
    return ColorBy(colorBy);
}

bool
PoincareAttributes::GetShowIslands() const
{
    return showIslands;
}

bool
PoincareAttributes::GetShowLines() const
{
    return showLines;
}

bool
PoincareAttributes::GetShowPoints() const
{
    return showPoints;
}

bool
PoincareAttributes::GetVerboseFlag() const
{
    return verboseFlag;
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
PoincareAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

void
PoincareAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
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
    case ID_minPunctures:            return "minPunctures";
    case ID_maxPunctures:            return "maxPunctures";
    case ID_sourceType:              return "sourceType";
    case ID_pointSource:             return "pointSource";
    case ID_lineStart:               return "lineStart";
    case ID_lineEnd:                 return "lineEnd";
    case ID_pointDensity:            return "pointDensity";
    case ID_integrationType:         return "integrationType";
    case ID_maxStepLength:           return "maxStepLength";
    case ID_relTol:                  return "relTol";
    case ID_absTol:                  return "absTol";
    case ID_maxToroidalWinding:      return "maxToroidalWinding";
    case ID_overrideToroidalWinding: return "overrideToroidalWinding";
    case ID_hitRate:                 return "hitRate";
    case ID_adjustPlane:             return "adjustPlane";
    case ID_overlaps:                return "overlaps";
    case ID_showCurves:              return "showCurves";
    case ID_numberPlanes:            return "numberPlanes";
    case ID_min:                     return "min";
    case ID_max:                     return "max";
    case ID_minFlag:                 return "minFlag";
    case ID_maxFlag:                 return "maxFlag";
    case ID_colorType:               return "colorType";
    case ID_singleColor:             return "singleColor";
    case ID_colorTableName:          return "colorTableName";
    case ID_colorBy:                 return "colorBy";
    case ID_showIslands:             return "showIslands";
    case ID_showLines:               return "showLines";
    case ID_showPoints:              return "showPoints";
    case ID_verboseFlag:             return "verboseFlag";
    case ID_legendFlag:              return "legendFlag";
    case ID_lightingFlag:            return "lightingFlag";
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
    case ID_minPunctures:            return FieldType_double;
    case ID_maxPunctures:            return FieldType_double;
    case ID_sourceType:              return FieldType_enum;
    case ID_pointSource:             return FieldType_doubleArray;
    case ID_lineStart:               return FieldType_doubleArray;
    case ID_lineEnd:                 return FieldType_doubleArray;
    case ID_pointDensity:            return FieldType_int;
    case ID_integrationType:         return FieldType_enum;
    case ID_maxStepLength:           return FieldType_double;
    case ID_relTol:                  return FieldType_double;
    case ID_absTol:                  return FieldType_double;
    case ID_maxToroidalWinding:      return FieldType_int;
    case ID_overrideToroidalWinding: return FieldType_int;
    case ID_hitRate:                 return FieldType_double;
    case ID_adjustPlane:             return FieldType_int;
    case ID_overlaps:                return FieldType_enum;
    case ID_showCurves:              return FieldType_enum;
    case ID_numberPlanes:            return FieldType_int;
    case ID_min:                     return FieldType_double;
    case ID_max:                     return FieldType_double;
    case ID_minFlag:                 return FieldType_bool;
    case ID_maxFlag:                 return FieldType_bool;
    case ID_colorType:               return FieldType_enum;
    case ID_singleColor:             return FieldType_color;
    case ID_colorTableName:          return FieldType_colortable;
    case ID_colorBy:                 return FieldType_enum;
    case ID_showIslands:             return FieldType_bool;
    case ID_showLines:               return FieldType_bool;
    case ID_showPoints:              return FieldType_bool;
    case ID_verboseFlag:             return FieldType_bool;
    case ID_legendFlag:              return FieldType_bool;
    case ID_lightingFlag:            return FieldType_bool;
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
    case ID_minPunctures:            return "double";
    case ID_maxPunctures:            return "double";
    case ID_sourceType:              return "enum";
    case ID_pointSource:             return "doubleArray";
    case ID_lineStart:               return "doubleArray";
    case ID_lineEnd:                 return "doubleArray";
    case ID_pointDensity:            return "int";
    case ID_integrationType:         return "enum";
    case ID_maxStepLength:           return "double";
    case ID_relTol:                  return "double";
    case ID_absTol:                  return "double";
    case ID_maxToroidalWinding:      return "int";
    case ID_overrideToroidalWinding: return "int";
    case ID_hitRate:                 return "double";
    case ID_adjustPlane:             return "int";
    case ID_overlaps:                return "enum";
    case ID_showCurves:              return "enum";
    case ID_numberPlanes:            return "int";
    case ID_min:                     return "double";
    case ID_max:                     return "double";
    case ID_minFlag:                 return "bool";
    case ID_maxFlag:                 return "bool";
    case ID_colorType:               return "enum";
    case ID_singleColor:             return "color";
    case ID_colorTableName:          return "colortable";
    case ID_colorBy:                 return "enum";
    case ID_showIslands:             return "bool";
    case ID_showLines:               return "bool";
    case ID_showPoints:              return "bool";
    case ID_verboseFlag:             return "bool";
    case ID_legendFlag:              return "bool";
    case ID_lightingFlag:            return "bool";
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
    case ID_minPunctures:
        {  // new scope
        retval = (minPunctures == obj.minPunctures);
        }
        break;
    case ID_maxPunctures:
        {  // new scope
        retval = (maxPunctures == obj.maxPunctures);
        }
        break;
    case ID_sourceType:
        {  // new scope
        retval = (sourceType == obj.sourceType);
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
    case ID_pointDensity:
        {  // new scope
        retval = (pointDensity == obj.pointDensity);
        }
        break;
    case ID_integrationType:
        {  // new scope
        retval = (integrationType == obj.integrationType);
        }
        break;
    case ID_maxStepLength:
        {  // new scope
        retval = (maxStepLength == obj.maxStepLength);
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
    case ID_maxToroidalWinding:
        {  // new scope
        retval = (maxToroidalWinding == obj.maxToroidalWinding);
        }
        break;
    case ID_overrideToroidalWinding:
        {  // new scope
        retval = (overrideToroidalWinding == obj.overrideToroidalWinding);
        }
        break;
    case ID_hitRate:
        {  // new scope
        retval = (hitRate == obj.hitRate);
        }
        break;
    case ID_adjustPlane:
        {  // new scope
        retval = (adjustPlane == obj.adjustPlane);
        }
        break;
    case ID_overlaps:
        {  // new scope
        retval = (overlaps == obj.overlaps);
        }
        break;
    case ID_showCurves:
        {  // new scope
        retval = (showCurves == obj.showCurves);
        }
        break;
    case ID_numberPlanes:
        {  // new scope
        retval = (numberPlanes == obj.numberPlanes);
        }
        break;
    case ID_min:
        {  // new scope
        retval = (min == obj.min);
        }
        break;
    case ID_max:
        {  // new scope
        retval = (max == obj.max);
        }
        break;
    case ID_minFlag:
        {  // new scope
        retval = (minFlag == obj.minFlag);
        }
        break;
    case ID_maxFlag:
        {  // new scope
        retval = (maxFlag == obj.maxFlag);
        }
        break;
    case ID_colorType:
        {  // new scope
        retval = (colorType == obj.colorType);
        }
        break;
    case ID_singleColor:
        {  // new scope
        retval = (singleColor == obj.singleColor);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_colorBy:
        {  // new scope
        retval = (colorBy == obj.colorBy);
        }
        break;
    case ID_showIslands:
        {  // new scope
        retval = (showIslands == obj.showIslands);
        }
        break;
    case ID_showLines:
        {  // new scope
        retval = (showLines == obj.showLines);
        }
        break;
    case ID_showPoints:
        {  // new scope
        retval = (showPoints == obj.showPoints);
        }
        break;
    case ID_verboseFlag:
        {  // new scope
        retval = (verboseFlag == obj.verboseFlag);
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
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PoincareAttributes::ChangesRequireRecalculation
//
// Purpose: 
//   Determine if attribute changes require recalculation.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//
// ****************************************************************************

bool
PoincareAttributes::ChangesRequireRecalculation(const PoincareAttributes &obj) const
{
    return StreamlineAttsRequireRecalculation(obj) ||
           PoincareAttsRequireRecalculation(obj);
}

// ****************************************************************************
// Method: PoincareAttributes::StreamlineAttsRequireRecalculation
//
// Purpose: 
//   Determine if streamline attribute changes require recalculation.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//
// ****************************************************************************

#define PDIF(p1,p2,i) ((p1)[i] != (p2)[i])
#define POINT_DIFFERS(p1,p2) (PDIF(p1,p2,0) || PDIF(p1,p2,1) || PDIF(p1,p2,2))

bool
PoincareAttributes::StreamlineAttsRequireRecalculation(const PoincareAttributes &obj) const
{
    // If we're in point source mode and the points differ, sourcePointsDiffer
    // evaluates to true.
    bool sourcePointsDiffer = ((sourceType == SpecifiedPoint) &&
                               POINT_DIFFERS(pointSource, obj.pointSource));

    // If we're in line source mode and the line differs, sourceLineDiffers
    // evaluates to true.
    bool sourceLineDiffers = ((sourceType == SpecifiedLine) &&
                              (POINT_DIFFERS(lineStart, obj.lineStart) ||
                               POINT_DIFFERS(lineEnd, obj.lineEnd)));
    
    // Other things need to be true before we start paying attention to
    // point density.
    bool densityMatters = ((sourceType == SpecifiedLine) &&
                           (pointDensity != obj.pointDensity));

    return (minPunctures != obj.minPunctures ||
            maxPunctures != obj.maxPunctures ||
            sourcePointsDiffer ||
            sourceLineDiffers ||
            densityMatters ||

            sourceType != obj.sourceType ||
            integrationType != obj.integrationType ||
            maxStepLength != obj.maxStepLength ||
            relTol != obj.relTol ||
            absTol != obj.absTol);
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttsRequireRecalculation
//
// Purpose: 
//   Determine if poincare attribute changes require recalculation.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//
// ****************************************************************************

bool
PoincareAttributes::PoincareAttsRequireRecalculation(const PoincareAttributes &obj) const
{
    return maxToroidalWinding != obj.maxToroidalWinding ||
           overrideToroidalWinding != obj.overrideToroidalWinding ||
           hitRate != obj.hitRate ||
           adjustPlane != obj.adjustPlane ||
           overlaps != obj.overlaps ||

           showCurves != obj.showCurves ||
           numberPlanes != obj.numberPlanes ||

           colorBy != obj.colorBy ||

           showIslands != obj.showIslands ||
           showLines != obj.showLines ||
           showPoints != obj.showPoints ||
           verboseFlag != obj.verboseFlag;
}

