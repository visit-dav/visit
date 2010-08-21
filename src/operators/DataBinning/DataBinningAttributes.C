/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

#include <DataBinningAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for DataBinningAttributes::ReductionOperator
//

static const char *ReductionOperator_strings[] = {
"Average", "Minimum", "Maximum", 
"StandardDeviation", "Variance", "Sum", 
"Count", "RMS", "PDF"
};

std::string
DataBinningAttributes::ReductionOperator_ToString(DataBinningAttributes::ReductionOperator t)
{
    int index = int(t);
    if(index < 0 || index >= 9) index = 0;
    return ReductionOperator_strings[index];
}

std::string
DataBinningAttributes::ReductionOperator_ToString(int t)
{
    int index = (t < 0 || t >= 9) ? 0 : t;
    return ReductionOperator_strings[index];
}

bool
DataBinningAttributes::ReductionOperator_FromString(const std::string &s, DataBinningAttributes::ReductionOperator &val)
{
    val = DataBinningAttributes::Average;
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
// Enum conversion methods for DataBinningAttributes::NumDimensions
//

static const char *NumDimensions_strings[] = {
"One", "Two", "Three"
};

std::string
DataBinningAttributes::NumDimensions_ToString(DataBinningAttributes::NumDimensions t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return NumDimensions_strings[index];
}

std::string
DataBinningAttributes::NumDimensions_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return NumDimensions_strings[index];
}

