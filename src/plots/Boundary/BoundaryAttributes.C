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

#include <BoundaryAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for BoundaryAttributes::Boundary_Type
//

static const char *Boundary_Type_strings[] = {
"Domain", "Group", "Material", 
"Unknown"};

std::string
BoundaryAttributes::Boundary_Type_ToString(BoundaryAttributes::Boundary_Type t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return Boundary_Type_strings[index];
}

std::string
BoundaryAttributes::Boundary_Type_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return Boundary_Type_strings[index];
}

bool
BoundaryAttributes::Boundary_Type_FromString(const std::string &s, BoundaryAttributes::Boundary_Type &val)
{
    val = BoundaryAttributes::Domain;
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
// Enum conversion methods for BoundaryAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"ColorBySingleColor", "ColorByMultipleColors", "ColorByColorTable"
};

std::string
BoundaryAttributes::ColoringMethod_ToString(BoundaryAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ColoringMethod_strings[index];
}

std::string
BoundaryAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
BoundaryAttributes::ColoringMethod_FromString(const std::string &s, BoundaryAttributes::ColoringMethod &val)
{
    val = BoundaryAttributes::ColorBySingleColor;
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
// Enum conversion methods for BoundaryAttributes::PointType
//

static const char *PointType_strings[] = {
"Box", "Axis", "Icosahedron", 
"Point", "Sphere"};

std::string
BoundaryAttributes::PointType_ToString(BoundaryAttributes::PointType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return PointType_strings[index];
}

std::string
BoundaryAttributes::PointType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return PointType_strings[index];
}

bool
BoundaryAttributes::PointType_FromString(const std::string &s, BoundaryAttributes::PointType &val)
{
    val = BoundaryAttributes::Box;
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

// ****************************************************************************
// Method: BoundaryAttributes::BoundaryAttributes
//
// Purpose: 
//   Constructor for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

BoundaryAttributes::BoundaryAttributes() : AttributeSubject("isbbiiaas*idbidibsi"),
    colorTableName("Default"), singleColor(), 
    pointSizeVar("default")
{
    colorType = ColorByMultipleColors;
    filledFlag = true;
    legendFlag = true;
    lineStyle = 0;
    lineWidth = 0;
    boundaryType = Unknown;
    opacity = 1;
    wireframe = false;
    smoothingLevel = 0;
    pointSize = 0.05;
    pointType = Point;
    pointSizeVarEnabled = false;
    pointSizePixels = 2;
}

// ****************************************************************************
// Method: BoundaryAttributes::BoundaryAttributes
//
// Purpose: 
//   Copy constructor for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

BoundaryAttributes::BoundaryAttributes(const BoundaryAttributes &obj) : AttributeSubject("isbbiiaas*idbidibsi")
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
    smoothingLevel = obj.smoothingLevel;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;

    SelectAll();
}

// ****************************************************************************
// Method: BoundaryAttributes::~BoundaryAttributes
//
// Purpose: 
//   Destructor for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

BoundaryAttributes::~BoundaryAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: BoundaryAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

BoundaryAttributes& 
BoundaryAttributes::operator = (const BoundaryAttributes &obj)
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
// Method: BoundaryAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
BoundaryAttributes::operator == (const BoundaryAttributes &obj) const
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
            (smoothingLevel == obj.smoothingLevel) &&
            (pointSize == obj.pointSize) &&
            (pointType == obj.pointType) &&
            (pointSizeVarEnabled == obj.pointSizeVarEnabled) &&
            (pointSizeVar == obj.pointSizeVar) &&
            (pointSizePixels == obj.pointSizePixels));
}

