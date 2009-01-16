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

#include <VolumeAttributes.h>
#include <DataNode.h>
#include <ColorControlPoint.h>
#include <GaussianControlPoint.h>
#include <TransferFunctionWidget.h>

//
// Enum conversion methods for VolumeAttributes::Renderer
//

static const char *Renderer_strings[] = {
"Splatting", "Texture3D", "RayCasting", 
"RayCastingIntegration", "SLIVR", "Tuvok"
};

std::string
VolumeAttributes::Renderer_ToString(VolumeAttributes::Renderer t)
{
    int index = int(t);
    if(index < 0 || index >= 6) index = 0;
    return Renderer_strings[index];
}

std::string
VolumeAttributes::Renderer_ToString(int t)
{
    int index = (t < 0 || t >= 6) ? 0 : t;
    return Renderer_strings[index];
}

bool
VolumeAttributes::Renderer_FromString(const std::string &s, VolumeAttributes::Renderer &val)
{
    val = VolumeAttributes::Splatting;
    for(int i = 0; i < 6; ++i)
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
"Linear", "Log10", "Skew"
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
// Enum conversion methods for VolumeAttributes::SamplingType
//

static const char *SamplingType_strings[] = {
"KernelBased", "Rasterization"};

std::string
VolumeAttributes::SamplingType_ToString(VolumeAttributes::SamplingType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SamplingType_strings[index];
}

std::string
VolumeAttributes::SamplingType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SamplingType_strings[index];
}