bool
DataBinningAttributes::NumDimensions_FromString(const std::string &s, DataBinningAttributes::NumDimensions &val)
{
    val = DataBinningAttributes::One;
    for(int i = 0; i < 3; ++i)
    {
        if(s == NumDimensions_strings[i])
        {
            val = (NumDimensions)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for DataBinningAttributes::OutOfBoundsBehavior
//

static const char *OutOfBoundsBehavior_strings[] = {
"Clamp", "Discard"};

std::string
DataBinningAttributes::OutOfBoundsBehavior_ToString(DataBinningAttributes::OutOfBoundsBehavior t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OutOfBoundsBehavior_strings[index];
}

std::string
DataBinningAttributes::OutOfBoundsBehavior_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OutOfBoundsBehavior_strings[index];
}

bool
DataBinningAttributes::OutOfBoundsBehavior_FromString(const std::string &s, DataBinningAttributes::OutOfBoundsBehavior &val)
{
    val = DataBinningAttributes::Clamp;
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

// ****************************************************************************
// Method: DataBinningAttributes::DataBinningAttributes
//
// Purpose: 
//   Init utility for the DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void DataBinningAttributes::Init()
{
    numDimensions = One;
    dim1SpecifyRange = false;
    dim1MinRange = 0;
    dim1MaxRange = 1;
    dim1NumBins = 50;
    dim2SpecifyRange = false;
    dim2MinRange = 0;
    dim2MaxRange = 1;
    dim2NumBins = 50;
    dim3SpecifyRange = false;
    dim3MinRange = 0;
    dim3MaxRange = 1;
    dim3NumBins = 50;
    outOfBoundsBehavior = Clamp;
    reductionOperator = Average;
    emptyVal = 0;

    DataBinningAttributes::SelectAll();
}

// ****************************************************************************
// Method: DataBinningAttributes::DataBinningAttributes
//
// Purpose: 
//   Copy utility for the DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void DataBinningAttributes::Copy(const DataBinningAttributes &obj)
{
    numDimensions = obj.numDimensions;
    dim1Var = obj.dim1Var;
    dim1SpecifyRange = obj.dim1SpecifyRange;
    dim1MinRange = obj.dim1MinRange;
    dim1MaxRange = obj.dim1MaxRange;
    dim1NumBins = obj.dim1NumBins;
    dim2Var = obj.dim2Var;
    dim2SpecifyRange = obj.dim2SpecifyRange;
    dim2MinRange = obj.dim2MinRange;
    dim2MaxRange = obj.dim2MaxRange;
    dim2NumBins = obj.dim2NumBins;
    dim3Var = obj.dim3Var;
    dim3SpecifyRange = obj.dim3SpecifyRange;
    dim3MinRange = obj.dim3MinRange;
    dim3MaxRange = obj.dim3MaxRange;
    dim3NumBins = obj.dim3NumBins;
    outOfBoundsBehavior = obj.outOfBoundsBehavior;
    reductionOperator = obj.reductionOperator;
    varForReduction = obj.varForReduction;
    emptyVal = obj.emptyVal;

    DataBinningAttributes::SelectAll();
}

// Type map format string
const char *DataBinningAttributes::TypeMapFormatString = DATABINNINGATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t DataBinningAttributes::TmfsStruct = {DATABINNINGATTRIBUTES_TMFS};


// ****************************************************************************
// Method: DataBinningAttributes::DataBinningAttributes
//
// Purpose: 
//   Default constructor for the DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

DataBinningAttributes::DataBinningAttributes() : 
    AttributeSubject(DataBinningAttributes::TypeMapFormatString),
    dim1Var("default"), dim2Var("default"), 
    dim3Var("default"), varForReduction("default")
{
    DataBinningAttributes::Init();
}

// ****************************************************************************
// Method: DataBinningAttributes::DataBinningAttributes
//
// Purpose: 
//   Constructor for the derived classes of DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

DataBinningAttributes::DataBinningAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs),
    dim1Var("default"), dim2Var("default"), 
    dim3Var("default"), varForReduction("default")
{
    DataBinningAttributes::Init();
}

// ****************************************************************************
// Method: DataBinningAttributes::DataBinningAttributes
//
// Purpose: 
//   Copy constructor for the DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

DataBinningAttributes::DataBinningAttributes(const DataBinningAttributes &obj) : 
    AttributeSubject(DataBinningAttributes::TypeMapFormatString)
{
    DataBinningAttributes::Copy(obj);
}

// ****************************************************************************
// Method: DataBinningAttributes::DataBinningAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

DataBinningAttributes::DataBinningAttributes(const DataBinningAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    DataBinningAttributes::Copy(obj);
}

// ****************************************************************************
// Method: DataBinningAttributes::~DataBinningAttributes
//
// Purpose: 
//   Destructor for the DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

DataBinningAttributes::~DataBinningAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: DataBinningAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the DataBinningAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

DataBinningAttributes& 
DataBinningAttributes::operator = (const DataBinningAttributes &obj)
{
    if (this == &obj) return *this;

    DataBinningAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: DataBinningAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the DataBinningAttributes class.
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
DataBinningAttributes::operator == (const DataBinningAttributes &obj) const
{
    // Create the return value
    return ((numDimensions == obj.numDimensions) &&
            (dim1Var == obj.dim1Var) &&
            (dim1SpecifyRange == obj.dim1SpecifyRange) &&
            (dim1MinRange == obj.dim1MinRange) &&
            (dim1MaxRange == obj.dim1MaxRange) &&
            (dim1NumBins == obj.dim1NumBins) &&
            (dim2Var == obj.dim2Var) &&
            (dim2SpecifyRange == obj.dim2SpecifyRange) &&
            (dim2MinRange == obj.dim2MinRange) &&
            (dim2MaxRange == obj.dim2MaxRange) &&
            (dim2NumBins == obj.dim2NumBins) &&
            (dim3Var == obj.dim3Var) &&
            (dim3SpecifyRange == obj.dim3SpecifyRange) &&
            (dim3MinRange == obj.dim3MinRange) &&
            (dim3MaxRange == obj.dim3MaxRange) &&
            (dim3NumBins == obj.dim3NumBins) &&
            (outOfBoundsBehavior == obj.outOfBoundsBehavior) &&
            (reductionOperator == obj.reductionOperator) &&
            (varForReduction == obj.varForReduction) &&
            (emptyVal == obj.emptyVal));
}

// ****************************************************************************
// Method: DataBinningAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the DataBinningAttributes class.
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
DataBinningAttributes::operator != (const DataBinningAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: DataBinningAttributes::TypeName
//
// Purpose: 
//   Type name method for the DataBinningAttributes class.
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
DataBinningAttributes::TypeName() const
{
    return "DataBinningAttributes";
}

// ****************************************************************************
// Method: DataBinningAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the DataBinningAttributes class.
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
DataBinningAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const DataBinningAttributes *tmp = (const DataBinningAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: DataBinningAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the DataBinningAttributes class.
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
DataBinningAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new DataBinningAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: DataBinningAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the DataBinningAttributes class.
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
DataBinningAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new DataBinningAttributes(*this);
    else
        retval = new DataBinningAttributes;

    return retval;
}

// ****************************************************************************
// Method: DataBinningAttributes::SelectAll
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
DataBinningAttributes::SelectAll()
{
    Select(ID_numDimensions,       (void *)&numDimensions);
    Select(ID_dim1Var,             (void *)&dim1Var);
    Select(ID_dim1SpecifyRange,    (void *)&dim1SpecifyRange);
    Select(ID_dim1MinRange,        (void *)&dim1MinRange);
    Select(ID_dim1MaxRange,        (void *)&dim1MaxRange);
    Select(ID_dim1NumBins,         (void *)&dim1NumBins);
    Select(ID_dim2Var,             (void *)&dim2Var);
    Select(ID_dim2SpecifyRange,    (void *)&dim2SpecifyRange);
    Select(ID_dim2MinRange,        (void *)&dim2MinRange);
    Select(ID_dim2MaxRange,        (void *)&dim2MaxRange);
    Select(ID_dim2NumBins,         (void *)&dim2NumBins);
    Select(ID_dim3Var,             (void *)&dim3Var);
    Select(ID_dim3SpecifyRange,    (void *)&dim3SpecifyRange);
    Select(ID_dim3MinRange,        (void *)&dim3MinRange);
    Select(ID_dim3MaxRange,        (void *)&dim3MaxRange);
    Select(ID_dim3NumBins,         (void *)&dim3NumBins);
    Select(ID_outOfBoundsBehavior, (void *)&outOfBoundsBehavior);
    Select(ID_reductionOperator,   (void *)&reductionOperator);
    Select(ID_varForReduction,     (void *)&varForReduction);
    Select(ID_emptyVal,            (void *)&emptyVal);
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
DataBinningAttributes::SetNumDimensions(DataBinningAttributes::NumDimensions numDimensions_)
{
    numDimensions = numDimensions_;
    Select(ID_numDimensions, (void *)&numDimensions);
}

void
DataBinningAttributes::SetDim1Var(const std::string &dim1Var_)
{
    dim1Var = dim1Var_;
    Select(ID_dim1Var, (void *)&dim1Var);
}

void
DataBinningAttributes::SetDim1SpecifyRange(bool dim1SpecifyRange_)
{
    dim1SpecifyRange = dim1SpecifyRange_;
    Select(ID_dim1SpecifyRange, (void *)&dim1SpecifyRange);
}

void
DataBinningAttributes::SetDim1MinRange(double dim1MinRange_)
{
    dim1MinRange = dim1MinRange_;
    Select(ID_dim1MinRange, (void *)&dim1MinRange);
}

void
DataBinningAttributes::SetDim1MaxRange(double dim1MaxRange_)
{
    dim1MaxRange = dim1MaxRange_;
    Select(ID_dim1MaxRange, (void *)&dim1MaxRange);
}

void
DataBinningAttributes::SetDim1NumBins(int dim1NumBins_)
{
    dim1NumBins = dim1NumBins_;
    Select(ID_dim1NumBins, (void *)&dim1NumBins);
}

void
DataBinningAttributes::SetDim2Var(const std::string &dim2Var_)
{
    dim2Var = dim2Var_;
    Select(ID_dim2Var, (void *)&dim2Var);
}

void
DataBinningAttributes::SetDim2SpecifyRange(bool dim2SpecifyRange_)
{
    dim2SpecifyRange = dim2SpecifyRange_;
    Select(ID_dim2SpecifyRange, (void *)&dim2SpecifyRange);
}

void
DataBinningAttributes::SetDim2MinRange(double dim2MinRange_)
{
    dim2MinRange = dim2MinRange_;
    Select(ID_dim2MinRange, (void *)&dim2MinRange);
}

void
DataBinningAttributes::SetDim2MaxRange(double dim2MaxRange_)
{
    dim2MaxRange = dim2MaxRange_;
    Select(ID_dim2MaxRange, (void *)&dim2MaxRange);
}

void
DataBinningAttributes::SetDim2NumBins(int dim2NumBins_)
{
    dim2NumBins = dim2NumBins_;
    Select(ID_dim2NumBins, (void *)&dim2NumBins);
}

void
DataBinningAttributes::SetDim3Var(const std::string &dim3Var_)
{
    dim3Var = dim3Var_;
    Select(ID_dim3Var, (void *)&dim3Var);
}

void
DataBinningAttributes::SetDim3SpecifyRange(bool dim3SpecifyRange_)
{
    dim3SpecifyRange = dim3SpecifyRange_;
    Select(ID_dim3SpecifyRange, (void *)&dim3SpecifyRange);
}

void
DataBinningAttributes::SetDim3MinRange(double dim3MinRange_)
{
    dim3MinRange = dim3MinRange_;
    Select(ID_dim3MinRange, (void *)&dim3MinRange);
}

void
DataBinningAttributes::SetDim3MaxRange(double dim3MaxRange_)
{
    dim3MaxRange = dim3MaxRange_;
    Select(ID_dim3MaxRange, (void *)&dim3MaxRange);
}

void
DataBinningAttributes::SetDim3NumBins(int dim3NumBins_)
{
    dim3NumBins = dim3NumBins_;
    Select(ID_dim3NumBins, (void *)&dim3NumBins);
}

void
DataBinningAttributes::SetOutOfBoundsBehavior(DataBinningAttributes::OutOfBoundsBehavior outOfBoundsBehavior_)
{
    outOfBoundsBehavior = outOfBoundsBehavior_;
    Select(ID_outOfBoundsBehavior, (void *)&outOfBoundsBehavior);
}

void
DataBinningAttributes::SetReductionOperator(DataBinningAttributes::ReductionOperator reductionOperator_)
{
    reductionOperator = reductionOperator_;
    Select(ID_reductionOperator, (void *)&reductionOperator);
}

void
DataBinningAttributes::SetVarForReduction(const std::string &varForReduction_)
{
    varForReduction = varForReduction_;
    Select(ID_varForReduction, (void *)&varForReduction);
}

void
DataBinningAttributes::SetEmptyVal(double emptyVal_)
{
    emptyVal = emptyVal_;
    Select(ID_emptyVal, (void *)&emptyVal);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

DataBinningAttributes::NumDimensions
DataBinningAttributes::GetNumDimensions() const
{
    return NumDimensions(numDimensions);
}

const std::string &
DataBinningAttributes::GetDim1Var() const
{
    return dim1Var;
}

std::string &
DataBinningAttributes::GetDim1Var()
{
    return dim1Var;
}

bool
DataBinningAttributes::GetDim1SpecifyRange() const
{
    return dim1SpecifyRange;
}

double
DataBinningAttributes::GetDim1MinRange() const
{
    return dim1MinRange;
}

double
DataBinningAttributes::GetDim1MaxRange() const
{
    return dim1MaxRange;
}

int
DataBinningAttributes::GetDim1NumBins() const
{
    return dim1NumBins;
}

const std::string &
DataBinningAttributes::GetDim2Var() const
{
    return dim2Var;
}

std::string &
DataBinningAttributes::GetDim2Var()
{
    return dim2Var;
}

bool
DataBinningAttributes::GetDim2SpecifyRange() const
{
    return dim2SpecifyRange;
}

double
DataBinningAttributes::GetDim2MinRange() const
{
    return dim2MinRange;
}

double
DataBinningAttributes::GetDim2MaxRange() const
{
    return dim2MaxRange;
}

int
DataBinningAttributes::GetDim2NumBins() const
{
    return dim2NumBins;
}

const std::string &
DataBinningAttributes::GetDim3Var() const
{
    return dim3Var;
}

std::string &
DataBinningAttributes::GetDim3Var()
{
    return dim3Var;
}

bool
DataBinningAttributes::GetDim3SpecifyRange() const
{
    return dim3SpecifyRange;
}

double
DataBinningAttributes::GetDim3MinRange() const
{
    return dim3MinRange;
}

double
DataBinningAttributes::GetDim3MaxRange() const
{
    return dim3MaxRange;
}

int
DataBinningAttributes::GetDim3NumBins() const
{
    return dim3NumBins;
}

DataBinningAttributes::OutOfBoundsBehavior
DataBinningAttributes::GetOutOfBoundsBehavior() const
{
    return OutOfBoundsBehavior(outOfBoundsBehavior);
}

DataBinningAttributes::ReductionOperator
DataBinningAttributes::GetReductionOperator() const
{
    return ReductionOperator(reductionOperator);
}

const std::string &
DataBinningAttributes::GetVarForReduction() const
{
    return varForReduction;
}

std::string &
DataBinningAttributes::GetVarForReduction()
{
    return varForReduction;
}

double
DataBinningAttributes::GetEmptyVal() const
{
    return emptyVal;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
DataBinningAttributes::SelectDim1Var()
{
    Select(ID_dim1Var, (void *)&dim1Var);
}

void
DataBinningAttributes::SelectDim2Var()
{
    Select(ID_dim2Var, (void *)&dim2Var);
}

void
DataBinningAttributes::SelectDim3Var()
{
    Select(ID_dim3Var, (void *)&dim3Var);
}

void
DataBinningAttributes::SelectVarForReduction()
{
    Select(ID_varForReduction, (void *)&varForReduction);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DataBinningAttributes::GetFieldName
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
DataBinningAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_numDimensions:       return "numDimensions";
    case ID_dim1Var:             return "dim1Var";
    case ID_dim1SpecifyRange:    return "dim1SpecifyRange";
    case ID_dim1MinRange:        return "dim1MinRange";
    case ID_dim1MaxRange:        return "dim1MaxRange";
    case ID_dim1NumBins:         return "dim1NumBins";
    case ID_dim2Var:             return "dim2Var";
    case ID_dim2SpecifyRange:    return "dim2SpecifyRange";
    case ID_dim2MinRange:        return "dim2MinRange";
    case ID_dim2MaxRange:        return "dim2MaxRange";
    case ID_dim2NumBins:         return "dim2NumBins";
    case ID_dim3Var:             return "dim3Var";
    case ID_dim3SpecifyRange:    return "dim3SpecifyRange";
    case ID_dim3MinRange:        return "dim3MinRange";
    case ID_dim3MaxRange:        return "dim3MaxRange";
    case ID_dim3NumBins:         return "dim3NumBins";
    case ID_outOfBoundsBehavior: return "outOfBoundsBehavior";
    case ID_reductionOperator:   return "reductionOperator";
    case ID_varForReduction:     return "varForReduction";
    case ID_emptyVal:            return "emptyVal";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: DataBinningAttributes::GetFieldType
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
DataBinningAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_numDimensions:       return FieldType_enum;
    case ID_dim1Var:             return FieldType_variablename;
    case ID_dim1SpecifyRange:    return FieldType_bool;
    case ID_dim1MinRange:        return FieldType_double;
    case ID_dim1MaxRange:        return FieldType_double;
    case ID_dim1NumBins:         return FieldType_int;
    case ID_dim2Var:             return FieldType_variablename;
    case ID_dim2SpecifyRange:    return FieldType_bool;
    case ID_dim2MinRange:        return FieldType_double;
    case ID_dim2MaxRange:        return FieldType_double;
    case ID_dim2NumBins:         return FieldType_int;
    case ID_dim3Var:             return FieldType_variablename;
    case ID_dim3SpecifyRange:    return FieldType_bool;
    case ID_dim3MinRange:        return FieldType_double;
    case ID_dim3MaxRange:        return FieldType_double;
    case ID_dim3NumBins:         return FieldType_int;
    case ID_outOfBoundsBehavior: return FieldType_enum;
    case ID_reductionOperator:   return FieldType_enum;
    case ID_varForReduction:     return FieldType_variablename;
    case ID_emptyVal:            return FieldType_double;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: DataBinningAttributes::GetFieldTypeName
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
DataBinningAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_numDimensions:       return "enum";
    case ID_dim1Var:             return "variablename";
    case ID_dim1SpecifyRange:    return "bool";
    case ID_dim1MinRange:        return "double";
    case ID_dim1MaxRange:        return "double";
    case ID_dim1NumBins:         return "int";
    case ID_dim2Var:             return "variablename";
    case ID_dim2SpecifyRange:    return "bool";
    case ID_dim2MinRange:        return "double";
    case ID_dim2MaxRange:        return "double";
    case ID_dim2NumBins:         return "int";
    case ID_dim3Var:             return "variablename";
    case ID_dim3SpecifyRange:    return "bool";
    case ID_dim3MinRange:        return "double";
    case ID_dim3MaxRange:        return "double";
    case ID_dim3NumBins:         return "int";
    case ID_outOfBoundsBehavior: return "enum";
    case ID_reductionOperator:   return "enum";
    case ID_varForReduction:     return "variablename";
    case ID_emptyVal:            return "double";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: DataBinningAttributes::FieldsEqual
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
DataBinningAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const DataBinningAttributes &obj = *((const DataBinningAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_numDimensions:
        {  // new scope
        retval = (numDimensions == obj.numDimensions);
        }
        break;
    case ID_dim1Var:
        {  // new scope
        retval = (dim1Var == obj.dim1Var);
        }
        break;
    case ID_dim1SpecifyRange:
        {  // new scope
        retval = (dim1SpecifyRange == obj.dim1SpecifyRange);
        }
        break;
    case ID_dim1MinRange:
        {  // new scope
        retval = (dim1MinRange == obj.dim1MinRange);
        }
        break;
    case ID_dim1MaxRange:
        {  // new scope
        retval = (dim1MaxRange == obj.dim1MaxRange);
        }
        break;
    case ID_dim1NumBins:
        {  // new scope
        retval = (dim1NumBins == obj.dim1NumBins);
        }
        break;
    case ID_dim2Var:
        {  // new scope
        retval = (dim2Var == obj.dim2Var);
        }
        break;
    case ID_dim2SpecifyRange:
        {  // new scope
        retval = (dim2SpecifyRange == obj.dim2SpecifyRange);
        }
        break;
    case ID_dim2MinRange:
        {  // new scope
        retval = (dim2MinRange == obj.dim2MinRange);
        }
        break;
    case ID_dim2MaxRange:
        {  // new scope
        retval = (dim2MaxRange == obj.dim2MaxRange);
        }
        break;
    case ID_dim2NumBins:
        {  // new scope
        retval = (dim2NumBins == obj.dim2NumBins);
        }
        break;
    case ID_dim3Var:
        {  // new scope
        retval = (dim3Var == obj.dim3Var);
        }
        break;
    case ID_dim3SpecifyRange:
        {  // new scope
        retval = (dim3SpecifyRange == obj.dim3SpecifyRange);
        }
        break;
    case ID_dim3MinRange:
        {  // new scope
        retval = (dim3MinRange == obj.dim3MinRange);
        }
        break;
    case ID_dim3MaxRange:
        {  // new scope
        retval = (dim3MaxRange == obj.dim3MaxRange);
        }
        break;
    case ID_dim3NumBins:
        {  // new scope
        retval = (dim3NumBins == obj.dim3NumBins);
        }
        break;
    case ID_outOfBoundsBehavior:
        {  // new scope
        retval = (outOfBoundsBehavior == obj.outOfBoundsBehavior);
        }
        break;
    case ID_reductionOperator:
        {  // new scope
        retval = (reductionOperator == obj.reductionOperator);
        }
        break;
    case ID_varForReduction:
        {  // new scope
        retval = (varForReduction == obj.varForReduction);
        }
        break;
    case ID_emptyVal:
        {  // new scope
        retval = (emptyVal == obj.emptyVal);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

// **************************************************************
//  Method: DataBinningAttributes::CreateConstructionAtts
//
//  Purpose:
//      Creates attributes of type ConstructDataBinningAtts.
//
//  Programmer: Hank Childs
//  Creation:   August 19, 2010
//
// **************************************************************

ConstructDataBinningAttributes
DataBinningAttributes::CreateConstructionAtts(void)
{
    ConstructDataBinningAttributes dba;
    static int times = 0;
    char name[1024];
    sprintf(name, "_db_op_%d", times++);
    dba.SetName(name);
    stringVector varnames;
    doubleVector range;
    intVector    numBins;
    varnames.push_back(dim1Var);
    range.push_back(dim1MinRange);
    range.push_back(dim1MaxRange);
    numBins.push_back(dim1NumBins);
    if (numDimensions == Two || numDimensions == Three)
    {
        varnames.push_back(dim2Var);
        range.push_back(dim2MinRange);
        range.push_back(dim2MaxRange);
        numBins.push_back(dim2NumBins);
    }
    if (numDimensions == Three)
    {
        varnames.push_back(dim3Var);
        range.push_back(dim3MinRange);
        range.push_back(dim3MaxRange);
        numBins.push_back(dim3NumBins);
    }
    dba.SetVarnames(varnames);
    dba.SetBinBoundaries(range);
    dba.SetNumBins(numBins);
    dba.SetReductionOperator((ConstructDataBinningAttributes::ReductionOperator) reductionOperator);
    dba.SetVarForReductionOperator(varForReduction);
    dba.SetUndefinedValue(emptyVal);
    dba.SetOverTime(false);
    dba.SetOutOfBoundsBehavior((ConstructDataBinningAttributes::OutOfBoundsBehavior) outOfBoundsBehavior);
    
    return dba;
}

