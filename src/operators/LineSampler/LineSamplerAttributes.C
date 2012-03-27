/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include <LineSamplerAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for LineSamplerAttributes::MeshGeometry
//

static const char *MeshGeometry_strings[] = {
"Cartesian", "Cylindrical", "Toroidal"
};

std::string
LineSamplerAttributes::MeshGeometry_ToString(LineSamplerAttributes::MeshGeometry t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return MeshGeometry_strings[index];
}

std::string
LineSamplerAttributes::MeshGeometry_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return MeshGeometry_strings[index];
}

bool
LineSamplerAttributes::MeshGeometry_FromString(const std::string &s, LineSamplerAttributes::MeshGeometry &val)
{
    val = LineSamplerAttributes::Cartesian;
    for(int i = 0; i < 3; ++i)
    {
        if(s == MeshGeometry_strings[i])
        {
            val = (MeshGeometry)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ArrayConfiguration
//

static const char *ArrayConfiguration_strings[] = {
"Geometry", "Manual"};

std::string
LineSamplerAttributes::ArrayConfiguration_ToString(LineSamplerAttributes::ArrayConfiguration t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ArrayConfiguration_strings[index];
}

std::string
LineSamplerAttributes::ArrayConfiguration_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ArrayConfiguration_strings[index];
}

bool
LineSamplerAttributes::ArrayConfiguration_FromString(const std::string &s, LineSamplerAttributes::ArrayConfiguration &val)
{
    val = LineSamplerAttributes::Geometry;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ArrayConfiguration_strings[i])
        {
            val = (ArrayConfiguration)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::Boundary
//

static const char *Boundary_strings[] = {
"Data", "Wall"};

std::string
LineSamplerAttributes::Boundary_ToString(LineSamplerAttributes::Boundary t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return Boundary_strings[index];
}

std::string
LineSamplerAttributes::Boundary_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return Boundary_strings[index];
}

bool
LineSamplerAttributes::Boundary_FromString(const std::string &s, LineSamplerAttributes::Boundary &val)
{
    val = LineSamplerAttributes::Data;
    for(int i = 0; i < 2; ++i)
    {
        if(s == Boundary_strings[i])
        {
            val = (Boundary)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ChannelProjection
//

static const char *ChannelProjection_strings[] = {
"Divergent", "Parallel", "Grid"
};

std::string
LineSamplerAttributes::ChannelProjection_ToString(LineSamplerAttributes::ChannelProjection t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ChannelProjection_strings[index];
}

std::string
LineSamplerAttributes::ChannelProjection_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ChannelProjection_strings[index];
}

bool
LineSamplerAttributes::ChannelProjection_FromString(const std::string &s, LineSamplerAttributes::ChannelProjection &val)
{
    val = LineSamplerAttributes::Divergent;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ChannelProjection_strings[i])
        {
            val = (ChannelProjection)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ChannelLayoutType
//

static const char *ChannelLayoutType_strings[] = {
"ChannelAbsolute", "ChannelRelative"};

std::string
LineSamplerAttributes::ChannelLayoutType_ToString(LineSamplerAttributes::ChannelLayoutType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ChannelLayoutType_strings[index];
}

std::string
LineSamplerAttributes::ChannelLayoutType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ChannelLayoutType_strings[index];
}

bool
LineSamplerAttributes::ChannelLayoutType_FromString(const std::string &s, LineSamplerAttributes::ChannelLayoutType &val)
{
    val = LineSamplerAttributes::ChannelAbsolute;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ChannelLayoutType_strings[i])
        {
            val = (ChannelLayoutType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ArrayAxis
//

static const char *ArrayAxis_strings[] = {
"R", "Z"};

std::string
LineSamplerAttributes::ArrayAxis_ToString(LineSamplerAttributes::ArrayAxis t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ArrayAxis_strings[index];
}

std::string
LineSamplerAttributes::ArrayAxis_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ArrayAxis_strings[index];
}

bool
LineSamplerAttributes::ArrayAxis_FromString(const std::string &s, LineSamplerAttributes::ArrayAxis &val)
{
    val = LineSamplerAttributes::R;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ArrayAxis_strings[i])
        {
            val = (ArrayAxis)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ViewGeometry
//

static const char *ViewGeometry_strings[] = {
"Points", "Lines", "Surfaces"
};

std::string
LineSamplerAttributes::ViewGeometry_ToString(LineSamplerAttributes::ViewGeometry t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ViewGeometry_strings[index];
}

std::string
LineSamplerAttributes::ViewGeometry_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ViewGeometry_strings[index];
}

bool
LineSamplerAttributes::ViewGeometry_FromString(const std::string &s, LineSamplerAttributes::ViewGeometry &val)
{
    val = LineSamplerAttributes::Points;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ViewGeometry_strings[i])
        {
            val = (ViewGeometry)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::DisplayTime
//

static const char *DisplayTime_strings[] = {
"Step", "Time", "Cycle"
};

std::string
LineSamplerAttributes::DisplayTime_ToString(LineSamplerAttributes::DisplayTime t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return DisplayTime_strings[index];
}

std::string
LineSamplerAttributes::DisplayTime_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return DisplayTime_strings[index];
}

bool
LineSamplerAttributes::DisplayTime_FromString(const std::string &s, LineSamplerAttributes::DisplayTime &val)
{
    val = LineSamplerAttributes::Step;
    for(int i = 0; i < 3; ++i)
    {
        if(s == DisplayTime_strings[i])
        {
            val = (DisplayTime)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ChannelGeometry
//

static const char *ChannelGeometry_strings[] = {
"Point", "Line", "Cylinder", 
"Cone"};

std::string
LineSamplerAttributes::ChannelGeometry_ToString(LineSamplerAttributes::ChannelGeometry t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return ChannelGeometry_strings[index];
}

std::string
LineSamplerAttributes::ChannelGeometry_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return ChannelGeometry_strings[index];
}

bool
LineSamplerAttributes::ChannelGeometry_FromString(const std::string &s, LineSamplerAttributes::ChannelGeometry &val)
{
    val = LineSamplerAttributes::Point;
    for(int i = 0; i < 4; ++i)
    {
        if(s == ChannelGeometry_strings[i])
        {
            val = (ChannelGeometry)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ViewDimension
//

static const char *ViewDimension_strings[] = {
"One", "Two", "Three"
};

std::string
LineSamplerAttributes::ViewDimension_ToString(LineSamplerAttributes::ViewDimension t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ViewDimension_strings[index];
}

std::string
LineSamplerAttributes::ViewDimension_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ViewDimension_strings[index];
}

bool
LineSamplerAttributes::ViewDimension_FromString(const std::string &s, LineSamplerAttributes::ViewDimension &val)
{
    val = LineSamplerAttributes::One;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ViewDimension_strings[i])
        {
            val = (ViewDimension)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ChannelProfile
//

static const char *ChannelProfile_strings[] = {
"TopHat", "Gaussian"};

std::string
LineSamplerAttributes::ChannelProfile_ToString(LineSamplerAttributes::ChannelProfile t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ChannelProfile_strings[index];
}

std::string
LineSamplerAttributes::ChannelProfile_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ChannelProfile_strings[index];
}

bool
LineSamplerAttributes::ChannelProfile_FromString(const std::string &s, LineSamplerAttributes::ChannelProfile &val)
{
    val = LineSamplerAttributes::TopHat;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ChannelProfile_strings[i])
        {
            val = (ChannelProfile)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ChannelIntegration
//

static const char *ChannelIntegration_strings[] = {
"NoChannelIntegration", "IntegrateAlongChannel"};

std::string
LineSamplerAttributes::ChannelIntegration_ToString(LineSamplerAttributes::ChannelIntegration t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ChannelIntegration_strings[index];
}

std::string
LineSamplerAttributes::ChannelIntegration_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ChannelIntegration_strings[index];
}

bool
LineSamplerAttributes::ChannelIntegration_FromString(const std::string &s, LineSamplerAttributes::ChannelIntegration &val)
{
    val = LineSamplerAttributes::NoChannelIntegration;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ChannelIntegration_strings[i])
        {
            val = (ChannelIntegration)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ToroidalIntegration
//

static const char *ToroidalIntegration_strings[] = {
"NoToroidalIntegration", "ToroidalTimeSample", "IntegrateToroidally"
};

std::string
LineSamplerAttributes::ToroidalIntegration_ToString(LineSamplerAttributes::ToroidalIntegration t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ToroidalIntegration_strings[index];
}

std::string
LineSamplerAttributes::ToroidalIntegration_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ToroidalIntegration_strings[index];
}

bool
LineSamplerAttributes::ToroidalIntegration_FromString(const std::string &s, LineSamplerAttributes::ToroidalIntegration &val)
{
    val = LineSamplerAttributes::NoToroidalIntegration;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ToroidalIntegration_strings[i])
        {
            val = (ToroidalIntegration)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ToroidalAngleSampling
//

static const char *ToroidalAngleSampling_strings[] = {
"ToroidalAngleAbsoluteSampling", "ToroidalAngleRelativeSampling"};

std::string
LineSamplerAttributes::ToroidalAngleSampling_ToString(LineSamplerAttributes::ToroidalAngleSampling t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ToroidalAngleSampling_strings[index];
}

std::string
LineSamplerAttributes::ToroidalAngleSampling_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ToroidalAngleSampling_strings[index];
}

bool
LineSamplerAttributes::ToroidalAngleSampling_FromString(const std::string &s, LineSamplerAttributes::ToroidalAngleSampling &val)
{
    val = LineSamplerAttributes::ToroidalAngleAbsoluteSampling;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ToroidalAngleSampling_strings[i])
        {
            val = (ToroidalAngleSampling)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::TimeSampling
//

static const char *TimeSampling_strings[] = {
"CurrentTimeStep", "MultipleTimeSteps"};

std::string
LineSamplerAttributes::TimeSampling_ToString(LineSamplerAttributes::TimeSampling t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return TimeSampling_strings[index];
}

std::string
LineSamplerAttributes::TimeSampling_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return TimeSampling_strings[index];
}

bool
LineSamplerAttributes::TimeSampling_FromString(const std::string &s, LineSamplerAttributes::TimeSampling &val)
{
    val = LineSamplerAttributes::CurrentTimeStep;
    for(int i = 0; i < 2; ++i)
    {
        if(s == TimeSampling_strings[i])
        {
            val = (TimeSampling)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Init utility for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void LineSamplerAttributes::Init()
{
    meshGeometry = Toroidal;
    arrayConfiguration = Geometry;
    boundary = Data;
    instanceId = 0;
    nArrays = 1;
    toroidalArrayAngle = 5;
    nChannels = 5;
    channelProjection = Parallel;
    channelLayoutType = ChannelRelative;
    channelOffset = 0.1;
    channelAngle = 5;
    nRows = 1;
    rowOffset = 0.1;
    arrayOrigin[0] = 0;
    arrayOrigin[1] = 0;
    arrayOrigin[2] = 0;
    arrayAxis = Z;
    poloidalAngleStart = 180;
    poloidalAngleStop = 220;
    poloialAngle = 0;
    poloialRTilt = 0;
    poloialZTilt = 0;
    toroidalAngle = 0;
    flipToroidalAngle = false;
    viewGeometry = Surfaces;
    viewDimension = Three;
    donotApplyToAll = true;
    heightPlotScale = 1;
    channelPlotOffset = 0;
    arrayPlotOffset = 0;
    displayTime = Step;
    channelGeometry = Line;
    radius = 0.1;
    divergence = 1;
    channelProfile = TopHat;
    standardDeviation = 1;
    sampleDistance = 0.1;
    sampleVolume = 1;
    sampleArc = 10;
    channelIntegration = NoChannelIntegration;
    toroidalIntegration = NoToroidalIntegration;
    toroidalAngleSampling = ToroidalAngleAbsoluteSampling;
    toroidalAngleStart = 0;
    toroidalAngleStop = 360;
    toroidalAngleStride = 1;
    timeSampling = CurrentTimeStep;
    timeStepStart = 0;
    timeStepStop = 0;
    timeStepStride = 1;
    channelList.push_back(0);
    channelList.push_back(0);
    channelList.push_back(0);
    channelList.push_back(90);
    wallList.push_back(0);
    wallList.push_back(0);
    nChannelListArrays = 1;
    channelListToroidalArrayAngle = 5;
    channelListToroidalAngle = 0;

    LineSamplerAttributes::SelectAll();
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Copy utility for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void LineSamplerAttributes::Copy(const LineSamplerAttributes &obj)
{
    meshGeometry = obj.meshGeometry;
    arrayConfiguration = obj.arrayConfiguration;
    boundary = obj.boundary;
    instanceId = obj.instanceId;
    nArrays = obj.nArrays;
    toroidalArrayAngle = obj.toroidalArrayAngle;
    nChannels = obj.nChannels;
    channelProjection = obj.channelProjection;
    channelLayoutType = obj.channelLayoutType;
    channelOffset = obj.channelOffset;
    channelAngle = obj.channelAngle;
    nRows = obj.nRows;
    rowOffset = obj.rowOffset;
    arrayOrigin[0] = obj.arrayOrigin[0];
    arrayOrigin[1] = obj.arrayOrigin[1];
    arrayOrigin[2] = obj.arrayOrigin[2];

    arrayAxis = obj.arrayAxis;
    poloidalAngleStart = obj.poloidalAngleStart;
    poloidalAngleStop = obj.poloidalAngleStop;
    poloialAngle = obj.poloialAngle;
    poloialRTilt = obj.poloialRTilt;
    poloialZTilt = obj.poloialZTilt;
    toroidalAngle = obj.toroidalAngle;
    flipToroidalAngle = obj.flipToroidalAngle;
    viewGeometry = obj.viewGeometry;
    viewDimension = obj.viewDimension;
    donotApplyToAll = obj.donotApplyToAll;
    heightPlotScale = obj.heightPlotScale;
    channelPlotOffset = obj.channelPlotOffset;
    arrayPlotOffset = obj.arrayPlotOffset;
    displayTime = obj.displayTime;
    channelGeometry = obj.channelGeometry;
    radius = obj.radius;
    divergence = obj.divergence;
    channelProfile = obj.channelProfile;
    standardDeviation = obj.standardDeviation;
    sampleDistance = obj.sampleDistance;
    sampleVolume = obj.sampleVolume;
    sampleArc = obj.sampleArc;
    channelIntegration = obj.channelIntegration;
    toroidalIntegration = obj.toroidalIntegration;
    toroidalAngleSampling = obj.toroidalAngleSampling;
    toroidalAngleStart = obj.toroidalAngleStart;
    toroidalAngleStop = obj.toroidalAngleStop;
    toroidalAngleStride = obj.toroidalAngleStride;
    timeSampling = obj.timeSampling;
    timeStepStart = obj.timeStepStart;
    timeStepStop = obj.timeStepStop;
    timeStepStride = obj.timeStepStride;
    channelList = obj.channelList;
    wallList = obj.wallList;
    nChannelListArrays = obj.nChannelListArrays;
    channelListToroidalArrayAngle = obj.channelListToroidalArrayAngle;
    channelListToroidalAngle = obj.channelListToroidalAngle;

    LineSamplerAttributes::SelectAll();
}

// Type map format string
const char *LineSamplerAttributes::TypeMapFormatString = LINESAMPLERATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t LineSamplerAttributes::TmfsStruct = {LINESAMPLERATTRIBUTES_TMFS};


// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Default constructor for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes() : 
    AttributeSubject(LineSamplerAttributes::TypeMapFormatString)
{
    LineSamplerAttributes::Init();
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Constructor for the derived classes of LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    LineSamplerAttributes::Init();
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Copy constructor for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes(const LineSamplerAttributes &obj) : 
    AttributeSubject(LineSamplerAttributes::TypeMapFormatString)
{
    LineSamplerAttributes::Copy(obj);
}

// ****************************************************************************
// Method: LineSamplerAttributes::LineSamplerAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::LineSamplerAttributes(const LineSamplerAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    LineSamplerAttributes::Copy(obj);
}

// ****************************************************************************
// Method: LineSamplerAttributes::~LineSamplerAttributes
//
// Purpose: 
//   Destructor for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes::~LineSamplerAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: LineSamplerAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the LineSamplerAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

LineSamplerAttributes& 
LineSamplerAttributes::operator = (const LineSamplerAttributes &obj)
{
    if (this == &obj) return *this;

    LineSamplerAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: LineSamplerAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the LineSamplerAttributes class.
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
LineSamplerAttributes::operator == (const LineSamplerAttributes &obj) const
{
    // Compare the arrayOrigin arrays.
    bool arrayOrigin_equal = true;
    for(int i = 0; i < 3 && arrayOrigin_equal; ++i)
        arrayOrigin_equal = (arrayOrigin[i] == obj.arrayOrigin[i]);

    // Create the return value
    return ((meshGeometry == obj.meshGeometry) &&
            (arrayConfiguration == obj.arrayConfiguration) &&
            (boundary == obj.boundary) &&
            (instanceId == obj.instanceId) &&
            (nArrays == obj.nArrays) &&
            (toroidalArrayAngle == obj.toroidalArrayAngle) &&
            (nChannels == obj.nChannels) &&
            (channelProjection == obj.channelProjection) &&
            (channelLayoutType == obj.channelLayoutType) &&
            (channelOffset == obj.channelOffset) &&
            (channelAngle == obj.channelAngle) &&
            (nRows == obj.nRows) &&
            (rowOffset == obj.rowOffset) &&
            arrayOrigin_equal &&
            (arrayAxis == obj.arrayAxis) &&
            (poloidalAngleStart == obj.poloidalAngleStart) &&
            (poloidalAngleStop == obj.poloidalAngleStop) &&
            (poloialAngle == obj.poloialAngle) &&
            (poloialRTilt == obj.poloialRTilt) &&
            (poloialZTilt == obj.poloialZTilt) &&
            (toroidalAngle == obj.toroidalAngle) &&
            (flipToroidalAngle == obj.flipToroidalAngle) &&
            (viewGeometry == obj.viewGeometry) &&
            (viewDimension == obj.viewDimension) &&
            (donotApplyToAll == obj.donotApplyToAll) &&
            (heightPlotScale == obj.heightPlotScale) &&
            (channelPlotOffset == obj.channelPlotOffset) &&
            (arrayPlotOffset == obj.arrayPlotOffset) &&
            (displayTime == obj.displayTime) &&
            (channelGeometry == obj.channelGeometry) &&
            (radius == obj.radius) &&
            (divergence == obj.divergence) &&
            (channelProfile == obj.channelProfile) &&
            (standardDeviation == obj.standardDeviation) &&
            (sampleDistance == obj.sampleDistance) &&
            (sampleVolume == obj.sampleVolume) &&
            (sampleArc == obj.sampleArc) &&
            (channelIntegration == obj.channelIntegration) &&
            (toroidalIntegration == obj.toroidalIntegration) &&
            (toroidalAngleSampling == obj.toroidalAngleSampling) &&
            (toroidalAngleStart == obj.toroidalAngleStart) &&
            (toroidalAngleStop == obj.toroidalAngleStop) &&
            (toroidalAngleStride == obj.toroidalAngleStride) &&
            (timeSampling == obj.timeSampling) &&
            (timeStepStart == obj.timeStepStart) &&
            (timeStepStop == obj.timeStepStop) &&
            (timeStepStride == obj.timeStepStride) &&
            (channelList == obj.channelList) &&
            (wallList == obj.wallList) &&
            (nChannelListArrays == obj.nChannelListArrays) &&
            (channelListToroidalArrayAngle == obj.channelListToroidalArrayAngle) &&
            (channelListToroidalAngle == obj.channelListToroidalAngle));
}

// ****************************************************************************
// Method: LineSamplerAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the LineSamplerAttributes class.
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
LineSamplerAttributes::operator != (const LineSamplerAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: LineSamplerAttributes::TypeName
//
// Purpose: 
//   Type name method for the LineSamplerAttributes class.
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
LineSamplerAttributes::TypeName() const
{
    return "LineSamplerAttributes";
}

// ****************************************************************************
// Method: LineSamplerAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the LineSamplerAttributes class.
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
LineSamplerAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const LineSamplerAttributes *tmp = (const LineSamplerAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: LineSamplerAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the LineSamplerAttributes class.
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
LineSamplerAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new LineSamplerAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: LineSamplerAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the LineSamplerAttributes class.
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
LineSamplerAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new LineSamplerAttributes(*this);
    else
        retval = new LineSamplerAttributes;

    return retval;
}

// ****************************************************************************
// Method: LineSamplerAttributes::SelectAll
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
LineSamplerAttributes::SelectAll()
{
    Select(ID_meshGeometry,                  (void *)&meshGeometry);
    Select(ID_arrayConfiguration,            (void *)&arrayConfiguration);
    Select(ID_boundary,                      (void *)&boundary);
    Select(ID_instanceId,                    (void *)&instanceId);
    Select(ID_nArrays,                       (void *)&nArrays);
    Select(ID_toroidalArrayAngle,            (void *)&toroidalArrayAngle);
    Select(ID_nChannels,                     (void *)&nChannels);
    Select(ID_channelProjection,             (void *)&channelProjection);
    Select(ID_channelLayoutType,             (void *)&channelLayoutType);
    Select(ID_channelOffset,                 (void *)&channelOffset);
    Select(ID_channelAngle,                  (void *)&channelAngle);
    Select(ID_nRows,                         (void *)&nRows);
    Select(ID_rowOffset,                     (void *)&rowOffset);
    Select(ID_arrayOrigin,                   (void *)arrayOrigin, 3);
    Select(ID_arrayAxis,                     (void *)&arrayAxis);
    Select(ID_poloidalAngleStart,            (void *)&poloidalAngleStart);
    Select(ID_poloidalAngleStop,             (void *)&poloidalAngleStop);
    Select(ID_poloialAngle,                  (void *)&poloialAngle);
    Select(ID_poloialRTilt,                  (void *)&poloialRTilt);
    Select(ID_poloialZTilt,                  (void *)&poloialZTilt);
    Select(ID_toroidalAngle,                 (void *)&toroidalAngle);
    Select(ID_flipToroidalAngle,             (void *)&flipToroidalAngle);
    Select(ID_viewGeometry,                  (void *)&viewGeometry);
    Select(ID_viewDimension,                 (void *)&viewDimension);
    Select(ID_donotApplyToAll,               (void *)&donotApplyToAll);
    Select(ID_heightPlotScale,               (void *)&heightPlotScale);
    Select(ID_channelPlotOffset,             (void *)&channelPlotOffset);
    Select(ID_arrayPlotOffset,               (void *)&arrayPlotOffset);
    Select(ID_displayTime,                   (void *)&displayTime);
    Select(ID_channelGeometry,               (void *)&channelGeometry);
    Select(ID_radius,                        (void *)&radius);
    Select(ID_divergence,                    (void *)&divergence);
    Select(ID_channelProfile,                (void *)&channelProfile);
    Select(ID_standardDeviation,             (void *)&standardDeviation);
    Select(ID_sampleDistance,                (void *)&sampleDistance);
    Select(ID_sampleVolume,                  (void *)&sampleVolume);
    Select(ID_sampleArc,                     (void *)&sampleArc);
    Select(ID_channelIntegration,            (void *)&channelIntegration);
    Select(ID_toroidalIntegration,           (void *)&toroidalIntegration);
    Select(ID_toroidalAngleSampling,         (void *)&toroidalAngleSampling);
    Select(ID_toroidalAngleStart,            (void *)&toroidalAngleStart);
    Select(ID_toroidalAngleStop,             (void *)&toroidalAngleStop);
    Select(ID_toroidalAngleStride,           (void *)&toroidalAngleStride);
    Select(ID_timeSampling,                  (void *)&timeSampling);
    Select(ID_timeStepStart,                 (void *)&timeStepStart);
    Select(ID_timeStepStop,                  (void *)&timeStepStop);
    Select(ID_timeStepStride,                (void *)&timeStepStride);
    Select(ID_channelList,                   (void *)&channelList);
    Select(ID_wallList,                      (void *)&wallList);
    Select(ID_nChannelListArrays,            (void *)&nChannelListArrays);
    Select(ID_channelListToroidalArrayAngle, (void *)&channelListToroidalArrayAngle);
    Select(ID_channelListToroidalAngle,      (void *)&channelListToroidalAngle);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LineSamplerAttributes::CreateNode
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
LineSamplerAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    LineSamplerAttributes defaultObject;
    bool addToParent = false;
    // Create a node for LineSamplerAttributes.
    DataNode *node = new DataNode("LineSamplerAttributes");

    if(completeSave || !FieldsEqual(ID_meshGeometry, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("meshGeometry", MeshGeometry_ToString(meshGeometry)));
    }

    if(completeSave || !FieldsEqual(ID_arrayConfiguration, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arrayConfiguration", ArrayConfiguration_ToString(arrayConfiguration)));
    }

    if(completeSave || !FieldsEqual(ID_boundary, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("boundary", Boundary_ToString(boundary)));
    }

    if(completeSave || !FieldsEqual(ID_instanceId, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("instanceId", instanceId));
    }

    if(completeSave || !FieldsEqual(ID_nArrays, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nArrays", nArrays));
    }

    if(completeSave || !FieldsEqual(ID_toroidalArrayAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalArrayAngle", toroidalArrayAngle));
    }

    if(completeSave || !FieldsEqual(ID_nChannels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nChannels", nChannels));
    }

    if(completeSave || !FieldsEqual(ID_channelProjection, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelProjection", ChannelProjection_ToString(channelProjection)));
    }

    if(completeSave || !FieldsEqual(ID_channelLayoutType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelLayoutType", ChannelLayoutType_ToString(channelLayoutType)));
    }

    if(completeSave || !FieldsEqual(ID_channelOffset, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelOffset", channelOffset));
    }

    if(completeSave || !FieldsEqual(ID_channelAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelAngle", channelAngle));
    }

    if(completeSave || !FieldsEqual(ID_nRows, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nRows", nRows));
    }

    if(completeSave || !FieldsEqual(ID_rowOffset, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rowOffset", rowOffset));
    }

    if(completeSave || !FieldsEqual(ID_arrayOrigin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arrayOrigin", arrayOrigin, 3));
    }

    if(completeSave || !FieldsEqual(ID_arrayAxis, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arrayAxis", ArrayAxis_ToString(arrayAxis)));
    }

    if(completeSave || !FieldsEqual(ID_poloidalAngleStart, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("poloidalAngleStart", poloidalAngleStart));
    }

    if(completeSave || !FieldsEqual(ID_poloidalAngleStop, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("poloidalAngleStop", poloidalAngleStop));
    }

    if(completeSave || !FieldsEqual(ID_poloialAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("poloialAngle", poloialAngle));
    }

    if(completeSave || !FieldsEqual(ID_poloialRTilt, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("poloialRTilt", poloialRTilt));
    }

    if(completeSave || !FieldsEqual(ID_poloialZTilt, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("poloialZTilt", poloialZTilt));
    }

    if(completeSave || !FieldsEqual(ID_toroidalAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalAngle", toroidalAngle));
    }

    if(completeSave || !FieldsEqual(ID_flipToroidalAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("flipToroidalAngle", flipToroidalAngle));
    }

    if(completeSave || !FieldsEqual(ID_viewGeometry, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("viewGeometry", ViewGeometry_ToString(viewGeometry)));
    }

    if(completeSave || !FieldsEqual(ID_viewDimension, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("viewDimension", ViewDimension_ToString(viewDimension)));
    }

    if(completeSave || !FieldsEqual(ID_donotApplyToAll, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("donotApplyToAll", donotApplyToAll));
    }

    if(completeSave || !FieldsEqual(ID_heightPlotScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("heightPlotScale", heightPlotScale));
    }

    if(completeSave || !FieldsEqual(ID_channelPlotOffset, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelPlotOffset", channelPlotOffset));
    }

    if(completeSave || !FieldsEqual(ID_arrayPlotOffset, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arrayPlotOffset", arrayPlotOffset));
    }

    if(completeSave || !FieldsEqual(ID_displayTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayTime", DisplayTime_ToString(displayTime)));
    }

    if(completeSave || !FieldsEqual(ID_channelGeometry, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelGeometry", ChannelGeometry_ToString(channelGeometry)));
    }

    if(completeSave || !FieldsEqual(ID_radius, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("radius", radius));
    }

    if(completeSave || !FieldsEqual(ID_divergence, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("divergence", divergence));
    }

    if(completeSave || !FieldsEqual(ID_channelProfile, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelProfile", ChannelProfile_ToString(channelProfile)));
    }

    if(completeSave || !FieldsEqual(ID_standardDeviation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("standardDeviation", standardDeviation));
    }

    if(completeSave || !FieldsEqual(ID_sampleDistance, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleDistance", sampleDistance));
    }

    if(completeSave || !FieldsEqual(ID_sampleVolume, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleVolume", sampleVolume));
    }

    if(completeSave || !FieldsEqual(ID_sampleArc, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleArc", sampleArc));
    }

    if(completeSave || !FieldsEqual(ID_channelIntegration, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelIntegration", ChannelIntegration_ToString(channelIntegration)));
    }

    if(completeSave || !FieldsEqual(ID_toroidalIntegration, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalIntegration", ToroidalIntegration_ToString(toroidalIntegration)));
    }

    if(completeSave || !FieldsEqual(ID_toroidalAngleSampling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalAngleSampling", ToroidalAngleSampling_ToString(toroidalAngleSampling)));
    }

    if(completeSave || !FieldsEqual(ID_toroidalAngleStart, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalAngleStart", toroidalAngleStart));
    }

    if(completeSave || !FieldsEqual(ID_toroidalAngleStop, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalAngleStop", toroidalAngleStop));
    }

    if(completeSave || !FieldsEqual(ID_toroidalAngleStride, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalAngleStride", toroidalAngleStride));
    }

    if(completeSave || !FieldsEqual(ID_timeSampling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeSampling", TimeSampling_ToString(timeSampling)));
    }

    if(completeSave || !FieldsEqual(ID_timeStepStart, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeStepStart", timeStepStart));
    }

    if(completeSave || !FieldsEqual(ID_timeStepStop, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeStepStop", timeStepStop));
    }

    if(completeSave || !FieldsEqual(ID_timeStepStride, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeStepStride", timeStepStride));
    }

    if(completeSave || !FieldsEqual(ID_channelList, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelList", channelList));
    }

    if(completeSave || !FieldsEqual(ID_wallList, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("wallList", wallList));
    }

    if(completeSave || !FieldsEqual(ID_nChannelListArrays, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nChannelListArrays", nChannelListArrays));
    }

    if(completeSave || !FieldsEqual(ID_channelListToroidalArrayAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelListToroidalArrayAngle", channelListToroidalArrayAngle));
    }

    if(completeSave || !FieldsEqual(ID_channelListToroidalAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelListToroidalAngle", channelListToroidalAngle));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: LineSamplerAttributes::SetFromNode
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
LineSamplerAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("LineSamplerAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("meshGeometry")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetMeshGeometry(MeshGeometry(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            MeshGeometry value;
            if(MeshGeometry_FromString(node->AsString(), value))
                SetMeshGeometry(value);
        }
    }
    if((node = searchNode->GetNode("arrayConfiguration")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetArrayConfiguration(ArrayConfiguration(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ArrayConfiguration value;
            if(ArrayConfiguration_FromString(node->AsString(), value))
                SetArrayConfiguration(value);
        }
    }
    if((node = searchNode->GetNode("boundary")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetBoundary(Boundary(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Boundary value;
            if(Boundary_FromString(node->AsString(), value))
                SetBoundary(value);
        }
    }
    if((node = searchNode->GetNode("instanceId")) != 0)
        SetInstanceId(node->AsInt());
    if((node = searchNode->GetNode("nArrays")) != 0)
        SetNArrays(node->AsInt());
    if((node = searchNode->GetNode("toroidalArrayAngle")) != 0)
        SetToroidalArrayAngle(node->AsDouble());
    if((node = searchNode->GetNode("nChannels")) != 0)
        SetNChannels(node->AsInt());
    if((node = searchNode->GetNode("channelProjection")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetChannelProjection(ChannelProjection(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ChannelProjection value;
            if(ChannelProjection_FromString(node->AsString(), value))
                SetChannelProjection(value);
        }
    }
    if((node = searchNode->GetNode("channelLayoutType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetChannelLayoutType(ChannelLayoutType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ChannelLayoutType value;
            if(ChannelLayoutType_FromString(node->AsString(), value))
                SetChannelLayoutType(value);
        }
    }
    if((node = searchNode->GetNode("channelOffset")) != 0)
        SetChannelOffset(node->AsDouble());
    if((node = searchNode->GetNode("channelAngle")) != 0)
        SetChannelAngle(node->AsDouble());
    if((node = searchNode->GetNode("nRows")) != 0)
        SetNRows(node->AsInt());
    if((node = searchNode->GetNode("rowOffset")) != 0)
        SetRowOffset(node->AsDouble());
    if((node = searchNode->GetNode("arrayOrigin")) != 0)
        SetArrayOrigin(node->AsDoubleArray());
    if((node = searchNode->GetNode("arrayAxis")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetArrayAxis(ArrayAxis(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ArrayAxis value;
            if(ArrayAxis_FromString(node->AsString(), value))
                SetArrayAxis(value);
        }
    }
    if((node = searchNode->GetNode("poloidalAngleStart")) != 0)
        SetPoloidalAngleStart(node->AsDouble());
    if((node = searchNode->GetNode("poloidalAngleStop")) != 0)
        SetPoloidalAngleStop(node->AsDouble());
    if((node = searchNode->GetNode("poloialAngle")) != 0)
        SetPoloialAngle(node->AsDouble());
    if((node = searchNode->GetNode("poloialRTilt")) != 0)
        SetPoloialRTilt(node->AsDouble());
    if((node = searchNode->GetNode("poloialZTilt")) != 0)
        SetPoloialZTilt(node->AsDouble());
    if((node = searchNode->GetNode("toroidalAngle")) != 0)
        SetToroidalAngle(node->AsDouble());
    if((node = searchNode->GetNode("flipToroidalAngle")) != 0)
        SetFlipToroidalAngle(node->AsBool());
    if((node = searchNode->GetNode("viewGeometry")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetViewGeometry(ViewGeometry(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ViewGeometry value;
            if(ViewGeometry_FromString(node->AsString(), value))
                SetViewGeometry(value);
        }
    }
    if((node = searchNode->GetNode("viewDimension")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetViewDimension(ViewDimension(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ViewDimension value;
            if(ViewDimension_FromString(node->AsString(), value))
                SetViewDimension(value);
        }
    }
    if((node = searchNode->GetNode("donotApplyToAll")) != 0)
        SetDonotApplyToAll(node->AsBool());
    if((node = searchNode->GetNode("heightPlotScale")) != 0)
        SetHeightPlotScale(node->AsDouble());
    if((node = searchNode->GetNode("channelPlotOffset")) != 0)
        SetChannelPlotOffset(node->AsDouble());
    if((node = searchNode->GetNode("arrayPlotOffset")) != 0)
        SetArrayPlotOffset(node->AsDouble());
    if((node = searchNode->GetNode("displayTime")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDisplayTime(DisplayTime(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DisplayTime value;
            if(DisplayTime_FromString(node->AsString(), value))
                SetDisplayTime(value);
        }
    }
    if((node = searchNode->GetNode("channelGeometry")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetChannelGeometry(ChannelGeometry(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ChannelGeometry value;
            if(ChannelGeometry_FromString(node->AsString(), value))
                SetChannelGeometry(value);
        }
    }
    if((node = searchNode->GetNode("radius")) != 0)
        SetRadius(node->AsDouble());
    if((node = searchNode->GetNode("divergence")) != 0)
        SetDivergence(node->AsDouble());
    if((node = searchNode->GetNode("channelProfile")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetChannelProfile(ChannelProfile(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ChannelProfile value;
            if(ChannelProfile_FromString(node->AsString(), value))
                SetChannelProfile(value);
        }
    }
    if((node = searchNode->GetNode("standardDeviation")) != 0)
        SetStandardDeviation(node->AsDouble());
    if((node = searchNode->GetNode("sampleDistance")) != 0)
        SetSampleDistance(node->AsDouble());
    if((node = searchNode->GetNode("sampleVolume")) != 0)
        SetSampleVolume(node->AsDouble());
    if((node = searchNode->GetNode("sampleArc")) != 0)
        SetSampleArc(node->AsDouble());
    if((node = searchNode->GetNode("channelIntegration")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetChannelIntegration(ChannelIntegration(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ChannelIntegration value;
            if(ChannelIntegration_FromString(node->AsString(), value))
                SetChannelIntegration(value);
        }
    }
    if((node = searchNode->GetNode("toroidalIntegration")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetToroidalIntegration(ToroidalIntegration(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ToroidalIntegration value;
            if(ToroidalIntegration_FromString(node->AsString(), value))
                SetToroidalIntegration(value);
        }
    }
    if((node = searchNode->GetNode("toroidalAngleSampling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetToroidalAngleSampling(ToroidalAngleSampling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ToroidalAngleSampling value;
            if(ToroidalAngleSampling_FromString(node->AsString(), value))
                SetToroidalAngleSampling(value);
        }
    }
    if((node = searchNode->GetNode("toroidalAngleStart")) != 0)
        SetToroidalAngleStart(node->AsDouble());
    if((node = searchNode->GetNode("toroidalAngleStop")) != 0)
        SetToroidalAngleStop(node->AsDouble());
    if((node = searchNode->GetNode("toroidalAngleStride")) != 0)
        SetToroidalAngleStride(node->AsDouble());
    if((node = searchNode->GetNode("timeSampling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetTimeSampling(TimeSampling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TimeSampling value;
            if(TimeSampling_FromString(node->AsString(), value))
                SetTimeSampling(value);
        }
    }
    if((node = searchNode->GetNode("timeStepStart")) != 0)
        SetTimeStepStart(node->AsInt());
    if((node = searchNode->GetNode("timeStepStop")) != 0)
        SetTimeStepStop(node->AsInt());
    if((node = searchNode->GetNode("timeStepStride")) != 0)
        SetTimeStepStride(node->AsInt());
    if((node = searchNode->GetNode("channelList")) != 0)
        SetChannelList(node->AsDoubleVector());
    if((node = searchNode->GetNode("wallList")) != 0)
        SetWallList(node->AsDoubleVector());
    if((node = searchNode->GetNode("nChannelListArrays")) != 0)
        SetNChannelListArrays(node->AsInt());
    if((node = searchNode->GetNode("channelListToroidalArrayAngle")) != 0)
        SetChannelListToroidalArrayAngle(node->AsDouble());
    if((node = searchNode->GetNode("channelListToroidalAngle")) != 0)
        SetChannelListToroidalAngle(node->AsDouble());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
LineSamplerAttributes::SetMeshGeometry(LineSamplerAttributes::MeshGeometry meshGeometry_)
{
    meshGeometry = meshGeometry_;
    Select(ID_meshGeometry, (void *)&meshGeometry);
}

void
LineSamplerAttributes::SetArrayConfiguration(LineSamplerAttributes::ArrayConfiguration arrayConfiguration_)
{
    arrayConfiguration = arrayConfiguration_;
    Select(ID_arrayConfiguration, (void *)&arrayConfiguration);
}

void
LineSamplerAttributes::SetBoundary(LineSamplerAttributes::Boundary boundary_)
{
    boundary = boundary_;
    Select(ID_boundary, (void *)&boundary);
}

void
LineSamplerAttributes::SetInstanceId(int instanceId_)
{
    instanceId = instanceId_;
    Select(ID_instanceId, (void *)&instanceId);
}

void
LineSamplerAttributes::SetNArrays(int nArrays_)
{
    nArrays = nArrays_;
    Select(ID_nArrays, (void *)&nArrays);
}

void
LineSamplerAttributes::SetToroidalArrayAngle(double toroidalArrayAngle_)
{
    toroidalArrayAngle = toroidalArrayAngle_;
    Select(ID_toroidalArrayAngle, (void *)&toroidalArrayAngle);
}

void
LineSamplerAttributes::SetNChannels(int nChannels_)
{
    nChannels = nChannels_;
    Select(ID_nChannels, (void *)&nChannels);
}

void
LineSamplerAttributes::SetChannelProjection(LineSamplerAttributes::ChannelProjection channelProjection_)
{
    channelProjection = channelProjection_;
    Select(ID_channelProjection, (void *)&channelProjection);
}

void
LineSamplerAttributes::SetChannelLayoutType(LineSamplerAttributes::ChannelLayoutType channelLayoutType_)
{
    channelLayoutType = channelLayoutType_;
    Select(ID_channelLayoutType, (void *)&channelLayoutType);
}

void
LineSamplerAttributes::SetChannelOffset(double channelOffset_)
{
    channelOffset = channelOffset_;
    Select(ID_channelOffset, (void *)&channelOffset);
}

void
LineSamplerAttributes::SetChannelAngle(double channelAngle_)
{
    channelAngle = channelAngle_;
    Select(ID_channelAngle, (void *)&channelAngle);
}

void
LineSamplerAttributes::SetNRows(int nRows_)
{
    nRows = nRows_;
    Select(ID_nRows, (void *)&nRows);
}

void
LineSamplerAttributes::SetRowOffset(double rowOffset_)
{
    rowOffset = rowOffset_;
    Select(ID_rowOffset, (void *)&rowOffset);
}

void
LineSamplerAttributes::SetArrayOrigin(const double *arrayOrigin_)
{
    arrayOrigin[0] = arrayOrigin_[0];
    arrayOrigin[1] = arrayOrigin_[1];
    arrayOrigin[2] = arrayOrigin_[2];
    Select(ID_arrayOrigin, (void *)arrayOrigin, 3);
}

void
LineSamplerAttributes::SetArrayAxis(LineSamplerAttributes::ArrayAxis arrayAxis_)
{
    arrayAxis = arrayAxis_;
    Select(ID_arrayAxis, (void *)&arrayAxis);
}

void
LineSamplerAttributes::SetPoloidalAngleStart(double poloidalAngleStart_)
{
    poloidalAngleStart = poloidalAngleStart_;
    Select(ID_poloidalAngleStart, (void *)&poloidalAngleStart);
}

void
LineSamplerAttributes::SetPoloidalAngleStop(double poloidalAngleStop_)
{
    poloidalAngleStop = poloidalAngleStop_;
    Select(ID_poloidalAngleStop, (void *)&poloidalAngleStop);
}

void
LineSamplerAttributes::SetPoloialAngle(double poloialAngle_)
{
    poloialAngle = poloialAngle_;
    Select(ID_poloialAngle, (void *)&poloialAngle);
}

void
LineSamplerAttributes::SetPoloialRTilt(double poloialRTilt_)
{
    poloialRTilt = poloialRTilt_;
    Select(ID_poloialRTilt, (void *)&poloialRTilt);
}

void
LineSamplerAttributes::SetPoloialZTilt(double poloialZTilt_)
{
    poloialZTilt = poloialZTilt_;
    Select(ID_poloialZTilt, (void *)&poloialZTilt);
}

void
LineSamplerAttributes::SetToroidalAngle(double toroidalAngle_)
{
    toroidalAngle = toroidalAngle_;
    Select(ID_toroidalAngle, (void *)&toroidalAngle);
}

void
LineSamplerAttributes::SetFlipToroidalAngle(bool flipToroidalAngle_)
{
    flipToroidalAngle = flipToroidalAngle_;
    Select(ID_flipToroidalAngle, (void *)&flipToroidalAngle);
}

void
LineSamplerAttributes::SetViewGeometry(LineSamplerAttributes::ViewGeometry viewGeometry_)
{
    viewGeometry = viewGeometry_;
    Select(ID_viewGeometry, (void *)&viewGeometry);
}

void
LineSamplerAttributes::SetViewDimension(LineSamplerAttributes::ViewDimension viewDimension_)
{
    viewDimension = viewDimension_;
    Select(ID_viewDimension, (void *)&viewDimension);
}

void
LineSamplerAttributes::SetDonotApplyToAll(bool donotApplyToAll_)
{
    donotApplyToAll = donotApplyToAll_;
    Select(ID_donotApplyToAll, (void *)&donotApplyToAll);
}

void
LineSamplerAttributes::SetHeightPlotScale(double heightPlotScale_)
{
    heightPlotScale = heightPlotScale_;
    Select(ID_heightPlotScale, (void *)&heightPlotScale);
}

void
LineSamplerAttributes::SetChannelPlotOffset(double channelPlotOffset_)
{
    channelPlotOffset = channelPlotOffset_;
    Select(ID_channelPlotOffset, (void *)&channelPlotOffset);
}

void
LineSamplerAttributes::SetArrayPlotOffset(double arrayPlotOffset_)
{
    arrayPlotOffset = arrayPlotOffset_;
    Select(ID_arrayPlotOffset, (void *)&arrayPlotOffset);
}

void
LineSamplerAttributes::SetDisplayTime(LineSamplerAttributes::DisplayTime displayTime_)
{
    displayTime = displayTime_;
    Select(ID_displayTime, (void *)&displayTime);
}

void
LineSamplerAttributes::SetChannelGeometry(LineSamplerAttributes::ChannelGeometry channelGeometry_)
{
    channelGeometry = channelGeometry_;
    Select(ID_channelGeometry, (void *)&channelGeometry);
}

void
LineSamplerAttributes::SetRadius(double radius_)
{
    radius = radius_;
    Select(ID_radius, (void *)&radius);
}

void
LineSamplerAttributes::SetDivergence(double divergence_)
{
    divergence = divergence_;
    Select(ID_divergence, (void *)&divergence);
}

void
LineSamplerAttributes::SetChannelProfile(LineSamplerAttributes::ChannelProfile channelProfile_)
{
    channelProfile = channelProfile_;
    Select(ID_channelProfile, (void *)&channelProfile);
}

void
LineSamplerAttributes::SetStandardDeviation(double standardDeviation_)
{
    standardDeviation = standardDeviation_;
    Select(ID_standardDeviation, (void *)&standardDeviation);
}

void
LineSamplerAttributes::SetSampleDistance(double sampleDistance_)
{
    sampleDistance = sampleDistance_;
    Select(ID_sampleDistance, (void *)&sampleDistance);
}

void
LineSamplerAttributes::SetSampleVolume(double sampleVolume_)
{
    sampleVolume = sampleVolume_;
    Select(ID_sampleVolume, (void *)&sampleVolume);
}

void
LineSamplerAttributes::SetSampleArc(double sampleArc_)
{
    sampleArc = sampleArc_;
    Select(ID_sampleArc, (void *)&sampleArc);
}

void
LineSamplerAttributes::SetChannelIntegration(LineSamplerAttributes::ChannelIntegration channelIntegration_)
{
    channelIntegration = channelIntegration_;
    Select(ID_channelIntegration, (void *)&channelIntegration);
}

void
LineSamplerAttributes::SetToroidalIntegration(LineSamplerAttributes::ToroidalIntegration toroidalIntegration_)
{
    toroidalIntegration = toroidalIntegration_;
    Select(ID_toroidalIntegration, (void *)&toroidalIntegration);
}

void
LineSamplerAttributes::SetToroidalAngleSampling(LineSamplerAttributes::ToroidalAngleSampling toroidalAngleSampling_)
{
    toroidalAngleSampling = toroidalAngleSampling_;
    Select(ID_toroidalAngleSampling, (void *)&toroidalAngleSampling);
}

void
LineSamplerAttributes::SetToroidalAngleStart(double toroidalAngleStart_)
{
    toroidalAngleStart = toroidalAngleStart_;
    Select(ID_toroidalAngleStart, (void *)&toroidalAngleStart);
}

void
LineSamplerAttributes::SetToroidalAngleStop(double toroidalAngleStop_)
{
    toroidalAngleStop = toroidalAngleStop_;
    Select(ID_toroidalAngleStop, (void *)&toroidalAngleStop);
}

void
LineSamplerAttributes::SetToroidalAngleStride(double toroidalAngleStride_)
{
    toroidalAngleStride = toroidalAngleStride_;
    Select(ID_toroidalAngleStride, (void *)&toroidalAngleStride);
}

void
LineSamplerAttributes::SetTimeSampling(LineSamplerAttributes::TimeSampling timeSampling_)
{
    timeSampling = timeSampling_;
    Select(ID_timeSampling, (void *)&timeSampling);
}

void
LineSamplerAttributes::SetTimeStepStart(int timeStepStart_)
{
    timeStepStart = timeStepStart_;
    Select(ID_timeStepStart, (void *)&timeStepStart);
}

void
LineSamplerAttributes::SetTimeStepStop(int timeStepStop_)
{
    timeStepStop = timeStepStop_;
    Select(ID_timeStepStop, (void *)&timeStepStop);
}

void
LineSamplerAttributes::SetTimeStepStride(int timeStepStride_)
{
    timeStepStride = timeStepStride_;
    Select(ID_timeStepStride, (void *)&timeStepStride);
}

void
LineSamplerAttributes::SetChannelList(const doubleVector &channelList_)
{
    channelList = channelList_;
    Select(ID_channelList, (void *)&channelList);
}

void
LineSamplerAttributes::SetWallList(const doubleVector &wallList_)
{
    wallList = wallList_;
    Select(ID_wallList, (void *)&wallList);
}

void
LineSamplerAttributes::SetNChannelListArrays(int nChannelListArrays_)
{
    nChannelListArrays = nChannelListArrays_;
    Select(ID_nChannelListArrays, (void *)&nChannelListArrays);
}

void
LineSamplerAttributes::SetChannelListToroidalArrayAngle(double channelListToroidalArrayAngle_)
{
    channelListToroidalArrayAngle = channelListToroidalArrayAngle_;
    Select(ID_channelListToroidalArrayAngle, (void *)&channelListToroidalArrayAngle);
}

void
LineSamplerAttributes::SetChannelListToroidalAngle(double channelListToroidalAngle_)
{
    channelListToroidalAngle = channelListToroidalAngle_;
    Select(ID_channelListToroidalAngle, (void *)&channelListToroidalAngle);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

LineSamplerAttributes::MeshGeometry
LineSamplerAttributes::GetMeshGeometry() const
{
    return MeshGeometry(meshGeometry);
}

LineSamplerAttributes::ArrayConfiguration
LineSamplerAttributes::GetArrayConfiguration() const
{
    return ArrayConfiguration(arrayConfiguration);
}

LineSamplerAttributes::Boundary
LineSamplerAttributes::GetBoundary() const
{
    return Boundary(boundary);
}

int
LineSamplerAttributes::GetInstanceId() const
{
    return instanceId;
}

int
LineSamplerAttributes::GetNArrays() const
{
    return nArrays;
}

double
LineSamplerAttributes::GetToroidalArrayAngle() const
{
    return toroidalArrayAngle;
}

int
LineSamplerAttributes::GetNChannels() const
{
    return nChannels;
}

LineSamplerAttributes::ChannelProjection
LineSamplerAttributes::GetChannelProjection() const
{
    return ChannelProjection(channelProjection);
}

LineSamplerAttributes::ChannelLayoutType
LineSamplerAttributes::GetChannelLayoutType() const
{
    return ChannelLayoutType(channelLayoutType);
}

double
LineSamplerAttributes::GetChannelOffset() const
{
    return channelOffset;
}

double
LineSamplerAttributes::GetChannelAngle() const
{
    return channelAngle;
}

int
LineSamplerAttributes::GetNRows() const
{
    return nRows;
}

double
LineSamplerAttributes::GetRowOffset() const
{
    return rowOffset;
}

const double *
LineSamplerAttributes::GetArrayOrigin() const
{
    return arrayOrigin;
}

double *
LineSamplerAttributes::GetArrayOrigin()
{
    return arrayOrigin;
}

LineSamplerAttributes::ArrayAxis
LineSamplerAttributes::GetArrayAxis() const
{
    return ArrayAxis(arrayAxis);
}

double
LineSamplerAttributes::GetPoloidalAngleStart() const
{
    return poloidalAngleStart;
}

double
LineSamplerAttributes::GetPoloidalAngleStop() const
{
    return poloidalAngleStop;
}

double
LineSamplerAttributes::GetPoloialAngle() const
{
    return poloialAngle;
}

double
LineSamplerAttributes::GetPoloialRTilt() const
{
    return poloialRTilt;
}

double
LineSamplerAttributes::GetPoloialZTilt() const
{
    return poloialZTilt;
}

double
LineSamplerAttributes::GetToroidalAngle() const
{
    return toroidalAngle;
}

bool
LineSamplerAttributes::GetFlipToroidalAngle() const
{
    return flipToroidalAngle;
}

LineSamplerAttributes::ViewGeometry
LineSamplerAttributes::GetViewGeometry() const
{
    return ViewGeometry(viewGeometry);
}

LineSamplerAttributes::ViewDimension
LineSamplerAttributes::GetViewDimension() const
{
    return ViewDimension(viewDimension);
}

bool
LineSamplerAttributes::GetDonotApplyToAll() const
{
    return donotApplyToAll;
}

double
LineSamplerAttributes::GetHeightPlotScale() const
{
    return heightPlotScale;
}

double
LineSamplerAttributes::GetChannelPlotOffset() const
{
    return channelPlotOffset;
}

double
LineSamplerAttributes::GetArrayPlotOffset() const
{
    return arrayPlotOffset;
}

LineSamplerAttributes::DisplayTime
LineSamplerAttributes::GetDisplayTime() const
{
    return DisplayTime(displayTime);
}

LineSamplerAttributes::ChannelGeometry
LineSamplerAttributes::GetChannelGeometry() const
{
    return ChannelGeometry(channelGeometry);
}

double
LineSamplerAttributes::GetRadius() const
{
    return radius;
}

double
LineSamplerAttributes::GetDivergence() const
{
    return divergence;
}

LineSamplerAttributes::ChannelProfile
LineSamplerAttributes::GetChannelProfile() const
{
    return ChannelProfile(channelProfile);
}

double
LineSamplerAttributes::GetStandardDeviation() const
{
    return standardDeviation;
}

double
LineSamplerAttributes::GetSampleDistance() const
{
    return sampleDistance;
}

double
LineSamplerAttributes::GetSampleVolume() const
{
    return sampleVolume;
}

double
LineSamplerAttributes::GetSampleArc() const
{
    return sampleArc;
}

LineSamplerAttributes::ChannelIntegration
LineSamplerAttributes::GetChannelIntegration() const
{
    return ChannelIntegration(channelIntegration);
}

LineSamplerAttributes::ToroidalIntegration
LineSamplerAttributes::GetToroidalIntegration() const
{
    return ToroidalIntegration(toroidalIntegration);
}

LineSamplerAttributes::ToroidalAngleSampling
LineSamplerAttributes::GetToroidalAngleSampling() const
{
    return ToroidalAngleSampling(toroidalAngleSampling);
}

double
LineSamplerAttributes::GetToroidalAngleStart() const
{
    return toroidalAngleStart;
}

double
LineSamplerAttributes::GetToroidalAngleStop() const
{
    return toroidalAngleStop;
}

double
LineSamplerAttributes::GetToroidalAngleStride() const
{
    return toroidalAngleStride;
}

LineSamplerAttributes::TimeSampling
LineSamplerAttributes::GetTimeSampling() const
{
    return TimeSampling(timeSampling);
}

int
LineSamplerAttributes::GetTimeStepStart() const
{
    return timeStepStart;
}

int
LineSamplerAttributes::GetTimeStepStop() const
{
    return timeStepStop;
}

int
LineSamplerAttributes::GetTimeStepStride() const
{
    return timeStepStride;
}

const doubleVector &
LineSamplerAttributes::GetChannelList() const
{
    return channelList;
}

doubleVector &
LineSamplerAttributes::GetChannelList()
{
    return channelList;
}

const doubleVector &
LineSamplerAttributes::GetWallList() const
{
    return wallList;
}

doubleVector &
LineSamplerAttributes::GetWallList()
{
    return wallList;
}

int
LineSamplerAttributes::GetNChannelListArrays() const
{
    return nChannelListArrays;
}

double
LineSamplerAttributes::GetChannelListToroidalArrayAngle() const
{
    return channelListToroidalArrayAngle;
}

double
LineSamplerAttributes::GetChannelListToroidalAngle() const
{
    return channelListToroidalAngle;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
LineSamplerAttributes::SelectArrayOrigin()
{
    Select(ID_arrayOrigin, (void *)arrayOrigin, 3);
}

void
LineSamplerAttributes::SelectChannelList()
{
    Select(ID_channelList, (void *)&channelList);
}

void
LineSamplerAttributes::SelectWallList()
{
    Select(ID_wallList, (void *)&wallList);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: LineSamplerAttributes::GetFieldName
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
LineSamplerAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_meshGeometry:                  return "meshGeometry";
    case ID_arrayConfiguration:            return "arrayConfiguration";
    case ID_boundary:                      return "boundary";
    case ID_instanceId:                    return "instanceId";
    case ID_nArrays:                       return "nArrays";
    case ID_toroidalArrayAngle:            return "toroidalArrayAngle";
    case ID_nChannels:                     return "nChannels";
    case ID_channelProjection:             return "channelProjection";
    case ID_channelLayoutType:             return "channelLayoutType";
    case ID_channelOffset:                 return "channelOffset";
    case ID_channelAngle:                  return "channelAngle";
    case ID_nRows:                         return "nRows";
    case ID_rowOffset:                     return "rowOffset";
    case ID_arrayOrigin:                   return "arrayOrigin";
    case ID_arrayAxis:                     return "arrayAxis";
    case ID_poloidalAngleStart:            return "poloidalAngleStart";
    case ID_poloidalAngleStop:             return "poloidalAngleStop";
    case ID_poloialAngle:                  return "poloialAngle";
    case ID_poloialRTilt:                  return "poloialRTilt";
    case ID_poloialZTilt:                  return "poloialZTilt";
    case ID_toroidalAngle:                 return "toroidalAngle";
    case ID_flipToroidalAngle:             return "flipToroidalAngle";
    case ID_viewGeometry:                  return "viewGeometry";
    case ID_viewDimension:                 return "viewDimension";
    case ID_donotApplyToAll:               return "donotApplyToAll";
    case ID_heightPlotScale:               return "heightPlotScale";
    case ID_channelPlotOffset:             return "channelPlotOffset";
    case ID_arrayPlotOffset:               return "arrayPlotOffset";
    case ID_displayTime:                   return "displayTime";
    case ID_channelGeometry:               return "channelGeometry";
    case ID_radius:                        return "radius";
    case ID_divergence:                    return "divergence";
    case ID_channelProfile:                return "channelProfile";
    case ID_standardDeviation:             return "standardDeviation";
    case ID_sampleDistance:                return "sampleDistance";
    case ID_sampleVolume:                  return "sampleVolume";
    case ID_sampleArc:                     return "sampleArc";
    case ID_channelIntegration:            return "channelIntegration";
    case ID_toroidalIntegration:           return "toroidalIntegration";
    case ID_toroidalAngleSampling:         return "toroidalAngleSampling";
    case ID_toroidalAngleStart:            return "toroidalAngleStart";
    case ID_toroidalAngleStop:             return "toroidalAngleStop";
    case ID_toroidalAngleStride:           return "toroidalAngleStride";
    case ID_timeSampling:                  return "timeSampling";
    case ID_timeStepStart:                 return "timeStepStart";
    case ID_timeStepStop:                  return "timeStepStop";
    case ID_timeStepStride:                return "timeStepStride";
    case ID_channelList:                   return "channelList";
    case ID_wallList:                      return "wallList";
    case ID_nChannelListArrays:            return "nChannelListArrays";
    case ID_channelListToroidalArrayAngle: return "channelListToroidalArrayAngle";
    case ID_channelListToroidalAngle:      return "channelListToroidalAngle";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LineSamplerAttributes::GetFieldType
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
LineSamplerAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_meshGeometry:                  return FieldType_enum;
    case ID_arrayConfiguration:            return FieldType_enum;
    case ID_boundary:                      return FieldType_enum;
    case ID_instanceId:                    return FieldType_int;
    case ID_nArrays:                       return FieldType_int;
    case ID_toroidalArrayAngle:            return FieldType_double;
    case ID_nChannels:                     return FieldType_int;
    case ID_channelProjection:             return FieldType_enum;
    case ID_channelLayoutType:             return FieldType_enum;
    case ID_channelOffset:                 return FieldType_double;
    case ID_channelAngle:                  return FieldType_double;
    case ID_nRows:                         return FieldType_int;
    case ID_rowOffset:                     return FieldType_double;
    case ID_arrayOrigin:                   return FieldType_doubleArray;
    case ID_arrayAxis:                     return FieldType_enum;
    case ID_poloidalAngleStart:            return FieldType_double;
    case ID_poloidalAngleStop:             return FieldType_double;
    case ID_poloialAngle:                  return FieldType_double;
    case ID_poloialRTilt:                  return FieldType_double;
    case ID_poloialZTilt:                  return FieldType_double;
    case ID_toroidalAngle:                 return FieldType_double;
    case ID_flipToroidalAngle:             return FieldType_bool;
    case ID_viewGeometry:                  return FieldType_enum;
    case ID_viewDimension:                 return FieldType_enum;
    case ID_donotApplyToAll:               return FieldType_bool;
    case ID_heightPlotScale:               return FieldType_double;
    case ID_channelPlotOffset:             return FieldType_double;
    case ID_arrayPlotOffset:               return FieldType_double;
    case ID_displayTime:                   return FieldType_enum;
    case ID_channelGeometry:               return FieldType_enum;
    case ID_radius:                        return FieldType_double;
    case ID_divergence:                    return FieldType_double;
    case ID_channelProfile:                return FieldType_enum;
    case ID_standardDeviation:             return FieldType_double;
    case ID_sampleDistance:                return FieldType_double;
    case ID_sampleVolume:                  return FieldType_double;
    case ID_sampleArc:                     return FieldType_double;
    case ID_channelIntegration:            return FieldType_enum;
    case ID_toroidalIntegration:           return FieldType_enum;
    case ID_toroidalAngleSampling:         return FieldType_enum;
    case ID_toroidalAngleStart:            return FieldType_double;
    case ID_toroidalAngleStop:             return FieldType_double;
    case ID_toroidalAngleStride:           return FieldType_double;
    case ID_timeSampling:                  return FieldType_enum;
    case ID_timeStepStart:                 return FieldType_int;
    case ID_timeStepStop:                  return FieldType_int;
    case ID_timeStepStride:                return FieldType_int;
    case ID_channelList:                   return FieldType_doubleVector;
    case ID_wallList:                      return FieldType_doubleVector;
    case ID_nChannelListArrays:            return FieldType_int;
    case ID_channelListToroidalArrayAngle: return FieldType_double;
    case ID_channelListToroidalAngle:      return FieldType_double;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: LineSamplerAttributes::GetFieldTypeName
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
LineSamplerAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_meshGeometry:                  return "enum";
    case ID_arrayConfiguration:            return "enum";
    case ID_boundary:                      return "enum";
    case ID_instanceId:                    return "int";
    case ID_nArrays:                       return "int";
    case ID_toroidalArrayAngle:            return "double";
    case ID_nChannels:                     return "int";
    case ID_channelProjection:             return "enum";
    case ID_channelLayoutType:             return "enum";
    case ID_channelOffset:                 return "double";
    case ID_channelAngle:                  return "double";
    case ID_nRows:                         return "int";
    case ID_rowOffset:                     return "double";
    case ID_arrayOrigin:                   return "doubleArray";
    case ID_arrayAxis:                     return "enum";
    case ID_poloidalAngleStart:            return "double";
    case ID_poloidalAngleStop:             return "double";
    case ID_poloialAngle:                  return "double";
    case ID_poloialRTilt:                  return "double";
    case ID_poloialZTilt:                  return "double";
    case ID_toroidalAngle:                 return "double";
    case ID_flipToroidalAngle:             return "bool";
    case ID_viewGeometry:                  return "enum";
    case ID_viewDimension:                 return "enum";
    case ID_donotApplyToAll:               return "bool";
    case ID_heightPlotScale:               return "double";
    case ID_channelPlotOffset:             return "double";
    case ID_arrayPlotOffset:               return "double";
    case ID_displayTime:                   return "enum";
    case ID_channelGeometry:               return "enum";
    case ID_radius:                        return "double";
    case ID_divergence:                    return "double";
    case ID_channelProfile:                return "enum";
    case ID_standardDeviation:             return "double";
    case ID_sampleDistance:                return "double";
    case ID_sampleVolume:                  return "double";
    case ID_sampleArc:                     return "double";
    case ID_channelIntegration:            return "enum";
    case ID_toroidalIntegration:           return "enum";
    case ID_toroidalAngleSampling:         return "enum";
    case ID_toroidalAngleStart:            return "double";
    case ID_toroidalAngleStop:             return "double";
    case ID_toroidalAngleStride:           return "double";
    case ID_timeSampling:                  return "enum";
    case ID_timeStepStart:                 return "int";
    case ID_timeStepStop:                  return "int";
    case ID_timeStepStride:                return "int";
    case ID_channelList:                   return "doubleVector";
    case ID_wallList:                      return "doubleVector";
    case ID_nChannelListArrays:            return "int";
    case ID_channelListToroidalArrayAngle: return "double";
    case ID_channelListToroidalAngle:      return "double";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: LineSamplerAttributes::FieldsEqual
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
LineSamplerAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const LineSamplerAttributes &obj = *((const LineSamplerAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_meshGeometry:
        {  // new scope
        retval = (meshGeometry == obj.meshGeometry);
        }
        break;
    case ID_arrayConfiguration:
        {  // new scope
        retval = (arrayConfiguration == obj.arrayConfiguration);
        }
        break;
    case ID_boundary:
        {  // new scope
        retval = (boundary == obj.boundary);
        }
        break;
    case ID_instanceId:
        {  // new scope
        retval = (instanceId == obj.instanceId);
        }
        break;
    case ID_nArrays:
        {  // new scope
        retval = (nArrays == obj.nArrays);
        }
        break;
    case ID_toroidalArrayAngle:
        {  // new scope
        retval = (toroidalArrayAngle == obj.toroidalArrayAngle);
        }
        break;
    case ID_nChannels:
        {  // new scope
        retval = (nChannels == obj.nChannels);
        }
        break;
    case ID_channelProjection:
        {  // new scope
        retval = (channelProjection == obj.channelProjection);
        }
        break;
    case ID_channelLayoutType:
        {  // new scope
        retval = (channelLayoutType == obj.channelLayoutType);
        }
        break;
    case ID_channelOffset:
        {  // new scope
        retval = (channelOffset == obj.channelOffset);
        }
        break;
    case ID_channelAngle:
        {  // new scope
        retval = (channelAngle == obj.channelAngle);
        }
        break;
    case ID_nRows:
        {  // new scope
        retval = (nRows == obj.nRows);
        }
        break;
    case ID_rowOffset:
        {  // new scope
        retval = (rowOffset == obj.rowOffset);
        }
        break;
    case ID_arrayOrigin:
        {  // new scope
        // Compare the arrayOrigin arrays.
        bool arrayOrigin_equal = true;
        for(int i = 0; i < 3 && arrayOrigin_equal; ++i)
            arrayOrigin_equal = (arrayOrigin[i] == obj.arrayOrigin[i]);

        retval = arrayOrigin_equal;
        }
        break;
    case ID_arrayAxis:
        {  // new scope
        retval = (arrayAxis == obj.arrayAxis);
        }
        break;
    case ID_poloidalAngleStart:
        {  // new scope
        retval = (poloidalAngleStart == obj.poloidalAngleStart);
        }
        break;
    case ID_poloidalAngleStop:
        {  // new scope
        retval = (poloidalAngleStop == obj.poloidalAngleStop);
        }
        break;
    case ID_poloialAngle:
        {  // new scope
        retval = (poloialAngle == obj.poloialAngle);
        }
        break;
    case ID_poloialRTilt:
        {  // new scope
        retval = (poloialRTilt == obj.poloialRTilt);
        }
        break;
    case ID_poloialZTilt:
        {  // new scope
        retval = (poloialZTilt == obj.poloialZTilt);
        }
        break;
    case ID_toroidalAngle:
        {  // new scope
        retval = (toroidalAngle == obj.toroidalAngle);
        }
        break;
    case ID_flipToroidalAngle:
        {  // new scope
        retval = (flipToroidalAngle == obj.flipToroidalAngle);
        }
        break;
    case ID_viewGeometry:
        {  // new scope
        retval = (viewGeometry == obj.viewGeometry);
        }
        break;
    case ID_viewDimension:
        {  // new scope
        retval = (viewDimension == obj.viewDimension);
        }
        break;
    case ID_donotApplyToAll:
        {  // new scope
        retval = (donotApplyToAll == obj.donotApplyToAll);
        }
        break;
    case ID_heightPlotScale:
        {  // new scope
        retval = (heightPlotScale == obj.heightPlotScale);
        }
        break;
    case ID_channelPlotOffset:
        {  // new scope
        retval = (channelPlotOffset == obj.channelPlotOffset);
        }
        break;
    case ID_arrayPlotOffset:
        {  // new scope
        retval = (arrayPlotOffset == obj.arrayPlotOffset);
        }
        break;
    case ID_displayTime:
        {  // new scope
        retval = (displayTime == obj.displayTime);
        }
        break;
    case ID_channelGeometry:
        {  // new scope
        retval = (channelGeometry == obj.channelGeometry);
        }
        break;
    case ID_radius:
        {  // new scope
        retval = (radius == obj.radius);
        }
        break;
    case ID_divergence:
        {  // new scope
        retval = (divergence == obj.divergence);
        }
        break;
    case ID_channelProfile:
        {  // new scope
        retval = (channelProfile == obj.channelProfile);
        }
        break;
    case ID_standardDeviation:
        {  // new scope
        retval = (standardDeviation == obj.standardDeviation);
        }
        break;
    case ID_sampleDistance:
        {  // new scope
        retval = (sampleDistance == obj.sampleDistance);
        }
        break;
    case ID_sampleVolume:
        {  // new scope
        retval = (sampleVolume == obj.sampleVolume);
        }
        break;
    case ID_sampleArc:
        {  // new scope
        retval = (sampleArc == obj.sampleArc);
        }
        break;
    case ID_channelIntegration:
        {  // new scope
        retval = (channelIntegration == obj.channelIntegration);
        }
        break;
    case ID_toroidalIntegration:
        {  // new scope
        retval = (toroidalIntegration == obj.toroidalIntegration);
        }
        break;
    case ID_toroidalAngleSampling:
        {  // new scope
        retval = (toroidalAngleSampling == obj.toroidalAngleSampling);
        }
        break;
    case ID_toroidalAngleStart:
        {  // new scope
        retval = (toroidalAngleStart == obj.toroidalAngleStart);
        }
        break;
    case ID_toroidalAngleStop:
        {  // new scope
        retval = (toroidalAngleStop == obj.toroidalAngleStop);
        }
        break;
    case ID_toroidalAngleStride:
        {  // new scope
        retval = (toroidalAngleStride == obj.toroidalAngleStride);
        }
        break;
    case ID_timeSampling:
        {  // new scope
        retval = (timeSampling == obj.timeSampling);
        }
        break;
    case ID_timeStepStart:
        {  // new scope
        retval = (timeStepStart == obj.timeStepStart);
        }
        break;
    case ID_timeStepStop:
        {  // new scope
        retval = (timeStepStop == obj.timeStepStop);
        }
        break;
    case ID_timeStepStride:
        {  // new scope
        retval = (timeStepStride == obj.timeStepStride);
        }
        break;
    case ID_channelList:
        {  // new scope
        retval = (channelList == obj.channelList);
        }
        break;
    case ID_wallList:
        {  // new scope
        retval = (wallList == obj.wallList);
        }
        break;
    case ID_nChannelListArrays:
        {  // new scope
        retval = (nChannelListArrays == obj.nChannelListArrays);
        }
        break;
    case ID_channelListToroidalArrayAngle:
        {  // new scope
        retval = (channelListToroidalArrayAngle == obj.channelListToroidalArrayAngle);
        }
        break;
    case ID_channelListToroidalAngle:
        {  // new scope
        retval = (channelListToroidalAngle == obj.channelListToroidalAngle);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

