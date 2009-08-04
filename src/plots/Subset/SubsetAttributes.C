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

#include <SubsetAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for SubsetAttributes::Subset_Type
//

static const char *Subset_Type_strings[] = {
"Domain", "Group", "Material", 
"EnumScalar", "Mesh", "Unknown"
};

std::string
SubsetAttributes::Subset_Type_ToString(SubsetAttributes::Subset_Type t)
{
    int index = int(t);
    if(index < 0 || index >= 6) index = 0;
    return Subset_Type_strings[index];
}

std::string
SubsetAttributes::Subset_Type_ToString(int t)
{
    int index = (t < 0 || t >= 6) ? 0 : t;
    return Subset_Type_strings[index];
}

bool
SubsetAttributes::Subset_Type_FromString(const std::string &s, SubsetAttributes::Subset_Type &val)
{
    val = SubsetAttributes::Domain;
    for(int i = 0; i < 6; ++i)
    {
        if(s == Subset_Type_strings[i])
        {
            val = (Subset_Type)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SubsetAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"ColorBySingleColor", "ColorByMultipleColors", "ColorByColorTable"
};

std::string
SubsetAttributes::ColoringMethod_ToString(SubsetAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ColoringMethod_strings[index];
}

std::string
SubsetAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
SubsetAttributes::ColoringMethod_FromString(const std::string &s, SubsetAttributes::ColoringMethod &val)
{
    val = SubsetAttributes::ColorBySingleColor;
    for(int i = 0; i < 3; ++i)
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
// Enum conversion methods for SubsetAttributes::PointType
//

static const char *PointType_strings[] = {
"Box", "Axis", "Icosahedron", 
"Point", "Sphere"};

std::string
SubsetAttributes::PointType_ToString(SubsetAttributes::PointType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return PointType_strings[index];
}

std::string
SubsetAttributes::PointType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return PointType_strings[index];
}

bool
SubsetAttributes::PointType_FromString(const std::string &s, SubsetAttributes::PointType &val)
{
    val = SubsetAttributes::Box;
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
const char *SubsetAttributes::TypeMapFormatString = "isbbiiaas*idbbidibsi";

// ****************************************************************************
// Method: SubsetAttributes::SubsetAttributes
//
// Purpose: 
//   Constructor for the SubsetAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SubsetAttributes::SubsetAttributes() : 
    AttributeSubject(SubsetAttributes::TypeMapFormatString),
    colorTableName("Default"), singleColor(), 
    pointSizeVar("default")
{
    colorType = ColorByMultipleColors;
    filledFlag = true;
    legendFlag = true;
    lineStyle = 0;
    lineWidth = 0;
    subsetType = Unknown;
    opacity = 1;
    wireframe = false;
    drawInternal = false;
    smoothingLevel = 0;
    pointSize = 0.05;
    pointType = Point;
    pointSizeVarEnabled = false;
    pointSizePixels = 2;
}

// ****************************************************************************
// Method: SubsetAttributes::SubsetAttributes
//
// Purpose: 
//   Copy constructor for the SubsetAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SubsetAttributes::SubsetAttributes(const SubsetAttributes &obj) : 
    AttributeSubject(SubsetAttributes::TypeMapFormatString)
{
    colorType = obj.colorType;
    colorTableName = obj.colorTableName;
    filledFlag = obj.filledFlag;
    legendFlag = obj.legendFlag;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    singleColor = obj.singleColor;
    multiColor = obj.multiColor;
    subsetNames = obj.subsetNames;
    subsetType = obj.subsetType;
    opacity = obj.opacity;
    wireframe = obj.wireframe;
    drawInternal = obj.drawInternal;
    smoothingLevel = obj.smoothingLevel;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;

    SelectAll();
}

// ****************************************************************************
// Method: SubsetAttributes::~SubsetAttributes
//
// Purpose: 
//   Destructor for the SubsetAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SubsetAttributes::~SubsetAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: SubsetAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the SubsetAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SubsetAttributes& 
SubsetAttributes::operator = (const SubsetAttributes &obj)
{
    if (this == &obj) return *this;
    colorType = obj.colorType;
    colorTableName = obj.colorTableName;
    filledFlag = obj.filledFlag;
    legendFlag = obj.legendFlag;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    singleColor = obj.singleColor;
    multiColor = obj.multiColor;
    subsetNames = obj.subsetNames;
    subsetType = obj.subsetType;
    opacity = obj.opacity;
    wireframe = obj.wireframe;
    drawInternal = obj.drawInternal;
    smoothingLevel = obj.smoothingLevel;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: SubsetAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the SubsetAttributes class.
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
SubsetAttributes::operator == (const SubsetAttributes &obj) const
{
    // Create the return value
    return ((colorType == obj.colorType) &&
            (colorTableName == obj.colorTableName) &&
            (filledFlag == obj.filledFlag) &&
            (legendFlag == obj.legendFlag) &&
            (lineStyle == obj.lineStyle) &&
            (lineWidth == obj.lineWidth) &&
            (singleColor == obj.singleColor) &&
            (multiColor == obj.multiColor) &&
            (subsetNames == obj.subsetNames) &&
            (subsetType == obj.subsetType) &&
            (opacity == obj.opacity) &&
            (wireframe == obj.wireframe) &&
            (drawInternal == obj.drawInternal) &&
            (smoothingLevel == obj.smoothingLevel) &&
            (pointSize == obj.pointSize) &&
            (pointType == obj.pointType) &&
            (pointSizeVarEnabled == obj.pointSizeVarEnabled) &&
            (pointSizeVar == obj.pointSizeVar) &&
            (pointSizePixels == obj.pointSizePixels));
}

// ****************************************************************************
// Method: SubsetAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the SubsetAttributes class.
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
SubsetAttributes::operator != (const SubsetAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: SubsetAttributes::TypeName
//
// Purpose: 
//   Type name method for the SubsetAttributes class.
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
SubsetAttributes::TypeName() const
{
    return "SubsetAttributes";
}

// ****************************************************************************
// Method: SubsetAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the SubsetAttributes class.
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
SubsetAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const SubsetAttributes *tmp = (const SubsetAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: SubsetAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the SubsetAttributes class.
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
SubsetAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new SubsetAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: SubsetAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the SubsetAttributes class.
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
SubsetAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new SubsetAttributes(*this);
    else
        retval = new SubsetAttributes;

    return retval;
}

// ****************************************************************************
// Method: SubsetAttributes::SelectAll
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
SubsetAttributes::SelectAll()
{
    Select(ID_colorType,           (void *)&colorType);
    Select(ID_colorTableName,      (void *)&colorTableName);
    Select(ID_filledFlag,          (void *)&filledFlag);
    Select(ID_legendFlag,          (void *)&legendFlag);
    Select(ID_lineStyle,           (void *)&lineStyle);
    Select(ID_lineWidth,           (void *)&lineWidth);
    Select(ID_singleColor,         (void *)&singleColor);
    Select(ID_multiColor,          (void *)&multiColor);
    Select(ID_subsetNames,         (void *)&subsetNames);
    Select(ID_subsetType,          (void *)&subsetType);
    Select(ID_opacity,             (void *)&opacity);
    Select(ID_wireframe,           (void *)&wireframe);
    Select(ID_drawInternal,        (void *)&drawInternal);
    Select(ID_smoothingLevel,      (void *)&smoothingLevel);
    Select(ID_pointSize,           (void *)&pointSize);
    Select(ID_pointType,           (void *)&pointType);
    Select(ID_pointSizeVarEnabled, (void *)&pointSizeVarEnabled);
    Select(ID_pointSizeVar,        (void *)&pointSizeVar);
    Select(ID_pointSizePixels,     (void *)&pointSizePixels);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SubsetAttributes::CreateNode
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
SubsetAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    SubsetAttributes defaultObject;
    bool addToParent = false;
    // Create a node for SubsetAttributes.
    DataNode *node = new DataNode("SubsetAttributes");

    if(completeSave || !FieldsEqual(ID_colorType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorType", ColoringMethod_ToString(colorType)));
    }

    if(completeSave || !FieldsEqual(ID_colorTableName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

    if(completeSave || !FieldsEqual(ID_filledFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("filledFlag", filledFlag));
    }

    if(completeSave || !FieldsEqual(ID_legendFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
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

        DataNode *singleColorNode = new DataNode("singleColor");
        if(singleColor.CreateNode(singleColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(singleColorNode);
        }
        else
            delete singleColorNode;
    if(completeSave || !FieldsEqual(ID_multiColor, &defaultObject))
    {
        DataNode *multiColorNode = new DataNode("multiColor");
        if(multiColor.CreateNode(multiColorNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(multiColorNode);
        }
        else
            delete multiColorNode;
    }

    if(completeSave || !FieldsEqual(ID_subsetNames, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("subsetNames", subsetNames));
    }

    if(completeSave || !FieldsEqual(ID_subsetType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("subsetType", Subset_Type_ToString(subsetType)));
    }

    if(completeSave || !FieldsEqual(ID_opacity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacity", opacity));
    }

    if(completeSave || !FieldsEqual(ID_wireframe, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wireframe", wireframe));
    }

    if(completeSave || !FieldsEqual(ID_drawInternal, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("drawInternal", drawInternal));
    }

    if(completeSave || !FieldsEqual(ID_smoothingLevel, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("smoothingLevel", smoothingLevel));
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
// Method: SubsetAttributes::SetFromNode
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
SubsetAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("SubsetAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("colorType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetColorType(ColoringMethod(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ColoringMethod value;
            if(ColoringMethod_FromString(node->AsString(), value))
                SetColorType(value);
        }
    }
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("filledFlag")) != 0)
        SetFilledFlag(node->AsBool());
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lineStyle")) != 0)
        SetLineStyle(node->AsInt());
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("multiColor")) != 0)
        multiColor.SetFromNode(node);
    if((node = searchNode->GetNode("subsetNames")) != 0)
        SetSubsetNames(node->AsStringVector());
    if((node = searchNode->GetNode("subsetType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 6)
                SetSubsetType(Subset_Type(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Subset_Type value;
            if(Subset_Type_FromString(node->AsString(), value))
                SetSubsetType(value);
        }
    }
    if((node = searchNode->GetNode("opacity")) != 0)
        SetOpacity(node->AsDouble());
    if((node = searchNode->GetNode("wireframe")) != 0)
        SetWireframe(node->AsBool());
    if((node = searchNode->GetNode("drawInternal")) != 0)
        SetDrawInternal(node->AsBool());
    if((node = searchNode->GetNode("smoothingLevel")) != 0)
        SetSmoothingLevel(node->AsInt());
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
SubsetAttributes::SetColorType(SubsetAttributes::ColoringMethod colorType_)
{
    colorType = colorType_;
    Select(ID_colorType, (void *)&colorType);
}

void
SubsetAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
SubsetAttributes::SetFilledFlag(bool filledFlag_)
{
    filledFlag = filledFlag_;
    Select(ID_filledFlag, (void *)&filledFlag);
}

void
SubsetAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
SubsetAttributes::SetLineStyle(int lineStyle_)
{
    lineStyle = lineStyle_;
    Select(ID_lineStyle, (void *)&lineStyle);
}

void
SubsetAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
SubsetAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
SubsetAttributes::SetMultiColor(const ColorAttributeList &multiColor_)
{
    multiColor = multiColor_;
    Select(ID_multiColor, (void *)&multiColor);
}

void
SubsetAttributes::SetSubsetNames(const stringVector &subsetNames_)
{
    subsetNames = subsetNames_;
    Select(ID_subsetNames, (void *)&subsetNames);
}

void
SubsetAttributes::SetSubsetType(SubsetAttributes::Subset_Type subsetType_)
{
    subsetType = subsetType_;
    Select(ID_subsetType, (void *)&subsetType);
}

void
SubsetAttributes::SetOpacity(double opacity_)
{
    opacity = opacity_;
    Select(ID_opacity, (void *)&opacity);
}

void
SubsetAttributes::SetWireframe(bool wireframe_)
{
    wireframe = wireframe_;
    Select(ID_wireframe, (void *)&wireframe);
}

void
SubsetAttributes::SetDrawInternal(bool drawInternal_)
{
    drawInternal = drawInternal_;
    Select(ID_drawInternal, (void *)&drawInternal);
}

void
SubsetAttributes::SetSmoothingLevel(int smoothingLevel_)
{
    smoothingLevel = smoothingLevel_;
    Select(ID_smoothingLevel, (void *)&smoothingLevel);
}

void
SubsetAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(ID_pointSize, (void *)&pointSize);
}

void
SubsetAttributes::SetPointType(SubsetAttributes::PointType pointType_)
{
    pointType = pointType_;
    Select(ID_pointType, (void *)&pointType);
}

void
SubsetAttributes::SetPointSizeVarEnabled(bool pointSizeVarEnabled_)
{
    pointSizeVarEnabled = pointSizeVarEnabled_;
    Select(ID_pointSizeVarEnabled, (void *)&pointSizeVarEnabled);
}

void
SubsetAttributes::SetPointSizeVar(const std::string &pointSizeVar_)
{
    pointSizeVar = pointSizeVar_;
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

void
SubsetAttributes::SetPointSizePixels(int pointSizePixels_)
{
    pointSizePixels = pointSizePixels_;
    Select(ID_pointSizePixels, (void *)&pointSizePixels);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

SubsetAttributes::ColoringMethod
SubsetAttributes::GetColorType() const
{
    return ColoringMethod(colorType);
}

const std::string &
SubsetAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
SubsetAttributes::GetColorTableName()
{
    return colorTableName;
}

bool
SubsetAttributes::GetFilledFlag() const
{
    return filledFlag;
}

bool
SubsetAttributes::GetLegendFlag() const
{
    return legendFlag;
}

int
SubsetAttributes::GetLineStyle() const
{
    return lineStyle;
}

int
SubsetAttributes::GetLineWidth() const
{
    return lineWidth;
}

const ColorAttribute &
SubsetAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
SubsetAttributes::GetSingleColor()
{
    return singleColor;
}

const ColorAttributeList &
SubsetAttributes::GetMultiColor() const
{
    return multiColor;
}

ColorAttributeList &
SubsetAttributes::GetMultiColor()
{
    return multiColor;
}

const stringVector &
SubsetAttributes::GetSubsetNames() const
{
    return subsetNames;
}

stringVector &
SubsetAttributes::GetSubsetNames()
{
    return subsetNames;
}

SubsetAttributes::Subset_Type
SubsetAttributes::GetSubsetType() const
{
    return Subset_Type(subsetType);
}

double
SubsetAttributes::GetOpacity() const
{
    return opacity;
}

bool
SubsetAttributes::GetWireframe() const
{
    return wireframe;
}

bool
SubsetAttributes::GetDrawInternal() const
{
    return drawInternal;
}

int
SubsetAttributes::GetSmoothingLevel() const
{
    return smoothingLevel;
}

double
SubsetAttributes::GetPointSize() const
{
    return pointSize;
}

SubsetAttributes::PointType
SubsetAttributes::GetPointType() const
{
    return PointType(pointType);
}

bool
SubsetAttributes::GetPointSizeVarEnabled() const
{
    return pointSizeVarEnabled;
}

const std::string &
SubsetAttributes::GetPointSizeVar() const
{
    return pointSizeVar;
}

std::string &
SubsetAttributes::GetPointSizeVar()
{
    return pointSizeVar;
}

int
SubsetAttributes::GetPointSizePixels() const
{
    return pointSizePixels;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
SubsetAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
SubsetAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

void
SubsetAttributes::SelectMultiColor()
{
    Select(ID_multiColor, (void *)&multiColor);
}

void
SubsetAttributes::SelectSubsetNames()
{
    Select(ID_subsetNames, (void *)&subsetNames);
}

void
SubsetAttributes::SelectPointSizeVar()
{
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SubsetAttributes::GetFieldName
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
SubsetAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_colorType:           return "colorType";
    case ID_colorTableName:      return "colorTableName";
    case ID_filledFlag:          return "filledFlag";
    case ID_legendFlag:          return "legendFlag";
    case ID_lineStyle:           return "lineStyle";
    case ID_lineWidth:           return "lineWidth";
    case ID_singleColor:         return "singleColor";
    case ID_multiColor:          return "multiColor";
    case ID_subsetNames:         return "subsetNames";
    case ID_subsetType:          return "subsetType";
    case ID_opacity:             return "opacity";
    case ID_wireframe:           return "wireframe";
    case ID_drawInternal:        return "drawInternal";
    case ID_smoothingLevel:      return "smoothingLevel";
    case ID_pointSize:           return "pointSize";
    case ID_pointType:           return "pointType";
    case ID_pointSizeVarEnabled: return "pointSizeVarEnabled";
    case ID_pointSizeVar:        return "pointSizeVar";
    case ID_pointSizePixels:     return "pointSizePixels";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SubsetAttributes::GetFieldType
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
SubsetAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_colorType:           return FieldType_enum;
    case ID_colorTableName:      return FieldType_colortable;
    case ID_filledFlag:          return FieldType_bool;
    case ID_legendFlag:          return FieldType_bool;
    case ID_lineStyle:           return FieldType_linestyle;
    case ID_lineWidth:           return FieldType_linewidth;
    case ID_singleColor:         return FieldType_color;
    case ID_multiColor:          return FieldType_att;
    case ID_subsetNames:         return FieldType_stringVector;
    case ID_subsetType:          return FieldType_enum;
    case ID_opacity:             return FieldType_opacity;
    case ID_wireframe:           return FieldType_bool;
    case ID_drawInternal:        return FieldType_bool;
    case ID_smoothingLevel:      return FieldType_int;
    case ID_pointSize:           return FieldType_double;
    case ID_pointType:           return FieldType_enum;
    case ID_pointSizeVarEnabled: return FieldType_bool;
    case ID_pointSizeVar:        return FieldType_variablename;
    case ID_pointSizePixels:     return FieldType_int;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: SubsetAttributes::GetFieldTypeName
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
SubsetAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_colorType:           return "enum";
    case ID_colorTableName:      return "colortable";
    case ID_filledFlag:          return "bool";
    case ID_legendFlag:          return "bool";
    case ID_lineStyle:           return "linestyle";
    case ID_lineWidth:           return "linewidth";
    case ID_singleColor:         return "color";
    case ID_multiColor:          return "att";
    case ID_subsetNames:         return "stringVector";
    case ID_subsetType:          return "enum";
    case ID_opacity:             return "opacity";
    case ID_wireframe:           return "bool";
    case ID_drawInternal:        return "bool";
    case ID_smoothingLevel:      return "int";
    case ID_pointSize:           return "double";
    case ID_pointType:           return "enum";
    case ID_pointSizeVarEnabled: return "bool";
    case ID_pointSizeVar:        return "variablename";
    case ID_pointSizePixels:     return "int";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SubsetAttributes::FieldsEqual
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
SubsetAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const SubsetAttributes &obj = *((const SubsetAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_colorType:
        {  // new scope
        retval = (colorType == obj.colorType);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_filledFlag:
        {  // new scope
        retval = (filledFlag == obj.filledFlag);
        }
        break;
    case ID_legendFlag:
        {  // new scope
        retval = (legendFlag == obj.legendFlag);
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
    case ID_singleColor:
        {  // new scope
        retval = (singleColor == obj.singleColor);
        }
        break;
    case ID_multiColor:
        {  // new scope
        retval = (multiColor == obj.multiColor);
        }
        break;
    case ID_subsetNames:
        {  // new scope
        retval = (subsetNames == obj.subsetNames);
        }
        break;
    case ID_subsetType:
        {  // new scope
        retval = (subsetType == obj.subsetType);
        }
        break;
    case ID_opacity:
        {  // new scope
        retval = (opacity == obj.opacity);
        }
        break;
    case ID_wireframe:
        {  // new scope
        retval = (wireframe == obj.wireframe);
        }
        break;
    case ID_drawInternal:
        {  // new scope
        retval = (drawInternal == obj.drawInternal);
        }
        break;
    case ID_smoothingLevel:
        {  // new scope
        retval = (smoothingLevel == obj.smoothingLevel);
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
//    Jeremy Meredith, Mon Dec  9 16:30:54 PST 2002
//    Added smoothing level.
//
//    Kathleen Bonnell, Fri Nov 12 11:42:36 PST 2004 
//    Added needSecondaryVar. 
//
// ****************************************************************************
bool
SubsetAttributes::ChangesRequireRecalculation(const SubsetAttributes &obj)
{
    bool needSecondaryVar = obj.pointSizeVarEnabled &&
                            pointSizeVar != obj.pointSizeVar &&
                            obj.pointSizeVar != "default" && 
                            obj.pointSizeVar != "" &&
                            obj.pointSizeVar != "\0"; 

    return ((filledFlag != obj.filledFlag) ||
            (subsetType != obj.subsetType) || 
            (subsetNames != obj.subsetNames) ||
            (wireframe != obj.wireframe) ||
            (drawInternal != obj.drawInternal) ||
            (smoothingLevel != obj.smoothingLevel) ||
            needSecondaryVar);
}

bool
SubsetAttributes::VarChangeRequiresReset()
{ 
    return true;
}