// ****************************************************************************
// Method: BoundaryAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
BoundaryAttributes::operator != (const BoundaryAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: BoundaryAttributes::TypeName
//
// Purpose: 
//   Type name method for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

const std::string
BoundaryAttributes::TypeName() const
{
    return "BoundaryAttributes";
}

// ****************************************************************************
// Method: BoundaryAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
BoundaryAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const BoundaryAttributes *tmp = (const BoundaryAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: BoundaryAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
BoundaryAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new BoundaryAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: BoundaryAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the BoundaryAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
BoundaryAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new BoundaryAttributes(*this);
    else
        retval = new BoundaryAttributes;

    return retval;
}

// ****************************************************************************
// Method: BoundaryAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
BoundaryAttributes::SelectAll()
{
    Select(0, (void *)&colorType);
    Select(1, (void *)&colorTableName);
    Select(2, (void *)&filledFlag);
    Select(3, (void *)&legendFlag);
    Select(4, (void *)&lineStyle);
    Select(5, (void *)&lineWidth);
    Select(6, (void *)&singleColor);
    Select(7, (void *)&multiColor);
    Select(8, (void *)&boundaryNames);
    Select(9, (void *)&boundaryType);
    Select(10, (void *)&opacity);
    Select(11, (void *)&wireframe);
    Select(12, (void *)&smoothingLevel);
    Select(13, (void *)&pointSize);
    Select(14, (void *)&pointType);
    Select(15, (void *)&pointSizeVarEnabled);
    Select(16, (void *)&pointSizeVar);
    Select(17, (void *)&pointSizePixels);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: BoundaryAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
BoundaryAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    BoundaryAttributes defaultObject;
    bool addToParent = false;
    // Create a node for BoundaryAttributes.
    DataNode *node = new DataNode("BoundaryAttributes");

    if(completeSave || !FieldsEqual(0, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorType", ColoringMethod_ToString(colorType)));
    }

    if(completeSave || !FieldsEqual(1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

    if(completeSave || !FieldsEqual(2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("filledFlag", filledFlag));
    }

    if(completeSave || !FieldsEqual(3, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
    }

    if(completeSave || !FieldsEqual(4, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineStyle", lineStyle));
    }

    if(completeSave || !FieldsEqual(5, &defaultObject))
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
    if(completeSave || !FieldsEqual(7, &defaultObject))
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

    if(completeSave || !FieldsEqual(8, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("boundaryNames", boundaryNames));
    }

    if(completeSave || !FieldsEqual(9, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("boundaryType", Boundary_Type_ToString(boundaryType)));
    }

    if(completeSave || !FieldsEqual(10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacity", opacity));
    }

    if(completeSave || !FieldsEqual(11, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wireframe", wireframe));
    }

    if(completeSave || !FieldsEqual(12, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("smoothingLevel", smoothingLevel));
    }

    if(completeSave || !FieldsEqual(13, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSize", pointSize));
    }

    if(completeSave || !FieldsEqual(14, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointType", PointType_ToString(pointType)));
    }

    if(completeSave || !FieldsEqual(15, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizeVarEnabled", pointSizeVarEnabled));
    }

    if(completeSave || !FieldsEqual(16, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizeVar", pointSizeVar));
    }

    if(completeSave || !FieldsEqual(17, &defaultObject))
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
// Method: BoundaryAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
BoundaryAttributes::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("BoundaryAttributes");
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
BoundaryAttributes::SetColorType(BoundaryAttributes::ColoringMethod colorType_)
{
    colorType = colorType_;
    Select(0, (void *)&colorType);
}

void
BoundaryAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(1, (void *)&colorTableName);
}

void
BoundaryAttributes::SetFilledFlag(bool filledFlag_)
{
    filledFlag = filledFlag_;
    Select(2, (void *)&filledFlag);
}

void
BoundaryAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(3, (void *)&legendFlag);
}

void
BoundaryAttributes::SetLineStyle(int lineStyle_)
{
    lineStyle = lineStyle_;
    Select(4, (void *)&lineStyle);
}

void
BoundaryAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(5, (void *)&lineWidth);
}

void
BoundaryAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(6, (void *)&singleColor);
}

void
BoundaryAttributes::SetMultiColor(const ColorAttributeList &multiColor_)
{
    multiColor = multiColor_;
    Select(7, (void *)&multiColor);
}

void
BoundaryAttributes::SetBoundaryNames(const stringVector &boundaryNames_)
{
    boundaryNames = boundaryNames_;
    Select(8, (void *)&boundaryNames);
}

void
BoundaryAttributes::SetBoundaryType(BoundaryAttributes::Boundary_Type boundaryType_)
{
    boundaryType = boundaryType_;
    Select(9, (void *)&boundaryType);
}

void
BoundaryAttributes::SetOpacity(double opacity_)
{
    opacity = opacity_;
    Select(10, (void *)&opacity);
}

void
BoundaryAttributes::SetWireframe(bool wireframe_)
{
    wireframe = wireframe_;
    Select(11, (void *)&wireframe);
}

void
BoundaryAttributes::SetSmoothingLevel(int smoothingLevel_)
{
    smoothingLevel = smoothingLevel_;
    Select(12, (void *)&smoothingLevel);
}

void
BoundaryAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(13, (void *)&pointSize);
}

void
BoundaryAttributes::SetPointType(BoundaryAttributes::PointType pointType_)
{
    pointType = pointType_;
    Select(14, (void *)&pointType);
}

void
BoundaryAttributes::SetPointSizeVarEnabled(bool pointSizeVarEnabled_)
{
    pointSizeVarEnabled = pointSizeVarEnabled_;
    Select(15, (void *)&pointSizeVarEnabled);
}

void
BoundaryAttributes::SetPointSizeVar(const std::string &pointSizeVar_)
{
    pointSizeVar = pointSizeVar_;
    Select(16, (void *)&pointSizeVar);
}

void
BoundaryAttributes::SetPointSizePixels(int pointSizePixels_)
{
    pointSizePixels = pointSizePixels_;
    Select(17, (void *)&pointSizePixels);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

BoundaryAttributes::ColoringMethod
BoundaryAttributes::GetColorType() const
{
    return ColoringMethod(colorType);
}

const std::string &
BoundaryAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
BoundaryAttributes::GetColorTableName()
{
    return colorTableName;
}

bool
BoundaryAttributes::GetFilledFlag() const
{
    return filledFlag;
}

bool
BoundaryAttributes::GetLegendFlag() const
{
    return legendFlag;
}

int
BoundaryAttributes::GetLineStyle() const
{
    return lineStyle;
}

int
BoundaryAttributes::GetLineWidth() const
{
    return lineWidth;
}

const ColorAttribute &
BoundaryAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
BoundaryAttributes::GetSingleColor()
{
    return singleColor;
}

const ColorAttributeList &
BoundaryAttributes::GetMultiColor() const
{
    return multiColor;
}

ColorAttributeList &
BoundaryAttributes::GetMultiColor()
{
    return multiColor;
}

const stringVector &
BoundaryAttributes::GetBoundaryNames() const
{
    return boundaryNames;
}

stringVector &
BoundaryAttributes::GetBoundaryNames()
{
    return boundaryNames;
}

BoundaryAttributes::Boundary_Type
BoundaryAttributes::GetBoundaryType() const
{
    return Boundary_Type(boundaryType);
}

double
BoundaryAttributes::GetOpacity() const
{
    return opacity;
}

bool
BoundaryAttributes::GetWireframe() const
{
    return wireframe;
}

int
BoundaryAttributes::GetSmoothingLevel() const
{
    return smoothingLevel;
}

double
BoundaryAttributes::GetPointSize() const
{
    return pointSize;
}

BoundaryAttributes::PointType
BoundaryAttributes::GetPointType() const
{
    return PointType(pointType);
}

bool
BoundaryAttributes::GetPointSizeVarEnabled() const
{
    return pointSizeVarEnabled;
}

const std::string &
BoundaryAttributes::GetPointSizeVar() const
{
    return pointSizeVar;
}

std::string &
BoundaryAttributes::GetPointSizeVar()
{
    return pointSizeVar;
}

int
BoundaryAttributes::GetPointSizePixels() const
{
    return pointSizePixels;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
BoundaryAttributes::SelectColorTableName()
{
    Select(1, (void *)&colorTableName);
}

void
BoundaryAttributes::SelectSingleColor()
{
    Select(6, (void *)&singleColor);
}

void
BoundaryAttributes::SelectMultiColor()
{
    Select(7, (void *)&multiColor);
}

void
BoundaryAttributes::SelectBoundaryNames()
{
    Select(8, (void *)&boundaryNames);
}

void
BoundaryAttributes::SelectPointSizeVar()
{
    Select(16, (void *)&pointSizeVar);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: BoundaryAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
BoundaryAttributes::GetFieldName(int index) const
{
    switch (index)
    {
        case 0:  return "colorType";
        case 1:  return "Color table";
        case 2:  return "filledFlag";
        case 3:  return "legendFlag";
        case 4:  return "lineStyle";
        case 5:  return "lineWidth";
        case 6:  return "singleColor";
        case 7:  return "multiColor";
        case 8:  return "boundaryNames";
        case 9:  return "boundaryType";
        case 10:  return "opacity";
        case 11:  return "wireframe";
        case 12:  return "Smoothing level";
        case 13:  return "Point size";
        case 14:  return "Point Type";
        case 15:  return "Point size by var enabled";
        case 16:  return "Point size by var";
        case 17:  return "Point size pixels";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: BoundaryAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
BoundaryAttributes::GetFieldType(int index) const
{
    switch (index)
    {
        case 0:  return FieldType_enum;
        case 1:  return FieldType_colortable;
        case 2:  return FieldType_bool;
        case 3:  return FieldType_bool;
        case 4:  return FieldType_linestyle;
        case 5:  return FieldType_linewidth;
        case 6:  return FieldType_color;
        case 7:  return FieldType_att;
        case 8:  return FieldType_stringVector;
        case 9:  return FieldType_enum;
        case 10:  return FieldType_opacity;
        case 11:  return FieldType_bool;
        case 12:  return FieldType_int;
        case 13:  return FieldType_double;
        case 14:  return FieldType_enum;
        case 15:  return FieldType_bool;
        case 16:  return FieldType_variablename;
        case 17:  return FieldType_int;
        default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: BoundaryAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
BoundaryAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
        case 0:  return "enum";
        case 1:  return "colortable";
        case 2:  return "bool";
        case 3:  return "bool";
        case 4:  return "linestyle";
        case 5:  return "linewidth";
        case 6:  return "color";
        case 7:  return "att";
        case 8:  return "stringVector";
        case 9:  return "enum";
        case 10:  return "opacity";
        case 11:  return "bool";
        case 12:  return "int";
        case 13:  return "double";
        case 14:  return "enum";
        case 15:  return "bool";
        case 16:  return "variablename";
        case 17:  return "int";
        default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: BoundaryAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Thu Aug 25 09:07:49 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

bool
BoundaryAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const BoundaryAttributes &obj = *((const BoundaryAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case 0:
        {  // new scope
        retval = (colorType == obj.colorType);
        }
        break;
    case 1:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case 2:
        {  // new scope
        retval = (filledFlag == obj.filledFlag);
        }
        break;
    case 3:
        {  // new scope
        retval = (legendFlag == obj.legendFlag);
        }
        break;
    case 4:
        {  // new scope
        retval = (lineStyle == obj.lineStyle);
        }
        break;
    case 5:
        {  // new scope
        retval = (lineWidth == obj.lineWidth);
        }
        break;
    case 6:
        {  // new scope
        retval = (singleColor == obj.singleColor);
        }
        break;
    case 7:
        {  // new scope
        retval = (multiColor == obj.multiColor);
        }
        break;
    case 8:
        {  // new scope
        retval = (boundaryNames == obj.boundaryNames);
        }
        break;
    case 9:
        {  // new scope
        retval = (boundaryType == obj.boundaryType);
        }
        break;
    case 10:
        {  // new scope
        retval = (opacity == obj.opacity);
        }
        break;
    case 11:
        {  // new scope
        retval = (wireframe == obj.wireframe);
        }
        break;
    case 12:
        {  // new scope
        retval = (smoothingLevel == obj.smoothingLevel);
        }
        break;
    case 13:
        {  // new scope
        retval = (pointSize == obj.pointSize);
        }
        break;
    case 14:
        {  // new scope
        retval = (pointType == obj.pointType);
        }
        break;
    case 15:
        {  // new scope
        retval = (pointSizeVarEnabled == obj.pointSizeVarEnabled);
        }
        break;
    case 16:
        {  // new scope
        retval = (pointSizeVar == obj.pointSizeVar);
        }
        break;
    case 17:
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
//    Kathleen Bonnell, Wed Nov 10 09:37:01 PST 2004
//    Added needsSecondaryVar. 
//
// ****************************************************************************
bool
BoundaryAttributes::ChangesRequireRecalculation(const BoundaryAttributes &obj)
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
            (smoothingLevel != obj.smoothingLevel) ||
            needSecondaryVar);
}

bool
BoundaryAttributes::VarChangeRequiresReset()
{ 
    return true;
}

// ****************************************************************************
// Method: BoundaryAttributes::ProcessOldVersions
//
// Purpose: 
//   This method creates modifies a DataNode representation of the object
//   so it conforms to the newest representation of the object, which can
//   can be read back in.
//
// Programmer: Jeremy Meredith
// Creation:   June 18, 2003
//
// Modifications:
//
// ****************************************************************************
void
BoundaryAttributes::ProcessOldVersions(DataNode *parentNode,
                                       const char *configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("BoundaryAttributes");
    if(searchNode == 0)
        return;

    DataNode *wfNode = searchNode->GetNode("wireframe");
    if (wfNode == 0)
        return;

    if (VersionLessThan(configVersion, "1.1.5"))
        searchNode->RemoveNode("wireframe");
}

