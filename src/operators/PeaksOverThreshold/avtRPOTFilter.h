#ifndef AVT_R_EXTREMES_FILTER_H
#define AVT_R_EXTREMES_FILTER_H

#include <filters_exports.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>
#include <PeaksOverThresholdAttributes.h>
#include <string>
#include <vector>

class vtkDataSet;

// ****************************************************************************
// Class:  avtRPOTFilter
//
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

class AVTFILTERS_API avtRPOTFilter : virtual public avtDatasetToDatasetFilter,
                                     virtual public avtTimeLoopFilter
{
  public:
    avtRPOTFilter();
    virtual ~avtRPOTFilter();
    virtual const char* GetType() {return "avtRPOTFilter";}

    std::string newVarName;

    PeaksOverThresholdAttributes::AggregationType aggregation;
    PeaksOverThresholdAttributes::SeasonType displaySeason;
    PeaksOverThresholdAttributes::MonthType displayMonth;

    float annualPercentile, monthlyPercentile[12], seasonalPercentile[4];
    float cutoff;

    std::string codeDir;
    bool dumpData;
    float scalingVal;
    
  protected:
    void                    Initialize();
    virtual void            Execute();
    virtual void            PreExecute();
    virtual void            PostExecute();
    virtual void            CreateFinalOutput();
    virtual bool            ExecutionSuccessful();

    virtual bool            FilterSupportsTimeParallelization();
    virtual bool            DataCanBeParallelizedOverTime(void);

    float                   CalculateThreshold(int loc, int arr);
    int                     GetIndexFromDay(int t);
    std::string             CreateQuantileCommand(const char *var, const char *in, int aggregationIdx);
    std::string             GetDumpFileName(int idx, int var);

    vtkDataSet *outDS;
    int numTuples, numTimes, numYears;
    bool nodeCenteredData, initialized;
    int idx0, idxN;
    //values[location][aggregation][time_i]
    std::vector<std::vector<std::vector<double> > >values;
};

#endif
