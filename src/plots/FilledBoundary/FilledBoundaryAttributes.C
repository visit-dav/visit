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

#include <FilledBoundaryAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for FilledBoundaryAttributes::Boundary_Type
//

static const char *Boundary_Type_strings[] = {
"Domain", "Group", "Material", 
"Unknown"};

std::string
FilledBoundaryAttributes::Boundary_Type_ToString(FilledBoundaryAttributes::Boundary_Type t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return Boundary_Type_strings[index];
}

std::string
FilledBoundaryAttributes::Boundary_Type_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return Boundary_Type_strings[index];
}

bool
FilledBoundaryAttributes::Boundary_Type_FromString(const std::string &s, FilledBoundaryAttributes::Boundary_Type &val)
{
    val = FilledBoundaryAttributes::Domain;
    for(int i = 0; i < 4; ++i)
    {
        if(s == Boundary_Type_strings[i])
        {
            val = (Boundary_Type)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for FilledBoundaryAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"ColorBySingleColor", "ColorByMultipleColors", "ColorByColorTable"
};

std::string
FilledBoundaryAttributes::ColoringMethod_ToString(FilledBoundaryAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ColoringMethod_strings[index];
}

std::string
FilledBoundaryAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
FilledBoundaryAttributes::ColoringMethod_FromString(const std::string &s, FilledBoundaryAttributes::ColoringMethod &val)
{
    val = FilledBoundaryAttributes::ColorBySingleColor;
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
// Enum conversion methods for FilledBoundaryAttributes::PointType
//

static const char *PointType_strings[] = {
"Box", "Axis", "Icosahedron", 
"Point", "Sphere"};

std::string
FilledBoundaryAttributes::PointType_ToString(FilledBoundaryAttributes::PointType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return PointType_strings[index];
}

std::string
FilledBoundaryAttributes::PointType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return PointType_strings[index];
}

bool
FilledBoundaryAttributes::PointType_FromString(const std::string &s, FilledBoundaryAttributes::PointType &val)
{
    val = FilledBoundaryAttributes::Box;
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
const char *FilledBoundaryAttributes::TypeMapFormatString = "isbbiiaas*idbbibadibsi";

// ****************************************************************************
// Method: FilledBoundaryAttributes::FilledBoundaryAttributes
//
// Purpose: 
//   Constructor for the FilledBoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

FilledBoundaryAttributes::FilledBoundaryAttributes() : 
    AttributeSubject(FilledBoundaryAttributes::TypeMapFormatString),
    colorTableName("Default"), singleColor(), 
    mixedColor(255, 255, 255), pointSizeVar("default")
{
    colorType = ColorByMultipleColors;
    filledFlag = true;
    legendFlag = true;
    lineStyle = 0;
    lineWidth = 0;
    boundaryType = Unknown;
    opacity = 1;
    wireframe = false;
    drawInternal = false;
    smoothingLevel = 0;
    cleanZonesOnly = false;
    pointSize = 0.05;
    pointType = Point;
    pointSizeVarEnabled = false;
    pointSizePixels = 2;
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::FilledBoundaryAttributes
//
// Purpose: 
//   Copy constructor for the FilledBoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

FilledBoundaryAttributes::FilledBoundaryAttributes(const FilledBoundaryAttributes &obj) : 
    AttributeSubject(FilledBoundaryAttributes::TypeMapFormatString)
{
    colorType = obj.colorType;
    colorTableName = obj.colorTableName;
    filledFlag = obj.filledFlag;
    legendFlag = obj.legendFlag;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    singleColor = obj.singleColor;
    multiColor = obj.multiColor;
    boundaryNames = obj.boundaryNames;
    boundaryType = obj.boundaryType;
    opacity = obj.opacity;
    wireframe = obj.wireframe;
    drawInternal = obj.drawInternal;
    smoothingLevel = obj.smoothingLevel;
    cleanZonesOnly = obj.cleanZonesOnly;
    mixedColor = obj.mixedColor;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;

    SelectAll();
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::~FilledBoundaryAttributes
//
// Purpose: 
//   Destructor for the FilledBoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

FilledBoundaryAttributes::~FilledBoundaryAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the FilledBoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

FilledBoundaryAttributes& 
FilledBoundaryAttributes::operator = (const FilledBoundaryAttributes &obj)
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
    boundaryNames = obj.boundaryNames;
    boundaryType = obj.boundaryType;
    opacity = obj.opacity;
    wireframe = obj.wireframe;
    drawInternal = obj.drawInternal;
    smoothingLevel = obj.smoothingLevel;
    cleanZonesOnly = obj.cleanZonesOnly;
    mixedColor = obj.mixedColor;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the FilledBoundaryAttributes class.
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
FilledBoundaryAttributes::operator == (const FilledBoundaryAttributes &obj) const
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
            (boundaryNames == obj.boundaryNames) &&
            (boundaryType == obj.boundaryType) &&
            (opacity == obj.opacity) &&
            (wireframe == obj.wireframe) &&
            (drawInternal == obj.drawInternal) &&
            (smoothingLevel == obj.smoothingLevel) &&
            (cleanZonesOnly == obj.cleanZonesOnly) &&
            (mixedColor == obj.mixedColor) &&
            (pointSize == obj.pointSize) &&
            (pointType == obj.pointType) &&
            (pointSizeVarEnabled == obj.pointSizeVarEnabled) &&
            (pointSizeVar == obj.pointSizeVar) &&
            (pointSizePixels == obj.pointSizePixels));
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the FilledBoundaryAttributes class.
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
FilledBoundaryAttributes::operator != (const FilledBoundaryAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::TypeName
//
// Purpose: 
//   Type name method for the FilledBoundaryAttributes class.
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
FilledBoundaryAttributes::TypeName() const
{
    return "FilledBoundaryAttributes";
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the FilledBoundaryAttributes class.
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
FilledBoundaryAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const FilledBoundaryAttributes *tmp = (const FilledBoundaryAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the FilledBoundaryAttributes class.
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
FilledBoundaryAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new FilledBoundaryAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the FilledBoundaryAttributes class.
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
FilledBoundaryAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new FilledBoundaryAttributes(*this);
    else
        retval = new FilledBoundaryAttributes;

    return retval;
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::SelectAll
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
FilledBoundaryAttributes::SelectAll()
{
    Select(ID_colorType,           (void *)&colorType);
    Select(ID_colorTableName,      (void *)&colorTableName);
    Select(ID_filledFlag,          (void *)&filledFlag);
    Select(ID_legendFlag,          (void *)&legendFlag);
    Select(ID_lineStyle,           (void *)&lineStyle);
    Select(ID_lineWidth,           (void *)&lineWidth);
    Select(ID_singleColor,         (void *)&singleColor);
    Select(ID_multiColor,          (void *)&multiColor);
    Select(ID_boundaryNames,       (void *)&boundaryNames);
    Select(ID_boundaryType,        (void *)&boundaryType);
    Select(ID_opacity,             (void *)&opacity);
    Select(ID_wireframe,           (void *)&wireframe);
    Select(ID_drawInternal,        (void *)&drawInternal);
    Select(ID_smoothingLevel,      (void *)&smoothingLevel);
    Select(ID_cleanZonesOnly,      (void *)&cleanZonesOnly);
    Select(ID_mixedColor,          (void *)&mixedColor);
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
// Method: FilledBoundaryAttributes::CreateNode
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
FilledBoundaryAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    FilledBoundaryAttributes defaultObject;
    bool addToParent = false;
    // Create a node for FilledBoundaryAttributes.
    DataNode *node = new DataNode("FilledBoundaryAttributes");

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

    if(completeSave || !FieldsEqual(ID_boundaryNames, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("boundaryNames", boundaryNames));
    }

    if(completeSave || !FieldsEqual(ID_boundaryType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("boundaryType", Boundary_Type_ToString(boundaryType)));
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

    if(completeSave || !FieldsEqual(ID_cleanZonesOnly, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cleanZonesOnly", cleanZonesOnly));
    }

        DataNode *mixedColorNode = new DataNode("mixedColor");
        if(mixedColor.CreateNode(mixedColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(mixedColorNode);
        }
        else
            delete mixedColorNode;
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
// Method: FilledBoundaryAttributes::SetFromNode
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
FilledBoundaryAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("FilledBoundaryAttributes");
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
    if((node = searchNode->GetNode("boundaryNames")) != 0)
        SetBoundaryNames(node->AsStringVector());
    if((node = searchNode->GetNode("boundaryType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetBoundaryType(Boundary_Type(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Boundary_Type value;
            if(Boundary_Type_FromString(node->AsString(), value))
                SetBoundaryType(value);
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
    if((node = searchNode->GetNode("cleanZonesOnly")) != 0)
        SetCleanZonesOnly(node->AsBool());
    if((node = searchNode->GetNode("mixedColor")) != 0)
        mixedColor.SetFromNode(node);
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
    // We are no longer using MultiColor as the source for the "mixed"
    // color (in clean zones only), but someone may have saved their
    // settings with the "mixed" color in the list.  Remove it if it's
    // in there.
    //
    // NOTE: This code can be removed in a few months, as soon as
    // 1.3 is at least a few versions old, since it represents a little
    // bit of a hack.
    bool done = false;
    while (!done)
    {
        done = true;
        size_t index;
        for (index=0; index<boundaryNames.size(); index++)
        {
            if (boundaryNames[index] == "mixed")
            {
                done = false;
                break;
            }
        }
        if (!done)
        {
            multiColor.RemoveColors(index);
            for (size_t i=index+1; i<boundaryNames.size(); i++)
                boundaryNames[i-1] = boundaryNames[i];
            boundaryNames.resize(boundaryNames.size() - 1);
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
FilledBoundaryAttributes::SetColorType(FilledBoundaryAttributes::ColoringMethod colorType_)
{
    colorType = colorType_;
    Select(ID_colorType, (void *)&colorType);
}

void
FilledBoundaryAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
FilledBoundaryAttributes::SetFilledFlag(bool filledFlag_)
{
    filledFlag = filledFlag_;
    Select(ID_filledFlag, (void *)&filledFlag);
}

void
FilledBoundaryAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
FilledBoundaryAttributes::SetLineStyle(int lineStyle_)
{
    lineStyle = lineStyle_;
    Select(ID_lineStyle, (void *)&lineStyle);
}

void
FilledBoundaryAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
FilledBoundaryAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
FilledBoundaryAttributes::SetMultiColor(const ColorAttributeList &multiColor_)
{
    multiColor = multiColor_;
    Select(ID_multiColor, (void *)&multiColor);
}

void
FilledBoundaryAttributes::SetBoundaryNames(const stringVector &boundaryNames_)
{
    boundaryNames = boundaryNames_;
    Select(ID_boundaryNames, (void *)&boundaryNames);
}

void
FilledBoundaryAttributes::SetBoundaryType(FilledBoundaryAttributes::Boundary_Type boundaryType_)
{
    boundaryType = boundaryType_;
    Select(ID_boundaryType, (void *)&boundaryType);
}

void
FilledBoundaryAttributes::SetOpacity(double opacity_)
{
    opacity = opacity_;
    Select(ID_opacity, (void *)&opacity);
}

void
FilledBoundaryAttributes::SetWireframe(bool wireframe_)
{
    wireframe = wireframe_;
    Select(ID_wireframe, (void *)&wireframe);
}

void
FilledBoundaryAttributes::SetDrawInternal(bool drawInternal_)
{
    drawInternal = drawInternal_;
    Select(ID_drawInternal, (void *)&drawInternal);
}

void
FilledBoundaryAttributes::SetSmoothingLevel(int smoothingLevel_)
{
    smoothingLevel = smoothingLevel_;
    Select(ID_smoothingLevel, (void *)&smoothingLevel);
}

void
FilledBoundaryAttributes::SetCleanZonesOnly(bool cleanZonesOnly_)
{
    cleanZonesOnly = cleanZonesOnly_;
    Select(ID_cleanZonesOnly, (void *)&cleanZonesOnly);
}

void
FilledBoundaryAttributes::SetMixedColor(const ColorAttribute &mixedColor_)
{
    mixedColor = mixedColor_;
    Select(ID_mixedColor, (void *)&mixedColor);
}

void
FilledBoundaryAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(ID_pointSize, (void *)&pointSize);
}

void
FilledBoundaryAttributes::SetPointType(FilledBoundaryAttributes::PointType pointType_)
{
    pointType = pointType_;
    Select(ID_pointType, (void *)&pointType);
}

void
FilledBoundaryAttributes::SetPointSizeVarEnabled(bool pointSizeVarEnabled_)
{
    pointSizeVarEnabled = pointSizeVarEnabled_;
    Select(ID_pointSizeVarEnabled, (void *)&pointSizeVarEnabled);
}

void
FilledBoundaryAttributes::SetPointSizeVar(const std::string &pointSizeVar_)
{
    pointSizeVar = pointSizeVar_;
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

void
FilledBoundaryAttributes::SetPointSizePixels(int pointSizePixels_)
{
    pointSizePixels = pointSizePixels_;
    Select(ID_pointSizePixels, (void *)&pointSizePixels);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

FilledBoundaryAttributes::ColoringMethod
FilledBoundaryAttributes::GetColorType() const
{
    return ColoringMethod(colorType);
}

const std::string &
FilledBoundaryAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
FilledBoundaryAttributes::GetColorTableName()
{
    return colorTableName;
}

bool
FilledBoundaryAttributes::GetFilledFlag() const
{
    return filledFlag;
}

bool
FilledBoundaryAttributes::GetLegendFlag() const
{
    return legendFlag;
}

int
FilledBoundaryAttributes::GetLineStyle() const
{
    return lineStyle;
}

int
FilledBoundaryAttributes::GetLineWidth() const
{
    return lineWidth;
}

const ColorAttribute &
FilledBoundaryAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
FilledBoundaryAttributes::GetSingleColor()
{
    return singleColor;
}

const ColorAttributeList &
FilledBoundaryAttributes::GetMultiColor() const
{
    return multiColor;
}

ColorAttributeList &
FilledBoundaryAttributes::GetMultiColor()
{
    return multiColor;
}

const stringVector &
FilledBoundaryAttributes::GetBoundaryNames() const
{
    return boundaryNames;
}

stringVector &
FilledBoundaryAttributes::GetBoundaryNames()
{
    return boundaryNames;
}

FilledBoundaryAttributes::Boundary_Type
FilledBoundaryAttributes::GetBoundaryType() const
{
    return Boundary_Type(boundaryType);
}

double
FilledBoundaryAttributes::GetOpacity() const
{
    return opacity;
}

bool
FilledBoundaryAttributes::GetWireframe() const
{
    return wireframe;
}

bool
FilledBoundaryAttributes::GetDrawInternal() const
{
    return drawInternal;
}

int
FilledBoundaryAttributes::GetSmoothingLevel() const
{
    return smoothingLevel;
}

bool
FilledBoundaryAttributes::GetCleanZonesOnly() const
{
    return cleanZonesOnly;
}

const ColorAttribute &
FilledBoundaryAttributes::GetMixedColor() const
{
    return mixedColor;
}

ColorAttribute &
FilledBoundaryAttributes::GetMixedColor()
{
    return mixedColor;
}

double
FilledBoundaryAttributes::GetPointSize() const
{
    return pointSize;
}

FilledBoundaryAttributes::PointType
FilledBoundaryAttributes::GetPointType() const
{
    return PointType(pointType);
}

bool
FilledBoundaryAttributes::GetPointSizeVarEnabled() const
{
    return pointSizeVarEnabled;
}

const std::string &
FilledBoundaryAttributes::GetPointSizeVar() const
{
    return pointSizeVar;
}

std::string &
FilledBoundaryAttributes::GetPointSizeVar()
{
    return pointSizeVar;
}

int
FilledBoundaryAttributes::GetPointSizePixels() const
{
    return pointSizePixels;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
FilledBoundaryAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
FilledBoundaryAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

void
FilledBoundaryAttributes::SelectMultiColor()
{
    Select(ID_multiColor, (void *)&multiColor);
}

void
FilledBoundaryAttributes::SelectBoundaryNames()
{
    Select(ID_boundaryNames, (void *)&boundaryNames);
}

void
FilledBoundaryAttributes::SelectMixedColor()
{
    Select(ID_mixedColor, (void *)&mixedColor);
}

void
FilledBoundaryAttributes::SelectPointSizeVar()
{
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: FilledBoundaryAttributes::GetFieldName
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
FilledBoundaryAttributes::GetFieldName(int index) const
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
    case ID_boundaryNames:       return "boundaryNames";
    case ID_boundaryType:        return "boundaryType";
    case ID_opacity:             return "opacity";
    case ID_wireframe:           return "wireframe";
    case ID_drawInternal:        return "drawInternal";
    case ID_smoothingLevel:      return "smoothingLevel";
    case ID_cleanZonesOnly:      return "cleanZonesOnly";
    case ID_mixedColor:          return "mixedColor";
    case ID_pointSize:           return "pointSize";
    case ID_pointType:           return "pointType";
    case ID_pointSizeVarEnabled: return "pointSizeVarEnabled";
    case ID_pointSizeVar:        return "pointSizeVar";
    case ID_pointSizePixels:     return "pointSizePixels";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::GetFieldType
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
FilledBoundaryAttributes::GetFieldType(int index) const
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
    case ID_boundaryNames:       return FieldType_stringVector;
    case ID_boundaryType:        return FieldType_enum;
    case ID_opacity:             return FieldType_opacity;
    case ID_wireframe:           return FieldType_bool;
    case ID_drawInternal:        return FieldType_bool;
    case ID_smoothingLevel:      return FieldType_int;
    case ID_cleanZonesOnly:      return FieldType_bool;
    case ID_mixedColor:          return FieldType_color;
    case ID_pointSize:           return FieldType_double;
    case ID_pointType:           return FieldType_enum;
    case ID_pointSizeVarEnabled: return FieldType_bool;
    case ID_pointSizeVar:        return FieldType_variablename;
    case ID_pointSizePixels:     return FieldType_int;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::GetFieldTypeName
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
FilledBoundaryAttributes::GetFieldTypeName(int index) const
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
    case ID_boundaryNames:       return "stringVector";
    case ID_boundaryType:        return "enum";
    case ID_opacity:             return "opacity";
    case ID_wireframe:           return "bool";
    case ID_drawInternal:        return "bool";
    case ID_smoothingLevel:      return "int";
    case ID_cleanZonesOnly:      return "bool";
    case ID_mixedColor:          return "color";
    case ID_pointSize:           return "double";
    case ID_pointType:           return "enum";
    case ID_pointSizeVarEnabled: return "bool";
    case ID_pointSizeVar:        return "variablename";
    case ID_pointSizePixels:     return "int";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: FilledBoundaryAttributes::FieldsEqual
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
FilledBoundaryAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const FilledBoundaryAttributes &obj = *((const FilledBoundaryAttributes*)rhs);
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
    case ID_boundaryNames:
        {  // new scope
        retval = (boundaryNames == obj.boundaryNames);
        }
        break;
    case ID_boundaryType:
        {  // new scope
        retval = (boundaryType == obj.boundaryType);
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
    case ID_cleanZonesOnly:
        {  // new scope
        retval = (cleanZonesOnly == obj.cleanZonesOnly);
        }
        break;
    case ID_mixedColor:
        {  // new scope
        retval = (mixedColor == obj.mixedColor);
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
//    Jeremy Meredith, Fri Jun 13 16:58:16 PDT 2003
//    Added cleanZonesOnly.
//
//    Kathleen Bonnell, Wed Nov 10 09:22:35 PST 2004 
//    Added needSecondaryVar.
//
// ****************************************************************************
bool
FilledBoundaryAttributes::ChangesRequireRecalculation(const FilledBoundaryAttributes &obj)
{
    bool needSecondaryVar = obj.pointSizeVarEnabled &&
                            pointSizeVar != obj.pointSizeVar &&
                            obj.pointSizeVar != "default" && 
                            obj.pointSizeVar != "" &&
                            obj.pointSizeVar != "\0"; 

    return ((filledFlag != obj.filledFlag) ||
            (boundaryType != obj.boundaryType) || 
            (boundaryNames != obj.boundaryNames) ||
            (wireframe != obj.wireframe) ||
            (drawInternal != obj.drawInternal) ||
            (smoothingLevel != obj.smoothingLevel) ||
            (cleanZonesOnly != obj.cleanZonesOnly) ||
            needSecondaryVar);
}

bool
FilledBoundaryAttributes::VarChangeRequiresReset()
{ 
    return true;
}

