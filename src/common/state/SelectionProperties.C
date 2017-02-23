/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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

#include <SelectionProperties.h>
#include <DataNode.h>
#include <float.h>

const double SelectionProperties::MIN = -FLT_MAX;

const double SelectionProperties::MAX = FLT_MAX;

//
// Enum conversion methods for SelectionProperties::SelectionType
//

static const char *SelectionType_strings[] = {
"BasicSelection", "CumulativeQuerySelection"};

std::string
SelectionProperties::SelectionType_ToString(SelectionProperties::SelectionType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return SelectionType_strings[index];
}

std::string
SelectionProperties::SelectionType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return SelectionType_strings[index];
}

bool
SelectionProperties::SelectionType_FromString(const std::string &s, SelectionProperties::SelectionType &val)
{
    val = SelectionProperties::BasicSelection;
    for(int i = 0; i < 2; ++i)
    {
        if(s == SelectionType_strings[i])
        {
            val = (SelectionType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SelectionProperties::CombinationType
//

static const char *CombinationType_strings[] = {
"CombineAnd", "CombineOr"};

std::string
SelectionProperties::CombinationType_ToString(SelectionProperties::CombinationType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return CombinationType_strings[index];
}

std::string
SelectionProperties::CombinationType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return CombinationType_strings[index];
}

bool
SelectionProperties::CombinationType_FromString(const std::string &s, SelectionProperties::CombinationType &val)
{
    val = SelectionProperties::CombineAnd;
    for(int i = 0; i < 2; ++i)
    {
        if(s == CombinationType_strings[i])
        {
            val = (CombinationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SelectionProperties::HistogramType
//

static const char *HistogramType_strings[] = {
"HistogramTime", "HistogramMatches", "HistogramID", 
"HistogramVariable"};

std::string
SelectionProperties::HistogramType_ToString(SelectionProperties::HistogramType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return HistogramType_strings[index];
}

std::string
SelectionProperties::HistogramType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return HistogramType_strings[index];
}

bool
SelectionProperties::HistogramType_FromString(const std::string &s, SelectionProperties::HistogramType &val)
{
    val = SelectionProperties::HistogramTime;
    for(int i = 0; i < 4; ++i)
    {
        if(s == HistogramType_strings[i])
        {
            val = (HistogramType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for SelectionProperties::IDVariableType
//

static const char *IDVariableType_strings[] = {
"UseZoneIDForID", "UseGlobalZoneIDForID", "UseLocationsForID", 
"UseVariableForID"};

std::string
SelectionProperties::IDVariableType_ToString(SelectionProperties::IDVariableType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return IDVariableType_strings[index];
}

std::string
SelectionProperties::IDVariableType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return IDVariableType_strings[index];
}

bool
SelectionProperties::IDVariableType_FromString(const std::string &s, SelectionProperties::IDVariableType &val)
{
    val = SelectionProperties::UseZoneIDForID;
    for(int i = 0; i < 4; ++i)
    {
        if(s == IDVariableType_strings[i])
        {
            val = (IDVariableType)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: SelectionProperties::SelectionProperties
//
// Purpose: 
//   Init utility for the SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void SelectionProperties::Init()
{
    host = "localhost";
    selectionType = BasicSelection;
    idVariableType = UseZoneIDForID;
    minTimeState = 0;
    maxTimeState = -1;
    timeStateStride = 1;
    combineRule = CombineOr;
    histogramType = HistogramTime;
    histogramNumBins = 10;
    histogramAutoScaleNumBins = false;
    histogramStartBin = 0;
    histogramEndBin = 9;

    SelectionProperties::SelectAll();
}

// ****************************************************************************
// Method: SelectionProperties::SelectionProperties
//
// Purpose: 
//   Copy utility for the SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void SelectionProperties::Copy(const SelectionProperties &obj)
{
    name = obj.name;
    source = obj.source;
    host = obj.host;
    selectionType = obj.selectionType;
    idVariableType = obj.idVariableType;
    idVariable = obj.idVariable;
    variables = obj.variables;
    variableMins = obj.variableMins;
    variableMaxs = obj.variableMaxs;
    minTimeState = obj.minTimeState;
    maxTimeState = obj.maxTimeState;
    timeStateStride = obj.timeStateStride;
    combineRule = obj.combineRule;
    histogramType = obj.histogramType;
    histogramNumBins = obj.histogramNumBins;
    histogramAutoScaleNumBins = obj.histogramAutoScaleNumBins;
    histogramStartBin = obj.histogramStartBin;
    histogramEndBin = obj.histogramEndBin;
    histogramVariable = obj.histogramVariable;

    SelectionProperties::SelectAll();
}

// Type map format string
const char *SelectionProperties::TypeMapFormatString = SELECTIONPROPERTIES_TMFS;
const AttributeGroup::private_tmfs_t SelectionProperties::TmfsStruct = {SELECTIONPROPERTIES_TMFS};


// ****************************************************************************
// Method: SelectionProperties::SelectionProperties
//
// Purpose: 
//   Default constructor for the SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SelectionProperties::SelectionProperties() : 
    AttributeSubject(SelectionProperties::TypeMapFormatString)
{
    SelectionProperties::Init();
}

// ****************************************************************************
// Method: SelectionProperties::SelectionProperties
//
// Purpose: 
//   Constructor for the derived classes of SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SelectionProperties::SelectionProperties(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    SelectionProperties::Init();
}

// ****************************************************************************
// Method: SelectionProperties::SelectionProperties
//
// Purpose: 
//   Copy constructor for the SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SelectionProperties::SelectionProperties(const SelectionProperties &obj) : 
    AttributeSubject(SelectionProperties::TypeMapFormatString)
{
    SelectionProperties::Copy(obj);
}

// ****************************************************************************
// Method: SelectionProperties::SelectionProperties
//
// Purpose: 
//   Copy constructor for derived classes of the SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SelectionProperties::SelectionProperties(const SelectionProperties &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    SelectionProperties::Copy(obj);
}

// ****************************************************************************
// Method: SelectionProperties::~SelectionProperties
//
// Purpose: 
//   Destructor for the SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SelectionProperties::~SelectionProperties()
{
    // nothing here
}

// ****************************************************************************
// Method: SelectionProperties::operator = 
//
// Purpose: 
//   Assignment operator for the SelectionProperties class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

SelectionProperties& 
SelectionProperties::operator = (const SelectionProperties &obj)
{
    if (this == &obj) return *this;

    SelectionProperties::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: SelectionProperties::operator == 
//
// Purpose: 
//   Comparison operator == for the SelectionProperties class.
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
SelectionProperties::operator == (const SelectionProperties &obj) const
{
    // Create the return value
    return ((name == obj.name) &&
            (source == obj.source) &&
            (host == obj.host) &&
            (selectionType == obj.selectionType) &&
            (idVariableType == obj.idVariableType) &&
            (idVariable == obj.idVariable) &&
            (variables == obj.variables) &&
            (variableMins == obj.variableMins) &&
            (variableMaxs == obj.variableMaxs) &&
            (minTimeState == obj.minTimeState) &&
            (maxTimeState == obj.maxTimeState) &&
            (timeStateStride == obj.timeStateStride) &&
            (combineRule == obj.combineRule) &&
            (histogramType == obj.histogramType) &&
            (histogramNumBins == obj.histogramNumBins) &&
            (histogramAutoScaleNumBins == obj.histogramAutoScaleNumBins) &&
            (histogramStartBin == obj.histogramStartBin) &&
            (histogramEndBin == obj.histogramEndBin) &&
            (histogramVariable == obj.histogramVariable));
}

// ****************************************************************************
// Method: SelectionProperties::operator != 
//
// Purpose: 
//   Comparison operator != for the SelectionProperties class.
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
SelectionProperties::operator != (const SelectionProperties &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: SelectionProperties::TypeName
//
// Purpose: 
//   Type name method for the SelectionProperties class.
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
SelectionProperties::TypeName() const
{
    return "SelectionProperties";
}

// ****************************************************************************
// Method: SelectionProperties::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the SelectionProperties class.
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
SelectionProperties::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const SelectionProperties *tmp = (const SelectionProperties *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: SelectionProperties::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the SelectionProperties class.
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
SelectionProperties::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new SelectionProperties(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: SelectionProperties::NewInstance
//
// Purpose: 
//   NewInstance method for the SelectionProperties class.
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
SelectionProperties::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new SelectionProperties(*this);
    else
        retval = new SelectionProperties;

    return retval;
}

// ****************************************************************************
// Method: SelectionProperties::SelectAll
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
SelectionProperties::SelectAll()
{
    Select(ID_name,                      (void *)&name);
    Select(ID_source,                    (void *)&source);
    Select(ID_host,                      (void *)&host);
    Select(ID_selectionType,             (void *)&selectionType);
    Select(ID_idVariableType,            (void *)&idVariableType);
    Select(ID_idVariable,                (void *)&idVariable);
    Select(ID_variables,                 (void *)&variables);
    Select(ID_variableMins,              (void *)&variableMins);
    Select(ID_variableMaxs,              (void *)&variableMaxs);
    Select(ID_minTimeState,              (void *)&minTimeState);
    Select(ID_maxTimeState,              (void *)&maxTimeState);
    Select(ID_timeStateStride,           (void *)&timeStateStride);
    Select(ID_combineRule,               (void *)&combineRule);
    Select(ID_histogramType,             (void *)&histogramType);
    Select(ID_histogramNumBins,          (void *)&histogramNumBins);
    Select(ID_histogramAutoScaleNumBins, (void *)&histogramAutoScaleNumBins);
    Select(ID_histogramStartBin,         (void *)&histogramStartBin);
    Select(ID_histogramEndBin,           (void *)&histogramEndBin);
    Select(ID_histogramVariable,         (void *)&histogramVariable);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SelectionProperties::CreateNode
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
SelectionProperties::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    SelectionProperties defaultObject;
    bool addToParent = false;
    // Create a node for SelectionProperties.
    DataNode *node = new DataNode("SelectionProperties");

    if(completeSave || !FieldsEqual(ID_name, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("name", name));
    }

    if(completeSave || !FieldsEqual(ID_source, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("source", source));
    }

    if(completeSave || !FieldsEqual(ID_host, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("host", host));
    }

    if(completeSave || !FieldsEqual(ID_selectionType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("selectionType", SelectionType_ToString(selectionType)));
    }

    if(completeSave || !FieldsEqual(ID_idVariableType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("idVariableType", IDVariableType_ToString(idVariableType)));
    }

    if(completeSave || !FieldsEqual(ID_idVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("idVariable", idVariable));
    }

    if(completeSave || !FieldsEqual(ID_variables, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("variables", variables));
    }

    if(completeSave || !FieldsEqual(ID_variableMins, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("variableMins", variableMins));
    }

    if(completeSave || !FieldsEqual(ID_variableMaxs, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("variableMaxs", variableMaxs));
    }

    if(completeSave || !FieldsEqual(ID_minTimeState, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minTimeState", minTimeState));
    }

    if(completeSave || !FieldsEqual(ID_maxTimeState, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxTimeState", maxTimeState));
    }

    if(completeSave || !FieldsEqual(ID_timeStateStride, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeStateStride", timeStateStride));
    }

    if(completeSave || !FieldsEqual(ID_combineRule, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("combineRule", CombinationType_ToString(combineRule)));
    }

    if(completeSave || !FieldsEqual(ID_histogramType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("histogramType", HistogramType_ToString(histogramType)));
    }

    if(completeSave || !FieldsEqual(ID_histogramNumBins, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("histogramNumBins", histogramNumBins));
    }

    if(completeSave || !FieldsEqual(ID_histogramAutoScaleNumBins, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("histogramAutoScaleNumBins", histogramAutoScaleNumBins));
    }

    if(completeSave || !FieldsEqual(ID_histogramStartBin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("histogramStartBin", histogramStartBin));
    }

    if(completeSave || !FieldsEqual(ID_histogramEndBin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("histogramEndBin", histogramEndBin));
    }

    if(completeSave || !FieldsEqual(ID_histogramVariable, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("histogramVariable", histogramVariable));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: SelectionProperties::SetFromNode
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
SelectionProperties::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("SelectionProperties");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("name")) != 0)
        SetName(node->AsString());
    if((node = searchNode->GetNode("source")) != 0)
        SetSource(node->AsString());
    if((node = searchNode->GetNode("host")) != 0)
        SetHost(node->AsString());
    if((node = searchNode->GetNode("selectionType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetSelectionType(SelectionType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SelectionType value;
            if(SelectionType_FromString(node->AsString(), value))
                SetSelectionType(value);
        }
    }
    if((node = searchNode->GetNode("idVariableType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetIdVariableType(IDVariableType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            IDVariableType value;
            if(IDVariableType_FromString(node->AsString(), value))
                SetIdVariableType(value);
        }
    }
    if((node = searchNode->GetNode("idVariable")) != 0)
        SetIdVariable(node->AsString());
    if((node = searchNode->GetNode("variables")) != 0)
        SetVariables(node->AsStringVector());
    if((node = searchNode->GetNode("variableMins")) != 0)
        SetVariableMins(node->AsDoubleVector());
    if((node = searchNode->GetNode("variableMaxs")) != 0)
        SetVariableMaxs(node->AsDoubleVector());
    if((node = searchNode->GetNode("minTimeState")) != 0)
        SetMinTimeState(node->AsInt());
    if((node = searchNode->GetNode("maxTimeState")) != 0)
        SetMaxTimeState(node->AsInt());
    if((node = searchNode->GetNode("timeStateStride")) != 0)
        SetTimeStateStride(node->AsInt());
    if((node = searchNode->GetNode("combineRule")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetCombineRule(CombinationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CombinationType value;
            if(CombinationType_FromString(node->AsString(), value))
                SetCombineRule(value);
        }
    }
    if((node = searchNode->GetNode("histogramType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetHistogramType(HistogramType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            HistogramType value;
            if(HistogramType_FromString(node->AsString(), value))
                SetHistogramType(value);
        }
    }
    if((node = searchNode->GetNode("histogramNumBins")) != 0)
        SetHistogramNumBins(node->AsInt());
    if((node = searchNode->GetNode("histogramAutoScaleNumBins")) != 0)
        SetHistogramAutoScaleNumBins(node->AsBool());
    if((node = searchNode->GetNode("histogramStartBin")) != 0)
        SetHistogramStartBin(node->AsInt());
    if((node = searchNode->GetNode("histogramEndBin")) != 0)
        SetHistogramEndBin(node->AsInt());
    if((node = searchNode->GetNode("histogramVariable")) != 0)
        SetHistogramVariable(node->AsString());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
SelectionProperties::SetName(const std::string &name_)
{
    name = name_;
    Select(ID_name, (void *)&name);
}

void
SelectionProperties::SetSource(const std::string &source_)
{
    source = source_;
    Select(ID_source, (void *)&source);
}

void
SelectionProperties::SetHost(const std::string &host_)
{
    host = host_;
    Select(ID_host, (void *)&host);
}

void
SelectionProperties::SetSelectionType(SelectionProperties::SelectionType selectionType_)
{
    selectionType = selectionType_;
    Select(ID_selectionType, (void *)&selectionType);
}

void
SelectionProperties::SetIdVariableType(SelectionProperties::IDVariableType idVariableType_)
{
    idVariableType = idVariableType_;
    Select(ID_idVariableType, (void *)&idVariableType);
}

void
SelectionProperties::SetIdVariable(const std::string &idVariable_)
{
    idVariable = idVariable_;
    Select(ID_idVariable, (void *)&idVariable);
}

void
SelectionProperties::SetVariables(const stringVector &variables_)
{
    variables = variables_;
    Select(ID_variables, (void *)&variables);
}

void
SelectionProperties::SetVariableMins(const doubleVector &variableMins_)
{
    variableMins = variableMins_;
    Select(ID_variableMins, (void *)&variableMins);
}

void
SelectionProperties::SetVariableMaxs(const doubleVector &variableMaxs_)
{
    variableMaxs = variableMaxs_;
    Select(ID_variableMaxs, (void *)&variableMaxs);
}

void
SelectionProperties::SetMinTimeState(int minTimeState_)
{
    minTimeState = minTimeState_;
    Select(ID_minTimeState, (void *)&minTimeState);
}

void
SelectionProperties::SetMaxTimeState(int maxTimeState_)
{
    maxTimeState = maxTimeState_;
    Select(ID_maxTimeState, (void *)&maxTimeState);
}

void
SelectionProperties::SetTimeStateStride(int timeStateStride_)
{
    timeStateStride = timeStateStride_;
    Select(ID_timeStateStride, (void *)&timeStateStride);
}

void
SelectionProperties::SetCombineRule(SelectionProperties::CombinationType combineRule_)
{
    combineRule = combineRule_;
    Select(ID_combineRule, (void *)&combineRule);
}

void
SelectionProperties::SetHistogramType(SelectionProperties::HistogramType histogramType_)
{
    histogramType = histogramType_;
    Select(ID_histogramType, (void *)&histogramType);
}

void
SelectionProperties::SetHistogramNumBins(int histogramNumBins_)
{
    histogramNumBins = histogramNumBins_;
    Select(ID_histogramNumBins, (void *)&histogramNumBins);
}

void
SelectionProperties::SetHistogramAutoScaleNumBins(bool histogramAutoScaleNumBins_)
{
    histogramAutoScaleNumBins = histogramAutoScaleNumBins_;
    Select(ID_histogramAutoScaleNumBins, (void *)&histogramAutoScaleNumBins);
}

void
SelectionProperties::SetHistogramStartBin(int histogramStartBin_)
{
    histogramStartBin = histogramStartBin_;
    Select(ID_histogramStartBin, (void *)&histogramStartBin);
}

void
SelectionProperties::SetHistogramEndBin(int histogramEndBin_)
{
    histogramEndBin = histogramEndBin_;
    Select(ID_histogramEndBin, (void *)&histogramEndBin);
}

void
SelectionProperties::SetHistogramVariable(const std::string &histogramVariable_)
{
    histogramVariable = histogramVariable_;
    Select(ID_histogramVariable, (void *)&histogramVariable);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

const std::string &
SelectionProperties::GetName() const
{
    return name;
}

std::string &
SelectionProperties::GetName()
{
    return name;
}

const std::string &
SelectionProperties::GetSource() const
{
    return source;
}

std::string &
SelectionProperties::GetSource()
{
    return source;
}

const std::string &
SelectionProperties::GetHost() const
{
    return host;
}

std::string &
SelectionProperties::GetHost()
{
    return host;
}

SelectionProperties::SelectionType
SelectionProperties::GetSelectionType() const
{
    return SelectionType(selectionType);
}

SelectionProperties::IDVariableType
SelectionProperties::GetIdVariableType() const
{
    return IDVariableType(idVariableType);
}

const std::string &
SelectionProperties::GetIdVariable() const
{
    return idVariable;
}

std::string &
SelectionProperties::GetIdVariable()
{
    return idVariable;
}

const stringVector &
SelectionProperties::GetVariables() const
{
    return variables;
}

stringVector &
SelectionProperties::GetVariables()
{
    return variables;
}

const doubleVector &
SelectionProperties::GetVariableMins() const
{
    return variableMins;
}

doubleVector &
SelectionProperties::GetVariableMins()
{
    return variableMins;
}

const doubleVector &
SelectionProperties::GetVariableMaxs() const
{
    return variableMaxs;
}

doubleVector &
SelectionProperties::GetVariableMaxs()
{
    return variableMaxs;
}

int
SelectionProperties::GetMinTimeState() const
{
    return minTimeState;
}

int
SelectionProperties::GetMaxTimeState() const
{
    return maxTimeState;
}

int
SelectionProperties::GetTimeStateStride() const
{
    return timeStateStride;
}

SelectionProperties::CombinationType
SelectionProperties::GetCombineRule() const
{
    return CombinationType(combineRule);
}

SelectionProperties::HistogramType
SelectionProperties::GetHistogramType() const
{
    return HistogramType(histogramType);
}

int
SelectionProperties::GetHistogramNumBins() const
{
    return histogramNumBins;
}

bool
SelectionProperties::GetHistogramAutoScaleNumBins() const
{
    return histogramAutoScaleNumBins;
}

int
SelectionProperties::GetHistogramStartBin() const
{
    return histogramStartBin;
}

int
SelectionProperties::GetHistogramEndBin() const
{
    return histogramEndBin;
}

const std::string &
SelectionProperties::GetHistogramVariable() const
{
    return histogramVariable;
}

std::string &
SelectionProperties::GetHistogramVariable()
{
    return histogramVariable;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
SelectionProperties::SelectName()
{
    Select(ID_name, (void *)&name);
}

void
SelectionProperties::SelectSource()
{
    Select(ID_source, (void *)&source);
}

void
SelectionProperties::SelectHost()
{
    Select(ID_host, (void *)&host);
}

void
SelectionProperties::SelectIdVariable()
{
    Select(ID_idVariable, (void *)&idVariable);
}

void
SelectionProperties::SelectVariables()
{
    Select(ID_variables, (void *)&variables);
}

void
SelectionProperties::SelectVariableMins()
{
    Select(ID_variableMins, (void *)&variableMins);
}

void
SelectionProperties::SelectVariableMaxs()
{
    Select(ID_variableMaxs, (void *)&variableMaxs);
}

void
SelectionProperties::SelectHistogramVariable()
{
    Select(ID_histogramVariable, (void *)&histogramVariable);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SelectionProperties::GetFieldName
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
SelectionProperties::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_name:                      return "name";
    case ID_source:                    return "source";
    case ID_host:                      return "host";
    case ID_selectionType:             return "selectionType";
    case ID_idVariableType:            return "idVariableType";
    case ID_idVariable:                return "idVariable";
    case ID_variables:                 return "variables";
    case ID_variableMins:              return "variableMins";
    case ID_variableMaxs:              return "variableMaxs";
    case ID_minTimeState:              return "minTimeState";
    case ID_maxTimeState:              return "maxTimeState";
    case ID_timeStateStride:           return "timeStateStride";
    case ID_combineRule:               return "combineRule";
    case ID_histogramType:             return "histogramType";
    case ID_histogramNumBins:          return "histogramNumBins";
    case ID_histogramAutoScaleNumBins: return "histogramAutoScaleNumBins";
    case ID_histogramStartBin:         return "histogramStartBin";
    case ID_histogramEndBin:           return "histogramEndBin";
    case ID_histogramVariable:         return "histogramVariable";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SelectionProperties::GetFieldType
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
SelectionProperties::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_name:                      return FieldType_string;
    case ID_source:                    return FieldType_string;
    case ID_host:                      return FieldType_string;
    case ID_selectionType:             return FieldType_enum;
    case ID_idVariableType:            return FieldType_enum;
    case ID_idVariable:                return FieldType_string;
    case ID_variables:                 return FieldType_stringVector;
    case ID_variableMins:              return FieldType_doubleVector;
    case ID_variableMaxs:              return FieldType_doubleVector;
    case ID_minTimeState:              return FieldType_int;
    case ID_maxTimeState:              return FieldType_int;
    case ID_timeStateStride:           return FieldType_int;
    case ID_combineRule:               return FieldType_enum;
    case ID_histogramType:             return FieldType_enum;
    case ID_histogramNumBins:          return FieldType_int;
    case ID_histogramAutoScaleNumBins: return FieldType_bool;
    case ID_histogramStartBin:         return FieldType_int;
    case ID_histogramEndBin:           return FieldType_int;
    case ID_histogramVariable:         return FieldType_string;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: SelectionProperties::GetFieldTypeName
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
SelectionProperties::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_name:                      return "string";
    case ID_source:                    return "string";
    case ID_host:                      return "string";
    case ID_selectionType:             return "enum";
    case ID_idVariableType:            return "enum";
    case ID_idVariable:                return "string";
    case ID_variables:                 return "stringVector";
    case ID_variableMins:              return "doubleVector";
    case ID_variableMaxs:              return "doubleVector";
    case ID_minTimeState:              return "int";
    case ID_maxTimeState:              return "int";
    case ID_timeStateStride:           return "int";
    case ID_combineRule:               return "enum";
    case ID_histogramType:             return "enum";
    case ID_histogramNumBins:          return "int";
    case ID_histogramAutoScaleNumBins: return "bool";
    case ID_histogramStartBin:         return "int";
    case ID_histogramEndBin:           return "int";
    case ID_histogramVariable:         return "string";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: SelectionProperties::FieldsEqual
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
SelectionProperties::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const SelectionProperties &obj = *((const SelectionProperties*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_name:
        {  // new scope
        retval = (name == obj.name);
        }
        break;
    case ID_source:
        {  // new scope
        retval = (source == obj.source);
        }
        break;
    case ID_host:
        {  // new scope
        retval = (host == obj.host);
        }
        break;
    case ID_selectionType:
        {  // new scope
        retval = (selectionType == obj.selectionType);
        }
        break;
    case ID_idVariableType:
        {  // new scope
        retval = (idVariableType == obj.idVariableType);
        }
        break;
    case ID_idVariable:
        {  // new scope
        retval = (idVariable == obj.idVariable);
        }
        break;
    case ID_variables:
        {  // new scope
        retval = (variables == obj.variables);
        }
        break;
    case ID_variableMins:
        {  // new scope
        retval = (variableMins == obj.variableMins);
        }
        break;
    case ID_variableMaxs:
        {  // new scope
        retval = (variableMaxs == obj.variableMaxs);
        }
        break;
    case ID_minTimeState:
        {  // new scope
        retval = (minTimeState == obj.minTimeState);
        }
        break;
    case ID_maxTimeState:
        {  // new scope
        retval = (maxTimeState == obj.maxTimeState);
        }
        break;
    case ID_timeStateStride:
        {  // new scope
        retval = (timeStateStride == obj.timeStateStride);
        }
        break;
    case ID_combineRule:
        {  // new scope
        retval = (combineRule == obj.combineRule);
        }
        break;
    case ID_histogramType:
        {  // new scope
        retval = (histogramType == obj.histogramType);
        }
        break;
    case ID_histogramNumBins:
        {  // new scope
        retval = (histogramNumBins == obj.histogramNumBins);
        }
        break;
    case ID_histogramAutoScaleNumBins:
        {  // new scope
        retval = (histogramAutoScaleNumBins == obj.histogramAutoScaleNumBins);
        }
        break;
    case ID_histogramStartBin:
        {  // new scope
        retval = (histogramStartBin == obj.histogramStartBin);
        }
        break;
    case ID_histogramEndBin:
        {  // new scope
        retval = (histogramEndBin == obj.histogramEndBin);
        }
        break;
    case ID_histogramVariable:
        {  // new scope
        retval = (histogramVariable == obj.histogramVariable);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// Synonym for GetSource but makes more sense when a plotName is expected.
const std::string &
SelectionProperties::GetOriginatingPlot() const
{
    return GetSource();
}

// Synonym for SetSource but makes more sense when a plotName is expected.
void
SelectionProperties::SetOriginatingPlot(const std::string &p)
{
    SetSource(p);
}

