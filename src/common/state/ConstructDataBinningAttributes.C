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

#include <ConstructDataBinningAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for ConstructDataBinningAttributes::BinningScheme
//

static const char *BinningScheme_strings[] = {
"Uniform", "Unknown"};

std::string
ConstructDataBinningAttributes::BinningScheme_ToString(ConstructDataBinningAttributes::BinningScheme t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return BinningScheme_strings[index];
}

std::string
ConstructDataBinningAttributes::BinningScheme_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return BinningScheme_strings[index];
}

bool
ConstructDataBinningAttributes::BinningScheme_FromString(const std::string &s, ConstructDataBinningAttributes::BinningScheme &val)
{
    val = ConstructDataBinningAttributes::Uniform;
    for(int i = 0; i < 2; ++i)
    {
        if(s == BinningScheme_strings[i])
        {
            val = (BinningScheme)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ConstructDataBinningAttributes::ReductionOperator
//

static const char *ReductionOperator_strings[] = {
"Average", "Minimum", "Maximum", 
"StandardDeviation", "Variance", "Sum", 
"Count", "RMS", "PDF"
};

std::string
ConstructDataBinningAttributes::ReductionOperator_ToString(ConstructDataBinningAttributes::ReductionOperator t)
{
    int index = int(t);
    if(index < 0 || index >= 9) index = 0;
    return ReductionOperator_strings[index];
}

std::string
ConstructDataBinningAttributes::ReductionOperator_ToString(int t)
{
    int index = (t < 0 || t >= 9) ? 0 : t;
    return ReductionOperator_strings[index];
}

bool
ConstructDataBinningAttributes::ReductionOperator_FromString(const std::string &s, ConstructDataBinningAttributes::ReductionOperator &val)
{
    val = ConstructDataBinningAttributes::Average;
    for(int i = 0; i < 9; ++i)
    {
        if(s == ReductionOperator_strings[i])
        {
            val = (ReductionOperator)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ConstructDataBinningAttributes::OutOfBoundsBehavior
//

static const char *OutOfBoundsBehavior_strings[] = {
"Clamp", "Discard"};

std::string
ConstructDataBinningAttributes::OutOfBoundsBehavior_ToString(ConstructDataBinningAttributes::OutOfBoundsBehavior t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OutOfBoundsBehavior_strings[index];
}

std::string
ConstructDataBinningAttributes::OutOfBoundsBehavior_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OutOfBoundsBehavior_strings[index];
}

bool
ConstructDataBinningAttributes::OutOfBoundsBehavior_FromString(const std::string &s, ConstructDataBinningAttributes::OutOfBoundsBehavior &val)
{
    val = ConstructDataBinningAttributes::Clamp;
    for(int i = 0; i < 2; ++i)
    {
        if(s == OutOfBoundsBehavior_strings[i])
        {
            val = (OutOfBoundsBehavior)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ConstructDataBinningAttributes::BinType
//

static const char *BinType_strings[] = {
"Variable", "X", "Y", 
"Z"};

std::string
ConstructDataBinningAttributes::BinType_ToString(ConstructDataBinningAttributes::BinType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return BinType_strings[index];
}

std::string
ConstructDataBinningAttributes::BinType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return BinType_strings[index];
}

bool
ConstructDataBinningAttributes::BinType_FromString(const std::string &s, ConstructDataBinningAttributes::BinType &val)
{
    val = ConstructDataBinningAttributes::Variable;
    for(int i = 0; i < 4; ++i)
    {
        if(s == BinType_strings[i])
        {
            val = (BinType)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::ConstructDataBinningAttributes
//
// Purpose: 
//   Init utility for the ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void ConstructDataBinningAttributes::Init()
{
    reductionOperator = Average;
    undefinedValue = 0;
    binningScheme = Uniform;
    overTime = false;
    timeStart = 0;
    timeEnd = 1;
    timeStride = 1;
    outOfBoundsBehavior = Clamp;

    ConstructDataBinningAttributes::SelectAll();
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::ConstructDataBinningAttributes
//
// Purpose: 
//   Copy utility for the ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void ConstructDataBinningAttributes::Copy(const ConstructDataBinningAttributes &obj)
{
    name = obj.name;
    varnames = obj.varnames;
    binType = obj.binType;
    binBoundaries = obj.binBoundaries;
    reductionOperator = obj.reductionOperator;
    varForReductionOperator = obj.varForReductionOperator;
    undefinedValue = obj.undefinedValue;
    binningScheme = obj.binningScheme;
    numBins = obj.numBins;
    overTime = obj.overTime;
    timeStart = obj.timeStart;
    timeEnd = obj.timeEnd;
    timeStride = obj.timeStride;
    outOfBoundsBehavior = obj.outOfBoundsBehavior;

    ConstructDataBinningAttributes::SelectAll();
}

// Type map format string
const char *ConstructDataBinningAttributes::TypeMapFormatString = CONSTRUCTDATABINNINGATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t ConstructDataBinningAttributes::TmfsStruct = {CONSTRUCTDATABINNINGATTRIBUTES_TMFS};


// ****************************************************************************
// Method: ConstructDataBinningAttributes::ConstructDataBinningAttributes
//
// Purpose: 
//   Default constructor for the ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ConstructDataBinningAttributes::ConstructDataBinningAttributes() : 
    AttributeSubject(ConstructDataBinningAttributes::TypeMapFormatString)
{
    ConstructDataBinningAttributes::Init();
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::ConstructDataBinningAttributes
//
// Purpose: 
//   Constructor for the derived classes of ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ConstructDataBinningAttributes::ConstructDataBinningAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    ConstructDataBinningAttributes::Init();
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::ConstructDataBinningAttributes
//
// Purpose: 
//   Copy constructor for the ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ConstructDataBinningAttributes::ConstructDataBinningAttributes(const ConstructDataBinningAttributes &obj) : 
    AttributeSubject(ConstructDataBinningAttributes::TypeMapFormatString)
{
    ConstructDataBinningAttributes::Copy(obj);
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::ConstructDataBinningAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ConstructDataBinningAttributes::ConstructDataBinningAttributes(const ConstructDataBinningAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    ConstructDataBinningAttributes::Copy(obj);
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::~ConstructDataBinningAttributes
//
// Purpose: 
//   Destructor for the ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ConstructDataBinningAttributes::~ConstructDataBinningAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the ConstructDataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ConstructDataBinningAttributes& 
ConstructDataBinningAttributes::operator = (const ConstructDataBinningAttributes &obj)
{
    if (this == &obj) return *this;

    ConstructDataBinningAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the ConstructDataBinningAttributes class.
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
ConstructDataBinningAttributes::operator == (const ConstructDataBinningAttributes &obj) const
{
    // Create the return value
    return ((name == obj.name) &&
            (varnames == obj.varnames) &&
            (binType == obj.binType) &&
            (binBoundaries == obj.binBoundaries) &&
            (reductionOperator == obj.reductionOperator) &&
            (varForReductionOperator == obj.varForReductionOperator) &&
            (undefinedValue == obj.undefinedValue) &&
            (binningScheme == obj.binningScheme) &&
            (numBins == obj.numBins) &&
            (overTime == obj.overTime) &&
            (timeStart == obj.timeStart) &&
            (timeEnd == obj.timeEnd) &&
            (timeStride == obj.timeStride) &&
            (outOfBoundsBehavior == obj.outOfBoundsBehavior));
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the ConstructDataBinningAttributes class.
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
ConstructDataBinningAttributes::operator != (const ConstructDataBinningAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::TypeName
//
// Purpose: 
//   Type name method for the ConstructDataBinningAttributes class.
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
ConstructDataBinningAttributes::TypeName() const
{
    return "ConstructDataBinningAttributes";
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the ConstructDataBinningAttributes class.
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
ConstructDataBinningAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const ConstructDataBinningAttributes *tmp = (const ConstructDataBinningAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the ConstructDataBinningAttributes class.
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
ConstructDataBinningAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new ConstructDataBinningAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the ConstructDataBinningAttributes class.
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
ConstructDataBinningAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new ConstructDataBinningAttributes(*this);
    else
        retval = new ConstructDataBinningAttributes;

    return retval;
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::SelectAll
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
ConstructDataBinningAttributes::SelectAll()
{
    Select(ID_name,                    (void *)&name);
    Select(ID_varnames,                (void *)&varnames);
    Select(ID_binType,                 (void *)&binType);
    Select(ID_binBoundaries,           (void *)&binBoundaries);
    Select(ID_reductionOperator,       (void *)&reductionOperator);
    Select(ID_varForReductionOperator, (void *)&varForReductionOperator);
    Select(ID_undefinedValue,          (void *)&undefinedValue);
    Select(ID_binningScheme,           (void *)&binningScheme);
    Select(ID_numBins,                 (void *)&numBins);
    Select(ID_overTime,                (void *)&overTime);
    Select(ID_timeStart,               (void *)&timeStart);
    Select(ID_timeEnd,                 (void *)&timeEnd);
    Select(ID_timeStride,              (void *)&timeStride);
    Select(ID_outOfBoundsBehavior,     (void *)&outOfBoundsBehavior);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ConstructDataBinningAttributes::CreateNode
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
ConstructDataBinningAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    ConstructDataBinningAttributes defaultObject;
    bool addToParent = false;
    // Create a node for ConstructDataBinningAttributes.
    DataNode *node = new DataNode("ConstructDataBinningAttributes");

    if(completeSave || !FieldsEqual(ID_name, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("name", name));
    }

    if(completeSave || !FieldsEqual(ID_varnames, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("varnames", varnames));
    }

    if(completeSave || !FieldsEqual(ID_binType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("binType", binType));
    }

    if(completeSave || !FieldsEqual(ID_binBoundaries, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("binBoundaries", binBoundaries));
    }

    if(completeSave || !FieldsEqual(ID_reductionOperator, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("reductionOperator", ReductionOperator_ToString(reductionOperator)));
    }

    if(completeSave || !FieldsEqual(ID_varForReductionOperator, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("varForReductionOperator", varForReductionOperator));
    }

    if(completeSave || !FieldsEqual(ID_undefinedValue, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("undefinedValue", undefinedValue));
    }

    if(completeSave || !FieldsEqual(ID_binningScheme, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("binningScheme", BinningScheme_ToString(binningScheme)));
    }

    if(completeSave || !FieldsEqual(ID_numBins, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numBins", numBins));
    }

    if(completeSave || !FieldsEqual(ID_overTime, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("overTime", overTime));
    }

    if(completeSave || !FieldsEqual(ID_timeStart, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeStart", timeStart));
    }

    if(completeSave || !FieldsEqual(ID_timeEnd, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeEnd", timeEnd));
    }

    if(completeSave || !FieldsEqual(ID_timeStride, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("timeStride", timeStride));
    }

    if(completeSave || !FieldsEqual(ID_outOfBoundsBehavior, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("outOfBoundsBehavior", OutOfBoundsBehavior_ToString(outOfBoundsBehavior)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::SetFromNode
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
ConstructDataBinningAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ConstructDataBinningAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("name")) != 0)
        SetName(node->AsString());
    if((node = searchNode->GetNode("varnames")) != 0)
        SetVarnames(node->AsStringVector());
    if((node = searchNode->GetNode("binType")) != 0)
        SetBinType(node->AsUnsignedCharVector());
    if((node = searchNode->GetNode("binBoundaries")) != 0)
        SetBinBoundaries(node->AsDoubleVector());
    if((node = searchNode->GetNode("reductionOperator")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 9)
                SetReductionOperator(ReductionOperator(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ReductionOperator value;
            if(ReductionOperator_FromString(node->AsString(), value))
                SetReductionOperator(value);
        }
    }
    if((node = searchNode->GetNode("varForReductionOperator")) != 0)
        SetVarForReductionOperator(node->AsString());
    if((node = searchNode->GetNode("undefinedValue")) != 0)
        SetUndefinedValue(node->AsDouble());
    if((node = searchNode->GetNode("binningScheme")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetBinningScheme(BinningScheme(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            BinningScheme value;
            if(BinningScheme_FromString(node->AsString(), value))
                SetBinningScheme(value);
        }
    }
    if((node = searchNode->GetNode("numBins")) != 0)
        SetNumBins(node->AsIntVector());
    if((node = searchNode->GetNode("overTime")) != 0)
        SetOverTime(node->AsBool());
    if((node = searchNode->GetNode("timeStart")) != 0)
        SetTimeStart(node->AsInt());
    if((node = searchNode->GetNode("timeEnd")) != 0)
        SetTimeEnd(node->AsInt());
    if((node = searchNode->GetNode("timeStride")) != 0)
        SetTimeStride(node->AsInt());
    if((node = searchNode->GetNode("outOfBoundsBehavior")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetOutOfBoundsBehavior(OutOfBoundsBehavior(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OutOfBoundsBehavior value;
            if(OutOfBoundsBehavior_FromString(node->AsString(), value))
                SetOutOfBoundsBehavior(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ConstructDataBinningAttributes::SetName(const std::string &name_)
{
    name = name_;
    Select(ID_name, (void *)&name);
}

void
ConstructDataBinningAttributes::SetVarnames(const stringVector &varnames_)
{
    varnames = varnames_;
    Select(ID_varnames, (void *)&varnames);
}

void
ConstructDataBinningAttributes::SetBinType(const unsignedCharVector &binType_)
{
    binType = binType_;
    Select(ID_binType, (void *)&binType);
}

void
ConstructDataBinningAttributes::SetBinBoundaries(const doubleVector &binBoundaries_)
{
    binBoundaries = binBoundaries_;
    Select(ID_binBoundaries, (void *)&binBoundaries);
}

void
ConstructDataBinningAttributes::SetReductionOperator(ConstructDataBinningAttributes::ReductionOperator reductionOperator_)
{
    reductionOperator = reductionOperator_;
    Select(ID_reductionOperator, (void *)&reductionOperator);
}

void
ConstructDataBinningAttributes::SetVarForReductionOperator(const std::string &varForReductionOperator_)
{
    varForReductionOperator = varForReductionOperator_;
    Select(ID_varForReductionOperator, (void *)&varForReductionOperator);
}

void
ConstructDataBinningAttributes::SetUndefinedValue(double undefinedValue_)
{
    undefinedValue = undefinedValue_;
    Select(ID_undefinedValue, (void *)&undefinedValue);
}

void
ConstructDataBinningAttributes::SetBinningScheme(ConstructDataBinningAttributes::BinningScheme binningScheme_)
{
    binningScheme = binningScheme_;
    Select(ID_binningScheme, (void *)&binningScheme);
}

void
ConstructDataBinningAttributes::SetNumBins(const intVector &numBins_)
{
    numBins = numBins_;
    Select(ID_numBins, (void *)&numBins);
}

void
ConstructDataBinningAttributes::SetOverTime(bool overTime_)
{
    overTime = overTime_;
    Select(ID_overTime, (void *)&overTime);
}

void
ConstructDataBinningAttributes::SetTimeStart(int timeStart_)
{
    timeStart = timeStart_;
    Select(ID_timeStart, (void *)&timeStart);
}

void
ConstructDataBinningAttributes::SetTimeEnd(int timeEnd_)
{
    timeEnd = timeEnd_;
    Select(ID_timeEnd, (void *)&timeEnd);
}

void
ConstructDataBinningAttributes::SetTimeStride(int timeStride_)
{
    timeStride = timeStride_;
    Select(ID_timeStride, (void *)&timeStride);
}

void
ConstructDataBinningAttributes::SetOutOfBoundsBehavior(ConstructDataBinningAttributes::OutOfBoundsBehavior outOfBoundsBehavior_)
{
    outOfBoundsBehavior = outOfBoundsBehavior_;
    Select(ID_outOfBoundsBehavior, (void *)&outOfBoundsBehavior);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

const std::string &
ConstructDataBinningAttributes::GetName() const
{
    return name;
}

std::string &
ConstructDataBinningAttributes::GetName()
{
    return name;
}

const stringVector &
ConstructDataBinningAttributes::GetVarnames() const
{
    return varnames;
}

stringVector &
ConstructDataBinningAttributes::GetVarnames()
{
    return varnames;
}

const unsignedCharVector &
ConstructDataBinningAttributes::GetBinType() const
{
    return binType;
}

unsignedCharVector &
ConstructDataBinningAttributes::GetBinType()
{
    return binType;
}

const doubleVector &
ConstructDataBinningAttributes::GetBinBoundaries() const
{
    return binBoundaries;
}

doubleVector &
ConstructDataBinningAttributes::GetBinBoundaries()
{
    return binBoundaries;
}

ConstructDataBinningAttributes::ReductionOperator
ConstructDataBinningAttributes::GetReductionOperator() const
{
    return ReductionOperator(reductionOperator);
}

const std::string &
ConstructDataBinningAttributes::GetVarForReductionOperator() const
{
    return varForReductionOperator;
}

std::string &
ConstructDataBinningAttributes::GetVarForReductionOperator()
{
    return varForReductionOperator;
}

double
ConstructDataBinningAttributes::GetUndefinedValue() const
{
    return undefinedValue;
}

ConstructDataBinningAttributes::BinningScheme
ConstructDataBinningAttributes::GetBinningScheme() const
{
    return BinningScheme(binningScheme);
}

const intVector &
ConstructDataBinningAttributes::GetNumBins() const
{
    return numBins;
}

intVector &
ConstructDataBinningAttributes::GetNumBins()
{
    return numBins;
}

bool
ConstructDataBinningAttributes::GetOverTime() const
{
    return overTime;
}

int
ConstructDataBinningAttributes::GetTimeStart() const
{
    return timeStart;
}

int
ConstructDataBinningAttributes::GetTimeEnd() const
{
    return timeEnd;
}

int
ConstructDataBinningAttributes::GetTimeStride() const
{
    return timeStride;
}

ConstructDataBinningAttributes::OutOfBoundsBehavior
ConstructDataBinningAttributes::GetOutOfBoundsBehavior() const
{
    return OutOfBoundsBehavior(outOfBoundsBehavior);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ConstructDataBinningAttributes::SelectName()
{
    Select(ID_name, (void *)&name);
}

void
ConstructDataBinningAttributes::SelectVarnames()
{
    Select(ID_varnames, (void *)&varnames);
}

void
ConstructDataBinningAttributes::SelectBinType()
{
    Select(ID_binType, (void *)&binType);
}

void
ConstructDataBinningAttributes::SelectBinBoundaries()
{
    Select(ID_binBoundaries, (void *)&binBoundaries);
}

void
ConstructDataBinningAttributes::SelectVarForReductionOperator()
{
    Select(ID_varForReductionOperator, (void *)&varForReductionOperator);
}

void
ConstructDataBinningAttributes::SelectNumBins()
{
    Select(ID_numBins, (void *)&numBins);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ConstructDataBinningAttributes::GetFieldName
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
ConstructDataBinningAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_name:                    return "name";
    case ID_varnames:                return "varnames";
    case ID_binType:                 return "binType";
    case ID_binBoundaries:           return "binBoundaries";
    case ID_reductionOperator:       return "reductionOperator";
    case ID_varForReductionOperator: return "varForReductionOperator";
    case ID_undefinedValue:          return "undefinedValue";
    case ID_binningScheme:           return "binningScheme";
    case ID_numBins:                 return "numBins";
    case ID_overTime:                return "overTime";
    case ID_timeStart:               return "timeStart";
    case ID_timeEnd:                 return "timeEnd";
    case ID_timeStride:              return "timeStride";
    case ID_outOfBoundsBehavior:     return "outOfBoundsBehavior";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::GetFieldType
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
ConstructDataBinningAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_name:                    return FieldType_string;
    case ID_varnames:                return FieldType_stringVector;
    case ID_binType:                 return FieldType_ucharVector;
    case ID_binBoundaries:           return FieldType_doubleVector;
    case ID_reductionOperator:       return FieldType_enum;
    case ID_varForReductionOperator: return FieldType_string;
    case ID_undefinedValue:          return FieldType_double;
    case ID_binningScheme:           return FieldType_enum;
    case ID_numBins:                 return FieldType_intVector;
    case ID_overTime:                return FieldType_bool;
    case ID_timeStart:               return FieldType_int;
    case ID_timeEnd:                 return FieldType_int;
    case ID_timeStride:              return FieldType_int;
    case ID_outOfBoundsBehavior:     return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::GetFieldTypeName
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
ConstructDataBinningAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_name:                    return "string";
    case ID_varnames:                return "stringVector";
    case ID_binType:                 return "ucharVector";
    case ID_binBoundaries:           return "doubleVector";
    case ID_reductionOperator:       return "enum";
    case ID_varForReductionOperator: return "string";
    case ID_undefinedValue:          return "double";
    case ID_binningScheme:           return "enum";
    case ID_numBins:                 return "intVector";
    case ID_overTime:                return "bool";
    case ID_timeStart:               return "int";
    case ID_timeEnd:                 return "int";
    case ID_timeStride:              return "int";
    case ID_outOfBoundsBehavior:     return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ConstructDataBinningAttributes::FieldsEqual
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
ConstructDataBinningAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ConstructDataBinningAttributes &obj = *((const ConstructDataBinningAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_name:
        {  // new scope
        retval = (name == obj.name);
        }
        break;
    case ID_varnames:
        {  // new scope
        retval = (varnames == obj.varnames);
        }
        break;
    case ID_binType:
        {  // new scope
        retval = (binType == obj.binType);
        }
        break;
    case ID_binBoundaries:
        {  // new scope
        retval = (binBoundaries == obj.binBoundaries);
        }
        break;
    case ID_reductionOperator:
        {  // new scope
        retval = (reductionOperator == obj.reductionOperator);
        }
        break;
    case ID_varForReductionOperator:
        {  // new scope
        retval = (varForReductionOperator == obj.varForReductionOperator);
        }
        break;
    case ID_undefinedValue:
        {  // new scope
        retval = (undefinedValue == obj.undefinedValue);
        }
        break;
    case ID_binningScheme:
        {  // new scope
        retval = (binningScheme == obj.binningScheme);
        }
        break;
    case ID_numBins:
        {  // new scope
        retval = (numBins == obj.numBins);
        }
        break;
    case ID_overTime:
        {  // new scope
        retval = (overTime == obj.overTime);
        }
        break;
    case ID_timeStart:
        {  // new scope
        retval = (timeStart == obj.timeStart);
        }
        break;
    case ID_timeEnd:
        {  // new scope
        retval = (timeEnd == obj.timeEnd);
        }
        break;
    case ID_timeStride:
        {  // new scope
        retval = (timeStride == obj.timeStride);
        }
        break;
    case ID_outOfBoundsBehavior:
        {  // new scope
        retval = (outOfBoundsBehavior == obj.outOfBoundsBehavior);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// *******************************************************************
//  Method: ConstructDataBinningAttributes::ReductionRequiresVariable
//
//  Purpose:
//      Determine if a variable is needed to perform the reduction.
//
//  Programmer: Hank Childs
//  Creation:   August 19th, 2010
//
// *******************************************************************

bool
ConstructDataBinningAttributes::ReductionRequiresVariable(void)
{
    if (reductionOperator == Count || reductionOperator == PDF)
        return false;
    return true;
}

