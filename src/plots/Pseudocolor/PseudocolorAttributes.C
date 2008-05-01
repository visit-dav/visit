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

#include <PseudocolorAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for PseudocolorAttributes::Centering
//

static const char *Centering_strings[] = {
"Natural", "Nodal", "Zonal"
};

std::string
PseudocolorAttributes::Centering_ToString(PseudocolorAttributes::Centering t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Centering_strings[index];
}

std::string
PseudocolorAttributes::Centering_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Centering_strings[index];
}

bool
PseudocolorAttributes::Centering_FromString(const std::string &s, PseudocolorAttributes::Centering &val)
{
    val = PseudocolorAttributes::Natural;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Centering_strings[i])
        {
            val = (Centering)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PseudocolorAttributes::Scaling
//

static const char *Scaling_strings[] = {
"Linear", "Log", "Skew"
};

std::string
PseudocolorAttributes::Scaling_ToString(PseudocolorAttributes::Scaling t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Scaling_strings[index];
}

std::string
PseudocolorAttributes::Scaling_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Scaling_strings[index];
}

bool
PseudocolorAttributes::Scaling_FromString(const std::string &s, PseudocolorAttributes::Scaling &val)
{
    val = PseudocolorAttributes::Linear;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Scaling_strings[i])
        {
            val = (Scaling)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PseudocolorAttributes::LimitsMode
//

static const char *LimitsMode_strings[] = {
"OriginalData", "CurrentPlot"};

std::string
PseudocolorAttributes::LimitsMode_ToString(PseudocolorAttributes::LimitsMode t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return LimitsMode_strings[index];
}

std::string
PseudocolorAttributes::LimitsMode_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return LimitsMode_strings[index];
}

