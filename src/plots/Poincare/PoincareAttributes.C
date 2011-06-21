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

#include <PoincareAttributes.h>
#include <DataNode.h>
#include <PlaneAttributes.h>
#include <PointAttributes.h>
#include <Line.h>

//
// Enum conversion methods for PoincareAttributes::SourceType
//

static const char *SourceType_strings[] = {
"SpecifiedPoint", "SpecifiedLine"};

std::string
PoincareAttributes::SourceType_ToString(PoincareAttributes::SourceType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SourceType_strings[index];
}

std::string
PoincareAttributes::SourceType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SourceType_strings[index];
}

bool
PoincareAttributes::SourceType_FromString(const std::string &s, PoincareAttributes::SourceType &val)
{
    val = PoincareAttributes::SpecifiedPoint;
    for(int i = 0; i < 2; ++i)
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
// Enum conversion methods for PoincareAttributes::FieldType
//

static const char *FieldType_strings[] = {
"Default", "M3DC12DField", "M3DC13DField", 
"NIMRODField", "FlashField"};

std::string
PoincareAttributes::FieldType_ToString(PoincareAttributes::FieldType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return FieldType_strings[index];
}

std::string
PoincareAttributes::FieldType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return FieldType_strings[index];
}

bool
PoincareAttributes::FieldType_FromString(const std::string &s, PoincareAttributes::FieldType &val)
{
    val = PoincareAttributes::Default;
    for(int i = 0; i < 5; ++i)
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
// Enum conversion methods for PoincareAttributes::IntegrationType
//

static const char *IntegrationType_strings[] = {
"Euler", "DormandPrince", "AdamsBashforth", 
"Reserved_3", "Reserved_4", "M3DC12DIntegrator"
};

std::string
PoincareAttributes::IntegrationType_ToString(PoincareAttributes::IntegrationType t)
{
    int index = int(t);
    if(index < 0 || index >= 6) index = 0;
    return IntegrationType_strings[index];
}

std::string
PoincareAttributes::IntegrationType_ToString(int t)
{
    int index = (t < 0 || t >= 6) ? 0 : t;
    return IntegrationType_strings[index];
}

bool
PoincareAttributes::IntegrationType_FromString(const std::string &s, PoincareAttributes::IntegrationType &val)
{
    val = PoincareAttributes::Euler;
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
// Enum conversion methods for PoincareAttributes::SizeType
//

static const char *SizeType_strings[] = {
"Absolute", "FractionOfBBox"};

std::string
PoincareAttributes::SizeType_ToString(PoincareAttributes::SizeType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SizeType_strings[index];
}

std::string
PoincareAttributes::SizeType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SizeType_strings[index];
}

bool
PoincareAttributes::SizeType_FromString(const std::string &s, PoincareAttributes::SizeType &val)
{
    val = PoincareAttributes::Absolute;
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
// Enum conversion methods for PoincareAttributes::CoordinateSystem
//

static const char *CoordinateSystem_strings[] = {
"Cartesian", "Cylindrical"};

std::string
PoincareAttributes::CoordinateSystem_ToString(PoincareAttributes::CoordinateSystem t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return CoordinateSystem_strings[index];
}

std::string
PoincareAttributes::CoordinateSystem_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return CoordinateSystem_strings[index];
}

bool
PoincareAttributes::CoordinateSystem_FromString(const std::string &s, PoincareAttributes::CoordinateSystem &val)
{
    val = PoincareAttributes::Cartesian;
    for(int i = 0; i < 2; ++i)
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
// Enum conversion methods for PoincareAttributes::OverlapType
//

static const char *OverlapType_strings[] = {
"Raw", "Remove", "Merge", 
"Smooth"};

std::string
PoincareAttributes::OverlapType_ToString(PoincareAttributes::OverlapType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return OverlapType_strings[index];
}

std::string
PoincareAttributes::OverlapType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return OverlapType_strings[index];
}

bool
PoincareAttributes::OverlapType_FromString(const std::string &s, PoincareAttributes::OverlapType &val)
{
    val = PoincareAttributes::Raw;
    for(int i = 0; i < 4; ++i)
    {
        if(s == OverlapType_strings[i])
        {
            val = (OverlapType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::ShowMeshType
//

static const char *ShowMeshType_strings[] = {
"Curves", "Surfaces"};

std::string
PoincareAttributes::ShowMeshType_ToString(PoincareAttributes::ShowMeshType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ShowMeshType_strings[index];
}

std::string
PoincareAttributes::ShowMeshType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ShowMeshType_strings[index];
}

bool
PoincareAttributes::ShowMeshType_FromString(const std::string &s, PoincareAttributes::ShowMeshType &val)
{
    val = PoincareAttributes::Curves;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ShowMeshType_strings[i])
        {
            val = (ShowMeshType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::PuncturePlaneType
//

static const char *PuncturePlaneType_strings[] = {
"Poloidal", "Toroidal", "Arbitrary"
};

std::string
PoincareAttributes::PuncturePlaneType_ToString(PoincareAttributes::PuncturePlaneType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return PuncturePlaneType_strings[index];
}

std::string
PoincareAttributes::PuncturePlaneType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return PuncturePlaneType_strings[index];
}

bool
PoincareAttributes::PuncturePlaneType_FromString(const std::string &s, PoincareAttributes::PuncturePlaneType &val)
{
    val = PoincareAttributes::Poloidal;
    for(int i = 0; i < 3; ++i)
    {
        if(s == PuncturePlaneType_strings[i])
        {
            val = (PuncturePlaneType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::AnalysisType
//

static const char *AnalysisType_strings[] = {
"None", "Normal"};

std::string
PoincareAttributes::AnalysisType_ToString(PoincareAttributes::AnalysisType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return AnalysisType_strings[index];
}

std::string
PoincareAttributes::AnalysisType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return AnalysisType_strings[index];
}

bool
PoincareAttributes::AnalysisType_FromString(const std::string &s, PoincareAttributes::AnalysisType &val)
{
    val = PoincareAttributes::None;
    for(int i = 0; i < 2; ++i)
    {
        if(s == AnalysisType_strings[i])
        {
            val = (AnalysisType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::ColoringMethod
//

static const char *ColoringMethod_strings[] = {
"ColorBySingleColor", "ColorByColorTable"};

std::string
PoincareAttributes::ColoringMethod_ToString(PoincareAttributes::ColoringMethod t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ColoringMethod_strings[index];
}

std::string
PoincareAttributes::ColoringMethod_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ColoringMethod_strings[index];
}

bool
PoincareAttributes::ColoringMethod_FromString(const std::string &s, PoincareAttributes::ColoringMethod &val)
{
    val = PoincareAttributes::ColorBySingleColor;
    for(int i = 0; i < 2; ++i)
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
// Enum conversion methods for PoincareAttributes::Opacity
//

static const char *Opacity_strings[] = {
"Explicit", "ColorTable"};

std::string
PoincareAttributes::Opacity_ToString(PoincareAttributes::Opacity t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return Opacity_strings[index];
}

std::string
PoincareAttributes::Opacity_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return Opacity_strings[index];
}

bool
PoincareAttributes::Opacity_FromString(const std::string &s, PoincareAttributes::Opacity &val)
{
    val = PoincareAttributes::Explicit;
    for(int i = 0; i < 2; ++i)
    {
        if(s == Opacity_strings[i])
        {
            val = (Opacity)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PoincareAttributes::DataValue
//

static const char *DataValue_strings[] = {
"Solid", "SafetyFactorQ", "SafetyFactorP", 
"SafetyFactorQ_NotP", "SafetyFactorP_NotQ", "ToroidalWindings", 
"PoloidalWindingsQ", "PoloidalWindingsP", "FieldlineOrder", 
"PointOrder", "PlaneOrder", "WindingGroupOrder", 
"WindingPointOrder", "WindingPointOrderModulo"};

std::string
PoincareAttributes::DataValue_ToString(PoincareAttributes::DataValue t)
{
    int index = int(t);
    if(index < 0 || index >= 14) index = 0;
    return DataValue_strings[index];
}

std::string
PoincareAttributes::DataValue_ToString(int t)
{
    int index = (t < 0 || t >= 14) ? 0 : t;
    return DataValue_strings[index];
}

bool
PoincareAttributes::DataValue_FromString(const std::string &s, PoincareAttributes::DataValue &val)
{
    val = PoincareAttributes::Solid;
    for(int i = 0; i < 14; ++i)
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
// Enum conversion methods for PoincareAttributes::StreamlineAlgorithmType
//

static const char *StreamlineAlgorithmType_strings[] = {
"LoadOnDemand", "ParallelStaticDomains", "MasterSlave"
};

std::string
PoincareAttributes::StreamlineAlgorithmType_ToString(PoincareAttributes::StreamlineAlgorithmType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return StreamlineAlgorithmType_strings[index];
}

std::string
PoincareAttributes::StreamlineAlgorithmType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return StreamlineAlgorithmType_strings[index];
}

bool
PoincareAttributes::StreamlineAlgorithmType_FromString(const std::string &s, PoincareAttributes::StreamlineAlgorithmType &val)
{
    val = PoincareAttributes::LoadOnDemand;
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
// Enum conversion methods for PoincareAttributes::PointType
//

static const char *PointType_strings[] = {
"Box", "Axis", "Icosahedron", 
"Point", "Sphere"};

std::string
PoincareAttributes::PointType_ToString(PoincareAttributes::PointType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return PointType_strings[index];
}

std::string
PoincareAttributes::PointType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return PointType_strings[index];
}

bool
PoincareAttributes::PointType_FromString(const std::string &s, PoincareAttributes::PointType &val)
{
    val = PoincareAttributes::Box;
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
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Init utility for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PoincareAttributes::Init()
{
    opacityType = Explicit;
    opacity = 1;
    minPunctures = 10;
    maxPunctures = 100;
    puncturePlane = Poloidal;
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
    pointDensity = 1;
    fieldType = Default;
    fieldConstant = 1;
    integrationType = AdamsBashforth;
    coordinateSystem = Cartesian;
    maxStepLength = 0.1;
    limitMaximumTimestep = false;
    maxTimeStep = 0.1;
    relTol = 0.0001;
    absTolSizeType = FractionOfBBox;
    absTolAbsolute = 1e-05;
    absTolBBox = 1e-06;
    analysis = Normal;
    maximumToroidalWinding = 0;
    overrideToroidalWinding = 0;
    overridePoloidalWinding = 0;
    windingPairConfidence = 0.9;
    rationalTemplateSeedParm = 0.9;
    adjustPlane = -1;
    overlaps = Remove;
    meshType = Curves;
    numberPlanes = 1;
    singlePlane = 0;
    min = 0;
    max = 0;
    minFlag = false;
    maxFlag = false;
    colorType = ColorByColorTable;
    dataValue = SafetyFactorQ;
    showOPoints = false;
    OPointMaxIterations = 2;
    showXPoints = false;
    XPointMaxIterations = 2;
    showChaotic = false;
    showIslands = false;
    verboseFlag = true;
    show1DPlots = false;
    showLines = true;
    lineWidth = 0;
    lineStyle = 0;
    showPoints = false;
    pointSize = 1;
    pointSizePixels = 1;
    pointType = Point;
    legendFlag = true;
    lightingFlag = true;
    streamlineAlgorithmType = LoadOnDemand;
    maxStreamlineProcessCount = 10;
    maxDomainCacheSize = 3;
    workGroupSize = 32;
    forceNodeCenteredData = false;

    PoincareAttributes::SelectAll();
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Copy utility for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PoincareAttributes::Copy(const PoincareAttributes &obj)
{
    opacityType = obj.opacityType;
    opacity = obj.opacity;
    minPunctures = obj.minPunctures;
    maxPunctures = obj.maxPunctures;
    puncturePlane = obj.puncturePlane;
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

    pointDensity = obj.pointDensity;
    fieldType = obj.fieldType;
    fieldConstant = obj.fieldConstant;
    integrationType = obj.integrationType;
    coordinateSystem = obj.coordinateSystem;
    maxStepLength = obj.maxStepLength;
    limitMaximumTimestep = obj.limitMaximumTimestep;
    maxTimeStep = obj.maxTimeStep;
    relTol = obj.relTol;
    absTolSizeType = obj.absTolSizeType;
    absTolAbsolute = obj.absTolAbsolute;
    absTolBBox = obj.absTolBBox;
    analysis = obj.analysis;
    maximumToroidalWinding = obj.maximumToroidalWinding;
    overrideToroidalWinding = obj.overrideToroidalWinding;
    overridePoloidalWinding = obj.overridePoloidalWinding;
    windingPairConfidence = obj.windingPairConfidence;
    rationalTemplateSeedParm = obj.rationalTemplateSeedParm;
    adjustPlane = obj.adjustPlane;
    overlaps = obj.overlaps;
    meshType = obj.meshType;
    numberPlanes = obj.numberPlanes;
    singlePlane = obj.singlePlane;
    min = obj.min;
    max = obj.max;
    minFlag = obj.minFlag;
    maxFlag = obj.maxFlag;
    colorType = obj.colorType;
    singleColor = obj.singleColor;
    colorTableName = obj.colorTableName;
    dataValue = obj.dataValue;
    showOPoints = obj.showOPoints;
    OPointMaxIterations = obj.OPointMaxIterations;
    showXPoints = obj.showXPoints;
    XPointMaxIterations = obj.XPointMaxIterations;
    showChaotic = obj.showChaotic;
    showIslands = obj.showIslands;
    verboseFlag = obj.verboseFlag;
    show1DPlots = obj.show1DPlots;
    showLines = obj.showLines;
    lineWidth = obj.lineWidth;
    lineStyle = obj.lineStyle;
    showPoints = obj.showPoints;
    pointSize = obj.pointSize;
    pointSizePixels = obj.pointSizePixels;
    pointType = obj.pointType;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    streamlineAlgorithmType = obj.streamlineAlgorithmType;
    maxStreamlineProcessCount = obj.maxStreamlineProcessCount;
    maxDomainCacheSize = obj.maxDomainCacheSize;
    workGroupSize = obj.workGroupSize;
    forceNodeCenteredData = obj.forceNodeCenteredData;

    PoincareAttributes::SelectAll();
}

// Type map format string
const char *PoincareAttributes::TypeMapFormatString = POINCAREATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t PoincareAttributes::TmfsStruct = {POINCAREATTRIBUTES_TMFS};


// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Default constructor for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes() : 
    AttributeSubject(PoincareAttributes::TypeMapFormatString),
    singleColor(0, 0, 0), colorTableName("Default")
{
    PoincareAttributes::Init();
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Constructor for the derived classes of PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs),
    singleColor(0, 0, 0), colorTableName("Default")
{
    PoincareAttributes::Init();
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Copy constructor for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes(const PoincareAttributes &obj) : 
    AttributeSubject(PoincareAttributes::TypeMapFormatString)
{
    PoincareAttributes::Copy(obj);
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::PoincareAttributes(const PoincareAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    PoincareAttributes::Copy(obj);
}

// ****************************************************************************
// Method: PoincareAttributes::~PoincareAttributes
//
// Purpose: 
//   Destructor for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes::~PoincareAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: PoincareAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the PoincareAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PoincareAttributes& 
PoincareAttributes::operator = (const PoincareAttributes &obj)
{
    if (this == &obj) return *this;

    PoincareAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: PoincareAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the PoincareAttributes class.
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
PoincareAttributes::operator == (const PoincareAttributes &obj) const
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

    // Create the return value
    return ((opacityType == obj.opacityType) &&
            (opacity == obj.opacity) &&
            (minPunctures == obj.minPunctures) &&
            (maxPunctures == obj.maxPunctures) &&
            (puncturePlane == obj.puncturePlane) &&
            (sourceType == obj.sourceType) &&
            pointSource_equal &&
            lineStart_equal &&
            lineEnd_equal &&
            (pointDensity == obj.pointDensity) &&
            (fieldType == obj.fieldType) &&
            (fieldConstant == obj.fieldConstant) &&
            (integrationType == obj.integrationType) &&
            (coordinateSystem == obj.coordinateSystem) &&
            (maxStepLength == obj.maxStepLength) &&
            (limitMaximumTimestep == obj.limitMaximumTimestep) &&
            (maxTimeStep == obj.maxTimeStep) &&
            (relTol == obj.relTol) &&
            (absTolSizeType == obj.absTolSizeType) &&
            (absTolAbsolute == obj.absTolAbsolute) &&
            (absTolBBox == obj.absTolBBox) &&
            (analysis == obj.analysis) &&
            (maximumToroidalWinding == obj.maximumToroidalWinding) &&
            (overrideToroidalWinding == obj.overrideToroidalWinding) &&
            (overridePoloidalWinding == obj.overridePoloidalWinding) &&
            (windingPairConfidence == obj.windingPairConfidence) &&
            (rationalTemplateSeedParm == obj.rationalTemplateSeedParm) &&
            (adjustPlane == obj.adjustPlane) &&
            (overlaps == obj.overlaps) &&
            (meshType == obj.meshType) &&
            (numberPlanes == obj.numberPlanes) &&
            (singlePlane == obj.singlePlane) &&
            (min == obj.min) &&
            (max == obj.max) &&
            (minFlag == obj.minFlag) &&
            (maxFlag == obj.maxFlag) &&
            (colorType == obj.colorType) &&
            (singleColor == obj.singleColor) &&
            (colorTableName == obj.colorTableName) &&
            (dataValue == obj.dataValue) &&
            (showOPoints == obj.showOPoints) &&
            (OPointMaxIterations == obj.OPointMaxIterations) &&
            (showXPoints == obj.showXPoints) &&
            (XPointMaxIterations == obj.XPointMaxIterations) &&
            (showChaotic == obj.showChaotic) &&
            (showIslands == obj.showIslands) &&
            (verboseFlag == obj.verboseFlag) &&
            (show1DPlots == obj.show1DPlots) &&
            (showLines == obj.showLines) &&
            (lineWidth == obj.lineWidth) &&
            (lineStyle == obj.lineStyle) &&
            (showPoints == obj.showPoints) &&
            (pointSize == obj.pointSize) &&
            (pointSizePixels == obj.pointSizePixels) &&
            (pointType == obj.pointType) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (streamlineAlgorithmType == obj.streamlineAlgorithmType) &&
            (maxStreamlineProcessCount == obj.maxStreamlineProcessCount) &&
            (maxDomainCacheSize == obj.maxDomainCacheSize) &&
            (workGroupSize == obj.workGroupSize) &&
            (forceNodeCenteredData == obj.forceNodeCenteredData));
}

// ****************************************************************************
// Method: PoincareAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the PoincareAttributes class.
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
PoincareAttributes::operator != (const PoincareAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: PoincareAttributes::TypeName
//
// Purpose: 
//   Type name method for the PoincareAttributes class.
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
PoincareAttributes::TypeName() const
{
    return "PoincareAttributes";
}

// ****************************************************************************
// Method: PoincareAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the PoincareAttributes class.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//    Jeremy Meredith, Wed Apr  8 16:48:05 EDT 2009
//    Initial steps to unification with streamline attributes.
// ****************************************************************************

bool
PoincareAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;
    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const PoincareAttributes *tmp = (const PoincareAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    else if(atts->TypeName() == "PointAttributes")
    {
        const PointAttributes *p = (PointAttributes *)atts;
        SetPointSource(p->GetPoint());
        retval = true;
    }
    else if(atts->TypeName() == "Line")
    {
        const Line *line = (const Line *)atts;
        SetLineStart(line->GetPoint1());
        SetLineEnd(line->GetPoint2());
        retval = true;
    }
    return retval;
}

// ****************************************************************************
// Method: PoincareAttributes::CreateCompatible
//
// Purpose: 
//   Creates a new state object of the desired type.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//    Jeremy Meredith, Wed Apr  8 16:48:05 EDT 2009
//    Initial steps to unification with streamline attributes.
// ****************************************************************************

AttributeSubject *
PoincareAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if (TypeName() == tname)
    {
        retval = new PoincareAttributes(*this);
    }
    else if (tname == "PointAttributes")
    {
        PointAttributes *p = new PointAttributes;
        p->SetPoint(GetPointSource());
        retval = p;
    }
    else if (tname == "Line")
    {
        Line *l = new Line;
        l->SetPoint1(GetLineStart());
        l->SetPoint2(GetLineEnd());
        retval = l;
    }
    return retval;
}

// ****************************************************************************
// Method: PoincareAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the PoincareAttributes class.
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
PoincareAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new PoincareAttributes(*this);
    else
        retval = new PoincareAttributes;

    return retval;
}

// ****************************************************************************
// Method: PoincareAttributes::SelectAll
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
PoincareAttributes::SelectAll()
{
    Select(ID_opacityType,               (void *)&opacityType);
    Select(ID_opacity,                   (void *)&opacity);
    Select(ID_minPunctures,              (void *)&minPunctures);
    Select(ID_maxPunctures,              (void *)&maxPunctures);
    Select(ID_puncturePlane,             (void *)&puncturePlane);
    Select(ID_sourceType,                (void *)&sourceType);
    Select(ID_pointSource,               (void *)pointSource, 3);
    Select(ID_lineStart,                 (void *)lineStart, 3);
    Select(ID_lineEnd,                   (void *)lineEnd, 3);
    Select(ID_pointDensity,              (void *)&pointDensity);
    Select(ID_fieldType,                 (void *)&fieldType);
    Select(ID_fieldConstant,             (void *)&fieldConstant);
    Select(ID_integrationType,           (void *)&integrationType);
    Select(ID_coordinateSystem,          (void *)&coordinateSystem);
    Select(ID_maxStepLength,             (void *)&maxStepLength);
    Select(ID_limitMaximumTimestep,      (void *)&limitMaximumTimestep);
    Select(ID_maxTimeStep,               (void *)&maxTimeStep);
    Select(ID_relTol,                    (void *)&relTol);
    Select(ID_absTolSizeType,            (void *)&absTolSizeType);
    Select(ID_absTolAbsolute,            (void *)&absTolAbsolute);
    Select(ID_absTolBBox,                (void *)&absTolBBox);
    Select(ID_analysis,                  (void *)&analysis);
    Select(ID_maximumToroidalWinding,    (void *)&maximumToroidalWinding);
    Select(ID_overrideToroidalWinding,   (void *)&overrideToroidalWinding);
    Select(ID_overridePoloidalWinding,   (void *)&overridePoloidalWinding);
    Select(ID_windingPairConfidence,     (void *)&windingPairConfidence);
    Select(ID_rationalTemplateSeedParm,  (void *)&rationalTemplateSeedParm);
    Select(ID_adjustPlane,               (void *)&adjustPlane);
    Select(ID_overlaps,                  (void *)&overlaps);
    Select(ID_meshType,                  (void *)&meshType);
    Select(ID_numberPlanes,              (void *)&numberPlanes);
    Select(ID_singlePlane,               (void *)&singlePlane);
    Select(ID_min,                       (void *)&min);
    Select(ID_max,                       (void *)&max);
    Select(ID_minFlag,                   (void *)&minFlag);
    Select(ID_maxFlag,                   (void *)&maxFlag);
    Select(ID_colorType,                 (void *)&colorType);
    Select(ID_singleColor,               (void *)&singleColor);
    Select(ID_colorTableName,            (void *)&colorTableName);
    Select(ID_dataValue,                 (void *)&dataValue);
    Select(ID_showOPoints,               (void *)&showOPoints);
    Select(ID_OPointMaxIterations,       (void *)&OPointMaxIterations);
    Select(ID_showXPoints,               (void *)&showXPoints);
    Select(ID_XPointMaxIterations,       (void *)&XPointMaxIterations);
    Select(ID_showChaotic,               (void *)&showChaotic);
    Select(ID_showIslands,               (void *)&showIslands);
    Select(ID_verboseFlag,               (void *)&verboseFlag);
    Select(ID_show1DPlots,               (void *)&show1DPlots);
    Select(ID_showLines,                 (void *)&showLines);
    Select(ID_lineWidth,                 (void *)&lineWidth);
    Select(ID_lineStyle,                 (void *)&lineStyle);
    Select(ID_showPoints,                (void *)&showPoints);
    Select(ID_pointSize,                 (void *)&pointSize);
    Select(ID_pointSizePixels,           (void *)&pointSizePixels);
    Select(ID_pointType,                 (void *)&pointType);
    Select(ID_legendFlag,                (void *)&legendFlag);
    Select(ID_lightingFlag,              (void *)&lightingFlag);
    Select(ID_streamlineAlgorithmType,   (void *)&streamlineAlgorithmType);
    Select(ID_maxStreamlineProcessCount, (void *)&maxStreamlineProcessCount);
    Select(ID_maxDomainCacheSize,        (void *)&maxDomainCacheSize);
    Select(ID_workGroupSize,             (void *)&workGroupSize);
    Select(ID_forceNodeCenteredData,     (void *)&forceNodeCenteredData);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PoincareAttributes::CreateNode
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
PoincareAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    PoincareAttributes defaultObject;
    bool addToParent = false;
    // Create a node for PoincareAttributes.
    DataNode *node = new DataNode("PoincareAttributes");

    if(completeSave || !FieldsEqual(ID_opacityType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityType", Opacity_ToString(opacityType)));
    }

    if(completeSave || !FieldsEqual(ID_opacity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacity", opacity));
    }

    if(completeSave || !FieldsEqual(ID_minPunctures, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minPunctures", minPunctures));
    }

    if(completeSave || !FieldsEqual(ID_maxPunctures, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxPunctures", maxPunctures));
    }

    if(completeSave || !FieldsEqual(ID_puncturePlane, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("puncturePlane", PuncturePlaneType_ToString(puncturePlane)));
    }

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

    if(completeSave || !FieldsEqual(ID_pointDensity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointDensity", pointDensity));
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

    if(completeSave || !FieldsEqual(ID_integrationType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("integrationType", IntegrationType_ToString(integrationType)));
    }

    if(completeSave || !FieldsEqual(ID_coordinateSystem, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("coordinateSystem", CoordinateSystem_ToString(coordinateSystem)));
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

    if(completeSave || !FieldsEqual(ID_analysis, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("analysis", AnalysisType_ToString(analysis)));
    }

    if(completeSave || !FieldsEqual(ID_maximumToroidalWinding, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maximumToroidalWinding", maximumToroidalWinding));
    }

    if(completeSave || !FieldsEqual(ID_overrideToroidalWinding, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("overrideToroidalWinding", overrideToroidalWinding));
    }

    if(completeSave || !FieldsEqual(ID_overridePoloidalWinding, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("overridePoloidalWinding", overridePoloidalWinding));
    }

    if(completeSave || !FieldsEqual(ID_windingPairConfidence, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("windingPairConfidence", windingPairConfidence));
    }

    if(completeSave || !FieldsEqual(ID_rationalTemplateSeedParm, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rationalTemplateSeedParm", rationalTemplateSeedParm));
    }

    if(completeSave || !FieldsEqual(ID_adjustPlane, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("adjustPlane", adjustPlane));
    }

    if(completeSave || !FieldsEqual(ID_overlaps, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("overlaps", OverlapType_ToString(overlaps)));
    }

    if(completeSave || !FieldsEqual(ID_meshType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("meshType", ShowMeshType_ToString(meshType)));
    }

    if(completeSave || !FieldsEqual(ID_numberPlanes, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numberPlanes", numberPlanes));
    }

    if(completeSave || !FieldsEqual(ID_singlePlane, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("singlePlane", singlePlane));
    }

    if(completeSave || !FieldsEqual(ID_min, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("min", min));
    }

    if(completeSave || !FieldsEqual(ID_max, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("max", max));
    }

    if(completeSave || !FieldsEqual(ID_minFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minFlag", minFlag));
    }

    if(completeSave || !FieldsEqual(ID_maxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxFlag", maxFlag));
    }

    if(completeSave || !FieldsEqual(ID_colorType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorType", ColoringMethod_ToString(colorType)));
    }

        DataNode *singleColorNode = new DataNode("singleColor");
        if(singleColor.CreateNode(singleColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(singleColorNode);
        }
        else
            delete singleColorNode;
    if(completeSave || !FieldsEqual(ID_colorTableName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

    if(completeSave || !FieldsEqual(ID_dataValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataValue", DataValue_ToString(dataValue)));
    }

    if(completeSave || !FieldsEqual(ID_showOPoints, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showOPoints", showOPoints));
    }

    if(completeSave || !FieldsEqual(ID_OPointMaxIterations, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OPointMaxIterations", OPointMaxIterations));
    }

    if(completeSave || !FieldsEqual(ID_showXPoints, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showXPoints", showXPoints));
    }

    if(completeSave || !FieldsEqual(ID_XPointMaxIterations, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("XPointMaxIterations", XPointMaxIterations));
    }

    if(completeSave || !FieldsEqual(ID_showChaotic, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showChaotic", showChaotic));
    }

    if(completeSave || !FieldsEqual(ID_showIslands, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showIslands", showIslands));
    }

    if(completeSave || !FieldsEqual(ID_verboseFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("verboseFlag", verboseFlag));
    }

    if(completeSave || !FieldsEqual(ID_show1DPlots, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("show1DPlots", show1DPlots));
    }

    if(completeSave || !FieldsEqual(ID_showLines, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showLines", showLines));
    }

    if(completeSave || !FieldsEqual(ID_lineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineWidth", lineWidth));
    }

    if(completeSave || !FieldsEqual(ID_lineStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineStyle", lineStyle));
    }

    if(completeSave || !FieldsEqual(ID_showPoints, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("showPoints", showPoints));
    }

    if(completeSave || !FieldsEqual(ID_pointSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSize", pointSize));
    }

    if(completeSave || !FieldsEqual(ID_pointSizePixels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizePixels", pointSizePixels));
    }

    if(completeSave || !FieldsEqual(ID_pointType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointType", PointType_ToString(pointType)));
    }

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

    if(completeSave || !FieldsEqual(ID_forceNodeCenteredData, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("forceNodeCenteredData", forceNodeCenteredData));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: PoincareAttributes::SetFromNode
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
PoincareAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("PoincareAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("opacityType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetOpacityType(Opacity(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Opacity value;
            if(Opacity_FromString(node->AsString(), value))
                SetOpacityType(value);
        }
    }
    if((node = searchNode->GetNode("opacity")) != 0)
        SetOpacity(node->AsDouble());
    if((node = searchNode->GetNode("minPunctures")) != 0)
        SetMinPunctures(node->AsInt());
    if((node = searchNode->GetNode("maxPunctures")) != 0)
        SetMaxPunctures(node->AsInt());
    if((node = searchNode->GetNode("puncturePlane")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetPuncturePlane(PuncturePlaneType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            PuncturePlaneType value;
            if(PuncturePlaneType_FromString(node->AsString(), value))
                SetPuncturePlane(value);
        }
    }
    if((node = searchNode->GetNode("sourceType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
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
    if((node = searchNode->GetNode("pointDensity")) != 0)
        SetPointDensity(node->AsInt());
    if((node = searchNode->GetNode("fieldType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
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
    if((node = searchNode->GetNode("coordinateSystem")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetCoordinateSystem(CoordinateSystem(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CoordinateSystem value;
            if(CoordinateSystem_FromString(node->AsString(), value))
                SetCoordinateSystem(value);
        }
    }
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
    if((node = searchNode->GetNode("analysis")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetAnalysis(AnalysisType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            AnalysisType value;
            if(AnalysisType_FromString(node->AsString(), value))
                SetAnalysis(value);
        }
    }
    if((node = searchNode->GetNode("maximumToroidalWinding")) != 0)
        SetMaximumToroidalWinding(node->AsInt());
    if((node = searchNode->GetNode("overrideToroidalWinding")) != 0)
        SetOverrideToroidalWinding(node->AsInt());
    if((node = searchNode->GetNode("overridePoloidalWinding")) != 0)
        SetOverridePoloidalWinding(node->AsInt());
    if((node = searchNode->GetNode("windingPairConfidence")) != 0)
        SetWindingPairConfidence(node->AsDouble());
    if((node = searchNode->GetNode("rationalTemplateSeedParm")) != 0)
        SetRationalTemplateSeedParm(node->AsDouble());
    if((node = searchNode->GetNode("adjustPlane")) != 0)
        SetAdjustPlane(node->AsInt());
    if((node = searchNode->GetNode("overlaps")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetOverlaps(OverlapType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OverlapType value;
            if(OverlapType_FromString(node->AsString(), value))
                SetOverlaps(value);
        }
    }
    if((node = searchNode->GetNode("meshType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetMeshType(ShowMeshType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ShowMeshType value;
            if(ShowMeshType_FromString(node->AsString(), value))
                SetMeshType(value);
        }
    }
    if((node = searchNode->GetNode("numberPlanes")) != 0)
        SetNumberPlanes(node->AsInt());
    if((node = searchNode->GetNode("singlePlane")) != 0)
        SetSinglePlane(node->AsDouble());
    if((node = searchNode->GetNode("min")) != 0)
        SetMin(node->AsDouble());
    if((node = searchNode->GetNode("max")) != 0)
        SetMax(node->AsDouble());
    if((node = searchNode->GetNode("minFlag")) != 0)
        SetMinFlag(node->AsBool());
    if((node = searchNode->GetNode("maxFlag")) != 0)
        SetMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("colorType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetColorType(ColoringMethod(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ColoringMethod value;
            if(ColoringMethod_FromString(node->AsString(), value))
                SetColorType(value);
        }
    }
    if((node = searchNode->GetNode("singleColor")) != 0)
        singleColor.SetFromNode(node);
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("dataValue")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 14)
                SetDataValue(DataValue(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DataValue value;
            if(DataValue_FromString(node->AsString(), value))
                SetDataValue(value);
        }
    }
    if((node = searchNode->GetNode("showOPoints")) != 0)
        SetShowOPoints(node->AsBool());
    if((node = searchNode->GetNode("OPointMaxIterations")) != 0)
        SetOPointMaxIterations(node->AsInt());
    if((node = searchNode->GetNode("showXPoints")) != 0)
        SetShowXPoints(node->AsBool());
    if((node = searchNode->GetNode("XPointMaxIterations")) != 0)
        SetXPointMaxIterations(node->AsInt());
    if((node = searchNode->GetNode("showChaotic")) != 0)
        SetShowChaotic(node->AsBool());
    if((node = searchNode->GetNode("showIslands")) != 0)
        SetShowIslands(node->AsBool());
    if((node = searchNode->GetNode("verboseFlag")) != 0)
        SetVerboseFlag(node->AsBool());
    if((node = searchNode->GetNode("show1DPlots")) != 0)
        SetShow1DPlots(node->AsBool());
    if((node = searchNode->GetNode("showLines")) != 0)
        SetShowLines(node->AsBool());
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("lineStyle")) != 0)
        SetLineStyle(node->AsInt());
    if((node = searchNode->GetNode("showPoints")) != 0)
        SetShowPoints(node->AsBool());
    if((node = searchNode->GetNode("pointSize")) != 0)
        SetPointSize(node->AsDouble());
    if((node = searchNode->GetNode("pointSizePixels")) != 0)
        SetPointSizePixels(node->AsInt());
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
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
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
    if((node = searchNode->GetNode("forceNodeCenteredData")) != 0)
        SetForceNodeCenteredData(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
PoincareAttributes::SetOpacityType(PoincareAttributes::Opacity opacityType_)
{
    opacityType = opacityType_;
    Select(ID_opacityType, (void *)&opacityType);
}

void
PoincareAttributes::SetOpacity(double opacity_)
{
    opacity = opacity_;
    Select(ID_opacity, (void *)&opacity);
}

void
PoincareAttributes::SetMinPunctures(int minPunctures_)
{
    minPunctures = minPunctures_;
    Select(ID_minPunctures, (void *)&minPunctures);
}

void
PoincareAttributes::SetMaxPunctures(int maxPunctures_)
{
    maxPunctures = maxPunctures_;
    Select(ID_maxPunctures, (void *)&maxPunctures);
}

void
PoincareAttributes::SetPuncturePlane(PoincareAttributes::PuncturePlaneType puncturePlane_)
{
    puncturePlane = puncturePlane_;
    Select(ID_puncturePlane, (void *)&puncturePlane);
}

void
PoincareAttributes::SetSourceType(PoincareAttributes::SourceType sourceType_)
{
    sourceType = sourceType_;
    Select(ID_sourceType, (void *)&sourceType);
}

void
PoincareAttributes::SetPointSource(const double *pointSource_)
{
    pointSource[0] = pointSource_[0];
    pointSource[1] = pointSource_[1];
    pointSource[2] = pointSource_[2];
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
PoincareAttributes::SetLineStart(const double *lineStart_)
{
    lineStart[0] = lineStart_[0];
    lineStart[1] = lineStart_[1];
    lineStart[2] = lineStart_[2];
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
PoincareAttributes::SetLineEnd(const double *lineEnd_)
{
    lineEnd[0] = lineEnd_[0];
    lineEnd[1] = lineEnd_[1];
    lineEnd[2] = lineEnd_[2];
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
PoincareAttributes::SetPointDensity(int pointDensity_)
{
    pointDensity = pointDensity_;
    Select(ID_pointDensity, (void *)&pointDensity);
}

void
PoincareAttributes::SetFieldType(PoincareAttributes::FieldType fieldType_)
{
    fieldType = fieldType_;
    Select(ID_fieldType, (void *)&fieldType);
}

void
PoincareAttributes::SetFieldConstant(double fieldConstant_)
{
    fieldConstant = fieldConstant_;
    Select(ID_fieldConstant, (void *)&fieldConstant);
}

void
PoincareAttributes::SetIntegrationType(PoincareAttributes::IntegrationType integrationType_)
{
    integrationType = integrationType_;
    Select(ID_integrationType, (void *)&integrationType);
}

void
PoincareAttributes::SetCoordinateSystem(PoincareAttributes::CoordinateSystem coordinateSystem_)
{
    coordinateSystem = coordinateSystem_;
    Select(ID_coordinateSystem, (void *)&coordinateSystem);
}

void
PoincareAttributes::SetMaxStepLength(double maxStepLength_)
{
    maxStepLength = maxStepLength_;
    Select(ID_maxStepLength, (void *)&maxStepLength);
}

void
PoincareAttributes::SetLimitMaximumTimestep(bool limitMaximumTimestep_)
{
    limitMaximumTimestep = limitMaximumTimestep_;
    Select(ID_limitMaximumTimestep, (void *)&limitMaximumTimestep);
}

void
PoincareAttributes::SetMaxTimeStep(double maxTimeStep_)
{
    maxTimeStep = maxTimeStep_;
    Select(ID_maxTimeStep, (void *)&maxTimeStep);
}

void
PoincareAttributes::SetRelTol(double relTol_)
{
    relTol = relTol_;
    Select(ID_relTol, (void *)&relTol);
}

void
PoincareAttributes::SetAbsTolSizeType(PoincareAttributes::SizeType absTolSizeType_)
{
    absTolSizeType = absTolSizeType_;
    Select(ID_absTolSizeType, (void *)&absTolSizeType);
}

void
PoincareAttributes::SetAbsTolAbsolute(double absTolAbsolute_)
{
    absTolAbsolute = absTolAbsolute_;
    Select(ID_absTolAbsolute, (void *)&absTolAbsolute);
}

void
PoincareAttributes::SetAbsTolBBox(double absTolBBox_)
{
    absTolBBox = absTolBBox_;
    Select(ID_absTolBBox, (void *)&absTolBBox);
}

void
PoincareAttributes::SetAnalysis(PoincareAttributes::AnalysisType analysis_)
{
    analysis = analysis_;
    Select(ID_analysis, (void *)&analysis);
}

void
PoincareAttributes::SetMaximumToroidalWinding(int maximumToroidalWinding_)
{
    maximumToroidalWinding = maximumToroidalWinding_;
    Select(ID_maximumToroidalWinding, (void *)&maximumToroidalWinding);
}

void
PoincareAttributes::SetOverrideToroidalWinding(int overrideToroidalWinding_)
{
    overrideToroidalWinding = overrideToroidalWinding_;
    Select(ID_overrideToroidalWinding, (void *)&overrideToroidalWinding);
}

void
PoincareAttributes::SetOverridePoloidalWinding(int overridePoloidalWinding_)
{
    overridePoloidalWinding = overridePoloidalWinding_;
    Select(ID_overridePoloidalWinding, (void *)&overridePoloidalWinding);
}

void
PoincareAttributes::SetWindingPairConfidence(double windingPairConfidence_)
{
    windingPairConfidence = windingPairConfidence_;
    Select(ID_windingPairConfidence, (void *)&windingPairConfidence);
}

void
PoincareAttributes::SetRationalTemplateSeedParm(double rationalTemplateSeedParm_)
{
    rationalTemplateSeedParm = rationalTemplateSeedParm_;
    Select(ID_rationalTemplateSeedParm, (void *)&rationalTemplateSeedParm);
}

void
PoincareAttributes::SetAdjustPlane(int adjustPlane_)
{
    adjustPlane = adjustPlane_;
    Select(ID_adjustPlane, (void *)&adjustPlane);
}

void
PoincareAttributes::SetOverlaps(PoincareAttributes::OverlapType overlaps_)
{
    overlaps = overlaps_;
    Select(ID_overlaps, (void *)&overlaps);
}

void
PoincareAttributes::SetMeshType(PoincareAttributes::ShowMeshType meshType_)
{
    meshType = meshType_;
    Select(ID_meshType, (void *)&meshType);
}

void
PoincareAttributes::SetNumberPlanes(int numberPlanes_)
{
    numberPlanes = numberPlanes_;
    Select(ID_numberPlanes, (void *)&numberPlanes);
}

void
PoincareAttributes::SetSinglePlane(double singlePlane_)
{
    singlePlane = singlePlane_;
    Select(ID_singlePlane, (void *)&singlePlane);
}

void
PoincareAttributes::SetMin(double min_)
{
    min = min_;
    Select(ID_min, (void *)&min);
}

void
PoincareAttributes::SetMax(double max_)
{
    max = max_;
    Select(ID_max, (void *)&max);
}

void
PoincareAttributes::SetMinFlag(bool minFlag_)
{
    minFlag = minFlag_;
    Select(ID_minFlag, (void *)&minFlag);
}

void
PoincareAttributes::SetMaxFlag(bool maxFlag_)
{
    maxFlag = maxFlag_;
    Select(ID_maxFlag, (void *)&maxFlag);
}

void
PoincareAttributes::SetColorType(PoincareAttributes::ColoringMethod colorType_)
{
    colorType = colorType_;
    Select(ID_colorType, (void *)&colorType);
}

void
PoincareAttributes::SetSingleColor(const ColorAttribute &singleColor_)
{
    singleColor = singleColor_;
    Select(ID_singleColor, (void *)&singleColor);
}

void
PoincareAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PoincareAttributes::SetDataValue(PoincareAttributes::DataValue dataValue_)
{
    dataValue = dataValue_;
    Select(ID_dataValue, (void *)&dataValue);
}

void
PoincareAttributes::SetShowOPoints(bool showOPoints_)
{
    showOPoints = showOPoints_;
    Select(ID_showOPoints, (void *)&showOPoints);
}

void
PoincareAttributes::SetOPointMaxIterations(int OPointMaxIterations_)
{
    OPointMaxIterations = OPointMaxIterations_;
    Select(ID_OPointMaxIterations, (void *)&OPointMaxIterations);
}

void
PoincareAttributes::SetShowXPoints(bool showXPoints_)
{
    showXPoints = showXPoints_;
    Select(ID_showXPoints, (void *)&showXPoints);
}

void
PoincareAttributes::SetXPointMaxIterations(int XPointMaxIterations_)
{
    XPointMaxIterations = XPointMaxIterations_;
    Select(ID_XPointMaxIterations, (void *)&XPointMaxIterations);
}

void
PoincareAttributes::SetShowChaotic(bool showChaotic_)
{
    showChaotic = showChaotic_;
    Select(ID_showChaotic, (void *)&showChaotic);
}

void
PoincareAttributes::SetShowIslands(bool showIslands_)
{
    showIslands = showIslands_;
    Select(ID_showIslands, (void *)&showIslands);
}

void
PoincareAttributes::SetVerboseFlag(bool verboseFlag_)
{
    verboseFlag = verboseFlag_;
    Select(ID_verboseFlag, (void *)&verboseFlag);
}

void
PoincareAttributes::SetShow1DPlots(bool show1DPlots_)
{
    show1DPlots = show1DPlots_;
    Select(ID_show1DPlots, (void *)&show1DPlots);
}

void
PoincareAttributes::SetShowLines(bool showLines_)
{
    showLines = showLines_;
    Select(ID_showLines, (void *)&showLines);
}

void
PoincareAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
PoincareAttributes::SetLineStyle(int lineStyle_)
{
    lineStyle = lineStyle_;
    Select(ID_lineStyle, (void *)&lineStyle);
}

void
PoincareAttributes::SetShowPoints(bool showPoints_)
{
    showPoints = showPoints_;
    Select(ID_showPoints, (void *)&showPoints);
}

void
PoincareAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(ID_pointSize, (void *)&pointSize);
}

void
PoincareAttributes::SetPointSizePixels(int pointSizePixels_)
{
    pointSizePixels = pointSizePixels_;
    Select(ID_pointSizePixels, (void *)&pointSizePixels);
}

void
PoincareAttributes::SetPointType(PoincareAttributes::PointType pointType_)
{
    pointType = pointType_;
    Select(ID_pointType, (void *)&pointType);
}

void
PoincareAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
PoincareAttributes::SetLightingFlag(bool lightingFlag_)
{
    lightingFlag = lightingFlag_;
    Select(ID_lightingFlag, (void *)&lightingFlag);
}

void
PoincareAttributes::SetStreamlineAlgorithmType(PoincareAttributes::StreamlineAlgorithmType streamlineAlgorithmType_)
{
    streamlineAlgorithmType = streamlineAlgorithmType_;
    Select(ID_streamlineAlgorithmType, (void *)&streamlineAlgorithmType);
}

void
PoincareAttributes::SetMaxStreamlineProcessCount(int maxStreamlineProcessCount_)
{
    maxStreamlineProcessCount = maxStreamlineProcessCount_;
    Select(ID_maxStreamlineProcessCount, (void *)&maxStreamlineProcessCount);
}

void
PoincareAttributes::SetMaxDomainCacheSize(int maxDomainCacheSize_)
{
    maxDomainCacheSize = maxDomainCacheSize_;
    Select(ID_maxDomainCacheSize, (void *)&maxDomainCacheSize);
}

void
PoincareAttributes::SetWorkGroupSize(int workGroupSize_)
{
    workGroupSize = workGroupSize_;
    Select(ID_workGroupSize, (void *)&workGroupSize);
}

void
PoincareAttributes::SetForceNodeCenteredData(bool forceNodeCenteredData_)
{
    forceNodeCenteredData = forceNodeCenteredData_;
    Select(ID_forceNodeCenteredData, (void *)&forceNodeCenteredData);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

PoincareAttributes::Opacity
PoincareAttributes::GetOpacityType() const
{
    return Opacity(opacityType);
}

double
PoincareAttributes::GetOpacity() const
{
    return opacity;
}

int
PoincareAttributes::GetMinPunctures() const
{
    return minPunctures;
}

int
PoincareAttributes::GetMaxPunctures() const
{
    return maxPunctures;
}

PoincareAttributes::PuncturePlaneType
PoincareAttributes::GetPuncturePlane() const
{
    return PuncturePlaneType(puncturePlane);
}

PoincareAttributes::SourceType
PoincareAttributes::GetSourceType() const
{
    return SourceType(sourceType);
}

const double *
PoincareAttributes::GetPointSource() const
{
    return pointSource;
}

double *
PoincareAttributes::GetPointSource()
{
    return pointSource;
}

const double *
PoincareAttributes::GetLineStart() const
{
    return lineStart;
}

double *
PoincareAttributes::GetLineStart()
{
    return lineStart;
}

const double *
PoincareAttributes::GetLineEnd() const
{
    return lineEnd;
}

double *
PoincareAttributes::GetLineEnd()
{
    return lineEnd;
}

int
PoincareAttributes::GetPointDensity() const
{
    return pointDensity;
}

PoincareAttributes::FieldType
PoincareAttributes::GetFieldType() const
{
    return FieldType(fieldType);
}

double
PoincareAttributes::GetFieldConstant() const
{
    return fieldConstant;
}

PoincareAttributes::IntegrationType
PoincareAttributes::GetIntegrationType() const
{
    return IntegrationType(integrationType);
}

PoincareAttributes::CoordinateSystem
PoincareAttributes::GetCoordinateSystem() const
{
    return CoordinateSystem(coordinateSystem);
}

double
PoincareAttributes::GetMaxStepLength() const
{
    return maxStepLength;
}

bool
PoincareAttributes::GetLimitMaximumTimestep() const
{
    return limitMaximumTimestep;
}

double
PoincareAttributes::GetMaxTimeStep() const
{
    return maxTimeStep;
}

double
PoincareAttributes::GetRelTol() const
{
    return relTol;
}

PoincareAttributes::SizeType
PoincareAttributes::GetAbsTolSizeType() const
{
    return SizeType(absTolSizeType);
}

double
PoincareAttributes::GetAbsTolAbsolute() const
{
    return absTolAbsolute;
}

double
PoincareAttributes::GetAbsTolBBox() const
{
    return absTolBBox;
}

PoincareAttributes::AnalysisType
PoincareAttributes::GetAnalysis() const
{
    return AnalysisType(analysis);
}

int
PoincareAttributes::GetMaximumToroidalWinding() const
{
    return maximumToroidalWinding;
}

int
PoincareAttributes::GetOverrideToroidalWinding() const
{
    return overrideToroidalWinding;
}

int
PoincareAttributes::GetOverridePoloidalWinding() const
{
    return overridePoloidalWinding;
}

double
PoincareAttributes::GetWindingPairConfidence() const
{
    return windingPairConfidence;
}

double
PoincareAttributes::GetRationalTemplateSeedParm() const
{
    return rationalTemplateSeedParm;
}

int
PoincareAttributes::GetAdjustPlane() const
{
    return adjustPlane;
}

PoincareAttributes::OverlapType
PoincareAttributes::GetOverlaps() const
{
    return OverlapType(overlaps);
}

PoincareAttributes::ShowMeshType
PoincareAttributes::GetMeshType() const
{
    return ShowMeshType(meshType);
}

int
PoincareAttributes::GetNumberPlanes() const
{
    return numberPlanes;
}

double
PoincareAttributes::GetSinglePlane() const
{
    return singlePlane;
}

double
PoincareAttributes::GetMin() const
{
    return min;
}

double
PoincareAttributes::GetMax() const
{
    return max;
}

bool
PoincareAttributes::GetMinFlag() const
{
    return minFlag;
}

bool
PoincareAttributes::GetMaxFlag() const
{
    return maxFlag;
}

PoincareAttributes::ColoringMethod
PoincareAttributes::GetColorType() const
{
    return ColoringMethod(colorType);
}

const ColorAttribute &
PoincareAttributes::GetSingleColor() const
{
    return singleColor;
}

ColorAttribute &
PoincareAttributes::GetSingleColor()
{
    return singleColor;
}

const std::string &
PoincareAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
PoincareAttributes::GetColorTableName()
{
    return colorTableName;
}

PoincareAttributes::DataValue
PoincareAttributes::GetDataValue() const
{
    return DataValue(dataValue);
}

bool
PoincareAttributes::GetShowOPoints() const
{
    return showOPoints;
}

int
PoincareAttributes::GetOPointMaxIterations() const
{
    return OPointMaxIterations;
}

bool
PoincareAttributes::GetShowXPoints() const
{
    return showXPoints;
}

int
PoincareAttributes::GetXPointMaxIterations() const
{
    return XPointMaxIterations;
}

bool
PoincareAttributes::GetShowChaotic() const
{
    return showChaotic;
}

bool
PoincareAttributes::GetShowIslands() const
{
    return showIslands;
}

bool
PoincareAttributes::GetVerboseFlag() const
{
    return verboseFlag;
}

bool
PoincareAttributes::GetShow1DPlots() const
{
    return show1DPlots;
}

bool
PoincareAttributes::GetShowLines() const
{
    return showLines;
}

int
PoincareAttributes::GetLineWidth() const
{
    return lineWidth;
}

int
PoincareAttributes::GetLineStyle() const
{
    return lineStyle;
}

bool
PoincareAttributes::GetShowPoints() const
{
    return showPoints;
}

double
PoincareAttributes::GetPointSize() const
{
    return pointSize;
}

int
PoincareAttributes::GetPointSizePixels() const
{
    return pointSizePixels;
}

PoincareAttributes::PointType
PoincareAttributes::GetPointType() const
{
    return PointType(pointType);
}

bool
PoincareAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
PoincareAttributes::GetLightingFlag() const
{
    return lightingFlag;
}

PoincareAttributes::StreamlineAlgorithmType
PoincareAttributes::GetStreamlineAlgorithmType() const
{
    return StreamlineAlgorithmType(streamlineAlgorithmType);
}

int
PoincareAttributes::GetMaxStreamlineProcessCount() const
{
    return maxStreamlineProcessCount;
}

int
PoincareAttributes::GetMaxDomainCacheSize() const
{
    return maxDomainCacheSize;
}

int
PoincareAttributes::GetWorkGroupSize() const
{
    return workGroupSize;
}

bool
PoincareAttributes::GetForceNodeCenteredData() const
{
    return forceNodeCenteredData;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
PoincareAttributes::SelectPointSource()
{
    Select(ID_pointSource, (void *)pointSource, 3);
}

void
PoincareAttributes::SelectLineStart()
{
    Select(ID_lineStart, (void *)lineStart, 3);
}

void
PoincareAttributes::SelectLineEnd()
{
    Select(ID_lineEnd, (void *)lineEnd, 3);
}

void
PoincareAttributes::SelectSingleColor()
{
    Select(ID_singleColor, (void *)&singleColor);
}

void
PoincareAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PoincareAttributes::GetFieldName
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
PoincareAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_opacityType:               return "opacityType";
    case ID_opacity:                   return "opacity";
    case ID_minPunctures:              return "minPunctures";
    case ID_maxPunctures:              return "maxPunctures";
    case ID_puncturePlane:             return "puncturePlane";
    case ID_sourceType:                return "sourceType";
    case ID_pointSource:               return "pointSource";
    case ID_lineStart:                 return "lineStart";
    case ID_lineEnd:                   return "lineEnd";
    case ID_pointDensity:              return "pointDensity";
    case ID_fieldType:                 return "fieldType";
    case ID_fieldConstant:             return "fieldConstant";
    case ID_integrationType:           return "integrationType";
    case ID_coordinateSystem:          return "coordinateSystem";
    case ID_maxStepLength:             return "maxStepLength";
    case ID_limitMaximumTimestep:      return "limitMaximumTimestep";
    case ID_maxTimeStep:               return "maxTimeStep";
    case ID_relTol:                    return "relTol";
    case ID_absTolSizeType:            return "absTolSizeType";
    case ID_absTolAbsolute:            return "absTolAbsolute";
    case ID_absTolBBox:                return "absTolBBox";
    case ID_analysis:                  return "analysis";
    case ID_maximumToroidalWinding:    return "maximumToroidalWinding";
    case ID_overrideToroidalWinding:   return "overrideToroidalWinding";
    case ID_overridePoloidalWinding:   return "overridePoloidalWinding";
    case ID_windingPairConfidence:     return "windingPairConfidence";
    case ID_rationalTemplateSeedParm:  return "rationalTemplateSeedParm";
    case ID_adjustPlane:               return "adjustPlane";
    case ID_overlaps:                  return "overlaps";
    case ID_meshType:                  return "meshType";
    case ID_numberPlanes:              return "numberPlanes";
    case ID_singlePlane:               return "singlePlane";
    case ID_min:                       return "min";
    case ID_max:                       return "max";
    case ID_minFlag:                   return "minFlag";
    case ID_maxFlag:                   return "maxFlag";
    case ID_colorType:                 return "colorType";
    case ID_singleColor:               return "singleColor";
    case ID_colorTableName:            return "colorTableName";
    case ID_dataValue:                 return "dataValue";
    case ID_showOPoints:               return "showOPoints";
    case ID_OPointMaxIterations:       return "OPointMaxIterations";
    case ID_showXPoints:               return "showXPoints";
    case ID_XPointMaxIterations:       return "XPointMaxIterations";
    case ID_showChaotic:               return "showChaotic";
    case ID_showIslands:               return "showIslands";
    case ID_verboseFlag:               return "verboseFlag";
    case ID_show1DPlots:               return "show1DPlots";
    case ID_showLines:                 return "showLines";
    case ID_lineWidth:                 return "lineWidth";
    case ID_lineStyle:                 return "lineStyle";
    case ID_showPoints:                return "showPoints";
    case ID_pointSize:                 return "pointSize";
    case ID_pointSizePixels:           return "pointSizePixels";
    case ID_pointType:                 return "pointType";
    case ID_legendFlag:                return "legendFlag";
    case ID_lightingFlag:              return "lightingFlag";
    case ID_streamlineAlgorithmType:   return "streamlineAlgorithmType";
    case ID_maxStreamlineProcessCount: return "maxStreamlineProcessCount";
    case ID_maxDomainCacheSize:        return "maxDomainCacheSize";
    case ID_workGroupSize:             return "workGroupSize";
    case ID_forceNodeCenteredData:     return "forceNodeCenteredData";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PoincareAttributes::GetFieldType
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
PoincareAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_opacityType:               return FieldType_enum;
    case ID_opacity:                   return FieldType_opacity;
    case ID_minPunctures:              return FieldType_int;
    case ID_maxPunctures:              return FieldType_int;
    case ID_puncturePlane:             return FieldType_enum;
    case ID_sourceType:                return FieldType_enum;
    case ID_pointSource:               return FieldType_doubleArray;
    case ID_lineStart:                 return FieldType_doubleArray;
    case ID_lineEnd:                   return FieldType_doubleArray;
    case ID_pointDensity:              return FieldType_int;
    case ID_fieldType:                 return FieldType_enum;
    case ID_fieldConstant:             return FieldType_double;
    case ID_integrationType:           return FieldType_enum;
    case ID_coordinateSystem:          return FieldType_enum;
    case ID_maxStepLength:             return FieldType_double;
    case ID_limitMaximumTimestep:      return FieldType_bool;
    case ID_maxTimeStep:               return FieldType_double;
    case ID_relTol:                    return FieldType_double;
    case ID_absTolSizeType:            return FieldType_enum;
    case ID_absTolAbsolute:            return FieldType_double;
    case ID_absTolBBox:                return FieldType_double;
    case ID_analysis:                  return FieldType_enum;
    case ID_maximumToroidalWinding:    return FieldType_int;
    case ID_overrideToroidalWinding:   return FieldType_int;
    case ID_overridePoloidalWinding:   return FieldType_int;
    case ID_windingPairConfidence:     return FieldType_double;
    case ID_rationalTemplateSeedParm:  return FieldType_double;
    case ID_adjustPlane:               return FieldType_int;
    case ID_overlaps:                  return FieldType_enum;
    case ID_meshType:                  return FieldType_enum;
    case ID_numberPlanes:              return FieldType_int;
    case ID_singlePlane:               return FieldType_double;
    case ID_min:                       return FieldType_double;
    case ID_max:                       return FieldType_double;
    case ID_minFlag:                   return FieldType_bool;
    case ID_maxFlag:                   return FieldType_bool;
    case ID_colorType:                 return FieldType_enum;
    case ID_singleColor:               return FieldType_color;
    case ID_colorTableName:            return FieldType_colortable;
    case ID_dataValue:                 return FieldType_enum;
    case ID_showOPoints:               return FieldType_bool;
    case ID_OPointMaxIterations:       return FieldType_int;
    case ID_showXPoints:               return FieldType_bool;
    case ID_XPointMaxIterations:       return FieldType_int;
    case ID_showChaotic:               return FieldType_bool;
    case ID_showIslands:               return FieldType_bool;
    case ID_verboseFlag:               return FieldType_bool;
    case ID_show1DPlots:               return FieldType_bool;
    case ID_showLines:                 return FieldType_bool;
    case ID_lineWidth:                 return FieldType_linewidth;
    case ID_lineStyle:                 return FieldType_linestyle;
    case ID_showPoints:                return FieldType_bool;
    case ID_pointSize:                 return FieldType_double;
    case ID_pointSizePixels:           return FieldType_int;
    case ID_pointType:                 return FieldType_enum;
    case ID_legendFlag:                return FieldType_bool;
    case ID_lightingFlag:              return FieldType_bool;
    case ID_streamlineAlgorithmType:   return FieldType_enum;
    case ID_maxStreamlineProcessCount: return FieldType_int;
    case ID_maxDomainCacheSize:        return FieldType_int;
    case ID_workGroupSize:             return FieldType_int;
    case ID_forceNodeCenteredData:     return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: PoincareAttributes::GetFieldTypeName
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
PoincareAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_opacityType:               return "enum";
    case ID_opacity:                   return "opacity";
    case ID_minPunctures:              return "int";
    case ID_maxPunctures:              return "int";
    case ID_puncturePlane:             return "enum";
    case ID_sourceType:                return "enum";
    case ID_pointSource:               return "doubleArray";
    case ID_lineStart:                 return "doubleArray";
    case ID_lineEnd:                   return "doubleArray";
    case ID_pointDensity:              return "int";
    case ID_fieldType:                 return "enum";
    case ID_fieldConstant:             return "double";
    case ID_integrationType:           return "enum";
    case ID_coordinateSystem:          return "enum";
    case ID_maxStepLength:             return "double";
    case ID_limitMaximumTimestep:      return "bool";
    case ID_maxTimeStep:               return "double";
    case ID_relTol:                    return "double";
    case ID_absTolSizeType:            return "enum";
    case ID_absTolAbsolute:            return "double";
    case ID_absTolBBox:                return "double";
    case ID_analysis:                  return "enum";
    case ID_maximumToroidalWinding:    return "int";
    case ID_overrideToroidalWinding:   return "int";
    case ID_overridePoloidalWinding:   return "int";
    case ID_windingPairConfidence:     return "double";
    case ID_rationalTemplateSeedParm:  return "double";
    case ID_adjustPlane:               return "int";
    case ID_overlaps:                  return "enum";
    case ID_meshType:                  return "enum";
    case ID_numberPlanes:              return "int";
    case ID_singlePlane:               return "double";
    case ID_min:                       return "double";
    case ID_max:                       return "double";
    case ID_minFlag:                   return "bool";
    case ID_maxFlag:                   return "bool";
    case ID_colorType:                 return "enum";
    case ID_singleColor:               return "color";
    case ID_colorTableName:            return "colortable";
    case ID_dataValue:                 return "enum";
    case ID_showOPoints:               return "bool";
    case ID_OPointMaxIterations:       return "int";
    case ID_showXPoints:               return "bool";
    case ID_XPointMaxIterations:       return "int";
    case ID_showChaotic:               return "bool";
    case ID_showIslands:               return "bool";
    case ID_verboseFlag:               return "bool";
    case ID_show1DPlots:               return "bool";
    case ID_showLines:                 return "bool";
    case ID_lineWidth:                 return "linewidth";
    case ID_lineStyle:                 return "linestyle";
    case ID_showPoints:                return "bool";
    case ID_pointSize:                 return "double";
    case ID_pointSizePixels:           return "int";
    case ID_pointType:                 return "enum";
    case ID_legendFlag:                return "bool";
    case ID_lightingFlag:              return "bool";
    case ID_streamlineAlgorithmType:   return "enum";
    case ID_maxStreamlineProcessCount: return "int";
    case ID_maxDomainCacheSize:        return "int";
    case ID_workGroupSize:             return "int";
    case ID_forceNodeCenteredData:     return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PoincareAttributes::FieldsEqual
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
PoincareAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const PoincareAttributes &obj = *((const PoincareAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_opacityType:
        {  // new scope
        retval = (opacityType == obj.opacityType);
        }
        break;
    case ID_opacity:
        {  // new scope
        retval = (opacity == obj.opacity);
        }
        break;
    case ID_minPunctures:
        {  // new scope
        retval = (minPunctures == obj.minPunctures);
        }
        break;
    case ID_maxPunctures:
        {  // new scope
        retval = (maxPunctures == obj.maxPunctures);
        }
        break;
    case ID_puncturePlane:
        {  // new scope
        retval = (puncturePlane == obj.puncturePlane);
        }
        break;
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
    case ID_pointDensity:
        {  // new scope
        retval = (pointDensity == obj.pointDensity);
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
    case ID_integrationType:
        {  // new scope
        retval = (integrationType == obj.integrationType);
        }
        break;
    case ID_coordinateSystem:
        {  // new scope
        retval = (coordinateSystem == obj.coordinateSystem);
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
    case ID_analysis:
        {  // new scope
        retval = (analysis == obj.analysis);
        }
        break;
    case ID_maximumToroidalWinding:
        {  // new scope
        retval = (maximumToroidalWinding == obj.maximumToroidalWinding);
        }
        break;
    case ID_overrideToroidalWinding:
        {  // new scope
        retval = (overrideToroidalWinding == obj.overrideToroidalWinding);
        }
        break;
    case ID_overridePoloidalWinding:
        {  // new scope
        retval = (overridePoloidalWinding == obj.overridePoloidalWinding);
        }
        break;
    case ID_windingPairConfidence:
        {  // new scope
        retval = (windingPairConfidence == obj.windingPairConfidence);
        }
        break;
    case ID_rationalTemplateSeedParm:
        {  // new scope
        retval = (rationalTemplateSeedParm == obj.rationalTemplateSeedParm);
        }
        break;
    case ID_adjustPlane:
        {  // new scope
        retval = (adjustPlane == obj.adjustPlane);
        }
        break;
    case ID_overlaps:
        {  // new scope
        retval = (overlaps == obj.overlaps);
        }
        break;
    case ID_meshType:
        {  // new scope
        retval = (meshType == obj.meshType);
        }
        break;
    case ID_numberPlanes:
        {  // new scope
        retval = (numberPlanes == obj.numberPlanes);
        }
        break;
    case ID_singlePlane:
        {  // new scope
        retval = (singlePlane == obj.singlePlane);
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
    case ID_colorType:
        {  // new scope
        retval = (colorType == obj.colorType);
        }
        break;
    case ID_singleColor:
        {  // new scope
        retval = (singleColor == obj.singleColor);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_dataValue:
        {  // new scope
        retval = (dataValue == obj.dataValue);
        }
        break;
    case ID_showOPoints:
        {  // new scope
        retval = (showOPoints == obj.showOPoints);
        }
        break;
    case ID_OPointMaxIterations:
        {  // new scope
        retval = (OPointMaxIterations == obj.OPointMaxIterations);
        }
        break;
    case ID_showXPoints:
        {  // new scope
        retval = (showXPoints == obj.showXPoints);
        }
        break;
    case ID_XPointMaxIterations:
        {  // new scope
        retval = (XPointMaxIterations == obj.XPointMaxIterations);
        }
        break;
    case ID_showChaotic:
        {  // new scope
        retval = (showChaotic == obj.showChaotic);
        }
        break;
    case ID_showIslands:
        {  // new scope
        retval = (showIslands == obj.showIslands);
        }
        break;
    case ID_verboseFlag:
        {  // new scope
        retval = (verboseFlag == obj.verboseFlag);
        }
        break;
    case ID_show1DPlots:
        {  // new scope
        retval = (show1DPlots == obj.show1DPlots);
        }
        break;
    case ID_showLines:
        {  // new scope
        retval = (showLines == obj.showLines);
        }
        break;
    case ID_lineWidth:
        {  // new scope
        retval = (lineWidth == obj.lineWidth);
        }
        break;
    case ID_lineStyle:
        {  // new scope
        retval = (lineStyle == obj.lineStyle);
        }
        break;
    case ID_showPoints:
        {  // new scope
        retval = (showPoints == obj.showPoints);
        }
        break;
    case ID_pointSize:
        {  // new scope
        retval = (pointSize == obj.pointSize);
        }
        break;
    case ID_pointSizePixels:
        {  // new scope
        retval = (pointSizePixels == obj.pointSizePixels);
        }
        break;
    case ID_pointType:
        {  // new scope
        retval = (pointType == obj.pointType);
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
    case ID_forceNodeCenteredData:
        {  // new scope
        retval = (forceNodeCenteredData == obj.forceNodeCenteredData);
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
// Method: PoincareAttributes::ChangesRequireRecalculation
//
// Purpose: 
//   Determine if attribute changes require recalculation.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//
// ****************************************************************************

bool
PoincareAttributes::ChangesRequireRecalculation(const PoincareAttributes &obj) const
{
    return StreamlineAttsRequireRecalculation(obj) ||
           PoincareAttsRequireRecalculation(obj);
}

// ****************************************************************************
// Method: PoincareAttributes::StreamlineAttsRequireRecalculation
//
// Purpose: 
//   Determine if streamline attribute changes require recalculation.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//
// ****************************************************************************

#define PDIF(p1,p2,i) ((p1)[i] != (p2)[i])
#define POINT_DIFFERS(p1,p2) (PDIF(p1,p2,0) || PDIF(p1,p2,1) || PDIF(p1,p2,2))

bool
PoincareAttributes::StreamlineAttsRequireRecalculation(const PoincareAttributes &obj) const
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
    
    // Other things need to be true before we start paying attention to
    // point density.
    bool densityMatters = ((sourceType == SpecifiedLine) &&
                           (pointDensity != obj.pointDensity));

    return (sourceType != obj.sourceType ||
            sourcePointsDiffer ||
            sourceLineDiffers ||
            densityMatters ||

            minPunctures != obj.minPunctures ||
            maxPunctures != obj.maxPunctures ||

            puncturePlane != obj.puncturePlane ||

            fieldType != obj.fieldType ||
            fieldConstant != obj.fieldConstant ||
            integrationType != obj.integrationType ||
            maxStepLength != obj.maxStepLength ||
            limitMaximumTimestep != obj.limitMaximumTimestep ||
            maxTimeStep != obj.maxTimeStep ||
            relTol != obj.relTol ||
            absTolSizeType != obj.absTolSizeType ||
            absTolAbsolute != obj.absTolAbsolute ||
            absTolBBox != obj.absTolBBox);
}

// ****************************************************************************
// Method: PoincareAttributes::PoincareAttsRequireRecalculation
//
// Purpose: 
//   Determine if poincare attribute changes require recalculation.
//
// Programmer: Dave Pugmire
// Creation:   Tues Oct 21 14:22:17 EDT 2008
//
// Modifications:
//
// ****************************************************************************

bool
PoincareAttributes::PoincareAttsRequireRecalculation(const PoincareAttributes &obj) const
{
    return coordinateSystem != obj.coordinateSystem ||
 
           analysis != obj.analysis ||

           maximumToroidalWinding != obj.maximumToroidalWinding ||
           overrideToroidalWinding != obj.overrideToroidalWinding ||
           overridePoloidalWinding != obj.overridePoloidalWinding ||
           windingPairConfidence != obj.windingPairConfidence ||
           rationalTemplateSeedParm != obj.rationalTemplateSeedParm ||

           showOPoints != obj.showOPoints ||
           OPointMaxIterations != obj.OPointMaxIterations ||

           showXPoints != obj.showXPoints ||
           XPointMaxIterations != obj.XPointMaxIterations ||

           overlaps != obj.overlaps ||

           showIslands != obj.showIslands ||
           show1DPlots != obj.show1DPlots ||
           showChaotic != obj.showChaotic ||
           verboseFlag != obj.verboseFlag ||
 
           dataValue != obj.dataValue ||

           meshType != obj.meshType ||
           numberPlanes != obj.numberPlanes ||
           singlePlane != obj.singlePlane ||

           adjustPlane != obj.adjustPlane ||

           showLines != obj.showLines ||
           showPoints != obj.showPoints;
}
