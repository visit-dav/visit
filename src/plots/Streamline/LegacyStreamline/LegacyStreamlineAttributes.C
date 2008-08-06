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

#include <LegacyStreamlineAttributes.h>
#include <DataNode.h>
#include <PointAttributes.h>
#include <Line.h>
#include <PlaneAttributes.h>
#include <SphereAttributes.h>
#include <PointAttributes.h>
#include <BoxExtents.h>

//
// Enum conversion methods for LegacyStreamlineAttributes::SourceType
//

static const char *SourceType_strings[] = {
"SpecifiedPoint", "SpecifiedLine", "SpecifiedPlane", 
"SpecifiedSphere", "SpecifiedBox"};

std::string
LegacyStreamlineAttributes::SourceType_ToString(LegacyStreamlineAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return SourceType_strings[index];
}

std::string
LegacyStreamlineAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return SourceType_strings[index];
}

bool
LegacyStreamlineAttributes::SourceType_FromString(const std::string &s, LegacyStreamlineAttributes::SourceType &val)
{
    val = LegacyStreamlineAttributes::SpecifiedPoint;
    for(int i = 0; i < 5; ++i)
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
// Enum conversion methods for LegacyStreamlineAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"Solid", "ColorBySpeed", "ColorByVorticity"
};

std::string
LegacyStreamlineAttributes::ColoringMethod_ToString(LegacyStreamlineAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ColoringMethod_strings[index];
}

std::string
LegacyStreamlineAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
LegacyStreamlineAttributes::ColoringMethod_FromString(const std::string &s, LegacyStreamlineAttributes::ColoringMethod &val)
{
    val = LegacyStreamlineAttributes::Solid;
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
// Enum conversion methods for LegacyStreamlineAttributes::DisplayMethod
//

static const char *DisplayMethod_strings[] = {
"Lines", "Tubes", "Ribbons"
};

std::string
LegacyStreamlineAttributes::DisplayMethod_ToString(LegacyStreamlineAttributes::DisplayMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return DisplayMethod_strings[index];
}

std::string
LegacyStreamlineAttributes::DisplayMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return DisplayMethod_strings[index];
}

