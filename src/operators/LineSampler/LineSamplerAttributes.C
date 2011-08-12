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

#include <LineSamplerAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for LineSamplerAttributes::CoordinateSystem
//

static const char *CoordinateSystem_strings[] = {
"Cartesian", "Cylindrical"};

std::string
LineSamplerAttributes::CoordinateSystem_ToString(LineSamplerAttributes::CoordinateSystem t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return CoordinateSystem_strings[index];
}

std::string
LineSamplerAttributes::CoordinateSystem_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return CoordinateSystem_strings[index];
}

bool
LineSamplerAttributes::CoordinateSystem_FromString(const std::string &s, LineSamplerAttributes::CoordinateSystem &val)
{
    val = LineSamplerAttributes::Cartesian;
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
// Enum conversion methods for LineSamplerAttributes::ArrayConfiguration
//

static const char *ArrayConfiguration_strings[] = {
"Manual", "List"};

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
    val = LineSamplerAttributes::Manual;
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
// Enum conversion methods for LineSamplerAttributes::ArrayProjection
//

static const char *ArrayProjection_strings[] = {
"Parallel", "Divergent", "Grid"
};

std::string
LineSamplerAttributes::ArrayProjection_ToString(LineSamplerAttributes::ArrayProjection t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ArrayProjection_strings[index];
}

std::string
LineSamplerAttributes::ArrayProjection_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ArrayProjection_strings[index];
}

bool
LineSamplerAttributes::ArrayProjection_FromString(const std::string &s, LineSamplerAttributes::ArrayProjection &val)
{
    val = LineSamplerAttributes::Parallel;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ArrayProjection_strings[i])
        {
            val = (ArrayProjection)i;
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
// Enum conversion methods for LineSamplerAttributes::BeamShape
//

static const char *BeamShape_strings[] = {
"Point", "Line", "Cylinder", 
"Cone"};

std::string
LineSamplerAttributes::BeamShape_ToString(LineSamplerAttributes::BeamShape t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return BeamShape_strings[index];
}

std::string
LineSamplerAttributes::BeamShape_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return BeamShape_strings[index];
}

