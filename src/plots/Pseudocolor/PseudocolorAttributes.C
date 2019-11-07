// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <PseudocolorAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for PseudocolorAttributes::Scaling
//

static const char *Scaling_strings[] = {
"Linear", "Log", "Skew"
};

std::string
PseudocolorAttributes::Scaling_ToString(PseudocolorAttributes::Scaling t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Scaling_strings[index];
}

std::string
PseudocolorAttributes::Scaling_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Scaling_strings[index];
}

bool
PseudocolorAttributes::Scaling_FromString(const std::string &s, PseudocolorAttributes::Scaling &val)
{
    val = PseudocolorAttributes::Linear;
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
// Enum conversion methods for PseudocolorAttributes::LimitsMode
//

static const char *LimitsMode_strings[] = {
"OriginalData", "CurrentPlot"};

std::string
PseudocolorAttributes::LimitsMode_ToString(PseudocolorAttributes::LimitsMode t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return LimitsMode_strings[index];
}

std::string
PseudocolorAttributes::LimitsMode_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return LimitsMode_strings[index];
}

bool
PseudocolorAttributes::LimitsMode_FromString(const std::string &s, PseudocolorAttributes::LimitsMode &val)
{
    val = PseudocolorAttributes::OriginalData;
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
// Enum conversion methods for PseudocolorAttributes::Centering
//

static const char *Centering_strings[] = {
"Natural", "Nodal", "Zonal"
};

std::string
PseudocolorAttributes::Centering_ToString(PseudocolorAttributes::Centering t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return Centering_strings[index];
}

std::string
PseudocolorAttributes::Centering_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return Centering_strings[index];
}

