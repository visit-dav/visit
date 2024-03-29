// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ReflectAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for ReflectAttributes::Octant
//

static const char *Octant_strings[] = {
"PXPYPZ", "NXPYPZ", "PXNYPZ",
"NXNYPZ", "PXPYNZ", "NXPYNZ",
"PXNYNZ", "NXNYNZ"};

std::string
ReflectAttributes::Octant_ToString(ReflectAttributes::Octant t)
{
    int index = int(t);
    if(index < 0 || index >= 8) index = 0;
    return Octant_strings[index];
}

std::string
ReflectAttributes::Octant_ToString(int t)
{
    int index = (t < 0 || t >= 8) ? 0 : t;
    return Octant_strings[index];
}

bool
ReflectAttributes::Octant_FromString(const std::string &s, ReflectAttributes::Octant &val)
{
    val = ReflectAttributes::PXPYPZ;
    for(int i = 0; i < 8; ++i)
    {
        if(s == Octant_strings[i])
        {
            val = (Octant)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ReflectAttributes::ReflectType
//

static const char *ReflectType_strings[] = {
"Plane", "Axis"};

std::string
ReflectAttributes::ReflectType_ToString(ReflectAttributes::ReflectType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ReflectType_strings[index];
}

std::string
ReflectAttributes::ReflectType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ReflectType_strings[index];
}

bool
ReflectAttributes::ReflectType_FromString(const std::string &s, ReflectAttributes::ReflectType &val)
{
    val = ReflectAttributes::Plane;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ReflectType_strings[i])
        {
            val = (ReflectType)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: ReflectAttributes::ReflectAttributes
//
// Purpose:
//   Init utility for the ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

void ReflectAttributes::Init()
{
    octant = PXPYPZ;
    useXBoundary = true;
    specifiedX = 0;
    useYBoundary = true;
    specifiedY = 0;
    useZBoundary = true;
    specifiedZ = 0;
    reflections[0] = 1;
    reflections[1] = 0;
    reflections[2] = 1;
    reflections[3] = 0;
    reflections[4] = 0;
    reflections[5] = 0;
    reflections[6] = 0;
    reflections[7] = 0;
    planePoint[0] = 0;
    planePoint[1] = 0;
    planePoint[2] = 0;
    planeNormal[0] = 0;
    planeNormal[1] = 0;
    planeNormal[2] = 0;
    reflectType = Axis;

    ReflectAttributes::SelectAll();
}

// ****************************************************************************
// Method: ReflectAttributes::ReflectAttributes
//
// Purpose:
//   Copy utility for the ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

void ReflectAttributes::Copy(const ReflectAttributes &obj)
{

    octant = obj.octant;
    useXBoundary = obj.useXBoundary;
    specifiedX = obj.specifiedX;
    useYBoundary = obj.useYBoundary;
    specifiedY = obj.specifiedY;
    useZBoundary = obj.useZBoundary;
    specifiedZ = obj.specifiedZ;
    for(int i = 0; i < 8; ++i)
        reflections[i] = obj.reflections[i];

    planePoint[0] = obj.planePoint[0];
    planePoint[1] = obj.planePoint[1];
    planePoint[2] = obj.planePoint[2];

    planeNormal[0] = obj.planeNormal[0];
    planeNormal[1] = obj.planeNormal[1];
    planeNormal[2] = obj.planeNormal[2];

    reflectType = obj.reflectType;

    ReflectAttributes::SelectAll();
}

// Type map format string
const char *ReflectAttributes::TypeMapFormatString = REFLECTATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t ReflectAttributes::TmfsStruct = {REFLECTATTRIBUTES_TMFS};


// ****************************************************************************
// Method: ReflectAttributes::ReflectAttributes
//
// Purpose:
//   Default constructor for the ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

ReflectAttributes::ReflectAttributes() :
    AttributeSubject(ReflectAttributes::TypeMapFormatString)
{
    ReflectAttributes::Init();
}

// ****************************************************************************
// Method: ReflectAttributes::ReflectAttributes
//
// Purpose:
//   Constructor for the derived classes of ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

ReflectAttributes::ReflectAttributes(private_tmfs_t tmfs) :
    AttributeSubject(tmfs.tmfs)
{
    ReflectAttributes::Init();
}

// ****************************************************************************
// Method: ReflectAttributes::ReflectAttributes
//
// Purpose:
//   Copy constructor for the ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

ReflectAttributes::ReflectAttributes(const ReflectAttributes &obj) :
    AttributeSubject(ReflectAttributes::TypeMapFormatString)
{
    ReflectAttributes::Copy(obj);
}

// ****************************************************************************
// Method: ReflectAttributes::ReflectAttributes
//
// Purpose:
//   Copy constructor for derived classes of the ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

ReflectAttributes::ReflectAttributes(const ReflectAttributes &obj, private_tmfs_t tmfs) :
    AttributeSubject(tmfs.tmfs)
{
    ReflectAttributes::Copy(obj);
}

// ****************************************************************************
// Method: ReflectAttributes::~ReflectAttributes
//
// Purpose:
//   Destructor for the ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

ReflectAttributes::~ReflectAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: ReflectAttributes::operator =
//
// Purpose:
//   Assignment operator for the ReflectAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

ReflectAttributes&
ReflectAttributes::operator = (const ReflectAttributes &obj)
{
    if (this == &obj) return *this;

    ReflectAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: ReflectAttributes::operator ==
//
// Purpose:
//   Comparison operator == for the ReflectAttributes class.
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
ReflectAttributes::operator == (const ReflectAttributes &obj) const
{
    // Compare the reflections arrays.
    bool reflections_equal = true;
    for(int i = 0; i < 8 && reflections_equal; ++i)
        reflections_equal = (reflections[i] == obj.reflections[i]);

    // Compare the planePoint arrays.
    bool planePoint_equal = true;
    for(int i = 0; i < 3 && planePoint_equal; ++i)
        planePoint_equal = (planePoint[i] == obj.planePoint[i]);

    // Compare the planeNormal arrays.
    bool planeNormal_equal = true;
    for(int i = 0; i < 3 && planeNormal_equal; ++i)
        planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

    // Create the return value
    return ((octant == obj.octant) &&
            (useXBoundary == obj.useXBoundary) &&
            (specifiedX == obj.specifiedX) &&
            (useYBoundary == obj.useYBoundary) &&
            (specifiedY == obj.specifiedY) &&
            (useZBoundary == obj.useZBoundary) &&
            (specifiedZ == obj.specifiedZ) &&
            reflections_equal &&
            planePoint_equal &&
            planeNormal_equal &&
            (reflectType == obj.reflectType));
}

// ****************************************************************************
// Method: ReflectAttributes::operator !=
//
// Purpose:
//   Comparison operator != for the ReflectAttributes class.
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
ReflectAttributes::operator != (const ReflectAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: ReflectAttributes::TypeName
//
// Purpose:
//   Type name method for the ReflectAttributes class.
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
ReflectAttributes::TypeName() const
{
    return "ReflectAttributes";
}

// ****************************************************************************
// Method: ReflectAttributes::CopyAttributes
//
// Purpose:
//   CopyAttributes method for the ReflectAttributes class.
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
ReflectAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const ReflectAttributes *tmp = (const ReflectAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: ReflectAttributes::CreateCompatible
//
// Purpose:
//   CreateCompatible method for the ReflectAttributes class.
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
ReflectAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new ReflectAttributes(*this);
    // Other cases could go here too.

    return retval;
}

// ****************************************************************************
// Method: ReflectAttributes::NewInstance
//
// Purpose:
//   NewInstance method for the ReflectAttributes class.
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
ReflectAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new ReflectAttributes(*this);
    else
        retval = new ReflectAttributes;

    return retval;
}

// ****************************************************************************
// Method: ReflectAttributes::SelectAll
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
ReflectAttributes::SelectAll()
{
    Select(ID_octant,       (void *)&octant);
    Select(ID_useXBoundary, (void *)&useXBoundary);
    Select(ID_specifiedX,   (void *)&specifiedX);
    Select(ID_useYBoundary, (void *)&useYBoundary);
    Select(ID_specifiedY,   (void *)&specifiedY);
    Select(ID_useZBoundary, (void *)&useZBoundary);
    Select(ID_specifiedZ,   (void *)&specifiedZ);
    Select(ID_reflections,  (void *)reflections, 8);
    Select(ID_planePoint,   (void *)planePoint, 3);
    Select(ID_planeNormal,  (void *)planeNormal, 3);
    Select(ID_reflectType,  (void *)&reflectType);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ReflectAttributes::CreateNode
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
ReflectAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    ReflectAttributes defaultObject;
    bool addToParent = false;
    // Create a node for ReflectAttributes.
    DataNode *node = new DataNode("ReflectAttributes");

    if(completeSave || !FieldsEqual(ID_octant, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("octant", Octant_ToString(octant)));
    }

    if(completeSave || !FieldsEqual(ID_useXBoundary, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useXBoundary", useXBoundary));
    }

    if(completeSave || !FieldsEqual(ID_specifiedX, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specifiedX", specifiedX));
    }

    if(completeSave || !FieldsEqual(ID_useYBoundary, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useYBoundary", useYBoundary));
    }

    if(completeSave || !FieldsEqual(ID_specifiedY, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specifiedY", specifiedY));
    }

    if(completeSave || !FieldsEqual(ID_useZBoundary, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("useZBoundary", useZBoundary));
    }

    if(completeSave || !FieldsEqual(ID_specifiedZ, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("specifiedZ", specifiedZ));
    }

    if(completeSave || !FieldsEqual(ID_reflections, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("reflections", reflections, 8));
    }

    if(completeSave || !FieldsEqual(ID_planePoint, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("planePoint", planePoint, 3));
    }

    if(completeSave || !FieldsEqual(ID_planeNormal, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("planeNormal", planeNormal, 3));
    }

    if(completeSave || !FieldsEqual(ID_reflectType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("reflectType", ReflectType_ToString(reflectType)));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: ReflectAttributes::SetFromNode
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
ReflectAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ReflectAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("octant")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 8)
                SetOctant(Octant(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            Octant value;
            if(Octant_FromString(node->AsString(), value))
                SetOctant(value);
        }
    }
    if((node = searchNode->GetNode("useXBoundary")) != 0)
        SetUseXBoundary(node->AsBool());
    if((node = searchNode->GetNode("specifiedX")) != 0)
        SetSpecifiedX(node->AsDouble());
    if((node = searchNode->GetNode("useYBoundary")) != 0)
        SetUseYBoundary(node->AsBool());
    if((node = searchNode->GetNode("specifiedY")) != 0)
        SetSpecifiedY(node->AsDouble());
    if((node = searchNode->GetNode("useZBoundary")) != 0)
        SetUseZBoundary(node->AsBool());
    if((node = searchNode->GetNode("specifiedZ")) != 0)
        SetSpecifiedZ(node->AsDouble());
    if((node = searchNode->GetNode("reflections")) != 0)
        SetReflections(node->AsIntArray());
    if((node = searchNode->GetNode("planePoint")) != 0)
        SetPlanePoint(node->AsDoubleArray());
    if((node = searchNode->GetNode("planeNormal")) != 0)
        SetPlaneNormal(node->AsDoubleArray());
    if((node = searchNode->GetNode("reflectType")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetReflectType(ReflectType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ReflectType value;
            if(ReflectType_FromString(node->AsString(), value))
                SetReflectType(value);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ReflectAttributes::SetOctant(ReflectAttributes::Octant octant_)
{
    octant = octant_;
    Select(ID_octant, (void *)&octant);
}

void
ReflectAttributes::SetUseXBoundary(bool useXBoundary_)
{
    useXBoundary = useXBoundary_;
    Select(ID_useXBoundary, (void *)&useXBoundary);
}

void
ReflectAttributes::SetSpecifiedX(double specifiedX_)
{
    specifiedX = specifiedX_;
    Select(ID_specifiedX, (void *)&specifiedX);
}

void
ReflectAttributes::SetUseYBoundary(bool useYBoundary_)
{
    useYBoundary = useYBoundary_;
    Select(ID_useYBoundary, (void *)&useYBoundary);
}

void
ReflectAttributes::SetSpecifiedY(double specifiedY_)
{
    specifiedY = specifiedY_;
    Select(ID_specifiedY, (void *)&specifiedY);
}

void
ReflectAttributes::SetUseZBoundary(bool useZBoundary_)
{
    useZBoundary = useZBoundary_;
    Select(ID_useZBoundary, (void *)&useZBoundary);
}

void
ReflectAttributes::SetSpecifiedZ(double specifiedZ_)
{
    specifiedZ = specifiedZ_;
    Select(ID_specifiedZ, (void *)&specifiedZ);
}

void
ReflectAttributes::SetReflections(const int *reflections_)
{
    for(int i = 0; i < 8; ++i)
        reflections[i] = reflections_[i];
    Select(ID_reflections, (void *)reflections, 8);
}

void
ReflectAttributes::SetPlanePoint(const double *planePoint_)
{
    planePoint[0] = planePoint_[0];
    planePoint[1] = planePoint_[1];
    planePoint[2] = planePoint_[2];
    Select(ID_planePoint, (void *)planePoint, 3);
}

void
ReflectAttributes::SetPlaneNormal(const double *planeNormal_)
{
    planeNormal[0] = planeNormal_[0];
    planeNormal[1] = planeNormal_[1];
    planeNormal[2] = planeNormal_[2];
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

void
ReflectAttributes::SetReflectType(ReflectAttributes::ReflectType reflectType_)
{
    reflectType = reflectType_;
    Select(ID_reflectType, (void *)&reflectType);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

ReflectAttributes::Octant
ReflectAttributes::GetOctant() const
{
    return Octant(octant);
}

bool
ReflectAttributes::GetUseXBoundary() const
{
    return useXBoundary;
}

double
ReflectAttributes::GetSpecifiedX() const
{
    return specifiedX;
}

bool
ReflectAttributes::GetUseYBoundary() const
{
    return useYBoundary;
}

double
ReflectAttributes::GetSpecifiedY() const
{
    return specifiedY;
}

bool
ReflectAttributes::GetUseZBoundary() const
{
    return useZBoundary;
}

double
ReflectAttributes::GetSpecifiedZ() const
{
    return specifiedZ;
}

const int *
ReflectAttributes::GetReflections() const
{
    return reflections;
}

int *
ReflectAttributes::GetReflections()
{
    return reflections;
}

const double *
ReflectAttributes::GetPlanePoint() const
{
    return planePoint;
}

double *
ReflectAttributes::GetPlanePoint()
{
    return planePoint;
}

const double *
ReflectAttributes::GetPlaneNormal() const
{
    return planeNormal;
}

double *
ReflectAttributes::GetPlaneNormal()
{
    return planeNormal;
}

ReflectAttributes::ReflectType
ReflectAttributes::GetReflectType() const
{
    return ReflectType(reflectType);
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ReflectAttributes::SelectReflections()
{
    Select(ID_reflections, (void *)reflections, 8);
}

void
ReflectAttributes::SelectPlanePoint()
{
    Select(ID_planePoint, (void *)planePoint, 3);
}

void
ReflectAttributes::SelectPlaneNormal()
{
    Select(ID_planeNormal, (void *)planeNormal, 3);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ReflectAttributes::GetFieldName
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
ReflectAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_octant:       return "octant";
    case ID_useXBoundary: return "useXBoundary";
    case ID_specifiedX:   return "specifiedX";
    case ID_useYBoundary: return "useYBoundary";
    case ID_specifiedY:   return "specifiedY";
    case ID_useZBoundary: return "useZBoundary";
    case ID_specifiedZ:   return "specifiedZ";
    case ID_reflections:  return "reflections";
    case ID_planePoint:   return "planePoint";
    case ID_planeNormal:  return "planeNormal";
    case ID_reflectType:  return "reflectType";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ReflectAttributes::GetFieldType
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
ReflectAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_octant:       return FieldType_enum;
    case ID_useXBoundary: return FieldType_bool;
    case ID_specifiedX:   return FieldType_double;
    case ID_useYBoundary: return FieldType_bool;
    case ID_specifiedY:   return FieldType_double;
    case ID_useZBoundary: return FieldType_bool;
    case ID_specifiedZ:   return FieldType_double;
    case ID_reflections:  return FieldType_intArray;
    case ID_planePoint:   return FieldType_doubleArray;
    case ID_planeNormal:  return FieldType_doubleArray;
    case ID_reflectType:  return FieldType_enum;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: ReflectAttributes::GetFieldTypeName
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
ReflectAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_octant:       return "enum";
    case ID_useXBoundary: return "bool";
    case ID_specifiedX:   return "double";
    case ID_useYBoundary: return "bool";
    case ID_specifiedY:   return "double";
    case ID_useZBoundary: return "bool";
    case ID_specifiedZ:   return "double";
    case ID_reflections:  return "intArray";
    case ID_planePoint:   return "doubleArray";
    case ID_planeNormal:  return "doubleArray";
    case ID_reflectType:  return "enum";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ReflectAttributes::FieldsEqual
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
ReflectAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ReflectAttributes &obj = *((const ReflectAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_octant:
        {  // new scope
        retval = (octant == obj.octant);
        }
        break;
    case ID_useXBoundary:
        {  // new scope
        retval = (useXBoundary == obj.useXBoundary);
        }
        break;
    case ID_specifiedX:
        {  // new scope
        retval = (specifiedX == obj.specifiedX);
        }
        break;
    case ID_useYBoundary:
        {  // new scope
        retval = (useYBoundary == obj.useYBoundary);
        }
        break;
    case ID_specifiedY:
        {  // new scope
        retval = (specifiedY == obj.specifiedY);
        }
        break;
    case ID_useZBoundary:
        {  // new scope
        retval = (useZBoundary == obj.useZBoundary);
        }
        break;
    case ID_specifiedZ:
        {  // new scope
        retval = (specifiedZ == obj.specifiedZ);
        }
        break;
    case ID_reflections:
        {  // new scope
        // Compare the reflections arrays.
        bool reflections_equal = true;
        for(int i = 0; i < 8 && reflections_equal; ++i)
            reflections_equal = (reflections[i] == obj.reflections[i]);

        retval = reflections_equal;
        }
        break;
    case ID_planePoint:
        {  // new scope
        // Compare the planePoint arrays.
        bool planePoint_equal = true;
        for(int i = 0; i < 3 && planePoint_equal; ++i)
            planePoint_equal = (planePoint[i] == obj.planePoint[i]);

        retval = planePoint_equal;
        }
        break;
    case ID_planeNormal:
        {  // new scope
        // Compare the planeNormal arrays.
        bool planeNormal_equal = true;
        for(int i = 0; i < 3 && planeNormal_equal; ++i)
            planeNormal_equal = (planeNormal[i] == obj.planeNormal[i]);

        retval = planeNormal_equal;
        }
        break;
    case ID_reflectType:
        {  // new scope
        retval = (reflectType == obj.reflectType);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

