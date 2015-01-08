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

#include <ExtremeValueAnalysisAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for ExtremeValueAnalysisAttributes::AggregationType
//

static const char *AggregationType_strings[] = {
"ANNUAL", "SEASONAL", "MONTHLY"
};

std::string
ExtremeValueAnalysisAttributes::AggregationType_ToString(ExtremeValueAnalysisAttributes::AggregationType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return AggregationType_strings[index];
}

std::string
ExtremeValueAnalysisAttributes::AggregationType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return AggregationType_strings[index];
}

bool
ExtremeValueAnalysisAttributes::AggregationType_FromString(const std::string &s, ExtremeValueAnalysisAttributes::AggregationType &val)
{
    val = ExtremeValueAnalysisAttributes::ANNUAL;
    for(int i = 0; i < 3; ++i)
    {
        if(s == AggregationType_strings[i])
        {
            val = (AggregationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ExtremeValueAnalysisAttributes::MonthType
//

static const char *MonthType_strings[] = {
"JANUARY", "FEBRUARY", "MARCH", 
"APRIL", "MAY", "JUNE", 
"JULY", "AUGUST", "SEPTEMBER", 
"OCTOBER", "NOVEMBER", "DECEMBER"
};

std::string
ExtremeValueAnalysisAttributes::MonthType_ToString(ExtremeValueAnalysisAttributes::MonthType t)
{
    int index = int(t);
    if(index < 0 || index >= 12) index = 0;
    return MonthType_strings[index];
}

std::string
ExtremeValueAnalysisAttributes::MonthType_ToString(int t)
{
    int index = (t < 0 || t >= 12) ? 0 : t;
    return MonthType_strings[index];
}

bool
ExtremeValueAnalysisAttributes::MonthType_FromString(const std::string &s, ExtremeValueAnalysisAttributes::MonthType &val)
{
    val = ExtremeValueAnalysisAttributes::JANUARY;
    for(int i = 0; i < 12; ++i)
    {
        if(s == MonthType_strings[i])
        {
            val = (MonthType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ExtremeValueAnalysisAttributes::SeasonType
//

static const char *SeasonType_strings[] = {
"WINTER", "SPRING", "SUMMER", 
"FALL"};

std::string
ExtremeValueAnalysisAttributes::SeasonType_ToString(ExtremeValueAnalysisAttributes::SeasonType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return SeasonType_strings[index];
}

std::string
ExtremeValueAnalysisAttributes::SeasonType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return SeasonType_strings[index];
}

bool
ExtremeValueAnalysisAttributes::SeasonType_FromString(const std::string &s, ExtremeValueAnalysisAttributes::SeasonType &val)
{
    val = ExtremeValueAnalysisAttributes::WINTER;
    for(int i = 0; i < 4; ++i)
    {
        if(s == SeasonType_strings[i])
        {
            val = (SeasonType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ExtremeValueAnalysisAttributes::OptimizationType
//

static const char *OptimizationType_strings[] = {
"NELDER_MEAD", "BFGS"};

std::string
ExtremeValueAnalysisAttributes::OptimizationType_ToString(ExtremeValueAnalysisAttributes::OptimizationType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OptimizationType_strings[index];
}

std::string
ExtremeValueAnalysisAttributes::OptimizationType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OptimizationType_strings[index];
}

bool
ExtremeValueAnalysisAttributes::OptimizationType_FromString(const std::string &s, ExtremeValueAnalysisAttributes::OptimizationType &val)
{
    val = ExtremeValueAnalysisAttributes::NELDER_MEAD;
    for(int i = 0; i < 2; ++i)
    {
        if(s == OptimizationType_strings[i])
        {
            val = (OptimizationType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for ExtremeValueAnalysisAttributes::ExtremeType
//

static const char *ExtremeType_strings[] = {
"MINIMA", "MAXIMA"};

std::string
ExtremeValueAnalysisAttributes::ExtremeType_ToString(ExtremeValueAnalysisAttributes::ExtremeType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return ExtremeType_strings[index];
}

std::string
ExtremeValueAnalysisAttributes::ExtremeType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return ExtremeType_strings[index];
}

bool
ExtremeValueAnalysisAttributes::ExtremeType_FromString(const std::string &s, ExtremeValueAnalysisAttributes::ExtremeType &val)
{
    val = ExtremeValueAnalysisAttributes::MINIMA;
    for(int i = 0; i < 2; ++i)
    {
        if(s == ExtremeType_strings[i])
        {
            val = (ExtremeType)i;
            return true;
        }
    }
    return false;
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes
//
// Purpose: 
//   Init utility for the ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void ExtremeValueAnalysisAttributes::Init()
{
    dataYearBegin = 1;
    dataAnalysisYearRangeEnabled = false;
    dataAnalysisYear1 = 1;
    dataAnalysisYear2 = 1;
    ensemble = false;
    numEnsembles = 1;
    dataScaling = 1;
    extremeMethod = MAXIMA;
    optimizationMethod = NELDER_MEAD;
    aggregation = ANNUAL;
    covariateModelScale = false;
    covariateModelLocation = false;
    covariateModelShape = false;
    computeReturnValues = false;
    returnValues.push_back(1);
    computeRVDifferences = false;
    rvDifference1 = 1;
    rvDifference2 = 1;
    displayMonth = JANUARY;
    displaySeason = WINTER;
    computeParamValues = false;
    dumpData = true;
    dumpDebug = false;

    ExtremeValueAnalysisAttributes::SelectAll();
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes
//
// Purpose: 
//   Copy utility for the ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void ExtremeValueAnalysisAttributes::Copy(const ExtremeValueAnalysisAttributes &obj)
{
    dataYearBegin = obj.dataYearBegin;
    dataAnalysisYearRangeEnabled = obj.dataAnalysisYearRangeEnabled;
    dataAnalysisYear1 = obj.dataAnalysisYear1;
    dataAnalysisYear2 = obj.dataAnalysisYear2;
    ensemble = obj.ensemble;
    numEnsembles = obj.numEnsembles;
    dataScaling = obj.dataScaling;
    extremeMethod = obj.extremeMethod;
    optimizationMethod = obj.optimizationMethod;
    aggregation = obj.aggregation;
    covariateModelScale = obj.covariateModelScale;
    covariateModelLocation = obj.covariateModelLocation;
    covariateModelShape = obj.covariateModelShape;
    computeReturnValues = obj.computeReturnValues;
    returnValues = obj.returnValues;
    computeRVDifferences = obj.computeRVDifferences;
    rvDifference1 = obj.rvDifference1;
    rvDifference2 = obj.rvDifference2;
    displayMonth = obj.displayMonth;
    displaySeason = obj.displaySeason;
    computeParamValues = obj.computeParamValues;
    dumpData = obj.dumpData;
    dumpDebug = obj.dumpDebug;

    ExtremeValueAnalysisAttributes::SelectAll();
}

// Type map format string
const char *ExtremeValueAnalysisAttributes::TypeMapFormatString = EXTREMEVALUEANALYSISATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t ExtremeValueAnalysisAttributes::TmfsStruct = {EXTREMEVALUEANALYSISATTRIBUTES_TMFS};


// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes
//
// Purpose: 
//   Default constructor for the ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes() : 
    AttributeSubject(ExtremeValueAnalysisAttributes::TypeMapFormatString)
{
    ExtremeValueAnalysisAttributes::Init();
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes
//
// Purpose: 
//   Constructor for the derived classes of ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    ExtremeValueAnalysisAttributes::Init();
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes
//
// Purpose: 
//   Copy constructor for the ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes(const ExtremeValueAnalysisAttributes &obj) : 
    AttributeSubject(ExtremeValueAnalysisAttributes::TypeMapFormatString)
{
    ExtremeValueAnalysisAttributes::Copy(obj);
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ExtremeValueAnalysisAttributes::ExtremeValueAnalysisAttributes(const ExtremeValueAnalysisAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    ExtremeValueAnalysisAttributes::Copy(obj);
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::~ExtremeValueAnalysisAttributes
//
// Purpose: 
//   Destructor for the ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ExtremeValueAnalysisAttributes::~ExtremeValueAnalysisAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the ExtremeValueAnalysisAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

ExtremeValueAnalysisAttributes& 
ExtremeValueAnalysisAttributes::operator = (const ExtremeValueAnalysisAttributes &obj)
{
    if (this == &obj) return *this;

    ExtremeValueAnalysisAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the ExtremeValueAnalysisAttributes class.
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
ExtremeValueAnalysisAttributes::operator == (const ExtremeValueAnalysisAttributes &obj) const
{
    // Create the return value
    return ((dataYearBegin == obj.dataYearBegin) &&
            (dataAnalysisYearRangeEnabled == obj.dataAnalysisYearRangeEnabled) &&
            (dataAnalysisYear1 == obj.dataAnalysisYear1) &&
            (dataAnalysisYear2 == obj.dataAnalysisYear2) &&
            (ensemble == obj.ensemble) &&
            (numEnsembles == obj.numEnsembles) &&
            (dataScaling == obj.dataScaling) &&
            (extremeMethod == obj.extremeMethod) &&
            (optimizationMethod == obj.optimizationMethod) &&
            (aggregation == obj.aggregation) &&
            (covariateModelScale == obj.covariateModelScale) &&
            (covariateModelLocation == obj.covariateModelLocation) &&
            (covariateModelShape == obj.covariateModelShape) &&
            (computeReturnValues == obj.computeReturnValues) &&
            (returnValues == obj.returnValues) &&
            (computeRVDifferences == obj.computeRVDifferences) &&
            (rvDifference1 == obj.rvDifference1) &&
            (rvDifference2 == obj.rvDifference2) &&
            (displayMonth == obj.displayMonth) &&
            (displaySeason == obj.displaySeason) &&
            (computeParamValues == obj.computeParamValues) &&
            (dumpData == obj.dumpData) &&
            (dumpDebug == obj.dumpDebug));
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the ExtremeValueAnalysisAttributes class.
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
ExtremeValueAnalysisAttributes::operator != (const ExtremeValueAnalysisAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::TypeName
//
// Purpose: 
//   Type name method for the ExtremeValueAnalysisAttributes class.
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
ExtremeValueAnalysisAttributes::TypeName() const
{
    return "ExtremeValueAnalysisAttributes";
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the ExtremeValueAnalysisAttributes class.
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
ExtremeValueAnalysisAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const ExtremeValueAnalysisAttributes *tmp = (const ExtremeValueAnalysisAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the ExtremeValueAnalysisAttributes class.
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
ExtremeValueAnalysisAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new ExtremeValueAnalysisAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the ExtremeValueAnalysisAttributes class.
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
ExtremeValueAnalysisAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new ExtremeValueAnalysisAttributes(*this);
    else
        retval = new ExtremeValueAnalysisAttributes;

    return retval;
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::SelectAll
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
ExtremeValueAnalysisAttributes::SelectAll()
{
    Select(ID_dataYearBegin,                (void *)&dataYearBegin);
    Select(ID_dataAnalysisYearRangeEnabled, (void *)&dataAnalysisYearRangeEnabled);
    Select(ID_dataAnalysisYear1,            (void *)&dataAnalysisYear1);
    Select(ID_dataAnalysisYear2,            (void *)&dataAnalysisYear2);
    Select(ID_ensemble,                     (void *)&ensemble);
    Select(ID_numEnsembles,                 (void *)&numEnsembles);
    Select(ID_dataScaling,                  (void *)&dataScaling);
    Select(ID_extremeMethod,                (void *)&extremeMethod);
    Select(ID_optimizationMethod,           (void *)&optimizationMethod);
    Select(ID_aggregation,                  (void *)&aggregation);
    Select(ID_covariateModelScale,          (void *)&covariateModelScale);
    Select(ID_covariateModelLocation,       (void *)&covariateModelLocation);
    Select(ID_covariateModelShape,          (void *)&covariateModelShape);
    Select(ID_computeReturnValues,          (void *)&computeReturnValues);
    Select(ID_returnValues,                 (void *)&returnValues);
    Select(ID_computeRVDifferences,         (void *)&computeRVDifferences);
    Select(ID_rvDifference1,                (void *)&rvDifference1);
    Select(ID_rvDifference2,                (void *)&rvDifference2);
    Select(ID_displayMonth,                 (void *)&displayMonth);
    Select(ID_displaySeason,                (void *)&displaySeason);
    Select(ID_computeParamValues,           (void *)&computeParamValues);
    Select(ID_dumpData,                     (void *)&dumpData);
    Select(ID_dumpDebug,                    (void *)&dumpDebug);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::CreateNode
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
ExtremeValueAnalysisAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    ExtremeValueAnalysisAttributes defaultObject;
    bool addToParent = false;
    // Create a node for ExtremeValueAnalysisAttributes.
    DataNode *node = new DataNode("ExtremeValueAnalysisAttributes");

    if(completeSave || !FieldsEqual(ID_dataYearBegin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataYearBegin", dataYearBegin));
    }

    if(completeSave || !FieldsEqual(ID_dataAnalysisYearRangeEnabled, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataAnalysisYearRangeEnabled", dataAnalysisYearRangeEnabled));
    }

    if(completeSave || !FieldsEqual(ID_dataAnalysisYear1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataAnalysisYear1", dataAnalysisYear1));
    }

    if(completeSave || !FieldsEqual(ID_dataAnalysisYear2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataAnalysisYear2", dataAnalysisYear2));
    }

    if(completeSave || !FieldsEqual(ID_ensemble, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("ensemble", ensemble));
    }

    if(completeSave || !FieldsEqual(ID_numEnsembles, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("numEnsembles", numEnsembles));
    }

    if(completeSave || !FieldsEqual(ID_dataScaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataScaling", dataScaling));
    }

    if(completeSave || !FieldsEqual(ID_extremeMethod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("extremeMethod", ExtremeType_ToString(extremeMethod)));
    }

    if(completeSave || !FieldsEqual(ID_optimizationMethod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("optimizationMethod", OptimizationType_ToString(optimizationMethod)));
    }

    if(completeSave || !FieldsEqual(ID_aggregation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("aggregation", AggregationType_ToString(aggregation)));
    }

    if(completeSave || !FieldsEqual(ID_covariateModelScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("covariateModelScale", covariateModelScale));
    }

    if(completeSave || !FieldsEqual(ID_covariateModelLocation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("covariateModelLocation", covariateModelLocation));
    }

    if(completeSave || !FieldsEqual(ID_covariateModelShape, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("covariateModelShape", covariateModelShape));
    }

    if(completeSave || !FieldsEqual(ID_computeReturnValues, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("computeReturnValues", computeReturnValues));
    }

    if(completeSave || !FieldsEqual(ID_returnValues, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("returnValues", returnValues));
    }

    if(completeSave || !FieldsEqual(ID_computeRVDifferences, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("computeRVDifferences", computeRVDifferences));
    }

    if(completeSave || !FieldsEqual(ID_rvDifference1, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rvDifference1", rvDifference1));
    }

    if(completeSave || !FieldsEqual(ID_rvDifference2, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rvDifference2", rvDifference2));
    }

    if(completeSave || !FieldsEqual(ID_displayMonth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayMonth", MonthType_ToString(displayMonth)));
    }

    if(completeSave || !FieldsEqual(ID_displaySeason, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displaySeason", SeasonType_ToString(displaySeason)));
    }

    if(completeSave || !FieldsEqual(ID_computeParamValues, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("computeParamValues", computeParamValues));
    }

    if(completeSave || !FieldsEqual(ID_dumpData, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dumpData", dumpData));
    }

    if(completeSave || !FieldsEqual(ID_dumpDebug, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dumpDebug", dumpDebug));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::SetFromNode
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
ExtremeValueAnalysisAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ExtremeValueAnalysisAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("dataYearBegin")) != 0)
        SetDataYearBegin(node->AsInt());
    if((node = searchNode->GetNode("dataAnalysisYearRangeEnabled")) != 0)
        SetDataAnalysisYearRangeEnabled(node->AsBool());
    if((node = searchNode->GetNode("dataAnalysisYear1")) != 0)
        SetDataAnalysisYear1(node->AsInt());
    if((node = searchNode->GetNode("dataAnalysisYear2")) != 0)
        SetDataAnalysisYear2(node->AsInt());
    if((node = searchNode->GetNode("ensemble")) != 0)
        SetEnsemble(node->AsBool());
    if((node = searchNode->GetNode("numEnsembles")) != 0)
        SetNumEnsembles(node->AsInt());
    if((node = searchNode->GetNode("dataScaling")) != 0)
        SetDataScaling(node->AsDouble());
    if((node = searchNode->GetNode("extremeMethod")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetExtremeMethod(ExtremeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            ExtremeType value;
            if(ExtremeType_FromString(node->AsString(), value))
                SetExtremeMethod(value);
        }
    }
    if((node = searchNode->GetNode("optimizationMethod")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetOptimizationMethod(OptimizationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            OptimizationType value;
            if(OptimizationType_FromString(node->AsString(), value))
                SetOptimizationMethod(value);
        }
    }
    if((node = searchNode->GetNode("aggregation")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 3)
                SetAggregation(AggregationType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            AggregationType value;
            if(AggregationType_FromString(node->AsString(), value))
                SetAggregation(value);
        }
    }
    if((node = searchNode->GetNode("covariateModelScale")) != 0)
        SetCovariateModelScale(node->AsBool());
    if((node = searchNode->GetNode("covariateModelLocation")) != 0)
        SetCovariateModelLocation(node->AsBool());
    if((node = searchNode->GetNode("covariateModelShape")) != 0)
        SetCovariateModelShape(node->AsBool());
    if((node = searchNode->GetNode("computeReturnValues")) != 0)
        SetComputeReturnValues(node->AsBool());
    if((node = searchNode->GetNode("returnValues")) != 0)
        SetReturnValues(node->AsIntVector());
    if((node = searchNode->GetNode("computeRVDifferences")) != 0)
        SetComputeRVDifferences(node->AsBool());
    if((node = searchNode->GetNode("rvDifference1")) != 0)
        SetRvDifference1(node->AsInt());
    if((node = searchNode->GetNode("rvDifference2")) != 0)
        SetRvDifference2(node->AsInt());
    if((node = searchNode->GetNode("displayMonth")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 12)
                SetDisplayMonth(MonthType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            MonthType value;
            if(MonthType_FromString(node->AsString(), value))
                SetDisplayMonth(value);
        }
    }
    if((node = searchNode->GetNode("displaySeason")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 4)
                SetDisplaySeason(SeasonType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            SeasonType value;
            if(SeasonType_FromString(node->AsString(), value))
                SetDisplaySeason(value);
        }
    }
    if((node = searchNode->GetNode("computeParamValues")) != 0)
        SetComputeParamValues(node->AsBool());
    if((node = searchNode->GetNode("dumpData")) != 0)
        SetDumpData(node->AsBool());
    if((node = searchNode->GetNode("dumpDebug")) != 0)
        SetDumpDebug(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
ExtremeValueAnalysisAttributes::SetDataYearBegin(int dataYearBegin_)
{
    dataYearBegin = dataYearBegin_;
    Select(ID_dataYearBegin, (void *)&dataYearBegin);
}

void
ExtremeValueAnalysisAttributes::SetDataAnalysisYearRangeEnabled(bool dataAnalysisYearRangeEnabled_)
{
    dataAnalysisYearRangeEnabled = dataAnalysisYearRangeEnabled_;
    Select(ID_dataAnalysisYearRangeEnabled, (void *)&dataAnalysisYearRangeEnabled);
}

void
ExtremeValueAnalysisAttributes::SetDataAnalysisYear1(int dataAnalysisYear1_)
{
    dataAnalysisYear1 = dataAnalysisYear1_;
    Select(ID_dataAnalysisYear1, (void *)&dataAnalysisYear1);
}

void
ExtremeValueAnalysisAttributes::SetDataAnalysisYear2(int dataAnalysisYear2_)
{
    dataAnalysisYear2 = dataAnalysisYear2_;
    Select(ID_dataAnalysisYear2, (void *)&dataAnalysisYear2);
}

void
ExtremeValueAnalysisAttributes::SetEnsemble(bool ensemble_)
{
    ensemble = ensemble_;
    Select(ID_ensemble, (void *)&ensemble);
}

void
ExtremeValueAnalysisAttributes::SetNumEnsembles(int numEnsembles_)
{
    numEnsembles = numEnsembles_;
    Select(ID_numEnsembles, (void *)&numEnsembles);
}

void
ExtremeValueAnalysisAttributes::SetDataScaling(double dataScaling_)
{
    dataScaling = dataScaling_;
    Select(ID_dataScaling, (void *)&dataScaling);
}

void
ExtremeValueAnalysisAttributes::SetExtremeMethod(ExtremeValueAnalysisAttributes::ExtremeType extremeMethod_)
{
    extremeMethod = extremeMethod_;
    Select(ID_extremeMethod, (void *)&extremeMethod);
}

void
ExtremeValueAnalysisAttributes::SetOptimizationMethod(ExtremeValueAnalysisAttributes::OptimizationType optimizationMethod_)
{
    optimizationMethod = optimizationMethod_;
    Select(ID_optimizationMethod, (void *)&optimizationMethod);
}

void
ExtremeValueAnalysisAttributes::SetAggregation(ExtremeValueAnalysisAttributes::AggregationType aggregation_)
{
    aggregation = aggregation_;
    Select(ID_aggregation, (void *)&aggregation);
}

void
ExtremeValueAnalysisAttributes::SetCovariateModelScale(bool covariateModelScale_)
{
    covariateModelScale = covariateModelScale_;
    Select(ID_covariateModelScale, (void *)&covariateModelScale);
}

void
ExtremeValueAnalysisAttributes::SetCovariateModelLocation(bool covariateModelLocation_)
{
    covariateModelLocation = covariateModelLocation_;
    Select(ID_covariateModelLocation, (void *)&covariateModelLocation);
}

void
ExtremeValueAnalysisAttributes::SetCovariateModelShape(bool covariateModelShape_)
{
    covariateModelShape = covariateModelShape_;
    Select(ID_covariateModelShape, (void *)&covariateModelShape);
}

void
ExtremeValueAnalysisAttributes::SetComputeReturnValues(bool computeReturnValues_)
{
    computeReturnValues = computeReturnValues_;
    Select(ID_computeReturnValues, (void *)&computeReturnValues);
}

void
ExtremeValueAnalysisAttributes::SetReturnValues(const intVector &returnValues_)
{
    returnValues = returnValues_;
    Select(ID_returnValues, (void *)&returnValues);
}

void
ExtremeValueAnalysisAttributes::SetComputeRVDifferences(bool computeRVDifferences_)
{
    computeRVDifferences = computeRVDifferences_;
    Select(ID_computeRVDifferences, (void *)&computeRVDifferences);
}

void
ExtremeValueAnalysisAttributes::SetRvDifference1(int rvDifference1_)
{
    rvDifference1 = rvDifference1_;
    Select(ID_rvDifference1, (void *)&rvDifference1);
}

void
ExtremeValueAnalysisAttributes::SetRvDifference2(int rvDifference2_)
{
    rvDifference2 = rvDifference2_;
    Select(ID_rvDifference2, (void *)&rvDifference2);
}

void
ExtremeValueAnalysisAttributes::SetDisplayMonth(ExtremeValueAnalysisAttributes::MonthType displayMonth_)
{
    displayMonth = displayMonth_;
    Select(ID_displayMonth, (void *)&displayMonth);
}

void
ExtremeValueAnalysisAttributes::SetDisplaySeason(ExtremeValueAnalysisAttributes::SeasonType displaySeason_)
{
    displaySeason = displaySeason_;
    Select(ID_displaySeason, (void *)&displaySeason);
}

void
ExtremeValueAnalysisAttributes::SetComputeParamValues(bool computeParamValues_)
{
    computeParamValues = computeParamValues_;
    Select(ID_computeParamValues, (void *)&computeParamValues);
}

void
ExtremeValueAnalysisAttributes::SetDumpData(bool dumpData_)
{
    dumpData = dumpData_;
    Select(ID_dumpData, (void *)&dumpData);
}

void
ExtremeValueAnalysisAttributes::SetDumpDebug(bool dumpDebug_)
{
    dumpDebug = dumpDebug_;
    Select(ID_dumpDebug, (void *)&dumpDebug);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

int
ExtremeValueAnalysisAttributes::GetDataYearBegin() const
{
    return dataYearBegin;
}

bool
ExtremeValueAnalysisAttributes::GetDataAnalysisYearRangeEnabled() const
{
    return dataAnalysisYearRangeEnabled;
}

int
ExtremeValueAnalysisAttributes::GetDataAnalysisYear1() const
{
    return dataAnalysisYear1;
}

int
ExtremeValueAnalysisAttributes::GetDataAnalysisYear2() const
{
    return dataAnalysisYear2;
}

bool
ExtremeValueAnalysisAttributes::GetEnsemble() const
{
    return ensemble;
}

int
ExtremeValueAnalysisAttributes::GetNumEnsembles() const
{
    return numEnsembles;
}

double
ExtremeValueAnalysisAttributes::GetDataScaling() const
{
    return dataScaling;
}

ExtremeValueAnalysisAttributes::ExtremeType
ExtremeValueAnalysisAttributes::GetExtremeMethod() const
{
    return ExtremeType(extremeMethod);
}

ExtremeValueAnalysisAttributes::OptimizationType
ExtremeValueAnalysisAttributes::GetOptimizationMethod() const
{
    return OptimizationType(optimizationMethod);
}

ExtremeValueAnalysisAttributes::AggregationType
ExtremeValueAnalysisAttributes::GetAggregation() const
{
    return AggregationType(aggregation);
}

bool
ExtremeValueAnalysisAttributes::GetCovariateModelScale() const
{
    return covariateModelScale;
}

bool
ExtremeValueAnalysisAttributes::GetCovariateModelLocation() const
{
    return covariateModelLocation;
}

bool
ExtremeValueAnalysisAttributes::GetCovariateModelShape() const
{
    return covariateModelShape;
}

bool
ExtremeValueAnalysisAttributes::GetComputeReturnValues() const
{
    return computeReturnValues;
}

const intVector &
ExtremeValueAnalysisAttributes::GetReturnValues() const
{
    return returnValues;
}

intVector &
ExtremeValueAnalysisAttributes::GetReturnValues()
{
    return returnValues;
}

bool
ExtremeValueAnalysisAttributes::GetComputeRVDifferences() const
{
    return computeRVDifferences;
}

int
ExtremeValueAnalysisAttributes::GetRvDifference1() const
{
    return rvDifference1;
}

int
ExtremeValueAnalysisAttributes::GetRvDifference2() const
{
    return rvDifference2;
}

ExtremeValueAnalysisAttributes::MonthType
ExtremeValueAnalysisAttributes::GetDisplayMonth() const
{
    return MonthType(displayMonth);
}

ExtremeValueAnalysisAttributes::SeasonType
ExtremeValueAnalysisAttributes::GetDisplaySeason() const
{
    return SeasonType(displaySeason);
}

bool
ExtremeValueAnalysisAttributes::GetComputeParamValues() const
{
    return computeParamValues;
}

bool
ExtremeValueAnalysisAttributes::GetDumpData() const
{
    return dumpData;
}

bool
ExtremeValueAnalysisAttributes::GetDumpDebug() const
{
    return dumpDebug;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
ExtremeValueAnalysisAttributes::SelectReturnValues()
{
    Select(ID_returnValues, (void *)&returnValues);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::GetFieldName
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
ExtremeValueAnalysisAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_dataYearBegin:                return "dataYearBegin";
    case ID_dataAnalysisYearRangeEnabled: return "dataAnalysisYearRangeEnabled";
    case ID_dataAnalysisYear1:            return "dataAnalysisYear1";
    case ID_dataAnalysisYear2:            return "dataAnalysisYear2";
    case ID_ensemble:                     return "ensemble";
    case ID_numEnsembles:                 return "numEnsembles";
    case ID_dataScaling:                  return "dataScaling";
    case ID_extremeMethod:                return "extremeMethod";
    case ID_optimizationMethod:           return "optimizationMethod";
    case ID_aggregation:                  return "aggregation";
    case ID_covariateModelScale:          return "covariateModelScale";
    case ID_covariateModelLocation:       return "covariateModelLocation";
    case ID_covariateModelShape:          return "covariateModelShape";
    case ID_computeReturnValues:          return "computeReturnValues";
    case ID_returnValues:                 return "returnValues";
    case ID_computeRVDifferences:         return "computeRVDifferences";
    case ID_rvDifference1:                return "rvDifference1";
    case ID_rvDifference2:                return "rvDifference2";
    case ID_displayMonth:                 return "displayMonth";
    case ID_displaySeason:                return "displaySeason";
    case ID_computeParamValues:           return "computeParamValues";
    case ID_dumpData:                     return "dumpData";
    case ID_dumpDebug:                    return "dumpDebug";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::GetFieldType
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
ExtremeValueAnalysisAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_dataYearBegin:                return FieldType_int;
    case ID_dataAnalysisYearRangeEnabled: return FieldType_bool;
    case ID_dataAnalysisYear1:            return FieldType_int;
    case ID_dataAnalysisYear2:            return FieldType_int;
    case ID_ensemble:                     return FieldType_bool;
    case ID_numEnsembles:                 return FieldType_int;
    case ID_dataScaling:                  return FieldType_double;
    case ID_extremeMethod:                return FieldType_enum;
    case ID_optimizationMethod:           return FieldType_enum;
    case ID_aggregation:                  return FieldType_enum;
    case ID_covariateModelScale:          return FieldType_bool;
    case ID_covariateModelLocation:       return FieldType_bool;
    case ID_covariateModelShape:          return FieldType_bool;
    case ID_computeReturnValues:          return FieldType_bool;
    case ID_returnValues:                 return FieldType_intVector;
    case ID_computeRVDifferences:         return FieldType_bool;
    case ID_rvDifference1:                return FieldType_int;
    case ID_rvDifference2:                return FieldType_int;
    case ID_displayMonth:                 return FieldType_enum;
    case ID_displaySeason:                return FieldType_enum;
    case ID_computeParamValues:           return FieldType_bool;
    case ID_dumpData:                     return FieldType_bool;
    case ID_dumpDebug:                    return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::GetFieldTypeName
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
ExtremeValueAnalysisAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_dataYearBegin:                return "int";
    case ID_dataAnalysisYearRangeEnabled: return "bool";
    case ID_dataAnalysisYear1:            return "int";
    case ID_dataAnalysisYear2:            return "int";
    case ID_ensemble:                     return "bool";
    case ID_numEnsembles:                 return "int";
    case ID_dataScaling:                  return "double";
    case ID_extremeMethod:                return "enum";
    case ID_optimizationMethod:           return "enum";
    case ID_aggregation:                  return "enum";
    case ID_covariateModelScale:          return "bool";
    case ID_covariateModelLocation:       return "bool";
    case ID_covariateModelShape:          return "bool";
    case ID_computeReturnValues:          return "bool";
    case ID_returnValues:                 return "intVector";
    case ID_computeRVDifferences:         return "bool";
    case ID_rvDifference1:                return "int";
    case ID_rvDifference2:                return "int";
    case ID_displayMonth:                 return "enum";
    case ID_displaySeason:                return "enum";
    case ID_computeParamValues:           return "bool";
    case ID_dumpData:                     return "bool";
    case ID_dumpDebug:                    return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: ExtremeValueAnalysisAttributes::FieldsEqual
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
ExtremeValueAnalysisAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const ExtremeValueAnalysisAttributes &obj = *((const ExtremeValueAnalysisAttributes*)rhs);
    bool retval = false;
    switch (index_)
    {
    case ID_dataYearBegin:
        {  // new scope
        retval = (dataYearBegin == obj.dataYearBegin);
        }
        break;
    case ID_dataAnalysisYearRangeEnabled:
        {  // new scope
        retval = (dataAnalysisYearRangeEnabled == obj.dataAnalysisYearRangeEnabled);
        }
        break;
    case ID_dataAnalysisYear1:
        {  // new scope
        retval = (dataAnalysisYear1 == obj.dataAnalysisYear1);
        }
        break;
    case ID_dataAnalysisYear2:
        {  // new scope
        retval = (dataAnalysisYear2 == obj.dataAnalysisYear2);
        }
        break;
    case ID_ensemble:
        {  // new scope
        retval = (ensemble == obj.ensemble);
        }
        break;
    case ID_numEnsembles:
        {  // new scope
        retval = (numEnsembles == obj.numEnsembles);
        }
        break;
    case ID_dataScaling:
        {  // new scope
        retval = (dataScaling == obj.dataScaling);
        }
        break;
    case ID_extremeMethod:
        {  // new scope
        retval = (extremeMethod == obj.extremeMethod);
        }
        break;
    case ID_optimizationMethod:
        {  // new scope
        retval = (optimizationMethod == obj.optimizationMethod);
        }
        break;
    case ID_aggregation:
        {  // new scope
        retval = (aggregation == obj.aggregation);
        }
        break;
    case ID_covariateModelScale:
        {  // new scope
        retval = (covariateModelScale == obj.covariateModelScale);
        }
        break;
    case ID_covariateModelLocation:
        {  // new scope
        retval = (covariateModelLocation == obj.covariateModelLocation);
        }
        break;
    case ID_covariateModelShape:
        {  // new scope
        retval = (covariateModelShape == obj.covariateModelShape);
        }
        break;
    case ID_computeReturnValues:
        {  // new scope
        retval = (computeReturnValues == obj.computeReturnValues);
        }
        break;
    case ID_returnValues:
        {  // new scope
        retval = (returnValues == obj.returnValues);
        }
        break;
    case ID_computeRVDifferences:
        {  // new scope
        retval = (computeRVDifferences == obj.computeRVDifferences);
        }
        break;
    case ID_rvDifference1:
        {  // new scope
        retval = (rvDifference1 == obj.rvDifference1);
        }
        break;
    case ID_rvDifference2:
        {  // new scope
        retval = (rvDifference2 == obj.rvDifference2);
        }
        break;
    case ID_displayMonth:
        {  // new scope
        retval = (displayMonth == obj.displayMonth);
        }
        break;
    case ID_displaySeason:
        {  // new scope
        retval = (displaySeason == obj.displaySeason);
        }
        break;
    case ID_computeParamValues:
        {  // new scope
        retval = (computeParamValues == obj.computeParamValues);
        }
        break;
    case ID_dumpData:
        {  // new scope
        retval = (dumpData == obj.dumpData);
        }
        break;
    case ID_dumpDebug:
        {  // new scope
        retval = (dumpDebug == obj.dumpDebug);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