bool
PseudocolorAttributes::LimitsMode_FromString(const std::string &s, PseudocolorAttributes::LimitsMode &val)
{
    val = PseudocolorAttributes::OriginalData;
    for(int i = 0; i < 2; ++i)
    {
        if(s == LimitsMode_strings[i])
        {
            val = (LimitsMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PseudocolorAttributes::PointType
//

static const char *PointType_strings[] = {
"Box", "Axis", "Icosahedron", 
"Point", "Sphere"};

std::string
PseudocolorAttributes::PointType_ToString(PseudocolorAttributes::PointType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return PointType_strings[index];
}

std::string
PseudocolorAttributes::PointType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return PointType_strings[index];
}

bool
PseudocolorAttributes::PointType_FromString(const std::string &s, PseudocolorAttributes::PointType &val)
{
    val = PseudocolorAttributes::Box;
    for(int i = 0; i < 5; ++i)
    {
        if(s == PointType_strings[i])
        {
            val = (PointType)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *PseudocolorAttributes::TypeMapFormatString = "bbbbiiidddiddsibsi";

// ****************************************************************************
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Constructor for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::PseudocolorAttributes() : 
    AttributeSubject(PseudocolorAttributes::TypeMapFormatString),
    colorTableName("hot"), pointSizeVar("default")
{
    legendFlag = true;
    lightingFlag = true;
    minFlag = false;
    maxFlag = false;
    centering = Natural;
    scaling = Linear;
    limitsMode = OriginalData;
    min = 0;
    max = 1;
    pointSize = 0.05;
    pointType = Point;
    skewFactor = 1;
    opacity = 1;
    smoothingLevel = 0;
    pointSizeVarEnabled = false;
    pointSizePixels = 2;
}

// ****************************************************************************
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Copy constructor for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::PseudocolorAttributes(const PseudocolorAttributes &obj) : 
    AttributeSubject(PseudocolorAttributes::TypeMapFormatString)
{
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    minFlag = obj.minFlag;
    maxFlag = obj.maxFlag;
    centering = obj.centering;
    scaling = obj.scaling;
    limitsMode = obj.limitsMode;
    min = obj.min;
    max = obj.max;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    skewFactor = obj.skewFactor;
    opacity = obj.opacity;
    colorTableName = obj.colorTableName;
    smoothingLevel = obj.smoothingLevel;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;

    SelectAll();
}

// ****************************************************************************
// Method: PseudocolorAttributes::~PseudocolorAttributes
//
// Purpose: 
//   Destructor for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::~PseudocolorAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: PseudocolorAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes& 
PseudocolorAttributes::operator = (const PseudocolorAttributes &obj)
{
    if (this == &obj) return *this;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    minFlag = obj.minFlag;
    maxFlag = obj.maxFlag;
    centering = obj.centering;
    scaling = obj.scaling;
    limitsMode = obj.limitsMode;
    min = obj.min;
    max = obj.max;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    skewFactor = obj.skewFactor;
    opacity = obj.opacity;
    colorTableName = obj.colorTableName;
    smoothingLevel = obj.smoothingLevel;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: PseudocolorAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the PseudocolorAttributes class.
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
PseudocolorAttributes::operator == (const PseudocolorAttributes &obj) const
{
    // Create the return value
    return ((legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (minFlag == obj.minFlag) &&
            (maxFlag == obj.maxFlag) &&
            (centering == obj.centering) &&
            (scaling == obj.scaling) &&
            (limitsMode == obj.limitsMode) &&
            (min == obj.min) &&
            (max == obj.max) &&
            (pointSize == obj.pointSize) &&
            (pointType == obj.pointType) &&
            (skewFactor == obj.skewFactor) &&
            (opacity == obj.opacity) &&
            (colorTableName == obj.colorTableName) &&
            (smoothingLevel == obj.smoothingLevel) &&
            (pointSizeVarEnabled == obj.pointSizeVarEnabled) &&
            (pointSizeVar == obj.pointSizeVar) &&
            (pointSizePixels == obj.pointSizePixels));
}

// ****************************************************************************
// Method: PseudocolorAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the PseudocolorAttributes class.
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
PseudocolorAttributes::operator != (const PseudocolorAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: PseudocolorAttributes::TypeName
//
// Purpose: 
//   Type name method for the PseudocolorAttributes class.
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
PseudocolorAttributes::TypeName() const
{
    return "PseudocolorAttributes";
}

// ****************************************************************************
// Method: PseudocolorAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the PseudocolorAttributes class.
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
PseudocolorAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const PseudocolorAttributes *tmp = (const PseudocolorAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: PseudocolorAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the PseudocolorAttributes class.
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
PseudocolorAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new PseudocolorAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: PseudocolorAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the PseudocolorAttributes class.
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
PseudocolorAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new PseudocolorAttributes(*this);
    else
        retval = new PseudocolorAttributes;

    return retval;
}

// ****************************************************************************
// Method: PseudocolorAttributes::SelectAll
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
PseudocolorAttributes::SelectAll()
{
    Select(ID_legendFlag,          (void *)&legendFlag);
    Select(ID_lightingFlag,        (void *)&lightingFlag);
    Select(ID_minFlag,             (void *)&minFlag);
    Select(ID_maxFlag,             (void *)&maxFlag);
    Select(ID_centering,           (void *)&centering);
    Select(ID_scaling,             (void *)&scaling);
    Select(ID_limitsMode,          (void *)&limitsMode);
    Select(ID_min,                 (void *)&min);
    Select(ID_max,                 (void *)&max);
    Select(ID_pointSize,           (void *)&pointSize);
    Select(ID_pointType,           (void *)&pointType);
    Select(ID_skewFactor,          (void *)&skewFactor);
    Select(ID_opacity,             (void *)&opacity);
    Select(ID_colorTableName,      (void *)&colorTableName);
    Select(ID_smoothingLevel,      (void *)&smoothingLevel);
    Select(ID_pointSizeVarEnabled, (void *)&pointSizeVarEnabled);
    Select(ID_pointSizeVar,        (void *)&pointSizeVar);
    Select(ID_pointSizePixels,     (void *)&pointSizePixels);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PseudocolorAttributes::CreateNode
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
PseudocolorAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    PseudocolorAttributes defaultObject;
    bool addToParent = false;
    // Create a node for PseudocolorAttributes.
    DataNode *node = new DataNode("PseudocolorAttributes");

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

    if(completeSave || !FieldsEqual(ID_centering, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("centering", Centering_ToString(centering)));
    }

    if(completeSave || !FieldsEqual(ID_scaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaling", Scaling_ToString(scaling)));
    }

    if(completeSave || !FieldsEqual(ID_limitsMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("limitsMode", LimitsMode_ToString(limitsMode)));
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

    if(completeSave || !FieldsEqual(ID_pointSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSize", pointSize));
    }

    if(completeSave || !FieldsEqual(ID_pointType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointType", PointType_ToString(pointType)));
    }

    if(completeSave || !FieldsEqual(ID_skewFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("skewFactor", skewFactor));
    }

    if(completeSave || !FieldsEqual(ID_opacity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacity", opacity));
    }

    if(completeSave || !FieldsEqual(ID_colorTableName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

    if(completeSave || !FieldsEqual(ID_smoothingLevel, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("smoothingLevel", smoothingLevel));
    }

    if(completeSave || !FieldsEqual(ID_pointSizeVarEnabled, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizeVarEnabled", pointSizeVarEnabled));
    }

    if(completeSave || !FieldsEqual(ID_pointSizeVar, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizeVar", pointSizeVar));
    }

    if(completeSave || !FieldsEqual(ID_pointSizePixels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizePixels", pointSizePixels));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: PseudocolorAttributes::SetFromNode
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
PseudocolorAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("PseudocolorAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("minFlag")) != 0)
        SetMinFlag(node->AsBool());
    if((node = searchNode->GetNode("maxFlag")) != 0)
        SetMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("centering")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetCentering(Centering(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Centering value;
            if(Centering_FromString(node->AsString(), value))
                SetCentering(value);
        }
    }
    if((node = searchNode->GetNode("scaling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetScaling(Scaling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Scaling value;
            if(Scaling_FromString(node->AsString(), value))
                SetScaling(value);
        }
    }
    if((node = searchNode->GetNode("limitsMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetLimitsMode(LimitsMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LimitsMode value;
            if(LimitsMode_FromString(node->AsString(), value))
                SetLimitsMode(value);
        }
    }
    if((node = searchNode->GetNode("min")) != 0)
        SetMin(node->AsDouble());
    if((node = searchNode->GetNode("max")) != 0)
        SetMax(node->AsDouble());
    if((node = searchNode->GetNode("pointSize")) != 0)
        SetPointSize(node->AsDouble());
    if((node = searchNode->GetNode("pointType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetPointType(PointType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PointType value;
            if(PointType_FromString(node->AsString(), value))
                SetPointType(value);
        }
    }
    if((node = searchNode->GetNode("skewFactor")) != 0)
        SetSkewFactor(node->AsDouble());
    if((node = searchNode->GetNode("opacity")) != 0)
        SetOpacity(node->AsDouble());
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("smoothingLevel")) != 0)
        SetSmoothingLevel(node->AsInt());
    if((node = searchNode->GetNode("pointSizeVarEnabled")) != 0)
        SetPointSizeVarEnabled(node->AsBool());
    if((node = searchNode->GetNode("pointSizeVar")) != 0)
        SetPointSizeVar(node->AsString());
    if((node = searchNode->GetNode("pointSizePixels")) != 0)
        SetPointSizePixels(node->AsInt());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
PseudocolorAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
PseudocolorAttributes::SetLightingFlag(bool lightingFlag_)
{
    lightingFlag = lightingFlag_;
    Select(ID_lightingFlag, (void *)&lightingFlag);
}

void
PseudocolorAttributes::SetMinFlag(bool minFlag_)
{
    minFlag = minFlag_;
    Select(ID_minFlag, (void *)&minFlag);
}

void
PseudocolorAttributes::SetMaxFlag(bool maxFlag_)
{
    maxFlag = maxFlag_;
    Select(ID_maxFlag, (void *)&maxFlag);
}

void
PseudocolorAttributes::SetCentering(PseudocolorAttributes::Centering centering_)
{
    centering = centering_;
    Select(ID_centering, (void *)&centering);
}

void
PseudocolorAttributes::SetScaling(PseudocolorAttributes::Scaling scaling_)
{
    scaling = scaling_;
    Select(ID_scaling, (void *)&scaling);
}

void
PseudocolorAttributes::SetLimitsMode(PseudocolorAttributes::LimitsMode limitsMode_)
{
    limitsMode = limitsMode_;
    Select(ID_limitsMode, (void *)&limitsMode);
}

void
PseudocolorAttributes::SetMin(double min_)
{
    min = min_;
    Select(ID_min, (void *)&min);
}

void
PseudocolorAttributes::SetMax(double max_)
{
    max = max_;
    Select(ID_max, (void *)&max);
}

void
PseudocolorAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(ID_pointSize, (void *)&pointSize);
}

void
PseudocolorAttributes::SetPointType(PseudocolorAttributes::PointType pointType_)
{
    pointType = pointType_;
    Select(ID_pointType, (void *)&pointType);
}

void
PseudocolorAttributes::SetSkewFactor(double skewFactor_)
{
    skewFactor = skewFactor_;
    Select(ID_skewFactor, (void *)&skewFactor);
}

void
PseudocolorAttributes::SetOpacity(double opacity_)
{
    opacity = opacity_;
    Select(ID_opacity, (void *)&opacity);
}

void
PseudocolorAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PseudocolorAttributes::SetSmoothingLevel(int smoothingLevel_)
{
    smoothingLevel = smoothingLevel_;
    Select(ID_smoothingLevel, (void *)&smoothingLevel);
}

void
PseudocolorAttributes::SetPointSizeVarEnabled(bool pointSizeVarEnabled_)
{
    pointSizeVarEnabled = pointSizeVarEnabled_;
    Select(ID_pointSizeVarEnabled, (void *)&pointSizeVarEnabled);
}

void
PseudocolorAttributes::SetPointSizeVar(const std::string &pointSizeVar_)
{
    pointSizeVar = pointSizeVar_;
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

void
PseudocolorAttributes::SetPointSizePixels(int pointSizePixels_)
{
    pointSizePixels = pointSizePixels_;
    Select(ID_pointSizePixels, (void *)&pointSizePixels);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
PseudocolorAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
PseudocolorAttributes::GetLightingFlag() const
{
    return lightingFlag;
}

bool
PseudocolorAttributes::GetMinFlag() const
{
    return minFlag;
}

bool
PseudocolorAttributes::GetMaxFlag() const
{
    return maxFlag;
}

PseudocolorAttributes::Centering
PseudocolorAttributes::GetCentering() const
{
    return Centering(centering);
}

PseudocolorAttributes::Scaling
PseudocolorAttributes::GetScaling() const
{
    return Scaling(scaling);
}

PseudocolorAttributes::LimitsMode
PseudocolorAttributes::GetLimitsMode() const
{
    return LimitsMode(limitsMode);
}

double
PseudocolorAttributes::GetMin() const
{
    return min;
}

double
PseudocolorAttributes::GetMax() const
{
    return max;
}

double
PseudocolorAttributes::GetPointSize() const
{
    return pointSize;
}

PseudocolorAttributes::PointType
PseudocolorAttributes::GetPointType() const
{
    return PointType(pointType);
}

double
PseudocolorAttributes::GetSkewFactor() const
{
    return skewFactor;
}

double
PseudocolorAttributes::GetOpacity() const
{
    return opacity;
}

const std::string &
PseudocolorAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
PseudocolorAttributes::GetColorTableName()
{
    return colorTableName;
}

int
PseudocolorAttributes::GetSmoothingLevel() const
{
    return smoothingLevel;
}

bool
PseudocolorAttributes::GetPointSizeVarEnabled() const
{
    return pointSizeVarEnabled;
}

const std::string &
PseudocolorAttributes::GetPointSizeVar() const
{
    return pointSizeVar;
}

std::string &
PseudocolorAttributes::GetPointSizeVar()
{
    return pointSizeVar;
}

int
PseudocolorAttributes::GetPointSizePixels() const
{
    return pointSizePixels;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
PseudocolorAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PseudocolorAttributes::SelectPointSizeVar()
{
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PseudocolorAttributes::GetFieldName
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
PseudocolorAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_legendFlag:          return "legendFlag";
    case ID_lightingFlag:        return "lightingFlag";
    case ID_minFlag:             return "minFlag";
    case ID_maxFlag:             return "maxFlag";
    case ID_centering:           return "centering";
    case ID_scaling:             return "scaling";
    case ID_limitsMode:          return "limitsMode";
    case ID_min:                 return "min";
    case ID_max:                 return "max";
    case ID_pointSize:           return "pointSize";
    case ID_pointType:           return "pointType";
    case ID_skewFactor:          return "skewFactor";
    case ID_opacity:             return "opacity";
    case ID_colorTableName:      return "colorTableName";
    case ID_smoothingLevel:      return "smoothingLevel";
    case ID_pointSizeVarEnabled: return "pointSizeVarEnabled";
    case ID_pointSizeVar:        return "pointSizeVar";
    case ID_pointSizePixels:     return "pointSizePixels";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PseudocolorAttributes::GetFieldType
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
PseudocolorAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_legendFlag:          return FieldType_bool;
    case ID_lightingFlag:        return FieldType_bool;
    case ID_minFlag:             return FieldType_bool;
    case ID_maxFlag:             return FieldType_bool;
    case ID_centering:           return FieldType_enum;
    case ID_scaling:             return FieldType_enum;
    case ID_limitsMode:          return FieldType_enum;
    case ID_min:                 return FieldType_double;
    case ID_max:                 return FieldType_double;
    case ID_pointSize:           return FieldType_double;
    case ID_pointType:           return FieldType_enum;
    case ID_skewFactor:          return FieldType_double;
    case ID_opacity:             return FieldType_opacity;
    case ID_colorTableName:      return FieldType_colortable;
    case ID_smoothingLevel:      return FieldType_int;
    case ID_pointSizeVarEnabled: return FieldType_bool;
    case ID_pointSizeVar:        return FieldType_variablename;
    case ID_pointSizePixels:     return FieldType_int;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: PseudocolorAttributes::GetFieldTypeName
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
PseudocolorAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_legendFlag:          return "bool";
    case ID_lightingFlag:        return "bool";
    case ID_minFlag:             return "bool";
    case ID_maxFlag:             return "bool";
    case ID_centering:           return "enum";
    case ID_scaling:             return "enum";
    case ID_limitsMode:          return "enum";
    case ID_min:                 return "double";
    case ID_max:                 return "double";
    case ID_pointSize:           return "double";
    case ID_pointType:           return "enum";
    case ID_skewFactor:          return "double";
    case ID_opacity:             return "opacity";
    case ID_colorTableName:      return "colortable";
    case ID_smoothingLevel:      return "int";
    case ID_pointSizeVarEnabled: return "bool";
    case ID_pointSizeVar:        return "variablename";
    case ID_pointSizePixels:     return "int";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PseudocolorAttributes::FieldsEqual
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
PseudocolorAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const PseudocolorAttributes &obj = *((const PseudocolorAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
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
    case ID_centering:
        {  // new scope
        retval = (centering == obj.centering);
        }
        break;
    case ID_scaling:
        {  // new scope
        retval = (scaling == obj.scaling);
        }
        break;
    case ID_limitsMode:
        {  // new scope
        retval = (limitsMode == obj.limitsMode);
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
    case ID_pointSize:
        {  // new scope
        retval = (pointSize == obj.pointSize);
        }
        break;
    case ID_pointType:
        {  // new scope
        retval = (pointType == obj.pointType);
        }
        break;
    case ID_skewFactor:
        {  // new scope
        retval = (skewFactor == obj.skewFactor);
        }
        break;
    case ID_opacity:
        {  // new scope
        retval = (opacity == obj.opacity);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_smoothingLevel:
        {  // new scope
        retval = (smoothingLevel == obj.smoothingLevel);
        }
        break;
    case ID_pointSizeVarEnabled:
        {  // new scope
        retval = (pointSizeVarEnabled == obj.pointSizeVarEnabled);
        }
        break;
    case ID_pointSizeVar:
        {  // new scope
        retval = (pointSizeVar == obj.pointSizeVar);
        }
        break;
    case ID_pointSizePixels:
        {  // new scope
        retval = (pointSizePixels == obj.pointSizePixels);
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
// Modifications:
//
//   Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001 
//   Only centering changes require the plot to be recalculated.
//   Removed scaling, pointsize and skewFactor from test. 
//
//   Hank Childs, Sun Jun 23 23:54:56 PDT 2002
//   Also recalculate if the point size changes.
//
//   Jeremy Meredith, Mon Dec  9 16:30:54 PST 2002
//   Added smoothing level.
//
//   Jeremy Meredith, Fri Dec 20 11:33:43 PST 2002
//   Added point size variable and its enabled flag.
//
//   Hank Childs, Thu Aug 21 22:28:25 PDT 2003
//   Added pointType.
//
//   Kathleen Bonnell, Wed Aug 18 18:08:52 PDT 2004 
//   Removed pointSize, pointType from test.  Added new conditions for
//   recalculation when pointSizeVar or pointSizeVarEnabled have changed. 
//
//   Kathleen Bonnell, Fri Nov 12 11:11:41 PST 2004 
//   Modified determination of when secondary var requires recalc. 
//
// ****************************************************************************
bool
PseudocolorAttributes::ChangesRequireRecalculation(const PseudocolorAttributes &obj) const
{
   bool needSecondaryVar = obj.pointSizeVarEnabled &&
                           pointSizeVar != obj.pointSizeVar &&
                           obj.pointSizeVar != "default" && 
                           obj.pointSizeVar != "" &&
                           obj.pointSizeVar != "\0"; 

    return (centering != obj.centering ||
            needSecondaryVar ||
            smoothingLevel != obj.smoothingLevel); 

}

// ****************************************************************************
// Method: PseudocolorAttributes::Print
//
// Purpose: Print the contents
//   
// Programmer: Jeremy Meredith
// Creation:   September 22, 2000
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 13 13:17:26 PST 2000
//   Added opacity.
//
//   Brad Whitlock, Tue Apr 23 12:03:27 PDT 2002
//   Ported to windows.
//
// ****************************************************************************

void
PseudocolorAttributes::Print(ostream &out, bool selected_only) const
{
    out << "{";

    for(int i = 0; i < NumAttributes(); ++i)
    {
        if(selected_only && !IsSelected(i))
            continue;

        switch(i)
        {
          case  0: out << "legendFlag=" << GetLegendFlag();    break;
          case  1: out << "lightingFlag=" << GetLightingFlag();break;
          case  2: out << "minFlag=" << GetMinFlag();          break;
          case  3: out << "maxFlag=" << GetMaxFlag();          break;
          case  4: out << "centering=" << GetCentering();      break;
          case  5: out << "scaling=" << GetScaling();          break;
          case  6: out << "limitsMode=" << GetLimitsMode();    break;
          case  7: out << "min=" << GetMin();                  break;
          case  8: out << "max=" << GetMax();                  break;
          case  9: out << "pointSize=" << GetPointSize();      break;
          case 10: out << "skewFactor=" << GetSkewFactor();    break;
          case 11: out << "opacity=" << GetOpacity();          break;
          case 12: out << "colorTableName=" << GetColorTableName().c_str(); break;
        }
        out << ",";
    }

    out << "}";
}

