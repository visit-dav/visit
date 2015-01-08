/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <IntegralCurveAttributes.h>
#include <DataNode.h>
#include <PointAttributes.h>
#include <Line.h>
#include <PlaneAttributes.h>
#include <SphereAttributes.h>
#include <PointAttributes.h>
#include <BoxExtents.h>
#include <stdlib.h>

//
// Enum conversion methods for IntegralCurveAttributes::SourceType
//

static const char *SourceType_strings[] = {
"Point", "PointList", "Line_", 
"Circle", "Plane", "Sphere", 
"Box", "Selection"};

std::string
IntegralCurveAttributes::SourceType_ToString(IntegralCurveAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 8) index = 0;
    return SourceType_strings[index];
}

std::string
IntegralCurveAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 8) ? 0 : t;
    return SourceType_strings[index];
}

bool
IntegralCurveAttributes::SourceType_FromString(const std::string &s, IntegralCurveAttributes::SourceType &val)
{
    val = IntegralCurveAttributes::Point;
    for(int i = 0; i < 8; ++i)
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
// Enum conversion methods for IntegralCurveAttributes::DataValue
//

static const char *DataValue_strings[] = {
"Solid", "SeedPointID", "Speed", 
"Vorticity", "ArcLength", "TimeAbsolute", 
"TimeRelative", "AverageDistanceFromSeed", "CorrelationDistance", 
"Difference", "Variable"};

std::string
IntegralCurveAttributes::DataValue_ToString(IntegralCurveAttributes::DataValue t)
{
    int index = int(t);
    if(index < 0 || index >= 11) index = 0;
    return DataValue_strings[index];
}

std::string
IntegralCurveAttributes::DataValue_ToString(int t)
{
    int index = (t < 0 || t >= 11) ? 0 : t;
    return DataValue_strings[index];
}

bool
IntegralCurveAttributes::DataValue_FromString(const std::string &s, IntegralCurveAttributes::DataValue &val)
{
    val = IntegralCurveAttributes::Solid;
    for(int i = 0; i < 11; ++i)
    {
        if(s == DataValue_strings[i])
        {
            val = (DataValue)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for IntegralCurveAttributes::CropValue
//

static const char *CropValue_strings[] = {
"Distance", "Time", "StepNumber"
};

std::string
IntegralCurveAttributes::CropValue_ToString(IntegralCurveAttributes::CropValue t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return CropValue_strings[index];
}

std::string
IntegralCurveAttributes::CropValue_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return CropValue_strings[index];
}

bool
IntegralCurveAttributes::CropValue_FromString(const std::string &s, IntegralCurveAttributes::CropValue &val)
{
    val = IntegralCurveAttributes::Distance;
    for(int i = 0; i < 3; ++i)
    {
        if(s == CropValue_strings[i])
        {
            val = (CropValue)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for IntegralCurveAttributes::DisplayGeometry
//

static const char *DisplayGeometry_strings[] = {
"Lines", "Tubes", "Ribbons"
};

std::string
IntegralCurveAttributes::DisplayGeometry_ToString(IntegralCurveAttributes::DisplayGeometry t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return DisplayGeometry_strings[index];
}

std::string
IntegralCurveAttributes::DisplayGeometry_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return DisplayGeometry_strings[index];
}

bool
IntegralCurveAttributes::DisplayGeometry_FromString(const std::string &s, IntegralCurveAttributes::DisplayGeometry &val)
{
    val = IntegralCurveAttributes::Lines;
    for(int i = 0; i < 3; ++i)
    {
        if(s == DisplayGeometry_strings[i])
        {
            val = (DisplayGeometry)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for IntegralCurveAttributes::IntegrationDirection
//

static const char *IntegrationDirection_strings[] = {
"Forward", "Backward", "Both"
};

std::string
IntegralCurveAttributes::IntegrationDirection_ToString(IntegralCurveAttributes::IntegrationDirection t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return IntegrationDirection_strings[index];
}

std::string
IntegralCurveAttributes::IntegrationDirection_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return IntegrationDirection_strings[index];
}

bool
IntegralCurveAttributes::IntegrationDirection_FromString(const std::string &s, IntegralCurveAttributes::IntegrationDirection &val)
{
    val = IntegralCurveAttributes::Forward;
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
// Enum conversion methods for IntegralCurveAttributes::ParallelizationAlgorithmType
//

static const char *ParallelizationAlgorithmType_strings[] = {
"LoadOnDemand", "ParallelStaticDomains", "MasterSlave", 
"VisItSelects"};

std::string
IntegralCurveAttributes::ParallelizationAlgorithmType_ToString(IntegralCurveAttributes::ParallelizationAlgorithmType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return ParallelizationAlgorithmType_strings[index];
}

std::string
IntegralCurveAttributes::ParallelizationAlgorithmType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return ParallelizationAlgorithmType_strings[index];
}

bool
IntegralCurveAttributes::ParallelizationAlgorithmType_FromString(const std::string &s, IntegralCurveAttributes::ParallelizationAlgorithmType &val)
{
    val = IntegralCurveAttributes::LoadOnDemand;
    for(int i = 0; i < 4; ++i)
    {
        if(s == ParallelizationAlgorithmType_strings[i])
        {
            val = (ParallelizationAlgorithmType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for IntegralCurveAttributes::FieldType
//

static const char *FieldType_strings[] = {
"Default", "FlashField", "M3DC12DField", 
"M3DC13DField", "Nek5000Field", "NektarPPField", 
"NIMRODField"};

std::string
IntegralCurveAttributes::FieldType_ToString(IntegralCurveAttributes::FieldType t)
{
    int index = int(t);
    if(index < 0 || index >= 7) index = 0;
    return FieldType_strings[index];
}

std::string
IntegralCurveAttributes::FieldType_ToString(int t)
{
    int index = (t < 0 || t >= 7) ? 0 : t;
    return FieldType_strings[index];
}

bool
IntegralCurveAttributes::FieldType_FromString(const std::string &s, IntegralCurveAttributes::FieldType &val)
{
    val = IntegralCurveAttributes::Default;
    for(int i = 0; i < 7; ++i)
    {
        if(s == FieldType_strings[i])
        {
            val = (FieldType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for IntegralCurveAttributes::IntegrationType
//

static const char *IntegrationType_strings[] = {
"Euler", "Leapfrog", "DormandPrince", 
"AdamsBashforth", "RK4", "M3DC12DIntegrator"
};

std::string
IntegralCurveAttributes::IntegrationType_ToString(IntegralCurveAttributes::IntegrationType t)
{
    int index = int(t);
    if(index < 0 || index >= 6) index = 0;
    return IntegrationType_strings[index];
}

std::string
IntegralCurveAttributes::IntegrationType_ToString(int t)
{
    int index = (t < 0 || t >= 6) ? 0 : t;
    return IntegrationType_strings[index];
}

bool
IntegralCurveAttributes::IntegrationType_FromString(const std::string &s, IntegralCurveAttributes::IntegrationType &val)
{
    val = IntegralCurveAttributes::Euler;
    for(int i = 0; i < 6; ++i)
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
// Enum conversion methods for IntegralCurveAttributes::PathlinesCMFE
//

static const char *PathlinesCMFE_strings[] = {
"CONN_CMFE", "POS_CMFE"};

std::string
IntegralCurveAttributes::PathlinesCMFE_ToString(IntegralCurveAttributes::PathlinesCMFE t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return PathlinesCMFE_strings[index];
}

std::string
IntegralCurveAttributes::PathlinesCMFE_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return PathlinesCMFE_strings[index];
}

bool
IntegralCurveAttributes::PathlinesCMFE_FromString(const std::string &s, IntegralCurveAttributes::PathlinesCMFE &val)
{
    val = IntegralCurveAttributes::CONN_CMFE;
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

//
// Enum conversion methods for IntegralCurveAttributes::SizeType
//

static const char *SizeType_strings[] = {
"Absolute", "FractionOfBBox"};

std::string
IntegralCurveAttributes::SizeType_ToString(IntegralCurveAttributes::SizeType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SizeType_strings[index];
}

std::string
IntegralCurveAttributes::SizeType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SizeType_strings[index];
}

bool
IntegralCurveAttributes::SizeType_FromString(const std::string &s, IntegralCurveAttributes::SizeType &val)
{
    val = IntegralCurveAttributes::Absolute;
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

// ****************************************************************************
// Method: IntegralCurveAttributes::IntegralCurveAttributes
//
// Purpose: 
//   Init utility for the IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void IntegralCurveAttributes::Init()
{
    sourceType = Point;
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
    dataValue = TimeAbsolute;
    integrationDirection = Forward;
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
    fieldType = Default;
    fieldConstant = 1;
    velocitySource[0] = 0;
    velocitySource[1] = 0;
    velocitySource[2] = 0;
    integrationType = DormandPrince;
    parallelizationAlgorithmType = VisItSelects;
    maxProcessCount = 10;
    maxDomainCacheSize = 3;
    workGroupSize = 32;
    pathlines = false;
    pathlinesOverrideStartingTimeFlag = false;
    pathlinesOverrideStartingTime = 0;
    pathlinesPeriod = 0;
    pathlinesCMFE = POS_CMFE;
    displayGeometry = Lines;
    cropBeginFlag = false;
    cropBegin = 0;
    cropEndFlag = false;
    cropEnd = 0;
    cropValue = Time;
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
    correlationDistanceAngTol = 5;
    correlationDistanceMinDistAbsolute = 1;
    correlationDistanceMinDistBBox = 0.005;
    correlationDistanceMinDistType = FractionOfBBox;

    IntegralCurveAttributes::SelectAll();
}

// ****************************************************************************
// Method: IntegralCurveAttributes::IntegralCurveAttributes
//
// Purpose: 
//   Copy utility for the IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void IntegralCurveAttributes::Copy(const IntegralCurveAttributes &obj)
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
    dataValue = obj.dataValue;
    dataVariable = obj.dataVariable;
    integrationDirection = obj.integrationDirection;
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
    fieldType = obj.fieldType;
    fieldConstant = obj.fieldConstant;
    velocitySource[0] = obj.velocitySource[0];
    velocitySource[1] = obj.velocitySource[1];
    velocitySource[2] = obj.velocitySource[2];

    integrationType = obj.integrationType;
    parallelizationAlgorithmType = obj.parallelizationAlgorithmType;
    maxProcessCount = obj.maxProcessCount;
    maxDomainCacheSize = obj.maxDomainCacheSize;
    workGroupSize = obj.workGroupSize;
    pathlines = obj.pathlines;
    pathlinesOverrideStartingTimeFlag = obj.pathlinesOverrideStartingTimeFlag;
    pathlinesOverrideStartingTime = obj.pathlinesOverrideStartingTime;
    pathlinesPeriod = obj.pathlinesPeriod;
    pathlinesCMFE = obj.pathlinesCMFE;
    displayGeometry = obj.displayGeometry;
    cropBeginFlag = obj.cropBeginFlag;
    cropBegin = obj.cropBegin;
    cropEndFlag = obj.cropEndFlag;
    cropEnd = obj.cropEnd;
    cropValue = obj.cropValue;
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
    correlationDistanceAngTol = obj.correlationDistanceAngTol;
    correlationDistanceMinDistAbsolute = obj.correlationDistanceMinDistAbsolute;
    correlationDistanceMinDistBBox = obj.correlationDistanceMinDistBBox;
    correlationDistanceMinDistType = obj.correlationDistanceMinDistType;
    selection = obj.selection;

    IntegralCurveAttributes::SelectAll();
}

// Type map format string
const char *IntegralCurveAttributes::TypeMapFormatString = INTEGRALCURVEATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t IntegralCurveAttributes::TmfsStruct = {INTEGRALCURVEATTRIBUTES_TMFS};


// ****************************************************************************
// Method: IntegralCurveAttributes::IntegralCurveAttributes
//
// Purpose: 
//   Default constructor for the IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

IntegralCurveAttributes::IntegralCurveAttributes() : 
    AttributeSubject(IntegralCurveAttributes::TypeMapFormatString)
{
    IntegralCurveAttributes::Init();
}

// ****************************************************************************
// Method: IntegralCurveAttributes::IntegralCurveAttributes
//
// Purpose: 
//   Constructor for the derived classes of IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

IntegralCurveAttributes::IntegralCurveAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    IntegralCurveAttributes::Init();
}

// ****************************************************************************
// Method: IntegralCurveAttributes::IntegralCurveAttributes
//
// Purpose: 
//   Copy constructor for the IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

IntegralCurveAttributes::IntegralCurveAttributes(const IntegralCurveAttributes &obj) : 
    AttributeSubject(IntegralCurveAttributes::TypeMapFormatString)
{
    IntegralCurveAttributes::Copy(obj);
}

// ****************************************************************************
// Method: IntegralCurveAttributes::IntegralCurveAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

IntegralCurveAttributes::IntegralCurveAttributes(const IntegralCurveAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    IntegralCurveAttributes::Copy(obj);
}

// ****************************************************************************
// Method: IntegralCurveAttributes::~IntegralCurveAttributes
//
// Purpose: 
//   Destructor for the IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

IntegralCurveAttributes::~IntegralCurveAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: IntegralCurveAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the IntegralCurveAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

IntegralCurveAttributes& 
IntegralCurveAttributes::operator = (const IntegralCurveAttributes &obj)
{
    if (this == &obj) return *this;

    IntegralCurveAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: IntegralCurveAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the IntegralCurveAttributes class.
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
IntegralCurveAttributes::operator == (const IntegralCurveAttributes &obj) const
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

    // Compare the velocitySource arrays.
    bool velocitySource_equal = true;
    for(int i = 0; i < 3 && velocitySource_equal; ++i)
        velocitySource_equal = (velocitySource[i] == obj.velocitySource[i]);

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
            (dataValue == obj.dataValue) &&
            (dataVariable == obj.dataVariable) &&
            (integrationDirection == obj.integrationDirection) &&
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
            (fieldType == obj.fieldType) &&
            (fieldConstant == obj.fieldConstant) &&
            velocitySource_equal &&
            (integrationType == obj.integrationType) &&
            (parallelizationAlgorithmType == obj.parallelizationAlgorithmType) &&
            (maxProcessCount == obj.maxProcessCount) &&
            (maxDomainCacheSize == obj.maxDomainCacheSize) &&
            (workGroupSize == obj.workGroupSize) &&
            (pathlines == obj.pathlines) &&
            (pathlinesOverrideStartingTimeFlag == obj.pathlinesOverrideStartingTimeFlag) &&
            (pathlinesOverrideStartingTime == obj.pathlinesOverrideStartingTime) &&
            (pathlinesPeriod == obj.pathlinesPeriod) &&
            (pathlinesCMFE == obj.pathlinesCMFE) &&
            (displayGeometry == obj.displayGeometry) &&
            (cropBeginFlag == obj.cropBeginFlag) &&
            (cropBegin == obj.cropBegin) &&
            (cropEndFlag == obj.cropEndFlag) &&
            (cropEnd == obj.cropEnd) &&
            (cropValue == obj.cropValue) &&
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
            (criticalPointThreshold == obj.criticalPointThreshold) &&
            (correlationDistanceAngTol == obj.correlationDistanceAngTol) &&
            (correlationDistanceMinDistAbsolute == obj.correlationDistanceMinDistAbsolute) &&
            (correlationDistanceMinDistBBox == obj.correlationDistanceMinDistBBox) &&
            (correlationDistanceMinDistType == obj.correlationDistanceMinDistType) &&
            (selection == obj.selection));
}

// ****************************************************************************
// Method: IntegralCurveAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the IntegralCurveAttributes class.
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
IntegralCurveAttributes::operator != (const IntegralCurveAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: IntegralCurveAttributes::TypeName
//
// Purpose: 
//   Type name method for the IntegralCurveAttributes class.
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
IntegralCurveAttributes::TypeName() const
{
    return "IntegralCurveAttributes";
}

// ****************************************************************************
// Method: StreamlineAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the IntegralCurveAttributes class.
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
IntegralCurveAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const IntegralCurveAttributes *tmp = (const IntegralCurveAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    else if(atts->TypeName() == "PointAttributes")
    {
        if(sourceType == Point)
        {
            const PointAttributes *p = (PointAttributes *)atts;
            SetPointSource(p->GetPoint());
            retval = true;
        }
    } 
    else if(atts->TypeName() == "Line")
    {
        if(sourceType == Line_)
        {
            const Line *line = (const Line *)atts;
            SetLineStart(line->GetPoint1());
            SetLineEnd(line->GetPoint2());
            retval = true;
        }
    }
    else if(atts->TypeName() == "PlaneAttributes")
    {
        if(sourceType == Plane || sourceType == Circle)
        {
            const PlaneAttributes *plane = (const PlaneAttributes *)atts;
            SetPlaneOrigin(plane->GetOrigin());
            SetPlaneNormal(plane->GetNormal());
            SetPlaneUpAxis(plane->GetUpAxis());
            if (sourceType == Circle)
                SetRadius(plane->GetRadius());
            retval = true;
        }
    }
    else if(atts->TypeName() == "SphereAttributes")
    {
        if(sourceType == Sphere)
        {
            const SphereAttributes *sphere = (const SphereAttributes *)atts;
            SetSphereOrigin(sphere->GetOrigin());
            SetRadius(sphere->GetRadius());
            retval = true;
        }
    }   
    else if(atts->TypeName() == "BoxExtents")
    {
        if(sourceType == Box)
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
//  Method: IntegralCurveAttributes::CreateCompatible
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
IntegralCurveAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;

    if(TypeName() == tname)
    {
        retval = new IntegralCurveAttributes(*this);
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
// Method: IntegralCurveAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the IntegralCurveAttributes class.
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
IntegralCurveAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new IntegralCurveAttributes(*this);
    else
        retval = new IntegralCurveAttributes;

    return retval;
}

// ****************************************************************************
// Method: IntegralCurveAttributes::SelectAll
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
IntegralCurveAttributes::SelectAll()
{
    Select(ID_sourceType,                         (void *)&sourceType);
    Select(ID_pointSource,                        (void *)pointSource, 3);
    Select(ID_lineStart,                          (void *)lineStart, 3);
    Select(ID_lineEnd,                            (void *)lineEnd, 3);
    Select(ID_planeOrigin,                        (void *)planeOrigin, 3);
    Select(ID_planeNormal,                        (void *)planeNormal, 3);
    Select(ID_planeUpAxis,                        (void *)planeUpAxis, 3);
    Select(ID_radius,                             (void *)&radius);
    Select(ID_sphereOrigin,                       (void *)sphereOrigin, 3);
    Select(ID_boxExtents,                         (void *)boxExtents, 6);
    Select(ID_useWholeBox,                        (void *)&useWholeBox);
    Select(ID_pointList,                          (void *)&pointList);
    Select(ID_sampleDensity0,                     (void *)&sampleDensity0);
    Select(ID_sampleDensity1,                     (void *)&sampleDensity1);
    Select(ID_sampleDensity2,                     (void *)&sampleDensity2);
    Select(ID_dataValue,                          (void *)&dataValue);
    Select(ID_dataVariable,                       (void *)&dataVariable);
    Select(ID_integrationDirection,               (void *)&integrationDirection);
    Select(ID_maxSteps,                           (void *)&maxSteps);
    Select(ID_terminateByDistance,                (void *)&terminateByDistance);
    Select(ID_termDistance,                       (void *)&termDistance);
    Select(ID_terminateByTime,                    (void *)&terminateByTime);
    Select(ID_termTime,                           (void *)&termTime);
    Select(ID_maxStepLength,                      (void *)&maxStepLength);
    Select(ID_limitMaximumTimestep,               (void *)&limitMaximumTimestep);
    Select(ID_maxTimeStep,                        (void *)&maxTimeStep);
    Select(ID_relTol,                             (void *)&relTol);
    Select(ID_absTolSizeType,                     (void *)&absTolSizeType);
    Select(ID_absTolAbsolute,                     (void *)&absTolAbsolute);
    Select(ID_absTolBBox,                         (void *)&absTolBBox);
    Select(ID_fieldType,                          (void *)&fieldType);
    Select(ID_fieldConstant,                      (void *)&fieldConstant);
    Select(ID_velocitySource,                     (void *)velocitySource, 3);
    Select(ID_integrationType,                    (void *)&integrationType);
    Select(ID_parallelizationAlgorithmType,       (void *)&parallelizationAlgorithmType);
    Select(ID_maxProcessCount,                    (void *)&maxProcessCount);
    Select(ID_maxDomainCacheSize,                 (void *)&maxDomainCacheSize);
    Select(ID_workGroupSize,                      (void *)&workGroupSize);
    Select(ID_pathlines,                          (void *)&pathlines);
    Select(ID_pathlinesOverrideStartingTimeFlag,  (void *)&pathlinesOverrideStartingTimeFlag);
    Select(ID_pathlinesOverrideStartingTime,      (void *)&pathlinesOverrideStartingTime);
    Select(ID_pathlinesPeriod,                    (void *)&pathlinesPeriod);
    Select(ID_pathlinesCMFE,                      (void *)&pathlinesCMFE);
    Select(ID_displayGeometry,                    (void *)&displayGeometry);
    Select(ID_cropBeginFlag,                      (void *)&cropBeginFlag);
    Select(ID_cropBegin,                          (void *)&cropBegin);
    Select(ID_cropEndFlag,                        (void *)&cropEndFlag);
    Select(ID_cropEnd,                            (void *)&cropEnd);
    Select(ID_cropValue,                          (void *)&cropValue);
    Select(ID_sampleDistance0,                    (void *)&sampleDistance0);
    Select(ID_sampleDistance1,                    (void *)&sampleDistance1);
    Select(ID_sampleDistance2,                    (void *)&sampleDistance2);
    Select(ID_fillInterior,                       (void *)&fillInterior);
    Select(ID_randomSamples,                      (void *)&randomSamples);
    Select(ID_randomSeed,                         (void *)&randomSeed);
    Select(ID_numberOfRandomSamples,              (void *)&numberOfRandomSamples);
    Select(ID_forceNodeCenteredData,              (void *)&forceNodeCenteredData);
    Select(ID_issueTerminationWarnings,           (void *)&issueTerminationWarnings);
    Select(ID_issueStiffnessWarnings,             (void *)&issueStiffnessWarnings);
    Select(ID_issueCriticalPointsWarnings,        (void *)&issueCriticalPointsWarnings);
    Select(ID_criticalPointThreshold,             (void *)&criticalPointThreshold);
    Select(ID_correlationDistanceAngTol,          (void *)&correlationDistanceAngTol);
    Select(ID_correlationDistanceMinDistAbsolute, (void *)&correlationDistanceMinDistAbsolute);
    Select(ID_correlationDistanceMinDistBBox,     (void *)&correlationDistanceMinDistBBox);
    Select(ID_correlationDistanceMinDistType,     (void *)&correlationDistanceMinDistType);
    Select(ID_selection,                          (void *)&selection);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: IntegralCurveAttributes::CreateNode
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
IntegralCurveAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    IntegralCurveAttributes defaultObject;
    bool addToParent = false;
    // Create a node for IntegralCurveAttributes.
    DataNode *node = new DataNode("IntegralCurveAttributes");

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

    if(completeSave || !FieldsEqual(ID_dataValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataValue", DataValue_ToString(dataValue)));
    }

    if(completeSave || !FieldsEqual(ID_dataVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataVariable", dataVariable));
    }

    if(completeSave || !FieldsEqual(ID_integrationDirection, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationDirection", IntegrationDirection_ToString(integrationDirection)));
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

    if(completeSave || !FieldsEqual(ID_fieldType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("fieldType", FieldType_ToString(fieldType)));
    }

    if(completeSave || !FieldsEqual(ID_fieldConstant, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("fieldConstant", fieldConstant));
    }

    if(completeSave || !FieldsEqual(ID_velocitySource, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("velocitySource", velocitySource, 3));
    }

    if(completeSave || !FieldsEqual(ID_integrationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationType", IntegrationType_ToString(integrationType)));
    }

    if(completeSave || !FieldsEqual(ID_parallelizationAlgorithmType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("parallelizationAlgorithmType", ParallelizationAlgorithmType_ToString(parallelizationAlgorithmType)));
    }

    if(completeSave || !FieldsEqual(ID_maxProcessCount, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxProcessCount", maxProcessCount));
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

    if(completeSave || !FieldsEqual(ID_pathlinesPeriod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesPeriod", pathlinesPeriod));
    }

    if(completeSave || !FieldsEqual(ID_pathlinesCMFE, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pathlinesCMFE", PathlinesCMFE_ToString(pathlinesCMFE)));
    }

    if(completeSave || !FieldsEqual(ID_displayGeometry, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayGeometry", DisplayGeometry_ToString(displayGeometry)));
    }

    if(completeSave || !FieldsEqual(ID_cropBeginFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cropBeginFlag", cropBeginFlag));
    }

    if(completeSave || !FieldsEqual(ID_cropBegin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cropBegin", cropBegin));
    }

    if(completeSave || !FieldsEqual(ID_cropEndFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cropEndFlag", cropEndFlag));
    }

    if(completeSave || !FieldsEqual(ID_cropEnd, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cropEnd", cropEnd));
    }

    if(completeSave || !FieldsEqual(ID_cropValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cropValue", CropValue_ToString(cropValue)));
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

    if(completeSave || !FieldsEqual(ID_correlationDistanceAngTol, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("correlationDistanceAngTol", correlationDistanceAngTol));
    }

    if(completeSave || !FieldsEqual(ID_correlationDistanceMinDistAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("correlationDistanceMinDistAbsolute", correlationDistanceMinDistAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_correlationDistanceMinDistBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("correlationDistanceMinDistBBox", correlationDistanceMinDistBBox));
    }

    if(completeSave || !FieldsEqual(ID_correlationDistanceMinDistType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("correlationDistanceMinDistType", SizeType_ToString(correlationDistanceMinDistType)));
    }

    if(completeSave || !FieldsEqual(ID_selection, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("selection", selection));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: IntegralCurveAttributes::SetFromNode
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
IntegralCurveAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("IntegralCurveAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("sourceType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 8)
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
    if((node = searchNode->GetNode("dataValue")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 11)
                SetDataValue(DataValue(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DataValue value;
            if(DataValue_FromString(node->AsString(), value))
                SetDataValue(value);
        }
    }
    if((node = searchNode->GetNode("dataVariable")) != 0)
        SetDataVariable(node->AsString());
    if((node = searchNode->GetNode("integrationDirection")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetIntegrationDirection(IntegrationDirection(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationDirection value;
            if(IntegrationDirection_FromString(node->AsString(), value))
                SetIntegrationDirection(value);
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
    if((node = searchNode->GetNode("fieldType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 7)
                SetFieldType(FieldType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            FieldType value;
            if(FieldType_FromString(node->AsString(), value))
                SetFieldType(value);
        }
    }
    if((node = searchNode->GetNode("fieldConstant")) != 0)
        SetFieldConstant(node->AsDouble());
    if((node = searchNode->GetNode("velocitySource")) != 0)
        SetVelocitySource(node->AsDoubleArray());
    if((node = searchNode->GetNode("integrationType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 6)
                SetIntegrationType(IntegrationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IntegrationType value;
            if(IntegrationType_FromString(node->AsString(), value))
                SetIntegrationType(value);
        }
    }
    if((node = searchNode->GetNode("parallelizationAlgorithmType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetParallelizationAlgorithmType(ParallelizationAlgorithmType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ParallelizationAlgorithmType value;
            if(ParallelizationAlgorithmType_FromString(node->AsString(), value))
                SetParallelizationAlgorithmType(value);
        }
    }
    if((node = searchNode->GetNode("maxProcessCount")) != 0)
        SetMaxProcessCount(node->AsInt());
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
    if((node = searchNode->GetNode("pathlinesPeriod")) != 0)
        SetPathlinesPeriod(node->AsDouble());
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
    if((node = searchNode->GetNode("displayGeometry")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDisplayGeometry(DisplayGeometry(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DisplayGeometry value;
            if(DisplayGeometry_FromString(node->AsString(), value))
                SetDisplayGeometry(value);
        }
    }
    if((node = searchNode->GetNode("cropBeginFlag")) != 0)
        SetCropBeginFlag(node->AsBool());
    if((node = searchNode->GetNode("cropBegin")) != 0)
        SetCropBegin(node->AsDouble());
    if((node = searchNode->GetNode("cropEndFlag")) != 0)
        SetCropEndFlag(node->AsBool());
    if((node = searchNode->GetNode("cropEnd")) != 0)
        SetCropEnd(node->AsDouble());
    if((node = searchNode->GetNode("cropValue")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetCropValue(CropValue(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CropValue value;
            if(CropValue_FromString(node->AsString(), value))
                SetCropValue(value);
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
    if((node = searchNode->GetNode("correlationDistanceAngTol")) != 0)
        SetCorrelationDistanceAngTol(node->AsDouble());
    if((node = searchNode->GetNode("correlationDistanceMinDistAbsolute")) != 0)
        SetCorrelationDistanceMinDistAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("correlationDistanceMinDistBBox")) != 0)
        SetCorrelationDistanceMinDistBBox(node->AsDouble());
    if((node = searchNode->GetNode("correlationDistanceMinDistType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetCorrelationDistanceMinDistType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetCorrelationDistanceMinDistType(value);
        }
    }
    if((node = searchNode->GetNode("selection")) != 0)
        SetSelection(node->AsString());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
IntegralCurveAttributes::SetSourceType(IntegralCurveAttributes::SourceType sourceType_)
{
    sourceType = sourceType_;
    Select(ID_sourceType, (void *)&sourceType);
}

void
IntegralCurveAttributes::SetPointSource(const double *pointSource_)
{
    pointSource[0] = pointSource_[0];
    pointSource[1] = pointSource_[1];
    pointSource[2] = pointSource_[2];
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
IntegralCurveAttributes::SetLineStart(const double *lineStart_)
{
    lineStart[0] = lineStart_[0];
    lineStart[1] = lineStart_[1];
    lineStart[2] = lineStart_[2];
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
IntegralCurveAttributes::SetLineEnd(const double *lineEnd_)
{
    lineEnd[0] = lineEnd_[0];
    lineEnd[1] = lineEnd_[1];
    lineEnd[2] = lineEnd_[2];
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
IntegralCurveAttributes::SetPlaneOrigin(const double *planeOrigin_)
{
    planeOrigin[0] = planeOrigin_[0];
    planeOrigin[1] = planeOrigin_[1];
    planeOrigin[2] = planeOrigin_[2];
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
IntegralCurveAttributes::SetPlaneNormal(const double *planeNormal_)
{
    planeNormal[0] = planeNormal_[0];
    planeNormal[1] = planeNormal_[1];
    planeNormal[2] = planeNormal_[2];
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
IntegralCurveAttributes::SetPlaneUpAxis(const double *planeUpAxis_)
{
    planeUpAxis[0] = planeUpAxis_[0];
    planeUpAxis[1] = planeUpAxis_[1];
    planeUpAxis[2] = planeUpAxis_[2];
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
IntegralCurveAttributes::SetRadius(double radius_)
{
    radius = radius_;
    Select(ID_radius, (void *)&radius);
}

void
IntegralCurveAttributes::SetSphereOrigin(const double *sphereOrigin_)
{
    sphereOrigin[0] = sphereOrigin_[0];
    sphereOrigin[1] = sphereOrigin_[1];
    sphereOrigin[2] = sphereOrigin_[2];
    Select(ID_sphereOrigin, (void *)sphereOrigin, 3);
}

void
IntegralCurveAttributes::SetBoxExtents(const double *boxExtents_)
{
    for(int i = 0; i < 6; ++i)
        boxExtents[i] = boxExtents_[i];
    Select(ID_boxExtents, (void *)boxExtents, 6);
}

void
IntegralCurveAttributes::SetUseWholeBox(bool useWholeBox_)
{
    useWholeBox = useWholeBox_;
    Select(ID_useWholeBox, (void *)&useWholeBox);
}

void
IntegralCurveAttributes::SetPointList(const doubleVector &pointList_)
{
    pointList = pointList_;
    Select(ID_pointList, (void *)&pointList);
}

void
IntegralCurveAttributes::SetSampleDensity0(int sampleDensity0_)
{
    sampleDensity0 = sampleDensity0_;
    Select(ID_sampleDensity0, (void *)&sampleDensity0);
}

void
IntegralCurveAttributes::SetSampleDensity1(int sampleDensity1_)
{
    sampleDensity1 = sampleDensity1_;
    Select(ID_sampleDensity1, (void *)&sampleDensity1);
}

void
IntegralCurveAttributes::SetSampleDensity2(int sampleDensity2_)
{
    sampleDensity2 = sampleDensity2_;
    Select(ID_sampleDensity2, (void *)&sampleDensity2);
}

void
IntegralCurveAttributes::SetDataValue(IntegralCurveAttributes::DataValue dataValue_)
{
    dataValue = dataValue_;
    Select(ID_dataValue, (void *)&dataValue);
}

void
IntegralCurveAttributes::SetDataVariable(const std::string &dataVariable_)
{
    dataVariable = dataVariable_;
    Select(ID_dataVariable, (void *)&dataVariable);
}

void
IntegralCurveAttributes::SetIntegrationDirection(IntegralCurveAttributes::IntegrationDirection integrationDirection_)
{
    integrationDirection = integrationDirection_;
    Select(ID_integrationDirection, (void *)&integrationDirection);
}

void
IntegralCurveAttributes::SetMaxSteps(int maxSteps_)
{
    maxSteps = maxSteps_;
    Select(ID_maxSteps, (void *)&maxSteps);
}

void
IntegralCurveAttributes::SetTerminateByDistance(bool terminateByDistance_)
{
    terminateByDistance = terminateByDistance_;
    Select(ID_terminateByDistance, (void *)&terminateByDistance);
}

void
IntegralCurveAttributes::SetTermDistance(double termDistance_)
{
    termDistance = termDistance_;
    Select(ID_termDistance, (void *)&termDistance);
}

void
IntegralCurveAttributes::SetTerminateByTime(bool terminateByTime_)
{
    terminateByTime = terminateByTime_;
    Select(ID_terminateByTime, (void *)&terminateByTime);
}

void
IntegralCurveAttributes::SetTermTime(double termTime_)
{
    termTime = termTime_;
    Select(ID_termTime, (void *)&termTime);
}

void
IntegralCurveAttributes::SetMaxStepLength(double maxStepLength_)
{
    maxStepLength = maxStepLength_;
    Select(ID_maxStepLength, (void *)&maxStepLength);
}

void
IntegralCurveAttributes::SetLimitMaximumTimestep(bool limitMaximumTimestep_)
{
    limitMaximumTimestep = limitMaximumTimestep_;
    Select(ID_limitMaximumTimestep, (void *)&limitMaximumTimestep);
}

void
IntegralCurveAttributes::SetMaxTimeStep(double maxTimeStep_)
{
    maxTimeStep = maxTimeStep_;
    Select(ID_maxTimeStep, (void *)&maxTimeStep);
}

void
IntegralCurveAttributes::SetRelTol(double relTol_)
{
    relTol = relTol_;
    Select(ID_relTol, (void *)&relTol);
}

void
IntegralCurveAttributes::SetAbsTolSizeType(IntegralCurveAttributes::SizeType absTolSizeType_)
{
    absTolSizeType = absTolSizeType_;
    Select(ID_absTolSizeType, (void *)&absTolSizeType);
}

void
IntegralCurveAttributes::SetAbsTolAbsolute(double absTolAbsolute_)
{
    absTolAbsolute = absTolAbsolute_;
    Select(ID_absTolAbsolute, (void *)&absTolAbsolute);
}

void
IntegralCurveAttributes::SetAbsTolBBox(double absTolBBox_)
{
    absTolBBox = absTolBBox_;
    Select(ID_absTolBBox, (void *)&absTolBBox);
}

void
IntegralCurveAttributes::SetFieldType(IntegralCurveAttributes::FieldType fieldType_)
{
    fieldType = fieldType_;
    Select(ID_fieldType, (void *)&fieldType);
}

void
IntegralCurveAttributes::SetFieldConstant(double fieldConstant_)
{
    fieldConstant = fieldConstant_;
    Select(ID_fieldConstant, (void *)&fieldConstant);
}

void
IntegralCurveAttributes::SetVelocitySource(const double *velocitySource_)
{
    velocitySource[0] = velocitySource_[0];
    velocitySource[1] = velocitySource_[1];
    velocitySource[2] = velocitySource_[2];
    Select(ID_velocitySource, (void *)velocitySource, 3);
}

void
IntegralCurveAttributes::SetIntegrationType(IntegralCurveAttributes::IntegrationType integrationType_)
{
    integrationType = integrationType_;
    Select(ID_integrationType, (void *)&integrationType);
}

void
IntegralCurveAttributes::SetParallelizationAlgorithmType(IntegralCurveAttributes::ParallelizationAlgorithmType parallelizationAlgorithmType_)
{
    parallelizationAlgorithmType = parallelizationAlgorithmType_;
    Select(ID_parallelizationAlgorithmType, (void *)&parallelizationAlgorithmType);
}

void
IntegralCurveAttributes::SetMaxProcessCount(int maxProcessCount_)
{
    maxProcessCount = maxProcessCount_;
    Select(ID_maxProcessCount, (void *)&maxProcessCount);
}

void
IntegralCurveAttributes::SetMaxDomainCacheSize(int maxDomainCacheSize_)
{
    maxDomainCacheSize = maxDomainCacheSize_;
    Select(ID_maxDomainCacheSize, (void *)&maxDomainCacheSize);
}

void
IntegralCurveAttributes::SetWorkGroupSize(int workGroupSize_)
{
    workGroupSize = workGroupSize_;
    Select(ID_workGroupSize, (void *)&workGroupSize);
}

void
IntegralCurveAttributes::SetPathlines(bool pathlines_)
{
    pathlines = pathlines_;
    Select(ID_pathlines, (void *)&pathlines);
}

void
IntegralCurveAttributes::SetPathlinesOverrideStartingTimeFlag(bool pathlinesOverrideStartingTimeFlag_)
{
    pathlinesOverrideStartingTimeFlag = pathlinesOverrideStartingTimeFlag_;
    Select(ID_pathlinesOverrideStartingTimeFlag, (void *)&pathlinesOverrideStartingTimeFlag);
}

void
IntegralCurveAttributes::SetPathlinesOverrideStartingTime(double pathlinesOverrideStartingTime_)
{
    pathlinesOverrideStartingTime = pathlinesOverrideStartingTime_;
    Select(ID_pathlinesOverrideStartingTime, (void *)&pathlinesOverrideStartingTime);
}

void
IntegralCurveAttributes::SetPathlinesPeriod(double pathlinesPeriod_)
{
    pathlinesPeriod = pathlinesPeriod_;
    Select(ID_pathlinesPeriod, (void *)&pathlinesPeriod);
}

void
IntegralCurveAttributes::SetPathlinesCMFE(IntegralCurveAttributes::PathlinesCMFE pathlinesCMFE_)
{
    pathlinesCMFE = pathlinesCMFE_;
    Select(ID_pathlinesCMFE, (void *)&pathlinesCMFE);
}

void
IntegralCurveAttributes::SetDisplayGeometry(IntegralCurveAttributes::DisplayGeometry displayGeometry_)
{
    displayGeometry = displayGeometry_;
    Select(ID_displayGeometry, (void *)&displayGeometry);
}

void
IntegralCurveAttributes::SetCropBeginFlag(bool cropBeginFlag_)
{
    cropBeginFlag = cropBeginFlag_;
    Select(ID_cropBeginFlag, (void *)&cropBeginFlag);
}

void
IntegralCurveAttributes::SetCropBegin(double cropBegin_)
{
    cropBegin = cropBegin_;
    Select(ID_cropBegin, (void *)&cropBegin);
}

void
IntegralCurveAttributes::SetCropEndFlag(bool cropEndFlag_)
{
    cropEndFlag = cropEndFlag_;
    Select(ID_cropEndFlag, (void *)&cropEndFlag);
}

void
IntegralCurveAttributes::SetCropEnd(double cropEnd_)
{
    cropEnd = cropEnd_;
    Select(ID_cropEnd, (void *)&cropEnd);
}

void
IntegralCurveAttributes::SetCropValue(IntegralCurveAttributes::CropValue cropValue_)
{
    cropValue = cropValue_;
    Select(ID_cropValue, (void *)&cropValue);
}

void
IntegralCurveAttributes::SetSampleDistance0(double sampleDistance0_)
{
    sampleDistance0 = sampleDistance0_;
    Select(ID_sampleDistance0, (void *)&sampleDistance0);
}

void
IntegralCurveAttributes::SetSampleDistance1(double sampleDistance1_)
{
    sampleDistance1 = sampleDistance1_;
    Select(ID_sampleDistance1, (void *)&sampleDistance1);
}

void
IntegralCurveAttributes::SetSampleDistance2(double sampleDistance2_)
{
    sampleDistance2 = sampleDistance2_;
    Select(ID_sampleDistance2, (void *)&sampleDistance2);
}

void
IntegralCurveAttributes::SetFillInterior(bool fillInterior_)
{
    fillInterior = fillInterior_;
    Select(ID_fillInterior, (void *)&fillInterior);
}

void
IntegralCurveAttributes::SetRandomSamples(bool randomSamples_)
{
    randomSamples = randomSamples_;
    Select(ID_randomSamples, (void *)&randomSamples);
}

void
IntegralCurveAttributes::SetRandomSeed(int randomSeed_)
{
    randomSeed = randomSeed_;
    Select(ID_randomSeed, (void *)&randomSeed);
}

void
IntegralCurveAttributes::SetNumberOfRandomSamples(int numberOfRandomSamples_)
{
    numberOfRandomSamples = numberOfRandomSamples_;
    Select(ID_numberOfRandomSamples, (void *)&numberOfRandomSamples);
}

void
IntegralCurveAttributes::SetForceNodeCenteredData(bool forceNodeCenteredData_)
{
    forceNodeCenteredData = forceNodeCenteredData_;
    Select(ID_forceNodeCenteredData, (void *)&forceNodeCenteredData);
}

void
IntegralCurveAttributes::SetIssueTerminationWarnings(bool issueTerminationWarnings_)
{
    issueTerminationWarnings = issueTerminationWarnings_;
    Select(ID_issueTerminationWarnings, (void *)&issueTerminationWarnings);
}

void
IntegralCurveAttributes::SetIssueStiffnessWarnings(bool issueStiffnessWarnings_)
{
    issueStiffnessWarnings = issueStiffnessWarnings_;
    Select(ID_issueStiffnessWarnings, (void *)&issueStiffnessWarnings);
}

void
IntegralCurveAttributes::SetIssueCriticalPointsWarnings(bool issueCriticalPointsWarnings_)
{
    issueCriticalPointsWarnings = issueCriticalPointsWarnings_;
    Select(ID_issueCriticalPointsWarnings, (void *)&issueCriticalPointsWarnings);
}

void
IntegralCurveAttributes::SetCriticalPointThreshold(double criticalPointThreshold_)
{
    criticalPointThreshold = criticalPointThreshold_;
    Select(ID_criticalPointThreshold, (void *)&criticalPointThreshold);
}

void
IntegralCurveAttributes::SetCorrelationDistanceAngTol(double correlationDistanceAngTol_)
{
    correlationDistanceAngTol = correlationDistanceAngTol_;
    Select(ID_correlationDistanceAngTol, (void *)&correlationDistanceAngTol);
}

void
IntegralCurveAttributes::SetCorrelationDistanceMinDistAbsolute(double correlationDistanceMinDistAbsolute_)
{
    correlationDistanceMinDistAbsolute = correlationDistanceMinDistAbsolute_;
    Select(ID_correlationDistanceMinDistAbsolute, (void *)&correlationDistanceMinDistAbsolute);
}

void
IntegralCurveAttributes::SetCorrelationDistanceMinDistBBox(double correlationDistanceMinDistBBox_)
{
    correlationDistanceMinDistBBox = correlationDistanceMinDistBBox_;
    Select(ID_correlationDistanceMinDistBBox, (void *)&correlationDistanceMinDistBBox);
}

void
IntegralCurveAttributes::SetCorrelationDistanceMinDistType(IntegralCurveAttributes::SizeType correlationDistanceMinDistType_)
{
    correlationDistanceMinDistType = correlationDistanceMinDistType_;
    Select(ID_correlationDistanceMinDistType, (void *)&correlationDistanceMinDistType);
}

void
IntegralCurveAttributes::SetSelection(const std::string &selection_)
{
    selection = selection_;
    Select(ID_selection, (void *)&selection);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

IntegralCurveAttributes::SourceType
IntegralCurveAttributes::GetSourceType() const
{
    return SourceType(sourceType);
}

const double *
IntegralCurveAttributes::GetPointSource() const
{
    return pointSource;
}

double *
IntegralCurveAttributes::GetPointSource()
{
    return pointSource;
}

const double *
IntegralCurveAttributes::GetLineStart() const
{
    return lineStart;
}

double *
IntegralCurveAttributes::GetLineStart()
{
    return lineStart;
}

const double *
IntegralCurveAttributes::GetLineEnd() const
{
    return lineEnd;
}

double *
IntegralCurveAttributes::GetLineEnd()
{
    return lineEnd;
}

const double *
IntegralCurveAttributes::GetPlaneOrigin() const
{
    return planeOrigin;
}

double *
IntegralCurveAttributes::GetPlaneOrigin()
{
    return planeOrigin;
}

const double *
IntegralCurveAttributes::GetPlaneNormal() const
{
    return planeNormal;
}

double *
IntegralCurveAttributes::GetPlaneNormal()
{
    return planeNormal;
}

const double *
IntegralCurveAttributes::GetPlaneUpAxis() const
{
    return planeUpAxis;
}

double *
IntegralCurveAttributes::GetPlaneUpAxis()
{
    return planeUpAxis;
}

double
IntegralCurveAttributes::GetRadius() const
{
    return radius;
}

const double *
IntegralCurveAttributes::GetSphereOrigin() const
{
    return sphereOrigin;
}

double *
IntegralCurveAttributes::GetSphereOrigin()
{
    return sphereOrigin;
}

const double *
IntegralCurveAttributes::GetBoxExtents() const
{
    return boxExtents;
}

double *
IntegralCurveAttributes::GetBoxExtents()
{
    return boxExtents;
}

bool
IntegralCurveAttributes::GetUseWholeBox() const
{
    return useWholeBox;
}

const doubleVector &
IntegralCurveAttributes::GetPointList() const
{
    return pointList;
}

doubleVector &
IntegralCurveAttributes::GetPointList()
{
    return pointList;
}

int
IntegralCurveAttributes::GetSampleDensity0() const
{
    return sampleDensity0;
}

int
IntegralCurveAttributes::GetSampleDensity1() const
{
    return sampleDensity1;
}

int
IntegralCurveAttributes::GetSampleDensity2() const
{
    return sampleDensity2;
}

IntegralCurveAttributes::DataValue
IntegralCurveAttributes::GetDataValue() const
{
    return DataValue(dataValue);
}

const std::string &
IntegralCurveAttributes::GetDataVariable() const
{
    return dataVariable;
}

std::string &
IntegralCurveAttributes::GetDataVariable()
{
    return dataVariable;
}

IntegralCurveAttributes::IntegrationDirection
IntegralCurveAttributes::GetIntegrationDirection() const
{
    return IntegrationDirection(integrationDirection);
}

int
IntegralCurveAttributes::GetMaxSteps() const
{
    return maxSteps;
}

bool
IntegralCurveAttributes::GetTerminateByDistance() const
{
    return terminateByDistance;
}

double
IntegralCurveAttributes::GetTermDistance() const
{
    return termDistance;
}

bool
IntegralCurveAttributes::GetTerminateByTime() const
{
    return terminateByTime;
}

double
IntegralCurveAttributes::GetTermTime() const
{
    return termTime;
}

double
IntegralCurveAttributes::GetMaxStepLength() const
{
    return maxStepLength;
}

bool
IntegralCurveAttributes::GetLimitMaximumTimestep() const
{
    return limitMaximumTimestep;
}

double
IntegralCurveAttributes::GetMaxTimeStep() const
{
    return maxTimeStep;
}

double
IntegralCurveAttributes::GetRelTol() const
{
    return relTol;
}

IntegralCurveAttributes::SizeType
IntegralCurveAttributes::GetAbsTolSizeType() const
{
    return SizeType(absTolSizeType);
}

double
IntegralCurveAttributes::GetAbsTolAbsolute() const
{
    return absTolAbsolute;
}

double
IntegralCurveAttributes::GetAbsTolBBox() const
{
    return absTolBBox;
}

IntegralCurveAttributes::FieldType
IntegralCurveAttributes::GetFieldType() const
{
    return FieldType(fieldType);
}

double
IntegralCurveAttributes::GetFieldConstant() const
{
    return fieldConstant;
}

const double *
IntegralCurveAttributes::GetVelocitySource() const
{
    return velocitySource;
}

double *
IntegralCurveAttributes::GetVelocitySource()
{
    return velocitySource;
}

IntegralCurveAttributes::IntegrationType
IntegralCurveAttributes::GetIntegrationType() const
{
    return IntegrationType(integrationType);
}

IntegralCurveAttributes::ParallelizationAlgorithmType
IntegralCurveAttributes::GetParallelizationAlgorithmType() const
{
    return ParallelizationAlgorithmType(parallelizationAlgorithmType);
}

int
IntegralCurveAttributes::GetMaxProcessCount() const
{
    return maxProcessCount;
}

int
IntegralCurveAttributes::GetMaxDomainCacheSize() const
{
    return maxDomainCacheSize;
}

int
IntegralCurveAttributes::GetWorkGroupSize() const
{
    return workGroupSize;
}

bool
IntegralCurveAttributes::GetPathlines() const
{
    return pathlines;
}

bool
IntegralCurveAttributes::GetPathlinesOverrideStartingTimeFlag() const
{
    return pathlinesOverrideStartingTimeFlag;
}

double
IntegralCurveAttributes::GetPathlinesOverrideStartingTime() const
{
    return pathlinesOverrideStartingTime;
}

double
IntegralCurveAttributes::GetPathlinesPeriod() const
{
    return pathlinesPeriod;
}

IntegralCurveAttributes::PathlinesCMFE
IntegralCurveAttributes::GetPathlinesCMFE() const
{
    return PathlinesCMFE(pathlinesCMFE);
}

IntegralCurveAttributes::DisplayGeometry
IntegralCurveAttributes::GetDisplayGeometry() const
{
    return DisplayGeometry(displayGeometry);
}

bool
IntegralCurveAttributes::GetCropBeginFlag() const
{
    return cropBeginFlag;
}

double
IntegralCurveAttributes::GetCropBegin() const
{
    return cropBegin;
}

bool
IntegralCurveAttributes::GetCropEndFlag() const
{
    return cropEndFlag;
}

double
IntegralCurveAttributes::GetCropEnd() const
{
    return cropEnd;
}

IntegralCurveAttributes::CropValue
IntegralCurveAttributes::GetCropValue() const
{
    return CropValue(cropValue);
}

double
IntegralCurveAttributes::GetSampleDistance0() const
{
    return sampleDistance0;
}

double
IntegralCurveAttributes::GetSampleDistance1() const
{
    return sampleDistance1;
}

double
IntegralCurveAttributes::GetSampleDistance2() const
{
    return sampleDistance2;
}

bool
IntegralCurveAttributes::GetFillInterior() const
{
    return fillInterior;
}

bool
IntegralCurveAttributes::GetRandomSamples() const
{
    return randomSamples;
}

int
IntegralCurveAttributes::GetRandomSeed() const
{
    return randomSeed;
}

int
IntegralCurveAttributes::GetNumberOfRandomSamples() const
{
    return numberOfRandomSamples;
}

bool
IntegralCurveAttributes::GetForceNodeCenteredData() const
{
    return forceNodeCenteredData;
}

bool
IntegralCurveAttributes::GetIssueTerminationWarnings() const
{
    return issueTerminationWarnings;
}

bool
IntegralCurveAttributes::GetIssueStiffnessWarnings() const
{
    return issueStiffnessWarnings;
}

bool
IntegralCurveAttributes::GetIssueCriticalPointsWarnings() const
{
    return issueCriticalPointsWarnings;
}

double
IntegralCurveAttributes::GetCriticalPointThreshold() const
{
    return criticalPointThreshold;
}

double
IntegralCurveAttributes::GetCorrelationDistanceAngTol() const
{
    return correlationDistanceAngTol;
}

double
IntegralCurveAttributes::GetCorrelationDistanceMinDistAbsolute() const
{
    return correlationDistanceMinDistAbsolute;
}

double
IntegralCurveAttributes::GetCorrelationDistanceMinDistBBox() const
{
    return correlationDistanceMinDistBBox;
}

IntegralCurveAttributes::SizeType
IntegralCurveAttributes::GetCorrelationDistanceMinDistType() const
{
    return SizeType(correlationDistanceMinDistType);
}

const std::string &
IntegralCurveAttributes::GetSelection() const
{
    return selection;
}

std::string &
IntegralCurveAttributes::GetSelection()
{
    return selection;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
IntegralCurveAttributes::SelectPointSource()
{
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
IntegralCurveAttributes::SelectLineStart()
{
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
IntegralCurveAttributes::SelectLineEnd()
{
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
IntegralCurveAttributes::SelectPlaneOrigin()
{
    Select(ID_planeOrigin, (void *)planeOrigin, 3);
}

void
IntegralCurveAttributes::SelectPlaneNormal()
{
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
IntegralCurveAttributes::SelectPlaneUpAxis()
{
    Select(ID_planeUpAxis, (void *)planeUpAxis, 3);
}

void
IntegralCurveAttributes::SelectSphereOrigin()
{
    Select(ID_sphereOrigin, (void *)sphereOrigin, 3);
}

void
IntegralCurveAttributes::SelectBoxExtents()
{
    Select(ID_boxExtents, (void *)boxExtents, 6);
}

void
IntegralCurveAttributes::SelectPointList()
{
    Select(ID_pointList, (void *)&pointList);
}

void
IntegralCurveAttributes::SelectDataVariable()
{
    Select(ID_dataVariable, (void *)&dataVariable);
}

void
IntegralCurveAttributes::SelectVelocitySource()
{
    Select(ID_velocitySource, (void *)velocitySource, 3);
}

void
IntegralCurveAttributes::SelectSelection()
{
    Select(ID_selection, (void *)&selection);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: IntegralCurveAttributes::GetFieldName
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
IntegralCurveAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_sourceType:                         return "sourceType";
    case ID_pointSource:                        return "pointSource";
    case ID_lineStart:                          return "lineStart";
    case ID_lineEnd:                            return "lineEnd";
    case ID_planeOrigin:                        return "planeOrigin";
    case ID_planeNormal:                        return "planeNormal";
    case ID_planeUpAxis:                        return "planeUpAxis";
    case ID_radius:                             return "radius";
    case ID_sphereOrigin:                       return "sphereOrigin";
    case ID_boxExtents:                         return "boxExtents";
    case ID_useWholeBox:                        return "useWholeBox";
    case ID_pointList:                          return "pointList";
    case ID_sampleDensity0:                     return "sampleDensity0";
    case ID_sampleDensity1:                     return "sampleDensity1";
    case ID_sampleDensity2:                     return "sampleDensity2";
    case ID_dataValue:                          return "dataValue";
    case ID_dataVariable:                       return "dataVariable";
    case ID_integrationDirection:               return "integrationDirection";
    case ID_maxSteps:                           return "maxSteps";
    case ID_terminateByDistance:                return "terminateByDistance";
    case ID_termDistance:                       return "termDistance";
    case ID_terminateByTime:                    return "terminateByTime";
    case ID_termTime:                           return "termTime";
    case ID_maxStepLength:                      return "maxStepLength";
    case ID_limitMaximumTimestep:               return "limitMaximumTimestep";
    case ID_maxTimeStep:                        return "maxTimeStep";
    case ID_relTol:                             return "relTol";
    case ID_absTolSizeType:                     return "absTolSizeType";
    case ID_absTolAbsolute:                     return "absTolAbsolute";
    case ID_absTolBBox:                         return "absTolBBox";
    case ID_fieldType:                          return "fieldType";
    case ID_fieldConstant:                      return "fieldConstant";
    case ID_velocitySource:                     return "velocitySource";
    case ID_integrationType:                    return "integrationType";
    case ID_parallelizationAlgorithmType:       return "parallelizationAlgorithmType";
    case ID_maxProcessCount:                    return "maxProcessCount";
    case ID_maxDomainCacheSize:                 return "maxDomainCacheSize";
    case ID_workGroupSize:                      return "workGroupSize";
    case ID_pathlines:                          return "pathlines";
    case ID_pathlinesOverrideStartingTimeFlag:  return "pathlinesOverrideStartingTimeFlag";
    case ID_pathlinesOverrideStartingTime:      return "pathlinesOverrideStartingTime";
    case ID_pathlinesPeriod:                    return "pathlinesPeriod";
    case ID_pathlinesCMFE:                      return "pathlinesCMFE";
    case ID_displayGeometry:                    return "displayGeometry";
    case ID_cropBeginFlag:                      return "cropBeginFlag";
    case ID_cropBegin:                          return "cropBegin";
    case ID_cropEndFlag:                        return "cropEndFlag";
    case ID_cropEnd:                            return "cropEnd";
    case ID_cropValue:                          return "cropValue";
    case ID_sampleDistance0:                    return "sampleDistance0";
    case ID_sampleDistance1:                    return "sampleDistance1";
    case ID_sampleDistance2:                    return "sampleDistance2";
    case ID_fillInterior:                       return "fillInterior";
    case ID_randomSamples:                      return "randomSamples";
    case ID_randomSeed:                         return "randomSeed";
    case ID_numberOfRandomSamples:              return "numberOfRandomSamples";
    case ID_forceNodeCenteredData:              return "forceNodeCenteredData";
    case ID_issueTerminationWarnings:           return "issueTerminationWarnings";
    case ID_issueStiffnessWarnings:             return "issueStiffnessWarnings";
    case ID_issueCriticalPointsWarnings:        return "issueCriticalPointsWarnings";
    case ID_criticalPointThreshold:             return "criticalPointThreshold";
    case ID_correlationDistanceAngTol:          return "correlationDistanceAngTol";
    case ID_correlationDistanceMinDistAbsolute: return "correlationDistanceMinDistAbsolute";
    case ID_correlationDistanceMinDistBBox:     return "correlationDistanceMinDistBBox";
    case ID_correlationDistanceMinDistType:     return "correlationDistanceMinDistType";
    case ID_selection:                          return "selection";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: IntegralCurveAttributes::GetFieldType
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
IntegralCurveAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_sourceType:                         return FieldType_enum;
    case ID_pointSource:                        return FieldType_doubleArray;
    case ID_lineStart:                          return FieldType_doubleArray;
    case ID_lineEnd:                            return FieldType_doubleArray;
    case ID_planeOrigin:                        return FieldType_doubleArray;
    case ID_planeNormal:                        return FieldType_doubleArray;
    case ID_planeUpAxis:                        return FieldType_doubleArray;
    case ID_radius:                             return FieldType_double;
    case ID_sphereOrigin:                       return FieldType_doubleArray;
    case ID_boxExtents:                         return FieldType_doubleArray;
    case ID_useWholeBox:                        return FieldType_bool;
    case ID_pointList:                          return FieldType_doubleVector;
    case ID_sampleDensity0:                     return FieldType_int;
    case ID_sampleDensity1:                     return FieldType_int;
    case ID_sampleDensity2:                     return FieldType_int;
    case ID_dataValue:                          return FieldType_enum;
    case ID_dataVariable:                       return FieldType_string;
    case ID_integrationDirection:               return FieldType_enum;
    case ID_maxSteps:                           return FieldType_int;
    case ID_terminateByDistance:                return FieldType_bool;
    case ID_termDistance:                       return FieldType_double;
    case ID_terminateByTime:                    return FieldType_bool;
    case ID_termTime:                           return FieldType_double;
    case ID_maxStepLength:                      return FieldType_double;
    case ID_limitMaximumTimestep:               return FieldType_bool;
    case ID_maxTimeStep:                        return FieldType_double;
    case ID_relTol:                             return FieldType_double;
    case ID_absTolSizeType:                     return FieldType_enum;
    case ID_absTolAbsolute:                     return FieldType_double;
    case ID_absTolBBox:                         return FieldType_double;
    case ID_fieldType:                          return FieldType_enum;
    case ID_fieldConstant:                      return FieldType_double;
    case ID_velocitySource:                     return FieldType_doubleArray;
    case ID_integrationType:                    return FieldType_enum;
    case ID_parallelizationAlgorithmType:       return FieldType_enum;
    case ID_maxProcessCount:                    return FieldType_int;
    case ID_maxDomainCacheSize:                 return FieldType_int;
    case ID_workGroupSize:                      return FieldType_int;
    case ID_pathlines:                          return FieldType_bool;
    case ID_pathlinesOverrideStartingTimeFlag:  return FieldType_bool;
    case ID_pathlinesOverrideStartingTime:      return FieldType_double;
    case ID_pathlinesPeriod:                    return FieldType_double;
    case ID_pathlinesCMFE:                      return FieldType_enum;
    case ID_displayGeometry:                    return FieldType_enum;
    case ID_cropBeginFlag:                      return FieldType_bool;
    case ID_cropBegin:                          return FieldType_double;
    case ID_cropEndFlag:                        return FieldType_bool;
    case ID_cropEnd:                            return FieldType_double;
    case ID_cropValue:                          return FieldType_enum;
    case ID_sampleDistance0:                    return FieldType_double;
    case ID_sampleDistance1:                    return FieldType_double;
    case ID_sampleDistance2:                    return FieldType_double;
    case ID_fillInterior:                       return FieldType_bool;
    case ID_randomSamples:                      return FieldType_bool;
    case ID_randomSeed:                         return FieldType_int;
    case ID_numberOfRandomSamples:              return FieldType_int;
    case ID_forceNodeCenteredData:              return FieldType_bool;
    case ID_issueTerminationWarnings:           return FieldType_bool;
    case ID_issueStiffnessWarnings:             return FieldType_bool;
    case ID_issueCriticalPointsWarnings:        return FieldType_bool;
    case ID_criticalPointThreshold:             return FieldType_double;
    case ID_correlationDistanceAngTol:          return FieldType_double;
    case ID_correlationDistanceMinDistAbsolute: return FieldType_double;
    case ID_correlationDistanceMinDistBBox:     return FieldType_double;
    case ID_correlationDistanceMinDistType:     return FieldType_enum;
    case ID_selection:                          return FieldType_string;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: IntegralCurveAttributes::GetFieldTypeName
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
IntegralCurveAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_sourceType:                         return "enum";
    case ID_pointSource:                        return "doubleArray";
    case ID_lineStart:                          return "doubleArray";
    case ID_lineEnd:                            return "doubleArray";
    case ID_planeOrigin:                        return "doubleArray";
    case ID_planeNormal:                        return "doubleArray";
    case ID_planeUpAxis:                        return "doubleArray";
    case ID_radius:                             return "double";
    case ID_sphereOrigin:                       return "doubleArray";
    case ID_boxExtents:                         return "doubleArray";
    case ID_useWholeBox:                        return "bool";
    case ID_pointList:                          return "doubleVector";
    case ID_sampleDensity0:                     return "int";
    case ID_sampleDensity1:                     return "int";
    case ID_sampleDensity2:                     return "int";
    case ID_dataValue:                          return "enum";
    case ID_dataVariable:                       return "string";
    case ID_integrationDirection:               return "enum";
    case ID_maxSteps:                           return "int";
    case ID_terminateByDistance:                return "bool";
    case ID_termDistance:                       return "double";
    case ID_terminateByTime:                    return "bool";
    case ID_termTime:                           return "double";
    case ID_maxStepLength:                      return "double";
    case ID_limitMaximumTimestep:               return "bool";
    case ID_maxTimeStep:                        return "double";
    case ID_relTol:                             return "double";
    case ID_absTolSizeType:                     return "enum";
    case ID_absTolAbsolute:                     return "double";
    case ID_absTolBBox:                         return "double";
    case ID_fieldType:                          return "enum";
    case ID_fieldConstant:                      return "double";
    case ID_velocitySource:                     return "doubleArray";
    case ID_integrationType:                    return "enum";
    case ID_parallelizationAlgorithmType:       return "enum";
    case ID_maxProcessCount:                    return "int";
    case ID_maxDomainCacheSize:                 return "int";
    case ID_workGroupSize:                      return "int";
    case ID_pathlines:                          return "bool";
    case ID_pathlinesOverrideStartingTimeFlag:  return "bool";
    case ID_pathlinesOverrideStartingTime:      return "double";
    case ID_pathlinesPeriod:                    return "double";
    case ID_pathlinesCMFE:                      return "enum";
    case ID_displayGeometry:                    return "enum";
    case ID_cropBeginFlag:                      return "bool";
    case ID_cropBegin:                          return "double";
    case ID_cropEndFlag:                        return "bool";
    case ID_cropEnd:                            return "double";
    case ID_cropValue:                          return "enum";
    case ID_sampleDistance0:                    return "double";
    case ID_sampleDistance1:                    return "double";
    case ID_sampleDistance2:                    return "double";
    case ID_fillInterior:                       return "bool";
    case ID_randomSamples:                      return "bool";
    case ID_randomSeed:                         return "int";
    case ID_numberOfRandomSamples:              return "int";
    case ID_forceNodeCenteredData:              return "bool";
    case ID_issueTerminationWarnings:           return "bool";
    case ID_issueStiffnessWarnings:             return "bool";
    case ID_issueCriticalPointsWarnings:        return "bool";
    case ID_criticalPointThreshold:             return "double";
    case ID_correlationDistanceAngTol:          return "double";
    case ID_correlationDistanceMinDistAbsolute: return "double";
    case ID_correlationDistanceMinDistBBox:     return "double";
    case ID_correlationDistanceMinDistType:     return "enum";
    case ID_selection:                          return "string";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: IntegralCurveAttributes::FieldsEqual
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
IntegralCurveAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const IntegralCurveAttributes &obj = *((const IntegralCurveAttributes*)rhs);
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
    case ID_dataValue:
        {  // new scope
        retval = (dataValue == obj.dataValue);
        }
        break;
    case ID_dataVariable:
        {  // new scope
        retval = (dataVariable == obj.dataVariable);
        }
        break;
    case ID_integrationDirection:
        {  // new scope
        retval = (integrationDirection == obj.integrationDirection);
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
    case ID_fieldType:
        {  // new scope
        retval = (fieldType == obj.fieldType);
        }
        break;
    case ID_fieldConstant:
        {  // new scope
        retval = (fieldConstant == obj.fieldConstant);
        }
        break;
    case ID_velocitySource:
        {  // new scope
        // Compare the velocitySource arrays.
        bool velocitySource_equal = true;
        for(int i = 0; i < 3 && velocitySource_equal; ++i)
            velocitySource_equal = (velocitySource[i] == obj.velocitySource[i]);

        retval = velocitySource_equal;
        }
        break;
    case ID_integrationType:
        {  // new scope
        retval = (integrationType == obj.integrationType);
        }
        break;
    case ID_parallelizationAlgorithmType:
        {  // new scope
        retval = (parallelizationAlgorithmType == obj.parallelizationAlgorithmType);
        }
        break;
    case ID_maxProcessCount:
        {  // new scope
        retval = (maxProcessCount == obj.maxProcessCount);
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
    case ID_pathlinesPeriod:
        {  // new scope
        retval = (pathlinesPeriod == obj.pathlinesPeriod);
        }
        break;
    case ID_pathlinesCMFE:
        {  // new scope
        retval = (pathlinesCMFE == obj.pathlinesCMFE);
        }
        break;
    case ID_displayGeometry:
        {  // new scope
        retval = (displayGeometry == obj.displayGeometry);
        }
        break;
    case ID_cropBeginFlag:
        {  // new scope
        retval = (cropBeginFlag == obj.cropBeginFlag);
        }
        break;
    case ID_cropBegin:
        {  // new scope
        retval = (cropBegin == obj.cropBegin);
        }
        break;
    case ID_cropEndFlag:
        {  // new scope
        retval = (cropEndFlag == obj.cropEndFlag);
        }
        break;
    case ID_cropEnd:
        {  // new scope
        retval = (cropEnd == obj.cropEnd);
        }
        break;
    case ID_cropValue:
        {  // new scope
        retval = (cropValue == obj.cropValue);
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
    case ID_correlationDistanceAngTol:
        {  // new scope
        retval = (correlationDistanceAngTol == obj.correlationDistanceAngTol);
        }
        break;
    case ID_correlationDistanceMinDistAbsolute:
        {  // new scope
        retval = (correlationDistanceMinDistAbsolute == obj.correlationDistanceMinDistAbsolute);
        }
        break;
    case ID_correlationDistanceMinDistBBox:
        {  // new scope
        retval = (correlationDistanceMinDistBBox == obj.correlationDistanceMinDistBBox);
        }
        break;
    case ID_correlationDistanceMinDistType:
        {  // new scope
        retval = (correlationDistanceMinDistType == obj.correlationDistanceMinDistType);
        }
        break;
    case ID_selection:
        {  // new scope
        retval = (selection == obj.selection);
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
//  Method: IntegralCurveAttributes::ChangesRequireRecalculation
//
//  Purpose:
//     Determines whether or not the curves must be recalculated based on the
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
//   Add custom renderer and lots of appearance options.
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
//   Dave Pugmire, Fri Jan 28 14:49:50 EST 2011
//   Add vary tube radius by variable.
//
//   Dave Pugmire, Thu Mar 15 11:23:18 EDT 2012
//   Add named selections as a seed source.
//
// ****************************************************************************

#define PDIF(p1,p2,i) ((p1)[i] != (p2)[i])
#define POINT_DIFFERS(p1,p2) (PDIF(p1,p2,0) || PDIF(p1,p2,1) || PDIF(p1,p2,2))

bool
IntegralCurveAttributes::ChangesRequireRecalculation(const IntegralCurveAttributes &obj) const
{
    //Check the general stuff first...
    if (sourceType != obj.sourceType ||
        maxSteps != obj.maxSteps ||
        terminateByDistance != obj.terminateByDistance ||
        termDistance != obj.termDistance ||
        terminateByTime != obj.terminateByTime ||
        termTime != obj.termTime ||
        integrationDirection != obj.integrationDirection ||
        fieldType != obj.fieldType ||
        fieldConstant != obj.fieldConstant ||
        integrationType != obj.integrationType ||
        maxStepLength != obj.maxStepLength ||
        maxTimeStep != obj.maxTimeStep ||
        limitMaximumTimestep != obj.limitMaximumTimestep ||
        relTol != obj.relTol ||
        absTolAbsolute != obj.absTolAbsolute ||
        absTolBBox != obj.absTolBBox ||
        absTolSizeType != obj.absTolSizeType ||
        forceNodeCenteredData != obj.forceNodeCenteredData ||
        cropBeginFlag != obj.cropBeginFlag ||
        cropBegin != obj.cropBegin ||
        cropEndFlag != obj.cropEndFlag ||
        cropEnd != obj.cropEnd ||
        cropValue != obj.cropValue ||
        pathlines != obj.pathlines ||
        pathlinesOverrideStartingTimeFlag != obj.pathlinesOverrideStartingTimeFlag ||
        pathlinesOverrideStartingTime != obj.pathlinesOverrideStartingTime ||
        pathlinesCMFE != obj.pathlinesCMFE ||
        dataVariable != obj.dataVariable ||
        (dataValue != obj.dataValue && obj.dataValue != Solid) ||
        ((dataValue == CorrelationDistance) &&
         (correlationDistanceAngTol != obj.correlationDistanceAngTol ||
          correlationDistanceMinDistAbsolute != obj.correlationDistanceMinDistAbsolute ||
          correlationDistanceMinDistBBox != obj.correlationDistanceMinDistBBox ||
          correlationDistanceMinDistType != obj.correlationDistanceMinDistType)) ||
        0 )
    {

        return true;
    }
         
    // If they say they don't want warnings, then don't re-execute.  If they say they do,
    // then we better re-execute so we can give them that warning.
    if (issueTerminationWarnings != obj.issueTerminationWarnings &&
        obj.issueTerminationWarnings == true)
        return true;

    if (fieldType == FlashField &&
        POINT_DIFFERS(velocitySource, obj.velocitySource))
    {
        return true;
    }

    //Check by source type.
    if ((sourceType == Point) && POINT_DIFFERS(pointSource, obj.pointSource))
    {
        return true;
    }

    if (sourceType == Line_)
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

    if (sourceType == Plane)
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

    if (sourceType == Circle)
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

    if (sourceType == Sphere)
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

    if (sourceType == Box)
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
    
    if (sourceType == PointList)
    {
        if (pointList.size() != obj.pointList.size())
            return true;
        else
            for (size_t i = 0 ; i < pointList.size() ; i++)
                if (pointList[i] != obj.pointList[i])
                    return true;
    }
    if (sourceType == Selection)
    {
        if (selection != obj.selection ||
            randomSamples != obj.randomSamples ||
            (!randomSamples && (sampleDensity0 != obj.sampleDensity0)) ||
            (randomSamples && (randomSeed != obj.randomSeed ||
                               numberOfRandomSamples != obj.numberOfRandomSamples)))
        {
            return true;
        }
    }

    return false;
}