bool
VolumeAttributes::SamplingType_FromString(const std::string &s, VolumeAttributes::SamplingType &val)
{
    val = VolumeAttributes::KernelBased;
    for(int i = 0; i < 2; ++i)
    {
        if(s == SamplingType_strings[i])
        {
            val = (SamplingType)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *VolumeAttributes::TypeMapFormatString = "bbafbaisUbfbfbfbfbiiiiidifa*i";

// ****************************************************************************
// Method: VolumeAttributes::VolumeAttributes
//
// Purpose: 
//   Constructor for the VolumeAttributes class.
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
    legendFlag = true;
    lightingFlag = true;
    SetDefaultColorControlPoints();
    opacityAttenuation = 1;
    freeformFlag = true;
    resampleTarget = 50000;
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
    rendererType = Splatting;
    gradientType = SobelOperator;
    num3DSlices = 200;
    scaling = Linear;
    skewFactor = 1;
    sampling = Rasterization;
    rendererSamples = 3;
    transferFunctionDim = 1;
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
    AttributeGroupVector::const_iterator pos;

    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    colorControlPoints = obj.colorControlPoints;
    opacityAttenuation = obj.opacityAttenuation;
    freeformFlag = obj.freeformFlag;
    opacityControlPoints = obj.opacityControlPoints;
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
    num3DSlices = obj.num3DSlices;
    scaling = obj.scaling;
    skewFactor = obj.skewFactor;
    sampling = obj.sampling;
    rendererSamples = obj.rendererSamples;
    // *** Copy the transferFunction2DWidgets field ***
    // Delete the AttributeGroup objects and clear the vector.
    for(pos = transferFunction2DWidgets.begin(); pos != transferFunction2DWidgets.end(); ++pos)
        delete *pos;
    transferFunction2DWidgets.clear();
    if(obj.transferFunction2DWidgets.size() > 0)
        transferFunction2DWidgets.reserve(obj.transferFunction2DWidgets.size());
    // Duplicate the transferFunction2DWidgets from obj.
    for(pos = obj.transferFunction2DWidgets.begin(); pos != obj.transferFunction2DWidgets.end(); ++pos)
    {
        TransferFunctionWidget *oldTransferFunctionWidget = (TransferFunctionWidget *)(*pos);
        TransferFunctionWidget *newTransferFunctionWidget = new TransferFunctionWidget(*oldTransferFunctionWidget);
        transferFunction2DWidgets.push_back(newTransferFunctionWidget);
    }

    transferFunctionDim = obj.transferFunctionDim;

    SelectAll();
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
    AttributeGroupVector::iterator pos;

    // Destroy the transferFunction2DWidgets field.
    for(pos = transferFunction2DWidgets.begin(); pos != transferFunction2DWidgets.end(); ++pos)
        delete *pos;
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
    AttributeGroupVector::const_iterator pos;

    legendFlag = obj.legendFlag;
    lightingFlag = obj.lightingFlag;
    colorControlPoints = obj.colorControlPoints;
    opacityAttenuation = obj.opacityAttenuation;
    freeformFlag = obj.freeformFlag;
    opacityControlPoints = obj.opacityControlPoints;
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
    num3DSlices = obj.num3DSlices;
    scaling = obj.scaling;
    skewFactor = obj.skewFactor;
    sampling = obj.sampling;
    rendererSamples = obj.rendererSamples;
    // *** Copy the transferFunction2DWidgets field ***
    // Delete the AttributeGroup objects and clear the vector.
    for(pos = transferFunction2DWidgets.begin(); pos != transferFunction2DWidgets.end(); ++pos)
        delete *pos;
    transferFunction2DWidgets.clear();
    if(obj.transferFunction2DWidgets.size() > 0)
        transferFunction2DWidgets.reserve(obj.transferFunction2DWidgets.size());
    // Duplicate the transferFunction2DWidgets from obj.
    for(pos = obj.transferFunction2DWidgets.begin(); pos != obj.transferFunction2DWidgets.end(); ++pos)
    {
        TransferFunctionWidget *oldTransferFunctionWidget = (TransferFunctionWidget *)(*pos);
        TransferFunctionWidget *newTransferFunctionWidget = new TransferFunctionWidget(*oldTransferFunctionWidget);
        transferFunction2DWidgets.push_back(newTransferFunctionWidget);
    }

    transferFunctionDim = obj.transferFunctionDim;

    SelectAll();
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

    bool transferFunction2DWidgets_equal = (obj.transferFunction2DWidgets.size() == transferFunction2DWidgets.size());
    for(size_t i = 0; (i < transferFunction2DWidgets.size()) && transferFunction2DWidgets_equal; ++i)
    {
        // Make references to TransferFunctionWidget from AttributeGroup *.
        const TransferFunctionWidget &transferFunction2DWidgets1 = *((const TransferFunctionWidget *)(transferFunction2DWidgets[i]));
        const TransferFunctionWidget &transferFunction2DWidgets2 = *((const TransferFunctionWidget *)(obj.transferFunction2DWidgets[i]));
        transferFunction2DWidgets_equal = (transferFunction2DWidgets1 == transferFunction2DWidgets2);
    }

    // Create the return value
    return ((legendFlag == obj.legendFlag) &&
            (lightingFlag == obj.lightingFlag) &&
            (colorControlPoints == obj.colorControlPoints) &&
            (opacityAttenuation == obj.opacityAttenuation) &&
            (freeformFlag == obj.freeformFlag) &&
            (opacityControlPoints == obj.opacityControlPoints) &&
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
            (num3DSlices == obj.num3DSlices) &&
            (scaling == obj.scaling) &&
            (skewFactor == obj.skewFactor) &&
            (sampling == obj.sampling) &&
            (rendererSamples == obj.rendererSamples) &&
            transferFunction2DWidgets_equal &&
            (transferFunctionDim == obj.transferFunctionDim));
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
    Select(ID_legendFlag,                (void *)&legendFlag);
    Select(ID_lightingFlag,              (void *)&lightingFlag);
    Select(ID_colorControlPoints,        (void *)&colorControlPoints);
    Select(ID_opacityAttenuation,        (void *)&opacityAttenuation);
    Select(ID_freeformFlag,              (void *)&freeformFlag);
    Select(ID_opacityControlPoints,      (void *)&opacityControlPoints);
    Select(ID_resampleTarget,            (void *)&resampleTarget);
    Select(ID_opacityVariable,           (void *)&opacityVariable);
    Select(ID_freeformOpacity,           (void *)freeformOpacity, 256);
    Select(ID_useColorVarMin,            (void *)&useColorVarMin);
    Select(ID_colorVarMin,               (void *)&colorVarMin);
    Select(ID_useColorVarMax,            (void *)&useColorVarMax);
    Select(ID_colorVarMax,               (void *)&colorVarMax);
    Select(ID_useOpacityVarMin,          (void *)&useOpacityVarMin);
    Select(ID_opacityVarMin,             (void *)&opacityVarMin);
    Select(ID_useOpacityVarMax,          (void *)&useOpacityVarMax);
    Select(ID_opacityVarMax,             (void *)&opacityVarMax);
    Select(ID_smoothData,                (void *)&smoothData);
    Select(ID_samplesPerRay,             (void *)&samplesPerRay);
    Select(ID_rendererType,              (void *)&rendererType);
    Select(ID_gradientType,              (void *)&gradientType);
    Select(ID_num3DSlices,               (void *)&num3DSlices);
    Select(ID_scaling,                   (void *)&scaling);
    Select(ID_skewFactor,                (void *)&skewFactor);
    Select(ID_sampling,                  (void *)&sampling);
    Select(ID_rendererSamples,           (void *)&rendererSamples);
    Select(ID_transferFunction2DWidgets, (void *)&transferFunction2DWidgets);
    Select(ID_transferFunctionDim,       (void *)&transferFunctionDim);
}

// ****************************************************************************
// Method: VolumeAttributes::CreateSubAttributeGroup
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

AttributeGroup *
VolumeAttributes::CreateSubAttributeGroup(int)
{
    return new TransferFunctionWidget;
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

    if(completeSave || !FieldsEqual(ID_freeformFlag, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("freeformFlag", freeformFlag));
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

    if(completeSave || !FieldsEqual(ID_num3DSlices, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("num3DSlices", num3DSlices));
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

    if(completeSave || !FieldsEqual(ID_transferFunction2DWidgets, &defaultObject))
    {
        addToParent = true;
        for(size_t i = 0; i < transferFunction2DWidgets.size(); ++i)
            transferFunction2DWidgets[i]->CreateNode(node, completeSave, true);
    }

    if(completeSave || !FieldsEqual(ID_transferFunctionDim, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("transferFunctionDim", transferFunctionDim));
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
    DataNode **children;
    if((node = searchNode->GetNode("legendFlag")) != 0)
        SetLegendFlag(node->AsBool());
    if((node = searchNode->GetNode("lightingFlag")) != 0)
        SetLightingFlag(node->AsBool());
    if((node = searchNode->GetNode("colorControlPoints")) != 0)
        colorControlPoints.SetFromNode(node);
    if((node = searchNode->GetNode("opacityAttenuation")) != 0)
        SetOpacityAttenuation(node->AsFloat());
    if((node = searchNode->GetNode("freeformFlag")) != 0)
        SetFreeformFlag(node->AsBool());
    if((node = searchNode->GetNode("opacityControlPoints")) != 0)
        opacityControlPoints.SetFromNode(node);
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
            if(ival >= 0 && ival < 6)
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
    if((node = searchNode->GetNode("num3DSlices")) != 0)
        SetNum3DSlices(node->AsInt());
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
    if((node = searchNode->GetNode("sampling")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
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
    // Clear all the TransferFunctionWidgets.
    ClearTransferFunction2DWidgets();

    // Go through all of the children and construct a new
    // TransferFunctionWidget for each one of them.
    children = searchNode->GetChildren();
    if(children != 0)
    {
        for(int i = 0; i < searchNode->GetNumChildren(); ++i)
        {
            if(children[i]->GetKey() == std::string("TransferFunctionWidget"))
            {
                TransferFunctionWidget temp;
                temp.SetFromNode(children[i]);
                AddTransferFunction2DWidgets(temp);
            }
        }
    }

    if((node = searchNode->GetNode("transferFunctionDim")) != 0)
        SetTransferFunctionDim(node->AsInt());
    if(colorControlPoints.GetNumControlPoints() < 2)
         SetDefaultColorControlPoints();

}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

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
VolumeAttributes::SetFreeformFlag(bool freeformFlag_)
{
    freeformFlag = freeformFlag_;
    Select(ID_freeformFlag, (void *)&freeformFlag);
}

void
VolumeAttributes::SetOpacityControlPoints(const GaussianControlPointList &opacityControlPoints_)
{
    opacityControlPoints = opacityControlPoints_;
    Select(ID_opacityControlPoints, (void *)&opacityControlPoints);
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
    if(rendererType != SLIVR && transferFunctionDim > 1)
        SetTransferFunctionDim(1);
}

void
VolumeAttributes::SetGradientType(VolumeAttributes::GradientType gradientType_)
{
    gradientType = gradientType_;
    Select(ID_gradientType, (void *)&gradientType);
}

void
VolumeAttributes::SetNum3DSlices(int num3DSlices_)
{
    num3DSlices = num3DSlices_;
    Select(ID_num3DSlices, (void *)&num3DSlices);
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
VolumeAttributes::SetTransferFunctionDim(int transferFunctionDim_)
{
    int d = transferFunctionDim_;
    if(d < 1 || d > 2 || rendererType != SLIVR)
        d = 1;
    transferFunctionDim = d;
    Select(ID_transferFunctionDim, (void *)&transferFunctionDim);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

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

bool
VolumeAttributes::GetFreeformFlag() const
{
    return freeformFlag;
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

int
VolumeAttributes::GetNum3DSlices() const
{
    return num3DSlices;
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

const AttributeGroupVector &
VolumeAttributes::GetTransferFunction2DWidgets() const
{
    return transferFunction2DWidgets;
}

AttributeGroupVector &
VolumeAttributes::GetTransferFunction2DWidgets()
{
    return transferFunction2DWidgets;
}

int
VolumeAttributes::GetTransferFunctionDim() const
{
    return transferFunctionDim;
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
VolumeAttributes::SelectTransferFunction2DWidgets()
{
    Select(ID_transferFunction2DWidgets, (void *)&transferFunction2DWidgets);
}

///////////////////////////////////////////////////////////////////////////////
// AttributeGroupVector convenience methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: VolumeAttributes::AddTransferFunction2DWidgets
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
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
VolumeAttributes::AddTransferFunction2DWidgets(const TransferFunctionWidget &obj)
{
    TransferFunctionWidget *newTransferFunctionWidget = new TransferFunctionWidget(obj);
    transferFunction2DWidgets.push_back(newTransferFunctionWidget);

    // Indicate that things have changed by selecting it.
    Select(ID_transferFunction2DWidgets, (void *)&transferFunction2DWidgets);
}

// ****************************************************************************
// Method: VolumeAttributes::ClearTransferFunction2DWidgets
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
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
VolumeAttributes::ClearTransferFunction2DWidgets()
{
    AttributeGroupVector::iterator pos;

    for(pos = transferFunction2DWidgets.begin(); pos != transferFunction2DWidgets.end(); ++pos)
        delete *pos;
    transferFunction2DWidgets.clear();

    // Indicate that things have changed by selecting the list.
    Select(ID_transferFunction2DWidgets, (void *)&transferFunction2DWidgets);
}

// ****************************************************************************
// Method: VolumeAttributes::RemoveTransferFunction2DWidgets
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
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
VolumeAttributes::RemoveTransferFunction2DWidgets(int index)
{
    AttributeGroupVector::iterator pos = transferFunction2DWidgets.begin();

    // Iterate through the vector "index" times. 
    for(int i = 0; i < index; ++i)
        if(pos != transferFunction2DWidgets.end()) ++pos;

    // If pos is still a valid iterator, remove that element.
    if(pos != transferFunction2DWidgets.end())
    {
        delete *pos;
        transferFunction2DWidgets.erase(pos);
    }

    // Indicate that things have changed by selecting the list.
    Select(ID_transferFunction2DWidgets, (void *)&transferFunction2DWidgets);
}

// ****************************************************************************
// Method: VolumeAttributes::GetNumTransferFunction2DWidgets
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

int
VolumeAttributes::GetNumTransferFunction2DWidgets() const
{
    return transferFunction2DWidgets.size();
}

// ****************************************************************************
// Method: VolumeAttributes::GetTransferFunction2DWidgets
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

TransferFunctionWidget &
VolumeAttributes::GetTransferFunction2DWidgets(int i)
{
    return *((TransferFunctionWidget *)transferFunction2DWidgets[i]);
}

// ****************************************************************************
// Method: VolumeAttributes::GetTransferFunction2DWidgets
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

const TransferFunctionWidget &
VolumeAttributes::GetTransferFunction2DWidgets(int i) const
{
    return *((TransferFunctionWidget *)transferFunction2DWidgets[i]);
}

// ****************************************************************************
// Method: VolumeAttributes::operator []
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

TransferFunctionWidget &
VolumeAttributes::operator [] (int i)
{
    return *((TransferFunctionWidget *)transferFunction2DWidgets[i]);
}

// ****************************************************************************
// Method: VolumeAttributes::operator []
//
// Purpose: 
//   This class contains the plot attributes for the volume plot.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

const TransferFunctionWidget &
VolumeAttributes::operator [] (int i) const
{
    return *((TransferFunctionWidget *)transferFunction2DWidgets[i]);
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
    case ID_legendFlag:                return "legendFlag";
    case ID_lightingFlag:              return "lightingFlag";
    case ID_colorControlPoints:        return "colorControlPoints";
    case ID_opacityAttenuation:        return "opacityAttenuation";
    case ID_freeformFlag:              return "freeformFlag";
    case ID_opacityControlPoints:      return "opacityControlPoints";
    case ID_resampleTarget:            return "resampleTarget";
    case ID_opacityVariable:           return "opacityVariable";
    case ID_freeformOpacity:           return "freeformOpacity";
    case ID_useColorVarMin:            return "useColorVarMin";
    case ID_colorVarMin:               return "colorVarMin";
    case ID_useColorVarMax:            return "useColorVarMax";
    case ID_colorVarMax:               return "colorVarMax";
    case ID_useOpacityVarMin:          return "useOpacityVarMin";
    case ID_opacityVarMin:             return "opacityVarMin";
    case ID_useOpacityVarMax:          return "useOpacityVarMax";
    case ID_opacityVarMax:             return "opacityVarMax";
    case ID_smoothData:                return "smoothData";
    case ID_samplesPerRay:             return "samplesPerRay";
    case ID_rendererType:              return "rendererType";
    case ID_gradientType:              return "gradientType";
    case ID_num3DSlices:               return "num3DSlices";
    case ID_scaling:                   return "scaling";
    case ID_skewFactor:                return "skewFactor";
    case ID_sampling:                  return "sampling";
    case ID_rendererSamples:           return "rendererSamples";
    case ID_transferFunction2DWidgets: return "transferFunction2DWidgets";
    case ID_transferFunctionDim:       return "transferFunctionDim";
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
    case ID_legendFlag:                return FieldType_bool;
    case ID_lightingFlag:              return FieldType_bool;
    case ID_colorControlPoints:        return FieldType_att;
    case ID_opacityAttenuation:        return FieldType_float;
    case ID_freeformFlag:              return FieldType_bool;
    case ID_opacityControlPoints:      return FieldType_att;
    case ID_resampleTarget:            return FieldType_int;
    case ID_opacityVariable:           return FieldType_variablename;
    case ID_freeformOpacity:           return FieldType_ucharArray;
    case ID_useColorVarMin:            return FieldType_bool;
    case ID_colorVarMin:               return FieldType_float;
    case ID_useColorVarMax:            return FieldType_bool;
    case ID_colorVarMax:               return FieldType_float;
    case ID_useOpacityVarMin:          return FieldType_bool;
    case ID_opacityVarMin:             return FieldType_float;
    case ID_useOpacityVarMax:          return FieldType_bool;
    case ID_opacityVarMax:             return FieldType_float;
    case ID_smoothData:                return FieldType_bool;
    case ID_samplesPerRay:             return FieldType_int;
    case ID_rendererType:              return FieldType_enum;
    case ID_gradientType:              return FieldType_enum;
    case ID_num3DSlices:               return FieldType_int;
    case ID_scaling:                   return FieldType_enum;
    case ID_skewFactor:                return FieldType_double;
    case ID_sampling:                  return FieldType_enum;
    case ID_rendererSamples:           return FieldType_float;
    case ID_transferFunction2DWidgets: return FieldType_attVector;
    case ID_transferFunctionDim:       return FieldType_int;
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
    case ID_legendFlag:                return "bool";
    case ID_lightingFlag:              return "bool";
    case ID_colorControlPoints:        return "att";
    case ID_opacityAttenuation:        return "float";
    case ID_freeformFlag:              return "bool";
    case ID_opacityControlPoints:      return "att";
    case ID_resampleTarget:            return "int";
    case ID_opacityVariable:           return "variablename";
    case ID_freeformOpacity:           return "ucharArray";
    case ID_useColorVarMin:            return "bool";
    case ID_colorVarMin:               return "float";
    case ID_useColorVarMax:            return "bool";
    case ID_colorVarMax:               return "float";
    case ID_useOpacityVarMin:          return "bool";
    case ID_opacityVarMin:             return "float";
    case ID_useOpacityVarMax:          return "bool";
    case ID_opacityVarMax:             return "float";
    case ID_smoothData:                return "bool";
    case ID_samplesPerRay:             return "int";
    case ID_rendererType:              return "enum";
    case ID_gradientType:              return "enum";
    case ID_num3DSlices:               return "int";
    case ID_scaling:                   return "enum";
    case ID_skewFactor:                return "double";
    case ID_sampling:                  return "enum";
    case ID_rendererSamples:           return "float";
    case ID_transferFunction2DWidgets: return "attVector";
    case ID_transferFunctionDim:       return "int";
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
    case ID_freeformFlag:
        {  // new scope
        retval = (freeformFlag == obj.freeformFlag);
        }
        break;
    case ID_opacityControlPoints:
        {  // new scope
        retval = (opacityControlPoints == obj.opacityControlPoints);
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
    case ID_num3DSlices:
        {  // new scope
        retval = (num3DSlices == obj.num3DSlices);
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
    case ID_transferFunction2DWidgets:
        {  // new scope
        bool transferFunction2DWidgets_equal = (obj.transferFunction2DWidgets.size() == transferFunction2DWidgets.size());
        for(size_t i = 0; (i < transferFunction2DWidgets.size()) && transferFunction2DWidgets_equal; ++i)
        {
            // Make references to TransferFunctionWidget from AttributeGroup *.
            const TransferFunctionWidget &transferFunction2DWidgets1 = *((const TransferFunctionWidget *)(transferFunction2DWidgets[i]));
            const TransferFunctionWidget &transferFunction2DWidgets2 = *((const TransferFunctionWidget *)(obj.transferFunction2DWidgets[i]));
            transferFunction2DWidgets_equal = (transferFunction2DWidgets1 == transferFunction2DWidgets2);
        }

        retval = transferFunction2DWidgets_equal;
        }
        break;
    case ID_transferFunctionDim:
        {  // new scope
        retval = (transferFunctionDim == obj.transferFunctionDim);
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
// ****************************************************************************

bool
VolumeAttributes::ChangesRequireRecalculation(const VolumeAttributes &obj) const
{
    if (opacityVariable != obj.opacityVariable)
        return true;

    if (resampleTarget != obj.resampleTarget)
        return true;

    if (rendererType == VolumeAttributes::RayCasting ||
        rendererType == VolumeAttributes::RayCastingIntegration)
    {
        // We're in software mode. Any change to the renderer type requires
        // a reexecute.
        if(rendererType != obj.rendererType)
            return true;

        if (scaling != obj.scaling)
            return true;
        if (scaling == VolumeAttributes::Skew && skewFactor != obj.skewFactor)
            return true;
    }
    else
    {
        // We're in hardware mode now but if we're transitioning to software
        // then we need to reexecute. Transferring between any of the hardware
        // modes does not require a reexecute.
        if(obj.rendererType == VolumeAttributes::RayCasting ||
           obj.rendererType == VolumeAttributes::RayCastingIntegration)
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
//  Method:  VolumeAttributes::GradientWontChange
//
//  Purpose:
//    Determines if the gradient can avoid being invalidated.
//
//  Arguments:
//    obj        the attributes to compare with
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 30, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Oct  2 13:30:29 PDT 2003
//    Added rendererType and gradientType to the list of modifications
//    that require re-calculating the gradient.
//
// ****************************************************************************
bool
VolumeAttributes::GradientWontChange(const VolumeAttributes &obj) const
{
    int i;

    // Compare the freeformOpacity arrays.
    bool freeformOpacity_equal = true;
    for(i = 0; i < 256 && freeformOpacity_equal; ++i)
        freeformOpacity_equal = (freeformOpacity[i] == obj.freeformOpacity[i]);

    // Create the return value
    return ((freeformFlag         == obj.freeformFlag) &&
            (opacityControlPoints == obj.opacityControlPoints) &&
            (resampleTarget       == obj.resampleTarget) &&
            (opacityVariable      == obj.opacityVariable) &&
            freeformOpacity_equal &&
            (useColorVarMin       == obj.useColorVarMin) &&
            (colorVarMin          == obj.colorVarMin) &&
            (useColorVarMax       == obj.useColorVarMax) &&
            (colorVarMax          == obj.colorVarMax) &&
            (useOpacityVarMin     == obj.useOpacityVarMin) &&
            (opacityVarMin        == obj.opacityVarMin) &&
            (useOpacityVarMax     == obj.useOpacityVarMax) &&
            (opacityVarMax        == obj.opacityVarMax) &&
            (rendererType         == obj.rendererType) &&
            (gradientType         == obj.gradientType));
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
// ****************************************************************************

void
VolumeAttributes::GetTransferFunction(unsigned char *rgba) const
{
    unsigned char rgb[256 * 3];
    unsigned char alphas[256];
    const unsigned char *a_ptr;

    // Figure out the colors
    colorControlPoints.GetColors(rgb, 256);
    // Figure out the opacities
    if(freeformFlag)
        a_ptr = freeformOpacity;
    else
    {
        GetGaussianOpacities(alphas);
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

void
VolumeAttributes::GetOpacities(unsigned char *alphas)
{
    if(freeformFlag)
    {
        for(int i = 0; i < 256; ++i)
            alphas[i] = freeformOpacity[i];
    }
    else
        GetGaussianOpacities(alphas);
}

void
VolumeAttributes::SetSmoothingFlag(bool val)
{
    colorControlPoints.SetSmoothingFlag(val);
    Select(2, (void *)&colorControlPoints);
}

bool
VolumeAttributes::GetSmoothingFlag() const
{
    return colorControlPoints.GetSmoothingFlag();
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
           i == ID_freeformFlag || 
           i == ID_opacityControlPoints ||
           i == ID_freeformOpacity ||
           i == ID_transferFunction2DWidgets ||
           i == ID_transferFunctionDim)
        {
            continue;
        }
        same &= FieldsEqual(i, &rhs);
    }

    return !same;
}

