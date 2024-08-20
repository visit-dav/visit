// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VolumeAttributes.h>
#include <DataNode.h>
#include <ColorControlPoint.h>
#include <GaussianControlPoint.h>

//
// Enum conversion methods for VolumeAttributes::Renderer
//

static const char *Renderer_strings[] = {
"Serial", "Parallel", "Composite",
"Integration", "SLIVR"};

std::string
VolumeAttributes::Renderer_ToString(VolumeAttributes::Renderer t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return Renderer_strings[index];
}

std::string
VolumeAttributes::Renderer_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return Renderer_strings[index];
}

bool
VolumeAttributes::Renderer_FromString(const std::string &s, VolumeAttributes::Renderer &val)
{
    val = VolumeAttributes::Serial;
    for(int i = 0; i < 5; ++i)
    {
        if(s == Renderer_strings[i])
        {
            val = (Renderer)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::ResampleType
//

static const char *ResampleType_strings[] = {
"NoResampling", "OnlyIfRequired", "SingleDomain",
"ParallelRedistribute", "ParallelPerRank"};

std::string
VolumeAttributes::ResampleType_ToString(VolumeAttributes::ResampleType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return ResampleType_strings[index];
}

std::string
VolumeAttributes::ResampleType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return ResampleType_strings[index];
}

bool
VolumeAttributes::ResampleType_FromString(const std::string &s, VolumeAttributes::ResampleType &val)
{
    val = VolumeAttributes::NoResampling;
    for(int i = 0; i < 5; ++i)
    {
        if(s == ResampleType_strings[i])
        {
            val = (ResampleType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::ResampleCentering
//

static const char *ResampleCentering_strings[] = {
"NativeCentering", "NodalCentering", "ZonalCentering"
};

std::string
VolumeAttributes::ResampleCentering_ToString(VolumeAttributes::ResampleCentering t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return ResampleCentering_strings[index];
}

std::string
VolumeAttributes::ResampleCentering_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return ResampleCentering_strings[index];
}

bool
VolumeAttributes::ResampleCentering_FromString(const std::string &s, VolumeAttributes::ResampleCentering &val)
{
    val = VolumeAttributes::NativeCentering;
    for(int i = 0; i < 3; ++i)
    {
        if(s == ResampleCentering_strings[i])
        {
            val = (ResampleCentering)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::GradientType
//

static const char *GradientType_strings[] = {
"CenteredDifferences", "SobelOperator"};

std::string
VolumeAttributes::GradientType_ToString(VolumeAttributes::GradientType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return GradientType_strings[index];
}

std::string
VolumeAttributes::GradientType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return GradientType_strings[index];
}

bool
VolumeAttributes::GradientType_FromString(const std::string &s, VolumeAttributes::GradientType &val)
{
    val = VolumeAttributes::CenteredDifferences;
    for(int i = 0; i < 2; ++i)
    {
        if(s == GradientType_strings[i])
        {
            val = (GradientType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::Scaling
//

static const char *Scaling_strings[] = {
"Linear", "Log", "Skew"
};

std::string
VolumeAttributes::Scaling_ToString(VolumeAttributes::Scaling t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Scaling_strings[index];
}

std::string
VolumeAttributes::Scaling_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Scaling_strings[index];
}

bool
VolumeAttributes::Scaling_FromString(const std::string &s, VolumeAttributes::Scaling &val)
{
    val = VolumeAttributes::Linear;
    for(int i = 0; i < 3; ++i)
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
// Enum conversion methods for VolumeAttributes::LimitsMode
//

static const char *LimitsMode_strings[] = {
"OriginalData", "CurrentPlot"};

std::string
VolumeAttributes::LimitsMode_ToString(VolumeAttributes::LimitsMode t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return LimitsMode_strings[index];
}

std::string
VolumeAttributes::LimitsMode_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return LimitsMode_strings[index];
}

bool
VolumeAttributes::LimitsMode_FromString(const std::string &s, VolumeAttributes::LimitsMode &val)
{
    val = VolumeAttributes::OriginalData;
    for(int i = 0; i < 2; ++i)
    {
        if(s == LimitsMode_strings[i])
        {
            val = (LimitsMode)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::SamplingType
//

static const char *SamplingType_strings[] = {
"KernelBased", "Rasterization", "Trilinear"
};

std::string
VolumeAttributes::SamplingType_ToString(VolumeAttributes::SamplingType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return SamplingType_strings[index];
}

std::string
VolumeAttributes::SamplingType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return SamplingType_strings[index];
}

bool
VolumeAttributes::SamplingType_FromString(const std::string &s, VolumeAttributes::SamplingType &val)
{
    val = VolumeAttributes::KernelBased;
    for(int i = 0; i < 3; ++i)
    {
        if(s == SamplingType_strings[i])
        {
            val = (SamplingType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::OpacityModes
//

static const char *OpacityModes_strings[] = {
"FreeformMode", "GaussianMode", "ColorTableMode"
};

std::string
VolumeAttributes::OpacityModes_ToString(VolumeAttributes::OpacityModes t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return OpacityModes_strings[index];
}

std::string
VolumeAttributes::OpacityModes_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return OpacityModes_strings[index];
}

bool
VolumeAttributes::OpacityModes_FromString(const std::string &s, VolumeAttributes::OpacityModes &val)
{
    val = VolumeAttributes::FreeformMode;
    for(int i = 0; i < 3; ++i)
    {
        if(s == OpacityModes_strings[i])
        {
            val = (OpacityModes)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::LowGradientLightingReduction
//

static const char *LowGradientLightingReduction_strings[] = {
"Off", "Lowest", "Lower",
"Low", "Medium", "High",
"Higher", "Highest"};

std::string
VolumeAttributes::LowGradientLightingReduction_ToString(VolumeAttributes::LowGradientLightingReduction t)
{
    int index = int(t);
    if(index < 0 || index >= 8) index = 0;
    return LowGradientLightingReduction_strings[index];
}

std::string
VolumeAttributes::LowGradientLightingReduction_ToString(int t)
{
    int index = (t < 0 || t >= 8) ? 0 : t;
    return LowGradientLightingReduction_strings[index];
}

bool
VolumeAttributes::LowGradientLightingReduction_FromString(const std::string &s, VolumeAttributes::LowGradientLightingReduction &val)
{
    val = VolumeAttributes::Off;
    for(int i = 0; i < 8; ++i)
    {
        if(s == LowGradientLightingReduction_strings[i])
        {
            val = (LowGradientLightingReduction)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for VolumeAttributes::OSPRayRenderTypes
//

static const char *OSPRayRenderTypes_strings[] = {
"SciVis", "PathTracer"};

std::string
VolumeAttributes::OSPRayRenderTypes_ToString(VolumeAttributes::OSPRayRenderTypes t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OSPRayRenderTypes_strings[index];
}

std::string
VolumeAttributes::OSPRayRenderTypes_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OSPRayRenderTypes_strings[index];
}

bool
VolumeAttributes::OSPRayRenderTypes_FromString(const std::string &s, VolumeAttributes::OSPRayRenderTypes &val)
{
    val = VolumeAttributes::SciVis;
    for(int i = 0; i < 2; ++i)
    {
        if(s == OSPRayRenderTypes_strings[i])
        {
            val = (OSPRayRenderTypes)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: VolumeAttributes::VolumeAttributes
//
// Purpose:
//   Init utility for the VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

void VolumeAttributes::Init()
{
    OSPRayEnabledFlag = false;
    OSPRayRenderType = SciVis;
    OSPRayShadowsEnabledFlag = false;
    OSPRayUseGridAcceleratorFlag = false;
    OSPRayPreIntegrationFlag = false;
    OSPRaySingleShadeFlag = false;
    OSPRayOneSidedLightingFlag = false;
    OSPRayAOTransparencyEnabledFlag = false;
    OSPRaySPP = 1;
    OSPRayAOSamples = 0;
    OSPRayAODistance = 100000;
    OSPRayMinContribution = 0.001;
    OSPRayMaxContribution = 2;
    legendFlag = true;
    lightingFlag = true;
    SetDefaultColorControlPoints();
    opacityAttenuation = 1;
    opacityMode = FreeformMode;
    resampleType = OnlyIfRequired;
    resampleTarget = 1000000;
    resampleCentering = NativeCentering;
    for(int i = 0; i < 256; ++i)
        freeformOpacity[i] = (unsigned char)i;
    useColorVarMin = false;
    colorVarMin = 0;
    useColorVarMax = false;
    colorVarMax = 0;
    useOpacityVarMin = false;
    opacityVarMin = 0;
    useOpacityVarMax = false;
    opacityVarMax = 0;
    smoothData = false;
    samplesPerRay = 500;
    rendererType = Serial;
    gradientType = SobelOperator;
    scaling = Linear;
    skewFactor = 1;
    limitsMode = OriginalData;
    sampling = Rasterization;
    rendererSamples = 3;
    lowGradientLightingReduction = Lower;
    lowGradientLightingClampFlag = false;
    lowGradientLightingClampValue = 1;
    materialProperties[0] = 0.4;
    materialProperties[1] = 0.75;
    materialProperties[2] = 0;
    materialProperties[3] = 15;
    anariRendering = false;
    anariSPP = 1;
    anariAO = 0;
    anariLibrary = "environment";
    anariLibrarySubtype = "default";
    anariRendererSubtype = "default";
    anariLightFalloff = 1;
    anariAmbientIntensity = 1;
    anariMaxDepth = 0;
    anariRValue = 1;
    usdAtCommit = false;
    usdOutputBinary = true;
    usdOutputMaterial = true;
    usdOutputPreviewSurface = true;
    usdOutputMDL = true;
    usdOutputMDLColors = true;
    usdOutputDisplayColors = true;

    VolumeAttributes::SelectAll();
}

// ****************************************************************************
// Method: VolumeAttributes::VolumeAttributes
//
// Purpose:
//   Copy utility for the VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

void VolumeAttributes::Copy(const VolumeAttributes &obj)
{

    OSPRayEnabledFlag = obj.OSPRayEnabledFlag;
    OSPRayRenderType = obj.OSPRayRenderType;
    OSPRayShadowsEnabledFlag = obj.OSPRayShadowsEnabledFlag;
    OSPRayUseGridAcceleratorFlag = obj.OSPRayUseGridAcceleratorFlag;
    OSPRayPreIntegrationFlag = obj.OSPRayPreIntegrationFlag;
    OSPRaySingleShadeFlag = obj.OSPRaySingleShadeFlag;
    OSPRayOneSidedLightingFlag = obj.OSPRayOneSidedLightingFlag;
    OSPRayAOTransparencyEnabledFlag = obj.OSPRayAOTransparencyEnabledFlag;
    OSPRaySPP = obj.OSPRaySPP;
    OSPRayAOSamples = obj.OSPRayAOSamples;
    OSPRayAODistance = obj.OSPRayAODistance;
    OSPRayMinContribution = obj.OSPRayMinContribution;
    OSPRayMaxContribution = obj.OSPRayMaxContribution;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    colorControlPoints = obj.colorControlPoints;
    opacityAttenuation = obj.opacityAttenuation;
    opacityMode = obj.opacityMode;
    opacityControlPoints = obj.opacityControlPoints;
    resampleType = obj.resampleType;
    resampleTarget = obj.resampleTarget;
    resampleCentering = obj.resampleCentering;
    opacityVariable = obj.opacityVariable;
    for(int i = 0; i < 256; ++i)
        freeformOpacity[i] = obj.freeformOpacity[i];

    useColorVarMin = obj.useColorVarMin;
    colorVarMin = obj.colorVarMin;
    useColorVarMax = obj.useColorVarMax;
    colorVarMax = obj.colorVarMax;
    useOpacityVarMin = obj.useOpacityVarMin;
    opacityVarMin = obj.opacityVarMin;
    useOpacityVarMax = obj.useOpacityVarMax;
    opacityVarMax = obj.opacityVarMax;
    smoothData = obj.smoothData;
    samplesPerRay = obj.samplesPerRay;
    rendererType = obj.rendererType;
    gradientType = obj.gradientType;
    scaling = obj.scaling;
    skewFactor = obj.skewFactor;
    limitsMode = obj.limitsMode;
    sampling = obj.sampling;
    rendererSamples = obj.rendererSamples;
    lowGradientLightingReduction = obj.lowGradientLightingReduction;
    lowGradientLightingClampFlag = obj.lowGradientLightingClampFlag;
    lowGradientLightingClampValue = obj.lowGradientLightingClampValue;
    for(int i = 0; i < 4; ++i)
        materialProperties[i] = obj.materialProperties[i];

    anariRendering = obj.anariRendering;
    anariSPP = obj.anariSPP;
    anariAO = obj.anariAO;
    anariLibrary = obj.anariLibrary;
    anariLibrarySubtype = obj.anariLibrarySubtype;
    anariRendererSubtype = obj.anariRendererSubtype;
    anariLightFalloff = obj.anariLightFalloff;
    anariAmbientIntensity = obj.anariAmbientIntensity;
    anariMaxDepth = obj.anariMaxDepth;
    anariRValue = obj.anariRValue;
    usdDir = obj.usdDir;
    usdAtCommit = obj.usdAtCommit;
    usdOutputBinary = obj.usdOutputBinary;
    usdOutputMaterial = obj.usdOutputMaterial;
    usdOutputPreviewSurface = obj.usdOutputPreviewSurface;
    usdOutputMDL = obj.usdOutputMDL;
    usdOutputMDLColors = obj.usdOutputMDLColors;
    usdOutputDisplayColors = obj.usdOutputDisplayColors;

    VolumeAttributes::SelectAll();
}

// Type map format string
const char *VolumeAttributes::TypeMapFormatString = VOLUMEATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t VolumeAttributes::TmfsStruct = {VOLUMEATTRIBUTES_TMFS};


// ****************************************************************************
// Method: VolumeAttributes::VolumeAttributes
//
// Purpose:
//   Default constructor for the VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

VolumeAttributes::VolumeAttributes() :
    AttributeSubject(VolumeAttributes::TypeMapFormatString),
    opacityVariable("default")
{
    VolumeAttributes::Init();
}

// ****************************************************************************
// Method: VolumeAttributes::VolumeAttributes
//
// Purpose:
//   Constructor for the derived classes of VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

VolumeAttributes::VolumeAttributes(private_tmfs_t tmfs) :
    AttributeSubject(tmfs.tmfs),
    opacityVariable("default")
{
    VolumeAttributes::Init();
}

// ****************************************************************************
// Method: VolumeAttributes::VolumeAttributes
//
// Purpose:
//   Copy constructor for the VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

VolumeAttributes::VolumeAttributes(const VolumeAttributes &obj) :
    AttributeSubject(VolumeAttributes::TypeMapFormatString)
{
    VolumeAttributes::Copy(obj);
}

// ****************************************************************************
// Method: VolumeAttributes::VolumeAttributes
//
// Purpose:
//   Copy constructor for derived classes of the VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

VolumeAttributes::VolumeAttributes(const VolumeAttributes &obj, private_tmfs_t tmfs) :
    AttributeSubject(tmfs.tmfs)
{
    VolumeAttributes::Copy(obj);
}

// ****************************************************************************
// Method: VolumeAttributes::~VolumeAttributes
//
// Purpose:
//   Destructor for the VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

VolumeAttributes::~VolumeAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: VolumeAttributes::operator =
//
// Purpose:
//   Assignment operator for the VolumeAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

VolumeAttributes&
VolumeAttributes::operator = (const VolumeAttributes &obj)
{
    if (this == &obj) return *this;

    VolumeAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: VolumeAttributes::operator ==
//
// Purpose:
//   Comparison operator == for the VolumeAttributes class.
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
VolumeAttributes::operator == (const VolumeAttributes &obj) const
{
    // Compare the freeformOpacity arrays.
    bool freeformOpacity_equal = true;
    for(int i = 0; i < 256 && freeformOpacity_equal; ++i)
        freeformOpacity_equal = (freeformOpacity[i] == obj.freeformOpacity[i]);

    // Compare the materialProperties arrays.
    bool materialProperties_equal = true;
    for(int i = 0; i < 4 && materialProperties_equal; ++i)
        materialProperties_equal = (materialProperties[i] == obj.materialProperties[i]);

    // Create the return value
    return ((OSPRayEnabledFlag == obj.OSPRayEnabledFlag) &&
            (OSPRayRenderType == obj.OSPRayRenderType) &&
            (OSPRayShadowsEnabledFlag == obj.OSPRayShadowsEnabledFlag) &&
            (OSPRayUseGridAcceleratorFlag == obj.OSPRayUseGridAcceleratorFlag) &&
            (OSPRayPreIntegrationFlag == obj.OSPRayPreIntegrationFlag) &&
            (OSPRaySingleShadeFlag == obj.OSPRaySingleShadeFlag) &&
            (OSPRayOneSidedLightingFlag == obj.OSPRayOneSidedLightingFlag) &&
            (OSPRayAOTransparencyEnabledFlag == obj.OSPRayAOTransparencyEnabledFlag) &&
            (OSPRaySPP == obj.OSPRaySPP) &&
            (OSPRayAOSamples == obj.OSPRayAOSamples) &&
            (OSPRayAODistance == obj.OSPRayAODistance) &&
            (OSPRayMinContribution == obj.OSPRayMinContribution) &&
            (OSPRayMaxContribution == obj.OSPRayMaxContribution) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (colorControlPoints == obj.colorControlPoints) &&
            (opacityAttenuation == obj.opacityAttenuation) &&
            (opacityMode == obj.opacityMode) &&
            (opacityControlPoints == obj.opacityControlPoints) &&
            (resampleType == obj.resampleType) &&
            (resampleTarget == obj.resampleTarget) &&
            (resampleCentering == obj.resampleCentering) &&
            (opacityVariable == obj.opacityVariable) &&
            freeformOpacity_equal &&
            (useColorVarMin == obj.useColorVarMin) &&
            (colorVarMin == obj.colorVarMin) &&
            (useColorVarMax == obj.useColorVarMax) &&
            (colorVarMax == obj.colorVarMax) &&
            (useOpacityVarMin == obj.useOpacityVarMin) &&
            (opacityVarMin == obj.opacityVarMin) &&
            (useOpacityVarMax == obj.useOpacityVarMax) &&
            (opacityVarMax == obj.opacityVarMax) &&
            (smoothData == obj.smoothData) &&
            (samplesPerRay == obj.samplesPerRay) &&
            (rendererType == obj.rendererType) &&
            (gradientType == obj.gradientType) &&
            (scaling == obj.scaling) &&
            (skewFactor == obj.skewFactor) &&
            (limitsMode == obj.limitsMode) &&
            (sampling == obj.sampling) &&
            (rendererSamples == obj.rendererSamples) &&
            (lowGradientLightingReduction == obj.lowGradientLightingReduction) &&
            (lowGradientLightingClampFlag == obj.lowGradientLightingClampFlag) &&
            (lowGradientLightingClampValue == obj.lowGradientLightingClampValue) &&
            materialProperties_equal &&
            (anariRendering == obj.anariRendering) &&
            (anariSPP == obj.anariSPP) &&
            (anariAO == obj.anariAO) &&
            (anariLibrary == obj.anariLibrary) &&
            (anariLibrarySubtype == obj.anariLibrarySubtype) &&
            (anariRendererSubtype == obj.anariRendererSubtype) &&
            (anariLightFalloff == obj.anariLightFalloff) &&
            (anariAmbientIntensity == obj.anariAmbientIntensity) &&
            (anariMaxDepth == obj.anariMaxDepth) &&
            (anariRValue == obj.anariRValue) &&
            (usdDir == obj.usdDir) &&
            (usdAtCommit == obj.usdAtCommit) &&
            (usdOutputBinary == obj.usdOutputBinary) &&
            (usdOutputMaterial == obj.usdOutputMaterial) &&
            (usdOutputPreviewSurface == obj.usdOutputPreviewSurface) &&
            (usdOutputMDL == obj.usdOutputMDL) &&
            (usdOutputMDLColors == obj.usdOutputMDLColors) &&
            (usdOutputDisplayColors == obj.usdOutputDisplayColors));
}

// ****************************************************************************
// Method: VolumeAttributes::operator !=
//
// Purpose:
//   Comparison operator != for the VolumeAttributes class.
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
VolumeAttributes::operator != (const VolumeAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: VolumeAttributes::TypeName
//
// Purpose:
//   Type name method for the VolumeAttributes class.
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
VolumeAttributes::TypeName() const
{
    return "VolumeAttributes";
}

// ****************************************************************************
// Method: VolumeAttributes::CopyAttributes
//
// Purpose:
//   CopyAttributes method for the VolumeAttributes class.
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
VolumeAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const VolumeAttributes *tmp = (const VolumeAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: VolumeAttributes::CreateCompatible
//
// Purpose:
//   CreateCompatible method for the VolumeAttributes class.
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
VolumeAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new VolumeAttributes(*this);
    // Other cases could go here too.

    return retval;
}

// ****************************************************************************
// Method: VolumeAttributes::NewInstance
//
// Purpose:
//   NewInstance method for the VolumeAttributes class.
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
VolumeAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new VolumeAttributes(*this);
    else
        retval = new VolumeAttributes;

    return retval;
}

// ****************************************************************************
// Method: VolumeAttributes::SelectAll
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
VolumeAttributes::SelectAll()
{
    Select(ID_OSPRayEnabledFlag,               (void *)&OSPRayEnabledFlag);
    Select(ID_OSPRayRenderType,                (void *)&OSPRayRenderType);
    Select(ID_OSPRayShadowsEnabledFlag,        (void *)&OSPRayShadowsEnabledFlag);
    Select(ID_OSPRayUseGridAcceleratorFlag,    (void *)&OSPRayUseGridAcceleratorFlag);
    Select(ID_OSPRayPreIntegrationFlag,        (void *)&OSPRayPreIntegrationFlag);
    Select(ID_OSPRaySingleShadeFlag,           (void *)&OSPRaySingleShadeFlag);
    Select(ID_OSPRayOneSidedLightingFlag,      (void *)&OSPRayOneSidedLightingFlag);
    Select(ID_OSPRayAOTransparencyEnabledFlag, (void *)&OSPRayAOTransparencyEnabledFlag);
    Select(ID_OSPRaySPP,                       (void *)&OSPRaySPP);
    Select(ID_OSPRayAOSamples,                 (void *)&OSPRayAOSamples);
    Select(ID_OSPRayAODistance,                (void *)&OSPRayAODistance);
    Select(ID_OSPRayMinContribution,           (void *)&OSPRayMinContribution);
    Select(ID_OSPRayMaxContribution,           (void *)&OSPRayMaxContribution);
    Select(ID_legendFlag,                      (void *)&legendFlag);
    Select(ID_lightingFlag,                    (void *)&lightingFlag);
    Select(ID_colorControlPoints,              (void *)&colorControlPoints);
    Select(ID_opacityAttenuation,              (void *)&opacityAttenuation);
    Select(ID_opacityMode,                     (void *)&opacityMode);
    Select(ID_opacityControlPoints,            (void *)&opacityControlPoints);
    Select(ID_resampleType,                    (void *)&resampleType);
    Select(ID_resampleTarget,                  (void *)&resampleTarget);
    Select(ID_resampleCentering,               (void *)&resampleCentering);
    Select(ID_opacityVariable,                 (void *)&opacityVariable);
    Select(ID_freeformOpacity,                 (void *)freeformOpacity, 256);
    Select(ID_useColorVarMin,                  (void *)&useColorVarMin);
    Select(ID_colorVarMin,                     (void *)&colorVarMin);
    Select(ID_useColorVarMax,                  (void *)&useColorVarMax);
    Select(ID_colorVarMax,                     (void *)&colorVarMax);
    Select(ID_useOpacityVarMin,                (void *)&useOpacityVarMin);
    Select(ID_opacityVarMin,                   (void *)&opacityVarMin);
    Select(ID_useOpacityVarMax,                (void *)&useOpacityVarMax);
    Select(ID_opacityVarMax,                   (void *)&opacityVarMax);
    Select(ID_smoothData,                      (void *)&smoothData);
    Select(ID_samplesPerRay,                   (void *)&samplesPerRay);
    Select(ID_rendererType,                    (void *)&rendererType);
    Select(ID_gradientType,                    (void *)&gradientType);
    Select(ID_scaling,                         (void *)&scaling);
    Select(ID_skewFactor,                      (void *)&skewFactor);
    Select(ID_limitsMode,                      (void *)&limitsMode);
    Select(ID_sampling,                        (void *)&sampling);
    Select(ID_rendererSamples,                 (void *)&rendererSamples);
    Select(ID_lowGradientLightingReduction,    (void *)&lowGradientLightingReduction);
    Select(ID_lowGradientLightingClampFlag,    (void *)&lowGradientLightingClampFlag);
    Select(ID_lowGradientLightingClampValue,   (void *)&lowGradientLightingClampValue);
    Select(ID_materialProperties,              (void *)materialProperties, 4);
    Select(ID_anariRendering,                  (void *)&anariRendering);
    Select(ID_anariSPP,                        (void *)&anariSPP);
    Select(ID_anariAO,                         (void *)&anariAO);
    Select(ID_anariLibrary,                    (void *)&anariLibrary);
    Select(ID_anariLibrarySubtype,             (void *)&anariLibrarySubtype);
    Select(ID_anariRendererSubtype,            (void *)&anariRendererSubtype);
    Select(ID_anariLightFalloff,               (void *)&anariLightFalloff);
    Select(ID_anariAmbientIntensity,           (void *)&anariAmbientIntensity);
    Select(ID_anariMaxDepth,                   (void *)&anariMaxDepth);
    Select(ID_anariRValue,                     (void *)&anariRValue);
    Select(ID_usdDir,                          (void *)&usdDir);
    Select(ID_usdAtCommit,                     (void *)&usdAtCommit);
    Select(ID_usdOutputBinary,                 (void *)&usdOutputBinary);
    Select(ID_usdOutputMaterial,               (void *)&usdOutputMaterial);
    Select(ID_usdOutputPreviewSurface,         (void *)&usdOutputPreviewSurface);
    Select(ID_usdOutputMDL,                    (void *)&usdOutputMDL);
    Select(ID_usdOutputMDLColors,              (void *)&usdOutputMDLColors);
    Select(ID_usdOutputDisplayColors,          (void *)&usdOutputDisplayColors);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: VolumeAttributes::CreateNode
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
VolumeAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    VolumeAttributes defaultObject;
    bool addToParent = false;
    // Create a node for VolumeAttributes.
    DataNode *node = new DataNode("VolumeAttributes");

    if(completeSave || !FieldsEqual(ID_OSPRayEnabledFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayEnabledFlag", OSPRayEnabledFlag));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayRenderType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayRenderType", OSPRayRenderTypes_ToString(OSPRayRenderType)));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayShadowsEnabledFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayShadowsEnabledFlag", OSPRayShadowsEnabledFlag));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayUseGridAcceleratorFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayUseGridAcceleratorFlag", OSPRayUseGridAcceleratorFlag));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayPreIntegrationFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayPreIntegrationFlag", OSPRayPreIntegrationFlag));
    }

    if(completeSave || !FieldsEqual(ID_OSPRaySingleShadeFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRaySingleShadeFlag", OSPRaySingleShadeFlag));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayOneSidedLightingFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayOneSidedLightingFlag", OSPRayOneSidedLightingFlag));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayAOTransparencyEnabledFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayAOTransparencyEnabledFlag", OSPRayAOTransparencyEnabledFlag));
    }

    if(completeSave || !FieldsEqual(ID_OSPRaySPP, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRaySPP", OSPRaySPP));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayAOSamples, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayAOSamples", OSPRayAOSamples));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayAODistance, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayAODistance", OSPRayAODistance));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayMinContribution, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayMinContribution", OSPRayMinContribution));
    }

    if(completeSave || !FieldsEqual(ID_OSPRayMaxContribution, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("OSPRayMaxContribution", OSPRayMaxContribution));
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

    if(completeSave || !FieldsEqual(ID_colorControlPoints, &defaultObject))
    {
        DataNode *colorControlPointsNode = new DataNode("colorControlPoints");
        if(colorControlPoints.CreateNode(colorControlPointsNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(colorControlPointsNode);
        }
        else
            delete colorControlPointsNode;
    }

    if(completeSave || !FieldsEqual(ID_opacityAttenuation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityAttenuation", opacityAttenuation));
    }

    if(completeSave || !FieldsEqual(ID_opacityMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityMode", OpacityModes_ToString(opacityMode)));
    }

    if(completeSave || !FieldsEqual(ID_opacityControlPoints, &defaultObject))
    {
        DataNode *opacityControlPointsNode = new DataNode("opacityControlPoints");
        if(opacityControlPoints.CreateNode(opacityControlPointsNode, completeSave, false))
        {
            addToParent = true;
            node->AddNode(opacityControlPointsNode);
        }
        else
            delete opacityControlPointsNode;
    }

    if(completeSave || !FieldsEqual(ID_resampleType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("resampleType", ResampleType_ToString(resampleType)));
    }

    if(completeSave || !FieldsEqual(ID_resampleTarget, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("resampleTarget", resampleTarget));
    }

    if(completeSave || !FieldsEqual(ID_resampleCentering, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("resampleCentering", ResampleCentering_ToString(resampleCentering)));
    }

    if(completeSave || !FieldsEqual(ID_opacityVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVariable", opacityVariable));
    }

    if(completeSave || !FieldsEqual(ID_freeformOpacity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("freeformOpacity", freeformOpacity, 256));
    }

    if(completeSave || !FieldsEqual(ID_useColorVarMin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useColorVarMin", useColorVarMin));
    }

    if(completeSave || !FieldsEqual(ID_colorVarMin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorVarMin", colorVarMin));
    }

    if(completeSave || !FieldsEqual(ID_useColorVarMax, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useColorVarMax", useColorVarMax));
    }

    if(completeSave || !FieldsEqual(ID_colorVarMax, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorVarMax", colorVarMax));
    }

    if(completeSave || !FieldsEqual(ID_useOpacityVarMin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useOpacityVarMin", useOpacityVarMin));
    }

    if(completeSave || !FieldsEqual(ID_opacityVarMin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVarMin", opacityVarMin));
    }

    if(completeSave || !FieldsEqual(ID_useOpacityVarMax, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useOpacityVarMax", useOpacityVarMax));
    }

    if(completeSave || !FieldsEqual(ID_opacityVarMax, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("opacityVarMax", opacityVarMax));
    }

    if(completeSave || !FieldsEqual(ID_smoothData, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("smoothData", smoothData));
    }

    if(completeSave || !FieldsEqual(ID_samplesPerRay, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("samplesPerRay", samplesPerRay));
    }

    if(completeSave || !FieldsEqual(ID_rendererType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rendererType", Renderer_ToString(rendererType)));
    }

    if(completeSave || !FieldsEqual(ID_gradientType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("gradientType", GradientType_ToString(gradientType)));
    }

    if(completeSave || !FieldsEqual(ID_scaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("scaling", Scaling_ToString(scaling)));
    }

    if(completeSave || !FieldsEqual(ID_skewFactor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("skewFactor", skewFactor));
    }

    if(completeSave || !FieldsEqual(ID_limitsMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("limitsMode", LimitsMode_ToString(limitsMode)));
    }

    if(completeSave || !FieldsEqual(ID_sampling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("sampling", SamplingType_ToString(sampling)));
    }

    if(completeSave || !FieldsEqual(ID_rendererSamples, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rendererSamples", rendererSamples));
    }

    if(completeSave || !FieldsEqual(ID_lowGradientLightingReduction, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lowGradientLightingReduction", LowGradientLightingReduction_ToString(lowGradientLightingReduction)));
    }

    if(completeSave || !FieldsEqual(ID_lowGradientLightingClampFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lowGradientLightingClampFlag", lowGradientLightingClampFlag));
    }

    if(completeSave || !FieldsEqual(ID_lowGradientLightingClampValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lowGradientLightingClampValue", lowGradientLightingClampValue));
    }

    if(completeSave || !FieldsEqual(ID_materialProperties, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("materialProperties", materialProperties, 4));
    }

    if(completeSave || !FieldsEqual(ID_anariRendering, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariRendering", anariRendering));
    }

    if(completeSave || !FieldsEqual(ID_anariSPP, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariSPP", anariSPP));
    }

    if(completeSave || !FieldsEqual(ID_anariAO, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariAO", anariAO));
    }

    if(completeSave || !FieldsEqual(ID_anariLibrary, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariLibrary", anariLibrary));
    }

    if(completeSave || !FieldsEqual(ID_anariLibrarySubtype, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariLibrarySubtype", anariLibrarySubtype));
    }

    if(completeSave || !FieldsEqual(ID_anariRendererSubtype, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariRendererSubtype", anariRendererSubtype));
    }

    if(completeSave || !FieldsEqual(ID_anariLightFalloff, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariLightFalloff", anariLightFalloff));
    }

    if(completeSave || !FieldsEqual(ID_anariAmbientIntensity, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariAmbientIntensity", anariAmbientIntensity));
    }

    if(completeSave || !FieldsEqual(ID_anariMaxDepth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariMaxDepth", anariMaxDepth));
    }

    if(completeSave || !FieldsEqual(ID_anariRValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("anariRValue", anariRValue));
    }

    if(completeSave || !FieldsEqual(ID_usdDir, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdDir", usdDir));
    }

    if(completeSave || !FieldsEqual(ID_usdAtCommit, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdAtCommit", usdAtCommit));
    }

    if(completeSave || !FieldsEqual(ID_usdOutputBinary, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdOutputBinary", usdOutputBinary));
    }

    if(completeSave || !FieldsEqual(ID_usdOutputMaterial, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdOutputMaterial", usdOutputMaterial));
    }

    if(completeSave || !FieldsEqual(ID_usdOutputPreviewSurface, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdOutputPreviewSurface", usdOutputPreviewSurface));
    }

    if(completeSave || !FieldsEqual(ID_usdOutputMDL, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdOutputMDL", usdOutputMDL));
    }

    if(completeSave || !FieldsEqual(ID_usdOutputMDLColors, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdOutputMDLColors", usdOutputMDLColors));
    }

    if(completeSave || !FieldsEqual(ID_usdOutputDisplayColors, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("usdOutputDisplayColors", usdOutputDisplayColors));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: VolumeAttributes::SetFromNode
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
VolumeAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("VolumeAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("OSPRayEnabledFlag")) != 0)
        SetOSPRayEnabledFlag(node->AsBool());
    if((node = searchNode->GetNode("OSPRayRenderType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetOSPRayRenderType(OSPRayRenderTypes(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OSPRayRenderTypes value;
            if(OSPRayRenderTypes_FromString(node->AsString(), value))
                SetOSPRayRenderType(value);
        }
    }
    if((node = searchNode->GetNode("OSPRayShadowsEnabledFlag")) != 0)
        SetOSPRayShadowsEnabledFlag(node->AsBool());
    if((node = searchNode->GetNode("OSPRayUseGridAcceleratorFlag")) != 0)
        SetOSPRayUseGridAcceleratorFlag(node->AsBool());
    if((node = searchNode->GetNode("OSPRayPreIntegrationFlag")) != 0)
        SetOSPRayPreIntegrationFlag(node->AsBool());
    if((node = searchNode->GetNode("OSPRaySingleShadeFlag")) != 0)
        SetOSPRaySingleShadeFlag(node->AsBool());
    if((node = searchNode->GetNode("OSPRayOneSidedLightingFlag")) != 0)
        SetOSPRayOneSidedLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("OSPRayAOTransparencyEnabledFlag")) != 0)
        SetOSPRayAOTransparencyEnabledFlag(node->AsBool());
    if((node = searchNode->GetNode("OSPRaySPP")) != 0)
        SetOSPRaySPP(node->AsInt());
    if((node = searchNode->GetNode("OSPRayAOSamples")) != 0)
        SetOSPRayAOSamples(node->AsInt());
    if((node = searchNode->GetNode("OSPRayAODistance")) != 0)
        SetOSPRayAODistance(node->AsDouble());
    if((node = searchNode->GetNode("OSPRayMinContribution")) != 0)
        SetOSPRayMinContribution(node->AsDouble());
    if((node = searchNode->GetNode("OSPRayMaxContribution")) != 0)
        SetOSPRayMaxContribution(node->AsDouble());
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("colorControlPoints")) != 0)
        colorControlPoints.SetFromNode(node);
    if((node = searchNode->GetNode("opacityAttenuation")) != 0)
        SetOpacityAttenuation(node->AsFloat());
    if((node = searchNode->GetNode("opacityMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetOpacityMode(OpacityModes(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OpacityModes value;
            if(OpacityModes_FromString(node->AsString(), value))
                SetOpacityMode(value);
        }
    }
    if((node = searchNode->GetNode("opacityControlPoints")) != 0)
        opacityControlPoints.SetFromNode(node);
    if((node = searchNode->GetNode("resampleType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetResampleType(ResampleType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ResampleType value;
            if(ResampleType_FromString(node->AsString(), value))
                SetResampleType(value);
        }
    }
    if((node = searchNode->GetNode("resampleTarget")) != 0)
        SetResampleTarget(node->AsInt());
    if((node = searchNode->GetNode("resampleCentering")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetResampleCentering(ResampleCentering(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ResampleCentering value;
            if(ResampleCentering_FromString(node->AsString(), value))
                SetResampleCentering(value);
        }
    }
    if((node = searchNode->GetNode("opacityVariable")) != 0)
        SetOpacityVariable(node->AsString());
    if((node = searchNode->GetNode("freeformOpacity")) != 0)
        SetFreeformOpacity(node->AsUnsignedCharArray());
    if((node = searchNode->GetNode("useColorVarMin")) != 0)
        SetUseColorVarMin(node->AsBool());
    if((node = searchNode->GetNode("colorVarMin")) != 0)
        SetColorVarMin(node->AsFloat());
    if((node = searchNode->GetNode("useColorVarMax")) != 0)
        SetUseColorVarMax(node->AsBool());
    if((node = searchNode->GetNode("colorVarMax")) != 0)
        SetColorVarMax(node->AsFloat());
    if((node = searchNode->GetNode("useOpacityVarMin")) != 0)
        SetUseOpacityVarMin(node->AsBool());
    if((node = searchNode->GetNode("opacityVarMin")) != 0)
        SetOpacityVarMin(node->AsFloat());
    if((node = searchNode->GetNode("useOpacityVarMax")) != 0)
        SetUseOpacityVarMax(node->AsBool());
    if((node = searchNode->GetNode("opacityVarMax")) != 0)
        SetOpacityVarMax(node->AsFloat());
    if((node = searchNode->GetNode("smoothData")) != 0)
        SetSmoothData(node->AsBool());
    if((node = searchNode->GetNode("samplesPerRay")) != 0)
        SetSamplesPerRay(node->AsInt());
    if((node = searchNode->GetNode("rendererType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
                SetRendererType(Renderer(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Renderer value;
            if(Renderer_FromString(node->AsString(), value))
                SetRendererType(value);
        }
    }
    if((node = searchNode->GetNode("gradientType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetGradientType(GradientType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            GradientType value;
            if(GradientType_FromString(node->AsString(), value))
                SetGradientType(value);
        }
    }
    if((node = searchNode->GetNode("scaling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetScaling(Scaling(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Scaling value;
            if(Scaling_FromString(node->AsString(), value))
                SetScaling(value);
        }
    }
    if((node = searchNode->GetNode("skewFactor")) != 0)
        SetSkewFactor(node->AsDouble());
    if((node = searchNode->GetNode("limitsMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetLimitsMode(LimitsMode(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LimitsMode value;
            if(LimitsMode_FromString(node->AsString(), value))
                SetLimitsMode(value);
        }
    }
    if((node = searchNode->GetNode("sampling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetSampling(SamplingType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SamplingType value;
            if(SamplingType_FromString(node->AsString(), value))
                SetSampling(value);
        }
    }
    if((node = searchNode->GetNode("rendererSamples")) != 0)
        SetRendererSamples(node->AsFloat());
    if((node = searchNode->GetNode("lowGradientLightingReduction")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 8)
                SetLowGradientLightingReduction(LowGradientLightingReduction(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LowGradientLightingReduction value;
            if(LowGradientLightingReduction_FromString(node->AsString(), value))
                SetLowGradientLightingReduction(value);
        }
    }
    if((node = searchNode->GetNode("lowGradientLightingClampFlag")) != 0)
        SetLowGradientLightingClampFlag(node->AsBool());
    if((node = searchNode->GetNode("lowGradientLightingClampValue")) != 0)
        SetLowGradientLightingClampValue(node->AsDouble());
    if((node = searchNode->GetNode("materialProperties")) != 0)
        SetMaterialProperties(node->AsDoubleArray());
    if((node = searchNode->GetNode("anariRendering")) != 0)
        SetAnariRendering(node->AsBool());
    if((node = searchNode->GetNode("anariSPP")) != 0)
        SetAnariSPP(node->AsInt());
    if((node = searchNode->GetNode("anariAO")) != 0)
        SetAnariAO(node->AsInt());
    if((node = searchNode->GetNode("anariLibrary")) != 0)
        SetAnariLibrary(node->AsString());
    if((node = searchNode->GetNode("anariLibrarySubtype")) != 0)
        SetAnariLibrarySubtype(node->AsString());
    if((node = searchNode->GetNode("anariRendererSubtype")) != 0)
        SetAnariRendererSubtype(node->AsString());
    if((node = searchNode->GetNode("anariLightFalloff")) != 0)
        SetAnariLightFalloff(node->AsFloat());
    if((node = searchNode->GetNode("anariAmbientIntensity")) != 0)
        SetAnariAmbientIntensity(node->AsFloat());
    if((node = searchNode->GetNode("anariMaxDepth")) != 0)
        SetAnariMaxDepth(node->AsInt());
    if((node = searchNode->GetNode("anariRValue")) != 0)
        SetAnariRValue(node->AsFloat());
    if((node = searchNode->GetNode("usdDir")) != 0)
        SetUsdDir(node->AsString());
    if((node = searchNode->GetNode("usdAtCommit")) != 0)
        SetUsdAtCommit(node->AsBool());
    if((node = searchNode->GetNode("usdOutputBinary")) != 0)
        SetUsdOutputBinary(node->AsBool());
    if((node = searchNode->GetNode("usdOutputMaterial")) != 0)
        SetUsdOutputMaterial(node->AsBool());
    if((node = searchNode->GetNode("usdOutputPreviewSurface")) != 0)
        SetUsdOutputPreviewSurface(node->AsBool());
    if((node = searchNode->GetNode("usdOutputMDL")) != 0)
        SetUsdOutputMDL(node->AsBool());
    if((node = searchNode->GetNode("usdOutputMDLColors")) != 0)
        SetUsdOutputMDLColors(node->AsBool());
    if((node = searchNode->GetNode("usdOutputDisplayColors")) != 0)
        SetUsdOutputDisplayColors(node->AsBool());
    if(colorControlPoints.GetNumControlPoints() < 2)
         SetDefaultColorControlPoints();

}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
VolumeAttributes::SetOSPRayEnabledFlag(bool OSPRayEnabledFlag_)
{
    OSPRayEnabledFlag = OSPRayEnabledFlag_;
    Select(ID_OSPRayEnabledFlag, (void *)&OSPRayEnabledFlag);
}

void
VolumeAttributes::SetOSPRayRenderType(VolumeAttributes::OSPRayRenderTypes OSPRayRenderType_)
{
    OSPRayRenderType = OSPRayRenderType_;
    Select(ID_OSPRayRenderType, (void *)&OSPRayRenderType);
}

void
VolumeAttributes::SetOSPRayShadowsEnabledFlag(bool OSPRayShadowsEnabledFlag_)
{
    OSPRayShadowsEnabledFlag = OSPRayShadowsEnabledFlag_;
    Select(ID_OSPRayShadowsEnabledFlag, (void *)&OSPRayShadowsEnabledFlag);
}

void
VolumeAttributes::SetOSPRayUseGridAcceleratorFlag(bool OSPRayUseGridAcceleratorFlag_)
{
    OSPRayUseGridAcceleratorFlag = OSPRayUseGridAcceleratorFlag_;
    Select(ID_OSPRayUseGridAcceleratorFlag, (void *)&OSPRayUseGridAcceleratorFlag);
}

void
VolumeAttributes::SetOSPRayPreIntegrationFlag(bool OSPRayPreIntegrationFlag_)
{
    OSPRayPreIntegrationFlag = OSPRayPreIntegrationFlag_;
    Select(ID_OSPRayPreIntegrationFlag, (void *)&OSPRayPreIntegrationFlag);
}

void
VolumeAttributes::SetOSPRaySingleShadeFlag(bool OSPRaySingleShadeFlag_)
{
    OSPRaySingleShadeFlag = OSPRaySingleShadeFlag_;
    Select(ID_OSPRaySingleShadeFlag, (void *)&OSPRaySingleShadeFlag);
}

void
VolumeAttributes::SetOSPRayOneSidedLightingFlag(bool OSPRayOneSidedLightingFlag_)
{
    OSPRayOneSidedLightingFlag = OSPRayOneSidedLightingFlag_;
    Select(ID_OSPRayOneSidedLightingFlag, (void *)&OSPRayOneSidedLightingFlag);
}

void
VolumeAttributes::SetOSPRayAOTransparencyEnabledFlag(bool OSPRayAOTransparencyEnabledFlag_)
{
    OSPRayAOTransparencyEnabledFlag = OSPRayAOTransparencyEnabledFlag_;
    Select(ID_OSPRayAOTransparencyEnabledFlag, (void *)&OSPRayAOTransparencyEnabledFlag);
}

void
VolumeAttributes::SetOSPRaySPP(int OSPRaySPP_)
{
    OSPRaySPP = OSPRaySPP_;
    Select(ID_OSPRaySPP, (void *)&OSPRaySPP);
}

void
VolumeAttributes::SetOSPRayAOSamples(int OSPRayAOSamples_)
{
    OSPRayAOSamples = OSPRayAOSamples_;
    Select(ID_OSPRayAOSamples, (void *)&OSPRayAOSamples);
}

void
VolumeAttributes::SetOSPRayAODistance(double OSPRayAODistance_)
{
    OSPRayAODistance = OSPRayAODistance_;
    Select(ID_OSPRayAODistance, (void *)&OSPRayAODistance);
}

void
VolumeAttributes::SetOSPRayMinContribution(double OSPRayMinContribution_)
{
    OSPRayMinContribution = OSPRayMinContribution_;
    Select(ID_OSPRayMinContribution, (void *)&OSPRayMinContribution);
}

void
VolumeAttributes::SetOSPRayMaxContribution(double OSPRayMaxContribution_)
{
    OSPRayMaxContribution = OSPRayMaxContribution_;
    Select(ID_OSPRayMaxContribution, (void *)&OSPRayMaxContribution);
}

void
VolumeAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
VolumeAttributes::SetLightingFlag(bool lightingFlag_)
{
    lightingFlag = lightingFlag_;
    Select(ID_lightingFlag, (void *)&lightingFlag);
}

void
VolumeAttributes::SetColorControlPoints(const ColorControlPointList &colorControlPoints_)
{
    colorControlPoints = colorControlPoints_;
    Select(ID_colorControlPoints, (void *)&colorControlPoints);
}

void
VolumeAttributes::SetOpacityAttenuation(float opacityAttenuation_)
{
    opacityAttenuation = opacityAttenuation_;
    Select(ID_opacityAttenuation, (void *)&opacityAttenuation);
}

void
VolumeAttributes::SetOpacityMode(VolumeAttributes::OpacityModes opacityMode_)
{
    opacityMode = opacityMode_;
    Select(ID_opacityMode, (void *)&opacityMode);
}

void
VolumeAttributes::SetOpacityControlPoints(const GaussianControlPointList &opacityControlPoints_)
{
    opacityControlPoints = opacityControlPoints_;
    Select(ID_opacityControlPoints, (void *)&opacityControlPoints);
}

void
VolumeAttributes::SetResampleType(VolumeAttributes::ResampleType resampleType_)
{
    resampleType = resampleType_;
    Select(ID_resampleType, (void *)&resampleType);
}

void
VolumeAttributes::SetResampleTarget(int resampleTarget_)
{
    resampleTarget = resampleTarget_;
    Select(ID_resampleTarget, (void *)&resampleTarget);
}

void
VolumeAttributes::SetResampleCentering(VolumeAttributes::ResampleCentering resampleCentering_)
{
    resampleCentering = resampleCentering_;
    Select(ID_resampleCentering, (void *)&resampleCentering);
}

void
VolumeAttributes::SetOpacityVariable(const std::string &opacityVariable_)
{
    opacityVariable = opacityVariable_;
    Select(ID_opacityVariable, (void *)&opacityVariable);
}

void
VolumeAttributes::SetFreeformOpacity(const unsigned char *freeformOpacity_)
{
    for(int i = 0; i < 256; ++i)
        freeformOpacity[i] = freeformOpacity_[i];
    Select(ID_freeformOpacity, (void *)freeformOpacity, 256);
}

void
VolumeAttributes::SetUseColorVarMin(bool useColorVarMin_)
{
    useColorVarMin = useColorVarMin_;
    Select(ID_useColorVarMin, (void *)&useColorVarMin);
}

void
VolumeAttributes::SetColorVarMin(float colorVarMin_)
{
    colorVarMin = colorVarMin_;
    Select(ID_colorVarMin, (void *)&colorVarMin);
}

void
VolumeAttributes::SetUseColorVarMax(bool useColorVarMax_)
{
    useColorVarMax = useColorVarMax_;
    Select(ID_useColorVarMax, (void *)&useColorVarMax);
}

void
VolumeAttributes::SetColorVarMax(float colorVarMax_)
{
    colorVarMax = colorVarMax_;
    Select(ID_colorVarMax, (void *)&colorVarMax);
}

void
VolumeAttributes::SetUseOpacityVarMin(bool useOpacityVarMin_)
{
    useOpacityVarMin = useOpacityVarMin_;
    Select(ID_useOpacityVarMin, (void *)&useOpacityVarMin);
}

void
VolumeAttributes::SetOpacityVarMin(float opacityVarMin_)
{
    opacityVarMin = opacityVarMin_;
    Select(ID_opacityVarMin, (void *)&opacityVarMin);
}

void
VolumeAttributes::SetUseOpacityVarMax(bool useOpacityVarMax_)
{
    useOpacityVarMax = useOpacityVarMax_;
    Select(ID_useOpacityVarMax, (void *)&useOpacityVarMax);
}

void
VolumeAttributes::SetOpacityVarMax(float opacityVarMax_)
{
    opacityVarMax = opacityVarMax_;
    Select(ID_opacityVarMax, (void *)&opacityVarMax);
}

void
VolumeAttributes::SetSmoothData(bool smoothData_)
{
    smoothData = smoothData_;
    Select(ID_smoothData, (void *)&smoothData);
}

void
VolumeAttributes::SetSamplesPerRay(int samplesPerRay_)
{
    samplesPerRay = samplesPerRay_;
    Select(ID_samplesPerRay, (void *)&samplesPerRay);
}

void
VolumeAttributes::SetRendererType(VolumeAttributes::Renderer rendererType_)
{
    rendererType = rendererType_;
    Select(ID_rendererType, (void *)&rendererType);
}

void
VolumeAttributes::SetGradientType(VolumeAttributes::GradientType gradientType_)
{
    gradientType = gradientType_;
    Select(ID_gradientType, (void *)&gradientType);
}

void
VolumeAttributes::SetScaling(VolumeAttributes::Scaling scaling_)
{
    scaling = scaling_;
    Select(ID_scaling, (void *)&scaling);
}

void
VolumeAttributes::SetSkewFactor(double skewFactor_)
{
    skewFactor = skewFactor_;
    Select(ID_skewFactor, (void *)&skewFactor);
}

void
VolumeAttributes::SetLimitsMode(VolumeAttributes::LimitsMode limitsMode_)
{
    limitsMode = limitsMode_;
    Select(ID_limitsMode, (void *)&limitsMode);
}

void
VolumeAttributes::SetSampling(VolumeAttributes::SamplingType sampling_)
{
    sampling = sampling_;
    Select(ID_sampling, (void *)&sampling);
}

void
VolumeAttributes::SetRendererSamples(float rendererSamples_)
{
    rendererSamples = rendererSamples_;
    Select(ID_rendererSamples, (void *)&rendererSamples);
}

void
VolumeAttributes::SetLowGradientLightingReduction(VolumeAttributes::LowGradientLightingReduction lowGradientLightingReduction_)
{
    lowGradientLightingReduction = lowGradientLightingReduction_;
    Select(ID_lowGradientLightingReduction, (void *)&lowGradientLightingReduction);
}

void
VolumeAttributes::SetLowGradientLightingClampFlag(bool lowGradientLightingClampFlag_)
{
    lowGradientLightingClampFlag = lowGradientLightingClampFlag_;
    Select(ID_lowGradientLightingClampFlag, (void *)&lowGradientLightingClampFlag);
}

void
VolumeAttributes::SetLowGradientLightingClampValue(double lowGradientLightingClampValue_)
{
    lowGradientLightingClampValue = lowGradientLightingClampValue_;
    Select(ID_lowGradientLightingClampValue, (void *)&lowGradientLightingClampValue);
}

void
VolumeAttributes::SetMaterialProperties(const double *materialProperties_)
{
    materialProperties[0] = materialProperties_[0];
    materialProperties[1] = materialProperties_[1];
    materialProperties[2] = materialProperties_[2];
    materialProperties[3] = materialProperties_[3];
    Select(ID_materialProperties, (void *)materialProperties, 4);
}

void
VolumeAttributes::SetAnariRendering(bool anariRendering_)
{
    anariRendering = anariRendering_;
    Select(ID_anariRendering, (void *)&anariRendering);
}

void
VolumeAttributes::SetAnariSPP(int anariSPP_)
{
    anariSPP = anariSPP_;
    Select(ID_anariSPP, (void *)&anariSPP);
}

void
VolumeAttributes::SetAnariAO(int anariAO_)
{
    anariAO = anariAO_;
    Select(ID_anariAO, (void *)&anariAO);
}

void
VolumeAttributes::SetAnariLibrary(const std::string &anariLibrary_)
{
    anariLibrary = anariLibrary_;
    Select(ID_anariLibrary, (void *)&anariLibrary);
}

void
VolumeAttributes::SetAnariLibrarySubtype(const std::string &anariLibrarySubtype_)
{
    anariLibrarySubtype = anariLibrarySubtype_;
    Select(ID_anariLibrarySubtype, (void *)&anariLibrarySubtype);
}

void
VolumeAttributes::SetAnariRendererSubtype(const std::string &anariRendererSubtype_)
{
    anariRendererSubtype = anariRendererSubtype_;
    Select(ID_anariRendererSubtype, (void *)&anariRendererSubtype);
}

void
VolumeAttributes::SetAnariLightFalloff(float anariLightFalloff_)
{
    anariLightFalloff = anariLightFalloff_;
    Select(ID_anariLightFalloff, (void *)&anariLightFalloff);
}

void
VolumeAttributes::SetAnariAmbientIntensity(float anariAmbientIntensity_)
{
    anariAmbientIntensity = anariAmbientIntensity_;
    Select(ID_anariAmbientIntensity, (void *)&anariAmbientIntensity);
}

void
VolumeAttributes::SetAnariMaxDepth(int anariMaxDepth_)
{
    anariMaxDepth = anariMaxDepth_;
    Select(ID_anariMaxDepth, (void *)&anariMaxDepth);
}

void
VolumeAttributes::SetAnariRValue(float anariRValue_)
{
    anariRValue = anariRValue_;
    Select(ID_anariRValue, (void *)&anariRValue);
}

void
VolumeAttributes::SetUsdDir(const std::string &usdDir_)
{
    usdDir = usdDir_;
    Select(ID_usdDir, (void *)&usdDir);
}

void
VolumeAttributes::SetUsdAtCommit(bool usdAtCommit_)
{
    usdAtCommit = usdAtCommit_;
    Select(ID_usdAtCommit, (void *)&usdAtCommit);
}

void
VolumeAttributes::SetUsdOutputBinary(bool usdOutputBinary_)
{
    usdOutputBinary = usdOutputBinary_;
    Select(ID_usdOutputBinary, (void *)&usdOutputBinary);
}

void
VolumeAttributes::SetUsdOutputMaterial(bool usdOutputMaterial_)
{
    usdOutputMaterial = usdOutputMaterial_;
    Select(ID_usdOutputMaterial, (void *)&usdOutputMaterial);
}

void
VolumeAttributes::SetUsdOutputPreviewSurface(bool usdOutputPreviewSurface_)
{
    usdOutputPreviewSurface = usdOutputPreviewSurface_;
    Select(ID_usdOutputPreviewSurface, (void *)&usdOutputPreviewSurface);
}

void
VolumeAttributes::SetUsdOutputMDL(bool usdOutputMDL_)
{
    usdOutputMDL = usdOutputMDL_;
    Select(ID_usdOutputMDL, (void *)&usdOutputMDL);
}

void
VolumeAttributes::SetUsdOutputMDLColors(bool usdOutputMDLColors_)
{
    usdOutputMDLColors = usdOutputMDLColors_;
    Select(ID_usdOutputMDLColors, (void *)&usdOutputMDLColors);
}

void
VolumeAttributes::SetUsdOutputDisplayColors(bool usdOutputDisplayColors_)
{
    usdOutputDisplayColors = usdOutputDisplayColors_;
    Select(ID_usdOutputDisplayColors, (void *)&usdOutputDisplayColors);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
VolumeAttributes::GetOSPRayEnabledFlag() const
{
    return OSPRayEnabledFlag;
}

VolumeAttributes::OSPRayRenderTypes
VolumeAttributes::GetOSPRayRenderType() const
{
    return OSPRayRenderTypes(OSPRayRenderType);
}

bool
VolumeAttributes::GetOSPRayShadowsEnabledFlag() const
{
    return OSPRayShadowsEnabledFlag;
}

bool
VolumeAttributes::GetOSPRayUseGridAcceleratorFlag() const
{
    return OSPRayUseGridAcceleratorFlag;
}

bool
VolumeAttributes::GetOSPRayPreIntegrationFlag() const
{
    return OSPRayPreIntegrationFlag;
}

bool
VolumeAttributes::GetOSPRaySingleShadeFlag() const
{
    return OSPRaySingleShadeFlag;
}

bool
VolumeAttributes::GetOSPRayOneSidedLightingFlag() const
{
    return OSPRayOneSidedLightingFlag;
}

bool
VolumeAttributes::GetOSPRayAOTransparencyEnabledFlag() const
{
    return OSPRayAOTransparencyEnabledFlag;
}

int
VolumeAttributes::GetOSPRaySPP() const
{
    return OSPRaySPP;
}

int
VolumeAttributes::GetOSPRayAOSamples() const
{
    return OSPRayAOSamples;
}

double
VolumeAttributes::GetOSPRayAODistance() const
{
    return OSPRayAODistance;
}

double
VolumeAttributes::GetOSPRayMinContribution() const
{
    return OSPRayMinContribution;
}

double
VolumeAttributes::GetOSPRayMaxContribution() const
{
    return OSPRayMaxContribution;
}

bool
VolumeAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
VolumeAttributes::GetLightingFlag() const
{
    return lightingFlag;
}

const ColorControlPointList &
VolumeAttributes::GetColorControlPoints() const
{
    return colorControlPoints;
}

ColorControlPointList &
VolumeAttributes::GetColorControlPoints()
{
    return colorControlPoints;
}

float
VolumeAttributes::GetOpacityAttenuation() const
{
    return opacityAttenuation;
}

VolumeAttributes::OpacityModes
VolumeAttributes::GetOpacityMode() const
{
    return OpacityModes(opacityMode);
}

const GaussianControlPointList &
VolumeAttributes::GetOpacityControlPoints() const
{
    return opacityControlPoints;
}

GaussianControlPointList &
VolumeAttributes::GetOpacityControlPoints()
{
    return opacityControlPoints;
}

VolumeAttributes::ResampleType
VolumeAttributes::GetResampleType() const
{
    return ResampleType(resampleType);
}

int
VolumeAttributes::GetResampleTarget() const
{
    return resampleTarget;
}

VolumeAttributes::ResampleCentering
VolumeAttributes::GetResampleCentering() const
{
    return ResampleCentering(resampleCentering);
}

const std::string &
VolumeAttributes::GetOpacityVariable() const
{
    return opacityVariable;
}

std::string &
VolumeAttributes::GetOpacityVariable()
{
    return opacityVariable;
}

const unsigned char *
VolumeAttributes::GetFreeformOpacity() const
{
    return freeformOpacity;
}

unsigned char *
VolumeAttributes::GetFreeformOpacity()
{
    return freeformOpacity;
}

bool
VolumeAttributes::GetUseColorVarMin() const
{
    return useColorVarMin;
}

float
VolumeAttributes::GetColorVarMin() const
{
    return colorVarMin;
}

bool
VolumeAttributes::GetUseColorVarMax() const
{
    return useColorVarMax;
}

float
VolumeAttributes::GetColorVarMax() const
{
    return colorVarMax;
}

bool
VolumeAttributes::GetUseOpacityVarMin() const
{
    return useOpacityVarMin;
}

float
VolumeAttributes::GetOpacityVarMin() const
{
    return opacityVarMin;
}

bool
VolumeAttributes::GetUseOpacityVarMax() const
{
    return useOpacityVarMax;
}

float
VolumeAttributes::GetOpacityVarMax() const
{
    return opacityVarMax;
}

bool
VolumeAttributes::GetSmoothData() const
{
    return smoothData;
}

int
VolumeAttributes::GetSamplesPerRay() const
{
    return samplesPerRay;
}

VolumeAttributes::Renderer
VolumeAttributes::GetRendererType() const
{
    return Renderer(rendererType);
}

VolumeAttributes::GradientType
VolumeAttributes::GetGradientType() const
{
    return GradientType(gradientType);
}

VolumeAttributes::Scaling
VolumeAttributes::GetScaling() const
{
    return Scaling(scaling);
}

double
VolumeAttributes::GetSkewFactor() const
{
    return skewFactor;
}

VolumeAttributes::LimitsMode
VolumeAttributes::GetLimitsMode() const
{
    return LimitsMode(limitsMode);
}

VolumeAttributes::SamplingType
VolumeAttributes::GetSampling() const
{
    return SamplingType(sampling);
}

float
VolumeAttributes::GetRendererSamples() const
{
    return rendererSamples;
}

VolumeAttributes::LowGradientLightingReduction
VolumeAttributes::GetLowGradientLightingReduction() const
{
    return LowGradientLightingReduction(lowGradientLightingReduction);
}

bool
VolumeAttributes::GetLowGradientLightingClampFlag() const
{
    return lowGradientLightingClampFlag;
}

double
VolumeAttributes::GetLowGradientLightingClampValue() const
{
    return lowGradientLightingClampValue;
}

const double *
VolumeAttributes::GetMaterialProperties() const
{
    return materialProperties;
}

double *
VolumeAttributes::GetMaterialProperties()
{
    return materialProperties;
}

bool
VolumeAttributes::GetAnariRendering() const
{
    return anariRendering;
}

int
VolumeAttributes::GetAnariSPP() const
{
    return anariSPP;
}

int
VolumeAttributes::GetAnariAO() const
{
    return anariAO;
}

const std::string &
VolumeAttributes::GetAnariLibrary() const
{
    return anariLibrary;
}

std::string &
VolumeAttributes::GetAnariLibrary()
{
    return anariLibrary;
}

const std::string &
VolumeAttributes::GetAnariLibrarySubtype() const
{
    return anariLibrarySubtype;
}

std::string &
VolumeAttributes::GetAnariLibrarySubtype()
{
    return anariLibrarySubtype;
}

const std::string &
VolumeAttributes::GetAnariRendererSubtype() const
{
    return anariRendererSubtype;
}

std::string &
VolumeAttributes::GetAnariRendererSubtype()
{
    return anariRendererSubtype;
}

float
VolumeAttributes::GetAnariLightFalloff() const
{
    return anariLightFalloff;
}

float
VolumeAttributes::GetAnariAmbientIntensity() const
{
    return anariAmbientIntensity;
}

int
VolumeAttributes::GetAnariMaxDepth() const
{
    return anariMaxDepth;
}

float
VolumeAttributes::GetAnariRValue() const
{
    return anariRValue;
}

const std::string &
VolumeAttributes::GetUsdDir() const
{
    return usdDir;
}

std::string &
VolumeAttributes::GetUsdDir()
{
    return usdDir;
}

bool
VolumeAttributes::GetUsdAtCommit() const
{
    return usdAtCommit;
}

bool
VolumeAttributes::GetUsdOutputBinary() const
{
    return usdOutputBinary;
}

bool
VolumeAttributes::GetUsdOutputMaterial() const
{
    return usdOutputMaterial;
}

bool
VolumeAttributes::GetUsdOutputPreviewSurface() const
{
    return usdOutputPreviewSurface;
}

bool
VolumeAttributes::GetUsdOutputMDL() const
{
    return usdOutputMDL;
}

bool
VolumeAttributes::GetUsdOutputMDLColors() const
{
    return usdOutputMDLColors;
}

bool
VolumeAttributes::GetUsdOutputDisplayColors() const
{
    return usdOutputDisplayColors;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
VolumeAttributes::SelectColorControlPoints()
{
    Select(ID_colorControlPoints, (void *)&colorControlPoints);
}

void
VolumeAttributes::SelectOpacityControlPoints()
{
    Select(ID_opacityControlPoints, (void *)&opacityControlPoints);
}

void
VolumeAttributes::SelectOpacityVariable()
{
    Select(ID_opacityVariable, (void *)&opacityVariable);
}

void
VolumeAttributes::SelectFreeformOpacity()
{
    Select(ID_freeformOpacity, (void *)freeformOpacity, 256);
}

void
VolumeAttributes::SelectMaterialProperties()
{
    Select(ID_materialProperties, (void *)materialProperties, 4);
}

void
VolumeAttributes::SelectAnariLibrary()
{
    Select(ID_anariLibrary, (void *)&anariLibrary);
}

void
VolumeAttributes::SelectAnariLibrarySubtype()
{
    Select(ID_anariLibrarySubtype, (void *)&anariLibrarySubtype);
}

void
VolumeAttributes::SelectAnariRendererSubtype()
{
    Select(ID_anariRendererSubtype, (void *)&anariRendererSubtype);
}

void
VolumeAttributes::SelectUsdDir()
{
    Select(ID_usdDir, (void *)&usdDir);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: VolumeAttributes::GetFieldName
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
VolumeAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_OSPRayEnabledFlag:               return "OSPRayEnabledFlag";
    case ID_OSPRayRenderType:                return "OSPRayRenderType";
    case ID_OSPRayShadowsEnabledFlag:        return "OSPRayShadowsEnabledFlag";
    case ID_OSPRayUseGridAcceleratorFlag:    return "OSPRayUseGridAcceleratorFlag";
    case ID_OSPRayPreIntegrationFlag:        return "OSPRayPreIntegrationFlag";
    case ID_OSPRaySingleShadeFlag:           return "OSPRaySingleShadeFlag";
    case ID_OSPRayOneSidedLightingFlag:      return "OSPRayOneSidedLightingFlag";
    case ID_OSPRayAOTransparencyEnabledFlag: return "OSPRayAOTransparencyEnabledFlag";
    case ID_OSPRaySPP:                       return "OSPRaySPP";
    case ID_OSPRayAOSamples:                 return "OSPRayAOSamples";
    case ID_OSPRayAODistance:                return "OSPRayAODistance";
    case ID_OSPRayMinContribution:           return "OSPRayMinContribution";
    case ID_OSPRayMaxContribution:           return "OSPRayMaxContribution";
    case ID_legendFlag:                      return "legendFlag";
    case ID_lightingFlag:                    return "lightingFlag";
    case ID_colorControlPoints:              return "colorControlPoints";
    case ID_opacityAttenuation:              return "opacityAttenuation";
    case ID_opacityMode:                     return "opacityMode";
    case ID_opacityControlPoints:            return "opacityControlPoints";
    case ID_resampleType:                    return "resampleType";
    case ID_resampleTarget:                  return "resampleTarget";
    case ID_resampleCentering:               return "resampleCentering";
    case ID_opacityVariable:                 return "opacityVariable";
    case ID_freeformOpacity:                 return "freeformOpacity";
    case ID_useColorVarMin:                  return "useColorVarMin";
    case ID_colorVarMin:                     return "colorVarMin";
    case ID_useColorVarMax:                  return "useColorVarMax";
    case ID_colorVarMax:                     return "colorVarMax";
    case ID_useOpacityVarMin:                return "useOpacityVarMin";
    case ID_opacityVarMin:                   return "opacityVarMin";
    case ID_useOpacityVarMax:                return "useOpacityVarMax";
    case ID_opacityVarMax:                   return "opacityVarMax";
    case ID_smoothData:                      return "smoothData";
    case ID_samplesPerRay:                   return "samplesPerRay";
    case ID_rendererType:                    return "rendererType";
    case ID_gradientType:                    return "gradientType";
    case ID_scaling:                         return "scaling";
    case ID_skewFactor:                      return "skewFactor";
    case ID_limitsMode:                      return "limitsMode";
    case ID_sampling:                        return "sampling";
    case ID_rendererSamples:                 return "rendererSamples";
    case ID_lowGradientLightingReduction:    return "lowGradientLightingReduction";
    case ID_lowGradientLightingClampFlag:    return "lowGradientLightingClampFlag";
    case ID_lowGradientLightingClampValue:   return "lowGradientLightingClampValue";
    case ID_materialProperties:              return "materialProperties";
    case ID_anariRendering:                  return "anariRendering";
    case ID_anariSPP:                        return "anariSPP";
    case ID_anariAO:                         return "anariAO";
    case ID_anariLibrary:                    return "anariLibrary";
    case ID_anariLibrarySubtype:             return "anariLibrarySubtype";
    case ID_anariRendererSubtype:            return "anariRendererSubtype";
    case ID_anariLightFalloff:               return "anariLightFalloff";
    case ID_anariAmbientIntensity:           return "anariAmbientIntensity";
    case ID_anariMaxDepth:                   return "anariMaxDepth";
    case ID_anariRValue:                     return "anariRValue";
    case ID_usdDir:                          return "usdDir";
    case ID_usdAtCommit:                     return "usdAtCommit";
    case ID_usdOutputBinary:                 return "usdOutputBinary";
    case ID_usdOutputMaterial:               return "usdOutputMaterial";
    case ID_usdOutputPreviewSurface:         return "usdOutputPreviewSurface";
    case ID_usdOutputMDL:                    return "usdOutputMDL";
    case ID_usdOutputMDLColors:              return "usdOutputMDLColors";
    case ID_usdOutputDisplayColors:          return "usdOutputDisplayColors";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: VolumeAttributes::GetFieldType
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
VolumeAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_OSPRayEnabledFlag:               return FieldType_bool;
    case ID_OSPRayRenderType:                return FieldType_enum;
    case ID_OSPRayShadowsEnabledFlag:        return FieldType_bool;
    case ID_OSPRayUseGridAcceleratorFlag:    return FieldType_bool;
    case ID_OSPRayPreIntegrationFlag:        return FieldType_bool;
    case ID_OSPRaySingleShadeFlag:           return FieldType_bool;
    case ID_OSPRayOneSidedLightingFlag:      return FieldType_bool;
    case ID_OSPRayAOTransparencyEnabledFlag: return FieldType_bool;
    case ID_OSPRaySPP:                       return FieldType_int;
    case ID_OSPRayAOSamples:                 return FieldType_int;
    case ID_OSPRayAODistance:                return FieldType_double;
    case ID_OSPRayMinContribution:           return FieldType_double;
    case ID_OSPRayMaxContribution:           return FieldType_double;
    case ID_legendFlag:                      return FieldType_bool;
    case ID_lightingFlag:                    return FieldType_bool;
    case ID_colorControlPoints:              return FieldType_att;
    case ID_opacityAttenuation:              return FieldType_float;
    case ID_opacityMode:                     return FieldType_enum;
    case ID_opacityControlPoints:            return FieldType_att;
    case ID_resampleType:                    return FieldType_enum;
    case ID_resampleTarget:                  return FieldType_int;
    case ID_resampleCentering:               return FieldType_enum;
    case ID_opacityVariable:                 return FieldType_variablename;
    case ID_freeformOpacity:                 return FieldType_ucharArray;
    case ID_useColorVarMin:                  return FieldType_bool;
    case ID_colorVarMin:                     return FieldType_float;
    case ID_useColorVarMax:                  return FieldType_bool;
    case ID_colorVarMax:                     return FieldType_float;
    case ID_useOpacityVarMin:                return FieldType_bool;
    case ID_opacityVarMin:                   return FieldType_float;
    case ID_useOpacityVarMax:                return FieldType_bool;
    case ID_opacityVarMax:                   return FieldType_float;
    case ID_smoothData:                      return FieldType_bool;
    case ID_samplesPerRay:                   return FieldType_int;
    case ID_rendererType:                    return FieldType_enum;
    case ID_gradientType:                    return FieldType_enum;
    case ID_scaling:                         return FieldType_enum;
    case ID_skewFactor:                      return FieldType_double;
    case ID_limitsMode:                      return FieldType_enum;
    case ID_sampling:                        return FieldType_enum;
    case ID_rendererSamples:                 return FieldType_float;
    case ID_lowGradientLightingReduction:    return FieldType_enum;
    case ID_lowGradientLightingClampFlag:    return FieldType_bool;
    case ID_lowGradientLightingClampValue:   return FieldType_double;
    case ID_materialProperties:              return FieldType_doubleArray;
    case ID_anariRendering:                  return FieldType_bool;
    case ID_anariSPP:                        return FieldType_int;
    case ID_anariAO:                         return FieldType_int;
    case ID_anariLibrary:                    return FieldType_string;
    case ID_anariLibrarySubtype:             return FieldType_string;
    case ID_anariRendererSubtype:            return FieldType_string;
    case ID_anariLightFalloff:               return FieldType_float;
    case ID_anariAmbientIntensity:           return FieldType_float;
    case ID_anariMaxDepth:                   return FieldType_int;
    case ID_anariRValue:                     return FieldType_float;
    case ID_usdDir:                          return FieldType_string;
    case ID_usdAtCommit:                     return FieldType_bool;
    case ID_usdOutputBinary:                 return FieldType_bool;
    case ID_usdOutputMaterial:               return FieldType_bool;
    case ID_usdOutputPreviewSurface:         return FieldType_bool;
    case ID_usdOutputMDL:                    return FieldType_bool;
    case ID_usdOutputMDLColors:              return FieldType_bool;
    case ID_usdOutputDisplayColors:          return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: VolumeAttributes::GetFieldTypeName
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
VolumeAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_OSPRayEnabledFlag:               return "bool";
    case ID_OSPRayRenderType:                return "enum";
    case ID_OSPRayShadowsEnabledFlag:        return "bool";
    case ID_OSPRayUseGridAcceleratorFlag:    return "bool";
    case ID_OSPRayPreIntegrationFlag:        return "bool";
    case ID_OSPRaySingleShadeFlag:           return "bool";
    case ID_OSPRayOneSidedLightingFlag:      return "bool";
    case ID_OSPRayAOTransparencyEnabledFlag: return "bool";
    case ID_OSPRaySPP:                       return "int";
    case ID_OSPRayAOSamples:                 return "int";
    case ID_OSPRayAODistance:                return "double";
    case ID_OSPRayMinContribution:           return "double";
    case ID_OSPRayMaxContribution:           return "double";
    case ID_legendFlag:                      return "bool";
    case ID_lightingFlag:                    return "bool";
    case ID_colorControlPoints:              return "att";
    case ID_opacityAttenuation:              return "float";
    case ID_opacityMode:                     return "enum";
    case ID_opacityControlPoints:            return "att";
    case ID_resampleType:                    return "enum";
    case ID_resampleTarget:                  return "int";
    case ID_resampleCentering:               return "enum";
    case ID_opacityVariable:                 return "variablename";
    case ID_freeformOpacity:                 return "ucharArray";
    case ID_useColorVarMin:                  return "bool";
    case ID_colorVarMin:                     return "float";
    case ID_useColorVarMax:                  return "bool";
    case ID_colorVarMax:                     return "float";
    case ID_useOpacityVarMin:                return "bool";
    case ID_opacityVarMin:                   return "float";
    case ID_useOpacityVarMax:                return "bool";
    case ID_opacityVarMax:                   return "float";
    case ID_smoothData:                      return "bool";
    case ID_samplesPerRay:                   return "int";
    case ID_rendererType:                    return "enum";
    case ID_gradientType:                    return "enum";
    case ID_scaling:                         return "enum";
    case ID_skewFactor:                      return "double";
    case ID_limitsMode:                      return "enum";
    case ID_sampling:                        return "enum";
    case ID_rendererSamples:                 return "float";
    case ID_lowGradientLightingReduction:    return "enum";
    case ID_lowGradientLightingClampFlag:    return "bool";
    case ID_lowGradientLightingClampValue:   return "double";
    case ID_materialProperties:              return "doubleArray";
    case ID_anariRendering:                  return "bool";
    case ID_anariSPP:                        return "int";
    case ID_anariAO:                         return "int";
    case ID_anariLibrary:                    return "string";
    case ID_anariLibrarySubtype:             return "string";
    case ID_anariRendererSubtype:            return "string";
    case ID_anariLightFalloff:               return "float";
    case ID_anariAmbientIntensity:           return "float";
    case ID_anariMaxDepth:                   return "int";
    case ID_anariRValue:                     return "float";
    case ID_usdDir:                          return "string";
    case ID_usdAtCommit:                     return "bool";
    case ID_usdOutputBinary:                 return "bool";
    case ID_usdOutputMaterial:               return "bool";
    case ID_usdOutputPreviewSurface:         return "bool";
    case ID_usdOutputMDL:                    return "bool";
    case ID_usdOutputMDLColors:              return "bool";
    case ID_usdOutputDisplayColors:          return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: VolumeAttributes::FieldsEqual
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
VolumeAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const VolumeAttributes &obj = *((const VolumeAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_OSPRayEnabledFlag:
        {  // new scope
        retval = (OSPRayEnabledFlag == obj.OSPRayEnabledFlag);
        }
        break;
    case ID_OSPRayRenderType:
        {  // new scope
        retval = (OSPRayRenderType == obj.OSPRayRenderType);
        }
        break;
    case ID_OSPRayShadowsEnabledFlag:
        {  // new scope
        retval = (OSPRayShadowsEnabledFlag == obj.OSPRayShadowsEnabledFlag);
        }
        break;
    case ID_OSPRayUseGridAcceleratorFlag:
        {  // new scope
        retval = (OSPRayUseGridAcceleratorFlag == obj.OSPRayUseGridAcceleratorFlag);
        }
        break;
    case ID_OSPRayPreIntegrationFlag:
        {  // new scope
        retval = (OSPRayPreIntegrationFlag == obj.OSPRayPreIntegrationFlag);
        }
        break;
    case ID_OSPRaySingleShadeFlag:
        {  // new scope
        retval = (OSPRaySingleShadeFlag == obj.OSPRaySingleShadeFlag);
        }
        break;
    case ID_OSPRayOneSidedLightingFlag:
        {  // new scope
        retval = (OSPRayOneSidedLightingFlag == obj.OSPRayOneSidedLightingFlag);
        }
        break;
    case ID_OSPRayAOTransparencyEnabledFlag:
        {  // new scope
        retval = (OSPRayAOTransparencyEnabledFlag == obj.OSPRayAOTransparencyEnabledFlag);
        }
        break;
    case ID_OSPRaySPP:
        {  // new scope
        retval = (OSPRaySPP == obj.OSPRaySPP);
        }
        break;
    case ID_OSPRayAOSamples:
        {  // new scope
        retval = (OSPRayAOSamples == obj.OSPRayAOSamples);
        }
        break;
    case ID_OSPRayAODistance:
        {  // new scope
        retval = (OSPRayAODistance == obj.OSPRayAODistance);
        }
        break;
    case ID_OSPRayMinContribution:
        {  // new scope
        retval = (OSPRayMinContribution == obj.OSPRayMinContribution);
        }
        break;
    case ID_OSPRayMaxContribution:
        {  // new scope
        retval = (OSPRayMaxContribution == obj.OSPRayMaxContribution);
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
    case ID_colorControlPoints:
        {  // new scope
        retval = (colorControlPoints == obj.colorControlPoints);
        }
        break;
    case ID_opacityAttenuation:
        {  // new scope
        retval = (opacityAttenuation == obj.opacityAttenuation);
        }
        break;
    case ID_opacityMode:
        {  // new scope
        retval = (opacityMode == obj.opacityMode);
        }
        break;
    case ID_opacityControlPoints:
        {  // new scope
        retval = (opacityControlPoints == obj.opacityControlPoints);
        }
        break;
    case ID_resampleType:
        {  // new scope
        retval = (resampleType == obj.resampleType);
        }
        break;
    case ID_resampleTarget:
        {  // new scope
        retval = (resampleTarget == obj.resampleTarget);
        }
        break;
    case ID_resampleCentering:
        {  // new scope
        retval = (resampleCentering == obj.resampleCentering);
        }
        break;
    case ID_opacityVariable:
        {  // new scope
        retval = (opacityVariable == obj.opacityVariable);
        }
        break;
    case ID_freeformOpacity:
        {  // new scope
        // Compare the freeformOpacity arrays.
        bool freeformOpacity_equal = true;
        for(int i = 0; i < 256 && freeformOpacity_equal; ++i)
            freeformOpacity_equal = (freeformOpacity[i] == obj.freeformOpacity[i]);

        retval = freeformOpacity_equal;
        }
        break;
    case ID_useColorVarMin:
        {  // new scope
        retval = (useColorVarMin == obj.useColorVarMin);
        }
        break;
    case ID_colorVarMin:
        {  // new scope
        retval = (colorVarMin == obj.colorVarMin);
        }
        break;
    case ID_useColorVarMax:
        {  // new scope
        retval = (useColorVarMax == obj.useColorVarMax);
        }
        break;
    case ID_colorVarMax:
        {  // new scope
        retval = (colorVarMax == obj.colorVarMax);
        }
        break;
    case ID_useOpacityVarMin:
        {  // new scope
        retval = (useOpacityVarMin == obj.useOpacityVarMin);
        }
        break;
    case ID_opacityVarMin:
        {  // new scope
        retval = (opacityVarMin == obj.opacityVarMin);
        }
        break;
    case ID_useOpacityVarMax:
        {  // new scope
        retval = (useOpacityVarMax == obj.useOpacityVarMax);
        }
        break;
    case ID_opacityVarMax:
        {  // new scope
        retval = (opacityVarMax == obj.opacityVarMax);
        }
        break;
    case ID_smoothData:
        {  // new scope
        retval = (smoothData == obj.smoothData);
        }
        break;
    case ID_samplesPerRay:
        {  // new scope
        retval = (samplesPerRay == obj.samplesPerRay);
        }
        break;
    case ID_rendererType:
        {  // new scope
        retval = (rendererType == obj.rendererType);
        }
        break;
    case ID_gradientType:
        {  // new scope
        retval = (gradientType == obj.gradientType);
        }
        break;
    case ID_scaling:
        {  // new scope
        retval = (scaling == obj.scaling);
        }
        break;
    case ID_skewFactor:
        {  // new scope
        retval = (skewFactor == obj.skewFactor);
        }
        break;
    case ID_limitsMode:
        {  // new scope
        retval = (limitsMode == obj.limitsMode);
        }
        break;
    case ID_sampling:
        {  // new scope
        retval = (sampling == obj.sampling);
        }
        break;
    case ID_rendererSamples:
        {  // new scope
        retval = (rendererSamples == obj.rendererSamples);
        }
        break;
    case ID_lowGradientLightingReduction:
        {  // new scope
        retval = (lowGradientLightingReduction == obj.lowGradientLightingReduction);
        }
        break;
    case ID_lowGradientLightingClampFlag:
        {  // new scope
        retval = (lowGradientLightingClampFlag == obj.lowGradientLightingClampFlag);
        }
        break;
    case ID_lowGradientLightingClampValue:
        {  // new scope
        retval = (lowGradientLightingClampValue == obj.lowGradientLightingClampValue);
        }
        break;
    case ID_materialProperties:
        {  // new scope
        // Compare the materialProperties arrays.
        bool materialProperties_equal = true;
        for(int i = 0; i < 4 && materialProperties_equal; ++i)
            materialProperties_equal = (materialProperties[i] == obj.materialProperties[i]);

        retval = materialProperties_equal;
        }
        break;
    case ID_anariRendering:
        {  // new scope
        retval = (anariRendering == obj.anariRendering);
        }
        break;
    case ID_anariSPP:
        {  // new scope
        retval = (anariSPP == obj.anariSPP);
        }
        break;
    case ID_anariAO:
        {  // new scope
        retval = (anariAO == obj.anariAO);
        }
        break;
    case ID_anariLibrary:
        {  // new scope
        retval = (anariLibrary == obj.anariLibrary);
        }
        break;
    case ID_anariLibrarySubtype:
        {  // new scope
        retval = (anariLibrarySubtype == obj.anariLibrarySubtype);
        }
        break;
    case ID_anariRendererSubtype:
        {  // new scope
        retval = (anariRendererSubtype == obj.anariRendererSubtype);
        }
        break;
    case ID_anariLightFalloff:
        {  // new scope
        retval = (anariLightFalloff == obj.anariLightFalloff);
        }
        break;
    case ID_anariAmbientIntensity:
        {  // new scope
        retval = (anariAmbientIntensity == obj.anariAmbientIntensity);
        }
        break;
    case ID_anariMaxDepth:
        {  // new scope
        retval = (anariMaxDepth == obj.anariMaxDepth);
        }
        break;
    case ID_anariRValue:
        {  // new scope
        retval = (anariRValue == obj.anariRValue);
        }
        break;
    case ID_usdDir:
        {  // new scope
        retval = (usdDir == obj.usdDir);
        }
        break;
    case ID_usdAtCommit:
        {  // new scope
        retval = (usdAtCommit == obj.usdAtCommit);
        }
        break;
    case ID_usdOutputBinary:
        {  // new scope
        retval = (usdOutputBinary == obj.usdOutputBinary);
        }
        break;
    case ID_usdOutputMaterial:
        {  // new scope
        retval = (usdOutputMaterial == obj.usdOutputMaterial);
        }
        break;
    case ID_usdOutputPreviewSurface:
        {  // new scope
        retval = (usdOutputPreviewSurface == obj.usdOutputPreviewSurface);
        }
        break;
    case ID_usdOutputMDL:
        {  // new scope
        retval = (usdOutputMDL == obj.usdOutputMDL);
        }
        break;
    case ID_usdOutputMDLColors:
        {  // new scope
        retval = (usdOutputMDLColors == obj.usdOutputMDLColors);
        }
        break;
    case ID_usdOutputDisplayColors:
        {  // new scope
        retval = (usdOutputDisplayColors == obj.usdOutputDisplayColors);
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
// Method: VolumeAttributes::ProcessOldVersions
//
// Purpose:
//   This method allows handling of older config/session files that may
//   contain fields that are no longer present or have been modified/renamed.
//
// Programmer: Jeremy Meredith
// Creation:   June 18, 2003
//
// ****************************************************************************

#include <visit-config.h>
#ifdef VIEWER
#include <avtCallback.h>
#endif

void
VolumeAttributes::ProcessOldVersions(DataNode *parentNode,
                                     const char *configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("VolumeAttributes");
    if(searchNode == 0)
        return;

#if VISIT_OBSOLETE_AT_VERSION(3,5,0)
#error This code is obsolete in this version of VisIt and should be removed.
#else
    if (VersionLessThan(configVersion, "3.4.0"))
    {
        DataNode *dn = nullptr;
        if (searchNode->GetNode("compactVariable") != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("compactVariable", "3.5.0"));
#endif
            searchNode->RemoveNode("compactVariable", true);
        }
        if (searchNode->GetNode("renderMode") != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("renderMode", "3.5.0"));
#endif
            searchNode->RemoveNode("renderMode", true);
        }
        if ((dn = searchNode->GetNode("resampleFlag")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("resampleFlag", "resampleType", "3.5.0"));
#endif
            int intVal = dn->AsInt();

            VolumeAttributes::ResampleType val = (intVal ? OnlyIfRequired : SingleDomain);

            searchNode->RemoveNode("resampleFlag", true);
            searchNode->AddNode(new DataNode("resampleType",
                                              ResampleType_ToString(val)));
        }
        if ((dn = searchNode->GetNode("osprayShadowsEnabledFlag")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayShadowsEnabledFlag",
                "OSPRayShadowsEnabledFlag", "3.5.0"));
#endif
            dn->SetKey("OSPRayShadowsEnabledFlag");
        }
        if ((dn = searchNode->GetNode("osprayUseGridAcceleratorFlag")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayUseGridAcceleratorFlag",
                "OSPRayUseGridAcceleratorFlag", "3.5.0"));
#endif
            dn->SetKey("OSPRayUseGridAcceleratorFlag");
        }
        if ((dn = searchNode->GetNode("osprayPreIntegrationFlag")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayPreIntegrationFlag",
                "OSPRayPreIntegrationFlag", "3.5.0"));
#endif
            dn->SetKey("OSPRayPreIntegrationFlag");
        }
        if ((dn = searchNode->GetNode("ospraySingleShadeFlag")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("ospraySingleShadeFlag",
                "OSPRaySingleShadeFlag", "3.5.0"));
#endif
            dn->SetKey("OSPRaySingleShadeFlag");
        }
        if ((dn = searchNode->GetNode("osprayOneSidedLightingFlag")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayOneSidedLightingFlag",
                "OSPRayOneSidedLightingFlag", "3.5.0"));
#endif
            dn->SetKey("OSPRayOneSidedLightingFlag");
        }
        if ((dn = searchNode->GetNode("osprayAoTransparencyEnabledFlag")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayAoTransparencyEnabledFlag",
                "OSPRayAOTransparencyEnabledFlag", "3.5.0"));
#endif
            dn->SetKey("OSPRayAOTransparencyEnabledFlag");
        }
        if ((dn = searchNode->GetNode("ospraySpp")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("ospraySpp",
                "OSPRaySPP", "3.5.0"));
#endif
            dn->SetKey("OSPRaySPP");
        }
        if ((dn = searchNode->GetNode("osprayAoSamples")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayAoSamples",
                "OSPRayAOSamples", "3.5.0"));
#endif
            dn->SetKey("OSPRayAOSamples");
        }
        if ((dn = searchNode->GetNode("osprayAoDistance")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayAoDistance",
                "OSPRayAODistance", "3.5.0"));
#endif
            dn->SetKey("OSPRayAODistance");
        }
        if ((dn = searchNode->GetNode("osprayMinContribution")) != nullptr)
        {
#ifdef VIEWER
            avtCallback::IssueWarning(DeprecationMessage("osprayMinContribution",
                "OSPRayMinContribution", "3.5.0"));
#endif
            dn->SetKey("OSPRayMinContribution");
        }
        if ((dn = searchNode->GetNode("rendererType")) != nullptr)
        {
            std::string type = dn->AsString();
            if (type == "Default")
            {
#ifdef VIEWER
                avtCallback::IssueWarning(DeprecationMessage("Default",
                    "Default", "3.5.0"));
#endif
                dn->SetString(Renderer_ToString(VolumeAttributes::Serial));
            }
            else if (type == "RayCasting")
            {
#ifdef VIEWER
                avtCallback::IssueWarning(DeprecationMessage("RayCasting",
                    "Composite", "3.5.0"));
#endif
                dn->SetString(Renderer_ToString(VolumeAttributes::Composite));
            }
            else if (type == "RayCastingIntegration")
            {
#ifdef VIEWER
                avtCallback::IssueWarning(DeprecationMessage("RayCastingIntegration",
                    "Integration", "3.5.0"));
#endif
                dn->SetString(Renderer_ToString(VolumeAttributes::Integration));
            }
            else if (type == "RayCastingSLIVR")
            {
#ifdef VIEWER
                avtCallback::IssueWarning(DeprecationMessage("RayCastingSLIVR",
                    "SLIVR", "3.5.0"));
#endif
                dn->SetString(Renderer_ToString(VolumeAttributes::SLIVR));
            }
            else if (type == "RayCastingOSPRay")
            {
#ifdef VIEWER
                avtCallback::IssueWarning(DeprecationMessage("RayCastingOSPRay",
                    "Parallel", "3.5.0"));
#endif
                dn->SetString(Renderer_ToString(VolumeAttributes::Parallel));
                searchNode->AddNode(new DataNode("OSPRayEnabledFlag", true));
            }
        }
    }
#endif
}

// ****************************************************************************
//  Method:  VolumeAttributes::ChangesRequireRecalculation
//
//  Modifications:
//    Jeremy Meredith, Thu Oct  2 13:27:54 PDT 2003
//    Let changes in rendererType to force a recalculation.  This is
//    appropriate since the 3D texturing renderer prefers different
//    dimensions (i.e. powers of two) than the splatting renderer.
//
//    Hank Childs, Mon Dec 15 14:42:26 PST 2003
//    Recalculate if the smooth option was hit.
//
//    Hank Childs, Mon Nov 22 09:37:12 PST 2004
//    Recalculate if the ray trace button was hit.
//
//    Brad Whitlock, Wed Dec 15 09:31:24 PDT 2004
//    Removed doSoftware since it's now part of rendererType.
//
//    Kathleen Bonnell, Thu Mar  3 09:27:40 PST 2005
//    Recalculate if scaling or skewFactor changed for RayCasting.
//
//    Brad Whitlock, Tue Dec 23 16:58:42 PST 2008
//    Recalculate more often with a HW renderer mode so we can recalculate
//    the histogram.
//
//    Hank Childs, Fri Jan 29 14:23:45 MST 2010
//    Re-execute the pipeline if we need lighting.
//
//    Allen Harvey, Thurs Nov 3 7:21:13 EST 2011
//    Added checks for not doing resampling
//
//    Qi WU, Sat Jun 10 22:21:27 MST 2018
//    Added RayCastingOSPRay option for volume rendering
//
// ****************************************************************************

bool
VolumeAttributes::ChangesRequireRecalculation(const VolumeAttributes &obj) const
{
    if (opacityVariable != obj.opacityVariable)
        return true;

    // Any change to the renderer type requires a reexecute.
    if(rendererType != obj.rendererType)
        return true;

    if (smoothData != obj.smoothData)
        return true;

    if (scaling != obj.scaling)
        return true;
    if (scaling == VolumeAttributes::Skew && skewFactor != obj.skewFactor)
        return true;

    if(rendererType == VolumeAttributes::Serial ||
       rendererType == VolumeAttributes::Parallel)
    {
        if(resampleType != obj.resampleType)
            return true;
        if(resampleTarget != obj.resampleTarget)
            return true;
        if(resampleCentering != obj.resampleCentering)
            return true;
    }
    else if(rendererType == VolumeAttributes::Composite)
    {
        // Trilinear requires ghost zone while Rasterization and KernelBased do not
        if ((sampling == Rasterization || sampling == KernelBased) && obj.sampling == Trilinear)
            return true;

        if ((sampling == Trilinear) && (obj.sampling == KernelBased || obj.sampling == Rasterization))
            return true;

       if(lightingFlag != obj.lightingFlag)
            return true;
    }
//    else if(rendererType == VolumeAttributes::Integration ||
//            rendererType == VolumeAttributes::SLIVR)
//    {
//    }

    return false;
}


// ****************************************************************************
// Method: VolumeAttributes::GetTransferFunction
//
// Purpose:
//   This method calculates the transfer function and stores it in the rgba
//   array that is passed in.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 21 15:44:34 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Nov 21 15:05:25 PST 2002
//   GetColors has been moved to ColorControlPointList. I updated this code
//   to take that into account.
//
//   Jeremy Meredith, Thu Oct  2 13:29:40 PDT 2003
//   Made the method const.
//
//   Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//   Made opacity mode be an enum instead of a flag.
//   Added support for alphas to come from a color table
//   instead of being specified by the user in the plot itself.
//
// ****************************************************************************

void
VolumeAttributes::GetTransferFunction(unsigned char *rgba) const
{
    unsigned char rgb[256 * 3];
    unsigned char alphas[256];
    const unsigned char *a_ptr;

    // Figure out the colors
    colorControlPoints.GetColors(rgb, 256, alphas);
    // Figure out the opacities
    if (opacityMode == FreeformMode)
        a_ptr = freeformOpacity;
    else if (opacityMode == GaussianMode)
    {
        GetGaussianOpacities(alphas);
        a_ptr = alphas;
    }
    else // color table mode
    {
        // we already got the opacities
        a_ptr = alphas;
    }

    unsigned char *rgb_ptr = rgb;
    unsigned char *rgba_ptr = rgba;
    for(int i = 0; i < 256; ++i)
    {
        // Copy the color
        *rgba_ptr++ = *rgb_ptr++;
        *rgba_ptr++ = *rgb_ptr++;
        *rgba_ptr++ = *rgb_ptr++;
        // Copy the alpha
        *rgba_ptr++ = *a_ptr++;
    }
}

// ****************************************************************************
// Method: VolumeAttributes::SetDefaultColorControlPoints
//
// Purpose:
//   This method replaces all of the color control points in the list with the
//   default color control points.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 21 15:44:34 PST 2001
//
// Modifications:
//
// ****************************************************************************

void
VolumeAttributes::SetDefaultColorControlPoints()
{
    const float positions[] = {0., 0.25, 0.5, 0.75, 1.};
    const unsigned char colors[5][4] = {
        {0,   0,   255, 255},
        {0,   255, 255, 255},
        {0,   255, 0,   255},
        {255, 255, 0,   255},
        {255, 0,   0,   255}};

    // Clear the color control point list.
    colorControlPoints.ClearControlPoints();

    // Set the default control points in the color control point list.
    for(int i = 0; i < 5; ++i)
    {
        ColorControlPoint cpt;
        cpt.SetPosition(positions[i]);
        cpt.SetColors(colors[i]);
        colorControlPoints.AddControlPoints(cpt);
    }
    SelectColorControlPoints();
}

// ****************************************************************************
// Method: VolumeAttributes::GetGaussianOpacities
//
// Purpose:
//   This method calculates the opacities using the object's gaussian control
//   point list and stores the results in the alphas array that is passed in.
//
// Arguments:
//   alphas : The return array for the colors.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 10:23:59 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Thu Oct  2 13:30:00 PDT 2003
//    Made the method const.
//
// ****************************************************************************
#include <math.h>
void
VolumeAttributes::GetGaussianOpacities(unsigned char *alphas) const
{
    int i;
    float values[256];
    for (i=0; i<256; i++)
        values[i] = 0.;

    for (int p=0; p<opacityControlPoints.GetNumControlPoints(); p++)
    {
        const GaussianControlPoint &pt =
                               opacityControlPoints.GetControlPoints(p);
        float pos    = pt.GetX();
        float width  = pt.GetWidth();
        float height = pt.GetHeight();
        float xbias  = pt.GetXBias();
        float ybias  = pt.GetYBias();
        for (int i=0; i<256; i++)
        {
            float x = float(i)/float(256-1);

            // clamp non-zero values to pos +/- width
            if (x > pos+width || x < pos-width)
            {
                values[i] = (values[i] > 0.) ? values[i] : 0.;
                continue;
            }

            // non-zero width
            if (width == 0)
                width = .00001;

            // translate the original x to a new x based on the xbias
            float x0;
            if (xbias==0 || x == pos+xbias)
            {
                x0 = x;
            }
            else if (x > pos+xbias)
            {
                if (width == xbias)
                    x0 = pos;
                else
                    x0 = pos+(x-pos-xbias)*(width/(width-xbias));
            }
            else // (x < pos+xbias)
            {
                if (-width == xbias)
                    x0 = pos;
                else
                    x0 = pos-(x-pos-xbias)*(width/(width+xbias));
            }

            // center around 0 and normalize to -1,1
            float x1 = (x0-pos)/width;

            // do a linear interpolation between:
            //    a gaussian and a parabola        if 0<ybias<1
            //    a parabola and a step function   if 1<ybias<2
            float h0a = exp(-(4*x1*x1));
            float h0b = 1. - x1*x1;
            float h0c = 1.;
            float h1;
            if (ybias < 1)
                h1 = ybias*h0b + (1-ybias)*h0a;
            else
                h1 = (2-ybias)*h0b + (ybias-1)*h0c;
            float h2 = height * h1;

            // perform the MAX over different guassians, not the sum
            values[i] = (values[i] > h2) ? values[i] : h2;
        }
    }

    // Convert to unsigned char and return.
    for(i = 0; i < 256; ++i)
    {
        int tmp = int(values[i] * 255.);
        if(tmp < 0)
            tmp = 0;
        else if(tmp > 255)
            tmp = 255;
        alphas[i] = (unsigned char)(tmp);
    }
}

// ****************************************************************************
//  Method:  VolumeAttributes::GetOpacities
//
//  Purpose:
//    Get the actual opacities for the plot.
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//    Made opacity mode be an enum instead of a flag.
//    Added support for alphas to come from a color table
//    instead of being specified by the user in the plot itself.
//
// ****************************************************************************
void
VolumeAttributes::GetOpacities(unsigned char *alphas)
{
    if (opacityMode == FreeformMode)
    {
        for(int i = 0; i < 256; ++i)
            alphas[i] = freeformOpacity[i];
    }
    else if (opacityMode == GaussianMode)
    {
        GetGaussianOpacities(alphas);
    }
    else // color table mode
    {
        unsigned char rgbtmp[256 * 3];
        colorControlPoints.GetColors(rgbtmp, 256, alphas);
    }
}

void
VolumeAttributes::SetSmoothing(ColorControlPointList::SmoothingMethod val)
{
    colorControlPoints.SetSmoothing(val);
    Select(2, (void *)&colorControlPoints);
}

ColorControlPointList::SmoothingMethod
VolumeAttributes::GetSmoothing() const
{
    return colorControlPoints.GetSmoothing();
}

void
VolumeAttributes::SetEqualSpacingFlag(bool val)
{
    colorControlPoints.SetEqualSpacingFlag(val);
    Select(2, (void *)&colorControlPoints);
}

bool
VolumeAttributes::GetEqualSpacingFlag() const
{
    return colorControlPoints.GetEqualSpacingFlag();
}

// ****************************************************************************
// Method: VolumeAttributes::AnyNonTransferFunctionMembersAreDifferent
//
// Purpose:
//   This method returns true if any non-transfer function members are different.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 10:23:59 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Fri Feb 20 15:16:43 EST 2009
//    Made opacity mode be an enum instead of a flag.
//
// ****************************************************************************

bool
VolumeAttributes::AnyNonTransferFunctionMembersAreDifferent(const VolumeAttributes &rhs) const
{
    bool same = true;
    for(int i = 0; i < NumAttributes(); ++i)
    {
        // Skip any of the transfer function members.
        if(i == ID_colorControlPoints ||
           i == ID_opacityAttenuation ||
           i == ID_opacityMode ||
           i == ID_opacityControlPoints ||
           i == ID_freeformOpacity)
        {
            continue;
        }
        same &= FieldsEqual(i, &rhs);
    }

    return !same;
}

