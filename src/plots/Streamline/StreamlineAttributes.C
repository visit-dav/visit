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

#include <StreamlineAttributes.h>
#include <DataNode.h>
#include <PointAttributes.h>
#include <Line.h>
#include <PlaneAttributes.h>
#include <SphereAttributes.h>
#include <PointAttributes.h>
#include <BoxExtents.h>

//
// Enum conversion methods for StreamlineAttributes::SourceType
//

static const char *SourceType_strings[] = {
"SpecifiedPoint", "SpecifiedLine", "SpecifiedPlane", 
"SpecifiedSphere", "SpecifiedBox", "SpecifiedPointList"
};

std::string
StreamlineAttributes::SourceType_ToString(StreamlineAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 6) index = 0;
    return SourceType_strings[index];
}

std::string
StreamlineAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 6) ? 0 : t;
    return SourceType_strings[index];
}

bool
StreamlineAttributes::SourceType_FromString(const std::string &s, StreamlineAttributes::SourceType &val)
{
    val = StreamlineAttributes::SpecifiedPoint;
    for(int i = 0; i < 6; ++i)
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
// Enum conversion methods for StreamlineAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"Solid", "ColorBySpeed", "ColorByVorticity", 
"ColorByLength", "ColorByTime", "ColorBySeedPointID", 
"ColorByVariable"};

std::string
StreamlineAttributes::ColoringMethod_ToString(StreamlineAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 7) index = 0;
    return ColoringMethod_strings[index];
}

std::string
StreamlineAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 7) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
StreamlineAttributes::ColoringMethod_FromString(const std::string &s, StreamlineAttributes::ColoringMethod &val)
{
    val = StreamlineAttributes::Solid;
    for(int i = 0; i < 7; ++i)
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
// Enum conversion methods for StreamlineAttributes::DisplayMethod
//

static const char *DisplayMethod_strings[] = {
"Lines", "Tubes", "Ribbons"
};

std::string
StreamlineAttributes::DisplayMethod_ToString(StreamlineAttributes::DisplayMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return DisplayMethod_strings[index];
}

std::string
StreamlineAttributes::DisplayMethod_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return DisplayMethod_strings[index];
}

bool
StreamlineAttributes::DisplayMethod_FromString(const std::string &s, StreamlineAttributes::DisplayMethod &val)
{
    val = StreamlineAttributes::Lines;
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
// Enum conversion methods for StreamlineAttributes::IntegrationDirection
//

static const char *IntegrationDirection_strings[] = {
"Forward", "Backward", "Both"
};

std::string
StreamlineAttributes::IntegrationDirection_ToString(StreamlineAttributes::IntegrationDirection t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return IntegrationDirection_strings[index];
}

std::string
StreamlineAttributes::IntegrationDirection_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return IntegrationDirection_strings[index];
}

bool
StreamlineAttributes::IntegrationDirection_FromString(const std::string &s, StreamlineAttributes::IntegrationDirection &val)
{
    val = StreamlineAttributes::Forward;
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

//
// Enum conversion methods for StreamlineAttributes::TerminationType
//

static const char *TerminationType_strings[] = {
"Distance", "Time", "Step"
};

std::string
StreamlineAttributes::TerminationType_ToString(StreamlineAttributes::TerminationType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return TerminationType_strings[index];
}

std::string
StreamlineAttributes::TerminationType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return TerminationType_strings[index];
}

