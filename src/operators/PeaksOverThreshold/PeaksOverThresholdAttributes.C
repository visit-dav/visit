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

//
// Enum conversion methods for PeaksOverThresholdAttributes::CutoffModeType
//

static const char *CutoffModeType_strings[] = {
"UPPER_TAIL", "LOWER_TAIL"};

std::string
PeaksOverThresholdAttributes::CutoffModeType_ToString(PeaksOverThresholdAttributes::CutoffModeType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return CutoffModeType_strings[index];
}

std::string
PeaksOverThresholdAttributes::CutoffModeType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return CutoffModeType_strings[index];
}

bool
PeaksOverThresholdAttributes::CutoffModeType_FromString(const std::string &s, PeaksOverThresholdAttributes::CutoffModeType &val)
{
    val = PeaksOverThresholdAttributes::UPPER_TAIL;
    for(int i = 0; i < 2; ++i)
    {
        if(s == CutoffModeType_strings[i])
        {
            val = (CutoffModeType)i;
            return true;
        }
    }
    return false;
}

//
// Enum conversion methods for PeaksOverThresholdAttributes::OptimizationType
//

static const char *OptimizationType_strings[] = {
"NELDER_MEAD", "BFGS"};

std::string
PeaksOverThresholdAttributes::OptimizationType_ToString(PeaksOverThresholdAttributes::OptimizationType t)
{
    int index = int(t);
    if(index < 0 || index >= 2) index = 0;
    return OptimizationType_strings[index];
}

std::string
PeaksOverThresholdAttributes::OptimizationType_ToString(int t)
{
    int index = (t < 0 || t >= 2) ? 0 : t;
    return OptimizationType_strings[index];
}

