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

#include <AnnotationAttributes.h>
#include <DataNode.h>

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
const char *AnnotationAttributes::TypeMapFormatString = "aababaiddbaaiaaisiia";

// ****************************************************************************
// Method: AnnotationAttributes::AnnotationAttributes
//
// Purpose: 
//   Constructor for the AnnotationAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AnnotationAttributes::AnnotationAttributes() : 
    AttributeSubject(AnnotationAttributes::TypeMapFormatString),
    backgroundColor(255, 255, 255), foregroundColor(0, 0, 0), 
    gradientColor1(0, 0, 255), gradientColor2(0, 0, 0)
{
    userInfoFlag = true;
    databaseInfoFlag = true;
    databaseInfoExpansionMode = File;
    databaseInfoTimeScale = 1;
    databaseInfoTimeOffset = 0;
    legendInfoFlag = true;
    gradientBackgroundStyle = Radial;
    backgroundMode = Solid;
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AnnotationAttributes::AnnotationAttributes(const AnnotationAttributes &obj) : 
    AttributeSubject(AnnotationAttributes::TypeMapFormatString)
{
    axes2D = obj.axes2D;
    axes3D = obj.axes3D;
    userInfoFlag = obj.userInfoFlag;
    userInfoFont = obj.userInfoFont;
    databaseInfoFlag = obj.databaseInfoFlag;
    databaseInfoFont = obj.databaseInfoFont;
    databaseInfoExpansionMode = obj.databaseInfoExpansionMode;
    databaseInfoTimeScale = obj.databaseInfoTimeScale;
    databaseInfoTimeOffset = obj.databaseInfoTimeOffset;
    legendInfoFlag = obj.legendInfoFlag;
    backgroundColor = obj.backgroundColor;
    foregroundColor = obj.foregroundColor;
    gradientBackgroundStyle = obj.gradientBackgroundStyle;
    gradientColor1 = obj.gradientColor1;
    gradientColor2 = obj.gradientColor2;
    backgroundMode = obj.backgroundMode;
    backgroundImage = obj.backgroundImage;
    imageRepeatX = obj.imageRepeatX;
    imageRepeatY = obj.imageRepeatY;
    axesArray = obj.axesArray;

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
// Creation:   omitted
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AnnotationAttributes& 
AnnotationAttributes::operator = (const AnnotationAttributes &obj)
{
    if (this == &obj) return *this;
    axes2D = obj.axes2D;
    axes3D = obj.axes3D;
    userInfoFlag = obj.userInfoFlag;
    userInfoFont = obj.userInfoFont;
    databaseInfoFlag = obj.databaseInfoFlag;
    databaseInfoFont = obj.databaseInfoFont;
    databaseInfoExpansionMode = obj.databaseInfoExpansionMode;
    databaseInfoTimeScale = obj.databaseInfoTimeScale;
    databaseInfoTimeOffset = obj.databaseInfoTimeOffset;
    legendInfoFlag = obj.legendInfoFlag;
    backgroundColor = obj.backgroundColor;
    foregroundColor = obj.foregroundColor;
    gradientBackgroundStyle = obj.gradientBackgroundStyle;
    gradientColor1 = obj.gradientColor1;
    gradientColor2 = obj.gradientColor2;
    backgroundMode = obj.backgroundMode;
    backgroundImage = obj.backgroundImage;
    imageRepeatX = obj.imageRepeatX;
    imageRepeatY = obj.imageRepeatY;
    axesArray = obj.axesArray;

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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

bool
AnnotationAttributes::operator == (const AnnotationAttributes &obj) const
{
    // Create the return value
    return ((axes2D == obj.axes2D) &&
            (axes3D == obj.axes3D) &&
            (userInfoFlag == obj.userInfoFlag) &&
            (userInfoFont == obj.userInfoFont) &&
            (databaseInfoFlag == obj.databaseInfoFlag) &&
            (databaseInfoFont == obj.databaseInfoFont) &&
            (databaseInfoExpansionMode == obj.databaseInfoExpansionMode) &&
            (databaseInfoTimeScale == obj.databaseInfoTimeScale) &&
            (databaseInfoTimeOffset == obj.databaseInfoTimeOffset) &&
            (legendInfoFlag == obj.legendInfoFlag) &&
            (backgroundColor == obj.backgroundColor) &&
            (foregroundColor == obj.foregroundColor) &&
            (gradientBackgroundStyle == obj.gradientBackgroundStyle) &&
            (gradientColor1 == obj.gradientColor1) &&
            (gradientColor2 == obj.gradientColor2) &&
            (backgroundMode == obj.backgroundMode) &&
            (backgroundImage == obj.backgroundImage) &&
            (imageRepeatX == obj.imageRepeatX) &&
            (imageRepeatY == obj.imageRepeatY) &&
            (axesArray == obj.axesArray));
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
// Creation:   omitted
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
// Creation:   omitted
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
// Creation:   omitted
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
// Creation:   omitted
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
// Creation:   omitted
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
AnnotationAttributes::SelectAll()
{
    Select(ID_axes2D,                    (void *)&axes2D);
    Select(ID_axes3D,                    (void *)&axes3D);
    Select(ID_userInfoFlag,              (void *)&userInfoFlag);
    Select(ID_userInfoFont,              (void *)&userInfoFont);
    Select(ID_databaseInfoFlag,          (void *)&databaseInfoFlag);
    Select(ID_databaseInfoFont,          (void *)&databaseInfoFont);
    Select(ID_databaseInfoExpansionMode, (void *)&databaseInfoExpansionMode);
    Select(ID_databaseInfoTimeScale,     (void *)&databaseInfoTimeScale);
    Select(ID_databaseInfoTimeOffset,    (void *)&databaseInfoTimeOffset);
    Select(ID_legendInfoFlag,            (void *)&legendInfoFlag);
    Select(ID_backgroundColor,           (void *)&backgroundColor);
    Select(ID_foregroundColor,           (void *)&foregroundColor);
    Select(ID_gradientBackgroundStyle,   (void *)&gradientBackgroundStyle);
    Select(ID_gradientColor1,            (void *)&gradientColor1);
    Select(ID_gradientColor2,            (void *)&gradientColor2);
    Select(ID_backgroundMode,            (void *)&backgroundMode);
    Select(ID_backgroundImage,           (void *)&backgroundImage);
    Select(ID_imageRepeatX,              (void *)&imageRepeatX);
    Select(ID_imageRepeatY,              (void *)&imageRepeatY);
    Select(ID_axesArray,                 (void *)&axesArray);
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
// Creation:   omitted
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

    if(completeSave || !FieldsEqual(ID_axes2D, &defaultObject))
    {
        DataNode *axes2DNode = new DataNode("axes2D");
        if(axes2D.CreateNode(axes2DNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(axes2DNode);
        }
        else
            delete axes2DNode;
    }

    if(completeSave || !FieldsEqual(ID_axes3D, &defaultObject))
    {
        DataNode *axes3DNode = new DataNode("axes3D");
        if(axes3D.CreateNode(axes3DNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(axes3DNode);
        }
        else
            delete axes3DNode;
    }

    if(completeSave || !FieldsEqual(ID_userInfoFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("userInfoFlag", userInfoFlag));
    }

    if(completeSave || !FieldsEqual(ID_userInfoFont, &defaultObject))
    {
        DataNode *userInfoFontNode = new DataNode("userInfoFont");
        if(userInfoFont.CreateNode(userInfoFontNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(userInfoFontNode);
        }
        else
            delete userInfoFontNode;
    }

    if(completeSave || !FieldsEqual(ID_databaseInfoFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("databaseInfoFlag", databaseInfoFlag));
    }

    if(completeSave || !FieldsEqual(ID_databaseInfoFont, &defaultObject))
    {
        DataNode *databaseInfoFontNode = new DataNode("databaseInfoFont");
        if(databaseInfoFont.CreateNode(databaseInfoFontNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(databaseInfoFontNode);
        }
        else
            delete databaseInfoFontNode;
    }

    if(completeSave || !FieldsEqual(ID_databaseInfoExpansionMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("databaseInfoExpansionMode", PathExpansionMode_ToString(databaseInfoExpansionMode)));
    }

    if(completeSave || !FieldsEqual(ID_databaseInfoTimeScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("databaseInfoTimeScale", databaseInfoTimeScale));
    }

    if(completeSave || !FieldsEqual(ID_databaseInfoTimeOffset, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("databaseInfoTimeOffset", databaseInfoTimeOffset));
    }

    if(completeSave || !FieldsEqual(ID_legendInfoFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendInfoFlag", legendInfoFlag));
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

    if(completeSave || !FieldsEqual(ID_axesArray, &defaultObject))
    {
        DataNode *axesArrayNode = new DataNode("axesArray");
        if(axesArray.CreateNode(axesArrayNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(axesArrayNode);
        }
        else
            delete axesArrayNode;
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void
AnnotationAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("AnnotationAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("axes2D")) != 0)
        axes2D.SetFromNode(node);
    if((node = searchNode->GetNode("axes3D")) != 0)
        axes3D.SetFromNode(node);
    if((node = searchNode->GetNode("userInfoFlag")) != 0)
        SetUserInfoFlag(node->AsBool());
    if((node = searchNode->GetNode("userInfoFont")) != 0)
        userInfoFont.SetFromNode(node);
    if((node = searchNode->GetNode("databaseInfoFlag")) != 0)
        SetDatabaseInfoFlag(node->AsBool());
    if((node = searchNode->GetNode("databaseInfoFont")) != 0)
        databaseInfoFont.SetFromNode(node);
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
    if((node = searchNode->GetNode("databaseInfoTimeScale")) != 0)
        SetDatabaseInfoTimeScale(node->AsDouble());
    if((node = searchNode->GetNode("databaseInfoTimeOffset")) != 0)
        SetDatabaseInfoTimeOffset(node->AsDouble());
    if((node = searchNode->GetNode("legendInfoFlag")) != 0)
        SetLegendInfoFlag(node->AsBool());
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
    if((node = searchNode->GetNode("imageRepeatX")) != 0)
        SetImageRepeatX(node->AsInt());
    if((node = searchNode->GetNode("imageRepeatY")) != 0)
        SetImageRepeatY(node->AsInt());
    if((node = searchNode->GetNode("axesArray")) != 0)
        axesArray.SetFromNode(node);
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
AnnotationAttributes::SetAxes2D(const Axes2D &axes2D_)
{
    axes2D = axes2D_;
    Select(ID_axes2D, (void *)&axes2D);
}

void
AnnotationAttributes::SetAxes3D(const Axes3D &axes3D_)
{
    axes3D = axes3D_;
    Select(ID_axes3D, (void *)&axes3D);
}

void
AnnotationAttributes::SetUserInfoFlag(bool userInfoFlag_)
{
    userInfoFlag = userInfoFlag_;
    Select(ID_userInfoFlag, (void *)&userInfoFlag);
}

void
AnnotationAttributes::SetUserInfoFont(const FontAttributes &userInfoFont_)
{
    userInfoFont = userInfoFont_;
    Select(ID_userInfoFont, (void *)&userInfoFont);
}

void
AnnotationAttributes::SetDatabaseInfoFlag(bool databaseInfoFlag_)
{
    databaseInfoFlag = databaseInfoFlag_;
    Select(ID_databaseInfoFlag, (void *)&databaseInfoFlag);
}

void
AnnotationAttributes::SetDatabaseInfoFont(const FontAttributes &databaseInfoFont_)
{
    databaseInfoFont = databaseInfoFont_;
    Select(ID_databaseInfoFont, (void *)&databaseInfoFont);
}

void
AnnotationAttributes::SetDatabaseInfoExpansionMode(AnnotationAttributes::PathExpansionMode databaseInfoExpansionMode_)
{
    databaseInfoExpansionMode = databaseInfoExpansionMode_;
    Select(ID_databaseInfoExpansionMode, (void *)&databaseInfoExpansionMode);
}

void
AnnotationAttributes::SetDatabaseInfoTimeScale(double databaseInfoTimeScale_)
{
    databaseInfoTimeScale = databaseInfoTimeScale_;
    Select(ID_databaseInfoTimeScale, (void *)&databaseInfoTimeScale);
}

void
AnnotationAttributes::SetDatabaseInfoTimeOffset(double databaseInfoTimeOffset_)
{
    databaseInfoTimeOffset = databaseInfoTimeOffset_;
    Select(ID_databaseInfoTimeOffset, (void *)&databaseInfoTimeOffset);
}

void
AnnotationAttributes::SetLegendInfoFlag(bool legendInfoFlag_)
{
    legendInfoFlag = legendInfoFlag_;
    Select(ID_legendInfoFlag, (void *)&legendInfoFlag);
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

void
AnnotationAttributes::SetAxesArray(const AxesArray &axesArray_)
{
    axesArray = axesArray_;
    Select(ID_axesArray, (void *)&axesArray);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

const Axes2D &
AnnotationAttributes::GetAxes2D() const
{
    return axes2D;
}

Axes2D &
AnnotationAttributes::GetAxes2D()
{
    return axes2D;
}

const Axes3D &
AnnotationAttributes::GetAxes3D() const
{
    return axes3D;
}

Axes3D &
AnnotationAttributes::GetAxes3D()
{
    return axes3D;
}

bool
AnnotationAttributes::GetUserInfoFlag() const
{
    return userInfoFlag;
}

const FontAttributes &
AnnotationAttributes::GetUserInfoFont() const
{
    return userInfoFont;
}

FontAttributes &
AnnotationAttributes::GetUserInfoFont()
{
    return userInfoFont;
}

bool
AnnotationAttributes::GetDatabaseInfoFlag() const
{
    return databaseInfoFlag;
}

const FontAttributes &
AnnotationAttributes::GetDatabaseInfoFont() const
{
    return databaseInfoFont;
}

FontAttributes &
AnnotationAttributes::GetDatabaseInfoFont()
{
    return databaseInfoFont;
}

AnnotationAttributes::PathExpansionMode
AnnotationAttributes::GetDatabaseInfoExpansionMode() const
{
    return PathExpansionMode(databaseInfoExpansionMode);
}

double
AnnotationAttributes::GetDatabaseInfoTimeScale() const
{
    return databaseInfoTimeScale;
}

double
AnnotationAttributes::GetDatabaseInfoTimeOffset() const
{
    return databaseInfoTimeOffset;
}

bool
AnnotationAttributes::GetLegendInfoFlag() const
{
    return legendInfoFlag;
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

const AxesArray &
AnnotationAttributes::GetAxesArray() const
{
    return axesArray;
}

AxesArray &
AnnotationAttributes::GetAxesArray()
{
    return axesArray;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
AnnotationAttributes::SelectAxes2D()
{
    Select(ID_axes2D, (void *)&axes2D);
}

void
AnnotationAttributes::SelectAxes3D()
{
    Select(ID_axes3D, (void *)&axes3D);
}

void
AnnotationAttributes::SelectUserInfoFont()
{
    Select(ID_userInfoFont, (void *)&userInfoFont);
}

void
AnnotationAttributes::SelectDatabaseInfoFont()
{
    Select(ID_databaseInfoFont, (void *)&databaseInfoFont);
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

void
AnnotationAttributes::SelectAxesArray()
{
    Select(ID_axesArray, (void *)&axesArray);
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
AnnotationAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_axes2D:                    return "axes2D";
    case ID_axes3D:                    return "axes3D";
    case ID_userInfoFlag:              return "userInfoFlag";
    case ID_userInfoFont:              return "userInfoFont";
    case ID_databaseInfoFlag:          return "databaseInfoFlag";
    case ID_databaseInfoFont:          return "databaseInfoFont";
    case ID_databaseInfoExpansionMode: return "databaseInfoExpansionMode";
    case ID_databaseInfoTimeScale:     return "databaseInfoTimeScale";
    case ID_databaseInfoTimeOffset:    return "databaseInfoTimeOffset";
    case ID_legendInfoFlag:            return "legendInfoFlag";
    case ID_backgroundColor:           return "backgroundColor";
    case ID_foregroundColor:           return "foregroundColor";
    case ID_gradientBackgroundStyle:   return "gradientBackgroundStyle";
    case ID_gradientColor1:            return "gradientColor1";
    case ID_gradientColor2:            return "gradientColor2";
    case ID_backgroundMode:            return "backgroundMode";
    case ID_backgroundImage:           return "backgroundImage";
    case ID_imageRepeatX:              return "imageRepeatX";
    case ID_imageRepeatY:              return "imageRepeatY";
    case ID_axesArray:                 return "axesArray";
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
AnnotationAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_axes2D:                    return FieldType_att;
    case ID_axes3D:                    return FieldType_att;
    case ID_userInfoFlag:              return FieldType_bool;
    case ID_userInfoFont:              return FieldType_att;
    case ID_databaseInfoFlag:          return FieldType_bool;
    case ID_databaseInfoFont:          return FieldType_att;
    case ID_databaseInfoExpansionMode: return FieldType_enum;
    case ID_databaseInfoTimeScale:     return FieldType_double;
    case ID_databaseInfoTimeOffset:    return FieldType_double;
    case ID_legendInfoFlag:            return FieldType_bool;
    case ID_backgroundColor:           return FieldType_color;
    case ID_foregroundColor:           return FieldType_color;
    case ID_gradientBackgroundStyle:   return FieldType_enum;
    case ID_gradientColor1:            return FieldType_color;
    case ID_gradientColor2:            return FieldType_color;
    case ID_backgroundMode:            return FieldType_enum;
    case ID_backgroundImage:           return FieldType_string;
    case ID_imageRepeatX:              return FieldType_int;
    case ID_imageRepeatY:              return FieldType_int;
    case ID_axesArray:                 return FieldType_att;
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
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

std::string
AnnotationAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_axes2D:                    return "att";
    case ID_axes3D:                    return "att";
    case ID_userInfoFlag:              return "bool";
    case ID_userInfoFont:              return "att";
    case ID_databaseInfoFlag:          return "bool";
    case ID_databaseInfoFont:          return "att";
    case ID_databaseInfoExpansionMode: return "enum";
    case ID_databaseInfoTimeScale:     return "double";
    case ID_databaseInfoTimeOffset:    return "double";
    case ID_legendInfoFlag:            return "bool";
    case ID_backgroundColor:           return "color";
    case ID_foregroundColor:           return "color";
    case ID_gradientBackgroundStyle:   return "enum";
    case ID_gradientColor1:            return "color";
    case ID_gradientColor2:            return "color";
    case ID_backgroundMode:            return "enum";
    case ID_backgroundImage:           return "string";
    case ID_imageRepeatX:              return "int";
    case ID_imageRepeatY:              return "int";
    case ID_axesArray:                 return "att";
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
// Creation:   omitted
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
    case ID_axes2D:
        {  // new scope
        retval = (axes2D == obj.axes2D);
        }
        break;
    case ID_axes3D:
        {  // new scope
        retval = (axes3D == obj.axes3D);
        }
        break;
    case ID_userInfoFlag:
        {  // new scope
        retval = (userInfoFlag == obj.userInfoFlag);
        }
        break;
    case ID_userInfoFont:
        {  // new scope
        retval = (userInfoFont == obj.userInfoFont);
        }
        break;
    case ID_databaseInfoFlag:
        {  // new scope
        retval = (databaseInfoFlag == obj.databaseInfoFlag);
        }
        break;
    case ID_databaseInfoFont:
        {  // new scope
        retval = (databaseInfoFont == obj.databaseInfoFont);
        }
        break;
    case ID_databaseInfoExpansionMode:
        {  // new scope
        retval = (databaseInfoExpansionMode == obj.databaseInfoExpansionMode);
        }
        break;
    case ID_databaseInfoTimeScale:
        {  // new scope
        retval = (databaseInfoTimeScale == obj.databaseInfoTimeScale);
        }
        break;
    case ID_databaseInfoTimeOffset:
        {  // new scope
        retval = (databaseInfoTimeOffset == obj.databaseInfoTimeOffset);
        }
        break;
    case ID_legendInfoFlag:
        {  // new scope
        retval = (legendInfoFlag == obj.legendInfoFlag);
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
    case ID_axesArray:
        {  // new scope
        retval = (axesArray == obj.axesArray);
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

// ****************************************************************************
// Method: AnnotationAttributes::ProcessOldVersions
//
// Purpose: 
//   This method maps old AnnotationAttributes fields to the 1.9.0 version of
//   the data node tree for AnnotationAttributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 14 15:40:02 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Mar 26 14:49:15 PDT 2008
//   Added conversion of heights to scales.
//
// ****************************************************************************

#include <Utility.h>
#include <DebugStream.h>

typedef struct
{
    const char *old_key;
    const char *new_key;
} axis_attribute_mapping;

static const axis_attribute_mapping attribute_mappings[] = {
    {"axesFlag2D",            "axes2D.Axes2D.visible"},
    {"axesAutoSetTicks2D",    "axes2D.Axes2D.autoSetTicks"},
    {"labelAutoSetScaling2D", "axes2D.Axes2D.autoSetScaling"},
    {"xAxisLabels2D",         "axes2D.Axes2D.xAxis.AxisAttributes.label.AxisLabels.visible"},
    {"yAxisLabels2D",         "axes2D.Axes2D.yAxis.AxisAttributes.label.AxisLabels.visible"},
    {"xAxisTitle2D",          "axes2D.Axes2D.xAxis.AxisAttributes.title.AxisTitles.visible"},
    {"yAxisTitle2D",          "axes2D.Axes2D.yAxis.AxisAttributes.title.AxisTitles.visible"},
    {"xGridLines2D",          "axes2D.Axes2D.xAxis.AxisAttributes.grid"},
    {"yGridLines2D",          "axes2D.Axes2D.yAxis.AxisAttributes.grid"},
    {"xMajorTickMinimum2D",   "axes2D.Axes2D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMinimum"},
    {"yMajorTickMinimum2D",   "axes2D.Axes2D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMinimum"},
    {"xMajorTickMaximum2D",   "axes2D.Axes2D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMaximum"},
    {"yMajorTickMaximum2D",   "axes2D.Axes2D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMaximum"},
    {"xMajorTickSpacing2D",   "axes2D.Axes2D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.majorSpacing"},
    {"yMajorTickSpacing2D",   "axes2D.Axes2D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.majorSpacing"},
    {"xMinorTickSpacing2D",   "axes2D.Axes2D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.minorSpacing"},
    {"yMinorTickSpacing2D",   "axes2D.Axes2D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.minorSpacing"},
    {"xLabelFontHeight2D",    "axes2D.Axes2D.xAxis.AxisAttributes.label.AxisLabels.font.FontAttributes.scale"},
    {"yLabelFontHeight2D",    "axes2D.Axes2D.yAxis.AxisAttributes.label.AxisLabels.font.FontAttributes.scale"},
    {"xTitleFontHeight2D",    "axes2D.Axes2D.xAxis.AxisAttributes.title.AxisTitles.font.FontAttributes.scale"},
    {"yTitleFontHeight2D",    "axes2D.Axes2D.yAxis.AxisAttributes.title.AxisTitles.font.FontAttributes.scale"},
    {"xLabelScaling2D",       "axes2D.Axes2D.xAxis.AxisAttributes.label.AxisLabels.scaling"},
    {"yLabelScaling2D",       "axes2D.Axes2D.yAxis.AxisAttributes.label.AxisLabels.scaling"},
    {"axesLineWidth2D",       "axes2D.Axes2D.lineWidth"},
    {"axesTickLocation2D",    "axes2D.Axes2D.tickLocation"},
    {"axesTicks2D",           "axes2D.Axes2D.tickAxes"},
    {"xAxisUserTitle2D",      "axes2D.Axes2D.xAxis.AxisAttributes.title.AxisTitles.title"},
    {"yAxisUserTitle2D",      "axes2D.Axes2D.yAxis.AxisAttributes.title.AxisTitles.title"},
    {"xAxisUserTitleFlag2D",  "axes2D.Axes2D.xAxis.AxisAttributes.title.AxisTitles.userTitle"},
    {"yAxisUserTitleFlag2D",  "axes2D.Axes2D.yAxis.AxisAttributes.title.AxisTitles.userTitle"},
    {"xAxisUserUnits2D",      "axes2D.Axes2D.xAxis.AxisAttributes.title.AxisTitles.units"},
    {"yAxisUserUnits2D",      "axes2D.Axes2D.yAxis.AxisAttributes.title.AxisTitles.units"},
    {"xAxisUserUnitsFlag2D",  "axes2D.Axes2D.xAxis.AxisAttributes.title.AxisTitles.userUnits"},
    {"yAxisUserUnitsFlag2D",  "axes2D.Axes2D.yAxis.AxisAttributes.title.AxisTitles.userUnits"},
    {"axesFlag",              "axes3D.Axes3D.visible"},
    {"axesAutoSetTicks",      "axes3D.Axes3D.autoSetTicks"},
    {"labelAutoSetScaling",   "axes3D.Axes3D.autoSetScaling"},
    {"xAxisLabels",           "axes3D.Axes3D.xAxis.AxisAttributes.label.AxisLabels.visible"},
    {"yAxisLabels",           "axes3D.Axes3D.yAxis.AxisAttributes.label.AxisLabels.visible"},
    {"zAxisLabels",           "axes3D.Axes3D.zAxis.AxisAttributes.label.AxisLabels.visible"},
    {"xAxisTitle",            "axes3D.Axes3D.xAxis.AxisAttributes.title.AxisTitles.visible"},
    {"yAxisTitle",            "axes3D.Axes3D.yAxis.AxisAttributes.title.AxisTitles.visible"},
    {"zAxisTitle",            "axes3D.Axes3D.zAxis.AxisAttributes.title.AxisTitles.visible"},
    {"xGridLines",            "axes3D.Axes3D.xAxis.AxisAttributes.grid"},
    {"yGridLines",            "axes3D.Axes3D.yAxis.AxisAttributes.grid"},
    {"zGridLines",            "axes3D.Axes3D.zAxis.AxisAttributes.grid"},
    {"xAxisTicks",            "axes3D.Axes3D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.visible"},
    {"yAxisTicks",            "axes3D.Axes3D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.visible"},
    {"zAxisTicks",            "axes3D.Axes3D.zAxis.AxisAttributes.tickMarks.AxisTickMarks.visible"},
    {"xMajorTickMinimum",     "axes3D.Axes3D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMinimum"},
    {"yMajorTickMinimum",     "axes3D.Axes3D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMinimum"},
    {"zMajorTickMinimum",     "axes3D.Axes3D.zAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMinimum"},
    {"xMajorTickMaximum",     "axes3D.Axes3D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMaximum"},
    {"yMajorTickMaximum",     "axes3D.Axes3D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMaximum"},
    {"zMajorTickMaximum",     "axes3D.Axes3D.zAxis.AxisAttributes.tickMarks.AxisTickMarks.majorMaximum"},
    {"xMajorTickSpacing",     "axes3D.Axes3D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.majorSpacing"},
    {"yMajorTickSpacing",     "axes3D.Axes3D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.majorSpacing"},
    {"zMajorTickSpacing",     "axes3D.Axes3D.zAxis.AxisAttributes.tickMarks.AxisTickMarks.majorSpacing"},
    {"xMinorTickSpacing",     "axes3D.Axes3D.xAxis.AxisAttributes.tickMarks.AxisTickMarks.minorSpacing"},
    {"yMinorTickSpacing",     "axes3D.Axes3D.yAxis.AxisAttributes.tickMarks.AxisTickMarks.minorSpacing"},
    {"zMinorTickSpacing",     "axes3D.Axes3D.zAxis.AxisAttributes.tickMarks.AxisTickMarks.minorSpacing"},
    {"xLabelFontHeight",      "axes3D.Axes3D.xAxis.AxisAttributes.label.AxisLabels.font.FontAttributes.scale"},
    {"yLabelFontHeight",      "axes3D.Axes3D.yAxis.AxisAttributes.label.AxisLabels.font.FontAttributes.scale"},
    {"zLabelFontHeight",      "axes3D.Axes3D.zAxis.AxisAttributes.label.AxisLabels.font.FontAttributes.scale"},
    {"xTitleFontHeight",      "axes3D.Axes3D.xAxis.AxisAttributes.title.AxisTitles.font.FontAttributes.scale"},
    {"yTitleFontHeight",      "axes3D.Axes3D.yAxis.AxisAttributes.title.AxisTitles.font.FontAttributes.scale"},
    {"zTitleFontHeight",      "axes3D.Axes3D.zAxis.AxisAttributes.title.AxisTitles.font.FontAttributes.scale"},
    {"xLabelScaling",         "axes3D.Axes3D.xAxis.AxisAttributes.label.AxisLabels.scaling"},
    {"yLabelScaling",         "axes3D.Axes3D.yAxis.AxisAttributes.label.AxisLabels.scaling"},
    {"zLabelScaling",         "axes3D.Axes3D.zAxis.AxisAttributes.label.AxisLabels.scaling"},
    {"xAxisUserTitle",        "axes3D.Axes3D.xAxis.AxisAttributes.title.AxisTitles.title"},
    {"yAxisUserTitle",        "axes3D.Axes3D.yAxis.AxisAttributes.title.AxisTitles.title"},
    {"zAxisUserTitle",        "axes3D.Axes3D.zAxis.AxisAttributes.title.AxisTitles.title"},
    {"xAxisUserTitleFlag",    "axes3D.Axes3D.xAxis.AxisAttributes.title.AxisTitles.userTitle"},
    {"yAxisUserTitleFlag",    "axes3D.Axes3D.yAxis.AxisAttributes.title.AxisTitles.userTitle"},
    {"zAxisUserTitleFlag",    "axes3D.Axes3D.zAxis.AxisAttributes.title.AxisTitles.userTitle"},
    {"xAxisUserUnits",        "axes3D.Axes3D.xAxis.AxisAttributes.title.AxisTitles.units"},
    {"yAxisUserUnits",        "axes3D.Axes3D.yAxis.AxisAttributes.title.AxisTitles.units"},
    {"zAxisUserUnits",        "axes3D.Axes3D.zAxis.AxisAttributes.title.AxisTitles.units"},
    {"xAxisUserUnitsFlag",    "axes3D.Axes3D.xAxis.AxisAttributes.title.AxisTitles.userUnits"},
    {"yAxisUserUnitsFlag",    "axes3D.Axes3D.yAxis.AxisAttributes.title.AxisTitles.userUnits"},
    {"zAxisUserUnitsFlag",    "axes3D.Axes3D.zAxis.AxisAttributes.title.AxisTitles.userUnits"},
    {"axesTickLocation",      "axes3D.Axes3D.tickLocation"},
    {"axesType",              "axes3D.Axes3D.axesType"},
    {"triadFlag",             "axes3D.Axes3D.triadFlag"},
    {"bboxFlag",              "axes3D.Axes3D.bboxFlag"}
};

const char *height_attributes[] = {
    "xLabelFontHeight2D",
    "yLabelFontHeight2D",
    "xTitleFontHeight2D",
    "yTitleFontHeight2D",
    "xLabelFontHeight",
    "yLabelFontHeight",
    "zLabelFontHeight",
    "xTitleFontHeight",
    "yTitleFontHeight",
    "zTitleFontHeight"
};

void
AnnotationAttributes::ProcessOldVersions(DataNode *parentNode,
    const char *configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("AnnotationAttributes");
    if(searchNode == 0)
        return;

    if(VersionLessThan(configVersion, "1.9.0"))
    {
        for(int key = 0; key < sizeof(attribute_mappings)/sizeof(axis_attribute_mapping); ++key)
        {
            DataNode *k = searchNode->GetNode(attribute_mappings[key].old_key);

            if(k != 0)
            {
                searchNode->RemoveNode(k, false);

                // Reparent the node.
                stringVector keys = SplitValues(attribute_mappings[key].new_key, '.');
                DataNode *pNode = searchNode;
                for(size_t i = 0; i < keys.size(); ++i)
                {
                    if(i < keys.size() - 1)
                    {                     
                        DataNode *node = pNode->GetNode(keys[i]);
                        if(node == 0)
                        {
                            node = new DataNode(keys[i]);
                            pNode->AddNode(node);
                            debug5 << "Created node: " << keys[i].c_str() << endl;
                        }
                        pNode = node;
                    }
                    else
                    {
                        // Add the node that we're moving and rename it too.
                        debug5 << "Moved node " << k->GetKey().c_str() << " to " << keys[i].c_str() << endl;
                        k->SetKey(keys[i]);
                        pNode->AddNode(k);

                        // If the node that we're moving is a height attribute 
                        // then we need to change it to a scale attribute.
                        for(int hIndex = 0; hIndex < 10; ++hIndex)
                        {
                            if(std::string(attribute_mappings[key].old_key) == std::string(height_attributes[hIndex]) &&
                               k->GetNodeType() == DOUBLE_NODE)
                            {
                                double h = k->AsDouble();
                                double scale = h / 0.02;
                                k->SetDouble(scale);
                                debug5 << "Turned height " << h << " into scale " << scale << endl;
                                break;
                            }
                        }
                    }
                }
            }               
        }
    }
}