bool
StreamlineAttributes::TerminationType_FromString(const std::string &s, StreamlineAttributes::TerminationType &val)
{
    val = StreamlineAttributes::Distance;
    for(int i = 0; i < 3; ++i)
    {
        if(s == TerminationType_strings[i])
        {
            val = (TerminationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::StreamlineAlgorithmType
//

static const char *StreamlineAlgorithmType_strings[] = {
"LoadOnDemand", "ParallelStaticDomains", "MasterSlave"
};

std::string
StreamlineAttributes::StreamlineAlgorithmType_ToString(StreamlineAttributes::StreamlineAlgorithmType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return StreamlineAlgorithmType_strings[index];
}

std::string
StreamlineAttributes::StreamlineAlgorithmType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return StreamlineAlgorithmType_strings[index];
}

bool
StreamlineAttributes::StreamlineAlgorithmType_FromString(const std::string &s, StreamlineAttributes::StreamlineAlgorithmType &val)
{
    val = StreamlineAttributes::LoadOnDemand;
    for(int i = 0; i < 3; ++i)
    {
        if(s == StreamlineAlgorithmType_strings[i])
        {
            val = (StreamlineAlgorithmType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::IntegrationType
//

static const char *IntegrationType_strings[] = {
"DormandPrince", "AdamsBashforth"};

std::string
StreamlineAttributes::IntegrationType_ToString(StreamlineAttributes::IntegrationType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return IntegrationType_strings[index];
}

std::string
StreamlineAttributes::IntegrationType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return IntegrationType_strings[index];
}

bool
StreamlineAttributes::IntegrationType_FromString(const std::string &s, StreamlineAttributes::IntegrationType &val)
{
    val = StreamlineAttributes::DormandPrince;
    for(int i = 0; i < 2; ++i)
    {
        if(s == IntegrationType_strings[i])
        {
            val = (IntegrationType)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: StreamlineAttributes::StreamlineAttributes
//
// Purpose: 
//   Init utility for the StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void StreamlineAttributes::Init()
{
    sourceType = SpecifiedPoint;
    maxStepLength = 0.1;
    termination = 10;
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
    pointList.push_back(0);
    pointList.push_back(0);
    pointList.push_back(0);
    pointList.push_back(1);
    pointList.push_back(0);
    pointList.push_back(0);
    pointList.push_back(0);
    pointList.push_back(1);
    pointList.push_back(0);
    pointDensity = 2;
    displayMethod = Lines;
    showStart = false;
    radius = 0.125;
    lineWidth = 2;
    coloringMethod = ColorBySpeed;
    legendFlag = true;
    lightingFlag = true;
    StreamlineDirection = Forward;
    relTol = 0.0001;
    absTol = 1e-05;
    terminationType = Distance;
    integrationType = DormandPrince;
    streamlineAlgorithmType = ParallelStaticDomains;
    maxStreamlineProcessCount = 10;
    maxDomainCacheSize = 3;
    workGroupSize = 32;
    pathlines = false;
    legendMinFlag = false;
    legendMaxFlag = false;
    legendMin = 0;
    legendMax = 1;

    StreamlineAttributes::SelectAll();
}

// ****************************************************************************
// Method: StreamlineAttributes::StreamlineAttributes
//
// Purpose: 
//   Copy utility for the StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void StreamlineAttributes::Copy(const StreamlineAttributes &obj)
{

    sourceType = obj.sourceType;
    maxStepLength = obj.maxStepLength;
    termination = obj.termination;
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
    for(int i = 0; i < 6; ++i)
        boxExtents[i] = obj.boxExtents[i];

    useWholeBox = obj.useWholeBox;
    pointList = obj.pointList;
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
    relTol = obj.relTol;
    absTol = obj.absTol;
    terminationType = obj.terminationType;
    integrationType = obj.integrationType;
    streamlineAlgorithmType = obj.streamlineAlgorithmType;
    maxStreamlineProcessCount = obj.maxStreamlineProcessCount;
    maxDomainCacheSize = obj.maxDomainCacheSize;
    workGroupSize = obj.workGroupSize;
    pathlines = obj.pathlines;
    coloringVariable = obj.coloringVariable;
    legendMinFlag = obj.legendMinFlag;
    legendMaxFlag = obj.legendMaxFlag;
    legendMin = obj.legendMin;
    legendMax = obj.legendMax;

    StreamlineAttributes::SelectAll();
}

// Type map format string
const char *StreamlineAttributes::TypeMapFormatString = STREAMLINEATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t StreamlineAttributes::TmfsStruct = {STREAMLINEATTRIBUTES_TMFS};


// ****************************************************************************
// Method: StreamlineAttributes::StreamlineAttributes
//
// Purpose: 
//   Default constructor for the StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StreamlineAttributes::StreamlineAttributes() : 
    AttributeSubject(StreamlineAttributes::TypeMapFormatString),
    colorTableName("Default"), singleColor(0, 0, 0)
{
    StreamlineAttributes::Init();
}

// ****************************************************************************
// Method: StreamlineAttributes::StreamlineAttributes
//
// Purpose: 
//   Constructor for the derived classes of StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StreamlineAttributes::StreamlineAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs),
    colorTableName("Default"), singleColor(0, 0, 0)
{
    StreamlineAttributes::Init();
}

// ****************************************************************************
// Method: StreamlineAttributes::StreamlineAttributes
//
// Purpose: 
//   Copy constructor for the StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StreamlineAttributes::StreamlineAttributes(const StreamlineAttributes &obj) : 
    AttributeSubject(StreamlineAttributes::TypeMapFormatString)
{
    StreamlineAttributes::Copy(obj);
}

// ****************************************************************************
// Method: StreamlineAttributes::StreamlineAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StreamlineAttributes::StreamlineAttributes(const StreamlineAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    StreamlineAttributes::Copy(obj);
}

// ****************************************************************************
// Method: StreamlineAttributes::~StreamlineAttributes
//
// Purpose: 
//   Destructor for the StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StreamlineAttributes::~StreamlineAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: StreamlineAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the StreamlineAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StreamlineAttributes& 
StreamlineAttributes::operator = (const StreamlineAttributes &obj)
{
    if (this == &obj) return *this;

    StreamlineAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: StreamlineAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the StreamlineAttributes class.
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
StreamlineAttributes::operator == (const StreamlineAttributes &obj) const
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

    // Compare the planeOrigin arrays.
    bool planeOrigin_equal = true;
    for(int i = 0; i < 3 && planeOrigin_equal; ++i)
        planeOrigin_equal = (planeOrigin[i] == obj.planeOrigin[i]);

    // Compare the planeNormal arrays.
    bool planeNormal_equal = true;
    for(int i = 0; i < 3 && planeNormal_equal; ++i)
        planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

    // Compare the planeUpAxis arrays.
    bool planeUpAxis_equal = true;
    for(int i = 0; i < 3 && planeUpAxis_equal; ++i)
        planeUpAxis_equal = (planeUpAxis[i] == obj.planeUpAxis[i]);

    // Compare the sphereOrigin arrays.
    bool sphereOrigin_equal = true;
    for(int i = 0; i < 3 && sphereOrigin_equal; ++i)
        sphereOrigin_equal = (sphereOrigin[i] == obj.sphereOrigin[i]);

    // Compare the boxExtents arrays.
    bool boxExtents_equal = true;
    for(int i = 0; i < 6 && boxExtents_equal; ++i)
        boxExtents_equal = (boxExtents[i] == obj.boxExtents[i]);

    // Create the return value
    return ((sourceType == obj.sourceType) &&
            (maxStepLength == obj.maxStepLength) &&
            (termination == obj.termination) &&
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
            (pointList == obj.pointList) &&
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
            (StreamlineDirection == obj.StreamlineDirection) &&
            (relTol == obj.relTol) &&
            (absTol == obj.absTol) &&
            (terminationType == obj.terminationType) &&
            (integrationType == obj.integrationType) &&
            (streamlineAlgorithmType == obj.streamlineAlgorithmType) &&
            (maxStreamlineProcessCount == obj.maxStreamlineProcessCount) &&
            (maxDomainCacheSize == obj.maxDomainCacheSize) &&
            (workGroupSize == obj.workGroupSize) &&
            (pathlines == obj.pathlines) &&
            (coloringVariable == obj.coloringVariable) &&
            (legendMinFlag == obj.legendMinFlag) &&
            (legendMaxFlag == obj.legendMaxFlag) &&
            (legendMin == obj.legendMin) &&
            (legendMax == obj.legendMax));
}

// ****************************************************************************
// Method: StreamlineAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the StreamlineAttributes class.
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
StreamlineAttributes::operator != (const StreamlineAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: StreamlineAttributes::TypeName
//
// Purpose: 
//   Type name method for the StreamlineAttributes class.
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
StreamlineAttributes::TypeName() const
{
    return "StreamlineAttributes";
}

// ****************************************************************************
// Method: StreamlineAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the StreamlineAttributes class.
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
StreamlineAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const StreamlineAttributes *tmp = (const StreamlineAttributes *)atts;
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
//  Method: StreamlineAttributes::CreateCompatible
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
StreamlineAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;

    if(TypeName() == tname)
    {
        retval = new StreamlineAttributes(*this);
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
// Method: StreamlineAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the StreamlineAttributes class.
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
StreamlineAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new StreamlineAttributes(*this);
    else
        retval = new StreamlineAttributes;

    return retval;
}

// ****************************************************************************
// Method: StreamlineAttributes::SelectAll
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
StreamlineAttributes::SelectAll()
{
    Select(ID_sourceType,                (void *)&sourceType);
    Select(ID_maxStepLength,             (void *)&maxStepLength);
    Select(ID_termination,               (void *)&termination);
    Select(ID_pointSource,               (void *)pointSource, 3);
    Select(ID_lineStart,                 (void *)lineStart, 3);
    Select(ID_lineEnd,                   (void *)lineEnd, 3);
    Select(ID_planeOrigin,               (void *)planeOrigin, 3);
    Select(ID_planeNormal,               (void *)planeNormal, 3);
    Select(ID_planeUpAxis,               (void *)planeUpAxis, 3);
    Select(ID_planeRadius,               (void *)&planeRadius);
    Select(ID_sphereOrigin,              (void *)sphereOrigin, 3);
    Select(ID_sphereRadius,              (void *)&sphereRadius);
    Select(ID_boxExtents,                (void *)boxExtents, 6);
    Select(ID_useWholeBox,               (void *)&useWholeBox);
    Select(ID_pointList,                 (void *)&pointList);
    Select(ID_pointDensity,              (void *)&pointDensity);
    Select(ID_displayMethod,             (void *)&displayMethod);
    Select(ID_showStart,                 (void *)&showStart);
    Select(ID_radius,                    (void *)&radius);
    Select(ID_lineWidth,                 (void *)&lineWidth);
    Select(ID_coloringMethod,            (void *)&coloringMethod);
    Select(ID_colorTableName,            (void *)&colorTableName);
    Select(ID_singleColor,               (void *)&singleColor);
    Select(ID_legendFlag,                (void *)&legendFlag);
    Select(ID_lightingFlag,              (void *)&lightingFlag);
    Select(ID_StreamlineDirection,       (void *)&StreamlineDirection);
    Select(ID_relTol,                    (void *)&relTol);
    Select(ID_absTol,                    (void *)&absTol);
    Select(ID_terminationType,           (void *)&terminationType);
    Select(ID_integrationType,           (void *)&integrationType);
    Select(ID_streamlineAlgorithmType,   (void *)&streamlineAlgorithmType);
    Select(ID_maxStreamlineProcessCount, (void *)&maxStreamlineProcessCount);
    Select(ID_maxDomainCacheSize,        (void *)&maxDomainCacheSize);
    Select(ID_workGroupSize,             (void *)&workGroupSize);
    Select(ID_pathlines,                 (void *)&pathlines);
    Select(ID_coloringVariable,          (void *)&coloringVariable);
    Select(ID_legendMinFlag,             (void *)&legendMinFlag);
    Select(ID_legendMaxFlag,             (void *)&legendMaxFlag);
    Select(ID_legendMin,                 (void *)&legendMin);
    Select(ID_legendMax,                 (void *)&legendMax);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: StreamlineAttributes::CreateNode
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
StreamlineAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    StreamlineAttributes defaultObject;
    bool addToParent = false;
    // Create a node for StreamlineAttributes.
    DataNode *node = new DataNode("StreamlineAttributes");

    if(completeSave || !FieldsEqual(ID_sourceType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sourceType", SourceType_ToString(sourceType)));
    }

    if(completeSave || !FieldsEqual(ID_maxStepLength, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxStepLength", maxStepLength));
    }

    if(completeSave || !FieldsEqual(ID_termination, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("termination", termination));
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

    if(completeSave || !FieldsEqual(ID_pointList, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointList", pointList));
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

    if(completeSave || !FieldsEqual(ID_terminationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminationType", TerminationType_ToString(terminationType)));
    }

    if(completeSave || !FieldsEqual(ID_integrationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationType", IntegrationType_ToString(integrationType)));
    }

    if(completeSave || !FieldsEqual(ID_streamlineAlgorithmType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("streamlineAlgorithmType", StreamlineAlgorithmType_ToString(streamlineAlgorithmType)));
    }

    if(completeSave || !FieldsEqual(ID_maxStreamlineProcessCount, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxStreamlineProcessCount", maxStreamlineProcessCount));
    }

    if(completeSave || !FieldsEqual(ID_maxDomainCacheSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxDomainCacheSize", maxDomainCacheSize));
    }

    if(completeSave || !FieldsEqual(ID_workGroupSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("workGroupSize", workGroupSize));
    }

    if(completeSave || !FieldsEqual(ID_pathlines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlines", pathlines));
    }

    if(completeSave || !FieldsEqual(ID_coloringVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("coloringVariable", coloringVariable));
    }

    if(completeSave || !FieldsEqual(ID_legendMinFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendMinFlag", legendMinFlag));
    }

    if(completeSave || !FieldsEqual(ID_legendMaxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendMaxFlag", legendMaxFlag));
    }

    if(completeSave || !FieldsEqual(ID_legendMin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendMin", legendMin));
    }

    if(completeSave || !FieldsEqual(ID_legendMax, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("legendMax", legendMax));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: StreamlineAttributes::SetFromNode
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
StreamlineAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("StreamlineAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("sourceType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 6)
                SetSourceType(SourceType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SourceType value;
            if(SourceType_FromString(node->AsString(), value))
                SetSourceType(value);
        }
    }
    if((node = searchNode->GetNode("maxStepLength")) != 0)
        SetMaxStepLength(node->AsDouble());
    if((node = searchNode->GetNode("termination")) != 0)
        SetTermination(node->AsDouble());
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
    if((node = searchNode->GetNode("pointList")) != 0)
        SetPointList(node->AsDoubleVector());
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
            if(ival >= 0 && ival < 7)
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
    if((node = searchNode->GetNode("relTol")) != 0)
        SetRelTol(node->AsDouble());
    if((node = searchNode->GetNode("absTol")) != 0)
        SetAbsTol(node->AsDouble());
    if((node = searchNode->GetNode("terminationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetTerminationType(TerminationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TerminationType value;
            if(TerminationType_FromString(node->AsString(), value))
                SetTerminationType(value);
        }
    }
    if((node = searchNode->GetNode("integrationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetIntegrationType(IntegrationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationType value;
            if(IntegrationType_FromString(node->AsString(), value))
                SetIntegrationType(value);
        }
    }
    if((node = searchNode->GetNode("streamlineAlgorithmType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetStreamlineAlgorithmType(StreamlineAlgorithmType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            StreamlineAlgorithmType value;
            if(StreamlineAlgorithmType_FromString(node->AsString(), value))
                SetStreamlineAlgorithmType(value);
        }
    }
    if((node = searchNode->GetNode("maxStreamlineProcessCount")) != 0)
        SetMaxStreamlineProcessCount(node->AsInt());
    if((node = searchNode->GetNode("maxDomainCacheSize")) != 0)
        SetMaxDomainCacheSize(node->AsInt());
    if((node = searchNode->GetNode("workGroupSize")) != 0)
        SetWorkGroupSize(node->AsInt());
    if((node = searchNode->GetNode("pathlines")) != 0)
        SetPathlines(node->AsBool());
    if((node = searchNode->GetNode("coloringVariable")) != 0)
        SetColoringVariable(node->AsString());
    if((node = searchNode->GetNode("legendMinFlag")) != 0)
        SetLegendMinFlag(node->AsBool());
    if((node = searchNode->GetNode("legendMaxFlag")) != 0)
        SetLegendMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("legendMin")) != 0)
        SetLegendMin(node->AsDouble());
    if((node = searchNode->GetNode("legendMax")) != 0)
        SetLegendMax(node->AsDouble());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
StreamlineAttributes::SetSourceType(StreamlineAttributes::SourceType sourceType_)
{
    sourceType = sourceType_;
    Select(ID_sourceType, (void *)&sourceType);
}

void
StreamlineAttributes::SetMaxStepLength(double maxStepLength_)
{
    maxStepLength = maxStepLength_;
    Select(ID_maxStepLength, (void *)&maxStepLength);
}

void
StreamlineAttributes::SetTermination(double termination_)
{
    termination = termination_;
    Select(ID_termination, (void *)&termination);
}

void
StreamlineAttributes::SetPointSource(const double *pointSource_)
{
    pointSource[0] = pointSource_[0];
    pointSource[1] = pointSource_[1];
    pointSource[2] = pointSource_[2];
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
StreamlineAttributes::SetLineStart(const double *lineStart_)
{
    lineStart[0] = lineStart_[0];
    lineStart[1] = lineStart_[1];
    lineStart[2] = lineStart_[2];
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
StreamlineAttributes::SetLineEnd(const double *lineEnd_)
{
    lineEnd[0] = lineEnd_[0];
    lineEnd[1] = lineEnd_[1];
    lineEnd[2] = lineEnd_[2];
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
StreamlineAttributes::SetPlaneOrigin(const double *planeOrigin_)
{
    planeOrigin[0] = planeOrigin_[0];
    planeOrigin[1] = planeOrigin_[1];
    planeOrigin[2] = planeOrigin_[2];
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
StreamlineAttributes::SetPlaneNormal(const double *planeNormal_)
{
    planeNormal[0] = planeNormal_[0];
    planeNormal[1] = planeNormal_[1];
    planeNormal[2] = planeNormal_[2];
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
StreamlineAttributes::SetPlaneUpAxis(const double *planeUpAxis_)
{
    planeUpAxis[0] = planeUpAxis_[0];
    planeUpAxis[1] = planeUpAxis_[1];
    planeUpAxis[2] = planeUpAxis_[2];
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
StreamlineAttributes::SetPlaneRadius(double planeRadius_)
{
    planeRadius = planeRadius_;
    Select(ID_planeRadius, (void *)&planeRadius);
}

void
StreamlineAttributes::SetSphereOrigin(const double *sphereOrigin_)
{
    sphereOrigin[0] = sphereOrigin_[0];
    sphereOrigin[1] = sphereOrigin_[1];
    sphereOrigin[2] = sphereOrigin_[2];
    Select(ID_sphereOrigin, (void *)sphereOrigin, 3);
}

void
StreamlineAttributes::SetSphereRadius(double sphereRadius_)
{
    sphereRadius = sphereRadius_;
    Select(ID_sphereRadius, (void *)&sphereRadius);
}

void
StreamlineAttributes::SetBoxExtents(const double *boxExtents_)
{
    for(int i = 0; i < 6; ++i)
        boxExtents[i] = boxExtents_[i];
    Select(ID_boxExtents, (void *)boxExtents, 6);
}

void
StreamlineAttributes::SetUseWholeBox(bool useWholeBox_)
{
    useWholeBox = useWholeBox_;
    Select(ID_useWholeBox, (void *)&useWholeBox);
}

void
StreamlineAttributes::SetPointList(const doubleVector &pointList_)
{
    pointList = pointList_;
    Select(ID_pointList, (void *)&pointList);
}

void
StreamlineAttributes::SetPointDensity(int pointDensity_)
{
    pointDensity = pointDensity_;
    Select(ID_pointDensity, (void *)&pointDensity);
}

void
StreamlineAttributes::SetDisplayMethod(StreamlineAttributes::DisplayMethod displayMethod_)
{
    displayMethod = displayMethod_;
    Select(ID_displayMethod, (void *)&displayMethod);
}

void
StreamlineAttributes::SetShowStart(bool showStart_)
{
    showStart = showStart_;
    Select(ID_showStart, (void *)&showStart);
}

void
StreamlineAttributes::SetRadius(double radius_)
{
    radius = radius_;
    Select(ID_radius, (void *)&radius);
}

void
StreamlineAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
StreamlineAttributes::SetColoringMethod(StreamlineAttributes::ColoringMethod coloringMethod_)
{
    coloringMethod = coloringMethod_;
    Select(ID_coloringMethod, (void *)&coloringMethod);
}

void
StreamlineAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
StreamlineAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
StreamlineAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
StreamlineAttributes::SetLightingFlag(bool lightingFlag_)
{
    lightingFlag = lightingFlag_;
    Select(ID_lightingFlag, (void *)&lightingFlag);
}

void
StreamlineAttributes::SetStreamlineDirection(StreamlineAttributes::IntegrationDirection StreamlineDirection_)
{
    StreamlineDirection = StreamlineDirection_;
    Select(ID_StreamlineDirection, (void *)&StreamlineDirection);
}

void
StreamlineAttributes::SetRelTol(double relTol_)
{
    relTol = relTol_;
    Select(ID_relTol, (void *)&relTol);
}

void
StreamlineAttributes::SetAbsTol(double absTol_)
{
    absTol = absTol_;
    Select(ID_absTol, (void *)&absTol);
}

void
StreamlineAttributes::SetTerminationType(StreamlineAttributes::TerminationType terminationType_)
{
    terminationType = terminationType_;
    Select(ID_terminationType, (void *)&terminationType);
}

void
StreamlineAttributes::SetIntegrationType(StreamlineAttributes::IntegrationType integrationType_)
{
    integrationType = integrationType_;
    Select(ID_integrationType, (void *)&integrationType);
}

void
StreamlineAttributes::SetStreamlineAlgorithmType(StreamlineAttributes::StreamlineAlgorithmType streamlineAlgorithmType_)
{
    streamlineAlgorithmType = streamlineAlgorithmType_;
    Select(ID_streamlineAlgorithmType, (void *)&streamlineAlgorithmType);
}

void
StreamlineAttributes::SetMaxStreamlineProcessCount(int maxStreamlineProcessCount_)
{
    maxStreamlineProcessCount = maxStreamlineProcessCount_;
    Select(ID_maxStreamlineProcessCount, (void *)&maxStreamlineProcessCount);
}

void
StreamlineAttributes::SetMaxDomainCacheSize(int maxDomainCacheSize_)
{
    maxDomainCacheSize = maxDomainCacheSize_;
    Select(ID_maxDomainCacheSize, (void *)&maxDomainCacheSize);
}

void
StreamlineAttributes::SetWorkGroupSize(int workGroupSize_)
{
    workGroupSize = workGroupSize_;
    Select(ID_workGroupSize, (void *)&workGroupSize);
}

void
StreamlineAttributes::SetPathlines(bool pathlines_)
{
    pathlines = pathlines_;
    Select(ID_pathlines, (void *)&pathlines);
}

void
StreamlineAttributes::SetColoringVariable(const std::string &coloringVariable_)
{
    coloringVariable = coloringVariable_;
    Select(ID_coloringVariable, (void *)&coloringVariable);
}

void
StreamlineAttributes::SetLegendMinFlag(bool legendMinFlag_)
{
    legendMinFlag = legendMinFlag_;
    Select(ID_legendMinFlag, (void *)&legendMinFlag);
}

void
StreamlineAttributes::SetLegendMaxFlag(bool legendMaxFlag_)
{
    legendMaxFlag = legendMaxFlag_;
    Select(ID_legendMaxFlag, (void *)&legendMaxFlag);
}

void
StreamlineAttributes::SetLegendMin(double legendMin_)
{
    legendMin = legendMin_;
    Select(ID_legendMin, (void *)&legendMin);
}

void
StreamlineAttributes::SetLegendMax(double legendMax_)
{
    legendMax = legendMax_;
    Select(ID_legendMax, (void *)&legendMax);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

StreamlineAttributes::SourceType
StreamlineAttributes::GetSourceType() const
{
    return SourceType(sourceType);
}

double
StreamlineAttributes::GetMaxStepLength() const
{
    return maxStepLength;
}

double
StreamlineAttributes::GetTermination() const
{
    return termination;
}

const double *
StreamlineAttributes::GetPointSource() const
{
    return pointSource;
}

double *
StreamlineAttributes::GetPointSource()
{
    return pointSource;
}

const double *
StreamlineAttributes::GetLineStart() const
{
    return lineStart;
}

double *
StreamlineAttributes::GetLineStart()
{
    return lineStart;
}

const double *
StreamlineAttributes::GetLineEnd() const
{
    return lineEnd;
}

double *
StreamlineAttributes::GetLineEnd()
{
    return lineEnd;
}

const double *
StreamlineAttributes::GetPlaneOrigin() const
{
    return planeOrigin;
}

double *
StreamlineAttributes::GetPlaneOrigin()
{
    return planeOrigin;
}

const double *
StreamlineAttributes::GetPlaneNormal() const
{
    return planeNormal;
}

double *
StreamlineAttributes::GetPlaneNormal()
{
    return planeNormal;
}

const double *
StreamlineAttributes::GetPlaneUpAxis() const
{
    return planeUpAxis;
}

double *
StreamlineAttributes::GetPlaneUpAxis()
{
    return planeUpAxis;
}

double
StreamlineAttributes::GetPlaneRadius() const
{
    return planeRadius;
}

const double *
StreamlineAttributes::GetSphereOrigin() const
{
    return sphereOrigin;
}

double *
StreamlineAttributes::GetSphereOrigin()
{
    return sphereOrigin;
}

double
StreamlineAttributes::GetSphereRadius() const
{
    return sphereRadius;
}

const double *
StreamlineAttributes::GetBoxExtents() const
{
    return boxExtents;
}

double *
StreamlineAttributes::GetBoxExtents()
{
    return boxExtents;
}

bool
StreamlineAttributes::GetUseWholeBox() const
{
    return useWholeBox;
}

const doubleVector &
StreamlineAttributes::GetPointList() const
{
    return pointList;
}

doubleVector &
StreamlineAttributes::GetPointList()
{
    return pointList;
}

int
StreamlineAttributes::GetPointDensity() const
{
    return pointDensity;
}

StreamlineAttributes::DisplayMethod
StreamlineAttributes::GetDisplayMethod() const
{
    return DisplayMethod(displayMethod);
}

bool
StreamlineAttributes::GetShowStart() const
{
    return showStart;
}

double
StreamlineAttributes::GetRadius() const
{
    return radius;
}

int
StreamlineAttributes::GetLineWidth() const
{
    return lineWidth;
}

StreamlineAttributes::ColoringMethod
StreamlineAttributes::GetColoringMethod() const
{
    return ColoringMethod(coloringMethod);
}

const std::string &
StreamlineAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
StreamlineAttributes::GetColorTableName()
{
    return colorTableName;
}

const ColorAttribute &
StreamlineAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
StreamlineAttributes::GetSingleColor()
{
    return singleColor;
}

bool
StreamlineAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
StreamlineAttributes::GetLightingFlag() const
{
    return lightingFlag;
}

StreamlineAttributes::IntegrationDirection
StreamlineAttributes::GetStreamlineDirection() const
{
    return IntegrationDirection(StreamlineDirection);
}

double
StreamlineAttributes::GetRelTol() const
{
    return relTol;
}

double
StreamlineAttributes::GetAbsTol() const
{
    return absTol;
}

StreamlineAttributes::TerminationType
StreamlineAttributes::GetTerminationType() const
{
    return TerminationType(terminationType);
}

StreamlineAttributes::IntegrationType
StreamlineAttributes::GetIntegrationType() const
{
    return IntegrationType(integrationType);
}

StreamlineAttributes::StreamlineAlgorithmType
StreamlineAttributes::GetStreamlineAlgorithmType() const
{
    return StreamlineAlgorithmType(streamlineAlgorithmType);
}

int
StreamlineAttributes::GetMaxStreamlineProcessCount() const
{
    return maxStreamlineProcessCount;
}

int
StreamlineAttributes::GetMaxDomainCacheSize() const
{
    return maxDomainCacheSize;
}

int
StreamlineAttributes::GetWorkGroupSize() const
{
    return workGroupSize;
}

bool
StreamlineAttributes::GetPathlines() const
{
    return pathlines;
}

const std::string &
StreamlineAttributes::GetColoringVariable() const
{
    return coloringVariable;
}

std::string &
StreamlineAttributes::GetColoringVariable()
{
    return coloringVariable;
}

bool
StreamlineAttributes::GetLegendMinFlag() const
{
    return legendMinFlag;
}

bool
StreamlineAttributes::GetLegendMaxFlag() const
{
    return legendMaxFlag;
}

double
StreamlineAttributes::GetLegendMin() const
{
    return legendMin;
}

double
StreamlineAttributes::GetLegendMax() const
{
    return legendMax;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
StreamlineAttributes::SelectPointSource()
{
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
StreamlineAttributes::SelectLineStart()
{
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
StreamlineAttributes::SelectLineEnd()
{
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
StreamlineAttributes::SelectPlaneOrigin()
{
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
StreamlineAttributes::SelectPlaneNormal()
{
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
StreamlineAttributes::SelectPlaneUpAxis()
{
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
StreamlineAttributes::SelectSphereOrigin()
{
    Select(ID_sphereOrigin, (void *)sphereOrigin, 3);
}

void
StreamlineAttributes::SelectBoxExtents()
{
    Select(ID_boxExtents, (void *)boxExtents, 6);
}

void
StreamlineAttributes::SelectPointList()
{
    Select(ID_pointList, (void *)&pointList);
}

void
StreamlineAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
StreamlineAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

void
StreamlineAttributes::SelectColoringVariable()
{
    Select(ID_coloringVariable, (void *)&coloringVariable);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: StreamlineAttributes::GetFieldName
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
StreamlineAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_sourceType:                return "sourceType";
    case ID_maxStepLength:             return "maxStepLength";
    case ID_termination:               return "termination";
    case ID_pointSource:               return "pointSource";
    case ID_lineStart:                 return "lineStart";
    case ID_lineEnd:                   return "lineEnd";
    case ID_planeOrigin:               return "planeOrigin";
    case ID_planeNormal:               return "planeNormal";
    case ID_planeUpAxis:               return "planeUpAxis";
    case ID_planeRadius:               return "planeRadius";
    case ID_sphereOrigin:              return "sphereOrigin";
    case ID_sphereRadius:              return "sphereRadius";
    case ID_boxExtents:                return "boxExtents";
    case ID_useWholeBox:               return "useWholeBox";
    case ID_pointList:                 return "pointList";
    case ID_pointDensity:              return "pointDensity";
    case ID_displayMethod:             return "displayMethod";
    case ID_showStart:                 return "showStart";
    case ID_radius:                    return "radius";
    case ID_lineWidth:                 return "lineWidth";
    case ID_coloringMethod:            return "coloringMethod";
    case ID_colorTableName:            return "colorTableName";
    case ID_singleColor:               return "singleColor";
    case ID_legendFlag:                return "legendFlag";
    case ID_lightingFlag:              return "lightingFlag";
    case ID_StreamlineDirection:       return "StreamlineDirection";
    case ID_relTol:                    return "relTol";
    case ID_absTol:                    return "absTol";
    case ID_terminationType:           return "terminationType";
    case ID_integrationType:           return "integrationType";
    case ID_streamlineAlgorithmType:   return "streamlineAlgorithmType";
    case ID_maxStreamlineProcessCount: return "maxStreamlineProcessCount";
    case ID_maxDomainCacheSize:        return "maxDomainCacheSize";
    case ID_workGroupSize:             return "workGroupSize";
    case ID_pathlines:                 return "pathlines";
    case ID_coloringVariable:          return "coloringVariable";
    case ID_legendMinFlag:             return "legendMinFlag";
    case ID_legendMaxFlag:             return "legendMaxFlag";
    case ID_legendMin:                 return "legendMin";
    case ID_legendMax:                 return "legendMax";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: StreamlineAttributes::GetFieldType
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
StreamlineAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_sourceType:                return FieldType_enum;
    case ID_maxStepLength:             return FieldType_double;
    case ID_termination:               return FieldType_double;
    case ID_pointSource:               return FieldType_doubleArray;
    case ID_lineStart:                 return FieldType_doubleArray;
    case ID_lineEnd:                   return FieldType_doubleArray;
    case ID_planeOrigin:               return FieldType_doubleArray;
    case ID_planeNormal:               return FieldType_doubleArray;
    case ID_planeUpAxis:               return FieldType_doubleArray;
    case ID_planeRadius:               return FieldType_double;
    case ID_sphereOrigin:              return FieldType_doubleArray;
    case ID_sphereRadius:              return FieldType_double;
    case ID_boxExtents:                return FieldType_doubleArray;
    case ID_useWholeBox:               return FieldType_bool;
    case ID_pointList:                 return FieldType_doubleVector;
    case ID_pointDensity:              return FieldType_int;
    case ID_displayMethod:             return FieldType_enum;
    case ID_showStart:                 return FieldType_bool;
    case ID_radius:                    return FieldType_double;
    case ID_lineWidth:                 return FieldType_linewidth;
    case ID_coloringMethod:            return FieldType_enum;
    case ID_colorTableName:            return FieldType_colortable;
    case ID_singleColor:               return FieldType_color;
    case ID_legendFlag:                return FieldType_bool;
    case ID_lightingFlag:              return FieldType_bool;
    case ID_StreamlineDirection:       return FieldType_enum;
    case ID_relTol:                    return FieldType_double;
    case ID_absTol:                    return FieldType_double;
    case ID_terminationType:           return FieldType_enum;
    case ID_integrationType:           return FieldType_enum;
    case ID_streamlineAlgorithmType:   return FieldType_enum;
    case ID_maxStreamlineProcessCount: return FieldType_int;
    case ID_maxDomainCacheSize:        return FieldType_int;
    case ID_workGroupSize:             return FieldType_int;
    case ID_pathlines:                 return FieldType_bool;
    case ID_coloringVariable:          return FieldType_string;
    case ID_legendMinFlag:             return FieldType_bool;
    case ID_legendMaxFlag:             return FieldType_bool;
    case ID_legendMin:                 return FieldType_double;
    case ID_legendMax:                 return FieldType_double;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: StreamlineAttributes::GetFieldTypeName
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
StreamlineAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_sourceType:                return "enum";
    case ID_maxStepLength:             return "double";
    case ID_termination:               return "double";
    case ID_pointSource:               return "doubleArray";
    case ID_lineStart:                 return "doubleArray";
    case ID_lineEnd:                   return "doubleArray";
    case ID_planeOrigin:               return "doubleArray";
    case ID_planeNormal:               return "doubleArray";
    case ID_planeUpAxis:               return "doubleArray";
    case ID_planeRadius:               return "double";
    case ID_sphereOrigin:              return "doubleArray";
    case ID_sphereRadius:              return "double";
    case ID_boxExtents:                return "doubleArray";
    case ID_useWholeBox:               return "bool";
    case ID_pointList:                 return "doubleVector";
    case ID_pointDensity:              return "int";
    case ID_displayMethod:             return "enum";
    case ID_showStart:                 return "bool";
    case ID_radius:                    return "double";
    case ID_lineWidth:                 return "linewidth";
    case ID_coloringMethod:            return "enum";
    case ID_colorTableName:            return "colortable";
    case ID_singleColor:               return "color";
    case ID_legendFlag:                return "bool";
    case ID_lightingFlag:              return "bool";
    case ID_StreamlineDirection:       return "enum";
    case ID_relTol:                    return "double";
    case ID_absTol:                    return "double";
    case ID_terminationType:           return "enum";
    case ID_integrationType:           return "enum";
    case ID_streamlineAlgorithmType:   return "enum";
    case ID_maxStreamlineProcessCount: return "int";
    case ID_maxDomainCacheSize:        return "int";
    case ID_workGroupSize:             return "int";
    case ID_pathlines:                 return "bool";
    case ID_coloringVariable:          return "string";
    case ID_legendMinFlag:             return "bool";
    case ID_legendMaxFlag:             return "bool";
    case ID_legendMin:                 return "double";
    case ID_legendMax:                 return "double";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: StreamlineAttributes::FieldsEqual
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
StreamlineAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const StreamlineAttributes &obj = *((const StreamlineAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_sourceType:
        {  // new scope
        retval = (sourceType == obj.sourceType);
        }
        break;
    case ID_maxStepLength:
        {  // new scope
        retval = (maxStepLength == obj.maxStepLength);
        }
        break;
    case ID_termination:
        {  // new scope
        retval = (termination == obj.termination);
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
    case ID_planeOrigin:
        {  // new scope
        // Compare the planeOrigin arrays.
        bool planeOrigin_equal = true;
        for(int i = 0; i < 3 && planeOrigin_equal; ++i)
            planeOrigin_equal = (planeOrigin[i] == obj.planeOrigin[i]);

        retval = planeOrigin_equal;
        }
        break;
    case ID_planeNormal:
        {  // new scope
        // Compare the planeNormal arrays.
        bool planeNormal_equal = true;
        for(int i = 0; i < 3 && planeNormal_equal; ++i)
            planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

        retval = planeNormal_equal;
        }
        break;
    case ID_planeUpAxis:
        {  // new scope
        // Compare the planeUpAxis arrays.
        bool planeUpAxis_equal = true;
        for(int i = 0; i < 3 && planeUpAxis_equal; ++i)
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
        for(int i = 0; i < 3 && sphereOrigin_equal; ++i)
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
        for(int i = 0; i < 6 && boxExtents_equal; ++i)
            boxExtents_equal = (boxExtents[i] == obj.boxExtents[i]);

        retval = boxExtents_equal;
        }
        break;
    case ID_useWholeBox:
        {  // new scope
        retval = (useWholeBox == obj.useWholeBox);
        }
        break;
    case ID_pointList:
        {  // new scope
        retval = (pointList == obj.pointList);
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
    case ID_terminationType:
        {  // new scope
        retval = (terminationType == obj.terminationType);
        }
        break;
    case ID_integrationType:
        {  // new scope
        retval = (integrationType == obj.integrationType);
        }
        break;
    case ID_streamlineAlgorithmType:
        {  // new scope
        retval = (streamlineAlgorithmType == obj.streamlineAlgorithmType);
        }
        break;
    case ID_maxStreamlineProcessCount:
        {  // new scope
        retval = (maxStreamlineProcessCount == obj.maxStreamlineProcessCount);
        }
        break;
    case ID_maxDomainCacheSize:
        {  // new scope
        retval = (maxDomainCacheSize == obj.maxDomainCacheSize);
        }
        break;
    case ID_workGroupSize:
        {  // new scope
        retval = (workGroupSize == obj.workGroupSize);
        }
        break;
    case ID_pathlines:
        {  // new scope
        retval = (pathlines == obj.pathlines);
        }
        break;
    case ID_coloringVariable:
        {  // new scope
        retval = (coloringVariable == obj.coloringVariable);
        }
        break;
    case ID_legendMinFlag:
        {  // new scope
        retval = (legendMinFlag == obj.legendMinFlag);
        }
        break;
    case ID_legendMaxFlag:
        {  // new scope
        retval = (legendMaxFlag == obj.legendMaxFlag);
        }
        break;
    case ID_legendMin:
        {  // new scope
        retval = (legendMin == obj.legendMin);
        }
        break;
    case ID_legendMax:
        {  // new scope
        retval = (legendMax == obj.legendMax);
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
//  Method: StreamlineAttributes::ChangesRequireRecalculation
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
//    Hank Childs, Sun May  3 11:49:31 CDT 2009
//    Add support for point lists.
//
// ****************************************************************************

#define PDIF(p1,p2,i) ((p1)[i] != (p2)[i])
#define POINT_DIFFERS(p1,p2) (PDIF(p1,p2,0) || PDIF(p1,p2,1) || PDIF(p1,p2,2))

bool
StreamlineAttributes::ChangesRequireRecalculation(const StreamlineAttributes &obj) const
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

    bool sourcePointListDiffers = (sourceType == SpecifiedPointList);
    if (sourcePointListDiffers)
    {
        sourcePointListDiffers = false;
        if (pointList.size() != obj.pointList.size())
            sourcePointListDiffers = true;
        else
            for (int i = 0 ; i < pointList.size() ; i++)
                if (pointList[i] != obj.pointList[i])
                    sourcePointListDiffers = true;
    }

    // If we're in box source mode and the box differs, boxDiffers
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

    bool radiusMatters = (radius != obj.radius);

    return (sourceType != obj.sourceType) ||
           (StreamlineDirection != obj.StreamlineDirection) ||
           (displayMethod != obj.displayMethod) ||
           (showStart != obj.showStart) ||
           (termination != obj.termination) ||
           (terminationType != obj.terminationType) ||
           (integrationType != obj.integrationType) ||
           (maxStepLength != obj.maxStepLength) ||
           (relTol != obj.relTol) ||
           (absTol != obj.absTol) ||
           (coloringMethod != obj.coloringMethod && obj.coloringMethod != Solid) ||
           (pathlines != obj.pathlines) ||
           (coloringVariable != obj.coloringVariable) ||
           sourcePointsDiffer ||
           sourceLineDiffers ||
           sourcePlaneDiffers ||
           sourceSphereDiffers ||
           sourcePointListDiffers ||
           boxSourceDiffers ||
           densityMatters ||
           radiusMatters;
}