bool
PeaksOverThresholdAttributes::OptimizationType_FromString(const std::string &s, PeaksOverThresholdAttributes::OptimizationType &val)
{
    val = PeaksOverThresholdAttributes::NELDER_MEAD;
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
    dataAnalysisYear1 = 0;
    dataAnalysisYear2 = 0;
    ensemble = false;
    numEnsembles = 1;
    cutoff = 0;
    cutoffMode = UPPER_TAIL;
    noConsecutiveDay = false;
    optimizationMethod = NELDER_MEAD;
    dataScaling = 1;
    aggregation = ANNUAL;
    annualPercentile = 0.95;
    seasonalPercentile[0] = 0.95;
    seasonalPercentile[1] = 0.95;
    seasonalPercentile[2] = 0.95;
    seasonalPercentile[3] = 0.95;
    monthlyPercentile[0] = 0.95;
    monthlyPercentile[1] = 0.95;
    monthlyPercentile[2] = 0.95;
    monthlyPercentile[3] = 0.95;
    monthlyPercentile[4] = 0.95;
    monthlyPercentile[5] = 0.95;
    monthlyPercentile[6] = 0.95;
    monthlyPercentile[7] = 0.95;
    monthlyPercentile[8] = 0.95;
    monthlyPercentile[9] = 0.95;
    monthlyPercentile[10] = 0.95;
    monthlyPercentile[11] = 0.95;
    daysPerYear = 365;
    daysPerMonth[0] = 31;
    daysPerMonth[1] = 28;
    daysPerMonth[2] = 31;
    daysPerMonth[3] = 30;
    daysPerMonth[4] = 31;
    daysPerMonth[5] = 30;
    daysPerMonth[6] = 31;
    daysPerMonth[7] = 31;
    daysPerMonth[8] = 30;
    daysPerMonth[9] = 31;
    daysPerMonth[10] = 30;
    daysPerMonth[11] = 31;
    covariateModelScale = false;
    covariateModelLocation = false;
    covariateModelShape = false;
    computeCovariates = false;
    covariateReturnYears.push_back(1);
    computeRVDifferences = false;
    rvDifference1 = 0;
    rvDifference2 = 0;
    computeParamValues = false;
    displaySeason = WINTER;
    displayMonth = JAN;
    dumpData = true;
    dumpDebug = false;

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
    dataAnalysisYear1 = obj.dataAnalysisYear1;
    dataAnalysisYear2 = obj.dataAnalysisYear2;
    ensemble = obj.ensemble;
    numEnsembles = obj.numEnsembles;
    cutoff = obj.cutoff;
    cutoffMode = obj.cutoffMode;
    noConsecutiveDay = obj.noConsecutiveDay;
    optimizationMethod = obj.optimizationMethod;
    dataScaling = obj.dataScaling;
    aggregation = obj.aggregation;
    annualPercentile = obj.annualPercentile;
    for(int i = 0; i < 4; ++i)
        seasonalPercentile[i] = obj.seasonalPercentile[i];

    for(int i = 0; i < 12; ++i)
        monthlyPercentile[i] = obj.monthlyPercentile[i];

    daysPerYear = obj.daysPerYear;
    for(int i = 0; i < 12; ++i)
        daysPerMonth[i] = obj.daysPerMonth[i];

    covariateModelScale = obj.covariateModelScale;
    covariateModelLocation = obj.covariateModelLocation;
    covariateModelShape = obj.covariateModelShape;
    computeCovariates = obj.computeCovariates;
    covariateReturnYears = obj.covariateReturnYears;
    computeRVDifferences = obj.computeRVDifferences;
    rvDifference1 = obj.rvDifference1;
    rvDifference2 = obj.rvDifference2;
    computeParamValues = obj.computeParamValues;
    displaySeason = obj.displaySeason;
    displayMonth = obj.displayMonth;
    dumpData = obj.dumpData;
    dumpDebug = obj.dumpDebug;

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
    // Compare the seasonalPercentile arrays.
    bool seasonalPercentile_equal = true;
    for(int i = 0; i < 4 && seasonalPercentile_equal; ++i)
        seasonalPercentile_equal = (seasonalPercentile[i] == obj.seasonalPercentile[i]);

    // Compare the monthlyPercentile arrays.
    bool monthlyPercentile_equal = true;
    for(int i = 0; i < 12 && monthlyPercentile_equal; ++i)
        monthlyPercentile_equal = (monthlyPercentile[i] == obj.monthlyPercentile[i]);

    // Compare the daysPerMonth arrays.
    bool daysPerMonth_equal = true;
    for(int i = 0; i < 12 && daysPerMonth_equal; ++i)
        daysPerMonth_equal = (daysPerMonth[i] == obj.daysPerMonth[i]);

    // Create the return value
    return ((dataYearBegin == obj.dataYearBegin) &&
            (dataAnalysisYearRangeEnabled == obj.dataAnalysisYearRangeEnabled) &&
            (dataAnalysisYear1 == obj.dataAnalysisYear1) &&
            (dataAnalysisYear2 == obj.dataAnalysisYear2) &&
            (ensemble == obj.ensemble) &&
            (numEnsembles == obj.numEnsembles) &&
            (cutoff == obj.cutoff) &&
            (cutoffMode == obj.cutoffMode) &&
            (noConsecutiveDay == obj.noConsecutiveDay) &&
            (optimizationMethod == obj.optimizationMethod) &&
            (dataScaling == obj.dataScaling) &&
            (aggregation == obj.aggregation) &&
            (annualPercentile == obj.annualPercentile) &&
            seasonalPercentile_equal &&
            monthlyPercentile_equal &&
            (daysPerYear == obj.daysPerYear) &&
            daysPerMonth_equal &&
            (covariateModelScale == obj.covariateModelScale) &&
            (covariateModelLocation == obj.covariateModelLocation) &&
            (covariateModelShape == obj.covariateModelShape) &&
            (computeCovariates == obj.computeCovariates) &&
            (covariateReturnYears == obj.covariateReturnYears) &&
            (computeRVDifferences == obj.computeRVDifferences) &&
            (rvDifference1 == obj.rvDifference1) &&
            (rvDifference2 == obj.rvDifference2) &&
            (computeParamValues == obj.computeParamValues) &&
            (displaySeason == obj.displaySeason) &&
            (displayMonth == obj.displayMonth) &&
            (dumpData == obj.dumpData) &&
            (dumpDebug == obj.dumpDebug));
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
    Select(ID_dataAnalysisYear1,            (void *)&dataAnalysisYear1);
    Select(ID_dataAnalysisYear2,            (void *)&dataAnalysisYear2);
    Select(ID_ensemble,                     (void *)&ensemble);
    Select(ID_numEnsembles,                 (void *)&numEnsembles);
    Select(ID_cutoff,                       (void *)&cutoff);
    Select(ID_cutoffMode,                   (void *)&cutoffMode);
    Select(ID_noConsecutiveDay,             (void *)&noConsecutiveDay);
    Select(ID_optimizationMethod,           (void *)&optimizationMethod);
    Select(ID_dataScaling,                  (void *)&dataScaling);
    Select(ID_aggregation,                  (void *)&aggregation);
    Select(ID_annualPercentile,             (void *)&annualPercentile);
    Select(ID_seasonalPercentile,           (void *)seasonalPercentile, 4);
    Select(ID_monthlyPercentile,            (void *)monthlyPercentile, 12);
    Select(ID_daysPerYear,                  (void *)&daysPerYear);
    Select(ID_daysPerMonth,                 (void *)daysPerMonth, 12);
    Select(ID_covariateModelScale,          (void *)&covariateModelScale);
    Select(ID_covariateModelLocation,       (void *)&covariateModelLocation);
    Select(ID_covariateModelShape,          (void *)&covariateModelShape);
    Select(ID_computeCovariates,            (void *)&computeCovariates);
    Select(ID_covariateReturnYears,         (void *)&covariateReturnYears);
    Select(ID_computeRVDifferences,         (void *)&computeRVDifferences);
    Select(ID_rvDifference1,                (void *)&rvDifference1);
    Select(ID_rvDifference2,                (void *)&rvDifference2);
    Select(ID_computeParamValues,           (void *)&computeParamValues);
    Select(ID_displaySeason,                (void *)&displaySeason);
    Select(ID_displayMonth,                 (void *)&displayMonth);
    Select(ID_dumpData,                     (void *)&dumpData);
    Select(ID_dumpDebug,                    (void *)&dumpDebug);
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

    if(completeSave || !FieldsEqual(ID_cutoff, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cutoff", cutoff));
    }

    if(completeSave || !FieldsEqual(ID_cutoffMode, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("cutoffMode", CutoffModeType_ToString(cutoffMode)));
    }

    if(completeSave || !FieldsEqual(ID_noConsecutiveDay, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("noConsecutiveDay", noConsecutiveDay));
    }

    if(completeSave || !FieldsEqual(ID_optimizationMethod, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("optimizationMethod", OptimizationType_ToString(optimizationMethod)));
    }

    if(completeSave || !FieldsEqual(ID_dataScaling, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("dataScaling", dataScaling));
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

    if(completeSave || !FieldsEqual(ID_daysPerYear, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("daysPerYear", daysPerYear));
    }

    if(completeSave || !FieldsEqual(ID_daysPerMonth, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("daysPerMonth", daysPerMonth, 12));
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

    if(completeSave || !FieldsEqual(ID_computeParamValues, &defaultObject))
    {
        addToParent = true;
        node->AddNode(new DataNode("computeParamValues", computeParamValues));
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
    if((node = searchNode->GetNode("dataAnalysisYear1")) != 0)
        SetDataAnalysisYear1(node->AsInt());
    if((node = searchNode->GetNode("dataAnalysisYear2")) != 0)
        SetDataAnalysisYear2(node->AsInt());
    if((node = searchNode->GetNode("ensemble")) != 0)
        SetEnsemble(node->AsBool());
    if((node = searchNode->GetNode("numEnsembles")) != 0)
        SetNumEnsembles(node->AsInt());
    if((node = searchNode->GetNode("cutoff")) != 0)
        SetCutoff(node->AsFloat());
    if((node = searchNode->GetNode("cutoffMode")) != 0)
    {
        // Allow enums to be int or string in the config file
        if(node->GetNodeType() == INT_NODE)
        {
            int ival = node->AsInt();
            if(ival >= 0 && ival < 2)
                SetCutoffMode(CutoffModeType(ival));
        }
        else if(node->GetNodeType() == STRING_NODE)
        {
            CutoffModeType value;
            if(CutoffModeType_FromString(node->AsString(), value))
                SetCutoffMode(value);
        }
    }
    if((node = searchNode->GetNode("noConsecutiveDay")) != 0)
        SetNoConsecutiveDay(node->AsBool());
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
    if((node = searchNode->GetNode("dataScaling")) != 0)
        SetDataScaling(node->AsDouble());
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
    if((node = searchNode->GetNode("daysPerYear")) != 0)
        SetDaysPerYear(node->AsInt());
    if((node = searchNode->GetNode("daysPerMonth")) != 0)
        SetDaysPerMonth(node->AsIntArray());
    if((node = searchNode->GetNode("covariateModelScale")) != 0)
        SetCovariateModelScale(node->AsBool());
    if((node = searchNode->GetNode("covariateModelLocation")) != 0)
        SetCovariateModelLocation(node->AsBool());
    if((node = searchNode->GetNode("covariateModelShape")) != 0)
        SetCovariateModelShape(node->AsBool());
    if((node = searchNode->GetNode("computeCovariates")) != 0)
        SetComputeCovariates(node->AsBool());
    if((node = searchNode->GetNode("covariateReturnYears")) != 0)
        SetCovariateReturnYears(node->AsIntVector());
    if((node = searchNode->GetNode("computeRVDifferences")) != 0)
        SetComputeRVDifferences(node->AsBool());
    if((node = searchNode->GetNode("rvDifference1")) != 0)
        SetRvDifference1(node->AsInt());
    if((node = searchNode->GetNode("rvDifference2")) != 0)
        SetRvDifference2(node->AsInt());
    if((node = searchNode->GetNode("computeParamValues")) != 0)
        SetComputeParamValues(node->AsBool());
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
    if((node = searchNode->GetNode("dumpData")) != 0)
        SetDumpData(node->AsBool());
    if((node = searchNode->GetNode("dumpDebug")) != 0)
        SetDumpDebug(node->AsBool());
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
PeaksOverThresholdAttributes::SetDataAnalysisYear1(int dataAnalysisYear1_)
{
    dataAnalysisYear1 = dataAnalysisYear1_;
    Select(ID_dataAnalysisYear1, (void *)&dataAnalysisYear1);
}

void
PeaksOverThresholdAttributes::SetDataAnalysisYear2(int dataAnalysisYear2_)
{
    dataAnalysisYear2 = dataAnalysisYear2_;
    Select(ID_dataAnalysisYear2, (void *)&dataAnalysisYear2);
}

void
PeaksOverThresholdAttributes::SetEnsemble(bool ensemble_)
{
    ensemble = ensemble_;
    Select(ID_ensemble, (void *)&ensemble);
}

void
PeaksOverThresholdAttributes::SetNumEnsembles(int numEnsembles_)
{
    numEnsembles = numEnsembles_;
    Select(ID_numEnsembles, (void *)&numEnsembles);
}

void
PeaksOverThresholdAttributes::SetCutoff(float cutoff_)
{
    cutoff = cutoff_;
    Select(ID_cutoff, (void *)&cutoff);
}

void
PeaksOverThresholdAttributes::SetCutoffMode(PeaksOverThresholdAttributes::CutoffModeType cutoffMode_)
{
    cutoffMode = cutoffMode_;
    Select(ID_cutoffMode, (void *)&cutoffMode);
}

void
PeaksOverThresholdAttributes::SetNoConsecutiveDay(bool noConsecutiveDay_)
{
    noConsecutiveDay = noConsecutiveDay_;
    Select(ID_noConsecutiveDay, (void *)&noConsecutiveDay);
}

void
PeaksOverThresholdAttributes::SetOptimizationMethod(PeaksOverThresholdAttributes::OptimizationType optimizationMethod_)
{
    optimizationMethod = optimizationMethod_;
    Select(ID_optimizationMethod, (void *)&optimizationMethod);
}

void
PeaksOverThresholdAttributes::SetDataScaling(double dataScaling_)
{
    dataScaling = dataScaling_;
    Select(ID_dataScaling, (void *)&dataScaling);
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
PeaksOverThresholdAttributes::SetDaysPerYear(int daysPerYear_)
{
    daysPerYear = daysPerYear_;
    Select(ID_daysPerYear, (void *)&daysPerYear);
}

void
PeaksOverThresholdAttributes::SetDaysPerMonth(const int *daysPerMonth_)
{
    for(int i = 0; i < 12; ++i)
        daysPerMonth[i] = daysPerMonth_[i];
    Select(ID_daysPerMonth, (void *)daysPerMonth, 12);
}

void
PeaksOverThresholdAttributes::SetCovariateModelScale(bool covariateModelScale_)
{
    covariateModelScale = covariateModelScale_;
    Select(ID_covariateModelScale, (void *)&covariateModelScale);
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
PeaksOverThresholdAttributes::SetComputeRVDifferences(bool computeRVDifferences_)
{
    computeRVDifferences = computeRVDifferences_;
    Select(ID_computeRVDifferences, (void *)&computeRVDifferences);
}

void
PeaksOverThresholdAttributes::SetRvDifference1(int rvDifference1_)
{
    rvDifference1 = rvDifference1_;
    Select(ID_rvDifference1, (void *)&rvDifference1);
}

void
PeaksOverThresholdAttributes::SetRvDifference2(int rvDifference2_)
{
    rvDifference2 = rvDifference2_;
    Select(ID_rvDifference2, (void *)&rvDifference2);
}

void
PeaksOverThresholdAttributes::SetComputeParamValues(bool computeParamValues_)
{
    computeParamValues = computeParamValues_;
    Select(ID_computeParamValues, (void *)&computeParamValues);
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
PeaksOverThresholdAttributes::SetDumpData(bool dumpData_)
{
    dumpData = dumpData_;
    Select(ID_dumpData, (void *)&dumpData);
}

void
PeaksOverThresholdAttributes::SetDumpDebug(bool dumpDebug_)
{
    dumpDebug = dumpDebug_;
    Select(ID_dumpDebug, (void *)&dumpDebug);
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

int
PeaksOverThresholdAttributes::GetDataAnalysisYear1() const
{
    return dataAnalysisYear1;
}

int
PeaksOverThresholdAttributes::GetDataAnalysisYear2() const
{
    return dataAnalysisYear2;
}

bool
PeaksOverThresholdAttributes::GetEnsemble() const
{
    return ensemble;
}

int
PeaksOverThresholdAttributes::GetNumEnsembles() const
{
    return numEnsembles;
}

float
PeaksOverThresholdAttributes::GetCutoff() const
{
    return cutoff;
}

PeaksOverThresholdAttributes::CutoffModeType
PeaksOverThresholdAttributes::GetCutoffMode() const
{
    return CutoffModeType(cutoffMode);
}

bool
PeaksOverThresholdAttributes::GetNoConsecutiveDay() const
{
    return noConsecutiveDay;
}

PeaksOverThresholdAttributes::OptimizationType
PeaksOverThresholdAttributes::GetOptimizationMethod() const
{
    return OptimizationType(optimizationMethod);
}

double
PeaksOverThresholdAttributes::GetDataScaling() const
{
    return dataScaling;
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

int
PeaksOverThresholdAttributes::GetDaysPerYear() const
{
    return daysPerYear;
}

const int *
PeaksOverThresholdAttributes::GetDaysPerMonth() const
{
    return daysPerMonth;
}

int *
PeaksOverThresholdAttributes::GetDaysPerMonth()
{
    return daysPerMonth;
}

bool
PeaksOverThresholdAttributes::GetCovariateModelScale() const
{
    return covariateModelScale;
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
PeaksOverThresholdAttributes::GetComputeRVDifferences() const
{
    return computeRVDifferences;
}

int
PeaksOverThresholdAttributes::GetRvDifference1() const
{
    return rvDifference1;
}

int
PeaksOverThresholdAttributes::GetRvDifference2() const
{
    return rvDifference2;
}

bool
PeaksOverThresholdAttributes::GetComputeParamValues() const
{
    return computeParamValues;
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

bool
PeaksOverThresholdAttributes::GetDumpData() const
{
    return dumpData;
}

bool
PeaksOverThresholdAttributes::GetDumpDebug() const
{
    return dumpDebug;
}

///////////////////////////////////////////////////////////////////////////////
// Select property methods
///////////////////////////////////////////////////////////////////////////////

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
PeaksOverThresholdAttributes::SelectDaysPerMonth()
{
    Select(ID_daysPerMonth, (void *)daysPerMonth, 12);
}

void
PeaksOverThresholdAttributes::SelectCovariateReturnYears()
{
    Select(ID_covariateReturnYears, (void *)&covariateReturnYears);
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
    case ID_dataAnalysisYear1:            return "dataAnalysisYear1";
    case ID_dataAnalysisYear2:            return "dataAnalysisYear2";
    case ID_ensemble:                     return "ensemble";
    case ID_numEnsembles:                 return "numEnsembles";
    case ID_cutoff:                       return "cutoff";
    case ID_cutoffMode:                   return "cutoffMode";
    case ID_noConsecutiveDay:             return "noConsecutiveDay";
    case ID_optimizationMethod:           return "optimizationMethod";
    case ID_dataScaling:                  return "dataScaling";
    case ID_aggregation:                  return "aggregation";
    case ID_annualPercentile:             return "annualPercentile";
    case ID_seasonalPercentile:           return "seasonalPercentile";
    case ID_monthlyPercentile:            return "monthlyPercentile";
    case ID_daysPerYear:                  return "daysPerYear";
    case ID_daysPerMonth:                 return "daysPerMonth";
    case ID_covariateModelScale:          return "covariateModelScale";
    case ID_covariateModelLocation:       return "covariateModelLocation";
    case ID_covariateModelShape:          return "covariateModelShape";
    case ID_computeCovariates:            return "computeCovariates";
    case ID_covariateReturnYears:         return "covariateReturnYears";
    case ID_computeRVDifferences:         return "computeRVDifferences";
    case ID_rvDifference1:                return "rvDifference1";
    case ID_rvDifference2:                return "rvDifference2";
    case ID_computeParamValues:           return "computeParamValues";
    case ID_displaySeason:                return "displaySeason";
    case ID_displayMonth:                 return "displayMonth";
    case ID_dumpData:                     return "dumpData";
    case ID_dumpDebug:                    return "dumpDebug";
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
    case ID_dataAnalysisYear1:            return FieldType_int;
    case ID_dataAnalysisYear2:            return FieldType_int;
    case ID_ensemble:                     return FieldType_bool;
    case ID_numEnsembles:                 return FieldType_int;
    case ID_cutoff:                       return FieldType_float;
    case ID_cutoffMode:                   return FieldType_enum;
    case ID_noConsecutiveDay:             return FieldType_bool;
    case ID_optimizationMethod:           return FieldType_enum;
    case ID_dataScaling:                  return FieldType_double;
    case ID_aggregation:                  return FieldType_enum;
    case ID_annualPercentile:             return FieldType_double;
    case ID_seasonalPercentile:           return FieldType_doubleArray;
    case ID_monthlyPercentile:            return FieldType_doubleArray;
    case ID_daysPerYear:                  return FieldType_int;
    case ID_daysPerMonth:                 return FieldType_intArray;
    case ID_covariateModelScale:          return FieldType_bool;
    case ID_covariateModelLocation:       return FieldType_bool;
    case ID_covariateModelShape:          return FieldType_bool;
    case ID_computeCovariates:            return FieldType_bool;
    case ID_covariateReturnYears:         return FieldType_intVector;
    case ID_computeRVDifferences:         return FieldType_bool;
    case ID_rvDifference1:                return FieldType_int;
    case ID_rvDifference2:                return FieldType_int;
    case ID_computeParamValues:           return FieldType_bool;
    case ID_displaySeason:                return FieldType_enum;
    case ID_displayMonth:                 return FieldType_enum;
    case ID_dumpData:                     return FieldType_bool;
    case ID_dumpDebug:                    return FieldType_bool;
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
    case ID_dataAnalysisYear1:            return "int";
    case ID_dataAnalysisYear2:            return "int";
    case ID_ensemble:                     return "bool";
    case ID_numEnsembles:                 return "int";
    case ID_cutoff:                       return "float";
    case ID_cutoffMode:                   return "enum";
    case ID_noConsecutiveDay:             return "bool";
    case ID_optimizationMethod:           return "enum";
    case ID_dataScaling:                  return "double";
    case ID_aggregation:                  return "enum";
    case ID_annualPercentile:             return "double";
    case ID_seasonalPercentile:           return "doubleArray";
    case ID_monthlyPercentile:            return "doubleArray";
    case ID_daysPerYear:                  return "int";
    case ID_daysPerMonth:                 return "intArray";
    case ID_covariateModelScale:          return "bool";
    case ID_covariateModelLocation:       return "bool";
    case ID_covariateModelShape:          return "bool";
    case ID_computeCovariates:            return "bool";
    case ID_covariateReturnYears:         return "intVector";
    case ID_computeRVDifferences:         return "bool";
    case ID_rvDifference1:                return "int";
    case ID_rvDifference2:                return "int";
    case ID_computeParamValues:           return "bool";
    case ID_displaySeason:                return "enum";
    case ID_displayMonth:                 return "enum";
    case ID_dumpData:                     return "bool";
    case ID_dumpDebug:                    return "bool";
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
    case ID_cutoff:
        {  // new scope
        retval = (cutoff == obj.cutoff);
        }
        break;
    case ID_cutoffMode:
        {  // new scope
        retval = (cutoffMode == obj.cutoffMode);
        }
        break;
    case ID_noConsecutiveDay:
        {  // new scope
        retval = (noConsecutiveDay == obj.noConsecutiveDay);
        }
        break;
    case ID_optimizationMethod:
        {  // new scope
        retval = (optimizationMethod == obj.optimizationMethod);
        }
        break;
    case ID_dataScaling:
        {  // new scope
        retval = (dataScaling == obj.dataScaling);
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
    case ID_daysPerYear:
        {  // new scope
        retval = (daysPerYear == obj.daysPerYear);
        }
        break;
    case ID_daysPerMonth:
        {  // new scope
        // Compare the daysPerMonth arrays.
        bool daysPerMonth_equal = true;
        for(int i = 0; i < 12 && daysPerMonth_equal; ++i)
            daysPerMonth_equal = (daysPerMonth[i] == obj.daysPerMonth[i]);

        retval = daysPerMonth_equal;
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
    case ID_computeParamValues:
        {  // new scope
        retval = (computeParamValues == obj.computeParamValues);
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

