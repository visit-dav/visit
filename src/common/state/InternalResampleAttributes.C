// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <InternalResampleAttributes.h>
#include <DataNode.h>

// ****************************************************************************
// Method: InternalResampleAttributes::InternalResampleAttributes
//
// Purpose:
//   Init utility for the InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

void InternalResampleAttributes::Init()
{
    useTargetVal = false;
    targetVal = 100000;
    width = 30;
    height = 30;
    depth = 30;
    prefersPowersOfTwo = false;
    defaultVal = -1e+38;
    useBounds = false;
    minX = 0;
    minY = 0;
    minZ = 0;
    maxX = 1;
    maxY = 1;
    maxZ = 1;
    useArbitrator = false;
    arbitratorLessThan = false;
    arbitratorVarName = "default";
    distributedResample = false;
    perRankResample = false;

    InternalResampleAttributes::SelectAll();
}

// ****************************************************************************
// Method: InternalResampleAttributes::InternalResampleAttributes
//
// Purpose:
//   Copy utility for the InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

void InternalResampleAttributes::Copy(const InternalResampleAttributes &obj)
{
    useTargetVal = obj.useTargetVal;
    targetVal = obj.targetVal;
    width = obj.width;
    height = obj.height;
    depth = obj.depth;
    prefersPowersOfTwo = obj.prefersPowersOfTwo;
    defaultVal = obj.defaultVal;
    useBounds = obj.useBounds;
    minX = obj.minX;
    minY = obj.minY;
    minZ = obj.minZ;
    maxX = obj.maxX;
    maxY = obj.maxY;
    maxZ = obj.maxZ;
    useArbitrator = obj.useArbitrator;
    arbitratorLessThan = obj.arbitratorLessThan;
    arbitratorVarName = obj.arbitratorVarName;
    distributedResample = obj.distributedResample;
    perRankResample = obj.perRankResample;

    InternalResampleAttributes::SelectAll();
}

// Type map format string
const char *InternalResampleAttributes::TypeMapFormatString = INTERNALRESAMPLEATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t InternalResampleAttributes::TmfsStruct = {INTERNALRESAMPLEATTRIBUTES_TMFS};


// ****************************************************************************
// Method: InternalResampleAttributes::InternalResampleAttributes
//
// Purpose:
//   Default constructor for the InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

InternalResampleAttributes::InternalResampleAttributes() :
    AttributeSubject(InternalResampleAttributes::TypeMapFormatString)
{
    InternalResampleAttributes::Init();
}

// ****************************************************************************
// Method: InternalResampleAttributes::InternalResampleAttributes
//
// Purpose:
//   Constructor for the derived classes of InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

InternalResampleAttributes::InternalResampleAttributes(private_tmfs_t tmfs) :
    AttributeSubject(tmfs.tmfs)
{
    InternalResampleAttributes::Init();
}

// ****************************************************************************
// Method: InternalResampleAttributes::InternalResampleAttributes
//
// Purpose:
//   Copy constructor for the InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

InternalResampleAttributes::InternalResampleAttributes(const InternalResampleAttributes &obj) :
    AttributeSubject(InternalResampleAttributes::TypeMapFormatString)
{
    InternalResampleAttributes::Copy(obj);
}

// ****************************************************************************
// Method: InternalResampleAttributes::InternalResampleAttributes
//
// Purpose:
//   Copy constructor for derived classes of the InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

InternalResampleAttributes::InternalResampleAttributes(const InternalResampleAttributes &obj, private_tmfs_t tmfs) :
    AttributeSubject(tmfs.tmfs)
{
    InternalResampleAttributes::Copy(obj);
}

// ****************************************************************************
// Method: InternalResampleAttributes::~InternalResampleAttributes
//
// Purpose:
//   Destructor for the InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

InternalResampleAttributes::~InternalResampleAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: InternalResampleAttributes::operator =
//
// Purpose:
//   Assignment operator for the InternalResampleAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

