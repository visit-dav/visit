/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <stdlib.h>

//
// Enum conversion methods for StreamlineAttributes::SourceType
//

static const char *SourceType_strings[] = {
"SpecifiedPoint", "SpecifiedPointList", "SpecifiedLine", 
"SpecifiedCircle", "SpecifiedPlane", "SpecifiedSphere", 
"SpecifiedBox"};

std::string
StreamlineAttributes::SourceType_ToString(StreamlineAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 7) index = 0;
    return SourceType_strings[index];
}

std::string
StreamlineAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 7) ? 0 : t;
    return SourceType_strings[index];
}

bool
StreamlineAttributes::SourceType_FromString(const std::string &s, StreamlineAttributes::SourceType &val)
{
    val = StreamlineAttributes::SpecifiedPoint;
    for(int i = 0; i < 7; ++i)
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
// Enum conversion methods for StreamlineAttributes::CoordinateSystem
//

static const char *CoordinateSystem_strings[] = {
"AsIs", "CylindricalToCartesian", "CartesianToCylindrical"
};

std::string
StreamlineAttributes::CoordinateSystem_ToString(StreamlineAttributes::CoordinateSystem t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return CoordinateSystem_strings[index];
}

std::string
StreamlineAttributes::CoordinateSystem_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return CoordinateSystem_strings[index];
}

