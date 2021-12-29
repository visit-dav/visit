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
// Enum conversion methods for VolumeAttributes::Resample
//

static const char *Resample_strings[] = {
"None", "SingleDomain", "ParallelRedistribute",
"ParallelPerRank"};

std::string
VolumeAttributes::Resample_ToString(VolumeAttributes::Resample t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return Resample_strings[index];
}

std::string
VolumeAttributes::Resample_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return Resample_strings[index];
}

bool
VolumeAttributes::Resample_FromString(const std::string &s, VolumeAttributes::Resample &val)
{
    val = VolumeAttributes::None;
    for(int i = 0; i < 4; ++i)
    {
        if(s == Resample_strings[i])
        {
            val = (Resample)i;
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
// Enum conversion methods for VolumeAttributes::OSPRayRenderType
//

static const char *OSPRayRenderType_strings[] = {
"SciVis", "PathTracer"};

std::string
VolumeAttributes::OSPRayRenderType_ToString(VolumeAttributes::OSPRayRenderType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OSPRayRenderType_strings[index];
}

std::string
VolumeAttributes::OSPRayRenderType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OSPRayRenderType_strings[index];
}

bool
VolumeAttributes::OSPRayRenderType_FromString(const std::string &s, VolumeAttributes::OSPRayRenderType &val)
{
    val = VolumeAttributes::SciVis;
    for(int i = 0; i < 2; ++i)
    {
        if(s == OSPRayRenderType_strings[i])
        {
            val = (OSPRayRenderType)i;
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
    osprayEnabledFlag = false;
    osprayRenderType = SciVis;
    osprayShadowsEnabledFlag = false;
    osprayUseGridAcceleratorFlag = false;
    osprayPreIntegrationFlag = false;
    ospraySingleShadeFlag = false;
    osprayOneSidedLightingFlag = false;
    osprayAOTransparencyEnabledFlag = false;
    ospraySPP = 1;
    osprayAOSamples = 0;
    osprayAODistance = 100000;
    osprayMinContribution = 0.001;
    osprayMaxContribution = 2;
    legendFlag = true;
    lightingFlag = true;
    SetDefaultColorControlPoints();
    opacityAttenuation = 1;
    opacityMode = FreeformMode;
    resampleType = None;
    resampleTarget = 1000000;
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

    osprayEnabledFlag = obj.osprayEnabledFlag;
    osprayRenderType = obj.osprayRenderType;
    osprayShadowsEnabledFlag = obj.osprayShadowsEnabledFlag;
    osprayUseGridAcceleratorFlag = obj.osprayUseGridAcceleratorFlag;
    osprayPreIntegrationFlag = obj.osprayPreIntegrationFlag;
    ospraySingleShadeFlag = obj.ospraySingleShadeFlag;
    osprayOneSidedLightingFlag = obj.osprayOneSidedLightingFlag;
    osprayAOTransparencyEnabledFlag = obj.osprayAOTransparencyEnabledFlag;
    ospraySPP = obj.ospraySPP;
    osprayAOSamples = obj.osprayAOSamples;
    osprayAODistance = obj.osprayAODistance;
    osprayMinContribution = obj.osprayMinContribution;
    osprayMaxContribution = obj.osprayMaxContribution;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    colorControlPoints = obj.colorControlPoints;
    opacityAttenuation = obj.opacityAttenuation;
    opacityMode = obj.opacityMode;
    opacityControlPoints = obj.opacityControlPoints;
    resampleType = obj.resampleType;
    resampleTarget = obj.resampleTarget;
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
    return ((osprayEnabledFlag == obj.osprayEnabledFlag) &&
            (osprayRenderType == obj.osprayRenderType) &&
            (osprayShadowsEnabledFlag == obj.osprayShadowsEnabledFlag) &&
            (osprayUseGridAcceleratorFlag == obj.osprayUseGridAcceleratorFlag) &&
            (osprayPreIntegrationFlag == obj.osprayPreIntegrationFlag) &&
            (ospraySingleShadeFlag == obj.ospraySingleShadeFlag) &&
            (osprayOneSidedLightingFlag == obj.osprayOneSidedLightingFlag) &&
            (osprayAOTransparencyEnabledFlag == obj.osprayAOTransparencyEnabledFlag) &&
            (ospraySPP == obj.ospraySPP) &&
            (osprayAOSamples == obj.osprayAOSamples) &&
            (osprayAODistance == obj.osprayAODistance) &&
            (osprayMinContribution == obj.osprayMinContribution) &&
            (osprayMaxContribution == obj.osprayMaxContribution) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (colorControlPoints == obj.colorControlPoints) &&
            (opacityAttenuation == obj.opacityAttenuation) &&
            (opacityMode == obj.opacityMode) &&
            (opacityControlPoints == obj.opacityControlPoints) &&
            (resampleType == obj.resampleType) &&
            (resampleTarget == obj.resampleTarget) &&
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
            materialProperties_equal);
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
    Select(ID_osprayEnabledFlag,               (void *)&osprayEnabledFlag);
    Select(ID_osprayRenderType,                (void *)&osprayRenderType);
    Select(ID_osprayShadowsEnabledFlag,        (void *)&osprayShadowsEnabledFlag);
    Select(ID_osprayUseGridAcceleratorFlag,    (void *)&osprayUseGridAcceleratorFlag);
    Select(ID_osprayPreIntegrationFlag,        (void *)&osprayPreIntegrationFlag);
    Select(ID_ospraySingleShadeFlag,           (void *)&ospraySingleShadeFlag);
    Select(ID_osprayOneSidedLightingFlag,      (void *)&osprayOneSidedLightingFlag);
    Select(ID_osprayAOTransparencyEnabledFlag, (void *)&osprayAOTransparencyEnabledFlag);
    Select(ID_ospraySPP,                       (void *)&ospraySPP);
    Select(ID_osprayAOSamples,                 (void *)&osprayAOSamples);
    Select(ID_osprayAODistance,                (void *)&osprayAODistance);
    Select(ID_osprayMinContribution,           (void *)&osprayMinContribution);
    Select(ID_osprayMaxContribution,           (void *)&osprayMaxContribution);
    Select(ID_legendFlag,                      (void *)&legendFlag);
    Select(ID_lightingFlag,                    (void *)&lightingFlag);
    Select(ID_colorControlPoints,              (void *)&colorControlPoints);
    Select(ID_opacityAttenuation,              (void *)&opacityAttenuation);
    Select(ID_opacityMode,                     (void *)&opacityMode);
    Select(ID_opacityControlPoints,            (void *)&opacityControlPoints);
    Select(ID_resampleType,                    (void *)&resampleType);
    Select(ID_resampleTarget,                  (void *)&resampleTarget);
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

    if(completeSave || !FieldsEqual(ID_osprayEnabledFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayEnabledFlag", osprayEnabledFlag));
    }

    if(completeSave || !FieldsEqual(ID_osprayRenderType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayRenderType", OSPRayRenderType_ToString(osprayRenderType)));
    }

    if(completeSave || !FieldsEqual(ID_osprayShadowsEnabledFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayShadowsEnabledFlag", osprayShadowsEnabledFlag));
    }

    if(completeSave || !FieldsEqual(ID_osprayUseGridAcceleratorFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayUseGridAcceleratorFlag", osprayUseGridAcceleratorFlag));
    }

    if(completeSave || !FieldsEqual(ID_osprayPreIntegrationFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayPreIntegrationFlag", osprayPreIntegrationFlag));
    }

    if(completeSave || !FieldsEqual(ID_ospraySingleShadeFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ospraySingleShadeFlag", ospraySingleShadeFlag));
    }

    if(completeSave || !FieldsEqual(ID_osprayOneSidedLightingFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayOneSidedLightingFlag", osprayOneSidedLightingFlag));
    }

    if(completeSave || !FieldsEqual(ID_osprayAOTransparencyEnabledFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayAOTransparencyEnabledFlag", osprayAOTransparencyEnabledFlag));
    }

    if(completeSave || !FieldsEqual(ID_ospraySPP, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ospraySPP", ospraySPP));
    }

    if(completeSave || !FieldsEqual(ID_osprayAOSamples, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayAOSamples", osprayAOSamples));
    }

    if(completeSave || !FieldsEqual(ID_osprayAODistance, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayAODistance", osprayAODistance));
    }

    if(completeSave || !FieldsEqual(ID_osprayMinContribution, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayMinContribution", osprayMinContribution));
    }

    if(completeSave || !FieldsEqual(ID_osprayMaxContribution, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("osprayMaxContribution", osprayMaxContribution));
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
        node->AddNode(new DataNode("resampleType", Resample_ToString(resampleType)));
    }

    if(completeSave || !FieldsEqual(ID_resampleTarget, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("resampleTarget", resampleTarget));
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
    if((node = searchNode->GetNode("osprayEnabledFlag")) != 0)
        SetOsprayEnabledFlag(node->AsBool());
    if((node = searchNode->GetNode("osprayRenderType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetOsprayRenderType(OSPRayRenderType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OSPRayRenderType value;
            if(OSPRayRenderType_FromString(node->AsString(), value))
                SetOsprayRenderType(value);
        }
    }
    if((node = searchNode->GetNode("osprayShadowsEnabledFlag")) != 0)
        SetOsprayShadowsEnabledFlag(node->AsBool());
    if((node = searchNode->GetNode("osprayUseGridAcceleratorFlag")) != 0)
        SetOsprayUseGridAcceleratorFlag(node->AsBool());
    if((node = searchNode->GetNode("osprayPreIntegrationFlag")) != 0)
        SetOsprayPreIntegrationFlag(node->AsBool());
    if((node = searchNode->GetNode("ospraySingleShadeFlag")) != 0)
        SetOspraySingleShadeFlag(node->AsBool());
    if((node = searchNode->GetNode("osprayOneSidedLightingFlag")) != 0)
        SetOsprayOneSidedLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("osprayAOTransparencyEnabledFlag")) != 0)
        SetOsprayAOTransparencyEnabledFlag(node->AsBool());
    if((node = searchNode->GetNode("ospraySPP")) != 0)
        SetOspraySPP(node->AsInt());
    if((node = searchNode->GetNode("osprayAOSamples")) != 0)
        SetOsprayAOSamples(node->AsInt());
    if((node = searchNode->GetNode("osprayAODistance")) != 0)
        SetOsprayAODistance(node->AsDouble());
    if((node = searchNode->GetNode("osprayMinContribution")) != 0)
        SetOsprayMinContribution(node->AsDouble());
    if((node = searchNode->GetNode("osprayMaxContribution")) != 0)
        SetOsprayMaxContribution(node->AsDouble());
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
            if(ival >= 0 && ival < 4)
                SetResampleType(Resample(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Resample value;
            if(Resample_FromString(node->AsString(), value))
                SetResampleType(value);
        }
    }
    if((node = searchNode->GetNode("resampleTarget")) != 0)
        SetResampleTarget(node->AsInt());
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
    if(colorControlPoints.GetNumControlPoints() < 2)
         SetDefaultColorControlPoints();

}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
VolumeAttributes::SetOsprayEnabledFlag(bool osprayEnabledFlag_)
{
    osprayEnabledFlag = osprayEnabledFlag_;
    Select(ID_osprayEnabledFlag, (void *)&osprayEnabledFlag);
}

void
VolumeAttributes::SetOsprayRenderType(VolumeAttributes::OSPRayRenderType osprayRenderType_)
{
    osprayRenderType = osprayRenderType_;
    Select(ID_osprayRenderType, (void *)&osprayRenderType);
}

void
VolumeAttributes::SetOsprayShadowsEnabledFlag(bool osprayShadowsEnabledFlag_)
{
    osprayShadowsEnabledFlag = osprayShadowsEnabledFlag_;
    Select(ID_osprayShadowsEnabledFlag, (void *)&osprayShadowsEnabledFlag);
}

void
VolumeAttributes::SetOsprayUseGridAcceleratorFlag(bool osprayUseGridAcceleratorFlag_)
{
    osprayUseGridAcceleratorFlag = osprayUseGridAcceleratorFlag_;
    Select(ID_osprayUseGridAcceleratorFlag, (void *)&osprayUseGridAcceleratorFlag);
}

void
VolumeAttributes::SetOsprayPreIntegrationFlag(bool osprayPreIntegrationFlag_)
{
    osprayPreIntegrationFlag = osprayPreIntegrationFlag_;
    Select(ID_osprayPreIntegrationFlag, (void *)&osprayPreIntegrationFlag);
}

void
VolumeAttributes::SetOspraySingleShadeFlag(bool ospraySingleShadeFlag_)
{
    ospraySingleShadeFlag = ospraySingleShadeFlag_;
    Select(ID_ospraySingleShadeFlag, (void *)&ospraySingleShadeFlag);
}

void
VolumeAttributes::SetOsprayOneSidedLightingFlag(bool osprayOneSidedLightingFlag_)
{
    osprayOneSidedLightingFlag = osprayOneSidedLightingFlag_;
    Select(ID_osprayOneSidedLightingFlag, (void *)&osprayOneSidedLightingFlag);
}

void
VolumeAttributes::SetOsprayAOTransparencyEnabledFlag(bool osprayAOTransparencyEnabledFlag_)
{
    osprayAOTransparencyEnabledFlag = osprayAOTransparencyEnabledFlag_;
    Select(ID_osprayAOTransparencyEnabledFlag, (void *)&osprayAOTransparencyEnabledFlag);
}

void
VolumeAttributes::SetOspraySPP(int ospraySPP_)
{
    ospraySPP = ospraySPP_;
    Select(ID_ospraySPP, (void *)&ospraySPP);
}

void
VolumeAttributes::SetOsprayAOSamples(int osprayAOSamples_)
{
    osprayAOSamples = osprayAOSamples_;
    Select(ID_osprayAOSamples, (void *)&osprayAOSamples);
}

void
VolumeAttributes::SetOsprayAODistance(double osprayAODistance_)
{
    osprayAODistance = osprayAODistance_;
    Select(ID_osprayAODistance, (void *)&osprayAODistance);
}

void
VolumeAttributes::SetOsprayMinContribution(double osprayMinContribution_)
{
    osprayMinContribution = osprayMinContribution_;
    Select(ID_osprayMinContribution, (void *)&osprayMinContribution);
}

void
VolumeAttributes::SetOsprayMaxContribution(double osprayMaxContribution_)
{
    osprayMaxContribution = osprayMaxContribution_;
    Select(ID_osprayMaxContribution, (void *)&osprayMaxContribution);
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
VolumeAttributes::SetResampleType(VolumeAttributes::Resample resampleType_)
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

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
VolumeAttributes::GetOsprayEnabledFlag() const
{
    return osprayEnabledFlag;
}

VolumeAttributes::OSPRayRenderType
VolumeAttributes::GetOsprayRenderType() const
{
    return OSPRayRenderType(osprayRenderType);
}

bool
VolumeAttributes::GetOsprayShadowsEnabledFlag() const
{
    return osprayShadowsEnabledFlag;
}

bool
VolumeAttributes::GetOsprayUseGridAcceleratorFlag() const
{
    return osprayUseGridAcceleratorFlag;
}

bool
VolumeAttributes::GetOsprayPreIntegrationFlag() const
{
    return osprayPreIntegrationFlag;
}

bool
VolumeAttributes::GetOspraySingleShadeFlag() const
{
    return ospraySingleShadeFlag;
}

bool
VolumeAttributes::GetOsprayOneSidedLightingFlag() const
{
    return osprayOneSidedLightingFlag;
}

bool
VolumeAttributes::GetOsprayAOTransparencyEnabledFlag() const
{
    return osprayAOTransparencyEnabledFlag;
}

int
VolumeAttributes::GetOspraySPP() const
{
    return ospraySPP;
}

int
VolumeAttributes::GetOsprayAOSamples() const
{
    return osprayAOSamples;
}

double
VolumeAttributes::GetOsprayAODistance() const
{
    return osprayAODistance;
}

double
VolumeAttributes::GetOsprayMinContribution() const
{
    return osprayMinContribution;
}

double
VolumeAttributes::GetOsprayMaxContribution() const
{
    return osprayMaxContribution;
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

VolumeAttributes::Resample
VolumeAttributes::GetResampleType() const
{
    return Resample(resampleType);
}

int
VolumeAttributes::GetResampleTarget() const
{
    return resampleTarget;
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
    case ID_osprayEnabledFlag:               return "osprayEnabledFlag";
    case ID_osprayRenderType:                return "osprayRenderType";
    case ID_osprayShadowsEnabledFlag:        return "osprayShadowsEnabledFlag";
    case ID_osprayUseGridAcceleratorFlag:    return "osprayUseGridAcceleratorFlag";
    case ID_osprayPreIntegrationFlag:        return "osprayPreIntegrationFlag";
    case ID_ospraySingleShadeFlag:           return "ospraySingleShadeFlag";
    case ID_osprayOneSidedLightingFlag:      return "osprayOneSidedLightingFlag";
    case ID_osprayAOTransparencyEnabledFlag: return "osprayAOTransparencyEnabledFlag";
    case ID_ospraySPP:                       return "ospraySPP";
    case ID_osprayAOSamples:                 return "osprayAOSamples";
    case ID_osprayAODistance:                return "osprayAODistance";
    case ID_osprayMinContribution:           return "osprayMinContribution";
    case ID_osprayMaxContribution:           return "osprayMaxContribution";
    case ID_legendFlag:                      return "legendFlag";
    case ID_lightingFlag:                    return "lightingFlag";
    case ID_colorControlPoints:              return "colorControlPoints";
    case ID_opacityAttenuation:              return "opacityAttenuation";
    case ID_opacityMode:                     return "opacityMode";
    case ID_opacityControlPoints:            return "opacityControlPoints";
    case ID_resampleType:                    return "resampleType";
    case ID_resampleTarget:                  return "resampleTarget";
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
    case ID_osprayEnabledFlag:               return FieldType_bool;
    case ID_osprayRenderType:                return FieldType_enum;
    case ID_osprayShadowsEnabledFlag:        return FieldType_bool;
    case ID_osprayUseGridAcceleratorFlag:    return FieldType_bool;
    case ID_osprayPreIntegrationFlag:        return FieldType_bool;
    case ID_ospraySingleShadeFlag:           return FieldType_bool;
    case ID_osprayOneSidedLightingFlag:      return FieldType_bool;
    case ID_osprayAOTransparencyEnabledFlag: return FieldType_bool;
    case ID_ospraySPP:                       return FieldType_int;
    case ID_osprayAOSamples:                 return FieldType_int;
    case ID_osprayAODistance:                return FieldType_double;
    case ID_osprayMinContribution:           return FieldType_double;
    case ID_osprayMaxContribution:           return FieldType_double;
    case ID_legendFlag:                      return FieldType_bool;
    case ID_lightingFlag:                    return FieldType_bool;
    case ID_colorControlPoints:              return FieldType_att;
    case ID_opacityAttenuation:              return FieldType_float;
    case ID_opacityMode:                     return FieldType_enum;
    case ID_opacityControlPoints:            return FieldType_att;
    case ID_resampleType:                    return FieldType_enum;
    case ID_resampleTarget:                  return FieldType_int;
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
    case ID_osprayEnabledFlag:               return "bool";
    case ID_osprayRenderType:                return "enum";
    case ID_osprayShadowsEnabledFlag:        return "bool";
    case ID_osprayUseGridAcceleratorFlag:    return "bool";
    case ID_osprayPreIntegrationFlag:        return "bool";
    case ID_ospraySingleShadeFlag:           return "bool";
    case ID_osprayOneSidedLightingFlag:      return "bool";
    case ID_osprayAOTransparencyEnabledFlag: return "bool";
    case ID_ospraySPP:                       return "int";
    case ID_osprayAOSamples:                 return "int";
    case ID_osprayAODistance:                return "double";
    case ID_osprayMinContribution:           return "double";
    case ID_osprayMaxContribution:           return "double";
    case ID_legendFlag:                      return "bool";
    case ID_lightingFlag:                    return "bool";
    case ID_colorControlPoints:              return "att";
    case ID_opacityAttenuation:              return "float";
    case ID_opacityMode:                     return "enum";
    case ID_opacityControlPoints:            return "att";
    case ID_resampleType:                    return "enum";
    case ID_resampleTarget:                  return "int";
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
    case ID_osprayEnabledFlag:
        {  // new scope
        retval = (osprayEnabledFlag == obj.osprayEnabledFlag);
        }
        break;
    case ID_osprayRenderType:
        {  // new scope
        retval = (osprayRenderType == obj.osprayRenderType);
        }
        break;
    case ID_osprayShadowsEnabledFlag:
        {  // new scope
        retval = (osprayShadowsEnabledFlag == obj.osprayShadowsEnabledFlag);
        }
        break;
    case ID_osprayUseGridAcceleratorFlag:
        {  // new scope
        retval = (osprayUseGridAcceleratorFlag == obj.osprayUseGridAcceleratorFlag);
        }
        break;
    case ID_osprayPreIntegrationFlag:
        {  // new scope
        retval = (osprayPreIntegrationFlag == obj.osprayPreIntegrationFlag);
        }
        break;
    case ID_ospraySingleShadeFlag:
        {  // new scope
        retval = (ospraySingleShadeFlag == obj.ospraySingleShadeFlag);
        }
        break;
    case ID_osprayOneSidedLightingFlag:
        {  // new scope
        retval = (osprayOneSidedLightingFlag == obj.osprayOneSidedLightingFlag);
        }
        break;
    case ID_osprayAOTransparencyEnabledFlag:
        {  // new scope
        retval = (osprayAOTransparencyEnabledFlag == obj.osprayAOTransparencyEnabledFlag);
        }
        break;
    case ID_ospraySPP:
        {  // new scope
        retval = (ospraySPP == obj.ospraySPP);
        }
        break;
    case ID_osprayAOSamples:
        {  // new scope
        retval = (osprayAOSamples == obj.osprayAOSamples);
        }
        break;
    case ID_osprayAODistance:
        {  // new scope
        retval = (osprayAODistance == obj.osprayAODistance);
        }
        break;
    case ID_osprayMinContribution:
        {  // new scope
        retval = (osprayMinContribution == obj.osprayMinContribution);
        }
        break;
    case ID_osprayMaxContribution:
        {  // new scope
        retval = (osprayMaxContribution == obj.osprayMaxContribution);
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
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

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

    if (resampleType != obj.resampleType)
        return true;

    if (resampleTarget != obj.resampleTarget)
        return true;

    if (rendererType == VolumeAttributes::Composite ||
        rendererType == VolumeAttributes::SLIVR ||
        rendererType == VolumeAttributes::Parallel ||
        rendererType == VolumeAttributes::Integration)
    {
        // Trilinear requires ghost zone while Rasterization and KernelBased do not
        if ((sampling == Rasterization || sampling == KernelBased) && obj.sampling == Trilinear)
            return true;

        if ((sampling == Trilinear) && (obj.sampling == KernelBased || obj.sampling == Rasterization))
            return true;

        // We're in software mode. Any change to the renderer type requires
        // a reexecute.
        if(rendererType != obj.rendererType)
            return true;

        if (scaling != obj.scaling)
            return true;
        if (scaling == VolumeAttributes::Skew && skewFactor != obj.skewFactor)
            return true;
        if (lightingFlag != obj.lightingFlag)
            return true;
    }
    else
    {
        // We're in hardware mode now but if we're transitioning to software
        // then we need to reexecute. Transferring between any of the hardware
        // modes does not require a reexecute.
        if(obj.rendererType == VolumeAttributes::Composite ||
           obj.rendererType == VolumeAttributes::SLIVR ||
           obj.rendererType == VolumeAttributes::Parallel ||
           obj.rendererType == VolumeAttributes::Integration)
        {
            return true;
        }

        // We need to reexecute on the engine for thse changes in HW mode.

        if(useColorVarMin != obj.useColorVarMin)
            return true;
        if(colorVarMin != obj.colorVarMin)
            return true;
        if(useColorVarMax != obj.useColorVarMax)
            return true;
        if(colorVarMax != obj.colorVarMax)
            return true;
        if(useOpacityVarMin != obj.useOpacityVarMin)
            return true;
        if(opacityVarMin != obj.opacityVarMin)
            return true;
        if(useOpacityVarMax != obj.useOpacityVarMax)
            return true;
        if(opacityVarMax != obj.opacityVarMax)
            return true;
        if(gradientType != obj.gradientType)
            return true;
        if(scaling != obj.scaling)
            return true;
        if(skewFactor != obj.skewFactor)
            return true;
    }

    if (smoothData != obj.smoothData)
        return true;

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

