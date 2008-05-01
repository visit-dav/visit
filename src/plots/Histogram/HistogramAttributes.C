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

#include <HistogramAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for HistogramAttributes::OutputType
//

static const char *OutputType_strings[] = {
"Curve", "Block"};

std::string
HistogramAttributes::OutputType_ToString(HistogramAttributes::OutputType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OutputType_strings[index];
}

std::string
HistogramAttributes::OutputType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OutputType_strings[index];
}

bool
HistogramAttributes::OutputType_FromString(const std::string &s, HistogramAttributes::OutputType &val)
{
    val = HistogramAttributes::Curve;
    for(int i = 0; i < 2; ++i)
    {
        if(s == OutputType_strings[i])
        {
            val = (OutputType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for HistogramAttributes::BasedOn
//

static const char *BasedOn_strings[] = {
"ManyVarsForSingleZone", "ManyZonesForSingleVar"};

std::string
HistogramAttributes::BasedOn_ToString(HistogramAttributes::BasedOn t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return BasedOn_strings[index];
}

std::string
HistogramAttributes::BasedOn_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return BasedOn_strings[index];
}

bool
HistogramAttributes::BasedOn_FromString(const std::string &s, HistogramAttributes::BasedOn &val)
{
    val = HistogramAttributes::ManyVarsForSingleZone;
    for(int i = 0; i < 2; ++i)
    {
        if(s == BasedOn_strings[i])
        {
            val = (BasedOn)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for HistogramAttributes::BinContribution
//

static const char *BinContribution_strings[] = {
"Frequency", "Weighted", "Variable"
};

std::string
HistogramAttributes::BinContribution_ToString(HistogramAttributes::BinContribution t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return BinContribution_strings[index];
}

std::string
HistogramAttributes::BinContribution_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return BinContribution_strings[index];
}

bool
HistogramAttributes::BinContribution_FromString(const std::string &s, HistogramAttributes::BinContribution &val)
{
    val = HistogramAttributes::Frequency;
    for(int i = 0; i < 3; ++i)
    {
        if(s == BinContribution_strings[i])
        {
            val = (BinContribution)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for HistogramAttributes::DataScale
//

static const char *DataScale_strings[] = {
"Linear", "Log", "SquareRoot"
};

std::string
HistogramAttributes::DataScale_ToString(HistogramAttributes::DataScale t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return DataScale_strings[index];
}

std::string
HistogramAttributes::DataScale_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return DataScale_strings[index];
}

bool
HistogramAttributes::DataScale_FromString(const std::string &s, HistogramAttributes::DataScale &val)
{
    val = HistogramAttributes::Linear;
    for(int i = 0; i < 3; ++i)
    {
        if(s == DataScale_strings[i])
        {
            val = (DataScale)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *HistogramAttributes::TypeMapFormatString = "iisbddiiibiiiaii";

// ****************************************************************************
// Method: HistogramAttributes::HistogramAttributes
//
// Purpose: 
//   Constructor for the HistogramAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

HistogramAttributes::HistogramAttributes() : 
    AttributeSubject(HistogramAttributes::TypeMapFormatString),
    weightVariable("default"), color(200, 80, 40)
{
    basedOn = ManyZonesForSingleVar;
    histogramType = Weighted;
    specifyRange = false;
    min = 0;
    max = 1;
    numBins = 32;
    domain = 0;
    zone = 0;
    useBinWidths = true;
    outputType = Block;
    lineStyle = 0;
    lineWidth = 0;
    dataScale = Linear;
    binScale = Linear;
}

// ****************************************************************************
// Method: HistogramAttributes::HistogramAttributes
//
// Purpose: 
//   Copy constructor for the HistogramAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

HistogramAttributes::HistogramAttributes(const HistogramAttributes &obj) : 
    AttributeSubject(HistogramAttributes::TypeMapFormatString)
{
    basedOn = obj.basedOn;
    histogramType = obj.histogramType;
    weightVariable = obj.weightVariable;
    specifyRange = obj.specifyRange;
    min = obj.min;
    max = obj.max;
    numBins = obj.numBins;
    domain = obj.domain;
    zone = obj.zone;
    useBinWidths = obj.useBinWidths;
    outputType = obj.outputType;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    color = obj.color;
    dataScale = obj.dataScale;
    binScale = obj.binScale;

    SelectAll();
}

// ****************************************************************************
// Method: HistogramAttributes::~HistogramAttributes
//
// Purpose: 
//   Destructor for the HistogramAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

HistogramAttributes::~HistogramAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: HistogramAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the HistogramAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

HistogramAttributes& 
HistogramAttributes::operator = (const HistogramAttributes &obj)
{
    if (this == &obj) return *this;
    basedOn = obj.basedOn;
    histogramType = obj.histogramType;
    weightVariable = obj.weightVariable;
    specifyRange = obj.specifyRange;
    min = obj.min;
    max = obj.max;
    numBins = obj.numBins;
    domain = obj.domain;
    zone = obj.zone;
    useBinWidths = obj.useBinWidths;
    outputType = obj.outputType;
    lineStyle = obj.lineStyle;
    lineWidth = obj.lineWidth;
    color = obj.color;
    dataScale = obj.dataScale;
    binScale = obj.binScale;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: HistogramAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the HistogramAttributes class.
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
HistogramAttributes::operator == (const HistogramAttributes &obj) const
{
    // Create the return value
    return ((basedOn == obj.basedOn) &&
            (histogramType == obj.histogramType) &&
            (weightVariable == obj.weightVariable) &&
            (specifyRange == obj.specifyRange) &&
            (min == obj.min) &&
            (max == obj.max) &&
            (numBins == obj.numBins) &&
            (domain == obj.domain) &&
            (zone == obj.zone) &&
            (useBinWidths == obj.useBinWidths) &&
            (outputType == obj.outputType) &&
            (lineStyle == obj.lineStyle) &&
            (lineWidth == obj.lineWidth) &&
            (color == obj.color) &&
            (dataScale == obj.dataScale) &&
            (binScale == obj.binScale));
}

// ****************************************************************************
// Method: HistogramAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the HistogramAttributes class.
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
HistogramAttributes::operator != (const HistogramAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: HistogramAttributes::TypeName
//
// Purpose: 
//   Type name method for the HistogramAttributes class.
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
HistogramAttributes::TypeName() const
{
    return "HistogramAttributes";
}

bool
HistogramAttributes::CopyAttributes(const AttributeGroup *atts)
{
    bool retval = false;

    if(TypeName() == atts->TypeName())
    {
        // Call assignment operator.
        const HistogramAttributes *tmp = (const HistogramAttributes *)atts;
        *this = *tmp;
        retval = true;
    }
    else if(atts->TypeName() == "PickAttributes")
    {
        const PickAttributes *tmp = (const PickAttributes*)atts;
        int dom = tmp->GetDomain();
        dom = (dom < 0 ? 0 : dom);  // -1 is val for single domain problems
        SetDomain(dom);
        SetZone(tmp->GetElementNumber());
        retval = true;
    }

    return retval;
}

// ****************************************************************************
// Method: HistogramAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the HistogramAttributes class.
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
HistogramAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new HistogramAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: HistogramAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the HistogramAttributes class.
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
HistogramAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new HistogramAttributes(*this);
    else
        retval = new HistogramAttributes;

    return retval;
}

// ****************************************************************************
// Method: HistogramAttributes::SelectAll
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
HistogramAttributes::SelectAll()
{
    Select(ID_basedOn,        (void *)&basedOn);
    Select(ID_histogramType,  (void *)&histogramType);
    Select(ID_weightVariable, (void *)&weightVariable);
    Select(ID_specifyRange,   (void *)&specifyRange);
    Select(ID_min,            (void *)&min);
    Select(ID_max,            (void *)&max);
    Select(ID_numBins,        (void *)&numBins);
    Select(ID_domain,         (void *)&domain);
    Select(ID_zone,           (void *)&zone);
    Select(ID_useBinWidths,   (void *)&useBinWidths);
    Select(ID_outputType,     (void *)&outputType);
    Select(ID_lineStyle,      (void *)&lineStyle);
    Select(ID_lineWidth,      (void *)&lineWidth);
    Select(ID_color,          (void *)&color);
    Select(ID_dataScale,      (void *)&dataScale);
    Select(ID_binScale,       (void *)&binScale);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: HistogramAttributes::CreateNode
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
HistogramAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    HistogramAttributes defaultObject;
    bool addToParent = false;
    // Create a node for HistogramAttributes.
    DataNode *node = new DataNode("HistogramAttributes");

    if(completeSave || !FieldsEqual(ID_basedOn, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("basedOn", BasedOn_ToString(basedOn)));
    }

    if(completeSave || !FieldsEqual(ID_histogramType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("histogramType", BinContribution_ToString(histogramType)));
    }

    if(completeSave || !FieldsEqual(ID_weightVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("weightVariable", weightVariable));
    }

    if(completeSave || !FieldsEqual(ID_specifyRange, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specifyRange", specifyRange));
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

    if(completeSave || !FieldsEqual(ID_numBins, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numBins", numBins));
    }

    if(completeSave || !FieldsEqual(ID_domain, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("domain", domain));
    }

    if(completeSave || !FieldsEqual(ID_zone, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("zone", zone));
    }

    if(completeSave || !FieldsEqual(ID_useBinWidths, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useBinWidths", useBinWidths));
    }

    if(completeSave || !FieldsEqual(ID_outputType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("outputType", OutputType_ToString(outputType)));
    }

    if(completeSave || !FieldsEqual(ID_lineStyle, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineStyle", lineStyle));
    }

    if(completeSave || !FieldsEqual(ID_lineWidth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("lineWidth", lineWidth));
    }

        DataNode *colorNode = new DataNode("color");
        if(color.CreateNode(colorNode, completeSave, true))
        {
            addToParent = true;
            node->AddNode(colorNode);
        }
        else
            delete colorNode;
    if(completeSave || !FieldsEqual(ID_dataScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataScale", DataScale_ToString(dataScale)));
    }

    if(completeSave || !FieldsEqual(ID_binScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("binScale", DataScale_ToString(binScale)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: HistogramAttributes::SetFromNode
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
HistogramAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("HistogramAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("basedOn")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetBasedOn(BasedOn(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BasedOn value;
            if(BasedOn_FromString(node->AsString(), value))
                SetBasedOn(value);
        }
    }
    if((node = searchNode->GetNode("histogramType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetHistogramType(BinContribution(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BinContribution value;
            if(BinContribution_FromString(node->AsString(), value))
                SetHistogramType(value);
        }
    }
    if((node = searchNode->GetNode("weightVariable")) != 0)
        SetWeightVariable(node->AsString());
    if((node = searchNode->GetNode("specifyRange")) != 0)
        SetSpecifyRange(node->AsBool());
    if((node = searchNode->GetNode("min")) != 0)
        SetMin(node->AsDouble());
    if((node = searchNode->GetNode("max")) != 0)
        SetMax(node->AsDouble());
    if((node = searchNode->GetNode("numBins")) != 0)
        SetNumBins(node->AsInt());
    if((node = searchNode->GetNode("domain")) != 0)
        SetDomain(node->AsInt());
    if((node = searchNode->GetNode("zone")) != 0)
        SetZone(node->AsInt());
    if((node = searchNode->GetNode("useBinWidths")) != 0)
        SetUseBinWidths(node->AsBool());
    if((node = searchNode->GetNode("outputType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetOutputType(OutputType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OutputType value;
            if(OutputType_FromString(node->AsString(), value))
                SetOutputType(value);
        }
    }
    if((node = searchNode->GetNode("lineStyle")) != 0)
        SetLineStyle(node->AsInt());
    if((node = searchNode->GetNode("lineWidth")) != 0)
        SetLineWidth(node->AsInt());
    if((node = searchNode->GetNode("color")) != 0)
        color.SetFromNode(node);
    if((node = searchNode->GetNode("dataScale")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetDataScale(DataScale(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DataScale value;
            if(DataScale_FromString(node->AsString(), value))
                SetDataScale(value);
        }
    }
    if((node = searchNode->GetNode("binScale")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetBinScale(DataScale(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            DataScale value;
            if(DataScale_FromString(node->AsString(), value))
                SetBinScale(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
HistogramAttributes::SetBasedOn(HistogramAttributes::BasedOn basedOn_)
{
    basedOn = basedOn_;
    Select(ID_basedOn, (void *)&basedOn);
}

void
HistogramAttributes::SetHistogramType(HistogramAttributes::BinContribution histogramType_)
{
    histogramType = histogramType_;
    Select(ID_histogramType, (void *)&histogramType);
}

void
HistogramAttributes::SetWeightVariable(const std::string &weightVariable_)
{
    weightVariable = weightVariable_;
    Select(ID_weightVariable, (void *)&weightVariable);
}

void
HistogramAttributes::SetSpecifyRange(bool specifyRange_)
{
    specifyRange = specifyRange_;
    Select(ID_specifyRange, (void *)&specifyRange);
}

void
HistogramAttributes::SetMin(double min_)
{
    min = min_;
    Select(ID_min, (void *)&min);
}

void
HistogramAttributes::SetMax(double max_)
{
    max = max_;
    Select(ID_max, (void *)&max);
}

void
HistogramAttributes::SetNumBins(int numBins_)
{
    numBins = numBins_;
    Select(ID_numBins, (void *)&numBins);
}

void
HistogramAttributes::SetDomain(int domain_)
{
    domain = domain_;
    Select(ID_domain, (void *)&domain);
}

void
HistogramAttributes::SetZone(int zone_)
{
    zone = zone_;
    Select(ID_zone, (void *)&zone);
}

void
HistogramAttributes::SetUseBinWidths(bool useBinWidths_)
{
    useBinWidths = useBinWidths_;
    Select(ID_useBinWidths, (void *)&useBinWidths);
}

void
HistogramAttributes::SetOutputType(HistogramAttributes::OutputType outputType_)
{
    outputType = outputType_;
    Select(ID_outputType, (void *)&outputType);
}

void
HistogramAttributes::SetLineStyle(int lineStyle_)
{
    lineStyle = lineStyle_;
    Select(ID_lineStyle, (void *)&lineStyle);
}

void
HistogramAttributes::SetLineWidth(int lineWidth_)
{
    lineWidth = lineWidth_;
    Select(ID_lineWidth, (void *)&lineWidth);
}

void
HistogramAttributes::SetColor(const ColorAttribute &color_)
{
    color = color_;
    Select(ID_color, (void *)&color);
}

void
HistogramAttributes::SetDataScale(HistogramAttributes::DataScale dataScale_)
{
    dataScale = dataScale_;
    Select(ID_dataScale, (void *)&dataScale);
}

void
HistogramAttributes::SetBinScale(HistogramAttributes::DataScale binScale_)
{
    binScale = binScale_;
    Select(ID_binScale, (void *)&binScale);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

HistogramAttributes::BasedOn
HistogramAttributes::GetBasedOn() const
{
    return BasedOn(basedOn);
}

HistogramAttributes::BinContribution
HistogramAttributes::GetHistogramType() const
{
    return BinContribution(histogramType);
}

const std::string &
HistogramAttributes::GetWeightVariable() const
{
    return weightVariable;
}

std::string &
HistogramAttributes::GetWeightVariable()
{
    return weightVariable;
}

bool
HistogramAttributes::GetSpecifyRange() const
{
    return specifyRange;
}

double
HistogramAttributes::GetMin() const
{
    return min;
}

double
HistogramAttributes::GetMax() const
{
    return max;
}

int
HistogramAttributes::GetNumBins() const
{
    return numBins;
}

int
HistogramAttributes::GetDomain() const
{
    return domain;
}

int
HistogramAttributes::GetZone() const
{
    return zone;
}

bool
HistogramAttributes::GetUseBinWidths() const
{
    return useBinWidths;
}

HistogramAttributes::OutputType
HistogramAttributes::GetOutputType() const
{
    return OutputType(outputType);
}

int
HistogramAttributes::GetLineStyle() const
{
    return lineStyle;
}

int
HistogramAttributes::GetLineWidth() const
{
    return lineWidth;
}

const ColorAttribute &
HistogramAttributes::GetColor() const
{
    return color;
}

ColorAttribute &
HistogramAttributes::GetColor()
{
    return color;
}

HistogramAttributes::DataScale
HistogramAttributes::GetDataScale() const
{
    return DataScale(dataScale);
}

HistogramAttributes::DataScale
HistogramAttributes::GetBinScale() const
{
    return DataScale(binScale);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
HistogramAttributes::SelectWeightVariable()
{
    Select(ID_weightVariable, (void *)&weightVariable);
}

void
HistogramAttributes::SelectColor()
{
    Select(ID_color, (void *)&color);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: HistogramAttributes::GetFieldName
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
HistogramAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_basedOn:        return "basedOn";
    case ID_histogramType:  return "histogramType";
    case ID_weightVariable: return "weightVariable";
    case ID_specifyRange:   return "specifyRange";
    case ID_min:            return "min";
    case ID_max:            return "max";
    case ID_numBins:        return "numBins";
    case ID_domain:         return "domain";
    case ID_zone:           return "zone";
    case ID_useBinWidths:   return "useBinWidths";
    case ID_outputType:     return "outputType";
    case ID_lineStyle:      return "lineStyle";
    case ID_lineWidth:      return "lineWidth";
    case ID_color:          return "color";
    case ID_dataScale:      return "dataScale";
    case ID_binScale:       return "binScale";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: HistogramAttributes::GetFieldType
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
HistogramAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_basedOn:        return FieldType_enum;
    case ID_histogramType:  return FieldType_enum;
    case ID_weightVariable: return FieldType_variablename;
    case ID_specifyRange:   return FieldType_bool;
    case ID_min:            return FieldType_double;
    case ID_max:            return FieldType_double;
    case ID_numBins:        return FieldType_int;
    case ID_domain:         return FieldType_int;
    case ID_zone:           return FieldType_int;
    case ID_useBinWidths:   return FieldType_bool;
    case ID_outputType:     return FieldType_enum;
    case ID_lineStyle:      return FieldType_linestyle;
    case ID_lineWidth:      return FieldType_linewidth;
    case ID_color:          return FieldType_color;
    case ID_dataScale:      return FieldType_enum;
    case ID_binScale:       return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: HistogramAttributes::GetFieldTypeName
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
HistogramAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_basedOn:        return "enum";
    case ID_histogramType:  return "enum";
    case ID_weightVariable: return "variablename";
    case ID_specifyRange:   return "bool";
    case ID_min:            return "double";
    case ID_max:            return "double";
    case ID_numBins:        return "int";
    case ID_domain:         return "int";
    case ID_zone:           return "int";
    case ID_useBinWidths:   return "bool";
    case ID_outputType:     return "enum";
    case ID_lineStyle:      return "linestyle";
    case ID_lineWidth:      return "linewidth";
    case ID_color:          return "color";
    case ID_dataScale:      return "enum";
    case ID_binScale:       return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: HistogramAttributes::FieldsEqual
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
HistogramAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const HistogramAttributes &obj = *((const HistogramAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_basedOn:
        {  // new scope
        retval = (basedOn == obj.basedOn);
        }
        break;
    case ID_histogramType:
        {  // new scope
        retval = (histogramType == obj.histogramType);
        }
        break;
    case ID_weightVariable:
        {  // new scope
        retval = (weightVariable == obj.weightVariable);
        }
        break;
    case ID_specifyRange:
        {  // new scope
        retval = (specifyRange == obj.specifyRange);
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
    case ID_numBins:
        {  // new scope
        retval = (numBins == obj.numBins);
        }
        break;
    case ID_domain:
        {  // new scope
        retval = (domain == obj.domain);
        }
        break;
    case ID_zone:
        {  // new scope
        retval = (zone == obj.zone);
        }
        break;
    case ID_useBinWidths:
        {  // new scope
        retval = (useBinWidths == obj.useBinWidths);
        }
        break;
    case ID_outputType:
        {  // new scope
        retval = (outputType == obj.outputType);
        }
        break;
    case ID_lineStyle:
        {  // new scope
        retval = (lineStyle == obj.lineStyle);
        }
        break;
    case ID_lineWidth:
        {  // new scope
        retval = (lineWidth == obj.lineWidth);
        }
        break;
    case ID_color:
        {  // new scope
        retval = (color == obj.color);
        }
        break;
    case ID_dataScale:
        {  // new scope
        retval = (dataScale == obj.dataScale);
        }
        break;
    case ID_binScale:
        {  // new scope
        retval = (binScale == obj.binScale);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

bool
HistogramAttributes::ChangesRequireRecalculation(const HistogramAttributes &obj) const
{
    if (specifyRange != obj.GetSpecifyRange())
        return true;
    if (specifyRange && (min != obj.GetMin()))
        return true;
    if (specifyRange && (max != obj.GetMax()))
        return true;
    if (useBinWidths != obj.GetUseBinWidths())
        return true;
    if (outputType != obj.GetOutputType())
        return true;
    if (numBins != obj.GetNumBins())
        return true;
    if (weightVariable != obj.GetWeightVariable())
        return true;
    if (zone != obj.GetZone())
        return true;
    if (domain != obj.GetDomain())
        return true;
    if (histogramType != obj.GetHistogramType())
        return true; 
    if ( dataScale != obj.GetDataScale() )
	return true;
    if ( binScale != obj.GetBinScale() )
	return true;
    return false;
}

bool
HistogramAttributes::VarChangeRequiresReset()
{
    return true;
}

