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

#include <QueryList.h>
#include <DataNode.h>

//
// Enum conversion methods for QueryList::QueryType
//

static const char *QueryType_strings[] = {
"DatabaseQuery", "PointQuery", "LineQuery"
};

std::string
QueryList::QueryType_ToString(QueryList::QueryType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return QueryType_strings[index];
}

std::string
QueryList::QueryType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return QueryType_strings[index];
}

bool
QueryList::QueryType_FromString(const std::string &s, QueryList::QueryType &val)
{
    val = QueryList::DatabaseQuery;
    for(int i = 0; i < 3; ++i)
    {
        if(s == QueryType_strings[i])
        {
            val = (QueryType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for QueryList::WindowType
//

static const char *WindowType_strings[] = {
"Basic", "SinglePoint", "DoublePoint", 
"DomainNode", "DomainNodeVars", "DomainZone", 
"DomainZoneVars", "ActualData", "ActualDataVars", 
"LineDistribution", "HohlraumFlux", "ConnCompSummary", 
"ShapeletsDecomp"};

std::string
QueryList::WindowType_ToString(QueryList::WindowType t)
{
    int index = int(t);
    if(index < 0 || index >= 13) index = 0;
    return WindowType_strings[index];
}

std::string
QueryList::WindowType_ToString(int t)
{
    int index = (t < 0 || t >= 13) ? 0 : t;
    return WindowType_strings[index];
}

bool
QueryList::WindowType_FromString(const std::string &s, QueryList::WindowType &val)
{
    val = QueryList::Basic;
    for(int i = 0; i < 13; ++i)
    {
        if(s == WindowType_strings[i])
        {
            val = (WindowType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for QueryList::Groups
//

static const char *Groups_strings[] = {
"CurveRelated", "MeshRelated", "PickRelated", 
"TimeRelated", "VariableRelated", "ShapeRelated", 
"ConnectedComponentsRelated", "Miscellaneous", "NumGroups"
};

std::string
QueryList::Groups_ToString(QueryList::Groups t)
{
    int index = int(t);
    if(index < 0 || index >= 9) index = 0;
    return Groups_strings[index];
}

std::string
QueryList::Groups_ToString(int t)
{
    int index = (t < 0 || t >= 9) ? 0 : t;
    return Groups_strings[index];
}

bool
QueryList::Groups_FromString(const std::string &s, QueryList::Groups &val)
{
    val = QueryList::CurveRelated;
    for(int i = 0; i < 9; ++i)
    {
        if(s == Groups_strings[i])
        {
            val = (Groups)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for QueryList::QueryMode
//

static const char *QueryMode_strings[] = {
"QueryOnly", "QueryAndTime", "TimeOnly"
};

std::string
QueryList::QueryMode_ToString(QueryList::QueryMode t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return QueryMode_strings[index];
}

std::string
QueryList::QueryMode_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return QueryMode_strings[index];
}

bool
QueryList::QueryMode_FromString(const std::string &s, QueryList::QueryMode &val)
{
    val = QueryList::QueryOnly;
    for(int i = 0; i < 3; ++i)
    {
        if(s == QueryMode_strings[i])
        {
            val = (QueryMode)i;
            return true;
        }
    }
    return false;
}

// Type map format string
const char *QueryList::TypeMapFormatString = "s*i*i*i*i*i*i*i*i*";

// ****************************************************************************
// Method: QueryList::QueryList
//
// Purpose: 
//   Constructor for the QueryList class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

QueryList::QueryList() : 
    AttributeSubject(QueryList::TypeMapFormatString)
{
}

// ****************************************************************************
// Method: QueryList::QueryList
//
// Purpose: 
//   Copy constructor for the QueryList class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

QueryList::QueryList(const QueryList &obj) : 
    AttributeSubject(QueryList::TypeMapFormatString)
{
    names = obj.names;
    types = obj.types;
    groups = obj.groups;
    numInputs = obj.numInputs;
    allowedVarTypes = obj.allowedVarTypes;
    winType = obj.winType;
    queryMode = obj.queryMode;
    numVars = obj.numVars;
    canBePublic = obj.canBePublic;

    SelectAll();
}

// ****************************************************************************
// Method: QueryList::~QueryList
//
// Purpose: 
//   Destructor for the QueryList class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

QueryList::~QueryList()
{
    // nothing here
}

// ****************************************************************************
// Method: QueryList::operator = 
//
// Purpose: 
//   Assignment operator for the QueryList class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

QueryList& 
QueryList::operator = (const QueryList &obj)
{
    if (this == &obj) return *this;
    names = obj.names;
    types = obj.types;
    groups = obj.groups;
    numInputs = obj.numInputs;
    allowedVarTypes = obj.allowedVarTypes;
    winType = obj.winType;
    queryMode = obj.queryMode;
    numVars = obj.numVars;
    canBePublic = obj.canBePublic;

    SelectAll();
    return *this;
}

// ****************************************************************************
// Method: QueryList::operator == 
//
// Purpose: 
//   Comparison operator == for the QueryList class.
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
QueryList::operator == (const QueryList &obj) const
{
    // Create the return value
    return ((names == obj.names) &&
            (types == obj.types) &&
            (groups == obj.groups) &&
            (numInputs == obj.numInputs) &&
            (allowedVarTypes == obj.allowedVarTypes) &&
            (winType == obj.winType) &&
            (queryMode == obj.queryMode) &&
            (numVars == obj.numVars) &&
            (canBePublic == obj.canBePublic));
}

// ****************************************************************************
// Method: QueryList::operator != 
//
// Purpose: 
//   Comparison operator != for the QueryList class.
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
QueryList::operator != (const QueryList &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: QueryList::TypeName
//
// Purpose: 
//   Type name method for the QueryList class.
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
QueryList::TypeName() const
{
    return "QueryList";
}

// ****************************************************************************
// Method: QueryList::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the QueryList class.
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
QueryList::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const QueryList *tmp = (const QueryList *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: QueryList::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the QueryList class.
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
QueryList::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new QueryList(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: QueryList::NewInstance
//
// Purpose: 
//   NewInstance method for the QueryList class.
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
QueryList::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new QueryList(*this);
    else
        retval = new QueryList;

    return retval;
}

// ****************************************************************************
// Method: QueryList::SelectAll
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
QueryList::SelectAll()
{
    Select(ID_names,           (void *)&names);
    Select(ID_types,           (void *)&types);
    Select(ID_groups,          (void *)&groups);
    Select(ID_numInputs,       (void *)&numInputs);
    Select(ID_allowedVarTypes, (void *)&allowedVarTypes);
    Select(ID_winType,         (void *)&winType);
    Select(ID_queryMode,       (void *)&queryMode);
    Select(ID_numVars,         (void *)&numVars);
    Select(ID_canBePublic,     (void *)&canBePublic);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: QueryList::CreateNode
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
QueryList::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    QueryList defaultObject;
    bool addToParent = false;
    // Create a node for QueryList.
    DataNode *node = new DataNode("QueryList");

    if(completeSave || !FieldsEqual(ID_names, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("names", names));
    }

    if(completeSave || !FieldsEqual(ID_types, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("types", types));
    }

    if(completeSave || !FieldsEqual(ID_groups, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("groups", groups));
    }

    if(completeSave || !FieldsEqual(ID_numInputs, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numInputs", numInputs));
    }

    if(completeSave || !FieldsEqual(ID_allowedVarTypes, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("allowedVarTypes", allowedVarTypes));
    }

    if(completeSave || !FieldsEqual(ID_winType, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("winType", winType));
    }

    if(completeSave || !FieldsEqual(ID_queryMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("queryMode", queryMode));
    }

    if(completeSave || !FieldsEqual(ID_numVars, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numVars", numVars));
    }

    if(completeSave || !FieldsEqual(ID_canBePublic, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("canBePublic", canBePublic));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: QueryList::SetFromNode
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
QueryList::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("QueryList");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("names")) != 0)
        SetNames(node->AsStringVector());
    if((node = searchNode->GetNode("types")) != 0)
        SetTypes(node->AsIntVector());
    if((node = searchNode->GetNode("groups")) != 0)
        SetGroups(node->AsIntVector());
    if((node = searchNode->GetNode("numInputs")) != 0)
        SetNumInputs(node->AsIntVector());
    if((node = searchNode->GetNode("allowedVarTypes")) != 0)
        SetAllowedVarTypes(node->AsIntVector());
    if((node = searchNode->GetNode("winType")) != 0)
        SetWinType(node->AsIntVector());
    if((node = searchNode->GetNode("queryMode")) != 0)
        SetQueryMode(node->AsIntVector());
    if((node = searchNode->GetNode("numVars")) != 0)
        SetNumVars(node->AsIntVector());
    if((node = searchNode->GetNode("canBePublic")) != 0)
        SetCanBePublic(node->AsIntVector());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
QueryList::SetNames(const stringVector &names_)
{
    names = names_;
    Select(ID_names, (void *)&names);
}

void
QueryList::SetTypes(const intVector &types_)
{
    types = types_;
    Select(ID_types, (void *)&types);
}

void
QueryList::SetGroups(const intVector &groups_)
{
    groups = groups_;
    Select(ID_groups, (void *)&groups);
}

void
QueryList::SetNumInputs(const intVector &numInputs_)
{
    numInputs = numInputs_;
    Select(ID_numInputs, (void *)&numInputs);
}

void
QueryList::SetAllowedVarTypes(const intVector &allowedVarTypes_)
{
    allowedVarTypes = allowedVarTypes_;
    Select(ID_allowedVarTypes, (void *)&allowedVarTypes);
}

void
QueryList::SetWinType(const intVector &winType_)
{
    winType = winType_;
    Select(ID_winType, (void *)&winType);
}

void
QueryList::SetQueryMode(const intVector &queryMode_)
{
    queryMode = queryMode_;
    Select(ID_queryMode, (void *)&queryMode);
}

void
QueryList::SetNumVars(const intVector &numVars_)
{
    numVars = numVars_;
    Select(ID_numVars, (void *)&numVars);
}

void
QueryList::SetCanBePublic(const intVector &canBePublic_)
{
    canBePublic = canBePublic_;
    Select(ID_canBePublic, (void *)&canBePublic);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

const stringVector &
QueryList::GetNames() const
{
    return names;
}

stringVector &
QueryList::GetNames()
{
    return names;
}

const intVector &
QueryList::GetTypes() const
{
    return types;
}

intVector &
QueryList::GetTypes()
{
    return types;
}

const intVector &
QueryList::GetGroups() const
{
    return groups;
}

intVector &
QueryList::GetGroups()
{
    return groups;
}

const intVector &
QueryList::GetNumInputs() const
{
    return numInputs;
}

intVector &
QueryList::GetNumInputs()
{
    return numInputs;
}

const intVector &
QueryList::GetAllowedVarTypes() const
{
    return allowedVarTypes;
}

intVector &
QueryList::GetAllowedVarTypes()
{
    return allowedVarTypes;
}

const intVector &
QueryList::GetWinType() const
{
    return winType;
}

intVector &
QueryList::GetWinType()
{
    return winType;
}

const intVector &
QueryList::GetQueryMode() const
{
    return queryMode;
}

intVector &
QueryList::GetQueryMode()
{
    return queryMode;
}

const intVector &
QueryList::GetNumVars() const
{
    return numVars;
}

intVector &
QueryList::GetNumVars()
{
    return numVars;
}

const intVector &
QueryList::GetCanBePublic() const
{
    return canBePublic;
}

intVector &
QueryList::GetCanBePublic()
{
    return canBePublic;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
QueryList::SelectNames()
{
    Select(ID_names, (void *)&names);
}

void
QueryList::SelectTypes()
{
    Select(ID_types, (void *)&types);
}

void
QueryList::SelectGroups()
{
    Select(ID_groups, (void *)&groups);
}

void
QueryList::SelectNumInputs()
{
    Select(ID_numInputs, (void *)&numInputs);
}

void
QueryList::SelectAllowedVarTypes()
{
    Select(ID_allowedVarTypes, (void *)&allowedVarTypes);
}

void
QueryList::SelectWinType()
{
    Select(ID_winType, (void *)&winType);
}

void
QueryList::SelectQueryMode()
{
    Select(ID_queryMode, (void *)&queryMode);
}

void
QueryList::SelectNumVars()
{
    Select(ID_numVars, (void *)&numVars);
}

void
QueryList::SelectCanBePublic()
{
    Select(ID_canBePublic, (void *)&canBePublic);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: QueryList::GetFieldName
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
QueryList::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_names:           return "names";
    case ID_types:           return "types";
    case ID_groups:          return "groups";
    case ID_numInputs:       return "numInputs";
    case ID_allowedVarTypes: return "allowedVarTypes";
    case ID_winType:         return "winType";
    case ID_queryMode:       return "queryMode";
    case ID_numVars:         return "numVars";
    case ID_canBePublic:     return "canBePublic";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: QueryList::GetFieldType
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
QueryList::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_names:           return FieldType_stringVector;
    case ID_types:           return FieldType_intVector;
    case ID_groups:          return FieldType_intVector;
    case ID_numInputs:       return FieldType_intVector;
    case ID_allowedVarTypes: return FieldType_intVector;
    case ID_winType:         return FieldType_intVector;
    case ID_queryMode:       return FieldType_intVector;
    case ID_numVars:         return FieldType_intVector;
    case ID_canBePublic:     return FieldType_intVector;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: QueryList::GetFieldTypeName
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
QueryList::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_names:           return "stringVector";
    case ID_types:           return "intVector";
    case ID_groups:          return "intVector";
    case ID_numInputs:       return "intVector";
    case ID_allowedVarTypes: return "intVector";
    case ID_winType:         return "intVector";
    case ID_queryMode:       return "intVector";
    case ID_numVars:         return "intVector";
    case ID_canBePublic:     return "intVector";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: QueryList::FieldsEqual
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
QueryList::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const QueryList &obj = *((const QueryList*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_names:
        {  // new scope
        retval = (names == obj.names);
        }
        break;
    case ID_types:
        {  // new scope
        retval = (types == obj.types);
        }
        break;
    case ID_groups:
        {  // new scope
        retval = (groups == obj.groups);
        }
        break;
    case ID_numInputs:
        {  // new scope
        retval = (numInputs == obj.numInputs);
        }
        break;
    case ID_allowedVarTypes:
        {  // new scope
        retval = (allowedVarTypes == obj.allowedVarTypes);
        }
        break;
    case ID_winType:
        {  // new scope
        retval = (winType == obj.winType);
        }
        break;
    case ID_queryMode:
        {  // new scope
        retval = (queryMode == obj.queryMode);
        }
        break;
    case ID_numVars:
        {  // new scope
        retval = (numVars == obj.numVars);
        }
        break;
    case ID_canBePublic:
        {  // new scope
        retval = (canBePublic == obj.canBePublic);
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
//  Method:  AddQuery
//
//  Purpose:
//    Adds a query to the query list.
//
//  Programmer:  Brad Whitlock
//  Creation:    Mon Sep 9 15:45:37 PST 2002
//
//  Modifications:
//
//    Hank Childs, Thu Oct  2 13:40:16 PDT 2003
//    Add number of inputs.
//
//    Kathleen Bonnell,  Tue Nov 18 08:20:36 PST 2003 
//    Set default window type based on query type. 
//
//    Kathleen Bonnell,  Sat Sep  4 11:41:18 PDT 2004 
//    Made all parameters required, added Groups. 
//
//    Kathleen Bonnell, Fri Sep 28 14:43:50 PDT 2007 
//    Aded 'canBePublic' which defaults to '1'. 
//
// ****************************************************************************
 
void
QueryList::AddQuery(const std::string &name, QueryType t, Groups g, WindowType w, int num_input, int allowedVars, QueryMode qMode, int num_vars)
{
    names.push_back(name);
    types.push_back((int)t);
    groups.push_back((int)g);
    numInputs.push_back(num_input);
    allowedVarTypes.push_back(allowedVars);
    queryMode.push_back(qMode);
    winType.push_back((int)w);
    numVars.push_back(num_vars);
    canBePublic.push_back(1);
}

// ****************************************************************************
//  Method:  QueryExists
//
//  Purpose:
//    Checks if a query matches the passed parameters. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    July 11, 2003 
//
// ****************************************************************************
 
bool
QueryList::QueryExists(const std::string &name, QueryType t)
{
    bool match = false;
    for (size_t i = 0; i < names.size(); i++)
    {
        if (name == names[i]) 
        {
            if (t == types[i])
            {
                match = true;
            }
            break;
        }
    }
    return match;
}

// ****************************************************************************
//  Method:  NumberOfInputsForQuery
//
//  Purpose:
//      Returns the number of inputs for a query.
//
//  Programmer:  Hank Childs
//  Creation:    October 2, 2003
//
// ****************************************************************************
 
int
QueryList::NumberOfInputsForQuery(const std::string &name)
{
    for (size_t i = 0; i < names.size(); i++)
    {
        if (name == names[i]) 
        {
            return numInputs[i];
        }
    }
    return -1;
}

// ****************************************************************************
//  Method:  AllowedVarsForQuery
//
//  Purpose:
//      Returns the number of inputs for a query.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 18, 2003 
//
// ****************************************************************************
 
int
QueryList::AllowedVarsForQuery(const std::string &name)
{
    for (size_t i = 0; i < names.size(); i++)
    {
        if (name == names[i]) 
        {
            return allowedVarTypes[i];
        }
    }
    return -1;
}

// ****************************************************************************
//  Method:  TimeQueryAvailable
//
//  Purpose:
//    Checks if a query matches the passed parameters. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 23, 2004 
//
// ****************************************************************************
 
bool
QueryList::TimeQueryAvailable(const std::string &name)
{
    bool canDoTime = false;
    for (size_t i = 0; i < names.size(); i++)
    {
        if (name == names[i]) 
        {
            canDoTime = (queryMode[i] != QueryList::QueryOnly);
            break;
        }
    }
    return canDoTime;
}

// ****************************************************************************
//  Method:  GetWindowType
//
//  Purpose:
//    Returns the window type for the named query. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 15, 2005 
//
// ****************************************************************************
 
int
QueryList::GetWindowType(const std::string &name)
{
    int wt = -1; 
    for (size_t i = 0; i < names.size(); i++)
    {
        if (name == names[i]) 
        {
            wt = winType[i];
            break;
        }
    }
    return wt;
}

// ****************************************************************************
//  Method:  NumberOfVarsForQuery
//
//  Purpose:
//      Returns the number of variables required for a query.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 1, 2005 
//
// ****************************************************************************
 
int
QueryList::NumberOfVarsForQuery(const std::string &name)
{
    for (size_t i = 0; i < names.size(); i++)
    {
        if (name == names[i]) 
        {
            return numVars[i];
        }
    }
    return -1;
}

// ****************************************************************************
//  Method:  RegularQueryAvailable
//
//  Purpose:
//    Checks if a regular query (non-time) matches the passed parameters. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 9, 2005 
//
// ****************************************************************************
 
bool
QueryList::RegularQueryAvailable(const std::string &name)
{
    bool canDoRegular = false;
    for (size_t i = 0; i < names.size(); i++)
    {
        if (name == names[i]) 
        {
            canDoRegular = (queryMode[i] != QueryList::TimeOnly);
            break;
        }
    }
    return canDoRegular;
}

