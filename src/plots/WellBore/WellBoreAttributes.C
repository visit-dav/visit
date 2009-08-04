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

#include <WellBoreAttributes.h>
#include <DataNode.h>
#include <ColorControlPoint.h>

//
// Enum conversion methods for WellBoreAttributes::WellRenderingMode
//

static const char *WellRenderingMode_strings[] = {
"Lines", "Cylinders"};

std::string
WellBoreAttributes::WellRenderingMode_ToString(WellBoreAttributes::WellRenderingMode t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return WellRenderingMode_strings[index];
}

std::string
WellBoreAttributes::WellRenderingMode_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return WellRenderingMode_strings[index];
}

bool
WellBoreAttributes::WellRenderingMode_FromString(const std::string &s, WellBoreAttributes::WellRenderingMode &val)
{
    val = WellBoreAttributes::Lines;
    for(int i = 0; i < 2; ++i)
    {
        if(s == WellRenderingMode_strings[i])
        {
            val = (WellRenderingMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for WellBoreAttributes::DetailLevel
//

static const char *DetailLevel_strings[] = {
"Low", "Medium", "High", 
"Super"};

std::string
WellBoreAttributes::DetailLevel_ToString(WellBoreAttributes::DetailLevel t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return DetailLevel_strings[index];
}

std::string
WellBoreAttributes::DetailLevel_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return DetailLevel_strings[index];
}

bool
WellBoreAttributes::DetailLevel_FromString(const std::string &s, WellBoreAttributes::DetailLevel &val)
{
    val = WellBoreAttributes::Low;
    for(int i = 0; i < 4; ++i)
    {
        if(s == DetailLevel_strings[i])
        {
            val = (DetailLevel)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for WellBoreAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"ColorBySingleColor", "ColorByMultipleColors", "ColorByColorTable"
};

std::string
WellBoreAttributes::ColoringMethod_ToString(WellBoreAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ColoringMethod_strings[index];
}

std::string
WellBoreAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
WellBoreAttributes::ColoringMethod_FromString(const std::string &s, WellBoreAttributes::ColoringMethod &val)
{
    val = WellBoreAttributes::ColorBySingleColor;
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
// Enum conversion methods for WellBoreAttributes::WellAnnotation
//

static const char *WellAnnotation_strings[] = {
"None", "StemOnly", "NameOnly", 
"StemAndName"};

std::string
WellBoreAttributes::WellAnnotation_ToString(WellBoreAttributes::WellAnnotation t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return WellAnnotation_strings[index];
}

std::string
WellBoreAttributes::WellAnnotation_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return WellAnnotation_strings[index];
}

bool
WellBoreAttributes::WellAnnotation_FromString(const std::string &s, WellBoreAttributes::WellAnnotation &val)
{
    val = WellBoreAttributes::None;
    for(int i = 0; i < 4; ++i)
    {
        if(s == WellAnnotation_strings[i])
        {
            val = (WellAnnotation)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *WellBoreAttributes::TypeMapFormatString = "au*isaaiifiiiffbii*s*";

// ****************************************************************************
// Method: WellBoreAttributes::WellBoreAttributes
//
// Purpose: 
//   Constructor for the WellBoreAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

WellBoreAttributes::WellBoreAttributes() : 
    AttributeSubject(WellBoreAttributes::TypeMapFormatString),
    colorTableName("Default"), singleColor(255, 0, 0)
{
    colorType = ColorByMultipleColors;
    drawWellsAs = Cylinders;
    wellCylinderQuality = Medium;
    wellRadius = 0.12;
    wellLineWidth = 0;
    wellLineStyle = 0;
    wellAnnotation = StemAndName;
    wellStemHeight = 10;
    wellNameScale = 0.2;
    legendFlag = true;
    nWellBores = 0;
}

// ****************************************************************************
// Method: WellBoreAttributes::WellBoreAttributes
//
// Purpose: 
//   Copy constructor for the WellBoreAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

WellBoreAttributes::WellBoreAttributes(const WellBoreAttributes &obj) : 
    AttributeSubject(WellBoreAttributes::TypeMapFormatString)
{
    defaultPalette = obj.defaultPalette;
    changedColors = obj.changedColors;
    colorType = obj.colorType;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    multiColor = obj.multiColor;
    drawWellsAs = obj.drawWellsAs;
    wellCylinderQuality = obj.wellCylinderQuality;
    wellRadius = obj.wellRadius;
    wellLineWidth = obj.wellLineWidth;
    wellLineStyle = obj.wellLineStyle;
    wellAnnotation = obj.wellAnnotation;
    wellStemHeight = obj.wellStemHeight;
    wellNameScale = obj.wellNameScale;
    legendFlag = obj.legendFlag;
    nWellBores = obj.nWellBores;
    wellBores = obj.wellBores;
    wellNames = obj.wellNames;

    SelectAll();
}

// ****************************************************************************
// Method: WellBoreAttributes::~WellBoreAttributes
//
// Purpose: 
//   Destructor for the WellBoreAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

WellBoreAttributes::~WellBoreAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: WellBoreAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the WellBoreAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

WellBoreAttributes& 
WellBoreAttributes::operator = (const WellBoreAttributes &obj)
{
    if (this == &obj) return *this;
    defaultPalette = obj.defaultPalette;
    changedColors = obj.changedColors;
    colorType = obj.colorType;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    multiColor = obj.multiColor;
    drawWellsAs = obj.drawWellsAs;
    wellCylinderQuality = obj.wellCylinderQuality;
    wellRadius = obj.wellRadius;
    wellLineWidth = obj.wellLineWidth;
    wellLineStyle = obj.wellLineStyle;
    wellAnnotation = obj.wellAnnotation;
    wellStemHeight = obj.wellStemHeight;
    wellNameScale = obj.wellNameScale;
    legendFlag = obj.legendFlag;
    nWellBores = obj.nWellBores;
    wellBores = obj.wellBores;
    wellNames = obj.wellNames;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: WellBoreAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the WellBoreAttributes class.
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
WellBoreAttributes::operator == (const WellBoreAttributes &obj) const
{
    // Create the return value
    return (true /* can ignore defaultPalette */ &&
            true /* can ignore changedColors */ &&
            (colorType == obj.colorType) &&
            (colorTableName == obj.colorTableName) &&
            (singleColor == obj.singleColor) &&
            (multiColor == obj.multiColor) &&
            (drawWellsAs == obj.drawWellsAs) &&
            (wellCylinderQuality == obj.wellCylinderQuality) &&
            (wellRadius == obj.wellRadius) &&
            (wellLineWidth == obj.wellLineWidth) &&
            (wellLineStyle == obj.wellLineStyle) &&
            (wellAnnotation == obj.wellAnnotation) &&
            (wellStemHeight == obj.wellStemHeight) &&
            (wellNameScale == obj.wellNameScale) &&
            (legendFlag == obj.legendFlag) &&
            (nWellBores == obj.nWellBores) &&
            (wellBores == obj.wellBores) &&
            (wellNames == obj.wellNames));
}

// ****************************************************************************
// Method: WellBoreAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the WellBoreAttributes class.
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
WellBoreAttributes::operator != (const WellBoreAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: WellBoreAttributes::TypeName
//
// Purpose: 
//   Type name method for the WellBoreAttributes class.
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
WellBoreAttributes::TypeName() const
{
    return "WellBoreAttributes";
}

// ****************************************************************************
// Method: WellBoreAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the WellBoreAttributes class.
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
WellBoreAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const WellBoreAttributes *tmp = (const WellBoreAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: WellBoreAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the WellBoreAttributes class.
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
WellBoreAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new WellBoreAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: WellBoreAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the WellBoreAttributes class.
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
WellBoreAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new WellBoreAttributes(*this);
    else
        retval = new WellBoreAttributes;

    return retval;
}

// ****************************************************************************
// Method: WellBoreAttributes::SelectAll
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
WellBoreAttributes::SelectAll()
{
    Select(ID_defaultPalette,      (void *)&defaultPalette);
    Select(ID_changedColors,       (void *)&changedColors);
    Select(ID_colorType,           (void *)&colorType);
    Select(ID_colorTableName,      (void *)&colorTableName);
    Select(ID_singleColor,         (void *)&singleColor);
    Select(ID_multiColor,          (void *)&multiColor);
    Select(ID_drawWellsAs,         (void *)&drawWellsAs);
    Select(ID_wellCylinderQuality, (void *)&wellCylinderQuality);
    Select(ID_wellRadius,          (void *)&wellRadius);
    Select(ID_wellLineWidth,       (void *)&wellLineWidth);
    Select(ID_wellLineStyle,       (void *)&wellLineStyle);
    Select(ID_wellAnnotation,      (void *)&wellAnnotation);
    Select(ID_wellStemHeight,      (void *)&wellStemHeight);
    Select(ID_wellNameScale,       (void *)&wellNameScale);
    Select(ID_legendFlag,          (void *)&legendFlag);
    Select(ID_nWellBores,          (void *)&nWellBores);
    Select(ID_wellBores,           (void *)&wellBores);
    Select(ID_wellNames,           (void *)&wellNames);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: WellBoreAttributes::CreateNode
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
WellBoreAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    WellBoreAttributes defaultObject;
    bool addToParent = false;
    // Create a node for WellBoreAttributes.
    DataNode *node = new DataNode("WellBoreAttributes");

    if(completeSave || !FieldsEqual(ID_defaultPalette, &defaultObject))
    {
        DataNode *defaultPaletteNode = new DataNode("defaultPalette");
        if(defaultPalette.CreateNode(defaultPaletteNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(defaultPaletteNode);
        }
        else
            delete defaultPaletteNode;
    }

    if(completeSave || !FieldsEqual(ID_changedColors, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("changedColors", changedColors));
    }

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

    if(completeSave || !FieldsEqual(ID_drawWellsAs, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("drawWellsAs", WellRenderingMode_ToString(drawWellsAs)));
    }

    if(completeSave || !FieldsEqual(ID_wellCylinderQuality, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellCylinderQuality", DetailLevel_ToString(wellCylinderQuality)));
    }

    if(completeSave || !FieldsEqual(ID_wellRadius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellRadius", wellRadius));
    }

    if(completeSave || !FieldsEqual(ID_wellLineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellLineWidth", wellLineWidth));
    }

    if(completeSave || !FieldsEqual(ID_wellLineStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellLineStyle", wellLineStyle));
    }

    if(completeSave || !FieldsEqual(ID_wellAnnotation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellAnnotation", WellAnnotation_ToString(wellAnnotation)));
    }

    if(completeSave || !FieldsEqual(ID_wellStemHeight, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellStemHeight", wellStemHeight));
    }

    if(completeSave || !FieldsEqual(ID_wellNameScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellNameScale", wellNameScale));
    }

    if(completeSave || !FieldsEqual(ID_legendFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
    }

    if(completeSave || !FieldsEqual(ID_nWellBores, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nWellBores", nWellBores));
    }

    if(completeSave || !FieldsEqual(ID_wellBores, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellBores", wellBores));
    }

    if(completeSave || !FieldsEqual(ID_wellNames, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wellNames", wellNames));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: WellBoreAttributes::SetFromNode
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
WellBoreAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("WellBoreAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("defaultPalette")) != 0)
        defaultPalette.SetFromNode(node);
    if((node = searchNode->GetNode("changedColors")) != 0)
        SetChangedColors(node->AsUnsignedCharVector());
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
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("multiColor")) != 0)
        multiColor.SetFromNode(node);
    if((node = searchNode->GetNode("drawWellsAs")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetDrawWellsAs(WellRenderingMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            WellRenderingMode value;
            if(WellRenderingMode_FromString(node->AsString(), value))
                SetDrawWellsAs(value);
        }
    }
    if((node = searchNode->GetNode("wellCylinderQuality")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetWellCylinderQuality(DetailLevel(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DetailLevel value;
            if(DetailLevel_FromString(node->AsString(), value))
                SetWellCylinderQuality(value);
        }
    }
    if((node = searchNode->GetNode("wellRadius")) != 0)
        SetWellRadius(node->AsFloat());
    if((node = searchNode->GetNode("wellLineWidth")) != 0)
        SetWellLineWidth(node->AsInt());
    if((node = searchNode->GetNode("wellLineStyle")) != 0)
        SetWellLineStyle(node->AsInt());
    if((node = searchNode->GetNode("wellAnnotation")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetWellAnnotation(WellAnnotation(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            WellAnnotation value;
            if(WellAnnotation_FromString(node->AsString(), value))
                SetWellAnnotation(value);
        }
    }
    if((node = searchNode->GetNode("wellStemHeight")) != 0)
        SetWellStemHeight(node->AsFloat());
    if((node = searchNode->GetNode("wellNameScale")) != 0)
        SetWellNameScale(node->AsFloat());
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("nWellBores")) != 0)
        SetNWellBores(node->AsInt());
    if((node = searchNode->GetNode("wellBores")) != 0)
        SetWellBores(node->AsIntVector());
    if((node = searchNode->GetNode("wellNames")) != 0)
        SetWellNames(node->AsStringVector());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
WellBoreAttributes::SetDefaultPalette(const ColorControlPointList &defaultPalette_)
{
    defaultPalette = defaultPalette_;
    Select(ID_defaultPalette, (void *)&defaultPalette);
}

void
WellBoreAttributes::SetChangedColors(const unsignedCharVector &changedColors_)
{
    changedColors = changedColors_;
    Select(ID_changedColors, (void *)&changedColors);
}

void
WellBoreAttributes::SetColorType(WellBoreAttributes::ColoringMethod colorType_)
{
    colorType = colorType_;
    Select(ID_colorType, (void *)&colorType);
}

void
WellBoreAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
WellBoreAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
WellBoreAttributes::SetMultiColor(const ColorAttributeList &multiColor_)
{
    multiColor = multiColor_;
    Select(ID_multiColor, (void *)&multiColor);
}

void
WellBoreAttributes::SetDrawWellsAs(WellBoreAttributes::WellRenderingMode drawWellsAs_)
{
    drawWellsAs = drawWellsAs_;
    Select(ID_drawWellsAs, (void *)&drawWellsAs);
}

void
WellBoreAttributes::SetWellCylinderQuality(WellBoreAttributes::DetailLevel wellCylinderQuality_)
{
    wellCylinderQuality = wellCylinderQuality_;
    Select(ID_wellCylinderQuality, (void *)&wellCylinderQuality);
}

void
WellBoreAttributes::SetWellRadius(float wellRadius_)
{
    wellRadius = wellRadius_;
    Select(ID_wellRadius, (void *)&wellRadius);
}

void
WellBoreAttributes::SetWellLineWidth(int wellLineWidth_)
{
    wellLineWidth = wellLineWidth_;
    Select(ID_wellLineWidth, (void *)&wellLineWidth);
}

void
WellBoreAttributes::SetWellLineStyle(int wellLineStyle_)
{
    wellLineStyle = wellLineStyle_;
    Select(ID_wellLineStyle, (void *)&wellLineStyle);
}

void
WellBoreAttributes::SetWellAnnotation(WellBoreAttributes::WellAnnotation wellAnnotation_)
{
    wellAnnotation = wellAnnotation_;
    Select(ID_wellAnnotation, (void *)&wellAnnotation);
}

void
WellBoreAttributes::SetWellStemHeight(float wellStemHeight_)
{
    wellStemHeight = wellStemHeight_;
    Select(ID_wellStemHeight, (void *)&wellStemHeight);
}

void
WellBoreAttributes::SetWellNameScale(float wellNameScale_)
{
    wellNameScale = wellNameScale_;
    Select(ID_wellNameScale, (void *)&wellNameScale);
}

void
WellBoreAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
WellBoreAttributes::SetNWellBores(int nWellBores_)
{
    nWellBores = nWellBores_;
    Select(ID_nWellBores, (void *)&nWellBores);
    // Grow the color vector.
    EnlargeMultiColor(nWellBores);

}

void
WellBoreAttributes::SetWellBores(const intVector &wellBores_)
{
    wellBores = wellBores_;
    Select(ID_wellBores, (void *)&wellBores);
}

void
WellBoreAttributes::SetWellNames(const stringVector &wellNames_)
{
    wellNames = wellNames_;
    Select(ID_wellNames, (void *)&wellNames);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

const ColorControlPointList &
WellBoreAttributes::GetDefaultPalette() const
{
    return defaultPalette;
}

ColorControlPointList &
WellBoreAttributes::GetDefaultPalette()
{
    return defaultPalette;
}

const unsignedCharVector &
WellBoreAttributes::GetChangedColors() const
{
    return changedColors;
}

unsignedCharVector &
WellBoreAttributes::GetChangedColors()
{
    return changedColors;
}

WellBoreAttributes::ColoringMethod
WellBoreAttributes::GetColorType() const
{
    return ColoringMethod(colorType);
}

const std::string &
WellBoreAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
WellBoreAttributes::GetColorTableName()
{
    return colorTableName;
}

const ColorAttribute &
WellBoreAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
WellBoreAttributes::GetSingleColor()
{
    return singleColor;
}

const ColorAttributeList &
WellBoreAttributes::GetMultiColor() const
{
    return multiColor;
}

ColorAttributeList &
WellBoreAttributes::GetMultiColor()
{
    return multiColor;
}

WellBoreAttributes::WellRenderingMode
WellBoreAttributes::GetDrawWellsAs() const
{
    return WellRenderingMode(drawWellsAs);
}

WellBoreAttributes::DetailLevel
WellBoreAttributes::GetWellCylinderQuality() const
{
    return DetailLevel(wellCylinderQuality);
}

float
WellBoreAttributes::GetWellRadius() const
{
    return wellRadius;
}

int
WellBoreAttributes::GetWellLineWidth() const
{
    return wellLineWidth;
}

int
WellBoreAttributes::GetWellLineStyle() const
{
    return wellLineStyle;
}

WellBoreAttributes::WellAnnotation
WellBoreAttributes::GetWellAnnotation() const
{
    return WellAnnotation(wellAnnotation);
}

float
WellBoreAttributes::GetWellStemHeight() const
{
    return wellStemHeight;
}

float
WellBoreAttributes::GetWellNameScale() const
{
    return wellNameScale;
}

bool
WellBoreAttributes::GetLegendFlag() const
{
    return legendFlag;
}

int
WellBoreAttributes::GetNWellBores() const
{
    return nWellBores;
}

const intVector &
WellBoreAttributes::GetWellBores() const
{
    return wellBores;
}

intVector &
WellBoreAttributes::GetWellBores()
{
    return wellBores;
}

const stringVector &
WellBoreAttributes::GetWellNames() const
{
    return wellNames;
}

stringVector &
WellBoreAttributes::GetWellNames()
{
    return wellNames;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
WellBoreAttributes::SelectDefaultPalette()
{
    Select(ID_defaultPalette, (void *)&defaultPalette);
}

void
WellBoreAttributes::SelectChangedColors()
{
    Select(ID_changedColors, (void *)&changedColors);
}

void
WellBoreAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
WellBoreAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

void
WellBoreAttributes::SelectMultiColor()
{
    Select(ID_multiColor, (void *)&multiColor);
}

void
WellBoreAttributes::SelectWellBores()
{
    Select(ID_wellBores, (void *)&wellBores);
}

void
WellBoreAttributes::SelectWellNames()
{
    Select(ID_wellNames, (void *)&wellNames);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: WellBoreAttributes::GetFieldName
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
WellBoreAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_defaultPalette:      return "defaultPalette";
    case ID_changedColors:       return "changedColors";
    case ID_colorType:           return "colorType";
    case ID_colorTableName:      return "colorTableName";
    case ID_singleColor:         return "singleColor";
    case ID_multiColor:          return "multiColor";
    case ID_drawWellsAs:         return "drawWellsAs";
    case ID_wellCylinderQuality: return "wellCylinderQuality";
    case ID_wellRadius:          return "wellRadius";
    case ID_wellLineWidth:       return "wellLineWidth";
    case ID_wellLineStyle:       return "wellLineStyle";
    case ID_wellAnnotation:      return "wellAnnotation";
    case ID_wellStemHeight:      return "wellStemHeight";
    case ID_wellNameScale:       return "wellNameScale";
    case ID_legendFlag:          return "legendFlag";
    case ID_nWellBores:          return "nWellBores";
    case ID_wellBores:           return "wellBores";
    case ID_wellNames:           return "wellNames";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: WellBoreAttributes::GetFieldType
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
WellBoreAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_defaultPalette:      return FieldType_att;
    case ID_changedColors:       return FieldType_ucharVector;
    case ID_colorType:           return FieldType_enum;
    case ID_colorTableName:      return FieldType_colortable;
    case ID_singleColor:         return FieldType_color;
    case ID_multiColor:          return FieldType_att;
    case ID_drawWellsAs:         return FieldType_enum;
    case ID_wellCylinderQuality: return FieldType_enum;
    case ID_wellRadius:          return FieldType_float;
    case ID_wellLineWidth:       return FieldType_linewidth;
    case ID_wellLineStyle:       return FieldType_linestyle;
    case ID_wellAnnotation:      return FieldType_enum;
    case ID_wellStemHeight:      return FieldType_float;
    case ID_wellNameScale:       return FieldType_float;
    case ID_legendFlag:          return FieldType_bool;
    case ID_nWellBores:          return FieldType_int;
    case ID_wellBores:           return FieldType_intVector;
    case ID_wellNames:           return FieldType_stringVector;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: WellBoreAttributes::GetFieldTypeName
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
WellBoreAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_defaultPalette:      return "att";
    case ID_changedColors:       return "ucharVector";
    case ID_colorType:           return "enum";
    case ID_colorTableName:      return "colortable";
    case ID_singleColor:         return "color";
    case ID_multiColor:          return "att";
    case ID_drawWellsAs:         return "enum";
    case ID_wellCylinderQuality: return "enum";
    case ID_wellRadius:          return "float";
    case ID_wellLineWidth:       return "linewidth";
    case ID_wellLineStyle:       return "linestyle";
    case ID_wellAnnotation:      return "enum";
    case ID_wellStemHeight:      return "float";
    case ID_wellNameScale:       return "float";
    case ID_legendFlag:          return "bool";
    case ID_nWellBores:          return "int";
    case ID_wellBores:           return "intVector";
    case ID_wellNames:           return "stringVector";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: WellBoreAttributes::FieldsEqual
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
WellBoreAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const WellBoreAttributes &obj = *((const WellBoreAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_defaultPalette:
        {  // new scope
        retval = (defaultPalette == obj.defaultPalette);
        }
        break;
    case ID_changedColors:
        {  // new scope
        retval = (changedColors == obj.changedColors);
        }
        break;
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
    case ID_drawWellsAs:
        {  // new scope
        retval = (drawWellsAs == obj.drawWellsAs);
        }
        break;
    case ID_wellCylinderQuality:
        {  // new scope
        retval = (wellCylinderQuality == obj.wellCylinderQuality);
        }
        break;
    case ID_wellRadius:
        {  // new scope
        retval = (wellRadius == obj.wellRadius);
        }
        break;
    case ID_wellLineWidth:
        {  // new scope
        retval = (wellLineWidth == obj.wellLineWidth);
        }
        break;
    case ID_wellLineStyle:
        {  // new scope
        retval = (wellLineStyle == obj.wellLineStyle);
        }
        break;
    case ID_wellAnnotation:
        {  // new scope
        retval = (wellAnnotation == obj.wellAnnotation);
        }
        break;
    case ID_wellStemHeight:
        {  // new scope
        retval = (wellStemHeight == obj.wellStemHeight);
        }
        break;
    case ID_wellNameScale:
        {  // new scope
        retval = (wellNameScale == obj.wellNameScale);
        }
        break;
    case ID_legendFlag:
        {  // new scope
        retval = (legendFlag == obj.legendFlag);
        }
        break;
    case ID_nWellBores:
        {  // new scope
        retval = (nWellBores == obj.nWellBores);
        }
        break;
    case ID_wellBores:
        {  // new scope
        retval = (wellBores == obj.wellBores);
        }
        break;
    case ID_wellNames:
        {  // new scope
        retval = (wellNames == obj.wellNames);
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
// Method: ContourAttributes::EnlargeMultiColor
//
// Purpose:
//   Enlarges the multiColor vector to the new size.
//
// Arguments:
//   newSize    The new size of the color vector.
//
// Programmer: Eric Brugger
// Creation:   October 1, 2008
//
// Modifications:
//
// ****************************************************************************

void
WellBoreAttributes::EnlargeMultiColor(int newSize)
{
    // Add any colors that are needed to the end of the vector to ensure
    // we have the right number of elements in the vector.
    if(newSize > 0)
    {
        unsigned char *rgb = new unsigned char[newSize * 4];

        // If it's a discrete color table, just use the colors of
        // the control points. Otherwise, sample the color table.
        if(defaultPalette.GetDiscreteFlag())
        {
            int nColors = defaultPalette.GetNumControlPoints();
            for(int i = 0, index = 0; i < newSize; ++i, index += 4)
            {
                int j = i % nColors;
                const ColorControlPoint &ccp = defaultPalette.operator[](j);
                const unsigned char *c = ccp.GetColors();
                rgb[index] = c[0];
                rgb[index+1] = c[1];
                rgb[index+2] = c[2];
                rgb[index+3] = c[3];
            }
        }
        else
        {
            defaultPalette.GetColors(rgb, newSize);
        }

        bool modified = false;
        for(int i = 0; i < newSize; ++i)
        {
            int j = i * 4;
            if(i < multiColor.GetNumColors())
            {
                if(!ColorIsChanged(i))
                {
                    ColorAttribute &ca = multiColor.GetColors(i);
                    ca.SetRgba(int(rgb[j]), int(rgb[j+1]), int(rgb[j+2]),int(rgb[j+3]));
                }
            }
            else
            {
                multiColor.AddColors(ColorAttribute(rgb[j],
                                                    rgb[j+1],
                                                    rgb[j+2],
                                                    rgb[j+3]));
            }
            modified = true;
        }
        delete [] rgb;

        // If the multiColor vector was modified, select it.
        if(modified)
            SelectMultiColor();
    }
}

// ****************************************************************************
// Method: WellBoreAttributes::ColorIsChanged
//
// Purpose:
//   Returns whether or not a color has been modified.
//
// Arguments:
//   index      A color index.
//
// Programmer: Eric Brugger
// Creation:   October 1, 2008
//
// Modifications:
//
// ***************************************************************************
bool
WellBoreAttributes::ColorIsChanged(int index) const
{
    size_t i = 0;
    for(; i < changedColors.size(); ++i)
    {
        if(changedColors[i] == ((unsigned char)index))
            break;
    }

    return (i < changedColors.size());
}

// ****************************************************************************
// Method: WellBoreAttributes::MarkColorAsChanged
//
// Purpose:
//   Marks a color as having been modified.
//
// Arguments:
//   index      A color index.
//
// Programmer: Eric Brugger
// Creation:   October 1, 2008
//
// Modifications:
//
// ****************************************************************************
void
WellBoreAttributes::MarkColorAsChanged(int index)
{
    if(!ColorIsChanged(index))
    {
        changedColors.push_back(((unsigned char)index));
        SelectChangedColors();
    }
}

bool
WellBoreAttributes::ChangesRequireRecalculation(const WellBoreAttributes &obj)
{
    if (drawWellsAs != obj.drawWellsAs) return true;
    if (drawWellsAs == Cylinders)
    {
        if (wellCylinderQuality != obj.wellCylinderQuality) return true;
        if (wellRadius != obj.wellRadius) return true;
    }
    if (nWellBores != obj.nWellBores) return true;
    if (wellBores != obj.wellBores) return true;
    if (wellNames != obj.wellNames) return true;
    if (wellAnnotation != obj.wellAnnotation) return true;
    if (wellStemHeight != obj.wellStemHeight) return true;

    return false;
}

