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

#include <RenderingAttributes.h>
#include <DataNode.h>
#include <climits>

const int RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD = 2000000;

const int RenderingAttributes::DEFAULT_SCALABLE_ACTIVATION_MODE = Auto;

//
// Enum conversion methods for RenderingAttributes::GeometryRepresentation
//

static const char *GeometryRepresentation_strings[] = {
"Surfaces", "Wireframe", "Points"
};

std::string
RenderingAttributes::GeometryRepresentation_ToString(RenderingAttributes::GeometryRepresentation t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return GeometryRepresentation_strings[index];
}

std::string
RenderingAttributes::GeometryRepresentation_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return GeometryRepresentation_strings[index];
}

bool
RenderingAttributes::GeometryRepresentation_FromString(const std::string &s, RenderingAttributes::GeometryRepresentation &val)
{
    val = RenderingAttributes::Surfaces;
    for(int i = 0; i < 3; ++i)
    {
        if(s == GeometryRepresentation_strings[i])
        {
            val = (GeometryRepresentation)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for RenderingAttributes::StereoTypes
//

static const char *StereoTypes_strings[] = {
"RedBlue", "Interlaced", "CrystalEyes", 
"RedGreen"};

std::string
RenderingAttributes::StereoTypes_ToString(RenderingAttributes::StereoTypes t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return StereoTypes_strings[index];
}

std::string
RenderingAttributes::StereoTypes_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return StereoTypes_strings[index];
}

bool
RenderingAttributes::StereoTypes_FromString(const std::string &s, RenderingAttributes::StereoTypes &val)
{
    val = RenderingAttributes::RedBlue;
    for(int i = 0; i < 4; ++i)
    {
        if(s == StereoTypes_strings[i])
        {
            val = (StereoTypes)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for RenderingAttributes::TriStateMode
//

static const char *TriStateMode_strings[] = {
"Never", "Always", "Auto"
};

std::string
RenderingAttributes::TriStateMode_ToString(RenderingAttributes::TriStateMode t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return TriStateMode_strings[index];
}

std::string
RenderingAttributes::TriStateMode_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return TriStateMode_strings[index];
}

bool
RenderingAttributes::TriStateMode_FromString(const std::string &s, RenderingAttributes::TriStateMode &val)
{
    val = RenderingAttributes::Never;
    for(int i = 0; i < 3; ++i)
    {
        if(s == TriStateMode_strings[i])
        {
            val = (TriStateMode)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *RenderingAttributes::TypeMapFormatString = "biibibiibffabdbDDib";

// ****************************************************************************
// Method: RenderingAttributes::RenderingAttributes
//
// Purpose: 
//   Constructor for the RenderingAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

RenderingAttributes::RenderingAttributes() : 
    AttributeSubject(RenderingAttributes::TypeMapFormatString),
    specularColor(255, 255, 255)
{
    antialiasing = false;
    geometryRepresentation = Surfaces;
    displayListMode = Auto;
    stereoRendering = false;
    stereoType = CrystalEyes;
    notifyForEachRender = false;
    scalableActivationMode = Auto;
    scalableAutoThreshold = 2000000;
    specularFlag = false;
    specularCoeff = 0.6;
    specularPower = 10;
    doShadowing = false;
    shadowStrength = 0.5;
    doDepthCueing = false;
    startCuePoint[0] = -10;
    startCuePoint[1] = 0;
    startCuePoint[2] = 0;
    endCuePoint[0] = 10;
    endCuePoint[1] = 0;
    endCuePoint[2] = 0;
    compressionActivationMode = Never;
    colorTexturingFlag = true;
}

// ****************************************************************************
// Method: RenderingAttributes::RenderingAttributes
//
// Purpose: 
//   Copy constructor for the RenderingAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

RenderingAttributes::RenderingAttributes(const RenderingAttributes &obj) : 
    AttributeSubject(RenderingAttributes::TypeMapFormatString)
{
    antialiasing = obj.antialiasing;
    geometryRepresentation = obj.geometryRepresentation;
    displayListMode = obj.displayListMode;
    stereoRendering = obj.stereoRendering;
    stereoType = obj.stereoType;
    notifyForEachRender = obj.notifyForEachRender;
    scalableActivationMode = obj.scalableActivationMode;
    scalableAutoThreshold = obj.scalableAutoThreshold;
    specularFlag = obj.specularFlag;
    specularCoeff = obj.specularCoeff;
    specularPower = obj.specularPower;
    specularColor = obj.specularColor;
    doShadowing = obj.doShadowing;
    shadowStrength = obj.shadowStrength;
    doDepthCueing = obj.doDepthCueing;
    startCuePoint[0] = obj.startCuePoint[0];
    startCuePoint[1] = obj.startCuePoint[1];
    startCuePoint[2] = obj.startCuePoint[2];

    endCuePoint[0] = obj.endCuePoint[0];
    endCuePoint[1] = obj.endCuePoint[1];
    endCuePoint[2] = obj.endCuePoint[2];

    compressionActivationMode = obj.compressionActivationMode;
    colorTexturingFlag = obj.colorTexturingFlag;

    SelectAll();
}

// ****************************************************************************
// Method: RenderingAttributes::~RenderingAttributes
//
// Purpose: 
//   Destructor for the RenderingAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

RenderingAttributes::~RenderingAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: RenderingAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the RenderingAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

RenderingAttributes& 
RenderingAttributes::operator = (const RenderingAttributes &obj)
{
    if (this == &obj) return *this;
    antialiasing = obj.antialiasing;
    geometryRepresentation = obj.geometryRepresentation;
    displayListMode = obj.displayListMode;
    stereoRendering = obj.stereoRendering;
    stereoType = obj.stereoType;
    notifyForEachRender = obj.notifyForEachRender;
    scalableActivationMode = obj.scalableActivationMode;
    scalableAutoThreshold = obj.scalableAutoThreshold;
    specularFlag = obj.specularFlag;
    specularCoeff = obj.specularCoeff;
    specularPower = obj.specularPower;
    specularColor = obj.specularColor;
    doShadowing = obj.doShadowing;
    shadowStrength = obj.shadowStrength;
    doDepthCueing = obj.doDepthCueing;
    startCuePoint[0] = obj.startCuePoint[0];
    startCuePoint[1] = obj.startCuePoint[1];
    startCuePoint[2] = obj.startCuePoint[2];

    endCuePoint[0] = obj.endCuePoint[0];
    endCuePoint[1] = obj.endCuePoint[1];
    endCuePoint[2] = obj.endCuePoint[2];

    compressionActivationMode = obj.compressionActivationMode;
    colorTexturingFlag = obj.colorTexturingFlag;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: RenderingAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the RenderingAttributes class.
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
RenderingAttributes::operator == (const RenderingAttributes &obj) const
{
    // Compare the startCuePoint arrays.
    bool startCuePoint_equal = true;
    for(int i = 0; i < 3 && startCuePoint_equal; ++i)
        startCuePoint_equal = (startCuePoint[i] == obj.startCuePoint[i]);

    // Compare the endCuePoint arrays.
    bool endCuePoint_equal = true;
    for(int i = 0; i < 3 && endCuePoint_equal; ++i)
        endCuePoint_equal = (endCuePoint[i] == obj.endCuePoint[i]);

    // Create the return value
    return ((antialiasing == obj.antialiasing) &&
            (geometryRepresentation == obj.geometryRepresentation) &&
            (displayListMode == obj.displayListMode) &&
            (stereoRendering == obj.stereoRendering) &&
            (stereoType == obj.stereoType) &&
            (notifyForEachRender == obj.notifyForEachRender) &&
            (scalableActivationMode == obj.scalableActivationMode) &&
            (scalableAutoThreshold == obj.scalableAutoThreshold) &&
            (specularFlag == obj.specularFlag) &&
            (specularCoeff == obj.specularCoeff) &&
            (specularPower == obj.specularPower) &&
            (specularColor == obj.specularColor) &&
            (doShadowing == obj.doShadowing) &&
            (shadowStrength == obj.shadowStrength) &&
            (doDepthCueing == obj.doDepthCueing) &&
            startCuePoint_equal &&
            endCuePoint_equal &&
            (compressionActivationMode == obj.compressionActivationMode) &&
            (colorTexturingFlag == obj.colorTexturingFlag));
}

// ****************************************************************************
// Method: RenderingAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the RenderingAttributes class.
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
RenderingAttributes::operator != (const RenderingAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: RenderingAttributes::TypeName
//
// Purpose: 
//   Type name method for the RenderingAttributes class.
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
RenderingAttributes::TypeName() const
{
    return "RenderingAttributes";
}

// ****************************************************************************
// Method: RenderingAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the RenderingAttributes class.
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
RenderingAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const RenderingAttributes *tmp = (const RenderingAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: RenderingAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the RenderingAttributes class.
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
RenderingAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new RenderingAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: RenderingAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the RenderingAttributes class.
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
RenderingAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new RenderingAttributes(*this);
    else
        retval = new RenderingAttributes;

    return retval;
}

// ****************************************************************************
// Method: RenderingAttributes::SelectAll
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
RenderingAttributes::SelectAll()
{
    Select(ID_antialiasing,              (void *)&antialiasing);
    Select(ID_geometryRepresentation,    (void *)&geometryRepresentation);
    Select(ID_displayListMode,           (void *)&displayListMode);
    Select(ID_stereoRendering,           (void *)&stereoRendering);
    Select(ID_stereoType,                (void *)&stereoType);
    Select(ID_notifyForEachRender,       (void *)&notifyForEachRender);
    Select(ID_scalableActivationMode,    (void *)&scalableActivationMode);
    Select(ID_scalableAutoThreshold,     (void *)&scalableAutoThreshold);
    Select(ID_specularFlag,              (void *)&specularFlag);
    Select(ID_specularCoeff,             (void *)&specularCoeff);
    Select(ID_specularPower,             (void *)&specularPower);
    Select(ID_specularColor,             (void *)&specularColor);
    Select(ID_doShadowing,               (void *)&doShadowing);
    Select(ID_shadowStrength,            (void *)&shadowStrength);
    Select(ID_doDepthCueing,             (void *)&doDepthCueing);
    Select(ID_startCuePoint,             (void *)startCuePoint, 3);
    Select(ID_endCuePoint,               (void *)endCuePoint, 3);
    Select(ID_compressionActivationMode, (void *)&compressionActivationMode);
    Select(ID_colorTexturingFlag,        (void *)&colorTexturingFlag);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RenderingAttributes::CreateNode
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
RenderingAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    RenderingAttributes defaultObject;
    bool addToParent = false;
    // Create a node for RenderingAttributes.
    DataNode *node = new DataNode("RenderingAttributes");

    if(completeSave || !FieldsEqual(ID_antialiasing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("antialiasing", antialiasing));
    }

    if(completeSave || !FieldsEqual(ID_geometryRepresentation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("geometryRepresentation", GeometryRepresentation_ToString(geometryRepresentation)));
    }

    if(completeSave || !FieldsEqual(ID_displayListMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayListMode", TriStateMode_ToString(displayListMode)));
    }

    if(completeSave || !FieldsEqual(ID_stereoRendering, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("stereoRendering", stereoRendering));
    }

    if(completeSave || !FieldsEqual(ID_stereoType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("stereoType", StereoTypes_ToString(stereoType)));
    }

    if(completeSave || !FieldsEqual(ID_notifyForEachRender, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("notifyForEachRender", notifyForEachRender));
    }

    if(completeSave || !FieldsEqual(ID_scalableActivationMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scalableActivationMode", TriStateMode_ToString(scalableActivationMode)));
    }

    if(completeSave || !FieldsEqual(ID_scalableAutoThreshold, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scalableAutoThreshold", scalableAutoThreshold));
    }

    if(completeSave || !FieldsEqual(ID_specularFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specularFlag", specularFlag));
    }

    if(completeSave || !FieldsEqual(ID_specularCoeff, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specularCoeff", specularCoeff));
    }

    if(completeSave || !FieldsEqual(ID_specularPower, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specularPower", specularPower));
    }

        DataNode *specularColorNode = new DataNode("specularColor");
        if(specularColor.CreateNode(specularColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(specularColorNode);
        }
        else
            delete specularColorNode;
    if(completeSave || !FieldsEqual(ID_doShadowing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doShadowing", doShadowing));
    }

    if(completeSave || !FieldsEqual(ID_shadowStrength, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("shadowStrength", shadowStrength));
    }

    if(completeSave || !FieldsEqual(ID_doDepthCueing, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("doDepthCueing", doDepthCueing));
    }

    if(completeSave || !FieldsEqual(ID_startCuePoint, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("startCuePoint", startCuePoint, 3));
    }

    if(completeSave || !FieldsEqual(ID_endCuePoint, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endCuePoint", endCuePoint, 3));
    }

    if(completeSave || !FieldsEqual(ID_compressionActivationMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("compressionActivationMode", TriStateMode_ToString(compressionActivationMode)));
    }

    if(completeSave || !FieldsEqual(ID_colorTexturingFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTexturingFlag", colorTexturingFlag));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: RenderingAttributes::SetFromNode
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
RenderingAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("RenderingAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("antialiasing")) != 0)
        SetAntialiasing(node->AsBool());
    if((node = searchNode->GetNode("geometryRepresentation")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetGeometryRepresentation(GeometryRepresentation(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            GeometryRepresentation value;
            if(GeometryRepresentation_FromString(node->AsString(), value))
                SetGeometryRepresentation(value);
        }
    }
    if((node = searchNode->GetNode("displayListMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDisplayListMode(TriStateMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TriStateMode value;
            if(TriStateMode_FromString(node->AsString(), value))
                SetDisplayListMode(value);
        }
    }
    if((node = searchNode->GetNode("stereoRendering")) != 0)
        SetStereoRendering(node->AsBool());
    if((node = searchNode->GetNode("stereoType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetStereoType(StereoTypes(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            StereoTypes value;
            if(StereoTypes_FromString(node->AsString(), value))
                SetStereoType(value);
        }
    }
    if((node = searchNode->GetNode("notifyForEachRender")) != 0)
        SetNotifyForEachRender(node->AsBool());
    if((node = searchNode->GetNode("scalableActivationMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetScalableActivationMode(TriStateMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TriStateMode value;
            if(TriStateMode_FromString(node->AsString(), value))
                SetScalableActivationMode(value);
        }
    }
    if((node = searchNode->GetNode("scalableAutoThreshold")) != 0)
        SetScalableAutoThreshold(node->AsInt());
    if((node = searchNode->GetNode("specularFlag")) != 0)
        SetSpecularFlag(node->AsBool());
    if((node = searchNode->GetNode("specularCoeff")) != 0)
        SetSpecularCoeff(node->AsFloat());
    if((node = searchNode->GetNode("specularPower")) != 0)
        SetSpecularPower(node->AsFloat());
    if((node = searchNode->GetNode("specularColor")) != 0)
        specularColor.SetFromNode(node);
    if((node = searchNode->GetNode("doShadowing")) != 0)
        SetDoShadowing(node->AsBool());
    if((node = searchNode->GetNode("shadowStrength")) != 0)
        SetShadowStrength(node->AsDouble());
    if((node = searchNode->GetNode("doDepthCueing")) != 0)
        SetDoDepthCueing(node->AsBool());
    if((node = searchNode->GetNode("startCuePoint")) != 0)
        SetStartCuePoint(node->AsDoubleArray());
    if((node = searchNode->GetNode("endCuePoint")) != 0)
        SetEndCuePoint(node->AsDoubleArray());
    if((node = searchNode->GetNode("compressionActivationMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetCompressionActivationMode(TriStateMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TriStateMode value;
            if(TriStateMode_FromString(node->AsString(), value))
                SetCompressionActivationMode(value);
        }
    }
    if((node = searchNode->GetNode("colorTexturingFlag")) != 0)
        SetColorTexturingFlag(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
RenderingAttributes::SetAntialiasing(bool antialiasing_)
{
    antialiasing = antialiasing_;
    Select(ID_antialiasing, (void *)&antialiasing);
}

void
RenderingAttributes::SetGeometryRepresentation(RenderingAttributes::GeometryRepresentation geometryRepresentation_)
{
    geometryRepresentation = geometryRepresentation_;
    Select(ID_geometryRepresentation, (void *)&geometryRepresentation);
}

void
RenderingAttributes::SetDisplayListMode(RenderingAttributes::TriStateMode displayListMode_)
{
    displayListMode = displayListMode_;
    Select(ID_displayListMode, (void *)&displayListMode);
}

void
RenderingAttributes::SetStereoRendering(bool stereoRendering_)
{
    stereoRendering = stereoRendering_;
    Select(ID_stereoRendering, (void *)&stereoRendering);
}

void
RenderingAttributes::SetStereoType(RenderingAttributes::StereoTypes stereoType_)
{
    stereoType = stereoType_;
    Select(ID_stereoType, (void *)&stereoType);
}

void
RenderingAttributes::SetNotifyForEachRender(bool notifyForEachRender_)
{
    notifyForEachRender = notifyForEachRender_;
    Select(ID_notifyForEachRender, (void *)&notifyForEachRender);
}

void
RenderingAttributes::SetScalableActivationMode(RenderingAttributes::TriStateMode scalableActivationMode_)
{
    scalableActivationMode = scalableActivationMode_;
    Select(ID_scalableActivationMode, (void *)&scalableActivationMode);
}

void
RenderingAttributes::SetScalableAutoThreshold(int scalableAutoThreshold_)
{
    scalableAutoThreshold = scalableAutoThreshold_;
    Select(ID_scalableAutoThreshold, (void *)&scalableAutoThreshold);
}

void
RenderingAttributes::SetSpecularFlag(bool specularFlag_)
{
    specularFlag = specularFlag_;
    Select(ID_specularFlag, (void *)&specularFlag);
}

void
RenderingAttributes::SetSpecularCoeff(float specularCoeff_)
{
    specularCoeff = specularCoeff_;
    Select(ID_specularCoeff, (void *)&specularCoeff);
}

void
RenderingAttributes::SetSpecularPower(float specularPower_)
{
    specularPower = specularPower_;
    Select(ID_specularPower, (void *)&specularPower);
}

void
RenderingAttributes::SetSpecularColor(const ColorAttribute &specularColor_)
{
    specularColor = specularColor_;
    Select(ID_specularColor, (void *)&specularColor);
}

void
RenderingAttributes::SetDoShadowing(bool doShadowing_)
{
    doShadowing = doShadowing_;
    Select(ID_doShadowing, (void *)&doShadowing);
}

void
RenderingAttributes::SetShadowStrength(double shadowStrength_)
{
    shadowStrength = shadowStrength_;
    Select(ID_shadowStrength, (void *)&shadowStrength);
}

void
RenderingAttributes::SetDoDepthCueing(bool doDepthCueing_)
{
    doDepthCueing = doDepthCueing_;
    Select(ID_doDepthCueing, (void *)&doDepthCueing);
}

void
RenderingAttributes::SetStartCuePoint(const double *startCuePoint_)
{
    startCuePoint[0] = startCuePoint_[0];
    startCuePoint[1] = startCuePoint_[1];
    startCuePoint[2] = startCuePoint_[2];
    Select(ID_startCuePoint, (void *)startCuePoint, 3);
}

void
RenderingAttributes::SetEndCuePoint(const double *endCuePoint_)
{
    endCuePoint[0] = endCuePoint_[0];
    endCuePoint[1] = endCuePoint_[1];
    endCuePoint[2] = endCuePoint_[2];
    Select(ID_endCuePoint, (void *)endCuePoint, 3);
}

void
RenderingAttributes::SetCompressionActivationMode(RenderingAttributes::TriStateMode compressionActivationMode_)
{
    compressionActivationMode = compressionActivationMode_;
    Select(ID_compressionActivationMode, (void *)&compressionActivationMode);
}

void
RenderingAttributes::SetColorTexturingFlag(bool colorTexturingFlag_)
{
    colorTexturingFlag = colorTexturingFlag_;
    Select(ID_colorTexturingFlag, (void *)&colorTexturingFlag);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
RenderingAttributes::GetAntialiasing() const
{
    return antialiasing;
}

RenderingAttributes::GeometryRepresentation
RenderingAttributes::GetGeometryRepresentation() const
{
    return GeometryRepresentation(geometryRepresentation);
}

RenderingAttributes::TriStateMode
RenderingAttributes::GetDisplayListMode() const
{
    return TriStateMode(displayListMode);
}

bool
RenderingAttributes::GetStereoRendering() const
{
    return stereoRendering;
}

RenderingAttributes::StereoTypes
RenderingAttributes::GetStereoType() const
{
    return StereoTypes(stereoType);
}

bool
RenderingAttributes::GetNotifyForEachRender() const
{
    return notifyForEachRender;
}

RenderingAttributes::TriStateMode
RenderingAttributes::GetScalableActivationMode() const
{
    return TriStateMode(scalableActivationMode);
}

int
RenderingAttributes::GetScalableAutoThreshold() const
{
    return scalableAutoThreshold;
}

bool
RenderingAttributes::GetSpecularFlag() const
{
    return specularFlag;
}

float
RenderingAttributes::GetSpecularCoeff() const
{
    return specularCoeff;
}

float
RenderingAttributes::GetSpecularPower() const
{
    return specularPower;
}

const ColorAttribute &
RenderingAttributes::GetSpecularColor() const
{
    return specularColor;
}

ColorAttribute &
RenderingAttributes::GetSpecularColor()
{
    return specularColor;
}

bool
RenderingAttributes::GetDoShadowing() const
{
    return doShadowing;
}

double
RenderingAttributes::GetShadowStrength() const
{
    return shadowStrength;
}

bool
RenderingAttributes::GetDoDepthCueing() const
{
    return doDepthCueing;
}

const double *
RenderingAttributes::GetStartCuePoint() const
{
    return startCuePoint;
}

double *
RenderingAttributes::GetStartCuePoint()
{
    return startCuePoint;
}

const double *
RenderingAttributes::GetEndCuePoint() const
{
    return endCuePoint;
}

double *
RenderingAttributes::GetEndCuePoint()
{
    return endCuePoint;
}

RenderingAttributes::TriStateMode
RenderingAttributes::GetCompressionActivationMode() const
{
    return TriStateMode(compressionActivationMode);
}

bool
RenderingAttributes::GetColorTexturingFlag() const
{
    return colorTexturingFlag;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
RenderingAttributes::SelectSpecularColor()
{
    Select(ID_specularColor, (void *)&specularColor);
}

void
RenderingAttributes::SelectStartCuePoint()
{
    Select(ID_startCuePoint, (void *)startCuePoint, 3);
}

void
RenderingAttributes::SelectEndCuePoint()
{
    Select(ID_endCuePoint, (void *)endCuePoint, 3);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RenderingAttributes::GetFieldName
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
RenderingAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_antialiasing:              return "antialiasing";
    case ID_geometryRepresentation:    return "geometryRepresentation";
    case ID_displayListMode:           return "displayListMode";
    case ID_stereoRendering:           return "stereoRendering";
    case ID_stereoType:                return "stereoType";
    case ID_notifyForEachRender:       return "notifyForEachRender";
    case ID_scalableActivationMode:    return "scalableActivationMode";
    case ID_scalableAutoThreshold:     return "scalableAutoThreshold";
    case ID_specularFlag:              return "specularFlag";
    case ID_specularCoeff:             return "specularCoeff";
    case ID_specularPower:             return "specularPower";
    case ID_specularColor:             return "specularColor";
    case ID_doShadowing:               return "doShadowing";
    case ID_shadowStrength:            return "shadowStrength";
    case ID_doDepthCueing:             return "doDepthCueing";
    case ID_startCuePoint:             return "startCuePoint";
    case ID_endCuePoint:               return "endCuePoint";
    case ID_compressionActivationMode: return "compressionActivationMode";
    case ID_colorTexturingFlag:        return "colorTexturingFlag";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: RenderingAttributes::GetFieldType
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
RenderingAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_antialiasing:              return FieldType_bool;
    case ID_geometryRepresentation:    return FieldType_enum;
    case ID_displayListMode:           return FieldType_enum;
    case ID_stereoRendering:           return FieldType_bool;
    case ID_stereoType:                return FieldType_enum;
    case ID_notifyForEachRender:       return FieldType_bool;
    case ID_scalableActivationMode:    return FieldType_enum;
    case ID_scalableAutoThreshold:     return FieldType_int;
    case ID_specularFlag:              return FieldType_bool;
    case ID_specularCoeff:             return FieldType_float;
    case ID_specularPower:             return FieldType_float;
    case ID_specularColor:             return FieldType_color;
    case ID_doShadowing:               return FieldType_bool;
    case ID_shadowStrength:            return FieldType_double;
    case ID_doDepthCueing:             return FieldType_bool;
    case ID_startCuePoint:             return FieldType_doubleArray;
    case ID_endCuePoint:               return FieldType_doubleArray;
    case ID_compressionActivationMode: return FieldType_enum;
    case ID_colorTexturingFlag:        return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: RenderingAttributes::GetFieldTypeName
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
RenderingAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_antialiasing:              return "bool";
    case ID_geometryRepresentation:    return "enum";
    case ID_displayListMode:           return "enum";
    case ID_stereoRendering:           return "bool";
    case ID_stereoType:                return "enum";
    case ID_notifyForEachRender:       return "bool";
    case ID_scalableActivationMode:    return "enum";
    case ID_scalableAutoThreshold:     return "int";
    case ID_specularFlag:              return "bool";
    case ID_specularCoeff:             return "float";
    case ID_specularPower:             return "float";
    case ID_specularColor:             return "color";
    case ID_doShadowing:               return "bool";
    case ID_shadowStrength:            return "double";
    case ID_doDepthCueing:             return "bool";
    case ID_startCuePoint:             return "doubleArray";
    case ID_endCuePoint:               return "doubleArray";
    case ID_compressionActivationMode: return "enum";
    case ID_colorTexturingFlag:        return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: RenderingAttributes::FieldsEqual
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
RenderingAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const RenderingAttributes &obj = *((const RenderingAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_antialiasing:
        {  // new scope
        retval = (antialiasing == obj.antialiasing);
        }
        break;
    case ID_geometryRepresentation:
        {  // new scope
        retval = (geometryRepresentation == obj.geometryRepresentation);
        }
        break;
    case ID_displayListMode:
        {  // new scope
        retval = (displayListMode == obj.displayListMode);
        }
        break;
    case ID_stereoRendering:
        {  // new scope
        retval = (stereoRendering == obj.stereoRendering);
        }
        break;
    case ID_stereoType:
        {  // new scope
        retval = (stereoType == obj.stereoType);
        }
        break;
    case ID_notifyForEachRender:
        {  // new scope
        retval = (notifyForEachRender == obj.notifyForEachRender);
        }
        break;
    case ID_scalableActivationMode:
        {  // new scope
        retval = (scalableActivationMode == obj.scalableActivationMode);
        }
        break;
    case ID_scalableAutoThreshold:
        {  // new scope
        retval = (scalableAutoThreshold == obj.scalableAutoThreshold);
        }
        break;
    case ID_specularFlag:
        {  // new scope
        retval = (specularFlag == obj.specularFlag);
        }
        break;
    case ID_specularCoeff:
        {  // new scope
        retval = (specularCoeff == obj.specularCoeff);
        }
        break;
    case ID_specularPower:
        {  // new scope
        retval = (specularPower == obj.specularPower);
        }
        break;
    case ID_specularColor:
        {  // new scope
        retval = (specularColor == obj.specularColor);
        }
        break;
    case ID_doShadowing:
        {  // new scope
        retval = (doShadowing == obj.doShadowing);
        }
        break;
    case ID_shadowStrength:
        {  // new scope
        retval = (shadowStrength == obj.shadowStrength);
        }
        break;
    case ID_doDepthCueing:
        {  // new scope
        retval = (doDepthCueing == obj.doDepthCueing);
        }
        break;
    case ID_startCuePoint:
        {  // new scope
        // Compare the startCuePoint arrays.
        bool startCuePoint_equal = true;
        for(int i = 0; i < 3 && startCuePoint_equal; ++i)
            startCuePoint_equal = (startCuePoint[i] == obj.startCuePoint[i]);

        retval = startCuePoint_equal;
        }
        break;
    case ID_endCuePoint:
        {  // new scope
        // Compare the endCuePoint arrays.
        bool endCuePoint_equal = true;
        for(int i = 0; i < 3 && endCuePoint_equal; ++i)
            endCuePoint_equal = (endCuePoint[i] == obj.endCuePoint[i]);

        retval = endCuePoint_equal;
        }
        break;
    case ID_compressionActivationMode:
        {  // new scope
        retval = (compressionActivationMode == obj.compressionActivationMode);
        }
        break;
    case ID_colorTexturingFlag:
        {  // new scope
        retval = (colorTexturingFlag == obj.colorTexturingFlag);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

int RenderingAttributes::GetEffectiveScalableThreshold(TriStateMode mode, int autoThreshold)
{
    if (mode == Never)
        return INT_MAX;
    else if (mode == Always)
        return 0;
    else if (mode == Auto)
        return autoThreshold;
    else
        return -1;
}