bool
PseudocolorAttributes::Centering_FromString(const std::string &s, PseudocolorAttributes::Centering &val)
{
    val = PseudocolorAttributes::Natural;
    for(int i = 0; i < 3; ++i)
    {
        if(s == Centering_strings[i])
        {
            val = (Centering)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PseudocolorAttributes::OpacityType
//

static const char *OpacityType_strings[] = {
"ColorTable", "FullyOpaque", "Constant", 
"Ramp", "VariableRange"};

std::string
PseudocolorAttributes::OpacityType_ToString(PseudocolorAttributes::OpacityType t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return OpacityType_strings[index];
}

std::string
PseudocolorAttributes::OpacityType_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return OpacityType_strings[index];
}

bool
PseudocolorAttributes::OpacityType_FromString(const std::string &s, PseudocolorAttributes::OpacityType &val)
{
    val = PseudocolorAttributes::ColorTable;
    for(int i = 0; i < 5; ++i)
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
// Enum conversion methods for PseudocolorAttributes::LineType
//

static const char *LineType_strings[] = {
"Line", "Tube", "Ribbon"
};

std::string
PseudocolorAttributes::LineType_ToString(PseudocolorAttributes::LineType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return LineType_strings[index];
}

std::string
PseudocolorAttributes::LineType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return LineType_strings[index];
}

bool
PseudocolorAttributes::LineType_FromString(const std::string &s, PseudocolorAttributes::LineType &val)
{
    val = PseudocolorAttributes::Line;
    for(int i = 0; i < 3; ++i)
    {
        if(s == LineType_strings[i])
        {
            val = (LineType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PseudocolorAttributes::EndPointStyle
//

static const char *EndPointStyle_strings[] = {
"None", "Spheres", "Cones"
};

std::string
PseudocolorAttributes::EndPointStyle_ToString(PseudocolorAttributes::EndPointStyle t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return EndPointStyle_strings[index];
}

std::string
PseudocolorAttributes::EndPointStyle_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return EndPointStyle_strings[index];
}

bool
PseudocolorAttributes::EndPointStyle_FromString(const std::string &s, PseudocolorAttributes::EndPointStyle &val)
{
    val = PseudocolorAttributes::None;
    for(int i = 0; i < 3; ++i)
    {
        if(s == EndPointStyle_strings[i])
        {
            val = (EndPointStyle)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PseudocolorAttributes::SizeType
//

static const char *SizeType_strings[] = {
"Absolute", "FractionOfBBox"};

std::string
PseudocolorAttributes::SizeType_ToString(PseudocolorAttributes::SizeType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SizeType_strings[index];
}

std::string
PseudocolorAttributes::SizeType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SizeType_strings[index];
}

bool
PseudocolorAttributes::SizeType_FromString(const std::string &s, PseudocolorAttributes::SizeType &val)
{
    val = PseudocolorAttributes::Absolute;
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
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Init utility for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PseudocolorAttributes::Init()
{
    scaling = Linear;
    skewFactor = 1;
    limitsMode = OriginalData;
    minFlag = false;
    min = 0;
    useBelowMinColor = false;
    maxFlag = false;
    max = 1;
    useAboveMaxColor = false;
    centering = Natural;
    invertColorTable = false;
    opacityType = FullyOpaque;
    opacity = 1;
    opacityVarMin = 0;
    opacityVarMax = 1;
    opacityVarMinFlag = false;
    opacityVarMaxFlag = false;
    pointSize = 0.05;
    pointType = Point;
    pointSizeVarEnabled = false;
    pointSizePixels = 2;
    lineType = Line;
    lineWidth = 0;
    tubeResolution = 10;
    tubeRadiusSizeType = FractionOfBBox;
    tubeRadiusAbsolute = 0.125;
    tubeRadiusBBox = 0.005;
    tubeRadiusVarEnabled = false;
    tubeRadiusVarRatio = 10;
    tailStyle = None;
    headStyle = None;
    endPointRadiusSizeType = FractionOfBBox;
    endPointRadiusAbsolute = 0.125;
    endPointRadiusBBox = 0.05;
    endPointResolution = 10;
    endPointRatio = 5;
    endPointRadiusVarEnabled = false;
    endPointRadiusVarRatio = 10;
    renderSurfaces = 1;
    renderWireframe = 0;
    renderPoints = 0;
    smoothingLevel = 0;
    legendFlag = true;
    lightingFlag = true;

    PseudocolorAttributes::SelectAll();
}

// ****************************************************************************
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Copy utility for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PseudocolorAttributes::Copy(const PseudocolorAttributes &obj)
{
    scaling = obj.scaling;
    skewFactor = obj.skewFactor;
    limitsMode = obj.limitsMode;
    minFlag = obj.minFlag;
    min = obj.min;
    useBelowMinColor = obj.useBelowMinColor;
    belowMinColor = obj.belowMinColor;
    maxFlag = obj.maxFlag;
    max = obj.max;
    useAboveMaxColor = obj.useAboveMaxColor;
    aboveMaxColor = obj.aboveMaxColor;
    centering = obj.centering;
    colorTableName = obj.colorTableName;
    invertColorTable = obj.invertColorTable;
    opacityType = obj.opacityType;
    opacityVariable = obj.opacityVariable;
    opacity = obj.opacity;
    opacityVarMin = obj.opacityVarMin;
    opacityVarMax = obj.opacityVarMax;
    opacityVarMinFlag = obj.opacityVarMinFlag;
    opacityVarMaxFlag = obj.opacityVarMaxFlag;
    pointSize = obj.pointSize;
    pointType = obj.pointType;
    pointSizeVarEnabled = obj.pointSizeVarEnabled;
    pointSizeVar = obj.pointSizeVar;
    pointSizePixels = obj.pointSizePixels;
    lineType = obj.lineType;
    lineWidth = obj.lineWidth;
    tubeResolution = obj.tubeResolution;
    tubeRadiusSizeType = obj.tubeRadiusSizeType;
    tubeRadiusAbsolute = obj.tubeRadiusAbsolute;
    tubeRadiusBBox = obj.tubeRadiusBBox;
    tubeRadiusVarEnabled = obj.tubeRadiusVarEnabled;
    tubeRadiusVar = obj.tubeRadiusVar;
    tubeRadiusVarRatio = obj.tubeRadiusVarRatio;
    tailStyle = obj.tailStyle;
    headStyle = obj.headStyle;
    endPointRadiusSizeType = obj.endPointRadiusSizeType;
    endPointRadiusAbsolute = obj.endPointRadiusAbsolute;
    endPointRadiusBBox = obj.endPointRadiusBBox;
    endPointResolution = obj.endPointResolution;
    endPointRatio = obj.endPointRatio;
    endPointRadiusVarEnabled = obj.endPointRadiusVarEnabled;
    endPointRadiusVar = obj.endPointRadiusVar;
    endPointRadiusVarRatio = obj.endPointRadiusVarRatio;
    renderSurfaces = obj.renderSurfaces;
    renderWireframe = obj.renderWireframe;
    renderPoints = obj.renderPoints;
    smoothingLevel = obj.smoothingLevel;
    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    wireframeColor = obj.wireframeColor;
    pointColor = obj.pointColor;

    PseudocolorAttributes::SelectAll();
}

// Type map format string
const char *PseudocolorAttributes::TypeMapFormatString = PSEUDOCOLORATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t PseudocolorAttributes::TmfsStruct = {PSEUDOCOLORATTRIBUTES_TMFS};


// ****************************************************************************
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Default constructor for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::PseudocolorAttributes() : 
    AttributeSubject(PseudocolorAttributes::TypeMapFormatString),
    belowMinColor(), aboveMaxColor(), 
    colorTableName("Default"), pointSizeVar("default"), 
    wireframeColor(0, 0, 0, 0), pointColor(0, 0, 0, 0)
{
    PseudocolorAttributes::Init();
}

// ****************************************************************************
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Constructor for the derived classes of PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::PseudocolorAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs),
    belowMinColor(), aboveMaxColor(), 
    colorTableName("Default"), pointSizeVar("default"), 
    wireframeColor(0, 0, 0, 0), pointColor(0, 0, 0, 0)
{
    PseudocolorAttributes::Init();
}

// ****************************************************************************
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Copy constructor for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::PseudocolorAttributes(const PseudocolorAttributes &obj) : 
    AttributeSubject(PseudocolorAttributes::TypeMapFormatString)
{
    PseudocolorAttributes::Copy(obj);
}

// ****************************************************************************
// Method: PseudocolorAttributes::PseudocolorAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::PseudocolorAttributes(const PseudocolorAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    PseudocolorAttributes::Copy(obj);
}

// ****************************************************************************
// Method: PseudocolorAttributes::~PseudocolorAttributes
//
// Purpose: 
//   Destructor for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes::~PseudocolorAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: PseudocolorAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the PseudocolorAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PseudocolorAttributes& 
PseudocolorAttributes::operator = (const PseudocolorAttributes &obj)
{
    if (this == &obj) return *this;

    PseudocolorAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: PseudocolorAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the PseudocolorAttributes class.
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
PseudocolorAttributes::operator == (const PseudocolorAttributes &obj) const
{
    // Create the return value
    return ((scaling == obj.scaling) &&
            (skewFactor == obj.skewFactor) &&
            (limitsMode == obj.limitsMode) &&
            (minFlag == obj.minFlag) &&
            (min == obj.min) &&
            (useBelowMinColor == obj.useBelowMinColor) &&
            (belowMinColor == obj.belowMinColor) &&
            (maxFlag == obj.maxFlag) &&
            (max == obj.max) &&
            (useAboveMaxColor == obj.useAboveMaxColor) &&
            (aboveMaxColor == obj.aboveMaxColor) &&
            (centering == obj.centering) &&
            (colorTableName == obj.colorTableName) &&
            (invertColorTable == obj.invertColorTable) &&
            (opacityType == obj.opacityType) &&
            (opacityVariable == obj.opacityVariable) &&
            (opacity == obj.opacity) &&
            (opacityVarMin == obj.opacityVarMin) &&
            (opacityVarMax == obj.opacityVarMax) &&
            (opacityVarMinFlag == obj.opacityVarMinFlag) &&
            (opacityVarMaxFlag == obj.opacityVarMaxFlag) &&
            (pointSize == obj.pointSize) &&
            (pointType == obj.pointType) &&
            (pointSizeVarEnabled == obj.pointSizeVarEnabled) &&
            (pointSizeVar == obj.pointSizeVar) &&
            (pointSizePixels == obj.pointSizePixels) &&
            (lineType == obj.lineType) &&
            (lineWidth == obj.lineWidth) &&
            (tubeResolution == obj.tubeResolution) &&
            (tubeRadiusSizeType == obj.tubeRadiusSizeType) &&
            (tubeRadiusAbsolute == obj.tubeRadiusAbsolute) &&
            (tubeRadiusBBox == obj.tubeRadiusBBox) &&
            (tubeRadiusVarEnabled == obj.tubeRadiusVarEnabled) &&
            (tubeRadiusVar == obj.tubeRadiusVar) &&
            (tubeRadiusVarRatio == obj.tubeRadiusVarRatio) &&
            (tailStyle == obj.tailStyle) &&
            (headStyle == obj.headStyle) &&
            (endPointRadiusSizeType == obj.endPointRadiusSizeType) &&
            (endPointRadiusAbsolute == obj.endPointRadiusAbsolute) &&
            (endPointRadiusBBox == obj.endPointRadiusBBox) &&
            (endPointResolution == obj.endPointResolution) &&
            (endPointRatio == obj.endPointRatio) &&
            (endPointRadiusVarEnabled == obj.endPointRadiusVarEnabled) &&
            (endPointRadiusVar == obj.endPointRadiusVar) &&
            (endPointRadiusVarRatio == obj.endPointRadiusVarRatio) &&
            (renderSurfaces == obj.renderSurfaces) &&
            (renderWireframe == obj.renderWireframe) &&
            (renderPoints == obj.renderPoints) &&
            (smoothingLevel == obj.smoothingLevel) &&
            (legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (wireframeColor == obj.wireframeColor) &&
            (pointColor == obj.pointColor));
}

// ****************************************************************************
// Method: PseudocolorAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the PseudocolorAttributes class.
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
PseudocolorAttributes::operator != (const PseudocolorAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: PseudocolorAttributes::TypeName
//
// Purpose: 
//   Type name method for the PseudocolorAttributes class.
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
PseudocolorAttributes::TypeName() const
{
    return "PseudocolorAttributes";
}

// ****************************************************************************
// Method: PseudocolorAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the PseudocolorAttributes class.
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
PseudocolorAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const PseudocolorAttributes *tmp = (const PseudocolorAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: PseudocolorAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the PseudocolorAttributes class.
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
PseudocolorAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new PseudocolorAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: PseudocolorAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the PseudocolorAttributes class.
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
PseudocolorAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new PseudocolorAttributes(*this);
    else
        retval = new PseudocolorAttributes;

    return retval;
}

// ****************************************************************************
// Method: PseudocolorAttributes::SelectAll
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
PseudocolorAttributes::SelectAll()
{
    Select(ID_scaling,                  (void *)&scaling);
    Select(ID_skewFactor,               (void *)&skewFactor);
    Select(ID_limitsMode,               (void *)&limitsMode);
    Select(ID_minFlag,                  (void *)&minFlag);
    Select(ID_min,                      (void *)&min);
    Select(ID_useBelowMinColor,         (void *)&useBelowMinColor);
    Select(ID_belowMinColor,            (void *)&belowMinColor);
    Select(ID_maxFlag,                  (void *)&maxFlag);
    Select(ID_max,                      (void *)&max);
    Select(ID_useAboveMaxColor,         (void *)&useAboveMaxColor);
    Select(ID_aboveMaxColor,            (void *)&aboveMaxColor);
    Select(ID_centering,                (void *)&centering);
    Select(ID_colorTableName,           (void *)&colorTableName);
    Select(ID_invertColorTable,         (void *)&invertColorTable);
    Select(ID_opacityType,              (void *)&opacityType);
    Select(ID_opacityVariable,          (void *)&opacityVariable);
    Select(ID_opacity,                  (void *)&opacity);
    Select(ID_opacityVarMin,            (void *)&opacityVarMin);
    Select(ID_opacityVarMax,            (void *)&opacityVarMax);
    Select(ID_opacityVarMinFlag,        (void *)&opacityVarMinFlag);
    Select(ID_opacityVarMaxFlag,        (void *)&opacityVarMaxFlag);
    Select(ID_pointSize,                (void *)&pointSize);
    Select(ID_pointType,                (void *)&pointType);
    Select(ID_pointSizeVarEnabled,      (void *)&pointSizeVarEnabled);
    Select(ID_pointSizeVar,             (void *)&pointSizeVar);
    Select(ID_pointSizePixels,          (void *)&pointSizePixels);
    Select(ID_lineType,                 (void *)&lineType);
    Select(ID_lineWidth,                (void *)&lineWidth);
    Select(ID_tubeResolution,           (void *)&tubeResolution);
    Select(ID_tubeRadiusSizeType,       (void *)&tubeRadiusSizeType);
    Select(ID_tubeRadiusAbsolute,       (void *)&tubeRadiusAbsolute);
    Select(ID_tubeRadiusBBox,           (void *)&tubeRadiusBBox);
    Select(ID_tubeRadiusVarEnabled,     (void *)&tubeRadiusVarEnabled);
    Select(ID_tubeRadiusVar,            (void *)&tubeRadiusVar);
    Select(ID_tubeRadiusVarRatio,       (void *)&tubeRadiusVarRatio);
    Select(ID_tailStyle,                (void *)&tailStyle);
    Select(ID_headStyle,                (void *)&headStyle);
    Select(ID_endPointRadiusSizeType,   (void *)&endPointRadiusSizeType);
    Select(ID_endPointRadiusAbsolute,   (void *)&endPointRadiusAbsolute);
    Select(ID_endPointRadiusBBox,       (void *)&endPointRadiusBBox);
    Select(ID_endPointResolution,       (void *)&endPointResolution);
    Select(ID_endPointRatio,            (void *)&endPointRatio);
    Select(ID_endPointRadiusVarEnabled, (void *)&endPointRadiusVarEnabled);
    Select(ID_endPointRadiusVar,        (void *)&endPointRadiusVar);
    Select(ID_endPointRadiusVarRatio,   (void *)&endPointRadiusVarRatio);
    Select(ID_renderSurfaces,           (void *)&renderSurfaces);
    Select(ID_renderWireframe,          (void *)&renderWireframe);
    Select(ID_renderPoints,             (void *)&renderPoints);
    Select(ID_smoothingLevel,           (void *)&smoothingLevel);
    Select(ID_legendFlag,               (void *)&legendFlag);
    Select(ID_lightingFlag,             (void *)&lightingFlag);
    Select(ID_wireframeColor,           (void *)&wireframeColor);
    Select(ID_pointColor,               (void *)&pointColor);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PseudocolorAttributes::CreateNode
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
PseudocolorAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    PseudocolorAttributes defaultObject;
    bool addToParent = false;
    // Create a node for PseudocolorAttributes.
    DataNode *node = new DataNode("PseudocolorAttributes");

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

    if(completeSave || !FieldsEqual(ID_minFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minFlag", minFlag));
    }

    if(completeSave || !FieldsEqual(ID_min, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("min", min));
    }

    if(completeSave || !FieldsEqual(ID_useBelowMinColor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useBelowMinColor", useBelowMinColor));
    }

        DataNode *belowMinColorNode = new DataNode("belowMinColor");
        if(belowMinColor.CreateNode(belowMinColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(belowMinColorNode);
        }
        else
            delete belowMinColorNode;
    if(completeSave || !FieldsEqual(ID_maxFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxFlag", maxFlag));
    }

    if(completeSave || !FieldsEqual(ID_max, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("max", max));
    }

    if(completeSave || !FieldsEqual(ID_useAboveMaxColor, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useAboveMaxColor", useAboveMaxColor));
    }

        DataNode *aboveMaxColorNode = new DataNode("aboveMaxColor");
        if(aboveMaxColor.CreateNode(aboveMaxColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(aboveMaxColorNode);
        }
        else
            delete aboveMaxColorNode;
    if(completeSave || !FieldsEqual(ID_centering, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("centering", Centering_ToString(centering)));
    }

    if(completeSave || !FieldsEqual(ID_colorTableName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("colorTableName", colorTableName));
    }

    if(completeSave || !FieldsEqual(ID_invertColorTable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("invertColorTable", invertColorTable));
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

    if(completeSave || !FieldsEqual(ID_pointSize, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSize", pointSize));
    }

    if(completeSave || !FieldsEqual(ID_pointType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointType", pointType));
    }

    if(completeSave || !FieldsEqual(ID_pointSizeVarEnabled, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizeVarEnabled", pointSizeVarEnabled));
    }

    if(completeSave || !FieldsEqual(ID_pointSizeVar, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizeVar", pointSizeVar));
    }

    if(completeSave || !FieldsEqual(ID_pointSizePixels, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("pointSizePixels", pointSizePixels));
    }

    if(completeSave || !FieldsEqual(ID_lineType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineType", LineType_ToString(lineType)));
    }

    if(completeSave || !FieldsEqual(ID_lineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineWidth", lineWidth));
    }

    if(completeSave || !FieldsEqual(ID_tubeResolution, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeResolution", tubeResolution));
    }

    if(completeSave || !FieldsEqual(ID_tubeRadiusSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeRadiusSizeType", SizeType_ToString(tubeRadiusSizeType)));
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

    if(completeSave || !FieldsEqual(ID_tubeRadiusVarEnabled, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeRadiusVarEnabled", tubeRadiusVarEnabled));
    }

    if(completeSave || !FieldsEqual(ID_tubeRadiusVar, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeRadiusVar", tubeRadiusVar));
    }

    if(completeSave || !FieldsEqual(ID_tubeRadiusVarRatio, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tubeRadiusVarRatio", tubeRadiusVarRatio));
    }

    if(completeSave || !FieldsEqual(ID_tailStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("tailStyle", EndPointStyle_ToString(tailStyle)));
    }

    if(completeSave || !FieldsEqual(ID_headStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("headStyle", EndPointStyle_ToString(headStyle)));
    }

    if(completeSave || !FieldsEqual(ID_endPointRadiusSizeType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointRadiusSizeType", SizeType_ToString(endPointRadiusSizeType)));
    }

    if(completeSave || !FieldsEqual(ID_endPointRadiusAbsolute, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointRadiusAbsolute", endPointRadiusAbsolute));
    }

    if(completeSave || !FieldsEqual(ID_endPointRadiusBBox, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointRadiusBBox", endPointRadiusBBox));
    }

    if(completeSave || !FieldsEqual(ID_endPointResolution, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointResolution", endPointResolution));
    }

    if(completeSave || !FieldsEqual(ID_endPointRatio, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointRatio", endPointRatio));
    }

    if(completeSave || !FieldsEqual(ID_endPointRadiusVarEnabled, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointRadiusVarEnabled", endPointRadiusVarEnabled));
    }

    if(completeSave || !FieldsEqual(ID_endPointRadiusVar, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointRadiusVar", endPointRadiusVar));
    }

    if(completeSave || !FieldsEqual(ID_endPointRadiusVarRatio, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("endPointRadiusVarRatio", endPointRadiusVarRatio));
    }

    if(completeSave || !FieldsEqual(ID_renderSurfaces, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("renderSurfaces", renderSurfaces));
    }

    if(completeSave || !FieldsEqual(ID_renderWireframe, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("renderWireframe", renderWireframe));
    }

    if(completeSave || !FieldsEqual(ID_renderPoints, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("renderPoints", renderPoints));
    }

    if(completeSave || !FieldsEqual(ID_smoothingLevel, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("smoothingLevel", smoothingLevel));
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

        DataNode *wireframeColorNode = new DataNode("wireframeColor");
        if(wireframeColor.CreateNode(wireframeColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(wireframeColorNode);
        }
        else
            delete wireframeColorNode;
        DataNode *pointColorNode = new DataNode("pointColor");
        if(pointColor.CreateNode(pointColorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(pointColorNode);
        }
        else
            delete pointColorNode;

    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: PseudocolorAttributes::SetFromNode
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
PseudocolorAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("PseudocolorAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
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
    if((node = searchNode->GetNode("minFlag")) != 0)
        SetMinFlag(node->AsBool());
    if((node = searchNode->GetNode("min")) != 0)
        SetMin(node->AsDouble());
    if((node = searchNode->GetNode("useBelowMinColor")) != 0)
        SetUseBelowMinColor(node->AsBool());
    if((node = searchNode->GetNode("belowMinColor")) != 0)
        belowMinColor.SetFromNode(node);
    if((node = searchNode->GetNode("maxFlag")) != 0)
        SetMaxFlag(node->AsBool());
    if((node = searchNode->GetNode("max")) != 0)
        SetMax(node->AsDouble());
    if((node = searchNode->GetNode("useAboveMaxColor")) != 0)
        SetUseAboveMaxColor(node->AsBool());
    if((node = searchNode->GetNode("aboveMaxColor")) != 0)
        aboveMaxColor.SetFromNode(node);
    if((node = searchNode->GetNode("centering")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetCentering(Centering(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Centering value;
            if(Centering_FromString(node->AsString(), value))
                SetCentering(value);
        }
    }
    if((node = searchNode->GetNode("colorTableName")) != 0)
        SetColorTableName(node->AsString());
    if((node = searchNode->GetNode("invertColorTable")) != 0)
        SetInvertColorTable(node->AsBool());
    if((node = searchNode->GetNode("opacityType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 5)
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
    if((node = searchNode->GetNode("pointSize")) != 0)
        SetPointSize(node->AsDouble());
    if((node = searchNode->GetNode("pointType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 8)
                SetPointType(GlyphType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            GlyphType value;
            if(GlyphType_FromString(node->AsString(), value))
                SetPointType(value);
        }
    }
    if((node = searchNode->GetNode("pointSizeVarEnabled")) != 0)
        SetPointSizeVarEnabled(node->AsBool());
    if((node = searchNode->GetNode("pointSizeVar")) != 0)
        SetPointSizeVar(node->AsString());
    if((node = searchNode->GetNode("pointSizePixels")) != 0)
        SetPointSizePixels(node->AsInt());
    if((node = searchNode->GetNode("lineType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetLineType(LineType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            LineType value;
            if(LineType_FromString(node->AsString(), value))
                SetLineType(value);
        }
    }
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("tubeResolution")) != 0)
        SetTubeResolution(node->AsInt());
    if((node = searchNode->GetNode("tubeRadiusSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetTubeRadiusSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetTubeRadiusSizeType(value);
        }
    }
    if((node = searchNode->GetNode("tubeRadiusAbsolute")) != 0)
        SetTubeRadiusAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("tubeRadiusBBox")) != 0)
        SetTubeRadiusBBox(node->AsDouble());
    if((node = searchNode->GetNode("tubeRadiusVarEnabled")) != 0)
        SetTubeRadiusVarEnabled(node->AsBool());
    if((node = searchNode->GetNode("tubeRadiusVar")) != 0)
        SetTubeRadiusVar(node->AsString());
    if((node = searchNode->GetNode("tubeRadiusVarRatio")) != 0)
        SetTubeRadiusVarRatio(node->AsDouble());
    if((node = searchNode->GetNode("tailStyle")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetTailStyle(EndPointStyle(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            EndPointStyle value;
            if(EndPointStyle_FromString(node->AsString(), value))
                SetTailStyle(value);
        }
    }
    if((node = searchNode->GetNode("headStyle")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetHeadStyle(EndPointStyle(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            EndPointStyle value;
            if(EndPointStyle_FromString(node->AsString(), value))
                SetHeadStyle(value);
        }
    }
    if((node = searchNode->GetNode("endPointRadiusSizeType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetEndPointRadiusSizeType(SizeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SizeType value;
            if(SizeType_FromString(node->AsString(), value))
                SetEndPointRadiusSizeType(value);
        }
    }
    if((node = searchNode->GetNode("endPointRadiusAbsolute")) != 0)
        SetEndPointRadiusAbsolute(node->AsDouble());
    if((node = searchNode->GetNode("endPointRadiusBBox")) != 0)
        SetEndPointRadiusBBox(node->AsDouble());
    if((node = searchNode->GetNode("endPointResolution")) != 0)
        SetEndPointResolution(node->AsInt());
    if((node = searchNode->GetNode("endPointRatio")) != 0)
        SetEndPointRatio(node->AsDouble());
    if((node = searchNode->GetNode("endPointRadiusVarEnabled")) != 0)
        SetEndPointRadiusVarEnabled(node->AsBool());
    if((node = searchNode->GetNode("endPointRadiusVar")) != 0)
        SetEndPointRadiusVar(node->AsString());
    if((node = searchNode->GetNode("endPointRadiusVarRatio")) != 0)
        SetEndPointRadiusVarRatio(node->AsDouble());
    if((node = searchNode->GetNode("renderSurfaces")) != 0)
        SetRenderSurfaces(node->AsInt());
    if((node = searchNode->GetNode("renderWireframe")) != 0)
        SetRenderWireframe(node->AsInt());
    if((node = searchNode->GetNode("renderPoints")) != 0)
        SetRenderPoints(node->AsInt());
    if((node = searchNode->GetNode("smoothingLevel")) != 0)
        SetSmoothingLevel(node->AsInt());
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("wireframeColor")) != 0)
        wireframeColor.SetFromNode(node);
    if((node = searchNode->GetNode("pointColor")) != 0)
        pointColor.SetFromNode(node);
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
PseudocolorAttributes::SetScaling(PseudocolorAttributes::Scaling scaling_)
{
    scaling = scaling_;
    Select(ID_scaling, (void *)&scaling);
}

void
PseudocolorAttributes::SetSkewFactor(double skewFactor_)
{
    skewFactor = skewFactor_;
    Select(ID_skewFactor, (void *)&skewFactor);
}

void
PseudocolorAttributes::SetLimitsMode(PseudocolorAttributes::LimitsMode limitsMode_)
{
    limitsMode = limitsMode_;
    Select(ID_limitsMode, (void *)&limitsMode);
}

void
PseudocolorAttributes::SetMinFlag(bool minFlag_)
{
    minFlag = minFlag_;
    Select(ID_minFlag, (void *)&minFlag);
}

void
PseudocolorAttributes::SetMin(double min_)
{
    min = min_;
    Select(ID_min, (void *)&min);
}

void
PseudocolorAttributes::SetUseBelowMinColor(bool useBelowMinColor_)
{
    useBelowMinColor = useBelowMinColor_;
    Select(ID_useBelowMinColor, (void *)&useBelowMinColor);
}

void
PseudocolorAttributes::SetBelowMinColor(const ColorAttribute &belowMinColor_)
{
    belowMinColor = belowMinColor_;
    Select(ID_belowMinColor, (void *)&belowMinColor);
}

void
PseudocolorAttributes::SetMaxFlag(bool maxFlag_)
{
    maxFlag = maxFlag_;
    Select(ID_maxFlag, (void *)&maxFlag);
}

void
PseudocolorAttributes::SetMax(double max_)
{
    max = max_;
    Select(ID_max, (void *)&max);
}

void
PseudocolorAttributes::SetUseAboveMaxColor(bool useAboveMaxColor_)
{
    useAboveMaxColor = useAboveMaxColor_;
    Select(ID_useAboveMaxColor, (void *)&useAboveMaxColor);
}

void
PseudocolorAttributes::SetAboveMaxColor(const ColorAttribute &aboveMaxColor_)
{
    aboveMaxColor = aboveMaxColor_;
    Select(ID_aboveMaxColor, (void *)&aboveMaxColor);
}

void
PseudocolorAttributes::SetCentering(PseudocolorAttributes::Centering centering_)
{
    centering = centering_;
    Select(ID_centering, (void *)&centering);
}

void
PseudocolorAttributes::SetColorTableName(const std::string &colorTableName_)
{
    colorTableName = colorTableName_;
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PseudocolorAttributes::SetInvertColorTable(bool invertColorTable_)
{
    invertColorTable = invertColorTable_;
    Select(ID_invertColorTable, (void *)&invertColorTable);
}

void
PseudocolorAttributes::SetOpacityType(PseudocolorAttributes::OpacityType opacityType_)
{
    opacityType = opacityType_;
    Select(ID_opacityType, (void *)&opacityType);
}

void
PseudocolorAttributes::SetOpacityVariable(const std::string &opacityVariable_)
{
    opacityVariable = opacityVariable_;
    Select(ID_opacityVariable, (void *)&opacityVariable);
}

void
PseudocolorAttributes::SetOpacity(double opacity_)
{
    opacity = opacity_;
    Select(ID_opacity, (void *)&opacity);
}

void
PseudocolorAttributes::SetOpacityVarMin(double opacityVarMin_)
{
    opacityVarMin = opacityVarMin_;
    Select(ID_opacityVarMin, (void *)&opacityVarMin);
}

void
PseudocolorAttributes::SetOpacityVarMax(double opacityVarMax_)
{
    opacityVarMax = opacityVarMax_;
    Select(ID_opacityVarMax, (void *)&opacityVarMax);
}

void
PseudocolorAttributes::SetOpacityVarMinFlag(bool opacityVarMinFlag_)
{
    opacityVarMinFlag = opacityVarMinFlag_;
    Select(ID_opacityVarMinFlag, (void *)&opacityVarMinFlag);
}

void
PseudocolorAttributes::SetOpacityVarMaxFlag(bool opacityVarMaxFlag_)
{
    opacityVarMaxFlag = opacityVarMaxFlag_;
    Select(ID_opacityVarMaxFlag, (void *)&opacityVarMaxFlag);
}

void
PseudocolorAttributes::SetPointSize(double pointSize_)
{
    pointSize = pointSize_;
    Select(ID_pointSize, (void *)&pointSize);
}

void
PseudocolorAttributes::SetPointType(GlyphType pointType_)
{
    pointType = pointType_;
    Select(ID_pointType, (void *)&pointType);
}

void
PseudocolorAttributes::SetPointSizeVarEnabled(bool pointSizeVarEnabled_)
{
    pointSizeVarEnabled = pointSizeVarEnabled_;
    Select(ID_pointSizeVarEnabled, (void *)&pointSizeVarEnabled);
}

void
PseudocolorAttributes::SetPointSizeVar(const std::string &pointSizeVar_)
{
    pointSizeVar = pointSizeVar_;
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

void
PseudocolorAttributes::SetPointSizePixels(int pointSizePixels_)
{
    pointSizePixels = pointSizePixels_;
    Select(ID_pointSizePixels, (void *)&pointSizePixels);
}

void
PseudocolorAttributes::SetLineType(PseudocolorAttributes::LineType lineType_)
{
    lineType = lineType_;
    Select(ID_lineType, (void *)&lineType);
}

void
PseudocolorAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
PseudocolorAttributes::SetTubeResolution(int tubeResolution_)
{
    tubeResolution = tubeResolution_;
    Select(ID_tubeResolution, (void *)&tubeResolution);
}

void
PseudocolorAttributes::SetTubeRadiusSizeType(PseudocolorAttributes::SizeType tubeRadiusSizeType_)
{
    tubeRadiusSizeType = tubeRadiusSizeType_;
    Select(ID_tubeRadiusSizeType, (void *)&tubeRadiusSizeType);
}

void
PseudocolorAttributes::SetTubeRadiusAbsolute(double tubeRadiusAbsolute_)
{
    tubeRadiusAbsolute = tubeRadiusAbsolute_;
    Select(ID_tubeRadiusAbsolute, (void *)&tubeRadiusAbsolute);
}

void
PseudocolorAttributes::SetTubeRadiusBBox(double tubeRadiusBBox_)
{
    tubeRadiusBBox = tubeRadiusBBox_;
    Select(ID_tubeRadiusBBox, (void *)&tubeRadiusBBox);
}

void
PseudocolorAttributes::SetTubeRadiusVarEnabled(bool tubeRadiusVarEnabled_)
{
    tubeRadiusVarEnabled = tubeRadiusVarEnabled_;
    Select(ID_tubeRadiusVarEnabled, (void *)&tubeRadiusVarEnabled);
}

void
PseudocolorAttributes::SetTubeRadiusVar(const std::string &tubeRadiusVar_)
{
    tubeRadiusVar = tubeRadiusVar_;
    Select(ID_tubeRadiusVar, (void *)&tubeRadiusVar);
}

void
PseudocolorAttributes::SetTubeRadiusVarRatio(double tubeRadiusVarRatio_)
{
    tubeRadiusVarRatio = tubeRadiusVarRatio_;
    Select(ID_tubeRadiusVarRatio, (void *)&tubeRadiusVarRatio);
}

void
PseudocolorAttributes::SetTailStyle(PseudocolorAttributes::EndPointStyle tailStyle_)
{
    tailStyle = tailStyle_;
    Select(ID_tailStyle, (void *)&tailStyle);
}

void
PseudocolorAttributes::SetHeadStyle(PseudocolorAttributes::EndPointStyle headStyle_)
{
    headStyle = headStyle_;
    Select(ID_headStyle, (void *)&headStyle);
}

void
PseudocolorAttributes::SetEndPointRadiusSizeType(PseudocolorAttributes::SizeType endPointRadiusSizeType_)
{
    endPointRadiusSizeType = endPointRadiusSizeType_;
    Select(ID_endPointRadiusSizeType, (void *)&endPointRadiusSizeType);
}

void
PseudocolorAttributes::SetEndPointRadiusAbsolute(double endPointRadiusAbsolute_)
{
    endPointRadiusAbsolute = endPointRadiusAbsolute_;
    Select(ID_endPointRadiusAbsolute, (void *)&endPointRadiusAbsolute);
}

void
PseudocolorAttributes::SetEndPointRadiusBBox(double endPointRadiusBBox_)
{
    endPointRadiusBBox = endPointRadiusBBox_;
    Select(ID_endPointRadiusBBox, (void *)&endPointRadiusBBox);
}

void
PseudocolorAttributes::SetEndPointResolution(int endPointResolution_)
{
    endPointResolution = endPointResolution_;
    Select(ID_endPointResolution, (void *)&endPointResolution);
}

void
PseudocolorAttributes::SetEndPointRatio(double endPointRatio_)
{
    endPointRatio = endPointRatio_;
    Select(ID_endPointRatio, (void *)&endPointRatio);
}

void
PseudocolorAttributes::SetEndPointRadiusVarEnabled(bool endPointRadiusVarEnabled_)
{
    endPointRadiusVarEnabled = endPointRadiusVarEnabled_;
    Select(ID_endPointRadiusVarEnabled, (void *)&endPointRadiusVarEnabled);
}

void
PseudocolorAttributes::SetEndPointRadiusVar(const std::string &endPointRadiusVar_)
{
    endPointRadiusVar = endPointRadiusVar_;
    Select(ID_endPointRadiusVar, (void *)&endPointRadiusVar);
}

void
PseudocolorAttributes::SetEndPointRadiusVarRatio(double endPointRadiusVarRatio_)
{
    endPointRadiusVarRatio = endPointRadiusVarRatio_;
    Select(ID_endPointRadiusVarRatio, (void *)&endPointRadiusVarRatio);
}

void
PseudocolorAttributes::SetRenderSurfaces(int renderSurfaces_)
{
    renderSurfaces = renderSurfaces_;
    Select(ID_renderSurfaces, (void *)&renderSurfaces);
}

void
PseudocolorAttributes::SetRenderWireframe(int renderWireframe_)
{
    renderWireframe = renderWireframe_;
    Select(ID_renderWireframe, (void *)&renderWireframe);
}

void
PseudocolorAttributes::SetRenderPoints(int renderPoints_)
{
    renderPoints = renderPoints_;
    Select(ID_renderPoints, (void *)&renderPoints);
}

void
PseudocolorAttributes::SetSmoothingLevel(int smoothingLevel_)
{
    smoothingLevel = smoothingLevel_;
    Select(ID_smoothingLevel, (void *)&smoothingLevel);
}

void
PseudocolorAttributes::SetLegendFlag(bool legendFlag_)
{
    legendFlag = legendFlag_;
    Select(ID_legendFlag, (void *)&legendFlag);
}

void
PseudocolorAttributes::SetLightingFlag(bool lightingFlag_)
{
    lightingFlag = lightingFlag_;
    Select(ID_lightingFlag, (void *)&lightingFlag);
}

void
PseudocolorAttributes::SetWireframeColor(const ColorAttribute &wireframeColor_)
{
    wireframeColor = wireframeColor_;
    Select(ID_wireframeColor, (void *)&wireframeColor);
}

void
PseudocolorAttributes::SetPointColor(const ColorAttribute &pointColor_)
{
    pointColor = pointColor_;
    Select(ID_pointColor, (void *)&pointColor);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

PseudocolorAttributes::Scaling
PseudocolorAttributes::GetScaling() const
{
    return Scaling(scaling);
}

double
PseudocolorAttributes::GetSkewFactor() const
{
    return skewFactor;
}

PseudocolorAttributes::LimitsMode
PseudocolorAttributes::GetLimitsMode() const
{
    return LimitsMode(limitsMode);
}

bool
PseudocolorAttributes::GetMinFlag() const
{
    return minFlag;
}

double
PseudocolorAttributes::GetMin() const
{
    return min;
}

bool
PseudocolorAttributes::GetUseBelowMinColor() const
{
    return useBelowMinColor;
}

const ColorAttribute &
PseudocolorAttributes::GetBelowMinColor() const
{
    return belowMinColor;
}

ColorAttribute &
PseudocolorAttributes::GetBelowMinColor()
{
    return belowMinColor;
}

bool
PseudocolorAttributes::GetMaxFlag() const
{
    return maxFlag;
}

double
PseudocolorAttributes::GetMax() const
{
    return max;
}

bool
PseudocolorAttributes::GetUseAboveMaxColor() const
{
    return useAboveMaxColor;
}

const ColorAttribute &
PseudocolorAttributes::GetAboveMaxColor() const
{
    return aboveMaxColor;
}

ColorAttribute &
PseudocolorAttributes::GetAboveMaxColor()
{
    return aboveMaxColor;
}

PseudocolorAttributes::Centering
PseudocolorAttributes::GetCentering() const
{
    return Centering(centering);
}

const std::string &
PseudocolorAttributes::GetColorTableName() const
{
    return colorTableName;
}

std::string &
PseudocolorAttributes::GetColorTableName()
{
    return colorTableName;
}

bool
PseudocolorAttributes::GetInvertColorTable() const
{
    return invertColorTable;
}

PseudocolorAttributes::OpacityType
PseudocolorAttributes::GetOpacityType() const
{
    return OpacityType(opacityType);
}

const std::string &
PseudocolorAttributes::GetOpacityVariable() const
{
    return opacityVariable;
}

std::string &
PseudocolorAttributes::GetOpacityVariable()
{
    return opacityVariable;
}

double
PseudocolorAttributes::GetOpacity() const
{
    return opacity;
}

double
PseudocolorAttributes::GetOpacityVarMin() const
{
    return opacityVarMin;
}

double
PseudocolorAttributes::GetOpacityVarMax() const
{
    return opacityVarMax;
}

bool
PseudocolorAttributes::GetOpacityVarMinFlag() const
{
    return opacityVarMinFlag;
}

bool
PseudocolorAttributes::GetOpacityVarMaxFlag() const
{
    return opacityVarMaxFlag;
}

double
PseudocolorAttributes::GetPointSize() const
{
    return pointSize;
}

GlyphType
PseudocolorAttributes::GetPointType() const
{
    return pointType;
}

bool
PseudocolorAttributes::GetPointSizeVarEnabled() const
{
    return pointSizeVarEnabled;
}

const std::string &
PseudocolorAttributes::GetPointSizeVar() const
{
    return pointSizeVar;
}

std::string &
PseudocolorAttributes::GetPointSizeVar()
{
    return pointSizeVar;
}

int
PseudocolorAttributes::GetPointSizePixels() const
{
    return pointSizePixels;
}

PseudocolorAttributes::LineType
PseudocolorAttributes::GetLineType() const
{
    return LineType(lineType);
}

int
PseudocolorAttributes::GetLineWidth() const
{
    return lineWidth;
}

int
PseudocolorAttributes::GetTubeResolution() const
{
    return tubeResolution;
}

PseudocolorAttributes::SizeType
PseudocolorAttributes::GetTubeRadiusSizeType() const
{
    return SizeType(tubeRadiusSizeType);
}

double
PseudocolorAttributes::GetTubeRadiusAbsolute() const
{
    return tubeRadiusAbsolute;
}

double
PseudocolorAttributes::GetTubeRadiusBBox() const
{
    return tubeRadiusBBox;
}

bool
PseudocolorAttributes::GetTubeRadiusVarEnabled() const
{
    return tubeRadiusVarEnabled;
}

const std::string &
PseudocolorAttributes::GetTubeRadiusVar() const
{
    return tubeRadiusVar;
}

std::string &
PseudocolorAttributes::GetTubeRadiusVar()
{
    return tubeRadiusVar;
}

double
PseudocolorAttributes::GetTubeRadiusVarRatio() const
{
    return tubeRadiusVarRatio;
}

PseudocolorAttributes::EndPointStyle
PseudocolorAttributes::GetTailStyle() const
{
    return EndPointStyle(tailStyle);
}

PseudocolorAttributes::EndPointStyle
PseudocolorAttributes::GetHeadStyle() const
{
    return EndPointStyle(headStyle);
}

PseudocolorAttributes::SizeType
PseudocolorAttributes::GetEndPointRadiusSizeType() const
{
    return SizeType(endPointRadiusSizeType);
}

double
PseudocolorAttributes::GetEndPointRadiusAbsolute() const
{
    return endPointRadiusAbsolute;
}

double
PseudocolorAttributes::GetEndPointRadiusBBox() const
{
    return endPointRadiusBBox;
}

int
PseudocolorAttributes::GetEndPointResolution() const
{
    return endPointResolution;
}

double
PseudocolorAttributes::GetEndPointRatio() const
{
    return endPointRatio;
}

bool
PseudocolorAttributes::GetEndPointRadiusVarEnabled() const
{
    return endPointRadiusVarEnabled;
}

const std::string &
PseudocolorAttributes::GetEndPointRadiusVar() const
{
    return endPointRadiusVar;
}

std::string &
PseudocolorAttributes::GetEndPointRadiusVar()
{
    return endPointRadiusVar;
}

double
PseudocolorAttributes::GetEndPointRadiusVarRatio() const
{
    return endPointRadiusVarRatio;
}

int
PseudocolorAttributes::GetRenderSurfaces() const
{
    return renderSurfaces;
}

int
PseudocolorAttributes::GetRenderWireframe() const
{
    return renderWireframe;
}

int
PseudocolorAttributes::GetRenderPoints() const
{
    return renderPoints;
}

int
PseudocolorAttributes::GetSmoothingLevel() const
{
    return smoothingLevel;
}

bool
PseudocolorAttributes::GetLegendFlag() const
{
    return legendFlag;
}

bool
PseudocolorAttributes::GetLightingFlag() const
{
    return lightingFlag;
}

const ColorAttribute &
PseudocolorAttributes::GetWireframeColor() const
{
    return wireframeColor;
}

ColorAttribute &
PseudocolorAttributes::GetWireframeColor()
{
    return wireframeColor;
}

const ColorAttribute &
PseudocolorAttributes::GetPointColor() const
{
    return pointColor;
}

ColorAttribute &
PseudocolorAttributes::GetPointColor()
{
    return pointColor;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
PseudocolorAttributes::SelectBelowMinColor()
{
    Select(ID_belowMinColor, (void *)&belowMinColor);
}

void
PseudocolorAttributes::SelectAboveMaxColor()
{
    Select(ID_aboveMaxColor, (void *)&aboveMaxColor);
}

void
PseudocolorAttributes::SelectColorTableName()
{
    Select(ID_colorTableName, (void *)&colorTableName);
}

void
PseudocolorAttributes::SelectOpacityVariable()
{
    Select(ID_opacityVariable, (void *)&opacityVariable);
}

void
PseudocolorAttributes::SelectPointSizeVar()
{
    Select(ID_pointSizeVar, (void *)&pointSizeVar);
}

void
PseudocolorAttributes::SelectTubeRadiusVar()
{
    Select(ID_tubeRadiusVar, (void *)&tubeRadiusVar);
}

void
PseudocolorAttributes::SelectEndPointRadiusVar()
{
    Select(ID_endPointRadiusVar, (void *)&endPointRadiusVar);
}

void
PseudocolorAttributes::SelectWireframeColor()
{
    Select(ID_wireframeColor, (void *)&wireframeColor);
}

void
PseudocolorAttributes::SelectPointColor()
{
    Select(ID_pointColor, (void *)&pointColor);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PseudocolorAttributes::GetFieldName
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
PseudocolorAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_scaling:                  return "scaling";
    case ID_skewFactor:               return "skewFactor";
    case ID_limitsMode:               return "limitsMode";
    case ID_minFlag:                  return "minFlag";
    case ID_min:                      return "min";
    case ID_useBelowMinColor:         return "useBelowMinColor";
    case ID_belowMinColor:            return "belowMinColor";
    case ID_maxFlag:                  return "maxFlag";
    case ID_max:                      return "max";
    case ID_useAboveMaxColor:         return "useAboveMaxColor";
    case ID_aboveMaxColor:            return "aboveMaxColor";
    case ID_centering:                return "centering";
    case ID_colorTableName:           return "colorTableName";
    case ID_invertColorTable:         return "invertColorTable";
    case ID_opacityType:              return "opacityType";
    case ID_opacityVariable:          return "opacityVariable";
    case ID_opacity:                  return "opacity";
    case ID_opacityVarMin:            return "opacityVarMin";
    case ID_opacityVarMax:            return "opacityVarMax";
    case ID_opacityVarMinFlag:        return "opacityVarMinFlag";
    case ID_opacityVarMaxFlag:        return "opacityVarMaxFlag";
    case ID_pointSize:                return "pointSize";
    case ID_pointType:                return "pointType";
    case ID_pointSizeVarEnabled:      return "pointSizeVarEnabled";
    case ID_pointSizeVar:             return "pointSizeVar";
    case ID_pointSizePixels:          return "pointSizePixels";
    case ID_lineType:                 return "lineType";
    case ID_lineWidth:                return "lineWidth";
    case ID_tubeResolution:           return "tubeResolution";
    case ID_tubeRadiusSizeType:       return "tubeRadiusSizeType";
    case ID_tubeRadiusAbsolute:       return "tubeRadiusAbsolute";
    case ID_tubeRadiusBBox:           return "tubeRadiusBBox";
    case ID_tubeRadiusVarEnabled:     return "tubeRadiusVarEnabled";
    case ID_tubeRadiusVar:            return "tubeRadiusVar";
    case ID_tubeRadiusVarRatio:       return "tubeRadiusVarRatio";
    case ID_tailStyle:                return "tailStyle";
    case ID_headStyle:                return "headStyle";
    case ID_endPointRadiusSizeType:   return "endPointRadiusSizeType";
    case ID_endPointRadiusAbsolute:   return "endPointRadiusAbsolute";
    case ID_endPointRadiusBBox:       return "endPointRadiusBBox";
    case ID_endPointResolution:       return "endPointResolution";
    case ID_endPointRatio:            return "endPointRatio";
    case ID_endPointRadiusVarEnabled: return "endPointRadiusVarEnabled";
    case ID_endPointRadiusVar:        return "endPointRadiusVar";
    case ID_endPointRadiusVarRatio:   return "endPointRadiusVarRatio";
    case ID_renderSurfaces:           return "renderSurfaces";
    case ID_renderWireframe:          return "renderWireframe";
    case ID_renderPoints:             return "renderPoints";
    case ID_smoothingLevel:           return "smoothingLevel";
    case ID_legendFlag:               return "legendFlag";
    case ID_lightingFlag:             return "lightingFlag";
    case ID_wireframeColor:           return "wireframeColor";
    case ID_pointColor:               return "pointColor";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PseudocolorAttributes::GetFieldType
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
PseudocolorAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_scaling:                  return FieldType_enum;
    case ID_skewFactor:               return FieldType_double;
    case ID_limitsMode:               return FieldType_enum;
    case ID_minFlag:                  return FieldType_bool;
    case ID_min:                      return FieldType_double;
    case ID_useBelowMinColor:         return FieldType_bool;
    case ID_belowMinColor:            return FieldType_color;
    case ID_maxFlag:                  return FieldType_bool;
    case ID_max:                      return FieldType_double;
    case ID_useAboveMaxColor:         return FieldType_bool;
    case ID_aboveMaxColor:            return FieldType_color;
    case ID_centering:                return FieldType_enum;
    case ID_colorTableName:           return FieldType_colortable;
    case ID_invertColorTable:         return FieldType_bool;
    case ID_opacityType:              return FieldType_enum;
    case ID_opacityVariable:          return FieldType_string;
    case ID_opacity:                  return FieldType_double;
    case ID_opacityVarMin:            return FieldType_double;
    case ID_opacityVarMax:            return FieldType_double;
    case ID_opacityVarMinFlag:        return FieldType_bool;
    case ID_opacityVarMaxFlag:        return FieldType_bool;
    case ID_pointSize:                return FieldType_double;
    case ID_pointType:                return FieldType_glyphtype;
    case ID_pointSizeVarEnabled:      return FieldType_bool;
    case ID_pointSizeVar:             return FieldType_variablename;
    case ID_pointSizePixels:          return FieldType_int;
    case ID_lineType:                 return FieldType_enum;
    case ID_lineWidth:                return FieldType_linewidth;
    case ID_tubeResolution:           return FieldType_int;
    case ID_tubeRadiusSizeType:       return FieldType_enum;
    case ID_tubeRadiusAbsolute:       return FieldType_double;
    case ID_tubeRadiusBBox:           return FieldType_double;
    case ID_tubeRadiusVarEnabled:     return FieldType_bool;
    case ID_tubeRadiusVar:            return FieldType_string;
    case ID_tubeRadiusVarRatio:       return FieldType_double;
    case ID_tailStyle:                return FieldType_enum;
    case ID_headStyle:                return FieldType_enum;
    case ID_endPointRadiusSizeType:   return FieldType_enum;
    case ID_endPointRadiusAbsolute:   return FieldType_double;
    case ID_endPointRadiusBBox:       return FieldType_double;
    case ID_endPointResolution:       return FieldType_int;
    case ID_endPointRatio:            return FieldType_double;
    case ID_endPointRadiusVarEnabled: return FieldType_bool;
    case ID_endPointRadiusVar:        return FieldType_string;
    case ID_endPointRadiusVarRatio:   return FieldType_double;
    case ID_renderSurfaces:           return FieldType_int;
    case ID_renderWireframe:          return FieldType_int;
    case ID_renderPoints:             return FieldType_int;
    case ID_smoothingLevel:           return FieldType_int;
    case ID_legendFlag:               return FieldType_bool;
    case ID_lightingFlag:             return FieldType_bool;
    case ID_wireframeColor:           return FieldType_color;
    case ID_pointColor:               return FieldType_color;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: PseudocolorAttributes::GetFieldTypeName
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
PseudocolorAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_scaling:                  return "enum";
    case ID_skewFactor:               return "double";
    case ID_limitsMode:               return "enum";
    case ID_minFlag:                  return "bool";
    case ID_min:                      return "double";
    case ID_useBelowMinColor:         return "bool";
    case ID_belowMinColor:            return "color";
    case ID_maxFlag:                  return "bool";
    case ID_max:                      return "double";
    case ID_useAboveMaxColor:         return "bool";
    case ID_aboveMaxColor:            return "color";
    case ID_centering:                return "enum";
    case ID_colorTableName:           return "colortable";
    case ID_invertColorTable:         return "bool";
    case ID_opacityType:              return "enum";
    case ID_opacityVariable:          return "string";
    case ID_opacity:                  return "double";
    case ID_opacityVarMin:            return "double";
    case ID_opacityVarMax:            return "double";
    case ID_opacityVarMinFlag:        return "bool";
    case ID_opacityVarMaxFlag:        return "bool";
    case ID_pointSize:                return "double";
    case ID_pointType:                return "glyphtype";
    case ID_pointSizeVarEnabled:      return "bool";
    case ID_pointSizeVar:             return "variablename";
    case ID_pointSizePixels:          return "int";
    case ID_lineType:                 return "enum";
    case ID_lineWidth:                return "linewidth";
    case ID_tubeResolution:           return "int";
    case ID_tubeRadiusSizeType:       return "enum";
    case ID_tubeRadiusAbsolute:       return "double";
    case ID_tubeRadiusBBox:           return "double";
    case ID_tubeRadiusVarEnabled:     return "bool";
    case ID_tubeRadiusVar:            return "string";
    case ID_tubeRadiusVarRatio:       return "double";
    case ID_tailStyle:                return "enum";
    case ID_headStyle:                return "enum";
    case ID_endPointRadiusSizeType:   return "enum";
    case ID_endPointRadiusAbsolute:   return "double";
    case ID_endPointRadiusBBox:       return "double";
    case ID_endPointResolution:       return "int";
    case ID_endPointRatio:            return "double";
    case ID_endPointRadiusVarEnabled: return "bool";
    case ID_endPointRadiusVar:        return "string";
    case ID_endPointRadiusVarRatio:   return "double";
    case ID_renderSurfaces:           return "int";
    case ID_renderWireframe:          return "int";
    case ID_renderPoints:             return "int";
    case ID_smoothingLevel:           return "int";
    case ID_legendFlag:               return "bool";
    case ID_lightingFlag:             return "bool";
    case ID_wireframeColor:           return "color";
    case ID_pointColor:               return "color";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PseudocolorAttributes::FieldsEqual
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
PseudocolorAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const PseudocolorAttributes &obj = *((const PseudocolorAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
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
    case ID_minFlag:
        {  // new scope
        retval = (minFlag == obj.minFlag);
        }
        break;
    case ID_min:
        {  // new scope
        retval = (min == obj.min);
        }
        break;
    case ID_useBelowMinColor:
        {  // new scope
        retval = (useBelowMinColor == obj.useBelowMinColor);
        }
        break;
    case ID_belowMinColor:
        {  // new scope
        retval = (belowMinColor == obj.belowMinColor);
        }
        break;
    case ID_maxFlag:
        {  // new scope
        retval = (maxFlag == obj.maxFlag);
        }
        break;
    case ID_max:
        {  // new scope
        retval = (max == obj.max);
        }
        break;
    case ID_useAboveMaxColor:
        {  // new scope
        retval = (useAboveMaxColor == obj.useAboveMaxColor);
        }
        break;
    case ID_aboveMaxColor:
        {  // new scope
        retval = (aboveMaxColor == obj.aboveMaxColor);
        }
        break;
    case ID_centering:
        {  // new scope
        retval = (centering == obj.centering);
        }
        break;
    case ID_colorTableName:
        {  // new scope
        retval = (colorTableName == obj.colorTableName);
        }
        break;
    case ID_invertColorTable:
        {  // new scope
        retval = (invertColorTable == obj.invertColorTable);
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
    case ID_pointSize:
        {  // new scope
        retval = (pointSize == obj.pointSize);
        }
        break;
    case ID_pointType:
        {  // new scope
        retval = (pointType == obj.pointType);
        }
        break;
    case ID_pointSizeVarEnabled:
        {  // new scope
        retval = (pointSizeVarEnabled == obj.pointSizeVarEnabled);
        }
        break;
    case ID_pointSizeVar:
        {  // new scope
        retval = (pointSizeVar == obj.pointSizeVar);
        }
        break;
    case ID_pointSizePixels:
        {  // new scope
        retval = (pointSizePixels == obj.pointSizePixels);
        }
        break;
    case ID_lineType:
        {  // new scope
        retval = (lineType == obj.lineType);
        }
        break;
    case ID_lineWidth:
        {  // new scope
        retval = (lineWidth == obj.lineWidth);
        }
        break;
    case ID_tubeResolution:
        {  // new scope
        retval = (tubeResolution == obj.tubeResolution);
        }
        break;
    case ID_tubeRadiusSizeType:
        {  // new scope
        retval = (tubeRadiusSizeType == obj.tubeRadiusSizeType);
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
    case ID_tubeRadiusVarEnabled:
        {  // new scope
        retval = (tubeRadiusVarEnabled == obj.tubeRadiusVarEnabled);
        }
        break;
    case ID_tubeRadiusVar:
        {  // new scope
        retval = (tubeRadiusVar == obj.tubeRadiusVar);
        }
        break;
    case ID_tubeRadiusVarRatio:
        {  // new scope
        retval = (tubeRadiusVarRatio == obj.tubeRadiusVarRatio);
        }
        break;
    case ID_tailStyle:
        {  // new scope
        retval = (tailStyle == obj.tailStyle);
        }
        break;
    case ID_headStyle:
        {  // new scope
        retval = (headStyle == obj.headStyle);
        }
        break;
    case ID_endPointRadiusSizeType:
        {  // new scope
        retval = (endPointRadiusSizeType == obj.endPointRadiusSizeType);
        }
        break;
    case ID_endPointRadiusAbsolute:
        {  // new scope
        retval = (endPointRadiusAbsolute == obj.endPointRadiusAbsolute);
        }
        break;
    case ID_endPointRadiusBBox:
        {  // new scope
        retval = (endPointRadiusBBox == obj.endPointRadiusBBox);
        }
        break;
    case ID_endPointResolution:
        {  // new scope
        retval = (endPointResolution == obj.endPointResolution);
        }
        break;
    case ID_endPointRatio:
        {  // new scope
        retval = (endPointRatio == obj.endPointRatio);
        }
        break;
    case ID_endPointRadiusVarEnabled:
        {  // new scope
        retval = (endPointRadiusVarEnabled == obj.endPointRadiusVarEnabled);
        }
        break;
    case ID_endPointRadiusVar:
        {  // new scope
        retval = (endPointRadiusVar == obj.endPointRadiusVar);
        }
        break;
    case ID_endPointRadiusVarRatio:
        {  // new scope
        retval = (endPointRadiusVarRatio == obj.endPointRadiusVarRatio);
        }
        break;
    case ID_renderSurfaces:
        {  // new scope
        retval = (renderSurfaces == obj.renderSurfaces);
        }
        break;
    case ID_renderWireframe:
        {  // new scope
        retval = (renderWireframe == obj.renderWireframe);
        }
        break;
    case ID_renderPoints:
        {  // new scope
        retval = (renderPoints == obj.renderPoints);
        }
        break;
    case ID_smoothingLevel:
        {  // new scope
        retval = (smoothingLevel == obj.smoothingLevel);
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
    case ID_wireframeColor:
        {  // new scope
        retval = (wireframeColor == obj.wireframeColor);
        }
        break;
    case ID_pointColor:
        {  // new scope
        retval = (pointColor == obj.pointColor);
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
// Modifications:
//
//   Kathleen Bonnell, Fri Aug 31 08:50:30 PDT 2001
//   Only centering changes require the plot to be recalculated.
//   Removed scaling, pointsize and skewFactor from test.
//
//   Hank Childs, Sun Jun 23 23:54:56 PDT 2002
//   Also recalculate if the point size changes.
//
//   Jeremy Meredith, Mon Dec  9 16:30:54 PST 2002
//   Added smoothing level.
//
//   Jeremy Meredith, Fri Dec 20 11:33:43 PST 2002
//   Added point size variable and its enabled flag.
//
//   Hank Childs, Thu Aug 21 22:28:25 PDT 2003
//   Added pointType.
//
//   Kathleen Bonnell, Wed Aug 18 18:08:52 PDT 2004
//   Removed pointSize, pointType from test.  Added new conditions for
//   recalculation when pointSizeVar or pointSizeVarEnabled have changed.
//
//   Kathleen Bonnell, Fri Nov 12 11:11:41 PST 2004
//   Modified determination of when secondary var requires recalc.
//
//   Eric Brugger, Wed Oct 26 09:36:37 PDT 2016
//   I modified the plot to support independently setting the point style
//   for the two end points of lines.
//
//   Alister Maguire, Tue Jul 16 11:00:45 PDT 2019
//   I added a check to see if transparency is allowed.
//
//   Kathleen Biagas, Tue Nov  5 08:23:09 PST 2019
//   Added check for certain pointType changes.
//
// ****************************************************************************
bool
PseudocolorAttributes::ChangesRequireRecalculation(const PseudocolorAttributes &obj) const
{
   bool needSecondaryVar = (obj.pointType != Point &&
                            obj.pointType != Sphere &&
                            obj.pointSizeVarEnabled &&
                            obj.pointSizeVar != pointSizeVar &&
                            obj.pointSizeVar != "default" &&
                            obj.pointSizeVar != "" &&
                            obj.pointSizeVar != "\0") ||

                           (obj.lineType == Tube &&
                            obj.tubeRadiusVarEnabled &&
                            obj.tubeRadiusVar != tubeRadiusVar &&
                            obj.tubeRadiusVar != "default" &&
                            obj.tubeRadiusVar != "" &&
                            obj.tubeRadiusVar != "\0") ||

                           ((obj.tailStyle != None ||
                             obj.headStyle != None) &&
                            obj.endPointRadiusVarEnabled &&
                            obj.endPointRadiusVar != endPointRadiusVar &&
                            obj.endPointRadiusVar != "default" &&
                            obj.endPointRadiusVar != "" &&
                            obj.endPointRadiusVar != "\0") ||

                           (obj.opacityType == VariableRange &&
                            obj.opacityVariable != opacityVariable &&
                            obj.opacityVariable != "default" &&
                            obj.opacityVariable != "" &&
                            obj.opacityVariable != "\0");


    bool geometryChange =  (lineType             != obj.lineType ||
                            tubeRadiusSizeType   != obj.tubeRadiusSizeType ||
                            tubeRadiusAbsolute   != obj.tubeRadiusAbsolute ||
                            tubeRadiusBBox       != obj.tubeRadiusBBox ||
                            tubeRadiusVarEnabled != obj.tubeRadiusVarEnabled ||
                            tubeRadiusVar        != obj.tubeRadiusVar ||
                            tubeRadiusVarRatio   != obj.tubeRadiusVarRatio ||
                            tubeResolution       != obj.tubeResolution ||

                            tailStyle                != obj.tailStyle ||
                            headStyle                != obj.headStyle ||
                            endPointRatio            != obj.endPointRatio ||
                            endPointRadiusSizeType   != obj.endPointRadiusSizeType ||
                            endPointRadiusAbsolute   != obj.endPointRadiusAbsolute ||
                            endPointRadiusBBox       != obj.endPointRadiusBBox ||
                            endPointRadiusVarEnabled != obj.endPointRadiusVarEnabled ||
                            endPointRadiusVar        != obj.endPointRadiusVar ||
                            endPointRadiusVarRatio   != obj.endPointRadiusVarRatio ||
                            endPointResolution       != obj.endPointResolution ||
                            0 );

    //
    // If we've switched to an opacity mode that contains transparency, we 
    // need to re-compute the face and ghost removal. 
    //
    OpacityType newOpacType = obj.GetOpacityType();
    bool alteringOpacChange = ((newOpacType != FullyOpaque &&
                                OpacityType(opacityType) == FullyOpaque) ||
                               (newOpacType == FullyOpaque &&
                                OpacityType(opacityType) != FullyOpaque));

    // Certain point type changes require a recalculation.
    bool currentPointTypeIsPoint =     pointType == Point;
    bool newPointTypeIsPoint     = obj.pointType == Point;

    bool changingPointType =  pointType != obj.pointType && currentPointTypeIsPoint != newPointTypeIsPoint;

    return (centering != obj.centering ||
            needSecondaryVar ||
            geometryChange ||
            smoothingLevel != obj.smoothingLevel ||
            renderSurfaces != obj.renderSurfaces ||
            renderWireframe != obj.renderWireframe ||
            renderPoints != obj.renderPoints ||
            wireframeColor != obj.wireframeColor ||
            pointColor != obj.pointColor ||
            alteringOpacChange ||
            changingPointType ||
            0);

}

// ****************************************************************************
// Method: PseudocolorAttributes::Print
//
// Purpose: Print the contents
//
// Programmer: Jeremy Meredith
// Creation:   September 22, 2000
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 13 13:17:26 PST 2000
//   Added opacity.
//
//   Brad Whitlock, Tue Apr 23 12:03:27 PDT 2002
//   Ported to windows.
//
// ****************************************************************************

void
PseudocolorAttributes::Print(ostream &out, bool selected_only) const
{
    out << "{";

    for(int i = 0; i < NumAttributes(); ++i)
    {
        if(selected_only && !IsSelected(i))
            continue;

        switch(i)
        {
          case  0: out << "legendFlag=" << GetLegendFlag();    break;
          case  1: out << "lightingFlag=" << GetLightingFlag();break;
          case  2: out << "minFlag=" << GetMinFlag();          break;
          case  3: out << "maxFlag=" << GetMaxFlag();          break;
          case  4: out << "centering=" << GetCentering();      break;
          case  5: out << "scaling=" << GetScaling();          break;
          case  6: out << "limitsMode=" << GetLimitsMode();    break;
          case  7: out << "min=" << GetMin();                  break;
          case  8: out << "max=" << GetMax();                  break;
          case  9: out << "pointSize=" << GetPointSize();      break;
          case 10: out << "skewFactor=" << GetSkewFactor();    break;
          case 11: out << "opacityType=" << GetOpacityType();  break;
          case 12: out << "colorTableName=" << GetColorTableName().c_str(); break;
        }
        out << ",";
    }

    out << "}";
}

// ****************************************************************************
// Method: PseudocolorAttributes::ProcessOldVersions
//
// Purpose:
//   This method handles some old fields by converting them to new fields.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 09:33:52 PST 2010
//
// Modifications:
//     Burlen Loring, Wed Jul 23 12:53:09 PDT 2014
//     fix a typing bug in patch r21912 dealing with new OpacityType
//     enumeration values added for version 2.7.0. That patch assumed
//     opacityType is stored as an integer when in reality it is stored
//     as a std::string.
//
// ****************************************************************************
#include <Utility.h>
#include <DebugStream.h>
void
PseudocolorAttributes::ProcessOldVersions(DataNode *parentNode,
    const char *configVersion)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("PseudocolorAttributes");
    if(searchNode == 0)
        return;

    if(VersionLessThan(configVersion, "2.0.0"))
    {
        DataNode *k = 0;
        if((k = searchNode->GetNode("useColorTableOpacity")) != 0)
        {
          // Previously a bool for "Explicit" or "ColorTable"
          // Now ColorTable, FullyOpaque, or Constant
          // Explicit has been split into FullyOpaque or Constant and
          // does not rely on the opacity value.

          // Assume false (prevously Explicit) is really Fully Opaque for now
          PseudocolorAttributes::OpacityType val =
            k->AsBool() ? ColorTable : FullyOpaque;

            if( val == FullyOpaque )
            {
              // If the opacity value is set and valid change to constant
              DataNode *op = 0;
              if((op = searchNode->GetNode("opacity")) != 0)
              {
                double opacity = op->AsDouble();
                if( 0.0 <= opacity && opacity < 1.0 )
                  val = Constant;
              }
            }

            // Update the opacityType to the new value.
            searchNode->RemoveNode(k, true);
            searchNode->AddNode(new DataNode("opacityType",
                                             OpacityType_ToString(val)));
        }
    }

    if(VersionLessThan(configVersion, "2.7.0"))
    {
      DataNode *k = 0;
      if((k = searchNode->GetNode("opacityType")) != 0)
      {
        // Previously "Explicit" or "ColorTable"
        // Now ColorTable, FullyOpaque, or Constant
        // Explicit has been split into FullyOpaque or Constant and
        // does not rely on the opacity value.

        // Assume Explicit is really Fully Opaque for now
        PseudocolorAttributes::OpacityType val = FullyOpaque;

        std::string strVal = k->AsString();
        if (strVal == "Explicit")
        {
            // If the opacity value is set and valid change to constant
            DataNode *op = 0;
            if((op = searchNode->GetNode("opacity")) != 0)
            {
                double opacity = op->AsDouble();
                if ((0.0 <= opacity) && (opacity < 1.0))
                {
                    val = Constant;
                }
            }
            else
            {
                val = FullyOpaque;
            }
        }
        else
        if (strVal == "ColorTable")
        {
            val = ColorTable;
        }
        else
        {
            debug1 <<
              "PseudocolorAttributes::ProcessOldVersions "
              "2.0.0 < configVersion < 2.7.0 bad value for "
              "opacityType detected " << strVal << endl;
        }

        // Update the opacityType to the new value.
        searchNode->RemoveNode(k, true);
        searchNode->AddNode(new DataNode("opacityType",
                                         OpacityType_ToString(val)));
      }

      // The opacityType may not be set as previously the default
      // was "Explicit" but the opacity value may be set and valid.
      else if((k = searchNode->GetNode("opacity")) != 0)
      {
        double opacity = k->AsDouble();

        // If the opacity value is set and valid set the opacityType
        // to constant and a new node for it.
        if( 0.0 <= opacity && opacity < 1.0 )
        {
          PseudocolorAttributes::OpacityType val = Constant;

          searchNode->AddNode(new DataNode("opacityType",
                                           OpacityType_ToString(val)));
        }
      }
    }

    if (VersionLessThan(configVersion, "3.0.0"))
    {
        if (searchNode->GetNode("lineStyle") != 0)
            searchNode->RemoveNode("lineStyle");
    }
}

