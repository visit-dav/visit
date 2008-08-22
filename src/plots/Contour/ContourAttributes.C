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

#include <ContourAttributes.h>
#include <DataNode.h>
#include <ColorControlPoint.h>

const int ContourAttributes::MAX_CONTOURS = 100;

//
// Enum conversion methods for ContourAttributes::Select_by
//

static const char *Select_by_strings[] = {
"Level", "Value", "Percent"
};

std::string
ContourAttributes::Select_by_ToString(ContourAttributes::Select_by t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Select_by_strings[index];
}

std::string
ContourAttributes::Select_by_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Select_by_strings[index];
}

bool
ContourAttributes::Select_by_FromString(const std::string &s, ContourAttributes::Select_by &val)
{
    val = ContourAttributes::Level;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Select_by_strings[i])
        {
            val = (Select_by)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ContourAttributes::Scaling
//

static const char *Scaling_strings[] = {
"Linear", "Log"};

std::string
ContourAttributes::Scaling_ToString(ContourAttributes::Scaling t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return Scaling_strings[index];
}

std::string
ContourAttributes::Scaling_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return Scaling_strings[index];
}

bool
ContourAttributes::Scaling_FromString(const std::string &s, ContourAttributes::Scaling &val)
{
    val = ContourAttributes::Linear;
    for(int i = 0; i < 2; ++i)
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
// Enum conversion methods for ContourAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"ColorBySingleColor", "ColorByMultipleColors", "ColorByColorTable"
};

std::string
ContourAttributes::ColoringMethod_ToString(ContourAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ColoringMethod_strings[index];
}

