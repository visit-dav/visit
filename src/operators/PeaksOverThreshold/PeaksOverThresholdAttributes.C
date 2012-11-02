/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include <PeaksOverThresholdAttributes.h>
#include <DataNode.h>

//
// Enum conversion methods for PeaksOverThresholdAttributes::AggregationType
//

static const char *AggregationType_strings[] = {
"ANNUAL", "SEASONAL", "MONTHLY"
};

std::string
PeaksOverThresholdAttributes::AggregationType_ToString(PeaksOverThresholdAttributes::AggregationType t)
{
    int index = int(t);
    if(index < 0 || index >= 3) index = 0;
    return AggregationType_strings[index];
}

std::string
PeaksOverThresholdAttributes::AggregationType_ToString(int t)
{
    int index = (t < 0 || t >= 3) ? 0 : t;
    return AggregationType_strings[index];
}

bool
PeaksOverThresholdAttributes::AggregationType_FromString(const std::string &s, PeaksOverThresholdAttributes::AggregationType &val)
{
    val = PeaksOverThresholdAttributes::ANNUAL;
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
// Enum conversion methods for PeaksOverThresholdAttributes::SeasonType
//

static const char *SeasonType_strings[] = {
"WINTER", "SPRING", "SUMMER", 
"FALL"};

std::string
PeaksOverThresholdAttributes::SeasonType_ToString(PeaksOverThresholdAttributes::SeasonType t)
{
    int index = int(t);
    if(index < 0 || index >= 4) index = 0;
    return SeasonType_strings[index];
}

std::string
PeaksOverThresholdAttributes::SeasonType_ToString(int t)
{
    int index = (t < 0 || t >= 4) ? 0 : t;
    return SeasonType_strings[index];
}

bool
PeaksOverThresholdAttributes::SeasonType_FromString(const std::string &s, PeaksOverThresholdAttributes::SeasonType &val)
{
    val = PeaksOverThresholdAttributes::WINTER;
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
// Enum conversion methods for PeaksOverThresholdAttributes::MonthType
//

static const char *MonthType_strings[] = {
"JAN", "FEB", "MAR", 
"APR", "MAY", "JUN", 
"JUL", "AUG", "SEP", 
"OCT", "NOV", "DEC"
};

std::string
PeaksOverThresholdAttributes::MonthType_ToString(PeaksOverThresholdAttributes::MonthType t)
{
    int index = int(t);
    if(index < 0 || index >= 12) index = 0;
    return MonthType_strings[index];
}

std::string
PeaksOverThresholdAttributes::MonthType_ToString(int t)
{
    int index = (t < 0 || t >= 12) ? 0 : t;
    return MonthType_strings[index];
}

bool
PeaksOverThresholdAttributes::MonthType_FromString(const std::string &s, PeaksOverThresholdAttributes::MonthType &val)
{
    val = PeaksOverThresholdAttributes::JAN;
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

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::PeaksOverThresholdAttributes
//
// Purpose: 
//   Init utility for the PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PeaksOverThresholdAttributes::Init()
{
    dataYearBegin = 1;
    dataAnalysisYearRangeEnabled = false;
    dataAnalysisYearRange[0] = 0;
    dataAnalysisYearRange[1] = 0;
    aggregation = ANNUAL;
    annualPercentile = 0.9;
    seasonalPercentile[0] = 0.9;
    seasonalPercentile[1] = 0.9;
    seasonalPercentile[2] = 0.9;
    seasonalPercentile[3] = 0.9;
    monthlyPercentile[0] = 0.9;
    monthlyPercentile[1] = 0.9;
    monthlyPercentile[2] = 0.9;
    monthlyPercentile[3] = 0.9;
    monthlyPercentile[4] = 0.9;
    monthlyPercentile[5] = 0.9;
    monthlyPercentile[6] = 0.9;
    monthlyPercentile[7] = 0.9;
    monthlyPercentile[8] = 0.9;
    monthlyPercentile[9] = 0.9;
    monthlyPercentile[10] = 0.9;
    monthlyPercentile[11] = 0.9;
    displaySeason = WINTER;
    displayMonth = JAN;
    cutoff = 0;
    computeParamValues = false;
    computeCovariates = false;
    covariateReturnYears.push_back(1);
    covariateModelLocation = false;
    covariateModelShape = false;
    covariateModelScale = false;
    computeRVDifferences = false;
    rvDifferences[0] = 0;
    rvDifferences[1] = 0;
    dataScaling = 86500;
    dumpData = false;

    PeaksOverThresholdAttributes::SelectAll();
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::PeaksOverThresholdAttributes
//
// Purpose: 
//   Copy utility for the PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

void PeaksOverThresholdAttributes::Copy(const PeaksOverThresholdAttributes &obj)
{

    dataYearBegin = obj.dataYearBegin;
    dataAnalysisYearRangeEnabled = obj.dataAnalysisYearRangeEnabled;
    dataAnalysisYearRange[0] = obj.dataAnalysisYearRange[0];
    dataAnalysisYearRange[1] = obj.dataAnalysisYearRange[1];

    aggregation = obj.aggregation;
    annualPercentile = obj.annualPercentile;
    for(int i = 0; i < 4; ++i)
        seasonalPercentile[i] = obj.seasonalPercentile[i];

    for(int i = 0; i < 12; ++i)
        monthlyPercentile[i] = obj.monthlyPercentile[i];

    displaySeason = obj.displaySeason;
    displayMonth = obj.displayMonth;
    dataYearBegin = obj.dataYearBegin;
    cutoff = obj.cutoff;
    computeParamValues = obj.computeParamValues;
    computeCovariates = obj.computeCovariates;
    covariateReturnYears = obj.covariateReturnYears;
    covariateModelLocation = obj.covariateModelLocation;
    covariateModelShape = obj.covariateModelShape;
    covariateModelScale = obj.covariateModelScale;
    computeRVDifferences = obj.computeRVDifferences;
    rvDifferences[0] = obj.rvDifferences[0];
    rvDifferences[1] = obj.rvDifferences[1];

    dataScaling = obj.dataScaling;
    dumpData = obj.dumpData;

    PeaksOverThresholdAttributes::SelectAll();
}

// Type map format string
const char *PeaksOverThresholdAttributes::TypeMapFormatString = PEAKSOVERTHRESHOLDATTRIBUTES_TMFS;
const AttributeGroup::private_tmfs_t PeaksOverThresholdAttributes::TmfsStruct = {PEAKSOVERTHRESHOLDATTRIBUTES_TMFS};


// ****************************************************************************
// Method: PeaksOverThresholdAttributes::PeaksOverThresholdAttributes
//
// Purpose: 
//   Default constructor for the PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PeaksOverThresholdAttributes::PeaksOverThresholdAttributes() : 
    AttributeSubject(PeaksOverThresholdAttributes::TypeMapFormatString)
{
    PeaksOverThresholdAttributes::Init();
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::PeaksOverThresholdAttributes
//
// Purpose: 
//   Constructor for the derived classes of PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PeaksOverThresholdAttributes::PeaksOverThresholdAttributes(private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    PeaksOverThresholdAttributes::Init();
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::PeaksOverThresholdAttributes
//
// Purpose: 
//   Copy constructor for the PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PeaksOverThresholdAttributes::PeaksOverThresholdAttributes(const PeaksOverThresholdAttributes &obj) : 
    AttributeSubject(PeaksOverThresholdAttributes::TypeMapFormatString)
{
    PeaksOverThresholdAttributes::Copy(obj);
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::PeaksOverThresholdAttributes
//
// Purpose: 
//   Copy constructor for derived classes of the PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PeaksOverThresholdAttributes::PeaksOverThresholdAttributes(const PeaksOverThresholdAttributes &obj, private_tmfs_t tmfs) : 
    AttributeSubject(tmfs.tmfs)
{
    PeaksOverThresholdAttributes::Copy(obj);
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::~PeaksOverThresholdAttributes
//
// Purpose: 
//   Destructor for the PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PeaksOverThresholdAttributes::~PeaksOverThresholdAttributes()
{
    // nothing here
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::operator = 
//
// Purpose: 
//   Assignment operator for the PeaksOverThresholdAttributes class.
//
// Note:       Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

PeaksOverThresholdAttributes& 
PeaksOverThresholdAttributes::operator = (const PeaksOverThresholdAttributes &obj)
{
    if (this == &obj) return *this;

    PeaksOverThresholdAttributes::Copy(obj);

    return *this;
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::operator == 
//
// Purpose: 
//   Comparison operator == for the PeaksOverThresholdAttributes class.
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
PeaksOverThresholdAttributes::operator == (const PeaksOverThresholdAttributes &obj) const
{
    // Compare the dataAnalysisYearRange arrays.
    bool dataAnalysisYearRange_equal = true;
    for(int i = 0; i < 2 && dataAnalysisYearRange_equal; ++i)
        dataAnalysisYearRange_equal = (dataAnalysisYearRange[i] == obj.dataAnalysisYearRange[i]);

    // Compare the seasonalPercentile arrays.
    bool seasonalPercentile_equal = true;
    for(int i = 0; i < 4 && seasonalPercentile_equal; ++i)
        seasonalPercentile_equal = (seasonalPercentile[i] == obj.seasonalPercentile[i]);

    // Compare the monthlyPercentile arrays.
    bool monthlyPercentile_equal = true;
    for(int i = 0; i < 12 && monthlyPercentile_equal; ++i)
        monthlyPercentile_equal = (monthlyPercentile[i] == obj.monthlyPercentile[i]);

    // Compare the rvDifferences arrays.
    bool rvDifferences_equal = true;
    for(int i = 0; i < 2 && rvDifferences_equal; ++i)
        rvDifferences_equal = (rvDifferences[i] == obj.rvDifferences[i]);

    // Create the return value
    return ((dataYearBegin == obj.dataYearBegin) &&
            (dataAnalysisYearRangeEnabled == obj.dataAnalysisYearRangeEnabled) &&
            dataAnalysisYearRange_equal &&
            (aggregation == obj.aggregation) &&
            (annualPercentile == obj.annualPercentile) &&
            seasonalPercentile_equal &&
            monthlyPercentile_equal &&
            (displaySeason == obj.displaySeason) &&
            (displayMonth == obj.displayMonth) &&
            (dataYearBegin == obj.dataYearBegin) &&
            (cutoff == obj.cutoff) &&
            (computeParamValues == obj.computeParamValues) &&
            (computeCovariates == obj.computeCovariates) &&
            (covariateReturnYears == obj.covariateReturnYears) &&
            (covariateModelLocation == obj.covariateModelLocation) &&
            (covariateModelShape == obj.covariateModelShape) &&
            (covariateModelScale == obj.covariateModelScale) &&
            (computeRVDifferences == obj.computeRVDifferences) &&
            rvDifferences_equal &&
            (dataScaling == obj.dataScaling) &&
            (dumpData == obj.dumpData));
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::operator != 
//
// Purpose: 
//   Comparison operator != for the PeaksOverThresholdAttributes class.
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
PeaksOverThresholdAttributes::operator != (const PeaksOverThresholdAttributes &obj) const
{
    return !(this->operator == (obj));
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::TypeName
//
// Purpose: 
//   Type name method for the PeaksOverThresholdAttributes class.
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
PeaksOverThresholdAttributes::TypeName() const
{
    return "PeaksOverThresholdAttributes";
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::CopyAttributes
//
// Purpose: 
//   CopyAttributes method for the PeaksOverThresholdAttributes class.
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
PeaksOverThresholdAttributes::CopyAttributes(const AttributeGroup *atts)
{
    if(TypeName() != atts->TypeName())
        return false;

    // Call assignment operator.
    const PeaksOverThresholdAttributes *tmp = (const PeaksOverThresholdAttributes *)atts;
    *this = *tmp;

    return true;
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::CreateCompatible
//
// Purpose: 
//   CreateCompatible method for the PeaksOverThresholdAttributes class.
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
PeaksOverThresholdAttributes::CreateCompatible(const std::string &tname) const
{
    AttributeSubject *retval = 0;
    if(TypeName() == tname)
        retval = new PeaksOverThresholdAttributes(*this);
    // Other cases could go here too. 

    return retval;
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::NewInstance
//
// Purpose: 
//   NewInstance method for the PeaksOverThresholdAttributes class.
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
PeaksOverThresholdAttributes::NewInstance(bool copy) const
{
    AttributeSubject *retval = 0;
    if(copy)
        retval = new PeaksOverThresholdAttributes(*this);
    else
        retval = new PeaksOverThresholdAttributes;

    return retval;
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::SelectAll
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
PeaksOverThresholdAttributes::SelectAll()
{
    Select(ID_dataYearBegin,                (void *)&dataYearBegin);
    Select(ID_dataAnalysisYearRangeEnabled, (void *)&dataAnalysisYearRangeEnabled);
    Select(ID_dataAnalysisYearRange,        (void *)dataAnalysisYearRange, 2);
    Select(ID_aggregation,                  (void *)&aggregation);
    Select(ID_annualPercentile,             (void *)&annualPercentile);
    Select(ID_seasonalPercentile,           (void *)seasonalPercentile, 4);
    Select(ID_monthlyPercentile,            (void *)monthlyPercentile, 12);
    Select(ID_displaySeason,                (void *)&displaySeason);
    Select(ID_displayMonth,                 (void *)&displayMonth);
    Select(ID_cutoff,                       (void *)&cutoff);
    Select(ID_computeParamValues,           (void *)&computeParamValues);
    Select(ID_computeCovariates,            (void *)&computeCovariates);
    Select(ID_covariateReturnYears,         (void *)&covariateReturnYears);
    Select(ID_covariateModelLocation,       (void *)&covariateModelLocation);
    Select(ID_covariateModelShape,          (void *)&covariateModelShape);
    Select(ID_covariateModelScale,          (void *)&covariateModelScale);
    Select(ID_computeRVDifferences,         (void *)&computeRVDifferences);
    Select(ID_rvDifferences,                (void *)rvDifferences, 2);
    Select(ID_dataScaling,                  (void *)&dataScaling);
    Select(ID_dumpData,                     (void *)&dumpData);
}

///////////////////////////////////////////////////////////////////////////////
// Persistence methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::CreateNode
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
PeaksOverThresholdAttributes::CreateNode(DataNode *parentNode, bool completeSave, bool forceAdd)
{
    if(parentNode == 0)
        return false;

    PeaksOverThresholdAttributes defaultObject;
    bool addToParent = false;
    // Create a node for PeaksOverThresholdAttributes.
    DataNode *node = new DataNode("PeaksOverThresholdAttributes");

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

    if(completeSave || !FieldsEqual(ID_dataAnalysisYearRange, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataAnalysisYearRange", dataAnalysisYearRange, 2));
    }

    if(completeSave || !FieldsEqual(ID_aggregation, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("aggregation", AggregationType_ToString(aggregation)));
    }

    if(completeSave || !FieldsEqual(ID_annualPercentile, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("annualPercentile", annualPercentile));
    }

    if(completeSave || !FieldsEqual(ID_seasonalPercentile, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("seasonalPercentile", seasonalPercentile, 4));
    }

    if(completeSave || !FieldsEqual(ID_monthlyPercentile, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("monthlyPercentile", monthlyPercentile, 12));
    }

    if(completeSave || !FieldsEqual(ID_displaySeason, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displaySeason", SeasonType_ToString(displaySeason)));
    }

    if(completeSave || !FieldsEqual(ID_displayMonth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("displayMonth", MonthType_ToString(displayMonth)));
    }

    if(completeSave || !FieldsEqual(ID_dataYearBegin, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataYearBegin", dataYearBegin));
    }

    if(completeSave || !FieldsEqual(ID_cutoff, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cutoff", cutoff));
    }

    if(completeSave || !FieldsEqual(ID_computeParamValues, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("computeParamValues", computeParamValues));
    }

    if(completeSave || !FieldsEqual(ID_computeCovariates, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("computeCovariates", computeCovariates));
    }

    if(completeSave || !FieldsEqual(ID_covariateReturnYears, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("covariateReturnYears", covariateReturnYears));
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

    if(completeSave || !FieldsEqual(ID_covariateModelScale, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("covariateModelScale", covariateModelScale));
    }

    if(completeSave || !FieldsEqual(ID_computeRVDifferences, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("computeRVDifferences", computeRVDifferences));
    }

    if(completeSave || !FieldsEqual(ID_rvDifferences, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("rvDifferences", rvDifferences, 2));
    }

    if(completeSave || !FieldsEqual(ID_dataScaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataScaling", dataScaling));
    }

    if(completeSave || !FieldsEqual(ID_dumpData, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dumpData", dumpData));
    }


    // Add the node to the parent node.
    if(addToParent || forceAdd)
        parentNode->AddNode(node);
    else
        delete node;

    return (addToParent || forceAdd);
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::SetFromNode
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
PeaksOverThresholdAttributes::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("PeaksOverThresholdAttributes");
    if(searchNode == 0)
        return;

    DataNode *node;
    if((node = searchNode->GetNode("dataYearBegin")) != 0)
        SetDataYearBegin(node->AsInt());
    if((node = searchNode->GetNode("dataAnalysisYearRangeEnabled")) != 0)
        SetDataAnalysisYearRangeEnabled(node->AsBool());
    if((node = searchNode->GetNode("dataAnalysisYearRange")) != 0)
        SetDataAnalysisYearRange(node->AsIntArray());
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
    if((node = searchNode->GetNode("annualPercentile")) != 0)
        SetAnnualPercentile(node->AsDouble());
    if((node = searchNode->GetNode("seasonalPercentile")) != 0)
        SetSeasonalPercentile(node->AsDoubleArray());
    if((node = searchNode->GetNode("monthlyPercentile")) != 0)
        SetMonthlyPercentile(node->AsDoubleArray());
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
    if((node = searchNode->GetNode("dataYearBegin")) != 0)
        SetDataYearBegin(node->AsInt());
    if((node = searchNode->GetNode("cutoff")) != 0)
        SetCutoff(node->AsFloat());
    if((node = searchNode->GetNode("computeParamValues")) != 0)
        SetComputeParamValues(node->AsBool());
    if((node = searchNode->GetNode("computeCovariates")) != 0)
        SetComputeCovariates(node->AsBool());
    if((node = searchNode->GetNode("covariateReturnYears")) != 0)
        SetCovariateReturnYears(node->AsIntVector());
    if((node = searchNode->GetNode("covariateModelLocation")) != 0)
        SetCovariateModelLocation(node->AsBool());
    if((node = searchNode->GetNode("covariateModelShape")) != 0)
        SetCovariateModelShape(node->AsBool());
    if((node = searchNode->GetNode("covariateModelScale")) != 0)
        SetCovariateModelScale(node->AsBool());
    if((node = searchNode->GetNode("computeRVDifferences")) != 0)
        SetComputeRVDifferences(node->AsBool());
    if((node = searchNode->GetNode("rvDifferences")) != 0)
        SetRvDifferences(node->AsIntArray());
    if((node = searchNode->GetNode("dataScaling")) != 0)
        SetDataScaling(node->AsDouble());
    if((node = searchNode->GetNode("dumpData")) != 0)
        SetDumpData(node->AsBool());
}

///////////////////////////////////////////////////////////////////////////////
// Set property methods
///////////////////////////////////////////////////////////////////////////////

void
PeaksOverThresholdAttributes::SetDataYearBegin(int dataYearBegin_)
{
    dataYearBegin = dataYearBegin_;
    Select(ID_dataYearBegin, (void *)&dataYearBegin);
}

void
PeaksOverThresholdAttributes::SetDataAnalysisYearRangeEnabled(bool dataAnalysisYearRangeEnabled_)
{
    dataAnalysisYearRangeEnabled = dataAnalysisYearRangeEnabled_;
    Select(ID_dataAnalysisYearRangeEnabled, (void *)&dataAnalysisYearRangeEnabled);
}

void
PeaksOverThresholdAttributes::SetDataAnalysisYearRange(const int *dataAnalysisYearRange_)
{
    dataAnalysisYearRange[0] = dataAnalysisYearRange_[0];
    dataAnalysisYearRange[1] = dataAnalysisYearRange_[1];
    Select(ID_dataAnalysisYearRange, (void *)dataAnalysisYearRange, 2);
}

void
PeaksOverThresholdAttributes::SetAggregation(PeaksOverThresholdAttributes::AggregationType aggregation_)
{
    aggregation = aggregation_;
    Select(ID_aggregation, (void *)&aggregation);
}

void
PeaksOverThresholdAttributes::SetAnnualPercentile(double annualPercentile_)
{
    annualPercentile = annualPercentile_;
    Select(ID_annualPercentile, (void *)&annualPercentile);
}

void
PeaksOverThresholdAttributes::SetSeasonalPercentile(const double *seasonalPercentile_)
{
    seasonalPercentile[0] = seasonalPercentile_[0];
    seasonalPercentile[1] = seasonalPercentile_[1];
    seasonalPercentile[2] = seasonalPercentile_[2];
    seasonalPercentile[3] = seasonalPercentile_[3];
    Select(ID_seasonalPercentile, (void *)seasonalPercentile, 4);
}

void
PeaksOverThresholdAttributes::SetMonthlyPercentile(const double *monthlyPercentile_)
{
    for(int i = 0; i < 12; ++i)
        monthlyPercentile[i] = monthlyPercentile_[i];
    Select(ID_monthlyPercentile, (void *)monthlyPercentile, 12);
}

void
PeaksOverThresholdAttributes::SetDisplaySeason(PeaksOverThresholdAttributes::SeasonType displaySeason_)
{
    displaySeason = displaySeason_;
    Select(ID_displaySeason, (void *)&displaySeason);
}

void
PeaksOverThresholdAttributes::SetDisplayMonth(PeaksOverThresholdAttributes::MonthType displayMonth_)
{
    displayMonth = displayMonth_;
    Select(ID_displayMonth, (void *)&displayMonth);
}

void
PeaksOverThresholdAttributes::SetDataYearBegin(int dataYearBegin_)
{
    dataYearBegin = dataYearBegin_;
    Select(ID_dataYearBegin, (void *)&dataYearBegin);
}

void
PeaksOverThresholdAttributes::SetCutoff(float cutoff_)
{
    cutoff = cutoff_;
    Select(ID_cutoff, (void *)&cutoff);
}

void
PeaksOverThresholdAttributes::SetComputeParamValues(bool computeParamValues_)
{
    computeParamValues = computeParamValues_;
    Select(ID_computeParamValues, (void *)&computeParamValues);
}

void
PeaksOverThresholdAttributes::SetComputeCovariates(bool computeCovariates_)
{
    computeCovariates = computeCovariates_;
    Select(ID_computeCovariates, (void *)&computeCovariates);
}

void
PeaksOverThresholdAttributes::SetCovariateReturnYears(const intVector &covariateReturnYears_)
{
    covariateReturnYears = covariateReturnYears_;
    Select(ID_covariateReturnYears, (void *)&covariateReturnYears);
}

void
PeaksOverThresholdAttributes::SetCovariateModelLocation(bool covariateModelLocation_)
{
    covariateModelLocation = covariateModelLocation_;
    Select(ID_covariateModelLocation, (void *)&covariateModelLocation);
}

void
PeaksOverThresholdAttributes::SetCovariateModelShape(bool covariateModelShape_)
{
    covariateModelShape = covariateModelShape_;
    Select(ID_covariateModelShape, (void *)&covariateModelShape);
}

void
PeaksOverThresholdAttributes::SetCovariateModelScale(bool covariateModelScale_)
{
    covariateModelScale = covariateModelScale_;
    Select(ID_covariateModelScale, (void *)&covariateModelScale);
}

void
PeaksOverThresholdAttributes::SetComputeRVDifferences(bool computeRVDifferences_)
{
    computeRVDifferences = computeRVDifferences_;
    Select(ID_computeRVDifferences, (void *)&computeRVDifferences);
}

void
PeaksOverThresholdAttributes::SetRvDifferences(const int *rvDifferences_)
{
    rvDifferences[0] = rvDifferences_[0];
    rvDifferences[1] = rvDifferences_[1];
    Select(ID_rvDifferences, (void *)rvDifferences, 2);
}

void
PeaksOverThresholdAttributes::SetDataScaling(double dataScaling_)
{
    dataScaling = dataScaling_;
    Select(ID_dataScaling, (void *)&dataScaling);
}

void
PeaksOverThresholdAttributes::SetDumpData(bool dumpData_)
{
    dumpData = dumpData_;
    Select(ID_dumpData, (void *)&dumpData);
}

///////////////////////////////////////////////////////////////////////////////
// Get property methods
///////////////////////////////////////////////////////////////////////////////

int
PeaksOverThresholdAttributes::GetDataYearBegin() const
{
    return dataYearBegin;
}

bool
PeaksOverThresholdAttributes::GetDataAnalysisYearRangeEnabled() const
{
    return dataAnalysisYearRangeEnabled;
}

const int *
PeaksOverThresholdAttributes::GetDataAnalysisYearRange() const
{
    return dataAnalysisYearRange;
}

int *
PeaksOverThresholdAttributes::GetDataAnalysisYearRange()
{
    return dataAnalysisYearRange;
}

PeaksOverThresholdAttributes::AggregationType
PeaksOverThresholdAttributes::GetAggregation() const
{
    return AggregationType(aggregation);
}

double
PeaksOverThresholdAttributes::GetAnnualPercentile() const
{
    return annualPercentile;
}

const double *
PeaksOverThresholdAttributes::GetSeasonalPercentile() const
{
    return seasonalPercentile;
}

double *
PeaksOverThresholdAttributes::GetSeasonalPercentile()
{
    return seasonalPercentile;
}

const double *
PeaksOverThresholdAttributes::GetMonthlyPercentile() const
{
    return monthlyPercentile;
}

double *
PeaksOverThresholdAttributes::GetMonthlyPercentile()
{
    return monthlyPercentile;
}

PeaksOverThresholdAttributes::SeasonType
PeaksOverThresholdAttributes::GetDisplaySeason() const
{
    return SeasonType(displaySeason);
}

PeaksOverThresholdAttributes::MonthType
PeaksOverThresholdAttributes::GetDisplayMonth() const
{
    return MonthType(displayMonth);
}

int
PeaksOverThresholdAttributes::GetDataYearBegin() const
{
    return dataYearBegin;
}

float
PeaksOverThresholdAttributes::GetCutoff() const
{
    return cutoff;
}

bool
PeaksOverThresholdAttributes::GetComputeParamValues() const
{
    return computeParamValues;
}

bool
PeaksOverThresholdAttributes::GetComputeCovariates() const
{
    return computeCovariates;
}

const intVector &
PeaksOverThresholdAttributes::GetCovariateReturnYears() const
{
    return covariateReturnYears;
}

intVector &
PeaksOverThresholdAttributes::GetCovariateReturnYears()
{
    return covariateReturnYears;
}

bool
PeaksOverThresholdAttributes::GetCovariateModelLocation() const
{
    return covariateModelLocation;
}

bool
PeaksOverThresholdAttributes::GetCovariateModelShape() const
{
    return covariateModelShape;
}

bool
PeaksOverThresholdAttributes::GetCovariateModelScale() const
{
    return covariateModelScale;
}

bool
PeaksOverThresholdAttributes::GetComputeRVDifferences() const
{
    return computeRVDifferences;
}

const int *
PeaksOverThresholdAttributes::GetRvDifferences() const
{
    return rvDifferences;
}

int *
PeaksOverThresholdAttributes::GetRvDifferences()
{
    return rvDifferences;
}

double
PeaksOverThresholdAttributes::GetDataScaling() const
{
    return dataScaling;
}

bool
PeaksOverThresholdAttributes::GetDumpData() const
{
    return dumpData;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

void
PeaksOverThresholdAttributes::SelectDataAnalysisYearRange()
{
    Select(ID_dataAnalysisYearRange, (void *)dataAnalysisYearRange, 2);
}

void
PeaksOverThresholdAttributes::SelectSeasonalPercentile()
{
    Select(ID_seasonalPercentile, (void *)seasonalPercentile, 4);
}

void
PeaksOverThresholdAttributes::SelectMonthlyPercentile()
{
    Select(ID_monthlyPercentile, (void *)monthlyPercentile, 12);
}

void
PeaksOverThresholdAttributes::SelectCovariateReturnYears()
{
    Select(ID_covariateReturnYears, (void *)&covariateReturnYears);
}

void
PeaksOverThresholdAttributes::SelectRvDifferences()
{
    Select(ID_rvDifferences, (void *)rvDifferences, 2);
}

///////////////////////////////////////////////////////////////////////////////
// Keyframing methods
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::GetFieldName
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
PeaksOverThresholdAttributes::GetFieldName(int index) const
{
    switch (index)
    {
    case ID_dataYearBegin:                return "dataYearBegin";
    case ID_dataAnalysisYearRangeEnabled: return "dataAnalysisYearRangeEnabled";
    case ID_dataAnalysisYearRange:        return "dataAnalysisYearRange";
    case ID_aggregation:                  return "aggregation";
    case ID_annualPercentile:             return "annualPercentile";
    case ID_seasonalPercentile:           return "seasonalPercentile";
    case ID_monthlyPercentile:            return "monthlyPercentile";
    case ID_displaySeason:                return "displaySeason";
    case ID_displayMonth:                 return "displayMonth";
    case ID_cutoff:                       return "cutoff";
    case ID_computeParamValues:           return "computeParamValues";
    case ID_computeCovariates:            return "computeCovariates";
    case ID_covariateReturnYears:         return "covariateReturnYears";
    case ID_covariateModelLocation:       return "covariateModelLocation";
    case ID_covariateModelShape:          return "covariateModelShape";
    case ID_covariateModelScale:          return "covariateModelScale";
    case ID_computeRVDifferences:         return "computeRVDifferences";
    case ID_rvDifferences:                return "rvDifferences";
    case ID_dataScaling:                  return "dataScaling";
    case ID_dumpData:                     return "dumpData";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::GetFieldType
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
PeaksOverThresholdAttributes::GetFieldType(int index) const
{
    switch (index)
    {
    case ID_dataYearBegin:                return FieldType_int;
    case ID_dataAnalysisYearRangeEnabled: return FieldType_bool;
    case ID_dataAnalysisYearRange:        return FieldType_intArray;
    case ID_aggregation:                  return FieldType_enum;
    case ID_annualPercentile:             return FieldType_double;
    case ID_seasonalPercentile:           return FieldType_doubleArray;
    case ID_monthlyPercentile:            return FieldType_doubleArray;
    case ID_displaySeason:                return FieldType_enum;
    case ID_displayMonth:                 return FieldType_enum;
    case ID_cutoff:                       return FieldType_float;
    case ID_computeParamValues:           return FieldType_bool;
    case ID_computeCovariates:            return FieldType_bool;
    case ID_covariateReturnYears:         return FieldType_intVector;
    case ID_covariateModelLocation:       return FieldType_bool;
    case ID_covariateModelShape:          return FieldType_bool;
    case ID_covariateModelScale:          return FieldType_bool;
    case ID_computeRVDifferences:         return FieldType_bool;
    case ID_rvDifferences:                return FieldType_intArray;
    case ID_dataScaling:                  return FieldType_double;
    case ID_dumpData:                     return FieldType_bool;
    default:  return FieldType_unknown;
    }
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::GetFieldTypeName
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
PeaksOverThresholdAttributes::GetFieldTypeName(int index) const
{
    switch (index)
    {
    case ID_dataYearBegin:                return "int";
    case ID_dataAnalysisYearRangeEnabled: return "bool";
    case ID_dataAnalysisYearRange:        return "intArray";
    case ID_aggregation:                  return "enum";
    case ID_annualPercentile:             return "double";
    case ID_seasonalPercentile:           return "doubleArray";
    case ID_monthlyPercentile:            return "doubleArray";
    case ID_displaySeason:                return "enum";
    case ID_displayMonth:                 return "enum";
    case ID_cutoff:                       return "float";
    case ID_computeParamValues:           return "bool";
    case ID_computeCovariates:            return "bool";
    case ID_covariateReturnYears:         return "intVector";
    case ID_covariateModelLocation:       return "bool";
    case ID_covariateModelShape:          return "bool";
    case ID_covariateModelScale:          return "bool";
    case ID_computeRVDifferences:         return "bool";
    case ID_rvDifferences:                return "intArray";
    case ID_dataScaling:                  return "double";
    case ID_dumpData:                     return "bool";
    default:  return "invalid index";
    }
}

// ****************************************************************************
// Method: PeaksOverThresholdAttributes::FieldsEqual
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
PeaksOverThresholdAttributes::FieldsEqual(int index_, const AttributeGroup *rhs) const
{
    const PeaksOverThresholdAttributes &obj = *((const PeaksOverThresholdAttributes*)rhs);
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
    case ID_dataAnalysisYearRange:
        {  // new scope
        // Compare the dataAnalysisYearRange arrays.
        bool dataAnalysisYearRange_equal = true;
        for(int i = 0; i < 2 && dataAnalysisYearRange_equal; ++i)
            dataAnalysisYearRange_equal = (dataAnalysisYearRange[i] == obj.dataAnalysisYearRange[i]);

        retval = dataAnalysisYearRange_equal;
        }
        break;
    case ID_aggregation:
        {  // new scope
        retval = (aggregation == obj.aggregation);
        }
        break;
    case ID_annualPercentile:
        {  // new scope
        retval = (annualPercentile == obj.annualPercentile);
        }
        break;
    case ID_seasonalPercentile:
        {  // new scope
        // Compare the seasonalPercentile arrays.
        bool seasonalPercentile_equal = true;
        for(int i = 0; i < 4 && seasonalPercentile_equal; ++i)
            seasonalPercentile_equal = (seasonalPercentile[i] == obj.seasonalPercentile[i]);

        retval = seasonalPercentile_equal;
        }
        break;
    case ID_monthlyPercentile:
        {  // new scope
        // Compare the monthlyPercentile arrays.
        bool monthlyPercentile_equal = true;
        for(int i = 0; i < 12 && monthlyPercentile_equal; ++i)
            monthlyPercentile_equal = (monthlyPercentile[i] == obj.monthlyPercentile[i]);

        retval = monthlyPercentile_equal;
        }
        break;
    case ID_displaySeason:
        {  // new scope
        retval = (displaySeason == obj.displaySeason);
        }
        break;
    case ID_displayMonth:
        {  // new scope
        retval = (displayMonth == obj.displayMonth);
        }
        break;
    case ID_dataYearBegin:
        {  // new scope
        retval = (dataYearBegin == obj.dataYearBegin);
        }
        break;
    case ID_cutoff:
        {  // new scope
        retval = (cutoff == obj.cutoff);
        }
        break;
    case ID_computeParamValues:
        {  // new scope
        retval = (computeParamValues == obj.computeParamValues);
        }
        break;
    case ID_computeCovariates:
        {  // new scope
        retval = (computeCovariates == obj.computeCovariates);
        }
        break;
    case ID_covariateReturnYears:
        {  // new scope
        retval = (covariateReturnYears == obj.covariateReturnYears);
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
    case ID_covariateModelScale:
        {  // new scope
        retval = (covariateModelScale == obj.covariateModelScale);
        }
        break;
    case ID_computeRVDifferences:
        {  // new scope
        retval = (computeRVDifferences == obj.computeRVDifferences);
        }
        break;
    case ID_rvDifferences:
        {  // new scope
        // Compare the rvDifferences arrays.
        bool rvDifferences_equal = true;
        for(int i = 0; i < 2 && rvDifferences_equal; ++i)
            rvDifferences_equal = (rvDifferences[i] == obj.rvDifferences[i]);

        retval = rvDifferences_equal;
        }
        break;
    case ID_dataScaling:
        {  // new scope
        retval = (dataScaling == obj.dataScaling);
        }
        break;
    case ID_dumpData:
        {  // new scope
        retval = (dumpData == obj.dumpData);
        }
        break;
    default: retval = false;
    }

    return retval;
}

///////////////////////////////////////////////////////////////////////////////
// User-defined methods.
///////////////////////////////////////////////////////////////////////////////

