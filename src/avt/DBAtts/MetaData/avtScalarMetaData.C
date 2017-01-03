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

#include <avtScalarMetaData.h>
#include <DataNode.h>
#include <ImproperUseException.h>

//
// Enum conversion methods for avtScalarMetaData::PartialCellModes
//

static const char *PartialCellModes_strings[] = {
"Include", "Exclude", "Dissect"
};

std::string
avtScalarMetaData::PartialCellModes_ToString(avtScalarMetaData::PartialCellModes t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return PartialCellModes_strings[index];
}

std::string
avtScalarMetaData::PartialCellModes_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return PartialCellModes_strings[index];
}

bool
avtScalarMetaData::PartialCellModes_FromString(const std::string &s, avtScalarMetaData::PartialCellModes &val)
{
    val = avtScalarMetaData::Include;
    for(int i = 0; i < 3; ++i)
    {
        if(s == PartialCellModes_strings[i])
        {
            val = (PartialCellModes)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for avtScalarMetaData::EnumTypes
//

static const char *EnumTypes_strings[] = {
"None", "ByValue", "ByRange", 
"ByBitMask", "ByNChooseR"};

std::string
avtScalarMetaData::EnumTypes_ToString(avtScalarMetaData::EnumTypes t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return EnumTypes_strings[index];
}

std::string
avtScalarMetaData::EnumTypes_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return EnumTypes_strings[index];
}

bool
avtScalarMetaData::EnumTypes_FromString(const std::string &s, avtScalarMetaData::EnumTypes &val)
{
    val = avtScalarMetaData::None;
    for(int i = 0; i < 5; ++i)
    {
        if(s == EnumTypes_strings[i])
        {
            val = (EnumTypes)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for avtScalarMetaData::MissingData
//

static const char *MissingData_strings[] = {
"MissingData_None", "MissingData_Value", "MissingData_Valid_Min", 
"MissingData_Valid_Max", "MissingData_Valid_Range"};

std::string
avtScalarMetaData::MissingData_ToString(avtScalarMetaData::MissingData t)
{
    int index = int(t);
    if(index < 0 || index >= 5) index = 0;
    return MissingData_strings[index];
}

std::string
avtScalarMetaData::MissingData_ToString(int t)
{
    int index = (t < 0 || t >= 5) ? 0 : t;
    return MissingData_strings[index];
}

bool
avtScalarMetaData::MissingData_FromString(const std::string &s, avtScalarMetaData::MissingData &val)
{
    val = avtScalarMetaData::MissingData_None;
    for(int i = 0; i < 5; ++i)
    {
        if(s == MissingData_strings[i])
        {
            val = (MissingData)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: avtScalarMetaData::avtScalarMetaData
//
// Purpose: 
//   Init utility for the avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void avtScalarMetaData::Init()
{
    treatAsASCII = false;
    enumerationType = None;
    enumAlwaysExclude[0] = +DBL_MAX;
    enumAlwaysExclude[1] = -DBL_MAX;
    enumAlwaysInclude[0] = +DBL_MAX;
    enumAlwaysInclude[1] = -DBL_MAX;
    enumPartialCellMode = Exclude;
    enumNChooseRN = 0;
    enumNChooseRMaxR = 0;
    missingDataType = MissingData_None;
    missingData[0] = 0;
    missingData[1] = 0;

    avtScalarMetaData::SelectAll();
}

// ****************************************************************************
// Method: avtScalarMetaData::avtScalarMetaData
//
// Purpose: 
//   Copy utility for the avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void avtScalarMetaData::Copy(const avtScalarMetaData &obj)
{
    treatAsASCII = obj.treatAsASCII;
    enumerationType = obj.enumerationType;
    enumNames = obj.enumNames;
    enumRanges = obj.enumRanges;
    enumAlwaysExclude[0] = obj.enumAlwaysExclude[0];
    enumAlwaysExclude[1] = obj.enumAlwaysExclude[1];

    enumAlwaysInclude[0] = obj.enumAlwaysInclude[0];
    enumAlwaysInclude[1] = obj.enumAlwaysInclude[1];

    enumPartialCellMode = obj.enumPartialCellMode;
    enumGraphEdges = obj.enumGraphEdges;
    enumNChooseRN = obj.enumNChooseRN;
    enumNChooseRMaxR = obj.enumNChooseRMaxR;
    missingDataType = obj.missingDataType;
    missingData[0] = obj.missingData[0];
    missingData[1] = obj.missingData[1];


    avtScalarMetaData::SelectAll();
}

// Type map format string
const char *avtScalarMetaData::TypeMapFormatString = AVTSCALARMETADATA_TMFS;
const AttributeGroup::private_tmfs_t avtScalarMetaData::TmfsStruct = {AVTSCALARMETADATA_TMFS};


// ****************************************************************************
// Method: avtScalarMetaData::avtScalarMetaData
//
// Purpose: 
//   Default constructor for the avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData() : 
    avtVarMetaData(avtScalarMetaData::TmfsStruct)
{
    avtScalarMetaData::Init();
}

// ****************************************************************************
// Method: avtScalarMetaData::avtScalarMetaData
//
// Purpose: 
//   Constructor for the derived classes of avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(private_tmfs_t tmfs) : 
    avtVarMetaData(tmfs)
{
    avtScalarMetaData::Init();
}

// ****************************************************************************
// Method: avtScalarMetaData::avtScalarMetaData
//
// Purpose: 
//   Copy constructor for the avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(const avtScalarMetaData &obj) : 
    avtVarMetaData(obj,avtScalarMetaData::TmfsStruct)
{
    avtScalarMetaData::Copy(obj);
}

// ****************************************************************************
// Method: avtScalarMetaData::avtScalarMetaData
//
// Purpose: 
//   Copy constructor for derived classes of the avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(const avtScalarMetaData &obj, private_tmfs_t tmfs) : 
    avtVarMetaData(obj,tmfs)
{
    avtScalarMetaData::Copy(obj);
}

// ****************************************************************************
// Method: avtScalarMetaData::~avtScalarMetaData
//
// Purpose: 
//   Destructor for the avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

avtScalarMetaData::~avtScalarMetaData()
{
    // nothing here
}

// ****************************************************************************
// Method: avtScalarMetaData::operator = 
//
// Purpose: 
//   Assignment operator for the avtScalarMetaData class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

avtScalarMetaData& 
avtScalarMetaData::operator = (const avtScalarMetaData &obj)
{
    if (this == &obj) return *this;

    // call the base class' assignment operator first
    avtVarMetaData::operator=(obj);

    avtScalarMetaData::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: avtScalarMetaData::operator == 
//
// Purpose: 
//   Comparison operator == for the avtScalarMetaData class.
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
avtScalarMetaData::operator == (const avtScalarMetaData &obj) const
{
    // Compare the enumAlwaysExclude arrays.
    bool enumAlwaysExclude_equal = true;
    for(int i = 0; i < 2 && enumAlwaysExclude_equal; ++i)
        enumAlwaysExclude_equal = (enumAlwaysExclude[i] == obj.enumAlwaysExclude[i]);

    // Compare the enumAlwaysInclude arrays.
    bool enumAlwaysInclude_equal = true;
    for(int i = 0; i < 2 && enumAlwaysInclude_equal; ++i)
        enumAlwaysInclude_equal = (enumAlwaysInclude[i] == obj.enumAlwaysInclude[i]);

    // Compare the missingData arrays.
    bool missingData_equal = true;
    for(int i = 0; i < 2 && missingData_equal; ++i)
        missingData_equal = (missingData[i] == obj.missingData[i]);

    // Create the return value
    return ((treatAsASCII == obj.treatAsASCII) &&
            (enumerationType == obj.enumerationType) &&
            (enumNames == obj.enumNames) &&
            (enumRanges == obj.enumRanges) &&
            enumAlwaysExclude_equal &&
            enumAlwaysInclude_equal &&
            (enumPartialCellMode == obj.enumPartialCellMode) &&
            (enumGraphEdges == obj.enumGraphEdges) &&
            (enumNChooseRN == obj.enumNChooseRN) &&
            (enumNChooseRMaxR == obj.enumNChooseRMaxR) &&
            (missingDataType == obj.missingDataType) &&
            missingData_equal &&
            avtVarMetaData::operator==(obj));
}

// ****************************************************************************
// Method: avtScalarMetaData::operator != 
//
// Purpose: 
//   Comparison operator != for the avtScalarMetaData class.
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
avtScalarMetaData::operator != (const avtScalarMetaData &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: avtScalarMetaData::TypeName
//
// Purpose: 
//   Type name method for the avtScalarMetaData class.
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
avtScalarMetaData::TypeName() const
{
    return "avtScalarMetaData";
}

// ****************************************************************************
// Method: avtScalarMetaData::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the avtScalarMetaData class.
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
avtScalarMetaData::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const avtScalarMetaData *tmp = (const avtScalarMetaData *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: avtScalarMetaData::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the avtScalarMetaData class.
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
avtScalarMetaData::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new avtScalarMetaData(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: avtScalarMetaData::NewInstance
//
// Purpose: 
//   NewInstance method for the avtScalarMetaData class.
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
avtScalarMetaData::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new avtScalarMetaData(*this);
    else
        retval = new avtScalarMetaData;

    return retval;
}

// ****************************************************************************
// Method: avtScalarMetaData::SelectAll
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
avtScalarMetaData::SelectAll()
{
    // call the base class' SelectAll() first
    avtVarMetaData::SelectAll();
    Select(ID_treatAsASCII,        (void *)&treatAsASCII);
    Select(ID_enumerationType,     (void *)&enumerationType);
    Select(ID_enumNames,           (void *)&enumNames);
    Select(ID_enumRanges,          (void *)&enumRanges);
    Select(ID_enumAlwaysExclude,   (void *)enumAlwaysExclude, 2);
    Select(ID_enumAlwaysInclude,   (void *)enumAlwaysInclude, 2);
    Select(ID_enumPartialCellMode, (void *)&enumPartialCellMode);
    Select(ID_enumGraphEdges,      (void *)&enumGraphEdges);
    Select(ID_enumNChooseRN,       (void *)&enumNChooseRN);
    Select(ID_enumNChooseRMaxR,    (void *)&enumNChooseRMaxR);
    Select(ID_missingDataType,     (void *)&missingDataType);
    Select(ID_missingData,         (void *)missingData, 2);
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
avtScalarMetaData::SetEnumerationType(avtScalarMetaData::EnumTypes enumerationType_)
{
    enumerationType = enumerationType_;
    Select(ID_enumerationType, (void *)&enumerationType);
}

void
avtScalarMetaData::SetEnumPartialCellMode(avtScalarMetaData::PartialCellModes enumPartialCellMode_)
{
    enumPartialCellMode = enumPartialCellMode_;
    Select(ID_enumPartialCellMode, (void *)&enumPartialCellMode);
}

void
avtScalarMetaData::SetEnumNChooseRN(int enumNChooseRN_)
{
    enumNChooseRN = enumNChooseRN_;
    Select(ID_enumNChooseRN, (void *)&enumNChooseRN);
}

void
avtScalarMetaData::SetEnumNChooseRMaxR(int enumNChooseRMaxR_)
{
    enumNChooseRMaxR = enumNChooseRMaxR_;
    Select(ID_enumNChooseRMaxR, (void *)&enumNChooseRMaxR);
}

void
avtScalarMetaData::SetMissingDataType(avtScalarMetaData::MissingData missingDataType_)
{
    missingDataType = missingDataType_;
    Select(ID_missingDataType, (void *)&missingDataType);
}

void
avtScalarMetaData::SetMissingData(const double *missingData_)
{
    missingData[0] = missingData_[0];
    missingData[1] = missingData_[1];
    Select(ID_missingData, (void *)missingData, 2);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

avtScalarMetaData::EnumTypes
avtScalarMetaData::GetEnumerationType() const
{
    return EnumTypes(enumerationType);
}

avtScalarMetaData::PartialCellModes
avtScalarMetaData::GetEnumPartialCellMode() const
{
    return PartialCellModes(enumPartialCellMode);
}

int
avtScalarMetaData::GetEnumNChooseRN() const
{
    return enumNChooseRN;
}

int
avtScalarMetaData::GetEnumNChooseRMaxR() const
{
    return enumNChooseRMaxR;
}

avtScalarMetaData::MissingData
avtScalarMetaData::GetMissingDataType() const
{
    return MissingData(missingDataType);
}

const double *
avtScalarMetaData::GetMissingData() const
{
    return missingData;
}

double *
avtScalarMetaData::GetMissingData()
{
    return missingData;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
avtScalarMetaData::SelectMissingData()
{
    Select(ID_missingData, (void *)missingData, 2);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: avtScalarMetaData::GetFieldName
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
avtScalarMetaData::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_treatAsASCII:        return "treatAsASCII";
    case ID_enumerationType:     return "enumerationType";
    case ID_enumNames:           return "enumNames";
    case ID_enumRanges:          return "enumRanges";
    case ID_enumAlwaysExclude:   return "enumAlwaysExclude";
    case ID_enumAlwaysInclude:   return "enumAlwaysInclude";
    case ID_enumPartialCellMode: return "enumPartialCellMode";
    case ID_enumGraphEdges:      return "enumGraphEdges";
    case ID_enumNChooseRN:       return "enumNChooseRN";
    case ID_enumNChooseRMaxR:    return "enumNChooseRMaxR";
    case ID_missingDataType:     return "missingDataType";
    case ID_missingData:         return "missingData";
    default:  return avtVarMetaData::GetFieldName(index);
    }
}

// ****************************************************************************
// Method: avtScalarMetaData::GetFieldType
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
avtScalarMetaData::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_treatAsASCII:        return FieldType_bool;
    case ID_enumerationType:     return FieldType_enum;
    case ID_enumNames:           return FieldType_stringVector;
    case ID_enumRanges:          return FieldType_doubleVector;
    case ID_enumAlwaysExclude:   return FieldType_doubleArray;
    case ID_enumAlwaysInclude:   return FieldType_doubleArray;
    case ID_enumPartialCellMode: return FieldType_enum;
    case ID_enumGraphEdges:      return FieldType_intVector;
    case ID_enumNChooseRN:       return FieldType_int;
    case ID_enumNChooseRMaxR:    return FieldType_int;
    case ID_missingDataType:     return FieldType_enum;
    case ID_missingData:         return FieldType_doubleArray;
    default:  return avtVarMetaData::GetFieldType(index);
    }
}

// ****************************************************************************
// Method: avtScalarMetaData::GetFieldTypeName
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
avtScalarMetaData::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_treatAsASCII:        return "bool";
    case ID_enumerationType:     return "enum";
    case ID_enumNames:           return "stringVector";
    case ID_enumRanges:          return "doubleVector";
    case ID_enumAlwaysExclude:   return "doubleArray";
    case ID_enumAlwaysInclude:   return "doubleArray";
    case ID_enumPartialCellMode: return "enum";
    case ID_enumGraphEdges:      return "intVector";
    case ID_enumNChooseRN:       return "int";
    case ID_enumNChooseRMaxR:    return "int";
    case ID_missingDataType:     return "enum";
    case ID_missingData:         return "doubleArray";
    default:  return avtVarMetaData::GetFieldTypeName(index);
    }
}

// ****************************************************************************
// Method: avtScalarMetaData::FieldsEqual
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
avtScalarMetaData::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const avtScalarMetaData &obj = *((const avtScalarMetaData*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_treatAsASCII:
        {  // new scope
        retval = (treatAsASCII == obj.treatAsASCII);
        }
        break;
    case ID_enumerationType:
        {  // new scope
        retval = (enumerationType == obj.enumerationType);
        }
        break;
    case ID_enumNames:
        {  // new scope
        retval = (enumNames == obj.enumNames);
        }
        break;
    case ID_enumRanges:
        {  // new scope
        retval = (enumRanges == obj.enumRanges);
        }
        break;
    case ID_enumAlwaysExclude:
        {  // new scope
        // Compare the enumAlwaysExclude arrays.
        bool enumAlwaysExclude_equal = true;
        for(int i = 0; i < 2 && enumAlwaysExclude_equal; ++i)
            enumAlwaysExclude_equal = (enumAlwaysExclude[i] == obj.enumAlwaysExclude[i]);

        retval = enumAlwaysExclude_equal;
        }
        break;
    case ID_enumAlwaysInclude:
        {  // new scope
        // Compare the enumAlwaysInclude arrays.
        bool enumAlwaysInclude_equal = true;
        for(int i = 0; i < 2 && enumAlwaysInclude_equal; ++i)
            enumAlwaysInclude_equal = (enumAlwaysInclude[i] == obj.enumAlwaysInclude[i]);

        retval = enumAlwaysInclude_equal;
        }
        break;
    case ID_enumPartialCellMode:
        {  // new scope
        retval = (enumPartialCellMode == obj.enumPartialCellMode);
        }
        break;
    case ID_enumGraphEdges:
        {  // new scope
        retval = (enumGraphEdges == obj.enumGraphEdges);
        }
        break;
    case ID_enumNChooseRN:
        {  // new scope
        retval = (enumNChooseRN == obj.enumNChooseRN);
        }
        break;
    case ID_enumNChooseRMaxR:
        {  // new scope
        retval = (enumNChooseRMaxR == obj.enumNChooseRMaxR);
        }
        break;
    case ID_missingDataType:
        {  // new scope
        retval = (missingDataType == obj.missingDataType);
        }
        break;
    case ID_missingData:
        {  // new scope
        // Compare the missingData arrays.
        bool missingData_equal = true;
        for(int i = 0; i < 2 && missingData_equal; ++i)
            missingData_equal = (missingData[i] == obj.missingData[i]);

        retval = missingData_equal;
        }
        break;
    default: retval = avtVarMetaData::FieldsEqual(index_, rhs);
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
//  Method: avtScalarMetaData constructor
//
//  Arguments:
//      n           The name of the scalar variable.
//      mn          The name of the mesh the scalar var is defined on.
//      c           The centering of the variable.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
//    Brad Whitlock, Tue Jul 20 13:44:42 PST 2004
//    Added units.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Initialized treatAsASCII.
//
//    Hank Childs, Mon Feb 14 14:16:49 PST 2005
//    Added original name.
//
//    Jeremy Meredith, Fri Aug 25 17:16:38 EDT 2006
//    Added enumerated scalars.
//
//    Mark C. Miller, Wed Mar 26 10:11:46 PDT 2008
//    Added initialization for other enumerated scalar options
//    and hideFromGUI.
//
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(std::string n, std::string mn, 
                                     avtCentering c)
    : avtVarMetaData(avtScalarMetaData::TmfsStruct, n, mn, c)
{
    avtScalarMetaData::Init();
}

// ****************************************************************************
//  Method: avtScalarMetaData constructor
//
//  Arguments:
//      n           The name of the scalar variable.
//      mn          The name of the mesh the scalar var is defined on.
//      c           The centering of the variable.
//      min         The minimum value of the scalar variable.
//      max         The maximum value of the scalar variable.
//
//  Programmer: Hank Childs
//  Creation:   August 25, 2000
//
//  Modifications:
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Initialized validVariable.
//
//    Brad Whitlock, Tue Jul 20 13:45:12 PST 2004
//    Added units.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Initialized treatAsASCII.
//
//    Hank Childs, Mon Feb 14 14:16:49 PST 2005
//    Added original name.
//
//    Jeremy Meredith, Fri Aug 25 17:16:38 EDT 2006
//    Added enumerated scalars.
//
//    Mark C. Miller, Wed Mar 26 10:11:46 PDT 2008
//    Added initialization for other enumerated scalar options
//    and hideFromGUI.
//
// ****************************************************************************

avtScalarMetaData::avtScalarMetaData(std::string n, std::string mn,
                                     avtCentering c, double min, double max)
    : avtVarMetaData(avtScalarMetaData::TmfsStruct, n, mn, c, min, max)
{
    avtScalarMetaData::Init();
}

// ****************************************************************************
//  Method: avtScalarMetaData::Print
//
//  Purpose:
//      Print statement for debugging.
//
//  Arguments:
//      out      The stream to output to.
//      indent   The number of tabs to indent each line with.
//
//  Programmer:  Hank Childs
//  Creation:    August 28, 2000
//
//  Modifications:
//    Hank Childs, Mon Dec  9 17:04:39 PST 2002
//    Print out validVariable.
//
//    Kathleen Bonnell, Thu Jul 22 12:10:19 PDT 2004 
//    Print out treatAsASCII.
//
//    Brad Whitlock, Tue Jul 20 13:47:04 PST 2004
//    Added units.
//
//    Hank Childs, Mon Feb 14 14:16:49 PST 2005
//    Added original name.
//
//    Jeremy Meredith, Fri Aug 25 17:16:38 EDT 2006
//    Added enumerated scalars.
//
//    Brad Whitlock, Wed Jan  4 14:58:08 PST 2012
//    Added missing data values.
//
// ****************************************************************************
inline void
Indent(ostream &out, int indent)
{
    for (int i = 0 ; i < indent ; i++)
    {
        out << "\t";
    }
}

void
avtScalarMetaData::Print(ostream &out, int indent) const
{
    avtVarMetaData::Print(out, indent);

    if (treatAsASCII)
    {
        Indent(out, indent);
        out << "This variable should be treated as ASCII." << endl;
    }

    if (enumerationType != None)
    {
        Indent(out, indent);
        out << "This variable contains enumerated values." << endl;
    }
    else
    {
        Indent(out, indent);
        out << "This variable does not contain enumerated values." << endl;
    }

    switch(missingDataType)
    {
    case MissingData_Value:
        Indent(out, indent);
        out << "Missing value: " << missingData[0] << endl;
        break;
    case MissingData_Valid_Min:
        Indent(out, indent);
        out << "Missing value less than: " << missingData[0] << endl;
        break;
    case MissingData_Valid_Max:
        Indent(out, indent);
        out << "Missing value greater than: " << missingData[0] << endl;
        break;
    case MissingData_Valid_Range:
        Indent(out, indent);
        out << "Missing value outside range: ["
            << missingData[0] << ", " << missingData[1] << "]" << endl;
        break;
    default:
        break;
    }
}

void
avtScalarMetaData::AddEnumGraphEdge(int head, int tail)
{
    enumGraphEdges.push_back(head);
    enumGraphEdges.push_back(tail);
}

// ****************************************************************************
//  Method: AddEnumNameValue
//
//  Purpose: Add name/value pair to an enumeration
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************

int
avtScalarMetaData::AddEnumNameValue(std::string name, double val)
{
    enumNames.push_back(name);
    enumRanges.push_back(val);
    enumRanges.push_back(val);

    return (int)enumNames.size() - 1;
}

// ****************************************************************************
//  Method: AddEnumNameRange
//
//  Purpose: Add name/range pair to an enumeration
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************

int
avtScalarMetaData::AddEnumNameRange(std::string name, double min, double max)
{
    if (min > max)
    {
        EXCEPTION1(ImproperUseException, name);
    }

    enumNames.push_back(name);
    enumRanges.push_back(min);
    enumRanges.push_back(max);

    return (int)enumNames.size() - 1;
}

// ****************************************************************************
//  Method: SetEnumAlwaysExcludeValue
//
//  Purpose: sets value to be used for always excluded in the selection
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************

void
avtScalarMetaData::SetEnumAlwaysExcludeValue(double val)
{
    enumAlwaysExclude[0] = val;
    enumAlwaysExclude[1] = val;
}

// ****************************************************************************
//  Method: SetEnumAlwaysExcludeRange
//
//  Purpose: sets range to be used for always excluded in the selection
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************

void
avtScalarMetaData::SetEnumAlwaysExcludeRange(double min, double max)
{
    enumAlwaysExclude[0] = min;
    enumAlwaysExclude[1] = max;
}

// ****************************************************************************
//  Method: SetEnumAlwaysIncludeValue
//
//  Purpose: sets value to be used for always included in the selection
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************

void
avtScalarMetaData::SetEnumAlwaysIncludeValue(double val)
{
    enumAlwaysInclude[0] = val;
    enumAlwaysInclude[1] = val;
}

// ****************************************************************************
//  Method: SetEnumAlwaysIncludeRange
//
//  Purpose: sets range to be used for always included in the selection
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************

void
avtScalarMetaData::SetEnumAlwaysIncludeRange(double min, double max)
{
    enumAlwaysInclude[0] = min;
    enumAlwaysInclude[1] = max;
}

// ****************************************************************************
//  Method: BuildEnumNChooseRMap
//
//  Purpose: convenience method for caller to build a pascal's triangle map
//  for given N/MaxR
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************
void
avtScalarMetaData::BuildEnumNChooseRMap(int n, int maxr, std::vector<std::vector<int> > &ptMap)
{
    for (size_t row = 0; row < ptMap.size(); row++)
        ptMap[row].clear();
    ptMap.clear();

    for (int row = 0; row < n; row++)
    {
        std::vector<int> tmpRow;
        for (int col = 0; col <= maxr; col++)
            tmpRow.push_back(0);
        ptMap.push_back(tmpRow);
    }

    for (int row = 0; row < n; row++)
        for (int col = 0; col <= maxr; col++)
            ptMap[row][col] = int(NChooseR(n-row-1,col));
}

// ****************************************************************************
//  Function: Bi-nomial coefficient (n-choose-r)
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:24:38 EDT 2008
//    Made it member function so that we got the static namespace benefits
//    without the warnings that static functions in headers tend to generate.
//
// ****************************************************************************
double avtScalarMetaData::NChooseR(int n, int r)
{
    if (n == 0)
    {
        if (r == 0)
            return 1;
        else
            return 0;
    }
    double result = 1;
    int k = 1;
    for (int i = r+1; i <=n; i++)
    {
        result *= (double) i;
        if (k <= n-r)
        {
            result /= (double) k;
            k++;
        }
    }
    return result;
}

// ****************************************************************************
//  Function: ComboValFromDigits
//
//  Purpose: Internal function to compute NChooseR value from list of 
//  ids (digits representing enum set membership)
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:24:38 EDT 2008
//    Made it member function so that we got the static namespace benefits
//    without the warnings that static functions in headers tend to generate.
//
// ****************************************************************************

void
avtScalarMetaData::ComboValFromDigits(const std::vector<std::vector<int> > &ptMap,
    const std::list<int> &digits, double *id)
{
    int row, col;
    *id = 0.0;
    std::list<int>::const_reverse_iterator it;
    for (col = (int)digits.size()-1, it = digits.rbegin(); col >= 0; col--, it++)
    {
        for (row = 0; row <= *it; row++)
            *id += ptMap[row][col];
    }
    *id -= 1;
}

// ****************************************************************************
//  Function: ComboDigitsFromVal
//
//  Purpose: Internal function to compute list of ids (digits representing enum
//  set membership) to single numerical value.
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Aug  7 14:24:38 EDT 2008
//    Made it member function so that we got the static namespace benefits
//    without the warnings that static functions in headers tend to generate.
//
// ****************************************************************************

void
avtScalarMetaData::ComboDigitsFromVal(double id, int n, int maxr,
    const std::vector<std::vector<int> > &ptMap, std::list<int> &digits)
{
    int row, col, grp;

    for (grp = 1; grp <= maxr; grp++)
    {
        double grpc = NChooseR(n,grp);
        if (id < grpc)
            break;
        else
            id -= grpc;
    }
    if (grp == maxr+1)
        return;

    int rowmin = 0;
    for (col = grp-1; col >= 0; col--)
    {
        for (row = rowmin; row < n; row++)
        {
            int seglen = ptMap[row][col];
            if (id < seglen)
            {
                rowmin = row+1;
                digits.push_front(row);
                break;
            }
            id -= seglen;
        }
    }
}

// ****************************************************************************
//  Method: UpdateValByInsertingDigit
//
//  Purpose: convenience method for callers to, given a current numerical value
//  reprsenting a list of set ids, compute a new numerical value by adding the
//  new set id.
//
//  Programmer: Mark C. Miller, Thu Mar 27 14:16:45 PDT 2008
// ****************************************************************************

void avtScalarMetaData::UpdateValByInsertingDigit(double *id, int n, int maxr,
const std::vector<std::vector<int> > &ptMap, int newDigit)
{
    std::list<int> currentDigits;
    ComboDigitsFromVal(*id, n, maxr, ptMap, currentDigits);

    std::list<int>::iterator it = currentDigits.begin();
    while (it != currentDigits.end() && *it < newDigit)
        it++;

    if (it == currentDigits.end())
    {
        currentDigits.push_front(newDigit);
        ComboValFromDigits(ptMap, currentDigits, id);
    }
    else
    {
        // only update if we actually adding a new digit
        if (newDigit < *it)
        {
            it++;
            currentDigits.insert(it, newDigit);
            ComboValFromDigits(ptMap, currentDigits, id);
        }
    }
}