bool
LegacyStreamlineAttributes::DisplayMethod_FromString(const std::string &s, LegacyStreamlineAttributes::DisplayMethod &val)
{
    val = LegacyStreamlineAttributes::Lines;
    for(int i = 0; i < 3; ++i)
    {
        if(s == DisplayMethod_strings[i])
        {
            val = (DisplayMethod)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LegacyStreamlineAttributes::IntegrationDirection
//

static const char *IntegrationDirection_strings[] = {
"Forward", "Backward", "Both"
};

std::string
LegacyStreamlineAttributes::IntegrationDirection_ToString(LegacyStreamlineAttributes::IntegrationDirection t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return IntegrationDirection_strings[index];
}

std::string
LegacyStreamlineAttributes::IntegrationDirection_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return IntegrationDirection_strings[index];
}

bool
LegacyStreamlineAttributes::IntegrationDirection_FromString(const std::string &s, LegacyStreamlineAttributes::IntegrationDirection &val)
{
    val = LegacyStreamlineAttributes::Forward;
    for(int i = 0; i < 3; ++i)
    {
        if(s == IntegrationDirection_strings[i])
        {
            val = (IntegrationDirection)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *LegacyStreamlineAttributes::TypeMapFormatString = "iddDDDDDDdDdDbiibdiisabbi";

// ****************************************************************************
// Method: LegacyStreamlineAttributes::LegacyStreamlineAttributes
//
// Purpose: 
//   Constructor for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

LegacyStreamlineAttributes::LegacyStreamlineAttributes() : 
    AttributeSubject(LegacyStreamlineAttributes::TypeMapFormatString),
    colorTableName("Default"), singleColor(0, 0, 0)
{
    sourceType = SpecifiedPoint;
    stepLength = 1;
    maxTime = 10;
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
    sphereOrigin[0] = 0;
    sphereOrigin[1] = 0;
    sphereOrigin[2] = 0;
    sphereRadius = 1;
    boxExtents[0] = 0;
    boxExtents[1] = 1;
    boxExtents[2] = 0;
    boxExtents[3] = 1;
    boxExtents[4] = 0;
    boxExtents[5] = 1;
    useWholeBox = true;
    pointDensity = 2;
    displayMethod = Lines;
    showStart = true;
    radius = 0.125;
    lineWidth = 2;
    coloringMethod = ColorBySpeed;
    legendFlag = true;
    lightingFlag = true;
    StreamlineDirection = Forward;
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::LegacyStreamlineAttributes
//
// Purpose: 
//   Copy constructor for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

LegacyStreamlineAttributes::LegacyStreamlineAttributes(const LegacyStreamlineAttributes &obj) : 
    AttributeSubject(LegacyStreamlineAttributes::TypeMapFormatString)
{
    int i;

    sourceType = obj.sourceType;
    stepLength = obj.stepLength;
    maxTime = obj.maxTime;
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
    sphereOrigin[0] = obj.sphereOrigin[0];
    sphereOrigin[1] = obj.sphereOrigin[1];
    sphereOrigin[2] = obj.sphereOrigin[2];

    sphereRadius = obj.sphereRadius;
    for(i = 0; i < 6; ++i)
        boxExtents[i] = obj.boxExtents[i];

    useWholeBox = obj.useWholeBox;
    pointDensity = obj.pointDensity;
    displayMethod = obj.displayMethod;
    showStart = obj.showStart;
    radius = obj.radius;
    lineWidth = obj.lineWidth;
    coloringMethod = obj.coloringMethod;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    StreamlineDirection = obj.StreamlineDirection;

    SelectAll();
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::~LegacyStreamlineAttributes
//
// Purpose: 
//   Destructor for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

LegacyStreamlineAttributes::~LegacyStreamlineAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

LegacyStreamlineAttributes& 
LegacyStreamlineAttributes::operator = (const LegacyStreamlineAttributes &obj)
{
    if (this == &obj) return *this;
    int i;

    sourceType = obj.sourceType;
    stepLength = obj.stepLength;
    maxTime = obj.maxTime;
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
    sphereOrigin[0] = obj.sphereOrigin[0];
    sphereOrigin[1] = obj.sphereOrigin[1];
    sphereOrigin[2] = obj.sphereOrigin[2];

    sphereRadius = obj.sphereRadius;
    for(i = 0; i < 6; ++i)
        boxExtents[i] = obj.boxExtents[i];

    useWholeBox = obj.useWholeBox;
    pointDensity = obj.pointDensity;
    displayMethod = obj.displayMethod;
    showStart = obj.showStart;
    radius = obj.radius;
    lineWidth = obj.lineWidth;
    coloringMethod = obj.coloringMethod;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    StreamlineDirection = obj.StreamlineDirection;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
LegacyStreamlineAttributes::operator == (const LegacyStreamlineAttributes &obj) const
{
    int i;

    // Compare the pointSource arrays.
    bool pointSource_equal = true;
    for(i = 0; i < 3 && pointSource_equal; ++i)
        pointSource_equal = (pointSource[i] == obj.pointSource[i]);

    // Compare the lineStart arrays.
    bool lineStart_equal = true;
    for(i = 0; i < 3 && lineStart_equal; ++i)
        lineStart_equal = (lineStart[i] == obj.lineStart[i]);

    // Compare the lineEnd arrays.
    bool lineEnd_equal = true;
    for(i = 0; i < 3 && lineEnd_equal; ++i)
        lineEnd_equal = (lineEnd[i] == obj.lineEnd[i]);

    // Compare the planeOrigin arrays.
    bool planeOrigin_equal = true;
    for(i = 0; i < 3 && planeOrigin_equal; ++i)
        planeOrigin_equal = (planeOrigin[i] == obj.planeOrigin[i]);

    // Compare the planeNormal arrays.
    bool planeNormal_equal = true;
    for(i = 0; i < 3 && planeNormal_equal; ++i)
        planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

    // Compare the planeUpAxis arrays.
    bool planeUpAxis_equal = true;
    for(i = 0; i < 3 && planeUpAxis_equal; ++i)
        planeUpAxis_equal = (planeUpAxis[i] == obj.planeUpAxis[i]);

    // Compare the sphereOrigin arrays.
    bool sphereOrigin_equal = true;
    for(i = 0; i < 3 && sphereOrigin_equal; ++i)
        sphereOrigin_equal = (sphereOrigin[i] == obj.sphereOrigin[i]);

    // Compare the boxExtents arrays.
    bool boxExtents_equal = true;
    for(i = 0; i < 6 && boxExtents_equal; ++i)
        boxExtents_equal = (boxExtents[i] == obj.boxExtents[i]);

    // Create the return value
    return ((sourceType == obj.sourceType) &&
            (stepLength == obj.stepLength) &&
            (maxTime == obj.maxTime) &&
            pointSource_equal &&
            lineStart_equal &&
            lineEnd_equal &&
            planeOrigin_equal &&
            planeNormal_equal &&
            planeUpAxis_equal &&
            (planeRadius == obj.planeRadius) &&
            sphereOrigin_equal &&
            (sphereRadius == obj.sphereRadius) &&
            boxExtents_equal &&
            (useWholeBox == obj.useWholeBox) &&
            (pointDensity == obj.pointDensity) &&
            (displayMethod == obj.displayMethod) &&
            (showStart == obj.showStart) &&
            (radius == obj.radius) &&
            (lineWidth == obj.lineWidth) &&
            (coloringMethod == obj.coloringMethod) &&
            (colorTableName == obj.colorTableName) &&
            (singleColor == obj.singleColor) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (StreamlineDirection == obj.StreamlineDirection));
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
LegacyStreamlineAttributes::operator != (const LegacyStreamlineAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::TypeName
//
// Purpose: 
//   Type name method for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

const std::string
LegacyStreamlineAttributes::TypeName() const
{
    return "LegacyStreamlineAttributes";
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the LegacyStreamlineAttributes class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 4 15:22:57 PST 2002
//
// Modifications:
//    Brad Whitlock, Wed Dec 22 12:57:53 PDT 2004
//    I added code to support the point tool.
//
//    Hank Childs, Sat Mar  3 09:00:12 PST 2007
//    Disable useWholeBox if we are copying box extents.
//
// ****************************************************************************

bool
LegacyStreamlineAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const LegacyStreamlineAttributes *tmp = (const LegacyStreamlineAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    else if(atts->TypeName() == "PointAttributes")
    {
        if(sourceType == SpecifiedPoint)
        {
            const PointAttributes *p = (PointAttributes *)atts;
            SetPointSource(p->GetPoint());
            retval = true;
        }
    } 
    else if(atts->TypeName() == "Line")
    {
        if(sourceType == SpecifiedLine)
        {
            const Line *line = (const Line *)atts;
            SetLineStart(line->GetPoint1());
            SetLineEnd(line->GetPoint2());
            retval = true;
        }
    }
    else if(atts->TypeName() == "PlaneAttributes")
    {
        if(sourceType == SpecifiedPlane)
        {
            const PlaneAttributes *plane = (const PlaneAttributes *)atts;
            SetPlaneOrigin(plane->GetOrigin());
            SetPlaneNormal(plane->GetNormal());
            SetPlaneUpAxis(plane->GetUpAxis());
            SetPlaneRadius(plane->GetRadius());
            retval = true;
        }
    }
    else if(atts->TypeName() == "SphereAttributes")
    {
        if(sourceType == SpecifiedSphere)
        {
            const SphereAttributes *sphere = (const SphereAttributes *)atts;
            SetSphereOrigin(sphere->GetOrigin());
            SetSphereRadius(sphere->GetRadius());
            retval = true;
        }
    }   
    else if(atts->TypeName() == "BoxExtents")
    {
        if(sourceType == SpecifiedBox)
        {
            const BoxExtents *box = (const BoxExtents *)atts;
            SetBoxExtents(box->GetExtents());
            SetUseWholeBox(false);
            retval = true;
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: LegacyStreamlineAttributes::CreateCompatible
//
//  Purpose:
//     Creates a new state object of the desired type.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Modifications:
//    Brad Whitlock, Tue Jan 21 12:33:04 PDT 2003
//    I added code to set the "have radius" flag to true so the plane tool
//    resizes properly when resizing the plane radius.
//
//    Brad Whitlock, Wed Dec 22 12:54:43 PDT 2004
//    I added code to support the point tool.
//
// ****************************************************************************

AttributeSubject *
LegacyStreamlineAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;

    if(TypeName() == tname)
    {
        retval = new LegacyStreamlineAttributes(*this);
    }
    else if(tname == "PointAttributes")
    {
        PointAttributes *p = new PointAttributes;
        p->SetPoint(GetPointSource());
        retval = p;
    }
    else if(tname == "Line")
    {
        Line *l = new Line;
        l->SetPoint1(GetLineStart());
        l->SetPoint2(GetLineEnd());
        retval = l;
    }
    else if(tname == "PlaneAttributes")
    {
        PlaneAttributes *p = new PlaneAttributes;
        p->SetOrigin(GetPlaneOrigin());
        p->SetNormal(GetPlaneNormal());
        p->SetUpAxis(GetPlaneUpAxis());
        p->SetRadius(GetPlaneRadius());
        p->SetHaveRadius(true);
        retval = p;
    }
    else if(tname == "SphereAttributes")
    {
        SphereAttributes *s = new SphereAttributes;
        s->SetOrigin(GetSphereOrigin());
        s->SetRadius(GetSphereRadius());
        retval = s;
    }
    else if(tname == "BoxExtents")
    {
        BoxExtents *b = new BoxExtents;
        b->SetExtents(GetBoxExtents());
        retval = b;
    }

    return retval;
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the LegacyStreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

AttributeSubject *
LegacyStreamlineAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new LegacyStreamlineAttributes(*this);
    else
        retval = new LegacyStreamlineAttributes;

    return retval;
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::SelectAll
//
// Purpose: 
//   Selects all attributes.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
LegacyStreamlineAttributes::SelectAll()
{
    Select(ID_sourceType,          (void *)&sourceType);
    Select(ID_stepLength,          (void *)&stepLength);
    Select(ID_maxTime,             (void *)&maxTime);
    Select(ID_pointSource,         (void *)pointSource, 3);
    Select(ID_lineStart,           (void *)lineStart, 3);
    Select(ID_lineEnd,             (void *)lineEnd, 3);
    Select(ID_planeOrigin,         (void *)planeOrigin, 3);
    Select(ID_planeNormal,         (void *)planeNormal, 3);
    Select(ID_planeUpAxis,         (void *)planeUpAxis, 3);
    Select(ID_planeRadius,         (void *)&planeRadius);
    Select(ID_sphereOrigin,        (void *)sphereOrigin, 3);
    Select(ID_sphereRadius,        (void *)&sphereRadius);
    Select(ID_boxExtents,          (void *)boxExtents, 6);
    Select(ID_useWholeBox,         (void *)&useWholeBox);
    Select(ID_pointDensity,        (void *)&pointDensity);
    Select(ID_displayMethod,       (void *)&displayMethod);
    Select(ID_showStart,           (void *)&showStart);
    Select(ID_radius,              (void *)&radius);
    Select(ID_lineWidth,           (void *)&lineWidth);
    Select(ID_coloringMethod,      (void *)&coloringMethod);
    Select(ID_colorTableName,      (void *)&colorTableName);
    Select(ID_singleColor,         (void *)&singleColor);
    Select(ID_legendFlag,          (void *)&legendFlag);
    Select(ID_lightingFlag,        (void *)&lightingFlag);
    Select(ID_StreamlineDirection, (void *)&StreamlineDirection);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LegacyStreamlineAttributes::CreateNode
//
// Purpose: 
//   This method creates a DataNode representation of the object so it can be saved to a config file.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
LegacyStreamlineAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    LegacyStreamlineAttributes defaultObject;
    bool addToParent = false;
    // Create a node for LegacyStreamlineAttributes.
    DataNode *node = new DataNode("LegacyStreamlineAttributes");

    if(completeSave || !FieldsEqual(ID_sourceType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sourceType", SourceType_ToString(sourceType)));
    }

    if(completeSave || !FieldsEqual(ID_stepLength, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("stepLength", stepLength));
    }

    if(completeSave || !FieldsEqual(ID_maxTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxTime", maxTime));
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

    if(completeSave || !FieldsEqual(ID_sphereOrigin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sphereOrigin", sphereOrigin, 3));
    }

    if(completeSave || !FieldsEqual(ID_sphereRadius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sphereRadius", sphereRadius));
    }

    if(completeSave || !FieldsEqual(ID_boxExtents, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("boxExtents", boxExtents, 6));
    }

    if(completeSave || !FieldsEqual(ID_useWholeBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useWholeBox", useWholeBox));
    }

    if(completeSave || !FieldsEqual(ID_pointDensity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointDensity", pointDensity));
    }

    if(completeSave || !FieldsEqual(ID_displayMethod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayMethod", DisplayMethod_ToString(displayMethod)));
    }

    if(completeSave || !FieldsEqual(ID_showStart, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showStart", showStart));
    }

    if(completeSave || !FieldsEqual(ID_radius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("radius", radius));
    }

    if(completeSave || !FieldsEqual(ID_lineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineWidth", lineWidth));
    }

    if(completeSave || !FieldsEqual(ID_coloringMethod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("coloringMethod", ColoringMethod_ToString(coloringMethod)));
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

    if(completeSave || !FieldsEqual(ID_StreamlineDirection, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("StreamlineDirection", IntegrationDirection_ToString(StreamlineDirection)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::SetFromNode
//
// Purpose: 
//   This method sets attributes in this object from values in a DataNode representation of the object.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
LegacyStreamlineAttributes::SetFromNode(DataNode *parentNode)
{
    int i;
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("LegacyStreamlineAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("sourceType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetSourceType(SourceType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SourceType value;
            if(SourceType_FromString(node->AsString(), value))
                SetSourceType(value);
        }
    }
    if((node = searchNode->GetNode("stepLength")) != 0)
        SetStepLength(node->AsDouble());
    if((node = searchNode->GetNode("maxTime")) != 0)
        SetMaxTime(node->AsDouble());
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
    if((node = searchNode->GetNode("sphereOrigin")) != 0)
        SetSphereOrigin(node->AsDoubleArray());
    if((node = searchNode->GetNode("sphereRadius")) != 0)
        SetSphereRadius(node->AsDouble());
    if((node = searchNode->GetNode("boxExtents")) != 0)
        SetBoxExtents(node->AsDoubleArray());
    if((node = searchNode->GetNode("useWholeBox")) != 0)
        SetUseWholeBox(node->AsBool());
    if((node = searchNode->GetNode("pointDensity")) != 0)
        SetPointDensity(node->AsInt());
    if((node = searchNode->GetNode("displayMethod")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDisplayMethod(DisplayMethod(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DisplayMethod value;
            if(DisplayMethod_FromString(node->AsString(), value))
                SetDisplayMethod(value);
        }
    }
    if((node = searchNode->GetNode("showStart")) != 0)
        SetShowStart(node->AsBool());
    if((node = searchNode->GetNode("radius")) != 0)
        SetRadius(node->AsDouble());
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("coloringMethod")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetColoringMethod(ColoringMethod(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ColoringMethod value;
            if(ColoringMethod_FromString(node->AsString(), value))
                SetColoringMethod(value);
        }
    }
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("StreamlineDirection")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetStreamlineDirection(IntegrationDirection(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationDirection value;
            if(IntegrationDirection_FromString(node->AsString(), value))
                SetStreamlineDirection(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
LegacyStreamlineAttributes::SetSourceType(LegacyStreamlineAttributes::SourceType sourceType_)
{
    sourceType = sourceType_;
    Select(ID_sourceType, (void *)&sourceType);
}

void
LegacyStreamlineAttributes::SetStepLength(double stepLength_)
{
    stepLength = stepLength_;
    Select(ID_stepLength, (void *)&stepLength);
}

void
LegacyStreamlineAttributes::SetMaxTime(double maxTime_)
{
    maxTime = maxTime_;
    Select(ID_maxTime, (void *)&maxTime);
}

void
LegacyStreamlineAttributes::SetPointSource(const double *pointSource_)
{
    pointSource[0] = pointSource_[0];
    pointSource[1] = pointSource_[1];
    pointSource[2] = pointSource_[2];
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
LegacyStreamlineAttributes::SetLineStart(const double *lineStart_)
{
    lineStart[0] = lineStart_[0];
    lineStart[1] = lineStart_[1];
    lineStart[2] = lineStart_[2];
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
LegacyStreamlineAttributes::SetLineEnd(const double *lineEnd_)
{
    lineEnd[0] = lineEnd_[0];
    lineEnd[1] = lineEnd_[1];
    lineEnd[2] = lineEnd_[2];
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
LegacyStreamlineAttributes::SetPlaneOrigin(const double *planeOrigin_)
{
    planeOrigin[0] = planeOrigin_[0];
    planeOrigin[1] = planeOrigin_[1];
    planeOrigin[2] = planeOrigin_[2];
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
LegacyStreamlineAttributes::SetPlaneNormal(const double *planeNormal_)
{
    planeNormal[0] = planeNormal_[0];
    planeNormal[1] = planeNormal_[1];
    planeNormal[2] = planeNormal_[2];
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
LegacyStreamlineAttributes::SetPlaneUpAxis(const double *planeUpAxis_)
{
    planeUpAxis[0] = planeUpAxis_[0];
    planeUpAxis[1] = planeUpAxis_[1];
    planeUpAxis[2] = planeUpAxis_[2];
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
LegacyStreamlineAttributes::SetPlaneRadius(double planeRadius_)
{
    planeRadius = planeRadius_;
    Select(ID_planeRadius, (void *)&planeRadius);
}

void
LegacyStreamlineAttributes::SetSphereOrigin(const double *sphereOrigin_)
{
    sphereOrigin[0] = sphereOrigin_[0];
    sphereOrigin[1] = sphereOrigin_[1];
    sphereOrigin[2] = sphereOrigin_[2];
    Select(ID_sphereOrigin, (void *)sphereOrigin, 3);
}

void
LegacyStreamlineAttributes::SetSphereRadius(double sphereRadius_)
{
    sphereRadius = sphereRadius_;
    Select(ID_sphereRadius, (void *)&sphereRadius);
}

void
LegacyStreamlineAttributes::SetBoxExtents(const double *boxExtents_)
{
    for(int i = 0; i < 6; ++i)
        boxExtents[i] = boxExtents_[i];
    Select(ID_boxExtents, (void *)boxExtents, 6);
}

void
LegacyStreamlineAttributes::SetUseWholeBox(bool useWholeBox_)
{
    useWholeBox = useWholeBox_;
    Select(ID_useWholeBox, (void *)&useWholeBox);
}

void
LegacyStreamlineAttributes::SetPointDensity(int pointDensity_)
{
    pointDensity = pointDensity_;
    Select(ID_pointDensity, (void *)&pointDensity);
}

void
LegacyStreamlineAttributes::SetDisplayMethod(LegacyStreamlineAttributes::DisplayMethod displayMethod_)
{
    displayMethod = displayMethod_;
    Select(ID_displayMethod, (void *)&displayMethod);
}

void
LegacyStreamlineAttributes::SetShowStart(bool showStart_)
{
    showStart = showStart_;
    Select(ID_showStart, (void *)&showStart);
}

void
LegacyStreamlineAttributes::SetRadius(double radius_)
{
    radius = radius_;
    Select(ID_radius, (void *)&radius);
}

void
LegacyStreamlineAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
LegacyStreamlineAttributes::SetColoringMethod(LegacyStreamlineAttributes::ColoringMethod coloringMethod_)
{
    coloringMethod = coloringMethod_;
    Select(ID_coloringMethod, (void *)&coloringMethod);
}

void
LegacyStreamlineAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
LegacyStreamlineAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
LegacyStreamlineAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
LegacyStreamlineAttributes::SetLightingFlag(bool lightingFlag_)
{
    lightingFlag = lightingFlag_;
    Select(ID_lightingFlag, (void *)&lightingFlag);
}

void
LegacyStreamlineAttributes::SetStreamlineDirection(LegacyStreamlineAttributes::IntegrationDirection StreamlineDirection_)
{
    StreamlineDirection = StreamlineDirection_;
    Select(ID_StreamlineDirection, (void *)&StreamlineDirection);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

LegacyStreamlineAttributes::SourceType
LegacyStreamlineAttributes::GetSourceType() const
{
    return SourceType(sourceType);
}

double
LegacyStreamlineAttributes::GetStepLength() const
{
    return stepLength;
}

double
LegacyStreamlineAttributes::GetMaxTime() const
{
    return maxTime;
}

const double *
LegacyStreamlineAttributes::GetPointSource() const
{
    return pointSource;
}

double *
LegacyStreamlineAttributes::GetPointSource()
{
    return pointSource;
}

const double *
LegacyStreamlineAttributes::GetLineStart() const
{
    return lineStart;
}

double *
LegacyStreamlineAttributes::GetLineStart()
{
    return lineStart;
}

const double *
LegacyStreamlineAttributes::GetLineEnd() const
{
    return lineEnd;
}

double *
LegacyStreamlineAttributes::GetLineEnd()
{
    return lineEnd;
}

const double *
LegacyStreamlineAttributes::GetPlaneOrigin() const
{
    return planeOrigin;
}

double *
LegacyStreamlineAttributes::GetPlaneOrigin()
{
    return planeOrigin;
}

const double *
LegacyStreamlineAttributes::GetPlaneNormal() const
{
    return planeNormal;
}

double *
LegacyStreamlineAttributes::GetPlaneNormal()
{
    return planeNormal;
}

const double *
LegacyStreamlineAttributes::GetPlaneUpAxis() const
{
    return planeUpAxis;
}

double *
LegacyStreamlineAttributes::GetPlaneUpAxis()
{
    return planeUpAxis;
}

double
LegacyStreamlineAttributes::GetPlaneRadius() const
{
    return planeRadius;
}

const double *
LegacyStreamlineAttributes::GetSphereOrigin() const
{
    return sphereOrigin;
}

double *
LegacyStreamlineAttributes::GetSphereOrigin()
{
    return sphereOrigin;
}

double
LegacyStreamlineAttributes::GetSphereRadius() const
{
    return sphereRadius;
}

const double *
LegacyStreamlineAttributes::GetBoxExtents() const
{
    return boxExtents;
}

double *
LegacyStreamlineAttributes::GetBoxExtents()
{
    return boxExtents;
}

bool
LegacyStreamlineAttributes::GetUseWholeBox() const
{
    return useWholeBox;
}

int
LegacyStreamlineAttributes::GetPointDensity() const
{
    return pointDensity;
}

LegacyStreamlineAttributes::DisplayMethod
LegacyStreamlineAttributes::GetDisplayMethod() const
{
    return DisplayMethod(displayMethod);
}

bool
LegacyStreamlineAttributes::GetShowStart() const
{
    return showStart;
}

double
LegacyStreamlineAttributes::GetRadius() const
{
    return radius;
}

int
LegacyStreamlineAttributes::GetLineWidth() const
{
    return lineWidth;
}

LegacyStreamlineAttributes::ColoringMethod
LegacyStreamlineAttributes::GetColoringMethod() const
{
    return ColoringMethod(coloringMethod);
}

const std::string &
LegacyStreamlineAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
LegacyStreamlineAttributes::GetColorTableName()
{
    return colorTableName;
}

const ColorAttribute &
LegacyStreamlineAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
LegacyStreamlineAttributes::GetSingleColor()
{
    return singleColor;
}

bool
LegacyStreamlineAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
LegacyStreamlineAttributes::GetLightingFlag() const
{
    return lightingFlag;
}

LegacyStreamlineAttributes::IntegrationDirection
LegacyStreamlineAttributes::GetStreamlineDirection() const
{
    return IntegrationDirection(StreamlineDirection);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
LegacyStreamlineAttributes::SelectPointSource()
{
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
LegacyStreamlineAttributes::SelectLineStart()
{
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
LegacyStreamlineAttributes::SelectLineEnd()
{
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
LegacyStreamlineAttributes::SelectPlaneOrigin()
{
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
LegacyStreamlineAttributes::SelectPlaneNormal()
{
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
LegacyStreamlineAttributes::SelectPlaneUpAxis()
{
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
LegacyStreamlineAttributes::SelectSphereOrigin()
{
    Select(ID_sphereOrigin, (void *)sphereOrigin, 3);
}

void
LegacyStreamlineAttributes::SelectBoxExtents()
{
    Select(ID_boxExtents, (void *)boxExtents, 6);
}

void
LegacyStreamlineAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
LegacyStreamlineAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LegacyStreamlineAttributes::GetFieldName
//
// Purpose: 
//   This method returns the name of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
LegacyStreamlineAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_sourceType:          return "sourceType";
    case ID_stepLength:          return "stepLength";
    case ID_maxTime:             return "maxTime";
    case ID_pointSource:         return "pointSource";
    case ID_lineStart:           return "lineStart";
    case ID_lineEnd:             return "lineEnd";
    case ID_planeOrigin:         return "planeOrigin";
    case ID_planeNormal:         return "planeNormal";
    case ID_planeUpAxis:         return "planeUpAxis";
    case ID_planeRadius:         return "planeRadius";
    case ID_sphereOrigin:        return "sphereOrigin";
    case ID_sphereRadius:        return "sphereRadius";
    case ID_boxExtents:          return "boxExtents";
    case ID_useWholeBox:         return "useWholeBox";
    case ID_pointDensity:        return "pointDensity";
    case ID_displayMethod:       return "displayMethod";
    case ID_showStart:           return "showStart";
    case ID_radius:              return "radius";
    case ID_lineWidth:           return "lineWidth";
    case ID_coloringMethod:      return "coloringMethod";
    case ID_colorTableName:      return "colorTableName";
    case ID_singleColor:         return "singleColor";
    case ID_legendFlag:          return "legendFlag";
    case ID_lightingFlag:        return "lightingFlag";
    case ID_StreamlineDirection: return "StreamlineDirection";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::GetFieldType
//
// Purpose: 
//   This method returns the type of a field given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup::FieldType
LegacyStreamlineAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_sourceType:          return FieldType_enum;
    case ID_stepLength:          return FieldType_double;
    case ID_maxTime:             return FieldType_double;
    case ID_pointSource:         return FieldType_doubleArray;
    case ID_lineStart:           return FieldType_doubleArray;
    case ID_lineEnd:             return FieldType_doubleArray;
    case ID_planeOrigin:         return FieldType_doubleArray;
    case ID_planeNormal:         return FieldType_doubleArray;
    case ID_planeUpAxis:         return FieldType_doubleArray;
    case ID_planeRadius:         return FieldType_double;
    case ID_sphereOrigin:        return FieldType_doubleArray;
    case ID_sphereRadius:        return FieldType_double;
    case ID_boxExtents:          return FieldType_doubleArray;
    case ID_useWholeBox:         return FieldType_bool;
    case ID_pointDensity:        return FieldType_int;
    case ID_displayMethod:       return FieldType_enum;
    case ID_showStart:           return FieldType_bool;
    case ID_radius:              return FieldType_double;
    case ID_lineWidth:           return FieldType_linewidth;
    case ID_coloringMethod:      return FieldType_enum;
    case ID_colorTableName:      return FieldType_colortable;
    case ID_singleColor:         return FieldType_color;
    case ID_legendFlag:          return FieldType_bool;
    case ID_lightingFlag:        return FieldType_bool;
    case ID_StreamlineDirection: return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::GetFieldTypeName
//
// Purpose: 
//   This method returns the name of a field type given its index.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
LegacyStreamlineAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_sourceType:          return "enum";
    case ID_stepLength:          return "double";
    case ID_maxTime:             return "double";
    case ID_pointSource:         return "doubleArray";
    case ID_lineStart:           return "doubleArray";
    case ID_lineEnd:             return "doubleArray";
    case ID_planeOrigin:         return "doubleArray";
    case ID_planeNormal:         return "doubleArray";
    case ID_planeUpAxis:         return "doubleArray";
    case ID_planeRadius:         return "double";
    case ID_sphereOrigin:        return "doubleArray";
    case ID_sphereRadius:        return "double";
    case ID_boxExtents:          return "doubleArray";
    case ID_useWholeBox:         return "bool";
    case ID_pointDensity:        return "int";
    case ID_displayMethod:       return "enum";
    case ID_showStart:           return "bool";
    case ID_radius:              return "double";
    case ID_lineWidth:           return "linewidth";
    case ID_coloringMethod:      return "enum";
    case ID_colorTableName:      return "colortable";
    case ID_singleColor:         return "color";
    case ID_legendFlag:          return "bool";
    case ID_lightingFlag:        return "bool";
    case ID_StreamlineDirection: return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LegacyStreamlineAttributes::FieldsEqual
//
// Purpose: 
//   This method compares two fields and return true if they are equal.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   Tue Feb 5 15:15:22 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
LegacyStreamlineAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    int i;

    const LegacyStreamlineAttributes &obj = *((const LegacyStreamlineAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_sourceType:
        {  // new scope
        retval = (sourceType == obj.sourceType);
        }
        break;
    case ID_stepLength:
        {  // new scope
        retval = (stepLength == obj.stepLength);
        }
        break;
    case ID_maxTime:
        {  // new scope
        retval = (maxTime == obj.maxTime);
        }
        break;
    case ID_pointSource:
        {  // new scope
        // Compare the pointSource arrays.
        bool pointSource_equal = true;
        for(i = 0; i < 3 && pointSource_equal; ++i)
            pointSource_equal = (pointSource[i] == obj.pointSource[i]);

        retval = pointSource_equal;
        }
        break;
    case ID_lineStart:
        {  // new scope
        // Compare the lineStart arrays.
        bool lineStart_equal = true;
        for(i = 0; i < 3 && lineStart_equal; ++i)
            lineStart_equal = (lineStart[i] == obj.lineStart[i]);

        retval = lineStart_equal;
        }
        break;
    case ID_lineEnd:
        {  // new scope
        // Compare the lineEnd arrays.
        bool lineEnd_equal = true;
        for(i = 0; i < 3 && lineEnd_equal; ++i)
            lineEnd_equal = (lineEnd[i] == obj.lineEnd[i]);

        retval = lineEnd_equal;
        }
        break;
    case ID_planeOrigin:
        {  // new scope
        // Compare the planeOrigin arrays.
        bool planeOrigin_equal = true;
        for(i = 0; i < 3 && planeOrigin_equal; ++i)
            planeOrigin_equal = (planeOrigin[i] == obj.planeOrigin[i]);

        retval = planeOrigin_equal;
        }
        break;
    case ID_planeNormal:
        {  // new scope
        // Compare the planeNormal arrays.
        bool planeNormal_equal = true;
        for(i = 0; i < 3 && planeNormal_equal; ++i)
            planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

        retval = planeNormal_equal;
        }
        break;
    case ID_planeUpAxis:
        {  // new scope
        // Compare the planeUpAxis arrays.
        bool planeUpAxis_equal = true;
        for(i = 0; i < 3 && planeUpAxis_equal; ++i)
            planeUpAxis_equal = (planeUpAxis[i] == obj.planeUpAxis[i]);

        retval = planeUpAxis_equal;
        }
        break;
    case ID_planeRadius:
        {  // new scope
        retval = (planeRadius == obj.planeRadius);
        }
        break;
    case ID_sphereOrigin:
        {  // new scope
        // Compare the sphereOrigin arrays.
        bool sphereOrigin_equal = true;
        for(i = 0; i < 3 && sphereOrigin_equal; ++i)
            sphereOrigin_equal = (sphereOrigin[i] == obj.sphereOrigin[i]);

        retval = sphereOrigin_equal;
        }
        break;
    case ID_sphereRadius:
        {  // new scope
        retval = (sphereRadius == obj.sphereRadius);
        }
        break;
    case ID_boxExtents:
        {  // new scope
        // Compare the boxExtents arrays.
        bool boxExtents_equal = true;
        for(i = 0; i < 6 && boxExtents_equal; ++i)
            boxExtents_equal = (boxExtents[i] == obj.boxExtents[i]);

        retval = boxExtents_equal;
        }
        break;
    case ID_useWholeBox:
        {  // new scope
        retval = (useWholeBox == obj.useWholeBox);
        }
        break;
    case ID_pointDensity:
        {  // new scope
        retval = (pointDensity == obj.pointDensity);
        }
        break;
    case ID_displayMethod:
        {  // new scope
        retval = (displayMethod == obj.displayMethod);
        }
        break;
    case ID_showStart:
        {  // new scope
        retval = (showStart == obj.showStart);
        }
        break;
    case ID_radius:
        {  // new scope
        retval = (radius == obj.radius);
        }
        break;
    case ID_lineWidth:
        {  // new scope
        retval = (lineWidth == obj.lineWidth);
        }
        break;
    case ID_coloringMethod:
        {  // new scope
        retval = (coloringMethod == obj.coloringMethod);
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
    case ID_StreamlineDirection:
        {  // new scope
        retval = (StreamlineDirection == obj.StreamlineDirection);
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
//  Method: LegacyStreamlineAttributes::ChangesRequireRecalculation
//
//  Purpose:
//     Determines whether or not the plot must be recalculated based on the
//     new attributes.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 4 15:22:57 PST 2002
//
//  Notes:  Most attributes cause the streamline to change.
//
//  Modifications:
//    Brad Whitlock, Wed Dec 22 12:52:45 PDT 2004
//    I made the coloring method matter when comparing streamline attributes
//    and I added support for ribbons.
//
//    Hank Childs, Sat Mar  3 09:00:12 PST 2007
//    Add support for useWholeBox.
//
// ****************************************************************************

#define PDIF(p1,p2,i) ((p1)[i] != (p2)[i])
#define POINT_DIFFERS(p1,p2) (PDIF(p1,p2,0) || PDIF(p1,p2,1) || PDIF(p1,p2,2))

bool
LegacyStreamlineAttributes::ChangesRequireRecalculation(const LegacyStreamlineAttributes &obj) const
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

    // If we're in plane source mode and the plane differs, sourcePlaneDiffers
    // evaluates to true.
    bool sourcePlaneDiffers = ((sourceType == SpecifiedPlane) &&
       (POINT_DIFFERS(planeOrigin, obj.planeOrigin) ||
        POINT_DIFFERS(planeNormal, obj.planeNormal) ||
        POINT_DIFFERS(planeUpAxis, obj.planeUpAxis) ||
        planeRadius != obj.planeRadius));

    // If we're in sphere source mode and the sphere differs, sourceSphereDiffers
    // evaluates to true.
    bool sourceSphereDiffers = ((sourceType == SpecifiedSphere) &&
       (POINT_DIFFERS(sphereOrigin, obj.sphereOrigin) ||
        (sphereRadius != obj.sphereRadius)));

    // If we're in box source mode and the box differs, boxSourceDiffers
    // evaluates to true.
    bool boxSourceDiffers = (sourceType == SpecifiedBox) &&
        (POINT_DIFFERS(boxExtents, obj.boxExtents) ||
         POINT_DIFFERS(boxExtents+3, obj.boxExtents+3));
    if (useWholeBox != obj.useWholeBox)
        boxSourceDiffers = true;

    // Other things need to be true before we start paying attention to
    // point density.
    bool densityMatters = (sourceType == SpecifiedLine ||
        sourceType == SpecifiedPlane || sourceType == SpecifiedSphere ||
        sourceType == SpecifiedBox) &&
        (pointDensity != obj.pointDensity);

    bool radiusMatters = (displayMethod != Lines) && (radius != obj.radius);

    return (sourceType != obj.sourceType) ||
           (StreamlineDirection != obj.StreamlineDirection) ||
           (displayMethod != obj.displayMethod) ||
           (showStart != obj.showStart) ||
           (maxTime != obj.maxTime) ||
           (stepLength != obj.stepLength) ||
           (coloringMethod != obj.coloringMethod && obj.coloringMethod != Solid) ||
           sourcePointsDiffer ||
           sourceLineDiffers ||
           sourcePlaneDiffers ||
           sourceSphereDiffers ||
           boxSourceDiffers ||
           densityMatters ||
           radiusMatters;
}