InternalResampleAttributes&
InternalResampleAttributes::operator = (const InternalResampleAttributes &obj)
{
    if (this == &obj) return *this;

    InternalResampleAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: InternalResampleAttributes::operator ==
//
// Purpose:
//   Comparison operator == for the InternalResampleAttributes class.
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
InternalResampleAttributes::operator == (const InternalResampleAttributes &obj) const
{
    // Create the return value
    return ((useTargetVal == obj.useTargetVal) &&
            (targetVal == obj.targetVal) &&
            (width == obj.width) &&
            (height == obj.height) &&
            (depth == obj.depth) &&
            (prefersPowersOfTwo == obj.prefersPowersOfTwo) &&
            (defaultVal == obj.defaultVal) &&
            (useBounds == obj.useBounds) &&
            (minX == obj.minX) &&
            (minY == obj.minY) &&
            (minZ == obj.minZ) &&
            (maxX == obj.maxX) &&
            (maxY == obj.maxY) &&
            (maxZ == obj.maxZ) &&
            (useArbitrator == obj.useArbitrator) &&
            (arbitratorLessThan == obj.arbitratorLessThan) &&
            (arbitratorVarName == obj.arbitratorVarName) &&
            (distributedResample == obj.distributedResample) &&
            (perRankResample == obj.perRankResample));
}

// ****************************************************************************
// Method: InternalResampleAttributes::operator !=
//
// Purpose:
//   Comparison operator != for the InternalResampleAttributes class.
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
InternalResampleAttributes::operator != (const InternalResampleAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: InternalResampleAttributes::TypeName
//
// Purpose:
//   Type name method for the InternalResampleAttributes class.
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
InternalResampleAttributes::TypeName() const
{
    return "InternalResampleAttributes";
}

// ****************************************************************************
// Method: InternalResampleAttributes::CopyAttributes
//
// Purpose:
//   CopyAttributes method for the InternalResampleAttributes class.
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
InternalResampleAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const InternalResampleAttributes *tmp = (const InternalResampleAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: InternalResampleAttributes::CreateCompatible
//
// Purpose:
//   CreateCompatible method for the InternalResampleAttributes class.
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
InternalResampleAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new InternalResampleAttributes(*this);
    // Other cases could go here too.

    return retval;
}

// ****************************************************************************
// Method: InternalResampleAttributes::NewInstance
//
// Purpose:
//   NewInstance method for the InternalResampleAttributes class.
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
InternalResampleAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new InternalResampleAttributes(*this);
    else
        retval = new InternalResampleAttributes;

    return retval;
}

// ****************************************************************************
// Method: InternalResampleAttributes::SelectAll
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
InternalResampleAttributes::SelectAll()
{
    Select(ID_useTargetVal,        (void *)&useTargetVal);
    Select(ID_targetVal,           (void *)&targetVal);
    Select(ID_width,               (void *)&width);
    Select(ID_height,              (void *)&height);
    Select(ID_depth,               (void *)&depth);
    Select(ID_prefersPowersOfTwo,  (void *)&prefersPowersOfTwo);
    Select(ID_defaultVal,          (void *)&defaultVal);
    Select(ID_useBounds,           (void *)&useBounds);
    Select(ID_minX,                (void *)&minX);
    Select(ID_minY,                (void *)&minY);
    Select(ID_minZ,                (void *)&minZ);
    Select(ID_maxX,                (void *)&maxX);
    Select(ID_maxY,                (void *)&maxY);
    Select(ID_maxZ,                (void *)&maxZ);
    Select(ID_useArbitrator,       (void *)&useArbitrator);
    Select(ID_arbitratorLessThan,  (void *)&arbitratorLessThan);
    Select(ID_arbitratorVarName,   (void *)&arbitratorVarName);
    Select(ID_distributedResample, (void *)&distributedResample);
    Select(ID_perRankResample,     (void *)&perRankResample);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: InternalResampleAttributes::CreateNode
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
InternalResampleAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    InternalResampleAttributes defaultObject;
    bool addToParent = false;
    // Create a node for InternalResampleAttributes.
    DataNode *node = new DataNode("InternalResampleAttributes");

    if(completeSave || !FieldsEqual(ID_useTargetVal, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useTargetVal", useTargetVal));
    }

    if(completeSave || !FieldsEqual(ID_targetVal, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("targetVal", targetVal));
    }

    if(completeSave || !FieldsEqual(ID_width, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("width", width));
    }

    if(completeSave || !FieldsEqual(ID_height, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("height", height));
    }

    if(completeSave || !FieldsEqual(ID_depth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("depth", depth));
    }

    if(completeSave || !FieldsEqual(ID_prefersPowersOfTwo, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("prefersPowersOfTwo", prefersPowersOfTwo));
    }

    if(completeSave || !FieldsEqual(ID_defaultVal, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("defaultVal", defaultVal));
    }

    if(completeSave || !FieldsEqual(ID_useBounds, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useBounds", useBounds));
    }

    if(completeSave || !FieldsEqual(ID_minX, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minX", minX));
    }

    if(completeSave || !FieldsEqual(ID_minY, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minY", minY));
    }

    if(completeSave || !FieldsEqual(ID_minZ, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("minZ", minZ));
    }

    if(completeSave || !FieldsEqual(ID_maxX, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxX", maxX));
    }

    if(completeSave || !FieldsEqual(ID_maxY, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxY", maxY));
    }

    if(completeSave || !FieldsEqual(ID_maxZ, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("maxZ", maxZ));
    }

    if(completeSave || !FieldsEqual(ID_useArbitrator, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useArbitrator", useArbitrator));
    }

    if(completeSave || !FieldsEqual(ID_arbitratorLessThan, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arbitratorLessThan", arbitratorLessThan));
    }

    if(completeSave || !FieldsEqual(ID_arbitratorVarName, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("arbitratorVarName", arbitratorVarName));
    }

    if(completeSave || !FieldsEqual(ID_distributedResample, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("distributedResample", distributedResample));
    }

    if(completeSave || !FieldsEqual(ID_perRankResample, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("perRankResample", perRankResample));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: InternalResampleAttributes::SetFromNode
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
InternalResampleAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("InternalResampleAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("useTargetVal")) != 0)
        SetUseTargetVal(node->AsBool());
    if((node = searchNode->GetNode("targetVal")) != 0)
        SetTargetVal(node->AsInt());
    if((node = searchNode->GetNode("width")) != 0)
        SetWidth(node->AsInt());
    if((node = searchNode->GetNode("height")) != 0)
        SetHeight(node->AsInt());
    if((node = searchNode->GetNode("depth")) != 0)
        SetDepth(node->AsInt());
    if((node = searchNode->GetNode("prefersPowersOfTwo")) != 0)
        SetPrefersPowersOfTwo(node->AsBool());
    if((node = searchNode->GetNode("defaultVal")) != 0)
        SetDefaultVal(node->AsFloat());
    if((node = searchNode->GetNode("useBounds")) != 0)
        SetUseBounds(node->AsBool());
    if((node = searchNode->GetNode("minX")) != 0)
        SetMinX(node->AsDouble());
    if((node = searchNode->GetNode("minY")) != 0)
        SetMinY(node->AsDouble());
    if((node = searchNode->GetNode("minZ")) != 0)
        SetMinZ(node->AsDouble());
    if((node = searchNode->GetNode("maxX")) != 0)
        SetMaxX(node->AsDouble());
    if((node = searchNode->GetNode("maxY")) != 0)
        SetMaxY(node->AsDouble());
    if((node = searchNode->GetNode("maxZ")) != 0)
        SetMaxZ(node->AsDouble());
    if((node = searchNode->GetNode("useArbitrator")) != 0)
        SetUseArbitrator(node->AsBool());
    if((node = searchNode->GetNode("arbitratorLessThan")) != 0)
        SetArbitratorLessThan(node->AsBool());
    if((node = searchNode->GetNode("arbitratorVarName")) != 0)
        SetArbitratorVarName(node->AsString());
    if((node = searchNode->GetNode("distributedResample")) != 0)
        SetDistributedResample(node->AsBool());
    if((node = searchNode->GetNode("perRankResample")) != 0)
        SetPerRankResample(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
InternalResampleAttributes::SetUseTargetVal(bool useTargetVal_)
{
    useTargetVal = useTargetVal_;
    Select(ID_useTargetVal, (void *)&useTargetVal);
}

void
InternalResampleAttributes::SetTargetVal(int targetVal_)
{
    targetVal = targetVal_;
    Select(ID_targetVal, (void *)&targetVal);
}

void
InternalResampleAttributes::SetWidth(int width_)
{
    width = width_;
    Select(ID_width, (void *)&width);
}

void
InternalResampleAttributes::SetHeight(int height_)
{
    height = height_;
    Select(ID_height, (void *)&height);
}

void
InternalResampleAttributes::SetDepth(int depth_)
{
    depth = depth_;
    Select(ID_depth, (void *)&depth);
}

void
InternalResampleAttributes::SetPrefersPowersOfTwo(bool prefersPowersOfTwo_)
{
    prefersPowersOfTwo = prefersPowersOfTwo_;
    Select(ID_prefersPowersOfTwo, (void *)&prefersPowersOfTwo);
}

void
InternalResampleAttributes::SetDefaultVal(float defaultVal_)
{
    defaultVal = defaultVal_;
    Select(ID_defaultVal, (void *)&defaultVal);
}

void
InternalResampleAttributes::SetUseBounds(bool useBounds_)
{
    useBounds = useBounds_;
    Select(ID_useBounds, (void *)&useBounds);
}

void
InternalResampleAttributes::SetMinX(double minX_)
{
    minX = minX_;
    Select(ID_minX, (void *)&minX);
}

void
InternalResampleAttributes::SetMinY(double minY_)
{
    minY = minY_;
    Select(ID_minY, (void *)&minY);
}

void
InternalResampleAttributes::SetMinZ(double minZ_)
{
    minZ = minZ_;
    Select(ID_minZ, (void *)&minZ);
}

void
InternalResampleAttributes::SetMaxX(double maxX_)
{
    maxX = maxX_;
    Select(ID_maxX, (void *)&maxX);
}

void
InternalResampleAttributes::SetMaxY(double maxY_)
{
    maxY = maxY_;
    Select(ID_maxY, (void *)&maxY);
}

void
InternalResampleAttributes::SetMaxZ(double maxZ_)
{
    maxZ = maxZ_;
    Select(ID_maxZ, (void *)&maxZ);
}

void
InternalResampleAttributes::SetUseArbitrator(bool useArbitrator_)
{
    useArbitrator = useArbitrator_;
    Select(ID_useArbitrator, (void *)&useArbitrator);
}

void
InternalResampleAttributes::SetArbitratorLessThan(bool arbitratorLessThan_)
{
    arbitratorLessThan = arbitratorLessThan_;
    Select(ID_arbitratorLessThan, (void *)&arbitratorLessThan);
}

void
InternalResampleAttributes::SetArbitratorVarName(const std::string &arbitratorVarName_)
{
    arbitratorVarName = arbitratorVarName_;
    Select(ID_arbitratorVarName, (void *)&arbitratorVarName);
}

void
InternalResampleAttributes::SetDistributedResample(bool distributedResample_)
{
    distributedResample = distributedResample_;
    Select(ID_distributedResample, (void *)&distributedResample);
}

void
InternalResampleAttributes::SetPerRankResample(bool perRankResample_)
{
    perRankResample = perRankResample_;
    Select(ID_perRankResample, (void *)&perRankResample);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

bool
InternalResampleAttributes::GetUseTargetVal() const
{
    return useTargetVal;
}

int
InternalResampleAttributes::GetTargetVal() const
{
    return targetVal;
}

int
InternalResampleAttributes::GetWidth() const
{
    return width;
}

int
InternalResampleAttributes::GetHeight() const
{
    return height;
}

int
InternalResampleAttributes::GetDepth() const
{
    return depth;
}

bool
InternalResampleAttributes::GetPrefersPowersOfTwo() const
{
    return prefersPowersOfTwo;
}

float
InternalResampleAttributes::GetDefaultVal() const
{
    return defaultVal;
}

bool
InternalResampleAttributes::GetUseBounds() const
{
    return useBounds;
}

double
InternalResampleAttributes::GetMinX() const
{
    return minX;
}

double
InternalResampleAttributes::GetMinY() const
{
    return minY;
}

double
InternalResampleAttributes::GetMinZ() const
{
    return minZ;
}

double
InternalResampleAttributes::GetMaxX() const
{
    return maxX;
}

double
InternalResampleAttributes::GetMaxY() const
{
    return maxY;
}

double
InternalResampleAttributes::GetMaxZ() const
{
    return maxZ;
}

bool
InternalResampleAttributes::GetUseArbitrator() const
{
    return useArbitrator;
}

bool
InternalResampleAttributes::GetArbitratorLessThan() const
{
    return arbitratorLessThan;
}

const std::string &
InternalResampleAttributes::GetArbitratorVarName() const
{
    return arbitratorVarName;
}

std::string &
InternalResampleAttributes::GetArbitratorVarName()
{
    return arbitratorVarName;
}

bool
InternalResampleAttributes::GetDistributedResample() const
{
    return distributedResample;
}

bool
InternalResampleAttributes::GetPerRankResample() const
{
    return perRankResample;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
InternalResampleAttributes::SelectArbitratorVarName()
{
    Select(ID_arbitratorVarName, (void *)&arbitratorVarName);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: InternalResampleAttributes::GetFieldName
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
InternalResampleAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_useTargetVal:        return "useTargetVal";
    case ID_targetVal:           return "targetVal";
    case ID_width:               return "width";
    case ID_height:              return "height";
    case ID_depth:               return "depth";
    case ID_prefersPowersOfTwo:  return "prefersPowersOfTwo";
    case ID_defaultVal:          return "defaultVal";
    case ID_useBounds:           return "useBounds";
    case ID_minX:                return "minX";
    case ID_minY:                return "minY";
    case ID_minZ:                return "minZ";
    case ID_maxX:                return "maxX";
    case ID_maxY:                return "maxY";
    case ID_maxZ:                return "maxZ";
    case ID_useArbitrator:       return "useArbitrator";
    case ID_arbitratorLessThan:  return "arbitratorLessThan";
    case ID_arbitratorVarName:   return "arbitratorVarName";
    case ID_distributedResample: return "distributedResample";
    case ID_perRankResample:     return "perRankResample";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: InternalResampleAttributes::GetFieldType
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
InternalResampleAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_useTargetVal:        return FieldType_bool;
    case ID_targetVal:           return FieldType_int;
    case ID_width:               return FieldType_int;
    case ID_height:              return FieldType_int;
    case ID_depth:               return FieldType_int;
    case ID_prefersPowersOfTwo:  return FieldType_bool;
    case ID_defaultVal:          return FieldType_float;
    case ID_useBounds:           return FieldType_bool;
    case ID_minX:                return FieldType_double;
    case ID_minY:                return FieldType_double;
    case ID_minZ:                return FieldType_double;
    case ID_maxX:                return FieldType_double;
    case ID_maxY:                return FieldType_double;
    case ID_maxZ:                return FieldType_double;
    case ID_useArbitrator:       return FieldType_bool;
    case ID_arbitratorLessThan:  return FieldType_bool;
    case ID_arbitratorVarName:   return FieldType_string;
    case ID_distributedResample: return FieldType_bool;
    case ID_perRankResample:     return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: InternalResampleAttributes::GetFieldTypeName
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
InternalResampleAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_useTargetVal:        return "bool";
    case ID_targetVal:           return "int";
    case ID_width:               return "int";
    case ID_height:              return "int";
    case ID_depth:               return "int";
    case ID_prefersPowersOfTwo:  return "bool";
    case ID_defaultVal:          return "float";
    case ID_useBounds:           return "bool";
    case ID_minX:                return "double";
    case ID_minY:                return "double";
    case ID_minZ:                return "double";
    case ID_maxX:                return "double";
    case ID_maxY:                return "double";
    case ID_maxZ:                return "double";
    case ID_useArbitrator:       return "bool";
    case ID_arbitratorLessThan:  return "bool";
    case ID_arbitratorVarName:   return "string";
    case ID_distributedResample: return "bool";
    case ID_perRankResample:     return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: InternalResampleAttributes::FieldsEqual
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
InternalResampleAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const InternalResampleAttributes &obj = *((const InternalResampleAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_useTargetVal:
        {  // new scope
        retval = (useTargetVal == obj.useTargetVal);
        }
        break;
    case ID_targetVal:
        {  // new scope
        retval = (targetVal == obj.targetVal);
        }
        break;
    case ID_width:
        {  // new scope
        retval = (width == obj.width);
        }
        break;
    case ID_height:
        {  // new scope
        retval = (height == obj.height);
        }
        break;
    case ID_depth:
        {  // new scope
        retval = (depth == obj.depth);
        }
        break;
    case ID_prefersPowersOfTwo:
        {  // new scope
        retval = (prefersPowersOfTwo == obj.prefersPowersOfTwo);
        }
        break;
    case ID_defaultVal:
        {  // new scope
        retval = (defaultVal == obj.defaultVal);
        }
        break;
    case ID_useBounds:
        {  // new scope
        retval = (useBounds == obj.useBounds);
        }
        break;
    case ID_minX:
        {  // new scope
        retval = (minX == obj.minX);
        }
        break;
    case ID_minY:
        {  // new scope
        retval = (minY == obj.minY);
        }
        break;
    case ID_minZ:
        {  // new scope
        retval = (minZ == obj.minZ);
        }
        break;
    case ID_maxX:
        {  // new scope
        retval = (maxX == obj.maxX);
        }
        break;
    case ID_maxY:
        {  // new scope
        retval = (maxY == obj.maxY);
        }
        break;
    case ID_maxZ:
        {  // new scope
        retval = (maxZ == obj.maxZ);
        }
        break;
    case ID_useArbitrator:
        {  // new scope
        retval = (useArbitrator == obj.useArbitrator);
        }
        break;
    case ID_arbitratorLessThan:
        {  // new scope
        retval = (arbitratorLessThan == obj.arbitratorLessThan);
        }
        break;
    case ID_arbitratorVarName:
        {  // new scope
        retval = (arbitratorVarName == obj.arbitratorVarName);
        }
        break;
    case ID_distributedResample:
        {  // new scope
        retval = (distributedResample == obj.distributedResample);
        }
        break;
    case ID_perRankResample:
        {  // new scope
        retval = (perRankResample == obj.perRankResample);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