bool
StreamlineAttributes::CoordinateSystem_FromString(const std::string &s, StreamlineAttributes::CoordinateSystem &val)
{
    val = StreamlineAttributes::AsIs;
    for(int i = 0; i < 3; ++i)
    {
        if(s == CoordinateSystem_strings[i])
        {
            val = (CoordinateSystem)i;
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
// Enum conversion methods for StreamlineAttributes::ReferenceType
//

static const char *ReferenceType_strings[] = {
"Distance", "Time", "Step"
};

std::string
StreamlineAttributes::ReferenceType_ToString(StreamlineAttributes::ReferenceType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ReferenceType_strings[index];
}

std::string
StreamlineAttributes::ReferenceType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ReferenceType_strings[index];
}

bool
StreamlineAttributes::ReferenceType_FromString(const std::string &s, StreamlineAttributes::ReferenceType &val)
{
    val = StreamlineAttributes::Distance;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ReferenceType_strings[i])
        {
            val = (ReferenceType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::StreamlineAlgorithmType
//

static const char *StreamlineAlgorithmType_strings[] = {
"LoadOnDemand", "ParallelStaticDomains", "MasterSlave", 
"VisItSelects"};

std::string
StreamlineAttributes::StreamlineAlgorithmType_ToString(StreamlineAttributes::StreamlineAlgorithmType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return StreamlineAlgorithmType_strings[index];
}

std::string
StreamlineAttributes::StreamlineAlgorithmType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return StreamlineAlgorithmType_strings[index];
}

bool
StreamlineAttributes::StreamlineAlgorithmType_FromString(const std::string &s, StreamlineAttributes::StreamlineAlgorithmType &val)
{
    val = StreamlineAttributes::LoadOnDemand;
    for(int i = 0; i < 4; ++i)
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
"DormandPrince", "AdamsBashforth", "M3DC1Integrator", 
"NIMRODIntegrator"};

std::string
StreamlineAttributes::IntegrationType_ToString(StreamlineAttributes::IntegrationType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return IntegrationType_strings[index];
}

std::string
StreamlineAttributes::IntegrationType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return IntegrationType_strings[index];
}

bool
StreamlineAttributes::IntegrationType_FromString(const std::string &s, StreamlineAttributes::IntegrationType &val)
{
    val = StreamlineAttributes::DormandPrince;
    for(int i = 0; i < 4; ++i)
    {
        if(s == IntegrationType_strings[i])
        {
            val = (IntegrationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::OpacityType
//

static const char *OpacityType_strings[] = {
"FullyOpaque", "Constant", "Ramp", 
"VariableRange"};

std::string
StreamlineAttributes::OpacityType_ToString(StreamlineAttributes::OpacityType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return OpacityType_strings[index];
}

std::string
StreamlineAttributes::OpacityType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return OpacityType_strings[index];
}

bool
StreamlineAttributes::OpacityType_FromString(const std::string &s, StreamlineAttributes::OpacityType &val)
{
    val = StreamlineAttributes::FullyOpaque;
    for(int i = 0; i < 4; ++i)
    {
        if(s == OpacityType_strings[i])
        {
            val = (OpacityType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::DisplayQuality
//

static const char *DisplayQuality_strings[] = {
"Low", "Medium", "High", 
"Super"};

std::string
StreamlineAttributes::DisplayQuality_ToString(StreamlineAttributes::DisplayQuality t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return DisplayQuality_strings[index];
}

std::string
StreamlineAttributes::DisplayQuality_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return DisplayQuality_strings[index];
}

bool
StreamlineAttributes::DisplayQuality_FromString(const std::string &s, StreamlineAttributes::DisplayQuality &val)
{
    val = StreamlineAttributes::Low;
    for(int i = 0; i < 4; ++i)
    {
        if(s == DisplayQuality_strings[i])
        {
            val = (DisplayQuality)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::GeomDisplayType
//

static const char *GeomDisplayType_strings[] = {
"Sphere", "Cone"};

std::string
StreamlineAttributes::GeomDisplayType_ToString(StreamlineAttributes::GeomDisplayType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return GeomDisplayType_strings[index];
}

std::string
StreamlineAttributes::GeomDisplayType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return GeomDisplayType_strings[index];
}

bool
StreamlineAttributes::GeomDisplayType_FromString(const std::string &s, StreamlineAttributes::GeomDisplayType &val)
{
    val = StreamlineAttributes::Sphere;
    for(int i = 0; i < 2; ++i)
    {
        if(s == GeomDisplayType_strings[i])
        {
            val = (GeomDisplayType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::SizeType
//

static const char *SizeType_strings[] = {
"Absolute", "FractionOfBBox"};

std::string
StreamlineAttributes::SizeType_ToString(StreamlineAttributes::SizeType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SizeType_strings[index];
}

std::string
StreamlineAttributes::SizeType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SizeType_strings[index];
}

bool
StreamlineAttributes::SizeType_FromString(const std::string &s, StreamlineAttributes::SizeType &val)
{
    val = StreamlineAttributes::Absolute;
    for(int i = 0; i < 2; ++i)
    {
        if(s == SizeType_strings[i])
        {
            val = (SizeType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StreamlineAttributes::PathlinesCMFE
//

static const char *PathlinesCMFE_strings[] = {
"CONN_CMFE", "POS_CMFE"};

std::string
StreamlineAttributes::PathlinesCMFE_ToString(StreamlineAttributes::PathlinesCMFE t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return PathlinesCMFE_strings[index];
}

std::string
StreamlineAttributes::PathlinesCMFE_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return PathlinesCMFE_strings[index];
}

bool
StreamlineAttributes::PathlinesCMFE_FromString(const std::string &s, StreamlineAttributes::PathlinesCMFE &val)
{
    val = StreamlineAttributes::CONN_CMFE;
    for(int i = 0; i < 2; ++i)
    {
        if(s == PathlinesCMFE_strings[i])
        {
            val = (PathlinesCMFE)i;
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
    radius = 1;
    sphereOrigin[0] = 0;
    sphereOrigin[1] = 0;
    sphereOrigin[2] = 0;
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
    sampleDensity0 = 2;
    sampleDensity1 = 2;
    sampleDensity2 = 2;
    coloringMethod = ColorByTime;
    legendFlag = true;
    lightingFlag = true;
    streamlineDirection = Forward;
    maxSteps = 1000;
    terminateByDistance = false;
    termDistance = 10;
    terminateByTime = false;
    termTime = 10;
    maxStepLength = 0.1;
    limitMaximumTimestep = false;
    maxTimeStep = 0.1;
    relTol = 0.0001;
    absTolSizeType = FractionOfBBox;
    absTolAbsolute = 1e-06;
    absTolBBox = 1e-06;
    integrationType = DormandPrince;
    streamlineAlgorithmType = VisItSelects;
    maxStreamlineProcessCount = 10;
    maxDomainCacheSize = 3;
    workGroupSize = 32;
    pathlines = false;
    pathlinesOverrideStartingTimeFlag = false;
    pathlinesOverrideStartingTime = 0;
    pathlinesCMFE = POS_CMFE;
    coordinateSystem = AsIs;
    phiFactor = 0;
    legendMinFlag = false;
    legendMaxFlag = false;
    legendMin = 0;
    legendMax = 1;
    displayBegin = 0;
    displayEnd = 1;
    displayBeginFlag = false;
    displayEndFlag = false;
    referenceTypeForDisplay = Distance;
    displayMethod = Lines;
    tubeSizeType = FractionOfBBox;
    tubeRadiusAbsolute = 0.125;
    tubeRadiusBBox = 0.005;
    ribbonWidthSizeType = FractionOfBBox;
    ribbonWidthAbsolute = 0.125;
    ribbonWidthBBox = 0.01;
    lineWidth = 2;
    showSeeds = true;
    seedRadiusSizeType = FractionOfBBox;
    seedRadiusAbsolute = 1;
    seedRadiusBBox = 0.015;
    showHeads = false;
    headDisplayType = Sphere;
    headRadiusSizeType = FractionOfBBox;
    headRadiusAbsolute = 0.25;
    headRadiusBBox = 0.02;
    headHeightRatio = 2;
    opacityType = FullyOpaque;
    opacity = 1;
    opacityVarMin = 0;
    opacityVarMax = 1;
    opacityVarMinFlag = false;
    opacityVarMaxFlag = false;
    tubeDisplayDensity = 10;
    geomDisplayQuality = Medium;
    sampleDistance0 = 10;
    sampleDistance1 = 10;
    sampleDistance2 = 10;
    fillInterior = true;
    randomSamples = false;
    randomSeed = 0;
    numberOfRandomSamples = 1;
    forceNodeCenteredData = false;
    issueTerminationWarnings = true;
    issueStiffnessWarnings = true;
    issueCriticalPointsWarnings = true;
    criticalPointThreshold = 0.001;

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

    radius = obj.radius;
    sphereOrigin[0] = obj.sphereOrigin[0];
    sphereOrigin[1] = obj.sphereOrigin[1];
    sphereOrigin[2] = obj.sphereOrigin[2];

    for(int i = 0; i < 6; ++i)
        boxExtents[i] = obj.boxExtents[i];

    useWholeBox = obj.useWholeBox;
    pointList = obj.pointList;
    sampleDensity0 = obj.sampleDensity0;
    sampleDensity1 = obj.sampleDensity1;
    sampleDensity2 = obj.sampleDensity2;
    coloringMethod = obj.coloringMethod;
    colorTableName = obj.colorTableName;
    singleColor = obj.singleColor;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    streamlineDirection = obj.streamlineDirection;
    maxSteps = obj.maxSteps;
    terminateByDistance = obj.terminateByDistance;
    termDistance = obj.termDistance;
    terminateByTime = obj.terminateByTime;
    termTime = obj.termTime;
    maxStepLength = obj.maxStepLength;
    limitMaximumTimestep = obj.limitMaximumTimestep;
    maxTimeStep = obj.maxTimeStep;
    relTol = obj.relTol;
    absTolSizeType = obj.absTolSizeType;
    absTolAbsolute = obj.absTolAbsolute;
    absTolBBox = obj.absTolBBox;
    integrationType = obj.integrationType;
    streamlineAlgorithmType = obj.streamlineAlgorithmType;
    maxStreamlineProcessCount = obj.maxStreamlineProcessCount;
    maxDomainCacheSize = obj.maxDomainCacheSize;
    workGroupSize = obj.workGroupSize;
    pathlines = obj.pathlines;
    pathlinesOverrideStartingTimeFlag = obj.pathlinesOverrideStartingTimeFlag;
    pathlinesOverrideStartingTime = obj.pathlinesOverrideStartingTime;
    pathlinesCMFE = obj.pathlinesCMFE;
    coordinateSystem = obj.coordinateSystem;
    phiFactor = obj.phiFactor;
    coloringVariable = obj.coloringVariable;
    legendMinFlag = obj.legendMinFlag;
    legendMaxFlag = obj.legendMaxFlag;
    legendMin = obj.legendMin;
    legendMax = obj.legendMax;
    displayBegin = obj.displayBegin;
    displayEnd = obj.displayEnd;
    displayBeginFlag = obj.displayBeginFlag;
    displayEndFlag = obj.displayEndFlag;
    referenceTypeForDisplay = obj.referenceTypeForDisplay;
    displayMethod = obj.displayMethod;
    tubeSizeType = obj.tubeSizeType;
    tubeRadiusAbsolute = obj.tubeRadiusAbsolute;
    tubeRadiusBBox = obj.tubeRadiusBBox;
    ribbonWidthSizeType = obj.ribbonWidthSizeType;
    ribbonWidthAbsolute = obj.ribbonWidthAbsolute;
    ribbonWidthBBox = obj.ribbonWidthBBox;
    lineWidth = obj.lineWidth;
    showSeeds = obj.showSeeds;
    seedRadiusSizeType = obj.seedRadiusSizeType;
    seedRadiusAbsolute = obj.seedRadiusAbsolute;
    seedRadiusBBox = obj.seedRadiusBBox;
    showHeads = obj.showHeads;
    headDisplayType = obj.headDisplayType;
    headRadiusSizeType = obj.headRadiusSizeType;
    headRadiusAbsolute = obj.headRadiusAbsolute;
    headRadiusBBox = obj.headRadiusBBox;
    headHeightRatio = obj.headHeightRatio;
    opacityType = obj.opacityType;
    opacityVariable = obj.opacityVariable;
    opacity = obj.opacity;
    opacityVarMin = obj.opacityVarMin;
    opacityVarMax = obj.opacityVarMax;
    opacityVarMinFlag = obj.opacityVarMinFlag;
    opacityVarMaxFlag = obj.opacityVarMaxFlag;
    tubeDisplayDensity = obj.tubeDisplayDensity;
    geomDisplayQuality = obj.geomDisplayQuality;
    sampleDistance0 = obj.sampleDistance0;
    sampleDistance1 = obj.sampleDistance1;
    sampleDistance2 = obj.sampleDistance2;
    fillInterior = obj.fillInterior;
    randomSamples = obj.randomSamples;
    randomSeed = obj.randomSeed;
    numberOfRandomSamples = obj.numberOfRandomSamples;
    forceNodeCenteredData = obj.forceNodeCenteredData;
    issueTerminationWarnings = obj.issueTerminationWarnings;
    issueStiffnessWarnings = obj.issueStiffnessWarnings;
    issueCriticalPointsWarnings = obj.issueCriticalPointsWarnings;
    criticalPointThreshold = obj.criticalPointThreshold;

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
            pointSource_equal &&
            lineStart_equal &&
            lineEnd_equal &&
            planeOrigin_equal &&
            planeNormal_equal &&
            planeUpAxis_equal &&
            (radius == obj.radius) &&
            sphereOrigin_equal &&
            boxExtents_equal &&
            (useWholeBox == obj.useWholeBox) &&
            (pointList == obj.pointList) &&
            (sampleDensity0 == obj.sampleDensity0) &&
            (sampleDensity1 == obj.sampleDensity1) &&
            (sampleDensity2 == obj.sampleDensity2) &&
            (coloringMethod == obj.coloringMethod) &&
            (colorTableName == obj.colorTableName) &&
            (singleColor == obj.singleColor) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (streamlineDirection == obj.streamlineDirection) &&
            (maxSteps == obj.maxSteps) &&
            (terminateByDistance == obj.terminateByDistance) &&
            (termDistance == obj.termDistance) &&
            (terminateByTime == obj.terminateByTime) &&
            (termTime == obj.termTime) &&
            (maxStepLength == obj.maxStepLength) &&
            (limitMaximumTimestep == obj.limitMaximumTimestep) &&
            (maxTimeStep == obj.maxTimeStep) &&
            (relTol == obj.relTol) &&
            (absTolSizeType == obj.absTolSizeType) &&
            (absTolAbsolute == obj.absTolAbsolute) &&
            (absTolBBox == obj.absTolBBox) &&
            (integrationType == obj.integrationType) &&
            (streamlineAlgorithmType == obj.streamlineAlgorithmType) &&
            (maxStreamlineProcessCount == obj.maxStreamlineProcessCount) &&
            (maxDomainCacheSize == obj.maxDomainCacheSize) &&
            (workGroupSize == obj.workGroupSize) &&
            (pathlines == obj.pathlines) &&
            (pathlinesOverrideStartingTimeFlag == obj.pathlinesOverrideStartingTimeFlag) &&
            (pathlinesOverrideStartingTime == obj.pathlinesOverrideStartingTime) &&
            (pathlinesCMFE == obj.pathlinesCMFE) &&
            (coordinateSystem == obj.coordinateSystem) &&
            (phiFactor == obj.phiFactor) &&
            (coloringVariable == obj.coloringVariable) &&
            (legendMinFlag == obj.legendMinFlag) &&
            (legendMaxFlag == obj.legendMaxFlag) &&
            (legendMin == obj.legendMin) &&
            (legendMax == obj.legendMax) &&
            (displayBegin == obj.displayBegin) &&
            (displayEnd == obj.displayEnd) &&
            (displayBeginFlag == obj.displayBeginFlag) &&
            (displayEndFlag == obj.displayEndFlag) &&
            (referenceTypeForDisplay == obj.referenceTypeForDisplay) &&
            (displayMethod == obj.displayMethod) &&
            (tubeSizeType == obj.tubeSizeType) &&
            (tubeRadiusAbsolute == obj.tubeRadiusAbsolute) &&
            (tubeRadiusBBox == obj.tubeRadiusBBox) &&
            (ribbonWidthSizeType == obj.ribbonWidthSizeType) &&
            (ribbonWidthAbsolute == obj.ribbonWidthAbsolute) &&
            (ribbonWidthBBox == obj.ribbonWidthBBox) &&
            (lineWidth == obj.lineWidth) &&
            (showSeeds == obj.showSeeds) &&
            (seedRadiusSizeType == obj.seedRadiusSizeType) &&
            (seedRadiusAbsolute == obj.seedRadiusAbsolute) &&
            (seedRadiusBBox == obj.seedRadiusBBox) &&
            (showHeads == obj.showHeads) &&
            (headDisplayType == obj.headDisplayType) &&
            (headRadiusSizeType == obj.headRadiusSizeType) &&
            (headRadiusAbsolute == obj.headRadiusAbsolute) &&
            (headRadiusBBox == obj.headRadiusBBox) &&
            (headHeightRatio == obj.headHeightRatio) &&
            (opacityType == obj.opacityType) &&
            (opacityVariable == obj.opacityVariable) &&
            (opacity == obj.opacity) &&
            (opacityVarMin == obj.opacityVarMin) &&
            (opacityVarMax == obj.opacityVarMax) &&
            (opacityVarMinFlag == obj.opacityVarMinFlag) &&
            (opacityVarMaxFlag == obj.opacityVarMaxFlag) &&
            (tubeDisplayDensity == obj.tubeDisplayDensity) &&
            (geomDisplayQuality == obj.geomDisplayQuality) &&
            (sampleDistance0 == obj.sampleDistance0) &&
            (sampleDistance1 == obj.sampleDistance1) &&
            (sampleDistance2 == obj.sampleDistance2) &&
            (fillInterior == obj.fillInterior) &&
            (randomSamples == obj.randomSamples) &&
            (randomSeed == obj.randomSeed) &&
            (numberOfRandomSamples == obj.numberOfRandomSamples) &&
            (forceNodeCenteredData == obj.forceNodeCenteredData) &&
            (issueTerminationWarnings == obj.issueTerminationWarnings) &&
            (issueStiffnessWarnings == obj.issueStiffnessWarnings) &&
            (issueCriticalPointsWarnings == obj.issueCriticalPointsWarnings) &&
            (criticalPointThreshold == obj.criticalPointThreshold));
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
//    Dave Pugmire, Thu Jun 10 10:44:02 EDT 2010
//    New seed sources.
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
        if(sourceType == SpecifiedPlane || sourceType == SpecifiedCircle)
        {
            const PlaneAttributes *plane = (const PlaneAttributes *)atts;
            SetPlaneOrigin(plane->GetOrigin());
            SetPlaneNormal(plane->GetNormal());
            SetPlaneUpAxis(plane->GetUpAxis());
            if (sourceType == SpecifiedCircle)
                SetRadius(plane->GetRadius());
            retval = true;
        }
    }
    else if(atts->TypeName() == "SphereAttributes")
    {
        if(sourceType == SpecifiedSphere)
        {
            const SphereAttributes *sphere = (const SphereAttributes *)atts;
            SetSphereOrigin(sphere->GetOrigin());
            SetRadius(sphere->GetRadius());
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
        p->SetRadius(GetRadius());
        p->SetHaveRadius(true);
        retval = p;
    }
    else if(tname == "SphereAttributes")
    {
        SphereAttributes *s = new SphereAttributes;
        s->SetOrigin(GetSphereOrigin());
        s->SetRadius(GetRadius());
        retval = s;
    }
    else if(tname == "CircleAttributes")
    {
        PlaneAttributes *p = new PlaneAttributes;
        p->SetOrigin(GetPlaneOrigin());
        p->SetNormal(GetPlaneNormal());
        p->SetUpAxis(GetPlaneUpAxis());
        p->SetRadius(GetRadius());
        p->SetHaveRadius(true);
        retval = p;
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
    Select(ID_sourceType,                        (void *)&sourceType);
    Select(ID_pointSource,                       (void *)pointSource, 3);
    Select(ID_lineStart,                         (void *)lineStart, 3);
    Select(ID_lineEnd,                           (void *)lineEnd, 3);
    Select(ID_planeOrigin,                       (void *)planeOrigin, 3);
    Select(ID_planeNormal,                       (void *)planeNormal, 3);
    Select(ID_planeUpAxis,                       (void *)planeUpAxis, 3);
    Select(ID_radius,                            (void *)&radius);
    Select(ID_sphereOrigin,                      (void *)sphereOrigin, 3);
    Select(ID_boxExtents,                        (void *)boxExtents, 6);
    Select(ID_useWholeBox,                       (void *)&useWholeBox);
    Select(ID_pointList,                         (void *)&pointList);
    Select(ID_sampleDensity0,                    (void *)&sampleDensity0);
    Select(ID_sampleDensity1,                    (void *)&sampleDensity1);
    Select(ID_sampleDensity2,                    (void *)&sampleDensity2);
    Select(ID_coloringMethod,                    (void *)&coloringMethod);
    Select(ID_colorTableName,                    (void *)&colorTableName);
    Select(ID_singleColor,                       (void *)&singleColor);
    Select(ID_legendFlag,                        (void *)&legendFlag);
    Select(ID_lightingFlag,                      (void *)&lightingFlag);
    Select(ID_streamlineDirection,               (void *)&streamlineDirection);
    Select(ID_maxSteps,                          (void *)&maxSteps);
    Select(ID_terminateByDistance,               (void *)&terminateByDistance);
    Select(ID_termDistance,                      (void *)&termDistance);
    Select(ID_terminateByTime,                   (void *)&terminateByTime);
    Select(ID_termTime,                          (void *)&termTime);
    Select(ID_maxStepLength,                     (void *)&maxStepLength);
    Select(ID_limitMaximumTimestep,              (void *)&limitMaximumTimestep);
    Select(ID_maxTimeStep,                       (void *)&maxTimeStep);
    Select(ID_relTol,                            (void *)&relTol);
    Select(ID_absTolSizeType,                    (void *)&absTolSizeType);
    Select(ID_absTolAbsolute,                    (void *)&absTolAbsolute);
    Select(ID_absTolBBox,                        (void *)&absTolBBox);
    Select(ID_integrationType,                   (void *)&integrationType);
    Select(ID_streamlineAlgorithmType,           (void *)&streamlineAlgorithmType);
    Select(ID_maxStreamlineProcessCount,         (void *)&maxStreamlineProcessCount);
    Select(ID_maxDomainCacheSize,                (void *)&maxDomainCacheSize);
    Select(ID_workGroupSize,                     (void *)&workGroupSize);
    Select(ID_pathlines,                         (void *)&pathlines);
    Select(ID_pathlinesOverrideStartingTimeFlag, (void *)&pathlinesOverrideStartingTimeFlag);
    Select(ID_pathlinesOverrideStartingTime,     (void *)&pathlinesOverrideStartingTime);
    Select(ID_pathlinesCMFE,                     (void *)&pathlinesCMFE);
    Select(ID_coordinateSystem,                  (void *)&coordinateSystem);
    Select(ID_phiFactor,                         (void *)&phiFactor);
    Select(ID_coloringVariable,                  (void *)&coloringVariable);
    Select(ID_legendMinFlag,                     (void *)&legendMinFlag);
    Select(ID_legendMaxFlag,                     (void *)&legendMaxFlag);
    Select(ID_legendMin,                         (void *)&legendMin);
    Select(ID_legendMax,                         (void *)&legendMax);
    Select(ID_displayBegin,                      (void *)&displayBegin);
    Select(ID_displayEnd,                        (void *)&displayEnd);
    Select(ID_displayBeginFlag,                  (void *)&displayBeginFlag);
    Select(ID_displayEndFlag,                    (void *)&displayEndFlag);
    Select(ID_referenceTypeForDisplay,           (void *)&referenceTypeForDisplay);
    Select(ID_displayMethod,                     (void *)&displayMethod);
    Select(ID_tubeSizeType,                      (void *)&tubeSizeType);
    Select(ID_tubeRadiusAbsolute,                (void *)&tubeRadiusAbsolute);
    Select(ID_tubeRadiusBBox,                    (void *)&tubeRadiusBBox);
    Select(ID_ribbonWidthSizeType,               (void *)&ribbonWidthSizeType);
    Select(ID_ribbonWidthAbsolute,               (void *)&ribbonWidthAbsolute);
    Select(ID_ribbonWidthBBox,                   (void *)&ribbonWidthBBox);
    Select(ID_lineWidth,                         (void *)&lineWidth);
    Select(ID_showSeeds,                         (void *)&showSeeds);
    Select(ID_seedRadiusSizeType,                (void *)&seedRadiusSizeType);
    Select(ID_seedRadiusAbsolute,                (void *)&seedRadiusAbsolute);
    Select(ID_seedRadiusBBox,                    (void *)&seedRadiusBBox);
    Select(ID_showHeads,                         (void *)&showHeads);
    Select(ID_headDisplayType,                   (void *)&headDisplayType);
    Select(ID_headRadiusSizeType,                (void *)&headRadiusSizeType);
    Select(ID_headRadiusAbsolute,                (void *)&headRadiusAbsolute);
    Select(ID_headRadiusBBox,                    (void *)&headRadiusBBox);
    Select(ID_headHeightRatio,                   (void *)&headHeightRatio);
    Select(ID_opacityType,                       (void *)&opacityType);
    Select(ID_opacityVariable,                   (void *)&opacityVariable);
    Select(ID_opacity,                           (void *)&opacity);
    Select(ID_opacityVarMin,                     (void *)&opacityVarMin);
    Select(ID_opacityVarMax,                     (void *)&opacityVarMax);
    Select(ID_opacityVarMinFlag,                 (void *)&opacityVarMinFlag);
    Select(ID_opacityVarMaxFlag,                 (void *)&opacityVarMaxFlag);
    Select(ID_tubeDisplayDensity,                (void *)&tubeDisplayDensity);
    Select(ID_geomDisplayQuality,                (void *)&geomDisplayQuality);
    Select(ID_sampleDistance0,                   (void *)&sampleDistance0);
    Select(ID_sampleDistance1,                   (void *)&sampleDistance1);
    Select(ID_sampleDistance2,                   (void *)&sampleDistance2);
    Select(ID_fillInterior,                      (void *)&fillInterior);
    Select(ID_randomSamples,                     (void *)&randomSamples);
    Select(ID_randomSeed,                        (void *)&randomSeed);
    Select(ID_numberOfRandomSamples,             (void *)&numberOfRandomSamples);
    Select(ID_forceNodeCenteredData,             (void *)&forceNodeCenteredData);
    Select(ID_issueTerminationWarnings,          (void *)&issueTerminationWarnings);
    Select(ID_issueStiffnessWarnings,            (void *)&issueStiffnessWarnings);
    Select(ID_issueCriticalPointsWarnings,       (void *)&issueCriticalPointsWarnings);
    Select(ID_criticalPointThreshold,            (void *)&criticalPointThreshold);
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

    if(completeSave || !FieldsEqual(ID_radius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("radius", radius));
    }

    if(completeSave || !FieldsEqual(ID_sphereOrigin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sphereOrigin", sphereOrigin, 3));
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

    if(completeSave || !FieldsEqual(ID_sampleDensity0, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleDensity0", sampleDensity0));
    }

    if(completeSave || !FieldsEqual(ID_sampleDensity1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleDensity1", sampleDensity1));
    }

    if(completeSave || !FieldsEqual(ID_sampleDensity2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleDensity2", sampleDensity2));
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

    if(completeSave || !FieldsEqual(ID_streamlineDirection, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("streamlineDirection", IntegrationDirection_ToString(streamlineDirection)));
    }

    if(completeSave || !FieldsEqual(ID_maxSteps, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxSteps", maxSteps));
    }

    if(completeSave || !FieldsEqual(ID_terminateByDistance, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminateByDistance", terminateByDistance));
    }

    if(completeSave || !FieldsEqual(ID_termDistance, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("termDistance", termDistance));
    }

    if(completeSave || !FieldsEqual(ID_terminateByTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("terminateByTime", terminateByTime));
    }

    if(completeSave || !FieldsEqual(ID_termTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("termTime", termTime));
    }

    if(completeSave || !FieldsEqual(ID_maxStepLength, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxStepLength", maxStepLength));
    }

    if(completeSave || !FieldsEqual(ID_limitMaximumTimestep, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("limitMaximumTimestep", limitMaximumTimestep));
    }

    if(completeSave || !FieldsEqual(ID_maxTimeStep, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxTimeStep", maxTimeStep));
    }

    if(completeSave || !FieldsEqual(ID_relTol, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("relTol", relTol));
    }

    if(completeSave || !FieldsEqual(ID_absTolSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("absTolSizeType", SizeType_ToString(absTolSizeType)));
    }

    if(completeSave || !FieldsEqual(ID_absTolAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("absTolAbsolute", absTolAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_absTolBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("absTolBBox", absTolBBox));
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

    if(completeSave || !FieldsEqual(ID_pathlinesOverrideStartingTimeFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesOverrideStartingTimeFlag", pathlinesOverrideStartingTimeFlag));
    }

    if(completeSave || !FieldsEqual(ID_pathlinesOverrideStartingTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesOverrideStartingTime", pathlinesOverrideStartingTime));
    }

    if(completeSave || !FieldsEqual(ID_pathlinesCMFE, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesCMFE", PathlinesCMFE_ToString(pathlinesCMFE)));
    }

    if(completeSave || !FieldsEqual(ID_coordinateSystem, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("coordinateSystem", CoordinateSystem_ToString(coordinateSystem)));
    }

    if(completeSave || !FieldsEqual(ID_phiFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("phiFactor", phiFactor));
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

    if(completeSave || !FieldsEqual(ID_displayBegin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayBegin", displayBegin));
    }

    if(completeSave || !FieldsEqual(ID_displayEnd, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayEnd", displayEnd));
    }

    if(completeSave || !FieldsEqual(ID_displayBeginFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayBeginFlag", displayBeginFlag));
    }

    if(completeSave || !FieldsEqual(ID_displayEndFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayEndFlag", displayEndFlag));
    }

    if(completeSave || !FieldsEqual(ID_referenceTypeForDisplay, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("referenceTypeForDisplay", ReferenceType_ToString(referenceTypeForDisplay)));
    }

    if(completeSave || !FieldsEqual(ID_displayMethod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayMethod", DisplayMethod_ToString(displayMethod)));
    }

    if(completeSave || !FieldsEqual(ID_tubeSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeSizeType", SizeType_ToString(tubeSizeType)));
    }

    if(completeSave || !FieldsEqual(ID_tubeRadiusAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeRadiusAbsolute", tubeRadiusAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_tubeRadiusBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeRadiusBBox", tubeRadiusBBox));
    }

    if(completeSave || !FieldsEqual(ID_ribbonWidthSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ribbonWidthSizeType", SizeType_ToString(ribbonWidthSizeType)));
    }

    if(completeSave || !FieldsEqual(ID_ribbonWidthAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ribbonWidthAbsolute", ribbonWidthAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_ribbonWidthBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ribbonWidthBBox", ribbonWidthBBox));
    }

    if(completeSave || !FieldsEqual(ID_lineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineWidth", lineWidth));
    }

    if(completeSave || !FieldsEqual(ID_showSeeds, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showSeeds", showSeeds));
    }

    if(completeSave || !FieldsEqual(ID_seedRadiusSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("seedRadiusSizeType", SizeType_ToString(seedRadiusSizeType)));
    }

    if(completeSave || !FieldsEqual(ID_seedRadiusAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("seedRadiusAbsolute", seedRadiusAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_seedRadiusBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("seedRadiusBBox", seedRadiusBBox));
    }

    if(completeSave || !FieldsEqual(ID_showHeads, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showHeads", showHeads));
    }

    if(completeSave || !FieldsEqual(ID_headDisplayType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("headDisplayType", GeomDisplayType_ToString(headDisplayType)));
    }

    if(completeSave || !FieldsEqual(ID_headRadiusSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("headRadiusSizeType", SizeType_ToString(headRadiusSizeType)));
    }

    if(completeSave || !FieldsEqual(ID_headRadiusAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("headRadiusAbsolute", headRadiusAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_headRadiusBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("headRadiusBBox", headRadiusBBox));
    }

    if(completeSave || !FieldsEqual(ID_headHeightRatio, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("headHeightRatio", headHeightRatio));
    }

    if(completeSave || !FieldsEqual(ID_opacityType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityType", OpacityType_ToString(opacityType)));
    }

    if(completeSave || !FieldsEqual(ID_opacityVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVariable", opacityVariable));
    }

    if(completeSave || !FieldsEqual(ID_opacity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacity", opacity));
    }

    if(completeSave || !FieldsEqual(ID_opacityVarMin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVarMin", opacityVarMin));
    }

    if(completeSave || !FieldsEqual(ID_opacityVarMax, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVarMax", opacityVarMax));
    }

    if(completeSave || !FieldsEqual(ID_opacityVarMinFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVarMinFlag", opacityVarMinFlag));
    }

    if(completeSave || !FieldsEqual(ID_opacityVarMaxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVarMaxFlag", opacityVarMaxFlag));
    }

    if(completeSave || !FieldsEqual(ID_tubeDisplayDensity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeDisplayDensity", tubeDisplayDensity));
    }

    if(completeSave || !FieldsEqual(ID_geomDisplayQuality, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("geomDisplayQuality", DisplayQuality_ToString(geomDisplayQuality)));
    }

    if(completeSave || !FieldsEqual(ID_sampleDistance0, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleDistance0", sampleDistance0));
    }

    if(completeSave || !FieldsEqual(ID_sampleDistance1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleDistance1", sampleDistance1));
    }

    if(completeSave || !FieldsEqual(ID_sampleDistance2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleDistance2", sampleDistance2));
    }

    if(completeSave || !FieldsEqual(ID_fillInterior, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("fillInterior", fillInterior));
    }

    if(completeSave || !FieldsEqual(ID_randomSamples, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("randomSamples", randomSamples));
    }

    if(completeSave || !FieldsEqual(ID_randomSeed, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("randomSeed", randomSeed));
    }

    if(completeSave || !FieldsEqual(ID_numberOfRandomSamples, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numberOfRandomSamples", numberOfRandomSamples));
    }

    if(completeSave || !FieldsEqual(ID_forceNodeCenteredData, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("forceNodeCenteredData", forceNodeCenteredData));
    }

    if(completeSave || !FieldsEqual(ID_issueTerminationWarnings, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("issueTerminationWarnings", issueTerminationWarnings));
    }

    if(completeSave || !FieldsEqual(ID_issueStiffnessWarnings, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("issueStiffnessWarnings", issueStiffnessWarnings));
    }

    if(completeSave || !FieldsEqual(ID_issueCriticalPointsWarnings, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("issueCriticalPointsWarnings", issueCriticalPointsWarnings));
    }

    if(completeSave || !FieldsEqual(ID_criticalPointThreshold, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("criticalPointThreshold", criticalPointThreshold));
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
            if(ival >= 0 && ival < 7)
                SetSourceType(SourceType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SourceType value;
            if(SourceType_FromString(node->AsString(), value))
                SetSourceType(value);
        }
    }
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
    if((node = searchNode->GetNode("radius")) != 0)
        SetRadius(node->AsDouble());
    if((node = searchNode->GetNode("sphereOrigin")) != 0)
        SetSphereOrigin(node->AsDoubleArray());
    if((node = searchNode->GetNode("boxExtents")) != 0)
        SetBoxExtents(node->AsDoubleArray());
    if((node = searchNode->GetNode("useWholeBox")) != 0)
        SetUseWholeBox(node->AsBool());
    if((node = searchNode->GetNode("pointList")) != 0)
        SetPointList(node->AsDoubleVector());
    if((node = searchNode->GetNode("sampleDensity0")) != 0)
        SetSampleDensity0(node->AsInt());
    if((node = searchNode->GetNode("sampleDensity1")) != 0)
        SetSampleDensity1(node->AsInt());
    if((node = searchNode->GetNode("sampleDensity2")) != 0)
        SetSampleDensity2(node->AsInt());
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
    if((node = searchNode->GetNode("streamlineDirection")) != 0)
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
    if((node = searchNode->GetNode("maxSteps")) != 0)
        SetMaxSteps(node->AsInt());
    if((node = searchNode->GetNode("terminateByDistance")) != 0)
        SetTerminateByDistance(node->AsBool());
    if((node = searchNode->GetNode("termDistance")) != 0)
        SetTermDistance(node->AsDouble());
    if((node = searchNode->GetNode("terminateByTime")) != 0)
        SetTerminateByTime(node->AsBool());
    if((node = searchNode->GetNode("termTime")) != 0)
        SetTermTime(node->AsDouble());
    if((node = searchNode->GetNode("maxStepLength")) != 0)
        SetMaxStepLength(node->AsDouble());
    if((node = searchNode->GetNode("limitMaximumTimestep")) != 0)
        SetLimitMaximumTimestep(node->AsBool());
    if((node = searchNode->GetNode("maxTimeStep")) != 0)
        SetMaxTimeStep(node->AsDouble());
    if((node = searchNode->GetNode("relTol")) != 0)
        SetRelTol(node->AsDouble());
    if((node = searchNode->GetNode("absTolSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetAbsTolSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetAbsTolSizeType(value);
        }
    }
    if((node = searchNode->GetNode("absTolAbsolute")) != 0)
        SetAbsTolAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("absTolBBox")) != 0)
        SetAbsTolBBox(node->AsDouble());
    if((node = searchNode->GetNode("integrationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
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
            if(ival >= 0 && ival < 4)
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
    if((node = searchNode->GetNode("pathlinesOverrideStartingTimeFlag")) != 0)
        SetPathlinesOverrideStartingTimeFlag(node->AsBool());
    if((node = searchNode->GetNode("pathlinesOverrideStartingTime")) != 0)
        SetPathlinesOverrideStartingTime(node->AsDouble());
    if((node = searchNode->GetNode("pathlinesCMFE")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetPathlinesCMFE(PathlinesCMFE(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PathlinesCMFE value;
            if(PathlinesCMFE_FromString(node->AsString(), value))
                SetPathlinesCMFE(value);
        }
    }
    if((node = searchNode->GetNode("coordinateSystem")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetCoordinateSystem(CoordinateSystem(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CoordinateSystem value;
            if(CoordinateSystem_FromString(node->AsString(), value))
                SetCoordinateSystem(value);
        }
    }
    if((node = searchNode->GetNode("phiFactor")) != 0)
        SetPhiFactor(node->AsDouble());
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
    if((node = searchNode->GetNode("displayBegin")) != 0)
        SetDisplayBegin(node->AsDouble());
    if((node = searchNode->GetNode("displayEnd")) != 0)
        SetDisplayEnd(node->AsDouble());
    if((node = searchNode->GetNode("displayBeginFlag")) != 0)
        SetDisplayBeginFlag(node->AsBool());
    if((node = searchNode->GetNode("displayEndFlag")) != 0)
        SetDisplayEndFlag(node->AsBool());
    if((node = searchNode->GetNode("referenceTypeForDisplay")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetReferenceTypeForDisplay(ReferenceType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ReferenceType value;
            if(ReferenceType_FromString(node->AsString(), value))
                SetReferenceTypeForDisplay(value);
        }
    }
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
    if((node = searchNode->GetNode("tubeSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetTubeSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetTubeSizeType(value);
        }
    }
    if((node = searchNode->GetNode("tubeRadiusAbsolute")) != 0)
        SetTubeRadiusAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("tubeRadiusBBox")) != 0)
        SetTubeRadiusBBox(node->AsDouble());
    if((node = searchNode->GetNode("ribbonWidthSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetRibbonWidthSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetRibbonWidthSizeType(value);
        }
    }
    if((node = searchNode->GetNode("ribbonWidthAbsolute")) != 0)
        SetRibbonWidthAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("ribbonWidthBBox")) != 0)
        SetRibbonWidthBBox(node->AsDouble());
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("showSeeds")) != 0)
        SetShowSeeds(node->AsBool());
    if((node = searchNode->GetNode("seedRadiusSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetSeedRadiusSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetSeedRadiusSizeType(value);
        }
    }
    if((node = searchNode->GetNode("seedRadiusAbsolute")) != 0)
        SetSeedRadiusAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("seedRadiusBBox")) != 0)
        SetSeedRadiusBBox(node->AsDouble());
    if((node = searchNode->GetNode("showHeads")) != 0)
        SetShowHeads(node->AsBool());
    if((node = searchNode->GetNode("headDisplayType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetHeadDisplayType(GeomDisplayType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            GeomDisplayType value;
            if(GeomDisplayType_FromString(node->AsString(), value))
                SetHeadDisplayType(value);
        }
    }
    if((node = searchNode->GetNode("headRadiusSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetHeadRadiusSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetHeadRadiusSizeType(value);
        }
    }
    if((node = searchNode->GetNode("headRadiusAbsolute")) != 0)
        SetHeadRadiusAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("headRadiusBBox")) != 0)
        SetHeadRadiusBBox(node->AsDouble());
    if((node = searchNode->GetNode("headHeightRatio")) != 0)
        SetHeadHeightRatio(node->AsDouble());
    if((node = searchNode->GetNode("opacityType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetOpacityType(OpacityType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OpacityType value;
            if(OpacityType_FromString(node->AsString(), value))
                SetOpacityType(value);
        }
    }
    if((node = searchNode->GetNode("opacityVariable")) != 0)
        SetOpacityVariable(node->AsString());
    if((node = searchNode->GetNode("opacity")) != 0)
        SetOpacity(node->AsDouble());
    if((node = searchNode->GetNode("opacityVarMin")) != 0)
        SetOpacityVarMin(node->AsDouble());
    if((node = searchNode->GetNode("opacityVarMax")) != 0)
        SetOpacityVarMax(node->AsDouble());
    if((node = searchNode->GetNode("opacityVarMinFlag")) != 0)
        SetOpacityVarMinFlag(node->AsBool());
    if((node = searchNode->GetNode("opacityVarMaxFlag")) != 0)
        SetOpacityVarMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("tubeDisplayDensity")) != 0)
        SetTubeDisplayDensity(node->AsInt());
    if((node = searchNode->GetNode("geomDisplayQuality")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetGeomDisplayQuality(DisplayQuality(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DisplayQuality value;
            if(DisplayQuality_FromString(node->AsString(), value))
                SetGeomDisplayQuality(value);
        }
    }
    if((node = searchNode->GetNode("sampleDistance0")) != 0)
        SetSampleDistance0(node->AsDouble());
    if((node = searchNode->GetNode("sampleDistance1")) != 0)
        SetSampleDistance1(node->AsDouble());
    if((node = searchNode->GetNode("sampleDistance2")) != 0)
        SetSampleDistance2(node->AsDouble());
    if((node = searchNode->GetNode("fillInterior")) != 0)
        SetFillInterior(node->AsBool());
    if((node = searchNode->GetNode("randomSamples")) != 0)
        SetRandomSamples(node->AsBool());
    if((node = searchNode->GetNode("randomSeed")) != 0)
        SetRandomSeed(node->AsInt());
    if((node = searchNode->GetNode("numberOfRandomSamples")) != 0)
        SetNumberOfRandomSamples(node->AsInt());
    if((node = searchNode->GetNode("forceNodeCenteredData")) != 0)
        SetForceNodeCenteredData(node->AsBool());
    if((node = searchNode->GetNode("issueTerminationWarnings")) != 0)
        SetIssueTerminationWarnings(node->AsBool());
    if((node = searchNode->GetNode("issueStiffnessWarnings")) != 0)
        SetIssueStiffnessWarnings(node->AsBool());
    if((node = searchNode->GetNode("issueCriticalPointsWarnings")) != 0)
        SetIssueCriticalPointsWarnings(node->AsBool());
    if((node = searchNode->GetNode("criticalPointThreshold")) != 0)
        SetCriticalPointThreshold(node->AsDouble());
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
StreamlineAttributes::SetRadius(double radius_)
{
    radius = radius_;
    Select(ID_radius, (void *)&radius);
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
StreamlineAttributes::SetSampleDensity0(int sampleDensity0_)
{
    sampleDensity0 = sampleDensity0_;
    Select(ID_sampleDensity0, (void *)&sampleDensity0);
}

void
StreamlineAttributes::SetSampleDensity1(int sampleDensity1_)
{
    sampleDensity1 = sampleDensity1_;
    Select(ID_sampleDensity1, (void *)&sampleDensity1);
}

void
StreamlineAttributes::SetSampleDensity2(int sampleDensity2_)
{
    sampleDensity2 = sampleDensity2_;
    Select(ID_sampleDensity2, (void *)&sampleDensity2);
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
StreamlineAttributes::SetStreamlineDirection(StreamlineAttributes::IntegrationDirection streamlineDirection_)
{
    streamlineDirection = streamlineDirection_;
    Select(ID_streamlineDirection, (void *)&streamlineDirection);
}

void
StreamlineAttributes::SetMaxSteps(int maxSteps_)
{
    maxSteps = maxSteps_;
    Select(ID_maxSteps, (void *)&maxSteps);
}

void
StreamlineAttributes::SetTerminateByDistance(bool terminateByDistance_)
{
    terminateByDistance = terminateByDistance_;
    Select(ID_terminateByDistance, (void *)&terminateByDistance);
}

void
StreamlineAttributes::SetTermDistance(double termDistance_)
{
    termDistance = termDistance_;
    Select(ID_termDistance, (void *)&termDistance);
}

void
StreamlineAttributes::SetTerminateByTime(bool terminateByTime_)
{
    terminateByTime = terminateByTime_;
    Select(ID_terminateByTime, (void *)&terminateByTime);
}

void
StreamlineAttributes::SetTermTime(double termTime_)
{
    termTime = termTime_;
    Select(ID_termTime, (void *)&termTime);
}

void
StreamlineAttributes::SetMaxStepLength(double maxStepLength_)
{
    maxStepLength = maxStepLength_;
    Select(ID_maxStepLength, (void *)&maxStepLength);
}

void
StreamlineAttributes::SetLimitMaximumTimestep(bool limitMaximumTimestep_)
{
    limitMaximumTimestep = limitMaximumTimestep_;
    Select(ID_limitMaximumTimestep, (void *)&limitMaximumTimestep);
}

void
StreamlineAttributes::SetMaxTimeStep(double maxTimeStep_)
{
    maxTimeStep = maxTimeStep_;
    Select(ID_maxTimeStep, (void *)&maxTimeStep);
}

void
StreamlineAttributes::SetRelTol(double relTol_)
{
    relTol = relTol_;
    Select(ID_relTol, (void *)&relTol);
}

void
StreamlineAttributes::SetAbsTolSizeType(StreamlineAttributes::SizeType absTolSizeType_)
{
    absTolSizeType = absTolSizeType_;
    Select(ID_absTolSizeType, (void *)&absTolSizeType);
}

void
StreamlineAttributes::SetAbsTolAbsolute(double absTolAbsolute_)
{
    absTolAbsolute = absTolAbsolute_;
    Select(ID_absTolAbsolute, (void *)&absTolAbsolute);
}

void
StreamlineAttributes::SetAbsTolBBox(double absTolBBox_)
{
    absTolBBox = absTolBBox_;
    Select(ID_absTolBBox, (void *)&absTolBBox);
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
StreamlineAttributes::SetPathlinesOverrideStartingTimeFlag(bool pathlinesOverrideStartingTimeFlag_)
{
    pathlinesOverrideStartingTimeFlag = pathlinesOverrideStartingTimeFlag_;
    Select(ID_pathlinesOverrideStartingTimeFlag, (void *)&pathlinesOverrideStartingTimeFlag);
}

void
StreamlineAttributes::SetPathlinesOverrideStartingTime(double pathlinesOverrideStartingTime_)
{
    pathlinesOverrideStartingTime = pathlinesOverrideStartingTime_;
    Select(ID_pathlinesOverrideStartingTime, (void *)&pathlinesOverrideStartingTime);
}

void
StreamlineAttributes::SetPathlinesCMFE(StreamlineAttributes::PathlinesCMFE pathlinesCMFE_)
{
    pathlinesCMFE = pathlinesCMFE_;
    Select(ID_pathlinesCMFE, (void *)&pathlinesCMFE);
}

void
StreamlineAttributes::SetCoordinateSystem(StreamlineAttributes::CoordinateSystem coordinateSystem_)
{
    coordinateSystem = coordinateSystem_;
    Select(ID_coordinateSystem, (void *)&coordinateSystem);
}

void
StreamlineAttributes::SetPhiFactor(double phiFactor_)
{
    phiFactor = phiFactor_;
    Select(ID_phiFactor, (void *)&phiFactor);
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

void
StreamlineAttributes::SetDisplayBegin(double displayBegin_)
{
    displayBegin = displayBegin_;
    Select(ID_displayBegin, (void *)&displayBegin);
}

void
StreamlineAttributes::SetDisplayEnd(double displayEnd_)
{
    displayEnd = displayEnd_;
    Select(ID_displayEnd, (void *)&displayEnd);
}

void
StreamlineAttributes::SetDisplayBeginFlag(bool displayBeginFlag_)
{
    displayBeginFlag = displayBeginFlag_;
    Select(ID_displayBeginFlag, (void *)&displayBeginFlag);
}

void
StreamlineAttributes::SetDisplayEndFlag(bool displayEndFlag_)
{
    displayEndFlag = displayEndFlag_;
    Select(ID_displayEndFlag, (void *)&displayEndFlag);
}

void
StreamlineAttributes::SetReferenceTypeForDisplay(StreamlineAttributes::ReferenceType referenceTypeForDisplay_)
{
    referenceTypeForDisplay = referenceTypeForDisplay_;
    Select(ID_referenceTypeForDisplay, (void *)&referenceTypeForDisplay);
}

void
StreamlineAttributes::SetDisplayMethod(StreamlineAttributes::DisplayMethod displayMethod_)
{
    displayMethod = displayMethod_;
    Select(ID_displayMethod, (void *)&displayMethod);
}

void
StreamlineAttributes::SetTubeSizeType(StreamlineAttributes::SizeType tubeSizeType_)
{
    tubeSizeType = tubeSizeType_;
    Select(ID_tubeSizeType, (void *)&tubeSizeType);
}

void
StreamlineAttributes::SetTubeRadiusAbsolute(double tubeRadiusAbsolute_)
{
    tubeRadiusAbsolute = tubeRadiusAbsolute_;
    Select(ID_tubeRadiusAbsolute, (void *)&tubeRadiusAbsolute);
}

void
StreamlineAttributes::SetTubeRadiusBBox(double tubeRadiusBBox_)
{
    tubeRadiusBBox = tubeRadiusBBox_;
    Select(ID_tubeRadiusBBox, (void *)&tubeRadiusBBox);
}

void
StreamlineAttributes::SetRibbonWidthSizeType(StreamlineAttributes::SizeType ribbonWidthSizeType_)
{
    ribbonWidthSizeType = ribbonWidthSizeType_;
    Select(ID_ribbonWidthSizeType, (void *)&ribbonWidthSizeType);
}

void
StreamlineAttributes::SetRibbonWidthAbsolute(double ribbonWidthAbsolute_)
{
    ribbonWidthAbsolute = ribbonWidthAbsolute_;
    Select(ID_ribbonWidthAbsolute, (void *)&ribbonWidthAbsolute);
}

void
StreamlineAttributes::SetRibbonWidthBBox(double ribbonWidthBBox_)
{
    ribbonWidthBBox = ribbonWidthBBox_;
    Select(ID_ribbonWidthBBox, (void *)&ribbonWidthBBox);
}

void
StreamlineAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
StreamlineAttributes::SetShowSeeds(bool showSeeds_)
{
    showSeeds = showSeeds_;
    Select(ID_showSeeds, (void *)&showSeeds);
}

void
StreamlineAttributes::SetSeedRadiusSizeType(StreamlineAttributes::SizeType seedRadiusSizeType_)
{
    seedRadiusSizeType = seedRadiusSizeType_;
    Select(ID_seedRadiusSizeType, (void *)&seedRadiusSizeType);
}

void
StreamlineAttributes::SetSeedRadiusAbsolute(double seedRadiusAbsolute_)
{
    seedRadiusAbsolute = seedRadiusAbsolute_;
    Select(ID_seedRadiusAbsolute, (void *)&seedRadiusAbsolute);
}

void
StreamlineAttributes::SetSeedRadiusBBox(double seedRadiusBBox_)
{
    seedRadiusBBox = seedRadiusBBox_;
    Select(ID_seedRadiusBBox, (void *)&seedRadiusBBox);
}

void
StreamlineAttributes::SetShowHeads(bool showHeads_)
{
    showHeads = showHeads_;
    Select(ID_showHeads, (void *)&showHeads);
}

void
StreamlineAttributes::SetHeadDisplayType(StreamlineAttributes::GeomDisplayType headDisplayType_)
{
    headDisplayType = headDisplayType_;
    Select(ID_headDisplayType, (void *)&headDisplayType);
}

void
StreamlineAttributes::SetHeadRadiusSizeType(StreamlineAttributes::SizeType headRadiusSizeType_)
{
    headRadiusSizeType = headRadiusSizeType_;
    Select(ID_headRadiusSizeType, (void *)&headRadiusSizeType);
}

void
StreamlineAttributes::SetHeadRadiusAbsolute(double headRadiusAbsolute_)
{
    headRadiusAbsolute = headRadiusAbsolute_;
    Select(ID_headRadiusAbsolute, (void *)&headRadiusAbsolute);
}

void
StreamlineAttributes::SetHeadRadiusBBox(double headRadiusBBox_)
{
    headRadiusBBox = headRadiusBBox_;
    Select(ID_headRadiusBBox, (void *)&headRadiusBBox);
}

void
StreamlineAttributes::SetHeadHeightRatio(double headHeightRatio_)
{
    headHeightRatio = headHeightRatio_;
    Select(ID_headHeightRatio, (void *)&headHeightRatio);
}

void
StreamlineAttributes::SetOpacityType(StreamlineAttributes::OpacityType opacityType_)
{
    opacityType = opacityType_;
    Select(ID_opacityType, (void *)&opacityType);
}

void
StreamlineAttributes::SetOpacityVariable(const std::string &opacityVariable_)
{
    opacityVariable = opacityVariable_;
    Select(ID_opacityVariable, (void *)&opacityVariable);
}

void
StreamlineAttributes::SetOpacity(double opacity_)
{
    opacity = opacity_;
    Select(ID_opacity, (void *)&opacity);
}

void
StreamlineAttributes::SetOpacityVarMin(double opacityVarMin_)
{
    opacityVarMin = opacityVarMin_;
    Select(ID_opacityVarMin, (void *)&opacityVarMin);
}

void
StreamlineAttributes::SetOpacityVarMax(double opacityVarMax_)
{
    opacityVarMax = opacityVarMax_;
    Select(ID_opacityVarMax, (void *)&opacityVarMax);
}

void
StreamlineAttributes::SetOpacityVarMinFlag(bool opacityVarMinFlag_)
{
    opacityVarMinFlag = opacityVarMinFlag_;
    Select(ID_opacityVarMinFlag, (void *)&opacityVarMinFlag);
}

void
StreamlineAttributes::SetOpacityVarMaxFlag(bool opacityVarMaxFlag_)
{
    opacityVarMaxFlag = opacityVarMaxFlag_;
    Select(ID_opacityVarMaxFlag, (void *)&opacityVarMaxFlag);
}

void
StreamlineAttributes::SetTubeDisplayDensity(int tubeDisplayDensity_)
{
    tubeDisplayDensity = tubeDisplayDensity_;
    Select(ID_tubeDisplayDensity, (void *)&tubeDisplayDensity);
}

void
StreamlineAttributes::SetGeomDisplayQuality(StreamlineAttributes::DisplayQuality geomDisplayQuality_)
{
    geomDisplayQuality = geomDisplayQuality_;
    Select(ID_geomDisplayQuality, (void *)&geomDisplayQuality);
}

void
StreamlineAttributes::SetSampleDistance0(double sampleDistance0_)
{
    sampleDistance0 = sampleDistance0_;
    Select(ID_sampleDistance0, (void *)&sampleDistance0);
}

void
StreamlineAttributes::SetSampleDistance1(double sampleDistance1_)
{
    sampleDistance1 = sampleDistance1_;
    Select(ID_sampleDistance1, (void *)&sampleDistance1);
}

void
StreamlineAttributes::SetSampleDistance2(double sampleDistance2_)
{
    sampleDistance2 = sampleDistance2_;
    Select(ID_sampleDistance2, (void *)&sampleDistance2);
}

void
StreamlineAttributes::SetFillInterior(bool fillInterior_)
{
    fillInterior = fillInterior_;
    Select(ID_fillInterior, (void *)&fillInterior);
}

void
StreamlineAttributes::SetRandomSamples(bool randomSamples_)
{
    randomSamples = randomSamples_;
    Select(ID_randomSamples, (void *)&randomSamples);
}

void
StreamlineAttributes::SetRandomSeed(int randomSeed_)
{
    randomSeed = randomSeed_;
    Select(ID_randomSeed, (void *)&randomSeed);
}

void
StreamlineAttributes::SetNumberOfRandomSamples(int numberOfRandomSamples_)
{
    numberOfRandomSamples = numberOfRandomSamples_;
    Select(ID_numberOfRandomSamples, (void *)&numberOfRandomSamples);
}

void
StreamlineAttributes::SetForceNodeCenteredData(bool forceNodeCenteredData_)
{
    forceNodeCenteredData = forceNodeCenteredData_;
    Select(ID_forceNodeCenteredData, (void *)&forceNodeCenteredData);
}

void
StreamlineAttributes::SetIssueTerminationWarnings(bool issueTerminationWarnings_)
{
    issueTerminationWarnings = issueTerminationWarnings_;
    Select(ID_issueTerminationWarnings, (void *)&issueTerminationWarnings);
}

void
StreamlineAttributes::SetIssueStiffnessWarnings(bool issueStiffnessWarnings_)
{
    issueStiffnessWarnings = issueStiffnessWarnings_;
    Select(ID_issueStiffnessWarnings, (void *)&issueStiffnessWarnings);
}

void
StreamlineAttributes::SetIssueCriticalPointsWarnings(bool issueCriticalPointsWarnings_)
{
    issueCriticalPointsWarnings = issueCriticalPointsWarnings_;
    Select(ID_issueCriticalPointsWarnings, (void *)&issueCriticalPointsWarnings);
}

void
StreamlineAttributes::SetCriticalPointThreshold(double criticalPointThreshold_)
{
    criticalPointThreshold = criticalPointThreshold_;
    Select(ID_criticalPointThreshold, (void *)&criticalPointThreshold);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

StreamlineAttributes::SourceType
StreamlineAttributes::GetSourceType() const
{
    return SourceType(sourceType);
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
StreamlineAttributes::GetRadius() const
{
    return radius;
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
StreamlineAttributes::GetSampleDensity0() const
{
    return sampleDensity0;
}

int
StreamlineAttributes::GetSampleDensity1() const
{
    return sampleDensity1;
}

int
StreamlineAttributes::GetSampleDensity2() const
{
    return sampleDensity2;
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
    return IntegrationDirection(streamlineDirection);
}

int
StreamlineAttributes::GetMaxSteps() const
{
    return maxSteps;
}

bool
StreamlineAttributes::GetTerminateByDistance() const
{
    return terminateByDistance;
}

double
StreamlineAttributes::GetTermDistance() const
{
    return termDistance;
}

bool
StreamlineAttributes::GetTerminateByTime() const
{
    return terminateByTime;
}

double
StreamlineAttributes::GetTermTime() const
{
    return termTime;
}

double
StreamlineAttributes::GetMaxStepLength() const
{
    return maxStepLength;
}

bool
StreamlineAttributes::GetLimitMaximumTimestep() const
{
    return limitMaximumTimestep;
}

double
StreamlineAttributes::GetMaxTimeStep() const
{
    return maxTimeStep;
}

double
StreamlineAttributes::GetRelTol() const
{
    return relTol;
}

StreamlineAttributes::SizeType
StreamlineAttributes::GetAbsTolSizeType() const
{
    return SizeType(absTolSizeType);
}

double
StreamlineAttributes::GetAbsTolAbsolute() const
{
    return absTolAbsolute;
}

double
StreamlineAttributes::GetAbsTolBBox() const
{
    return absTolBBox;
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

bool
StreamlineAttributes::GetPathlinesOverrideStartingTimeFlag() const
{
    return pathlinesOverrideStartingTimeFlag;
}

double
StreamlineAttributes::GetPathlinesOverrideStartingTime() const
{
    return pathlinesOverrideStartingTime;
}

StreamlineAttributes::PathlinesCMFE
StreamlineAttributes::GetPathlinesCMFE() const
{
    return PathlinesCMFE(pathlinesCMFE);
}

StreamlineAttributes::CoordinateSystem
StreamlineAttributes::GetCoordinateSystem() const
{
    return CoordinateSystem(coordinateSystem);
}

double
StreamlineAttributes::GetPhiFactor() const
{
    return phiFactor;
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

double
StreamlineAttributes::GetDisplayBegin() const
{
    return displayBegin;
}

double
StreamlineAttributes::GetDisplayEnd() const
{
    return displayEnd;
}

bool
StreamlineAttributes::GetDisplayBeginFlag() const
{
    return displayBeginFlag;
}

bool
StreamlineAttributes::GetDisplayEndFlag() const
{
    return displayEndFlag;
}

StreamlineAttributes::ReferenceType
StreamlineAttributes::GetReferenceTypeForDisplay() const
{
    return ReferenceType(referenceTypeForDisplay);
}

StreamlineAttributes::DisplayMethod
StreamlineAttributes::GetDisplayMethod() const
{
    return DisplayMethod(displayMethod);
}

StreamlineAttributes::SizeType
StreamlineAttributes::GetTubeSizeType() const
{
    return SizeType(tubeSizeType);
}

double
StreamlineAttributes::GetTubeRadiusAbsolute() const
{
    return tubeRadiusAbsolute;
}

double
StreamlineAttributes::GetTubeRadiusBBox() const
{
    return tubeRadiusBBox;
}

StreamlineAttributes::SizeType
StreamlineAttributes::GetRibbonWidthSizeType() const
{
    return SizeType(ribbonWidthSizeType);
}

double
StreamlineAttributes::GetRibbonWidthAbsolute() const
{
    return ribbonWidthAbsolute;
}

double
StreamlineAttributes::GetRibbonWidthBBox() const
{
    return ribbonWidthBBox;
}

int
StreamlineAttributes::GetLineWidth() const
{
    return lineWidth;
}

bool
StreamlineAttributes::GetShowSeeds() const
{
    return showSeeds;
}

StreamlineAttributes::SizeType
StreamlineAttributes::GetSeedRadiusSizeType() const
{
    return SizeType(seedRadiusSizeType);
}

double
StreamlineAttributes::GetSeedRadiusAbsolute() const
{
    return seedRadiusAbsolute;
}

double
StreamlineAttributes::GetSeedRadiusBBox() const
{
    return seedRadiusBBox;
}

bool
StreamlineAttributes::GetShowHeads() const
{
    return showHeads;
}

StreamlineAttributes::GeomDisplayType
StreamlineAttributes::GetHeadDisplayType() const
{
    return GeomDisplayType(headDisplayType);
}

StreamlineAttributes::SizeType
StreamlineAttributes::GetHeadRadiusSizeType() const
{
    return SizeType(headRadiusSizeType);
}

double
StreamlineAttributes::GetHeadRadiusAbsolute() const
{
    return headRadiusAbsolute;
}

double
StreamlineAttributes::GetHeadRadiusBBox() const
{
    return headRadiusBBox;
}

double
StreamlineAttributes::GetHeadHeightRatio() const
{
    return headHeightRatio;
}

StreamlineAttributes::OpacityType
StreamlineAttributes::GetOpacityType() const
{
    return OpacityType(opacityType);
}

const std::string &
StreamlineAttributes::GetOpacityVariable() const
{
    return opacityVariable;
}

std::string &
StreamlineAttributes::GetOpacityVariable()
{
    return opacityVariable;
}

double
StreamlineAttributes::GetOpacity() const
{
    return opacity;
}

double
StreamlineAttributes::GetOpacityVarMin() const
{
    return opacityVarMin;
}

double
StreamlineAttributes::GetOpacityVarMax() const
{
    return opacityVarMax;
}

bool
StreamlineAttributes::GetOpacityVarMinFlag() const
{
    return opacityVarMinFlag;
}

bool
StreamlineAttributes::GetOpacityVarMaxFlag() const
{
    return opacityVarMaxFlag;
}

int
StreamlineAttributes::GetTubeDisplayDensity() const
{
    return tubeDisplayDensity;
}

StreamlineAttributes::DisplayQuality
StreamlineAttributes::GetGeomDisplayQuality() const
{
    return DisplayQuality(geomDisplayQuality);
}

double
StreamlineAttributes::GetSampleDistance0() const
{
    return sampleDistance0;
}

double
StreamlineAttributes::GetSampleDistance1() const
{
    return sampleDistance1;
}

double
StreamlineAttributes::GetSampleDistance2() const
{
    return sampleDistance2;
}

bool
StreamlineAttributes::GetFillInterior() const
{
    return fillInterior;
}

bool
StreamlineAttributes::GetRandomSamples() const
{
    return randomSamples;
}

int
StreamlineAttributes::GetRandomSeed() const
{
    return randomSeed;
}

int
StreamlineAttributes::GetNumberOfRandomSamples() const
{
    return numberOfRandomSamples;
}

bool
StreamlineAttributes::GetForceNodeCenteredData() const
{
    return forceNodeCenteredData;
}

bool
StreamlineAttributes::GetIssueTerminationWarnings() const
{
    return issueTerminationWarnings;
}

bool
StreamlineAttributes::GetIssueStiffnessWarnings() const
{
    return issueStiffnessWarnings;
}

bool
StreamlineAttributes::GetIssueCriticalPointsWarnings() const
{
    return issueCriticalPointsWarnings;
}

double
StreamlineAttributes::GetCriticalPointThreshold() const
{
    return criticalPointThreshold;
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

void
StreamlineAttributes::SelectOpacityVariable()
{
    Select(ID_opacityVariable, (void *)&opacityVariable);
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
    case ID_sourceType:                        return "sourceType";
    case ID_pointSource:                       return "pointSource";
    case ID_lineStart:                         return "lineStart";
    case ID_lineEnd:                           return "lineEnd";
    case ID_planeOrigin:                       return "planeOrigin";
    case ID_planeNormal:                       return "planeNormal";
    case ID_planeUpAxis:                       return "planeUpAxis";
    case ID_radius:                            return "radius";
    case ID_sphereOrigin:                      return "sphereOrigin";
    case ID_boxExtents:                        return "boxExtents";
    case ID_useWholeBox:                       return "useWholeBox";
    case ID_pointList:                         return "pointList";
    case ID_sampleDensity0:                    return "sampleDensity0";
    case ID_sampleDensity1:                    return "sampleDensity1";
    case ID_sampleDensity2:                    return "sampleDensity2";
    case ID_coloringMethod:                    return "coloringMethod";
    case ID_colorTableName:                    return "colorTableName";
    case ID_singleColor:                       return "singleColor";
    case ID_legendFlag:                        return "legendFlag";
    case ID_lightingFlag:                      return "lightingFlag";
    case ID_streamlineDirection:               return "streamlineDirection";
    case ID_maxSteps:                          return "maxSteps";
    case ID_terminateByDistance:               return "terminateByDistance";
    case ID_termDistance:                      return "termDistance";
    case ID_terminateByTime:                   return "terminateByTime";
    case ID_termTime:                          return "termTime";
    case ID_maxStepLength:                     return "maxStepLength";
    case ID_limitMaximumTimestep:              return "limitMaximumTimestep";
    case ID_maxTimeStep:                       return "maxTimeStep";
    case ID_relTol:                            return "relTol";
    case ID_absTolSizeType:                    return "absTolSizeType";
    case ID_absTolAbsolute:                    return "absTolAbsolute";
    case ID_absTolBBox:                        return "absTolBBox";
    case ID_integrationType:                   return "integrationType";
    case ID_streamlineAlgorithmType:           return "streamlineAlgorithmType";
    case ID_maxStreamlineProcessCount:         return "maxStreamlineProcessCount";
    case ID_maxDomainCacheSize:                return "maxDomainCacheSize";
    case ID_workGroupSize:                     return "workGroupSize";
    case ID_pathlines:                         return "pathlines";
    case ID_pathlinesOverrideStartingTimeFlag: return "pathlinesOverrideStartingTimeFlag";
    case ID_pathlinesOverrideStartingTime:     return "pathlinesOverrideStartingTime";
    case ID_pathlinesCMFE:                     return "pathlinesCMFE";
    case ID_coordinateSystem:                  return "coordinateSystem";
    case ID_phiFactor:                         return "phiFactor";
    case ID_coloringVariable:                  return "coloringVariable";
    case ID_legendMinFlag:                     return "legendMinFlag";
    case ID_legendMaxFlag:                     return "legendMaxFlag";
    case ID_legendMin:                         return "legendMin";
    case ID_legendMax:                         return "legendMax";
    case ID_displayBegin:                      return "displayBegin";
    case ID_displayEnd:                        return "displayEnd";
    case ID_displayBeginFlag:                  return "displayBeginFlag";
    case ID_displayEndFlag:                    return "displayEndFlag";
    case ID_referenceTypeForDisplay:           return "referenceTypeForDisplay";
    case ID_displayMethod:                     return "displayMethod";
    case ID_tubeSizeType:                      return "tubeSizeType";
    case ID_tubeRadiusAbsolute:                return "tubeRadiusAbsolute";
    case ID_tubeRadiusBBox:                    return "tubeRadiusBBox";
    case ID_ribbonWidthSizeType:               return "ribbonWidthSizeType";
    case ID_ribbonWidthAbsolute:               return "ribbonWidthAbsolute";
    case ID_ribbonWidthBBox:                   return "ribbonWidthBBox";
    case ID_lineWidth:                         return "lineWidth";
    case ID_showSeeds:                         return "showSeeds";
    case ID_seedRadiusSizeType:                return "seedRadiusSizeType";
    case ID_seedRadiusAbsolute:                return "seedRadiusAbsolute";
    case ID_seedRadiusBBox:                    return "seedRadiusBBox";
    case ID_showHeads:                         return "showHeads";
    case ID_headDisplayType:                   return "headDisplayType";
    case ID_headRadiusSizeType:                return "headRadiusSizeType";
    case ID_headRadiusAbsolute:                return "headRadiusAbsolute";
    case ID_headRadiusBBox:                    return "headRadiusBBox";
    case ID_headHeightRatio:                   return "headHeightRatio";
    case ID_opacityType:                       return "opacityType";
    case ID_opacityVariable:                   return "opacityVariable";
    case ID_opacity:                           return "opacity";
    case ID_opacityVarMin:                     return "opacityVarMin";
    case ID_opacityVarMax:                     return "opacityVarMax";
    case ID_opacityVarMinFlag:                 return "opacityVarMinFlag";
    case ID_opacityVarMaxFlag:                 return "opacityVarMaxFlag";
    case ID_tubeDisplayDensity:                return "tubeDisplayDensity";
    case ID_geomDisplayQuality:                return "geomDisplayQuality";
    case ID_sampleDistance0:                   return "sampleDistance0";
    case ID_sampleDistance1:                   return "sampleDistance1";
    case ID_sampleDistance2:                   return "sampleDistance2";
    case ID_fillInterior:                      return "fillInterior";
    case ID_randomSamples:                     return "randomSamples";
    case ID_randomSeed:                        return "randomSeed";
    case ID_numberOfRandomSamples:             return "numberOfRandomSamples";
    case ID_forceNodeCenteredData:             return "forceNodeCenteredData";
    case ID_issueTerminationWarnings:          return "issueTerminationWarnings";
    case ID_issueStiffnessWarnings:            return "issueStiffnessWarnings";
    case ID_issueCriticalPointsWarnings:       return "issueCriticalPointsWarnings";
    case ID_criticalPointThreshold:            return "criticalPointThreshold";
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
    case ID_sourceType:                        return FieldType_enum;
    case ID_pointSource:                       return FieldType_doubleArray;
    case ID_lineStart:                         return FieldType_doubleArray;
    case ID_lineEnd:                           return FieldType_doubleArray;
    case ID_planeOrigin:                       return FieldType_doubleArray;
    case ID_planeNormal:                       return FieldType_doubleArray;
    case ID_planeUpAxis:                       return FieldType_doubleArray;
    case ID_radius:                            return FieldType_double;
    case ID_sphereOrigin:                      return FieldType_doubleArray;
    case ID_boxExtents:                        return FieldType_doubleArray;
    case ID_useWholeBox:                       return FieldType_bool;
    case ID_pointList:                         return FieldType_doubleVector;
    case ID_sampleDensity0:                    return FieldType_int;
    case ID_sampleDensity1:                    return FieldType_int;
    case ID_sampleDensity2:                    return FieldType_int;
    case ID_coloringMethod:                    return FieldType_enum;
    case ID_colorTableName:                    return FieldType_colortable;
    case ID_singleColor:                       return FieldType_color;
    case ID_legendFlag:                        return FieldType_bool;
    case ID_lightingFlag:                      return FieldType_bool;
    case ID_streamlineDirection:               return FieldType_enum;
    case ID_maxSteps:                          return FieldType_int;
    case ID_terminateByDistance:               return FieldType_bool;
    case ID_termDistance:                      return FieldType_double;
    case ID_terminateByTime:                   return FieldType_bool;
    case ID_termTime:                          return FieldType_double;
    case ID_maxStepLength:                     return FieldType_double;
    case ID_limitMaximumTimestep:              return FieldType_bool;
    case ID_maxTimeStep:                       return FieldType_double;
    case ID_relTol:                            return FieldType_double;
    case ID_absTolSizeType:                    return FieldType_enum;
    case ID_absTolAbsolute:                    return FieldType_double;
    case ID_absTolBBox:                        return FieldType_double;
    case ID_integrationType:                   return FieldType_enum;
    case ID_streamlineAlgorithmType:           return FieldType_enum;
    case ID_maxStreamlineProcessCount:         return FieldType_int;
    case ID_maxDomainCacheSize:                return FieldType_int;
    case ID_workGroupSize:                     return FieldType_int;
    case ID_pathlines:                         return FieldType_bool;
    case ID_pathlinesOverrideStartingTimeFlag: return FieldType_bool;
    case ID_pathlinesOverrideStartingTime:     return FieldType_double;
    case ID_pathlinesCMFE:                     return FieldType_enum;
    case ID_coordinateSystem:                  return FieldType_enum;
    case ID_phiFactor:                         return FieldType_double;
    case ID_coloringVariable:                  return FieldType_string;
    case ID_legendMinFlag:                     return FieldType_bool;
    case ID_legendMaxFlag:                     return FieldType_bool;
    case ID_legendMin:                         return FieldType_double;
    case ID_legendMax:                         return FieldType_double;
    case ID_displayBegin:                      return FieldType_double;
    case ID_displayEnd:                        return FieldType_double;
    case ID_displayBeginFlag:                  return FieldType_bool;
    case ID_displayEndFlag:                    return FieldType_bool;
    case ID_referenceTypeForDisplay:           return FieldType_enum;
    case ID_displayMethod:                     return FieldType_enum;
    case ID_tubeSizeType:                      return FieldType_enum;
    case ID_tubeRadiusAbsolute:                return FieldType_double;
    case ID_tubeRadiusBBox:                    return FieldType_double;
    case ID_ribbonWidthSizeType:               return FieldType_enum;
    case ID_ribbonWidthAbsolute:               return FieldType_double;
    case ID_ribbonWidthBBox:                   return FieldType_double;
    case ID_lineWidth:                         return FieldType_linewidth;
    case ID_showSeeds:                         return FieldType_bool;
    case ID_seedRadiusSizeType:                return FieldType_enum;
    case ID_seedRadiusAbsolute:                return FieldType_double;
    case ID_seedRadiusBBox:                    return FieldType_double;
    case ID_showHeads:                         return FieldType_bool;
    case ID_headDisplayType:                   return FieldType_enum;
    case ID_headRadiusSizeType:                return FieldType_enum;
    case ID_headRadiusAbsolute:                return FieldType_double;
    case ID_headRadiusBBox:                    return FieldType_double;
    case ID_headHeightRatio:                   return FieldType_double;
    case ID_opacityType:                       return FieldType_enum;
    case ID_opacityVariable:                   return FieldType_string;
    case ID_opacity:                           return FieldType_double;
    case ID_opacityVarMin:                     return FieldType_double;
    case ID_opacityVarMax:                     return FieldType_double;
    case ID_opacityVarMinFlag:                 return FieldType_bool;
    case ID_opacityVarMaxFlag:                 return FieldType_bool;
    case ID_tubeDisplayDensity:                return FieldType_int;
    case ID_geomDisplayQuality:                return FieldType_enum;
    case ID_sampleDistance0:                   return FieldType_double;
    case ID_sampleDistance1:                   return FieldType_double;
    case ID_sampleDistance2:                   return FieldType_double;
    case ID_fillInterior:                      return FieldType_bool;
    case ID_randomSamples:                     return FieldType_bool;
    case ID_randomSeed:                        return FieldType_int;
    case ID_numberOfRandomSamples:             return FieldType_int;
    case ID_forceNodeCenteredData:             return FieldType_bool;
    case ID_issueTerminationWarnings:          return FieldType_bool;
    case ID_issueStiffnessWarnings:            return FieldType_bool;
    case ID_issueCriticalPointsWarnings:       return FieldType_bool;
    case ID_criticalPointThreshold:            return FieldType_double;
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
    case ID_sourceType:                        return "enum";
    case ID_pointSource:                       return "doubleArray";
    case ID_lineStart:                         return "doubleArray";
    case ID_lineEnd:                           return "doubleArray";
    case ID_planeOrigin:                       return "doubleArray";
    case ID_planeNormal:                       return "doubleArray";
    case ID_planeUpAxis:                       return "doubleArray";
    case ID_radius:                            return "double";
    case ID_sphereOrigin:                      return "doubleArray";
    case ID_boxExtents:                        return "doubleArray";
    case ID_useWholeBox:                       return "bool";
    case ID_pointList:                         return "doubleVector";
    case ID_sampleDensity0:                    return "int";
    case ID_sampleDensity1:                    return "int";
    case ID_sampleDensity2:                    return "int";
    case ID_coloringMethod:                    return "enum";
    case ID_colorTableName:                    return "colortable";
    case ID_singleColor:                       return "color";
    case ID_legendFlag:                        return "bool";
    case ID_lightingFlag:                      return "bool";
    case ID_streamlineDirection:               return "enum";
    case ID_maxSteps:                          return "int";
    case ID_terminateByDistance:               return "bool";
    case ID_termDistance:                      return "double";
    case ID_terminateByTime:                   return "bool";
    case ID_termTime:                          return "double";
    case ID_maxStepLength:                     return "double";
    case ID_limitMaximumTimestep:              return "bool";
    case ID_maxTimeStep:                       return "double";
    case ID_relTol:                            return "double";
    case ID_absTolSizeType:                    return "enum";
    case ID_absTolAbsolute:                    return "double";
    case ID_absTolBBox:                        return "double";
    case ID_integrationType:                   return "enum";
    case ID_streamlineAlgorithmType:           return "enum";
    case ID_maxStreamlineProcessCount:         return "int";
    case ID_maxDomainCacheSize:                return "int";
    case ID_workGroupSize:                     return "int";
    case ID_pathlines:                         return "bool";
    case ID_pathlinesOverrideStartingTimeFlag: return "bool";
    case ID_pathlinesOverrideStartingTime:     return "double";
    case ID_pathlinesCMFE:                     return "enum";
    case ID_coordinateSystem:                  return "enum";
    case ID_phiFactor:                         return "double";
    case ID_coloringVariable:                  return "string";
    case ID_legendMinFlag:                     return "bool";
    case ID_legendMaxFlag:                     return "bool";
    case ID_legendMin:                         return "double";
    case ID_legendMax:                         return "double";
    case ID_displayBegin:                      return "double";
    case ID_displayEnd:                        return "double";
    case ID_displayBeginFlag:                  return "bool";
    case ID_displayEndFlag:                    return "bool";
    case ID_referenceTypeForDisplay:           return "enum";
    case ID_displayMethod:                     return "enum";
    case ID_tubeSizeType:                      return "enum";
    case ID_tubeRadiusAbsolute:                return "double";
    case ID_tubeRadiusBBox:                    return "double";
    case ID_ribbonWidthSizeType:               return "enum";
    case ID_ribbonWidthAbsolute:               return "double";
    case ID_ribbonWidthBBox:                   return "double";
    case ID_lineWidth:                         return "linewidth";
    case ID_showSeeds:                         return "bool";
    case ID_seedRadiusSizeType:                return "enum";
    case ID_seedRadiusAbsolute:                return "double";
    case ID_seedRadiusBBox:                    return "double";
    case ID_showHeads:                         return "bool";
    case ID_headDisplayType:                   return "enum";
    case ID_headRadiusSizeType:                return "enum";
    case ID_headRadiusAbsolute:                return "double";
    case ID_headRadiusBBox:                    return "double";
    case ID_headHeightRatio:                   return "double";
    case ID_opacityType:                       return "enum";
    case ID_opacityVariable:                   return "string";
    case ID_opacity:                           return "double";
    case ID_opacityVarMin:                     return "double";
    case ID_opacityVarMax:                     return "double";
    case ID_opacityVarMinFlag:                 return "bool";
    case ID_opacityVarMaxFlag:                 return "bool";
    case ID_tubeDisplayDensity:                return "int";
    case ID_geomDisplayQuality:                return "enum";
    case ID_sampleDistance0:                   return "double";
    case ID_sampleDistance1:                   return "double";
    case ID_sampleDistance2:                   return "double";
    case ID_fillInterior:                      return "bool";
    case ID_randomSamples:                     return "bool";
    case ID_randomSeed:                        return "int";
    case ID_numberOfRandomSamples:             return "int";
    case ID_forceNodeCenteredData:             return "bool";
    case ID_issueTerminationWarnings:          return "bool";
    case ID_issueStiffnessWarnings:            return "bool";
    case ID_issueCriticalPointsWarnings:       return "bool";
    case ID_criticalPointThreshold:            return "double";
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
    case ID_radius:
        {  // new scope
        retval = (radius == obj.radius);
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
    case ID_sampleDensity0:
        {  // new scope
        retval = (sampleDensity0 == obj.sampleDensity0);
        }
        break;
    case ID_sampleDensity1:
        {  // new scope
        retval = (sampleDensity1 == obj.sampleDensity1);
        }
        break;
    case ID_sampleDensity2:
        {  // new scope
        retval = (sampleDensity2 == obj.sampleDensity2);
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
    case ID_streamlineDirection:
        {  // new scope
        retval = (streamlineDirection == obj.streamlineDirection);
        }
        break;
    case ID_maxSteps:
        {  // new scope
        retval = (maxSteps == obj.maxSteps);
        }
        break;
    case ID_terminateByDistance:
        {  // new scope
        retval = (terminateByDistance == obj.terminateByDistance);
        }
        break;
    case ID_termDistance:
        {  // new scope
        retval = (termDistance == obj.termDistance);
        }
        break;
    case ID_terminateByTime:
        {  // new scope
        retval = (terminateByTime == obj.terminateByTime);
        }
        break;
    case ID_termTime:
        {  // new scope
        retval = (termTime == obj.termTime);
        }
        break;
    case ID_maxStepLength:
        {  // new scope
        retval = (maxStepLength == obj.maxStepLength);
        }
        break;
    case ID_limitMaximumTimestep:
        {  // new scope
        retval = (limitMaximumTimestep == obj.limitMaximumTimestep);
        }
        break;
    case ID_maxTimeStep:
        {  // new scope
        retval = (maxTimeStep == obj.maxTimeStep);
        }
        break;
    case ID_relTol:
        {  // new scope
        retval = (relTol == obj.relTol);
        }
        break;
    case ID_absTolSizeType:
        {  // new scope
        retval = (absTolSizeType == obj.absTolSizeType);
        }
        break;
    case ID_absTolAbsolute:
        {  // new scope
        retval = (absTolAbsolute == obj.absTolAbsolute);
        }
        break;
    case ID_absTolBBox:
        {  // new scope
        retval = (absTolBBox == obj.absTolBBox);
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
    case ID_pathlinesOverrideStartingTimeFlag:
        {  // new scope
        retval = (pathlinesOverrideStartingTimeFlag == obj.pathlinesOverrideStartingTimeFlag);
        }
        break;
    case ID_pathlinesOverrideStartingTime:
        {  // new scope
        retval = (pathlinesOverrideStartingTime == obj.pathlinesOverrideStartingTime);
        }
        break;
    case ID_pathlinesCMFE:
        {  // new scope
        retval = (pathlinesCMFE == obj.pathlinesCMFE);
        }
        break;
    case ID_coordinateSystem:
        {  // new scope
        retval = (coordinateSystem == obj.coordinateSystem);
        }
        break;
    case ID_phiFactor:
        {  // new scope
        retval = (phiFactor == obj.phiFactor);
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
    case ID_displayBegin:
        {  // new scope
        retval = (displayBegin == obj.displayBegin);
        }
        break;
    case ID_displayEnd:
        {  // new scope
        retval = (displayEnd == obj.displayEnd);
        }
        break;
    case ID_displayBeginFlag:
        {  // new scope
        retval = (displayBeginFlag == obj.displayBeginFlag);
        }
        break;
    case ID_displayEndFlag:
        {  // new scope
        retval = (displayEndFlag == obj.displayEndFlag);
        }
        break;
    case ID_referenceTypeForDisplay:
        {  // new scope
        retval = (referenceTypeForDisplay == obj.referenceTypeForDisplay);
        }
        break;
    case ID_displayMethod:
        {  // new scope
        retval = (displayMethod == obj.displayMethod);
        }
        break;
    case ID_tubeSizeType:
        {  // new scope
        retval = (tubeSizeType == obj.tubeSizeType);
        }
        break;
    case ID_tubeRadiusAbsolute:
        {  // new scope
        retval = (tubeRadiusAbsolute == obj.tubeRadiusAbsolute);
        }
        break;
    case ID_tubeRadiusBBox:
        {  // new scope
        retval = (tubeRadiusBBox == obj.tubeRadiusBBox);
        }
        break;
    case ID_ribbonWidthSizeType:
        {  // new scope
        retval = (ribbonWidthSizeType == obj.ribbonWidthSizeType);
        }
        break;
    case ID_ribbonWidthAbsolute:
        {  // new scope
        retval = (ribbonWidthAbsolute == obj.ribbonWidthAbsolute);
        }
        break;
    case ID_ribbonWidthBBox:
        {  // new scope
        retval = (ribbonWidthBBox == obj.ribbonWidthBBox);
        }
        break;
    case ID_lineWidth:
        {  // new scope
        retval = (lineWidth == obj.lineWidth);
        }
        break;
    case ID_showSeeds:
        {  // new scope
        retval = (showSeeds == obj.showSeeds);
        }
        break;
    case ID_seedRadiusSizeType:
        {  // new scope
        retval = (seedRadiusSizeType == obj.seedRadiusSizeType);
        }
        break;
    case ID_seedRadiusAbsolute:
        {  // new scope
        retval = (seedRadiusAbsolute == obj.seedRadiusAbsolute);
        }
        break;
    case ID_seedRadiusBBox:
        {  // new scope
        retval = (seedRadiusBBox == obj.seedRadiusBBox);
        }
        break;
    case ID_showHeads:
        {  // new scope
        retval = (showHeads == obj.showHeads);
        }
        break;
    case ID_headDisplayType:
        {  // new scope
        retval = (headDisplayType == obj.headDisplayType);
        }
        break;
    case ID_headRadiusSizeType:
        {  // new scope
        retval = (headRadiusSizeType == obj.headRadiusSizeType);
        }
        break;
    case ID_headRadiusAbsolute:
        {  // new scope
        retval = (headRadiusAbsolute == obj.headRadiusAbsolute);
        }
        break;
    case ID_headRadiusBBox:
        {  // new scope
        retval = (headRadiusBBox == obj.headRadiusBBox);
        }
        break;
    case ID_headHeightRatio:
        {  // new scope
        retval = (headHeightRatio == obj.headHeightRatio);
        }
        break;
    case ID_opacityType:
        {  // new scope
        retval = (opacityType == obj.opacityType);
        }
        break;
    case ID_opacityVariable:
        {  // new scope
        retval = (opacityVariable == obj.opacityVariable);
        }
        break;
    case ID_opacity:
        {  // new scope
        retval = (opacity == obj.opacity);
        }
        break;
    case ID_opacityVarMin:
        {  // new scope
        retval = (opacityVarMin == obj.opacityVarMin);
        }
        break;
    case ID_opacityVarMax:
        {  // new scope
        retval = (opacityVarMax == obj.opacityVarMax);
        }
        break;
    case ID_opacityVarMinFlag:
        {  // new scope
        retval = (opacityVarMinFlag == obj.opacityVarMinFlag);
        }
        break;
    case ID_opacityVarMaxFlag:
        {  // new scope
        retval = (opacityVarMaxFlag == obj.opacityVarMaxFlag);
        }
        break;
    case ID_tubeDisplayDensity:
        {  // new scope
        retval = (tubeDisplayDensity == obj.tubeDisplayDensity);
        }
        break;
    case ID_geomDisplayQuality:
        {  // new scope
        retval = (geomDisplayQuality == obj.geomDisplayQuality);
        }
        break;
    case ID_sampleDistance0:
        {  // new scope
        retval = (sampleDistance0 == obj.sampleDistance0);
        }
        break;
    case ID_sampleDistance1:
        {  // new scope
        retval = (sampleDistance1 == obj.sampleDistance1);
        }
        break;
    case ID_sampleDistance2:
        {  // new scope
        retval = (sampleDistance2 == obj.sampleDistance2);
        }
        break;
    case ID_fillInterior:
        {  // new scope
        retval = (fillInterior == obj.fillInterior);
        }
        break;
    case ID_randomSamples:
        {  // new scope
        retval = (randomSamples == obj.randomSamples);
        }
        break;
    case ID_randomSeed:
        {  // new scope
        retval = (randomSeed == obj.randomSeed);
        }
        break;
    case ID_numberOfRandomSamples:
        {  // new scope
        retval = (numberOfRandomSamples == obj.numberOfRandomSamples);
        }
        break;
    case ID_forceNodeCenteredData:
        {  // new scope
        retval = (forceNodeCenteredData == obj.forceNodeCenteredData);
        }
        break;
    case ID_issueTerminationWarnings:
        {  // new scope
        retval = (issueTerminationWarnings == obj.issueTerminationWarnings);
        }
        break;
    case ID_issueStiffnessWarnings:
        {  // new scope
        retval = (issueStiffnessWarnings == obj.issueStiffnessWarnings);
        }
        break;
    case ID_issueCriticalPointsWarnings:
        {  // new scope
        retval = (issueCriticalPointsWarnings == obj.issueCriticalPointsWarnings);
        }
        break;
    case ID_criticalPointThreshold:
        {  // new scope
        retval = (criticalPointThreshold == obj.criticalPointThreshold);
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
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Christoph Garth, Wed Jan 13 17:14:21 PST 2010 
//   Add support for circle source.
//
//   Hank Childs, Fri Oct  1 20:43:34 PDT 2010
//   Add support for absTol that is fraction of the bounding box.
//
//   Hank Childs, Mon Oct  4 14:32:06 PDT 2010
//   Add support for having multiple termination criterias.
//
// ****************************************************************************

#define PDIF(p1,p2,i) ((p1)[i] != (p2)[i])
#define POINT_DIFFERS(p1,p2) (PDIF(p1,p2,0) || PDIF(p1,p2,1) || PDIF(p1,p2,2))

bool
StreamlineAttributes::ChangesRequireRecalculation(const StreamlineAttributes &obj) const
{
    //Check the general stuff first...
    if (sourceType != obj.sourceType ||
        maxSteps != obj.maxSteps ||
        terminateByDistance != obj.terminateByDistance ||
        termDistance != obj.termDistance ||
        terminateByTime != obj.terminateByTime ||
        termTime != obj.termTime ||
        streamlineDirection != obj.streamlineDirection ||
        integrationType != obj.integrationType ||
        coordinateSystem != obj.coordinateSystem ||
        phiFactor != obj.phiFactor ||
        maxStepLength != obj.maxStepLength ||
        maxTimeStep != obj.maxTimeStep ||
        limitMaximumTimestep != obj.limitMaximumTimestep ||
        relTol != obj.relTol ||
        absTolAbsolute != obj.absTolAbsolute ||
        absTolBBox != obj.absTolBBox ||
        absTolSizeType != obj.absTolSizeType ||
        forceNodeCenteredData != obj.forceNodeCenteredData ||
        referenceTypeForDisplay != obj.referenceTypeForDisplay ||
        pathlines != obj.pathlines ||
        pathlinesOverrideStartingTimeFlag != obj.pathlinesOverrideStartingTimeFlag ||
        pathlinesOverrideStartingTime != obj.pathlinesOverrideStartingTime ||
        pathlinesCMFE != obj.pathlinesCMFE ||
        coloringVariable != obj.coloringVariable ||
        (displayMethod != obj.displayMethod && obj.displayMethod == Ribbons) ||
        (coloringMethod != obj.coloringMethod && obj.coloringMethod != Solid) ||

        ((opacityType == VariableRange) && (obj.opacityType != VariableRange ||
                                            opacityVariable != obj.opacityVariable)))
    {
        return true;
    }
         
    // If they say they don't want warnings, then don't re-execute.  If they say they do,
    // then we better re-execute so we can give them that warning.
    if (issueTerminationWarnings != obj.issueTerminationWarnings &&
        obj.issueTerminationWarnings == true)
        return true;

    // We need velocities for illuminate streamlines rendered as lines
    if (lightingFlag != obj.lightingFlag && obj.lightingFlag == true)
        return true;

    //Check by source type.
    if ((sourceType == SpecifiedPoint) && POINT_DIFFERS(pointSource, obj.pointSource))
    {
        return true;
    }

    if (sourceType == SpecifiedLine)
    {
        if (POINT_DIFFERS(lineStart, obj.lineStart) ||
            POINT_DIFFERS(lineEnd, obj.lineEnd) ||
            randomSamples != obj.randomSamples ||
            (!randomSamples && (sampleDensity0 != obj.sampleDensity0)) ||
            (randomSamples && (randomSeed != obj.randomSeed ||
                               numberOfRandomSamples != obj.numberOfRandomSamples)))
        {
            return true;
        }
    }

    if (sourceType == SpecifiedPlane)
    {
        if (POINT_DIFFERS(planeOrigin, obj.planeOrigin) ||
            POINT_DIFFERS(planeNormal, obj.planeNormal) ||
            POINT_DIFFERS(planeUpAxis, obj.planeUpAxis) ||
            sampleDistance0 != obj.sampleDistance0 ||
            sampleDistance1 != obj.sampleDistance1 ||
            randomSamples != obj.randomSamples ||
            fillInterior != obj.fillInterior ||
            (randomSamples && (randomSeed != obj.randomSeed ||
                               numberOfRandomSamples != obj.numberOfRandomSamples)) ||
            (!randomSamples && (sampleDensity0 != obj.sampleDensity0 ||
                                sampleDensity1 != obj.sampleDensity1)))
        {
            return true;
        }
    }

    if (sourceType == SpecifiedCircle)
    {
        if (POINT_DIFFERS(planeOrigin, obj.planeOrigin) ||
            POINT_DIFFERS(planeNormal, obj.planeNormal) ||
            POINT_DIFFERS(planeUpAxis, obj.planeUpAxis) ||
            radius != obj.radius ||
            randomSamples != obj.randomSamples ||
            fillInterior != obj.fillInterior ||
            (randomSamples && (randomSeed != obj.randomSeed ||
                               numberOfRandomSamples != obj.numberOfRandomSamples)) ||
            (!randomSamples && fillInterior && (sampleDensity0 != obj.sampleDensity0 ||
                                                sampleDensity1 != obj.sampleDensity1)) ||
            (!randomSamples && !fillInterior && (sampleDensity0 != obj.sampleDensity0)))
        {
            return true;
        }
    }

    if (sourceType == SpecifiedSphere)
    {
        if (POINT_DIFFERS(sphereOrigin, obj.sphereOrigin) ||
            radius != obj.radius ||
            randomSamples != obj.randomSamples ||
            fillInterior != obj.fillInterior ||
            (randomSamples && (randomSeed != obj.randomSeed ||
                               numberOfRandomSamples != obj.numberOfRandomSamples)) ||
            (!randomSamples && (sampleDensity0 != obj.sampleDensity0 ||
                                sampleDensity1 != obj.sampleDensity1 ||
                                sampleDensity2 != obj.sampleDensity2)))
        {
            return true;
        }
    }

    if (sourceType == SpecifiedBox)
    {
        if (POINT_DIFFERS(boxExtents, obj.boxExtents) ||
            POINT_DIFFERS(boxExtents+3, obj.boxExtents+3) ||
            useWholeBox != obj.useWholeBox ||
            randomSamples != obj.randomSamples ||
            fillInterior != obj.fillInterior ||
            (randomSamples && (randomSeed != obj.randomSeed ||
                               numberOfRandomSamples != obj.numberOfRandomSamples)) ||
            (!randomSamples && (sampleDensity0 != obj.sampleDensity0 ||
                                sampleDensity1 != obj.sampleDensity1 ||
                                sampleDensity2 != obj.sampleDensity2)))
        {
            return true;
        }
    }
    
    if (sourceType == SpecifiedPointList)
    {
        if (pointList.size() != obj.pointList.size())
            return true;
        else
            for (int i = 0 ; i < pointList.size() ; i++)
                if (pointList[i] != obj.pointList[i])
                    return true;
    }

    return false;
}

// ****************************************************************************
// Method: StreamlineAttributes::ProcessOldVersions
//
// Purpose: 
//   This method creates modifies a DataNode representation of the object
//   so it conforms to the newest representation of the object, which can
//   can be read back in.
//
// Programmer: Dave Pugmire
// Creation:   January 20 2010
//
// Modifications:
//
//   Dave Pugmire, Fri Sep 24 10:27:47 EDT 2010
//   Fix handling of radius and pointDensity fields.
//
// ****************************************************************************

void
StreamlineAttributes::ProcessOldVersions(DataNode *parentNode,
                                         const char *configVersion)
{
    char num1[2] = {configVersion[0], '\0'}, num2[2] = {configVersion[2], '\0'}, num3[2] = {configVersion[4], '\0'};
    int major = atoi(num1), minor = atoi(num2), patch = atoi(num3);
    
    DataNode *searchNode = parentNode->GetNode("StreamlineDirection");
    if (searchNode)
    {
        int val = searchNode->AsInt();
        parentNode->RemoveNode(searchNode);
        
        DataNode *newNode = new DataNode("streamlineDirection", val);
        parentNode->AddNode(newNode);
    }

    searchNode = parentNode->GetNode("showStart");
    if (searchNode)
    {
        bool val = searchNode->AsBool();
        parentNode->RemoveNode(searchNode);
        
        DataNode *newNode = new DataNode("showSeeds", val);
        parentNode->AddNode(newNode);
    }
    
    if (major < 2)
    {
        searchNode = parentNode->GetNode("radius");
        if (searchNode)
        {
            double val = searchNode->AsDouble();
            parentNode->RemoveNode(searchNode);
            DataNode *newNode = new DataNode("tubeRadius", val);
            parentNode->AddNode(newNode);
            
            newNode = new DataNode("ribbonWidth", val);
            parentNode->AddNode(newNode);
        }
    }

    searchNode = parentNode->GetNode("pointDensity");
    if (searchNode)
    {
        int val = searchNode->AsInt();
        parentNode->RemoveNode(searchNode);
        DataNode *newNode0 = new DataNode("sampleDensity0", val);
        parentNode->AddNode(newNode0);
        DataNode *newNode1 = new DataNode("sampleDensity1", val);
        parentNode->AddNode(newNode1);
        DataNode *newNode2 = new DataNode("sampleDensity2", val);
        parentNode->AddNode(newNode2);
    }
}