bool
LineSamplerAttributes::BeamShape_FromString(const std::string &s, LineSamplerAttributes::BeamShape &val)
{
    val = LineSamplerAttributes::Point;
    for(int i = 0; i < 4; ++i)
    {
        if(s == BeamShape_strings[i])
        {
            val = (BeamShape)i;
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
// Enum conversion methods for LineSamplerAttributes::BeamType
//

static const char *BeamType_strings[] = {
"TopHat", "Gaussian"};

std::string
LineSamplerAttributes::BeamType_ToString(LineSamplerAttributes::BeamType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return BeamType_strings[index];
}

std::string
LineSamplerAttributes::BeamType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return BeamType_strings[index];
}

bool
LineSamplerAttributes::BeamType_FromString(const std::string &s, LineSamplerAttributes::BeamType &val)
{
    val = LineSamplerAttributes::TopHat;
    for(int i = 0; i < 2; ++i)
    {
        if(s == BeamType_strings[i])
        {
            val = (BeamType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ChannelSampling
//

static const char *ChannelSampling_strings[] = {
"SingleChannelSampling", "SummationChannelSampling"};

std::string
LineSamplerAttributes::ChannelSampling_ToString(LineSamplerAttributes::ChannelSampling t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ChannelSampling_strings[index];
}

std::string
LineSamplerAttributes::ChannelSampling_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ChannelSampling_strings[index];
}

bool
LineSamplerAttributes::ChannelSampling_FromString(const std::string &s, LineSamplerAttributes::ChannelSampling &val)
{
    val = LineSamplerAttributes::SingleChannelSampling;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ChannelSampling_strings[i])
        {
            val = (ChannelSampling)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for LineSamplerAttributes::ToroidalSampling
//

static const char *ToroidalSampling_strings[] = {
"SingleToroidalSampling", "SummationToroidalSampling"};

std::string
LineSamplerAttributes::ToroidalSampling_ToString(LineSamplerAttributes::ToroidalSampling t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ToroidalSampling_strings[index];
}

std::string
LineSamplerAttributes::ToroidalSampling_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ToroidalSampling_strings[index];
}

bool
LineSamplerAttributes::ToroidalSampling_FromString(const std::string &s, LineSamplerAttributes::ToroidalSampling &val)
{
    val = LineSamplerAttributes::SingleToroidalSampling;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ToroidalSampling_strings[i])
        {
            val = (ToroidalSampling)i;
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
    coordinateSystem = Cylindrical;
    arrayConfiguration = Manual;
    nArrays = 1;
    nChannels = 5;
    toroialArrayAngle = 5;
    arrayProjection = Parallel;
    channelOffset = 0.1;
    channelAngle = 5;
    nRows = 1;
    rowOffset = 0.1;
    arrayOrigin[0] = 0;
    arrayOrigin[1] = 0;
    arrayOrigin[2] = 0;
    arrayAxis = Z;
    poloialAngle = 0;
    poloialRTilt = 0;
    poloialZTilt = 0;
    toroialAngle = 0;
    viewDimension = Three;
    heightPlotScale = 1;
    channelPlotOffset = 1;
    arrayPlotOffset = 1;
    timePlotScale = 1;
    beamShape = Line;
    radius = 0.1;
    divergence = 1;
    beamType = TopHat;
    standardDeviation = 1;
    sampleDistance = 0.1;
    sampleArc = 10;
    channelSampling = SingleChannelSampling;
    toroidalSampling = SingleToroidalSampling;
    toroidalSamplingAngle = 1;
    timeSampling = CurrentTimeStep;
    timeStepStart = 0;
    timeStepStop = 0;
    timeStepStride = 1;
    channelList.push_back(0);
    channelList.push_back(0);
    channelList.push_back(0);
    channelList.push_back(90);

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
    coordinateSystem = obj.coordinateSystem;
    arrayConfiguration = obj.arrayConfiguration;
    nArrays = obj.nArrays;
    nChannels = obj.nChannels;
    toroialArrayAngle = obj.toroialArrayAngle;
    arrayProjection = obj.arrayProjection;
    channelOffset = obj.channelOffset;
    channelAngle = obj.channelAngle;
    nRows = obj.nRows;
    rowOffset = obj.rowOffset;
    arrayOrigin[0] = obj.arrayOrigin[0];
    arrayOrigin[1] = obj.arrayOrigin[1];
    arrayOrigin[2] = obj.arrayOrigin[2];

    arrayAxis = obj.arrayAxis;
    poloialAngle = obj.poloialAngle;
    poloialRTilt = obj.poloialRTilt;
    poloialZTilt = obj.poloialZTilt;
    toroialAngle = obj.toroialAngle;
    viewDimension = obj.viewDimension;
    heightPlotScale = obj.heightPlotScale;
    channelPlotOffset = obj.channelPlotOffset;
    arrayPlotOffset = obj.arrayPlotOffset;
    timePlotScale = obj.timePlotScale;
    beamShape = obj.beamShape;
    radius = obj.radius;
    divergence = obj.divergence;
    beamType = obj.beamType;
    standardDeviation = obj.standardDeviation;
    sampleDistance = obj.sampleDistance;
    sampleArc = obj.sampleArc;
    channelSampling = obj.channelSampling;
    toroidalSampling = obj.toroidalSampling;
    toroidalSamplingAngle = obj.toroidalSamplingAngle;
    timeSampling = obj.timeSampling;
    timeStepStart = obj.timeStepStart;
    timeStepStop = obj.timeStepStop;
    timeStepStride = obj.timeStepStride;
    channelList = obj.channelList;

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
    return ((coordinateSystem == obj.coordinateSystem) &&
            (arrayConfiguration == obj.arrayConfiguration) &&
            (nArrays == obj.nArrays) &&
            (nChannels == obj.nChannels) &&
            (toroialArrayAngle == obj.toroialArrayAngle) &&
            (arrayProjection == obj.arrayProjection) &&
            (channelOffset == obj.channelOffset) &&
            (channelAngle == obj.channelAngle) &&
            (nRows == obj.nRows) &&
            (rowOffset == obj.rowOffset) &&
            arrayOrigin_equal &&
            (arrayAxis == obj.arrayAxis) &&
            (poloialAngle == obj.poloialAngle) &&
            (poloialRTilt == obj.poloialRTilt) &&
            (poloialZTilt == obj.poloialZTilt) &&
            (toroialAngle == obj.toroialAngle) &&
            (viewDimension == obj.viewDimension) &&
            (heightPlotScale == obj.heightPlotScale) &&
            (channelPlotOffset == obj.channelPlotOffset) &&
            (arrayPlotOffset == obj.arrayPlotOffset) &&
            (timePlotScale == obj.timePlotScale) &&
            (beamShape == obj.beamShape) &&
            (radius == obj.radius) &&
            (divergence == obj.divergence) &&
            (beamType == obj.beamType) &&
            (standardDeviation == obj.standardDeviation) &&
            (sampleDistance == obj.sampleDistance) &&
            (sampleArc == obj.sampleArc) &&
            (channelSampling == obj.channelSampling) &&
            (toroidalSampling == obj.toroidalSampling) &&
            (toroidalSamplingAngle == obj.toroidalSamplingAngle) &&
            (timeSampling == obj.timeSampling) &&
            (timeStepStart == obj.timeStepStart) &&
            (timeStepStop == obj.timeStepStop) &&
            (timeStepStride == obj.timeStepStride) &&
            (channelList == obj.channelList));
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
    Select(ID_coordinateSystem,      (void *)&coordinateSystem);
    Select(ID_arrayConfiguration,    (void *)&arrayConfiguration);
    Select(ID_nArrays,               (void *)&nArrays);
    Select(ID_nChannels,             (void *)&nChannels);
    Select(ID_toroialArrayAngle,     (void *)&toroialArrayAngle);
    Select(ID_arrayProjection,       (void *)&arrayProjection);
    Select(ID_channelOffset,         (void *)&channelOffset);
    Select(ID_channelAngle,          (void *)&channelAngle);
    Select(ID_nRows,                 (void *)&nRows);
    Select(ID_rowOffset,             (void *)&rowOffset);
    Select(ID_arrayOrigin,           (void *)arrayOrigin, 3);
    Select(ID_arrayAxis,             (void *)&arrayAxis);
    Select(ID_poloialAngle,          (void *)&poloialAngle);
    Select(ID_poloialRTilt,          (void *)&poloialRTilt);
    Select(ID_poloialZTilt,          (void *)&poloialZTilt);
    Select(ID_toroialAngle,          (void *)&toroialAngle);
    Select(ID_viewDimension,         (void *)&viewDimension);
    Select(ID_heightPlotScale,       (void *)&heightPlotScale);
    Select(ID_channelPlotOffset,     (void *)&channelPlotOffset);
    Select(ID_arrayPlotOffset,       (void *)&arrayPlotOffset);
    Select(ID_timePlotScale,         (void *)&timePlotScale);
    Select(ID_beamShape,             (void *)&beamShape);
    Select(ID_radius,                (void *)&radius);
    Select(ID_divergence,            (void *)&divergence);
    Select(ID_beamType,              (void *)&beamType);
    Select(ID_standardDeviation,     (void *)&standardDeviation);
    Select(ID_sampleDistance,        (void *)&sampleDistance);
    Select(ID_sampleArc,             (void *)&sampleArc);
    Select(ID_channelSampling,       (void *)&channelSampling);
    Select(ID_toroidalSampling,      (void *)&toroidalSampling);
    Select(ID_toroidalSamplingAngle, (void *)&toroidalSamplingAngle);
    Select(ID_timeSampling,          (void *)&timeSampling);
    Select(ID_timeStepStart,         (void *)&timeStepStart);
    Select(ID_timeStepStop,          (void *)&timeStepStop);
    Select(ID_timeStepStride,        (void *)&timeStepStride);
    Select(ID_channelList,           (void *)&channelList);
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

    if(completeSave || !FieldsEqual(ID_coordinateSystem, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("coordinateSystem", CoordinateSystem_ToString(coordinateSystem)));
    }

    if(completeSave || !FieldsEqual(ID_arrayConfiguration, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arrayConfiguration", ArrayConfiguration_ToString(arrayConfiguration)));
    }

    if(completeSave || !FieldsEqual(ID_nArrays, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nArrays", nArrays));
    }

    if(completeSave || !FieldsEqual(ID_nChannels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("nChannels", nChannels));
    }

    if(completeSave || !FieldsEqual(ID_toroialArrayAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroialArrayAngle", toroialArrayAngle));
    }

    if(completeSave || !FieldsEqual(ID_arrayProjection, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arrayProjection", ArrayProjection_ToString(arrayProjection)));
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

    if(completeSave || !FieldsEqual(ID_toroialAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroialAngle", toroialAngle));
    }

    if(completeSave || !FieldsEqual(ID_viewDimension, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("viewDimension", ViewDimension_ToString(viewDimension)));
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

    if(completeSave || !FieldsEqual(ID_timePlotScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timePlotScale", timePlotScale));
    }

    if(completeSave || !FieldsEqual(ID_beamShape, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("beamShape", BeamShape_ToString(beamShape)));
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

    if(completeSave || !FieldsEqual(ID_beamType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("beamType", BeamType_ToString(beamType)));
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

    if(completeSave || !FieldsEqual(ID_sampleArc, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampleArc", sampleArc));
    }

    if(completeSave || !FieldsEqual(ID_channelSampling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("channelSampling", ChannelSampling_ToString(channelSampling)));
    }

    if(completeSave || !FieldsEqual(ID_toroidalSampling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalSampling", ToroidalSampling_ToString(toroidalSampling)));
    }

    if(completeSave || !FieldsEqual(ID_toroidalSamplingAngle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("toroidalSamplingAngle", toroidalSamplingAngle));
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
    if((node = searchNode->GetNode("nArrays")) != 0)
        SetNArrays(node->AsInt());
    if((node = searchNode->GetNode("nChannels")) != 0)
        SetNChannels(node->AsInt());
    if((node = searchNode->GetNode("toroialArrayAngle")) != 0)
        SetToroialArrayAngle(node->AsDouble());
    if((node = searchNode->GetNode("arrayProjection")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetArrayProjection(ArrayProjection(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ArrayProjection value;
            if(ArrayProjection_FromString(node->AsString(), value))
                SetArrayProjection(value);
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
    if((node = searchNode->GetNode("poloialAngle")) != 0)
        SetPoloialAngle(node->AsDouble());
    if((node = searchNode->GetNode("poloialRTilt")) != 0)
        SetPoloialRTilt(node->AsDouble());
    if((node = searchNode->GetNode("poloialZTilt")) != 0)
        SetPoloialZTilt(node->AsDouble());
    if((node = searchNode->GetNode("toroialAngle")) != 0)
        SetToroialAngle(node->AsDouble());
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
    if((node = searchNode->GetNode("heightPlotScale")) != 0)
        SetHeightPlotScale(node->AsDouble());
    if((node = searchNode->GetNode("channelPlotOffset")) != 0)
        SetChannelPlotOffset(node->AsDouble());
    if((node = searchNode->GetNode("arrayPlotOffset")) != 0)
        SetArrayPlotOffset(node->AsDouble());
    if((node = searchNode->GetNode("timePlotScale")) != 0)
        SetTimePlotScale(node->AsDouble());
    if((node = searchNode->GetNode("beamShape")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetBeamShape(BeamShape(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BeamShape value;
            if(BeamShape_FromString(node->AsString(), value))
                SetBeamShape(value);
        }
    }
    if((node = searchNode->GetNode("radius")) != 0)
        SetRadius(node->AsDouble());
    if((node = searchNode->GetNode("divergence")) != 0)
        SetDivergence(node->AsDouble());
    if((node = searchNode->GetNode("beamType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetBeamType(BeamType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BeamType value;
            if(BeamType_FromString(node->AsString(), value))
                SetBeamType(value);
        }
    }
    if((node = searchNode->GetNode("standardDeviation")) != 0)
        SetStandardDeviation(node->AsDouble());
    if((node = searchNode->GetNode("sampleDistance")) != 0)
        SetSampleDistance(node->AsDouble());
    if((node = searchNode->GetNode("sampleArc")) != 0)
        SetSampleArc(node->AsDouble());
    if((node = searchNode->GetNode("channelSampling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetChannelSampling(ChannelSampling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ChannelSampling value;
            if(ChannelSampling_FromString(node->AsString(), value))
                SetChannelSampling(value);
        }
    }
    if((node = searchNode->GetNode("toroidalSampling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetToroidalSampling(ToroidalSampling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ToroidalSampling value;
            if(ToroidalSampling_FromString(node->AsString(), value))
                SetToroidalSampling(value);
        }
    }
    if((node = searchNode->GetNode("toroidalSamplingAngle")) != 0)
        SetToroidalSamplingAngle(node->AsDouble());
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
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
LineSamplerAttributes::SetCoordinateSystem(LineSamplerAttributes::CoordinateSystem coordinateSystem_)
{
    coordinateSystem = coordinateSystem_;
    Select(ID_coordinateSystem, (void *)&coordinateSystem);
}

void
LineSamplerAttributes::SetArrayConfiguration(LineSamplerAttributes::ArrayConfiguration arrayConfiguration_)
{
    arrayConfiguration = arrayConfiguration_;
    Select(ID_arrayConfiguration, (void *)&arrayConfiguration);
}

void
LineSamplerAttributes::SetNArrays(int nArrays_)
{
    nArrays = nArrays_;
    Select(ID_nArrays, (void *)&nArrays);
}

void
LineSamplerAttributes::SetNChannels(int nChannels_)
{
    nChannels = nChannels_;
    Select(ID_nChannels, (void *)&nChannels);
}

void
LineSamplerAttributes::SetToroialArrayAngle(double toroialArrayAngle_)
{
    toroialArrayAngle = toroialArrayAngle_;
    Select(ID_toroialArrayAngle, (void *)&toroialArrayAngle);
}

void
LineSamplerAttributes::SetArrayProjection(LineSamplerAttributes::ArrayProjection arrayProjection_)
{
    arrayProjection = arrayProjection_;
    Select(ID_arrayProjection, (void *)&arrayProjection);
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
LineSamplerAttributes::SetToroialAngle(double toroialAngle_)
{
    toroialAngle = toroialAngle_;
    Select(ID_toroialAngle, (void *)&toroialAngle);
}

void
LineSamplerAttributes::SetViewDimension(LineSamplerAttributes::ViewDimension viewDimension_)
{
    viewDimension = viewDimension_;
    Select(ID_viewDimension, (void *)&viewDimension);
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
LineSamplerAttributes::SetTimePlotScale(double timePlotScale_)
{
    timePlotScale = timePlotScale_;
    Select(ID_timePlotScale, (void *)&timePlotScale);
}

void
LineSamplerAttributes::SetBeamShape(LineSamplerAttributes::BeamShape beamShape_)
{
    beamShape = beamShape_;
    Select(ID_beamShape, (void *)&beamShape);
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
LineSamplerAttributes::SetBeamType(LineSamplerAttributes::BeamType beamType_)
{
    beamType = beamType_;
    Select(ID_beamType, (void *)&beamType);
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
LineSamplerAttributes::SetSampleArc(double sampleArc_)
{
    sampleArc = sampleArc_;
    Select(ID_sampleArc, (void *)&sampleArc);
}

void
LineSamplerAttributes::SetChannelSampling(LineSamplerAttributes::ChannelSampling channelSampling_)
{
    channelSampling = channelSampling_;
    Select(ID_channelSampling, (void *)&channelSampling);
}

void
LineSamplerAttributes::SetToroidalSampling(LineSamplerAttributes::ToroidalSampling toroidalSampling_)
{
    toroidalSampling = toroidalSampling_;
    Select(ID_toroidalSampling, (void *)&toroidalSampling);
}

void
LineSamplerAttributes::SetToroidalSamplingAngle(double toroidalSamplingAngle_)
{
    toroidalSamplingAngle = toroidalSamplingAngle_;
    Select(ID_toroidalSamplingAngle, (void *)&toroidalSamplingAngle);
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

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

LineSamplerAttributes::CoordinateSystem
LineSamplerAttributes::GetCoordinateSystem() const
{
    return CoordinateSystem(coordinateSystem);
}

LineSamplerAttributes::ArrayConfiguration
LineSamplerAttributes::GetArrayConfiguration() const
{
    return ArrayConfiguration(arrayConfiguration);
}

int
LineSamplerAttributes::GetNArrays() const
{
    return nArrays;
}

int
LineSamplerAttributes::GetNChannels() const
{
    return nChannels;
}

double
LineSamplerAttributes::GetToroialArrayAngle() const
{
    return toroialArrayAngle;
}

LineSamplerAttributes::ArrayProjection
LineSamplerAttributes::GetArrayProjection() const
{
    return ArrayProjection(arrayProjection);
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
LineSamplerAttributes::GetToroialAngle() const
{
    return toroialAngle;
}

LineSamplerAttributes::ViewDimension
LineSamplerAttributes::GetViewDimension() const
{
    return ViewDimension(viewDimension);
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

double
LineSamplerAttributes::GetTimePlotScale() const
{
    return timePlotScale;
}

LineSamplerAttributes::BeamShape
LineSamplerAttributes::GetBeamShape() const
{
    return BeamShape(beamShape);
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

LineSamplerAttributes::BeamType
LineSamplerAttributes::GetBeamType() const
{
    return BeamType(beamType);
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
LineSamplerAttributes::GetSampleArc() const
{
    return sampleArc;
}

LineSamplerAttributes::ChannelSampling
LineSamplerAttributes::GetChannelSampling() const
{
    return ChannelSampling(channelSampling);
}

LineSamplerAttributes::ToroidalSampling
LineSamplerAttributes::GetToroidalSampling() const
{
    return ToroidalSampling(toroidalSampling);
}

double
LineSamplerAttributes::GetToroidalSamplingAngle() const
{
    return toroidalSamplingAngle;
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
    case ID_coordinateSystem:      return "coordinateSystem";
    case ID_arrayConfiguration:    return "arrayConfiguration";
    case ID_nArrays:               return "nArrays";
    case ID_nChannels:             return "nChannels";
    case ID_toroialArrayAngle:     return "toroialArrayAngle";
    case ID_arrayProjection:       return "arrayProjection";
    case ID_channelOffset:         return "channelOffset";
    case ID_channelAngle:          return "channelAngle";
    case ID_nRows:                 return "nRows";
    case ID_rowOffset:             return "rowOffset";
    case ID_arrayOrigin:           return "arrayOrigin";
    case ID_arrayAxis:             return "arrayAxis";
    case ID_poloialAngle:          return "poloialAngle";
    case ID_poloialRTilt:          return "poloialRTilt";
    case ID_poloialZTilt:          return "poloialZTilt";
    case ID_toroialAngle:          return "toroialAngle";
    case ID_viewDimension:         return "viewDimension";
    case ID_heightPlotScale:       return "heightPlotScale";
    case ID_channelPlotOffset:     return "channelPlotOffset";
    case ID_arrayPlotOffset:       return "arrayPlotOffset";
    case ID_timePlotScale:         return "timePlotScale";
    case ID_beamShape:             return "beamShape";
    case ID_radius:                return "radius";
    case ID_divergence:            return "divergence";
    case ID_beamType:              return "beamType";
    case ID_standardDeviation:     return "standardDeviation";
    case ID_sampleDistance:        return "sampleDistance";
    case ID_sampleArc:             return "sampleArc";
    case ID_channelSampling:       return "channelSampling";
    case ID_toroidalSampling:      return "toroidalSampling";
    case ID_toroidalSamplingAngle: return "toroidalSamplingAngle";
    case ID_timeSampling:          return "timeSampling";
    case ID_timeStepStart:         return "timeStepStart";
    case ID_timeStepStop:          return "timeStepStop";
    case ID_timeStepStride:        return "timeStepStride";
    case ID_channelList:           return "channelList";
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
    case ID_coordinateSystem:      return FieldType_enum;
    case ID_arrayConfiguration:    return FieldType_enum;
    case ID_nArrays:               return FieldType_int;
    case ID_nChannels:             return FieldType_int;
    case ID_toroialArrayAngle:     return FieldType_double;
    case ID_arrayProjection:       return FieldType_enum;
    case ID_channelOffset:         return FieldType_double;
    case ID_channelAngle:          return FieldType_double;
    case ID_nRows:                 return FieldType_int;
    case ID_rowOffset:             return FieldType_double;
    case ID_arrayOrigin:           return FieldType_doubleArray;
    case ID_arrayAxis:             return FieldType_enum;
    case ID_poloialAngle:          return FieldType_double;
    case ID_poloialRTilt:          return FieldType_double;
    case ID_poloialZTilt:          return FieldType_double;
    case ID_toroialAngle:          return FieldType_double;
    case ID_viewDimension:         return FieldType_enum;
    case ID_heightPlotScale:       return FieldType_double;
    case ID_channelPlotOffset:     return FieldType_double;
    case ID_arrayPlotOffset:       return FieldType_double;
    case ID_timePlotScale:         return FieldType_double;
    case ID_beamShape:             return FieldType_enum;
    case ID_radius:                return FieldType_double;
    case ID_divergence:            return FieldType_double;
    case ID_beamType:              return FieldType_enum;
    case ID_standardDeviation:     return FieldType_double;
    case ID_sampleDistance:        return FieldType_double;
    case ID_sampleArc:             return FieldType_double;
    case ID_channelSampling:       return FieldType_enum;
    case ID_toroidalSampling:      return FieldType_enum;
    case ID_toroidalSamplingAngle: return FieldType_double;
    case ID_timeSampling:          return FieldType_enum;
    case ID_timeStepStart:         return FieldType_int;
    case ID_timeStepStop:          return FieldType_int;
    case ID_timeStepStride:        return FieldType_int;
    case ID_channelList:           return FieldType_doubleVector;
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
    case ID_coordinateSystem:      return "enum";
    case ID_arrayConfiguration:    return "enum";
    case ID_nArrays:               return "int";
    case ID_nChannels:             return "int";
    case ID_toroialArrayAngle:     return "double";
    case ID_arrayProjection:       return "enum";
    case ID_channelOffset:         return "double";
    case ID_channelAngle:          return "double";
    case ID_nRows:                 return "int";
    case ID_rowOffset:             return "double";
    case ID_arrayOrigin:           return "doubleArray";
    case ID_arrayAxis:             return "enum";
    case ID_poloialAngle:          return "double";
    case ID_poloialRTilt:          return "double";
    case ID_poloialZTilt:          return "double";
    case ID_toroialAngle:          return "double";
    case ID_viewDimension:         return "enum";
    case ID_heightPlotScale:       return "double";
    case ID_channelPlotOffset:     return "double";
    case ID_arrayPlotOffset:       return "double";
    case ID_timePlotScale:         return "double";
    case ID_beamShape:             return "enum";
    case ID_radius:                return "double";
    case ID_divergence:            return "double";
    case ID_beamType:              return "enum";
    case ID_standardDeviation:     return "double";
    case ID_sampleDistance:        return "double";
    case ID_sampleArc:             return "double";
    case ID_channelSampling:       return "enum";
    case ID_toroidalSampling:      return "enum";
    case ID_toroidalSamplingAngle: return "double";
    case ID_timeSampling:          return "enum";
    case ID_timeStepStart:         return "int";
    case ID_timeStepStop:          return "int";
    case ID_timeStepStride:        return "int";
    case ID_channelList:           return "doubleVector";
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
    case ID_coordinateSystem:
        {  // new scope
        retval = (coordinateSystem == obj.coordinateSystem);
        }
        break;
    case ID_arrayConfiguration:
        {  // new scope
        retval = (arrayConfiguration == obj.arrayConfiguration);
        }
        break;
    case ID_nArrays:
        {  // new scope
        retval = (nArrays == obj.nArrays);
        }
        break;
    case ID_nChannels:
        {  // new scope
        retval = (nChannels == obj.nChannels);
        }
        break;
    case ID_toroialArrayAngle:
        {  // new scope
        retval = (toroialArrayAngle == obj.toroialArrayAngle);
        }
        break;
    case ID_arrayProjection:
        {  // new scope
        retval = (arrayProjection == obj.arrayProjection);
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
    case ID_toroialAngle:
        {  // new scope
        retval = (toroialAngle == obj.toroialAngle);
        }
        break;
    case ID_viewDimension:
        {  // new scope
        retval = (viewDimension == obj.viewDimension);
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
    case ID_timePlotScale:
        {  // new scope
        retval = (timePlotScale == obj.timePlotScale);
        }
        break;
    case ID_beamShape:
        {  // new scope
        retval = (beamShape == obj.beamShape);
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
    case ID_beamType:
        {  // new scope
        retval = (beamType == obj.beamType);
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
    case ID_sampleArc:
        {  // new scope
        retval = (sampleArc == obj.sampleArc);
        }
        break;
    case ID_channelSampling:
        {  // new scope
        retval = (channelSampling == obj.channelSampling);
        }
        break;
    case ID_toroidalSampling:
        {  // new scope
        retval = (toroidalSampling == obj.toroidalSampling);
        }
        break;
    case ID_toroidalSamplingAngle:
        {  // new scope
        retval = (toroidalSamplingAngle == obj.toroidalSamplingAngle);
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
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