std::string
ContourAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
ContourAttributes::ColoringMethod_FromString(const std::string &s, ContourAttributes::ColoringMethod &val)
{
    val = ContourAttributes::ColorBySingleColor;
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

// Type map format string
const char *ContourAttributes::TypeMapFormatString = "au*isbiiaaid*d*ibbddib";

// ****************************************************************************
// Method: ContourAttributes::ContourAttributes
//
// Purpose: 
//   Constructor for the ContourAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ContourAttributes::ContourAttributes() : 
    AttributeSubject(ContourAttributes::TypeMapFormatString),
    colorTableName("Default"), singleColor(255, 0, 0)
{
    colorType = ColorByMultipleColors;
    legendFlag = true;
    lineStyle = 0;
    lineWidth = 0;
    contourNLevels = 10;
    contourMethod = Level;
    minFlag = false;
    maxFlag = false;
    min = 0;
    max = 1;
    scaling = Linear;
    wireframe = false;
}

// ****************************************************************************
// Method: ContourAttributes::ContourAttributes
//
// Purpose: 
//   Copy constructor for the ContourAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ContourAttributes::ContourAttributes(const ContourAttributes &obj) : 
    AttributeSubject(ContourAttributes::TypeMapFormatString)
{
    defaultPalette = obj.defaultPalette;
    changedColors = obj.changedColors;
    colorType = obj.colorType;
    colorTableName = obj.colorTableName;
    legendFlag = obj.legendFlag;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    singleColor = obj.singleColor;
    multiColor = obj.multiColor;
    contourNLevels = obj.contourNLevels;
    contourValue = obj.contourValue;
    contourPercent = obj.contourPercent;
    contourMethod = obj.contourMethod;
    minFlag = obj.minFlag;
    maxFlag = obj.maxFlag;
    min = obj.min;
    max = obj.max;
    scaling = obj.scaling;
    wireframe = obj.wireframe;

    SelectAll();
}

// ****************************************************************************
// Method: ContourAttributes::~ContourAttributes
//
// Purpose: 
//   Destructor for the ContourAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ContourAttributes::~ContourAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: ContourAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the ContourAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ContourAttributes& 
ContourAttributes::operator = (const ContourAttributes &obj)
{
    if (this == &obj) return *this;
    defaultPalette = obj.defaultPalette;
    changedColors = obj.changedColors;
    colorType = obj.colorType;
    colorTableName = obj.colorTableName;
    legendFlag = obj.legendFlag;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    singleColor = obj.singleColor;
    multiColor = obj.multiColor;
    contourNLevels = obj.contourNLevels;
    contourValue = obj.contourValue;
    contourPercent = obj.contourPercent;
    contourMethod = obj.contourMethod;
    minFlag = obj.minFlag;
    maxFlag = obj.maxFlag;
    min = obj.min;
    max = obj.max;
    scaling = obj.scaling;
    wireframe = obj.wireframe;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: ContourAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the ContourAttributes class.
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
ContourAttributes::operator == (const ContourAttributes &obj) const
{
    // Create the return value
    return (true /* can ignore defaultPalette */ &&
            true /* can ignore changedColors */ &&
            (colorType == obj.colorType) &&
            (colorTableName == obj.colorTableName) &&
            (legendFlag == obj.legendFlag) &&
            (lineStyle == obj.lineStyle) &&
            (lineWidth == obj.lineWidth) &&
            (singleColor == obj.singleColor) &&
            (multiColor == obj.multiColor) &&
            (contourNLevels == obj.contourNLevels) &&
            (contourValue == obj.contourValue) &&
            (contourPercent == obj.contourPercent) &&
            (contourMethod == obj.contourMethod) &&
            (minFlag == obj.minFlag) &&
            (maxFlag == obj.maxFlag) &&
            (min == obj.min) &&
            (max == obj.max) &&
            (scaling == obj.scaling) &&
            (wireframe == obj.wireframe));
}

// ****************************************************************************
// Method: ContourAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the ContourAttributes class.
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
ContourAttributes::operator != (const ContourAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: ContourAttributes::TypeName
//
// Purpose: 
//   Type name method for the ContourAttributes class.
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
ContourAttributes::TypeName() const
{
    return "ContourAttributes";
}

// ****************************************************************************
// Method: ContourAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the ContourAttributes class.
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
ContourAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const ContourAttributes *tmp = (const ContourAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: ContourAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the ContourAttributes class.
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
ContourAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new ContourAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: ContourAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the ContourAttributes class.
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
ContourAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new ContourAttributes(*this);
    else
        retval = new ContourAttributes;

    return retval;
}

// ****************************************************************************
// Method: ContourAttributes::SelectAll
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
ContourAttributes::SelectAll()
{
    Select(ID_defaultPalette, (void *)&defaultPalette);
    Select(ID_changedColors,  (void *)&changedColors);
    Select(ID_colorType,      (void *)&colorType);
    Select(ID_colorTableName, (void *)&colorTableName);
    Select(ID_legendFlag,     (void *)&legendFlag);
    Select(ID_lineStyle,      (void *)&lineStyle);
    Select(ID_lineWidth,      (void *)&lineWidth);
    Select(ID_singleColor,    (void *)&singleColor);
    Select(ID_multiColor,     (void *)&multiColor);
    Select(ID_contourNLevels, (void *)&contourNLevels);
    Select(ID_contourValue,   (void *)&contourValue);
    Select(ID_contourPercent, (void *)&contourPercent);
    Select(ID_contourMethod,  (void *)&contourMethod);
    Select(ID_minFlag,        (void *)&minFlag);
    Select(ID_maxFlag,        (void *)&maxFlag);
    Select(ID_min,            (void *)&min);
    Select(ID_max,            (void *)&max);
    Select(ID_scaling,        (void *)&scaling);
    Select(ID_wireframe,      (void *)&wireframe);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ContourAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Dec 3 08:50:34 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Oct 9 13:32:13 PST 2003
//   I made the single color always be saved out.
//
//   Brad Whitlock, Tue Oct 21 16:01:40 PST 2003
//   I made the default palette get saved out with the colors from the
//   multicolor in it.
//
//   Brad Whitlock, Thu Dec 18 11:52:04 PDT 2003
//   Added the completeSave flag.
//
// ****************************************************************************
bool
ContourAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    ContourAttributes defaultObject;
    bool addToParent = false;
    // Create a node for ContourAttributes.
    DataNode *node = new DataNode("ContourAttributes");

    //
    // Create a copy of the defaultPalette object and store the colors
    // from the multiColor into it. Once we do that, save the palette to
    // the node.
    //
    bool fixupPositions = false;
    DataNode *defaultPaletteNode = new DataNode("defaultPalette");
    ColorControlPointList savePalette(defaultPalette);
    int initialNPoints = savePalette.GetNumControlPoints();
    for(int i = 0; i < multiColor.GetNumColors(); ++i)
    {
        const ColorAttribute &cpt = multiColor.GetColors(i);

        if(i < initialNPoints)
        {
            savePalette[i].SetColors(cpt.GetColor());
        }
        else
        {
            ColorControlPoint ccpt;
            ccpt.SetColors(cpt.GetColor());
            savePalette.AddControlPoints(ccpt);
            fixupPositions = true;
        }
    }
    if(fixupPositions)
    {
        int npts = savePalette.GetNumControlPoints();
        for(int i = 0; i < npts; ++i)
            savePalette[i].SetPosition(float(i) / float(npts-1));
    }
    if(savePalette.CompactCreateNode(defaultPaletteNode, completeSave, forceAdd))
    {
        addToParent = true;
        node->AddNode(defaultPaletteNode);
    }
    else
        delete defaultPaletteNode;

    if(completeSave || completeSave || !FieldsEqual(2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorType", ColoringMethod_ToString(colorType)));
    }

    if(completeSave || completeSave || !FieldsEqual(3, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

    if(completeSave || completeSave || !FieldsEqual(4, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendFlag", legendFlag));
    }

    if(completeSave || !FieldsEqual(5, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineStyle", lineStyle));
    }

    if(completeSave || !FieldsEqual(6, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineWidth", lineWidth));
    }

        DataNode *singleColorNode = new DataNode("singleColor");
        if(singleColor.CreateNode(singleColorNode, true, true))
        {
            addToParent = true;
            node->AddNode(singleColorNode);
        }
        else
            delete singleColorNode;

    if(completeSave || !FieldsEqual(9, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("contourNLevels", contourNLevels));
    }

    if(completeSave || !FieldsEqual(10, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("contourValue", contourValue));
    }

    if(completeSave || !FieldsEqual(11, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("contourPercent", contourPercent));
    }

    if(completeSave || !FieldsEqual(12, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("contourMethod", Select_by_ToString(contourMethod)));
    }

    if(completeSave || !FieldsEqual(13, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minFlag", minFlag));
    }

    if(completeSave || !FieldsEqual(14, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxFlag", maxFlag));
    }

    if(completeSave || !FieldsEqual(15, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("min", min));
    }

    if(completeSave || !FieldsEqual(16, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("max", max));
    }

    if(completeSave || !FieldsEqual(17, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaling", Scaling_ToString(scaling)));
    }

    if(completeSave || !FieldsEqual(18, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wireframe", wireframe));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}
// ****************************************************************************
// Method: ContourAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Dec 3 08:50:34 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ContourAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ContourAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;

    // Set the default palette from the values in the DataNode.
    if((node = searchNode->GetNode("defaultPalette")) != 0)
        defaultPalette.SetFromNode(node);

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
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lineStyle")) != 0)
        SetLineStyle(node->AsInt());
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("contourNLevels")) != 0)
        SetContourNLevels(node->AsInt());
    if((node = searchNode->GetNode("contourValue")) != 0)
        SetContourValue(node->AsDoubleVector());
    if((node = searchNode->GetNode("contourPercent")) != 0)
        SetContourPercent(node->AsDoubleVector());
    if((node = searchNode->GetNode("contourMethod")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetContourMethod(Select_by(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Select_by value;
            if(Select_by_FromString(node->AsString(), value))
                SetContourMethod(value);
        }
    }
    if((node = searchNode->GetNode("minFlag")) != 0)
        SetMinFlag(node->AsBool());
    if((node = searchNode->GetNode("maxFlag")) != 0)
        SetMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("min")) != 0)
        SetMin(node->AsDouble());
    if((node = searchNode->GetNode("max")) != 0)
        SetMax(node->AsDouble());
    if((node = searchNode->GetNode("scaling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetScaling(Scaling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Scaling value;
            if(Scaling_FromString(node->AsString(), value))
                SetScaling(value);
        }
    }
    if((node = searchNode->GetNode("wireframe")) != 0)
        SetWireframe(node->AsBool());
}
///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ContourAttributes::SetDefaultPalette(const ColorControlPointList &defaultPalette_)
{
    defaultPalette = defaultPalette_;
    Select(ID_defaultPalette, (void *)&defaultPalette);
    // Grow the color vector if necessary.
    if(contourMethod == Level)
        EnlargeMultiColor(contourNLevels);
    else if(contourMethod == Value)
        EnlargeMultiColor(contourValue.size());
    else
        EnlargeMultiColor(contourPercent.size());

}

void
ContourAttributes::SetChangedColors(const unsignedCharVector &changedColors_)
{
    changedColors = changedColors_;
    Select(ID_changedColors, (void *)&changedColors);
}

void
ContourAttributes::SetColorType(ContourAttributes::ColoringMethod colorType_)
{
    colorType = colorType_;
    Select(ID_colorType, (void *)&colorType);
}

void
ContourAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
ContourAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
ContourAttributes::SetLineStyle(int lineStyle_)
{
    lineStyle = lineStyle_;
    Select(ID_lineStyle, (void *)&lineStyle);
}

void
ContourAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
ContourAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
ContourAttributes::SetMultiColor(const ColorAttributeList &multiColor_)
{
    multiColor = multiColor_;
    Select(ID_multiColor, (void *)&multiColor);
}

void
ContourAttributes::SetContourNLevels(int contourNLevels_)
{
    if(contourNLevels < 1)
        contourNLevels = 1;
    if(contourNLevels > MAX_CONTOURS)
        contourNLevels = MAX_CONTOURS;

    contourNLevels = contourNLevels_;
    Select(ID_contourNLevels, (void *)&contourNLevels);
    // Grow the color vector.
    EnlargeMultiColor(contourNLevels);

}

void
ContourAttributes::SetContourValue(const doubleVector &contourValue_)
{
    contourValue = contourValue_;
    Select(ID_contourValue, (void *)&contourValue);
    // Grow the color vector.
    EnlargeMultiColor(contourValue.size());

}

void
ContourAttributes::SetContourPercent(const doubleVector &contourPercent_)
{
    contourPercent = contourPercent_;
    Select(ID_contourPercent, (void *)&contourPercent);
    // Grow the color vector.
    EnlargeMultiColor(contourPercent.size());

}

void
ContourAttributes::SetContourMethod(ContourAttributes::Select_by contourMethod_)
{
    contourMethod = contourMethod_;
    Select(ID_contourMethod, (void *)&contourMethod);
}

void
ContourAttributes::SetMinFlag(bool minFlag_)
{
    minFlag = minFlag_;
    Select(ID_minFlag, (void *)&minFlag);
}

void
ContourAttributes::SetMaxFlag(bool maxFlag_)
{
    maxFlag = maxFlag_;
    Select(ID_maxFlag, (void *)&maxFlag);
}

void
ContourAttributes::SetMin(double min_)
{
    min = min_;
    Select(ID_min, (void *)&min);
}

void
ContourAttributes::SetMax(double max_)
{
    max = max_;
    Select(ID_max, (void *)&max);
}

void
ContourAttributes::SetScaling(ContourAttributes::Scaling scaling_)
{
    scaling = scaling_;
    Select(ID_scaling, (void *)&scaling);
}

void
ContourAttributes::SetWireframe(bool wireframe_)
{
    wireframe = wireframe_;
    Select(ID_wireframe, (void *)&wireframe);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

const ColorControlPointList &
ContourAttributes::GetDefaultPalette() const
{
    return defaultPalette;
}

ColorControlPointList &
ContourAttributes::GetDefaultPalette()
{
    return defaultPalette;
}

const unsignedCharVector &
ContourAttributes::GetChangedColors() const
{
    return changedColors;
}

unsignedCharVector &
ContourAttributes::GetChangedColors()
{
    return changedColors;
}

ContourAttributes::ColoringMethod
ContourAttributes::GetColorType() const
{
    return ColoringMethod(colorType);
}

const std::string &
ContourAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
ContourAttributes::GetColorTableName()
{
    return colorTableName;
}

bool
ContourAttributes::GetLegendFlag() const
{
    return legendFlag;
}

int
ContourAttributes::GetLineStyle() const
{
    return lineStyle;
}

int
ContourAttributes::GetLineWidth() const
{
    return lineWidth;
}

const ColorAttribute &
ContourAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
ContourAttributes::GetSingleColor()
{
    return singleColor;
}

const ColorAttributeList &
ContourAttributes::GetMultiColor() const
{
    return multiColor;
}

ColorAttributeList &
ContourAttributes::GetMultiColor()
{
    return multiColor;
}

int
ContourAttributes::GetContourNLevels() const
{
    return contourNLevels;
}

const doubleVector &
ContourAttributes::GetContourValue() const
{
    return contourValue;
}

doubleVector &
ContourAttributes::GetContourValue()
{
    return contourValue;
}

const doubleVector &
ContourAttributes::GetContourPercent() const
{
    return contourPercent;
}

doubleVector &
ContourAttributes::GetContourPercent()
{
    return contourPercent;
}

ContourAttributes::Select_by
ContourAttributes::GetContourMethod() const
{
    return Select_by(contourMethod);
}

bool
ContourAttributes::GetMinFlag() const
{
    return minFlag;
}

bool
ContourAttributes::GetMaxFlag() const
{
    return maxFlag;
}

double
ContourAttributes::GetMin() const
{
    return min;
}

double
ContourAttributes::GetMax() const
{
    return max;
}

ContourAttributes::Scaling
ContourAttributes::GetScaling() const
{
    return Scaling(scaling);
}

bool
ContourAttributes::GetWireframe() const
{
    return wireframe;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ContourAttributes::SelectDefaultPalette()
{
    Select(ID_defaultPalette, (void *)&defaultPalette);
}

void
ContourAttributes::SelectChangedColors()
{
    Select(ID_changedColors, (void *)&changedColors);
}

void
ContourAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
ContourAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

void
ContourAttributes::SelectMultiColor()
{
    Select(ID_multiColor, (void *)&multiColor);
}

void
ContourAttributes::SelectContourValue()
{
    Select(ID_contourValue, (void *)&contourValue);
}

void
ContourAttributes::SelectContourPercent()
{
    Select(ID_contourPercent, (void *)&contourPercent);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ContourAttributes::GetFieldName
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
ContourAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_defaultPalette: return "defaultPalette";
    case ID_changedColors:  return "changedColors";
    case ID_colorType:      return "colorType";
    case ID_colorTableName: return "colorTableName";
    case ID_legendFlag:     return "legendFlag";
    case ID_lineStyle:      return "lineStyle";
    case ID_lineWidth:      return "lineWidth";
    case ID_singleColor:    return "singleColor";
    case ID_multiColor:     return "multiColor";
    case ID_contourNLevels: return "contourNLevels";
    case ID_contourValue:   return "contourValue";
    case ID_contourPercent: return "contourPercent";
    case ID_contourMethod:  return "contourMethod";
    case ID_minFlag:        return "minFlag";
    case ID_maxFlag:        return "maxFlag";
    case ID_min:            return "min";
    case ID_max:            return "max";
    case ID_scaling:        return "scaling";
    case ID_wireframe:      return "wireframe";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ContourAttributes::GetFieldType
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
ContourAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_defaultPalette: return FieldType_att;
    case ID_changedColors:  return FieldType_ucharVector;
    case ID_colorType:      return FieldType_enum;
    case ID_colorTableName: return FieldType_colortable;
    case ID_legendFlag:     return FieldType_bool;
    case ID_lineStyle:      return FieldType_linestyle;
    case ID_lineWidth:      return FieldType_linewidth;
    case ID_singleColor:    return FieldType_color;
    case ID_multiColor:     return FieldType_att;
    case ID_contourNLevels: return FieldType_int;
    case ID_contourValue:   return FieldType_doubleVector;
    case ID_contourPercent: return FieldType_doubleVector;
    case ID_contourMethod:  return FieldType_enum;
    case ID_minFlag:        return FieldType_bool;
    case ID_maxFlag:        return FieldType_bool;
    case ID_min:            return FieldType_double;
    case ID_max:            return FieldType_double;
    case ID_scaling:        return FieldType_enum;
    case ID_wireframe:      return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: ContourAttributes::GetFieldTypeName
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
ContourAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_defaultPalette: return "att";
    case ID_changedColors:  return "ucharVector";
    case ID_colorType:      return "enum";
    case ID_colorTableName: return "colortable";
    case ID_legendFlag:     return "bool";
    case ID_lineStyle:      return "linestyle";
    case ID_lineWidth:      return "linewidth";
    case ID_singleColor:    return "color";
    case ID_multiColor:     return "att";
    case ID_contourNLevels: return "int";
    case ID_contourValue:   return "doubleVector";
    case ID_contourPercent: return "doubleVector";
    case ID_contourMethod:  return "enum";
    case ID_minFlag:        return "bool";
    case ID_maxFlag:        return "bool";
    case ID_min:            return "double";
    case ID_max:            return "double";
    case ID_scaling:        return "enum";
    case ID_wireframe:      return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ContourAttributes::FieldsEqual
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
ContourAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ContourAttributes &obj = *((const ContourAttributes*)rhs);
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
    case ID_contourNLevels:
        {  // new scope
        retval = (contourNLevels == obj.contourNLevels);
        }
        break;
    case ID_contourValue:
        {  // new scope
        retval = (contourValue == obj.contourValue);
        }
        break;
    case ID_contourPercent:
        {  // new scope
        retval = (contourPercent == obj.contourPercent);
        }
        break;
    case ID_contourMethod:
        {  // new scope
        retval = (contourMethod == obj.contourMethod);
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
    case ID_scaling:
        {  // new scope
        retval = (scaling == obj.scaling);
        }
        break;
    case ID_wireframe:
        {  // new scope
        retval = (wireframe == obj.wireframe);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

bool
ContourAttributes::ChangesRequireRecalculation(const ContourAttributes &obj)
{
    return ((contourNLevels != obj.contourNLevels) ||
            (contourValue != obj.contourValue) ||
            (contourPercent != obj.contourPercent) ||
            (contourMethod != obj.contourMethod) ||
            (minFlag != obj.minFlag) ||
            (maxFlag != obj.maxFlag) ||
            (min != obj.min) ||
            (max != obj.max) ||
            (scaling != obj.scaling) ||
            (wireframe != obj.wireframe));
}

void
ContourAttributes::SetContourValue(int i, double d)
{
    contourValue[i] = d;
    Select(7, (void *)&contourValue);
}

void
ContourAttributes::SetContourPercent(int i, double d)
{
    contourPercent[i] = d;
    Select(8, (void *)&contourPercent);
}

// ****************************************************************************
// Method: ContourAttributes::EnlargeMultiColor
//
// Purpose: 
//   Enlarges the multiColor vector to the new size.
//
// Arguments:
//   newSize : The new size of the color vector.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 2 17:08:32 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Nov 22 11:51:23 PDT 2002
//   I changed how the default colors are created so that the multicolor
//   gets the appropriate colors.
//   
//   Kathleen Bonnell, Thu Jan 13 18:00:01 PST 2005 
//   Ensure that opacities are included. 
// 
// ****************************************************************************

void
ContourAttributes::EnlargeMultiColor(int newSize)
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
// Method: ContourAttributes::ColorIsChanged
//
// Purpose: 
//   Returns whether or not a color has been modified.
//
// Arguments:
//   index : A color index.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 9 11:27:59 PDT 2002
//
// Modifications:
//   
// ***************************************************************************
bool
ContourAttributes::ColorIsChanged(int index) const
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
// Method: ContourAttributes::MarkColorAsChanged
//
// Purpose: 
//   Marks a color as having been modified.
//
// Arguments:
//   index : A color index.
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 9 11:27:59 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
ContourAttributes::MarkColorAsChanged(int index)
{
    if(!ColorIsChanged(index))
    {
        changedColors.push_back(((unsigned char)index));
        SelectChangedColors();
    }
}

bool
ContourAttributes::SetValue(const std::string &name, const int &value)
{
    int index = FieldNameToIndex(name);
    bool retval;
    if(index == ID_contourNLevels)
    {
        SetContourNLevels(value);
        retval = true;
    }
    else
        retval = AttributeSubject::SetValue(name, value);
    return retval;
}

bool
ContourAttributes::SetValue(const std::string &name, const doubleVector &value)
{
    int index = FieldNameToIndex(name);
    bool retval;
    if(index == ID_contourPercent)
    {
        SetContourPercent(value);
        retval = true;
    }
    else if(index == ID_contourValue)
    {
        SetContourValue(value);
        retval = true;
    }
    else
        retval = AttributeSubject::SetValue(name, value);
    return retval;
}

