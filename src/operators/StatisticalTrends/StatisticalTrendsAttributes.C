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

#include <StatisticalTrendsAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for StatisticalTrendsAttributes::TrendTypeEnum
//

static const char *TrendTypeEnum_strings[] = {
"Absolute", "Relative"};

std::string
StatisticalTrendsAttributes::TrendTypeEnum_ToString(StatisticalTrendsAttributes::TrendTypeEnum t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return TrendTypeEnum_strings[index];
}

std::string
StatisticalTrendsAttributes::TrendTypeEnum_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return TrendTypeEnum_strings[index];
}

bool
StatisticalTrendsAttributes::TrendTypeEnum_FromString(const std::string &s, StatisticalTrendsAttributes::TrendTypeEnum &val)
{
    val = StatisticalTrendsAttributes::Absolute;
    for(int i = 0; i < 2; ++i)
    {
        if(s == TrendTypeEnum_strings[i])
        {
            val = (TrendTypeEnum)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StatisticalTrendsAttributes::StatisticTypeEnum
//

static const char *StatisticTypeEnum_strings[] = {
"Sum", "Mean", "Variance", 
"StandardDeviation", "Slope", "Residuals"
};

std::string
StatisticalTrendsAttributes::StatisticTypeEnum_ToString(StatisticalTrendsAttributes::StatisticTypeEnum t)
{
    int index = int(t);
    if(index < 0 || index >= 6) index = 0;
    return StatisticTypeEnum_strings[index];
}

std::string
StatisticalTrendsAttributes::StatisticTypeEnum_ToString(int t)
{
    int index = (t < 0 || t >= 6) ? 0 : t;
    return StatisticTypeEnum_strings[index];
}

bool
StatisticalTrendsAttributes::StatisticTypeEnum_FromString(const std::string &s, StatisticalTrendsAttributes::StatisticTypeEnum &val)
{
    val = StatisticalTrendsAttributes::Sum;
    for(int i = 0; i < 6; ++i)
    {
        if(s == StatisticTypeEnum_strings[i])
        {
            val = (StatisticTypeEnum)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StatisticalTrendsAttributes::TrendAxisEnum
//

static const char *TrendAxisEnum_strings[] = {
"Step", "Time", "Cycle"
};

std::string
StatisticalTrendsAttributes::TrendAxisEnum_ToString(StatisticalTrendsAttributes::TrendAxisEnum t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return TrendAxisEnum_strings[index];
}

std::string
StatisticalTrendsAttributes::TrendAxisEnum_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return TrendAxisEnum_strings[index];
}

bool
StatisticalTrendsAttributes::TrendAxisEnum_FromString(const std::string &s, StatisticalTrendsAttributes::TrendAxisEnum &val)
{
    val = StatisticalTrendsAttributes::Step;
    for(int i = 0; i < 3; ++i)
    {
        if(s == TrendAxisEnum_strings[i])
        {
            val = (TrendAxisEnum)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for StatisticalTrendsAttributes::VariableSourceEnum
//

static const char *VariableSourceEnum_strings[] = {
"Default", "OperatorExpression"};

std::string
StatisticalTrendsAttributes::VariableSourceEnum_ToString(StatisticalTrendsAttributes::VariableSourceEnum t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return VariableSourceEnum_strings[index];
}

std::string
StatisticalTrendsAttributes::VariableSourceEnum_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return VariableSourceEnum_strings[index];
}

bool
StatisticalTrendsAttributes::VariableSourceEnum_FromString(const std::string &s, StatisticalTrendsAttributes::VariableSourceEnum &val)
{
    val = StatisticalTrendsAttributes::Default;
    for(int i = 0; i < 2; ++i)
    {
        if(s == VariableSourceEnum_strings[i])
        {
            val = (VariableSourceEnum)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::StatisticalTrendsAttributes
//
// Purpose: 
//   Init utility for the StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void StatisticalTrendsAttributes::Init()
{
    startIndex = 0;
    stopIndex = 1;
    stride = 1;
    startTrendType = Absolute;
    stopTrendType = Absolute;
    statisticType = Mean;
    trendAxis = Step;
    variableSource = Default;

    StatisticalTrendsAttributes::SelectAll();
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::StatisticalTrendsAttributes
//
// Purpose: 
//   Copy utility for the StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void StatisticalTrendsAttributes::Copy(const StatisticalTrendsAttributes &obj)
{
    startIndex = obj.startIndex;
    stopIndex = obj.stopIndex;
    stride = obj.stride;
    startTrendType = obj.startTrendType;
    stopTrendType = obj.stopTrendType;
    statisticType = obj.statisticType;
    trendAxis = obj.trendAxis;
    variableSource = obj.variableSource;

    StatisticalTrendsAttributes::SelectAll();
}

// Type map format string
const char *StatisticalTrendsAttributes::TypeMapFormatString = STATISTICALTRENDSATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t StatisticalTrendsAttributes::TmfsStruct = {STATISTICALTRENDSATTRIBUTES_TMFS};


// ****************************************************************************
// Method: StatisticalTrendsAttributes::StatisticalTrendsAttributes
//
// Purpose: 
//   Default constructor for the StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StatisticalTrendsAttributes::StatisticalTrendsAttributes() : 
    AttributeSubject(StatisticalTrendsAttributes::TypeMapFormatString)
{
    StatisticalTrendsAttributes::Init();
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::StatisticalTrendsAttributes
//
// Purpose: 
//   Constructor for the derived classes of StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StatisticalTrendsAttributes::StatisticalTrendsAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    StatisticalTrendsAttributes::Init();
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::StatisticalTrendsAttributes
//
// Purpose: 
//   Copy constructor for the StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StatisticalTrendsAttributes::StatisticalTrendsAttributes(const StatisticalTrendsAttributes &obj) : 
    AttributeSubject(StatisticalTrendsAttributes::TypeMapFormatString)
{
    StatisticalTrendsAttributes::Copy(obj);
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::StatisticalTrendsAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StatisticalTrendsAttributes::StatisticalTrendsAttributes(const StatisticalTrendsAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    StatisticalTrendsAttributes::Copy(obj);
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::~StatisticalTrendsAttributes
//
// Purpose: 
//   Destructor for the StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StatisticalTrendsAttributes::~StatisticalTrendsAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the StatisticalTrendsAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

StatisticalTrendsAttributes& 
StatisticalTrendsAttributes::operator = (const StatisticalTrendsAttributes &obj)
{
    if (this == &obj) return *this;

    StatisticalTrendsAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the StatisticalTrendsAttributes class.
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
StatisticalTrendsAttributes::operator == (const StatisticalTrendsAttributes &obj) const
{
    // Create the return value
    return ((startIndex == obj.startIndex) &&
            (stopIndex == obj.stopIndex) &&
            (stride == obj.stride) &&
            (startTrendType == obj.startTrendType) &&
            (stopTrendType == obj.stopTrendType) &&
            (statisticType == obj.statisticType) &&
            (trendAxis == obj.trendAxis) &&
            (variableSource == obj.variableSource));
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the StatisticalTrendsAttributes class.
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
StatisticalTrendsAttributes::operator != (const StatisticalTrendsAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::TypeName
//
// Purpose: 
//   Type name method for the StatisticalTrendsAttributes class.
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
StatisticalTrendsAttributes::TypeName() const
{
    return "StatisticalTrendsAttributes";
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the StatisticalTrendsAttributes class.
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
StatisticalTrendsAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const StatisticalTrendsAttributes *tmp = (const StatisticalTrendsAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the StatisticalTrendsAttributes class.
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
StatisticalTrendsAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new StatisticalTrendsAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the StatisticalTrendsAttributes class.
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
StatisticalTrendsAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new StatisticalTrendsAttributes(*this);
    else
        retval = new StatisticalTrendsAttributes;

    return retval;
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::SelectAll
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
StatisticalTrendsAttributes::SelectAll()
{
    Select(ID_startIndex,     (void *)&startIndex);
    Select(ID_stopIndex,      (void *)&stopIndex);
    Select(ID_stride,         (void *)&stride);
    Select(ID_startTrendType, (void *)&startTrendType);
    Select(ID_stopTrendType,  (void *)&stopTrendType);
    Select(ID_statisticType,  (void *)&statisticType);
    Select(ID_trendAxis,      (void *)&trendAxis);
    Select(ID_variableSource, (void *)&variableSource);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: StatisticalTrendsAttributes::CreateNode
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
StatisticalTrendsAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    StatisticalTrendsAttributes defaultObject;
    bool addToParent = false;
    // Create a node for StatisticalTrendsAttributes.
    DataNode *node = new DataNode("StatisticalTrendsAttributes");

    if(completeSave || !FieldsEqual(ID_startIndex, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("startIndex", startIndex));
    }

    if(completeSave || !FieldsEqual(ID_stopIndex, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("stopIndex", stopIndex));
    }

    if(completeSave || !FieldsEqual(ID_stride, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("stride", stride));
    }

    if(completeSave || !FieldsEqual(ID_startTrendType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("startTrendType", TrendTypeEnum_ToString(startTrendType)));
    }

    if(completeSave || !FieldsEqual(ID_stopTrendType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("stopTrendType", TrendTypeEnum_ToString(stopTrendType)));
    }

    if(completeSave || !FieldsEqual(ID_statisticType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("statisticType", StatisticTypeEnum_ToString(statisticType)));
    }

    if(completeSave || !FieldsEqual(ID_trendAxis, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("trendAxis", TrendAxisEnum_ToString(trendAxis)));
    }

    if(completeSave || !FieldsEqual(ID_variableSource, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("variableSource", VariableSourceEnum_ToString(variableSource)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::SetFromNode
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
StatisticalTrendsAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("StatisticalTrendsAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("startIndex")) != 0)
        SetStartIndex(node->AsInt());
    if((node = searchNode->GetNode("stopIndex")) != 0)
        SetStopIndex(node->AsInt());
    if((node = searchNode->GetNode("stride")) != 0)
        SetStride(node->AsInt());
    if((node = searchNode->GetNode("startTrendType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetStartTrendType(TrendTypeEnum(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TrendTypeEnum value;
            if(TrendTypeEnum_FromString(node->AsString(), value))
                SetStartTrendType(value);
        }
    }
    if((node = searchNode->GetNode("stopTrendType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetStopTrendType(TrendTypeEnum(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TrendTypeEnum value;
            if(TrendTypeEnum_FromString(node->AsString(), value))
                SetStopTrendType(value);
        }
    }
    if((node = searchNode->GetNode("statisticType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 6)
                SetStatisticType(StatisticTypeEnum(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            StatisticTypeEnum value;
            if(StatisticTypeEnum_FromString(node->AsString(), value))
                SetStatisticType(value);
        }
    }
    if((node = searchNode->GetNode("trendAxis")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetTrendAxis(TrendAxisEnum(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            TrendAxisEnum value;
            if(TrendAxisEnum_FromString(node->AsString(), value))
                SetTrendAxis(value);
        }
    }
    if((node = searchNode->GetNode("variableSource")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetVariableSource(VariableSourceEnum(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            VariableSourceEnum value;
            if(VariableSourceEnum_FromString(node->AsString(), value))
                SetVariableSource(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
StatisticalTrendsAttributes::SetStartIndex(int startIndex_)
{
    startIndex = startIndex_;
    Select(ID_startIndex, (void *)&startIndex);
}

void
StatisticalTrendsAttributes::SetStopIndex(int stopIndex_)
{
    stopIndex = stopIndex_;
    Select(ID_stopIndex, (void *)&stopIndex);
}

void
StatisticalTrendsAttributes::SetStride(int stride_)
{
    stride = stride_;
    Select(ID_stride, (void *)&stride);
}

void
StatisticalTrendsAttributes::SetStartTrendType(StatisticalTrendsAttributes::TrendTypeEnum startTrendType_)
{
    startTrendType = startTrendType_;
    Select(ID_startTrendType, (void *)&startTrendType);
}

void
StatisticalTrendsAttributes::SetStopTrendType(StatisticalTrendsAttributes::TrendTypeEnum stopTrendType_)
{
    stopTrendType = stopTrendType_;
    Select(ID_stopTrendType, (void *)&stopTrendType);
}

void
StatisticalTrendsAttributes::SetStatisticType(StatisticalTrendsAttributes::StatisticTypeEnum statisticType_)
{
    statisticType = statisticType_;
    Select(ID_statisticType, (void *)&statisticType);
}

void
StatisticalTrendsAttributes::SetTrendAxis(StatisticalTrendsAttributes::TrendAxisEnum trendAxis_)
{
    trendAxis = trendAxis_;
    Select(ID_trendAxis, (void *)&trendAxis);
}

void
StatisticalTrendsAttributes::SetVariableSource(StatisticalTrendsAttributes::VariableSourceEnum variableSource_)
{
    variableSource = variableSource_;
    Select(ID_variableSource, (void *)&variableSource);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

int
StatisticalTrendsAttributes::GetStartIndex() const
{
    return startIndex;
}

int
StatisticalTrendsAttributes::GetStopIndex() const
{
    return stopIndex;
}

int
StatisticalTrendsAttributes::GetStride() const
{
    return stride;
}

StatisticalTrendsAttributes::TrendTypeEnum
StatisticalTrendsAttributes::GetStartTrendType() const
{
    return TrendTypeEnum(startTrendType);
}

StatisticalTrendsAttributes::TrendTypeEnum
StatisticalTrendsAttributes::GetStopTrendType() const
{
    return TrendTypeEnum(stopTrendType);
}

StatisticalTrendsAttributes::StatisticTypeEnum
StatisticalTrendsAttributes::GetStatisticType() const
{
    return StatisticTypeEnum(statisticType);
}

StatisticalTrendsAttributes::TrendAxisEnum
StatisticalTrendsAttributes::GetTrendAxis() const
{
    return TrendAxisEnum(trendAxis);
}

StatisticalTrendsAttributes::VariableSourceEnum
StatisticalTrendsAttributes::GetVariableSource() const
{
    return VariableSourceEnum(variableSource);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: StatisticalTrendsAttributes::GetFieldName
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
StatisticalTrendsAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_startIndex:     return "startIndex";
    case ID_stopIndex:      return "stopIndex";
    case ID_stride:         return "stride";
    case ID_startTrendType: return "startTrendType";
    case ID_stopTrendType:  return "stopTrendType";
    case ID_statisticType:  return "statisticType";
    case ID_trendAxis:      return "trendAxis";
    case ID_variableSource: return "variableSource";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::GetFieldType
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
StatisticalTrendsAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_startIndex:     return FieldType_int;
    case ID_stopIndex:      return FieldType_int;
    case ID_stride:         return FieldType_int;
    case ID_startTrendType: return FieldType_enum;
    case ID_stopTrendType:  return FieldType_enum;
    case ID_statisticType:  return FieldType_enum;
    case ID_trendAxis:      return FieldType_enum;
    case ID_variableSource: return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::GetFieldTypeName
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
StatisticalTrendsAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_startIndex:     return "int";
    case ID_stopIndex:      return "int";
    case ID_stride:         return "int";
    case ID_startTrendType: return "enum";
    case ID_stopTrendType:  return "enum";
    case ID_statisticType:  return "enum";
    case ID_trendAxis:      return "enum";
    case ID_variableSource: return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: StatisticalTrendsAttributes::FieldsEqual
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
StatisticalTrendsAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const StatisticalTrendsAttributes &obj = *((const StatisticalTrendsAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_startIndex:
        {  // new scope
        retval = (startIndex == obj.startIndex);
        }
        break;
    case ID_stopIndex:
        {  // new scope
        retval = (stopIndex == obj.stopIndex);
        }
        break;
    case ID_stride:
        {  // new scope
        retval = (stride == obj.stride);
        }
        break;
    case ID_startTrendType:
        {  // new scope
        retval = (startTrendType == obj.startTrendType);
        }
        break;
    case ID_stopTrendType:
        {  // new scope
        retval = (stopTrendType == obj.stopTrendType);
        }
        break;
    case ID_statisticType:
        {  // new scope
        retval = (statisticType == obj.statisticType);
        }
        break;
    case ID_trendAxis:
        {  // new scope
        retval = (trendAxis == obj.trendAxis);
        }
        break;
    case ID_variableSource:
        {  // new scope
        retval = (variableSource == obj.variableSource);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

