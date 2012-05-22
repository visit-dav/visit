#ifndef AVT_R_EXTREMES_FILTER_H
#define AVT_R_EXTREMES_FILTER_H

#include <filters_exports.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>
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

    enum ComputeMaxMethod { MONTHLY=0, YEARLY, SEASONAL};

    ComputeMaxMethod computeMaxes;
    int monthDisplay;
    std::string codeDir;
    bool dumpData;
    float scalingVal, percentile;
    
  protected:
    void                    Initialize();
    virtual void            Execute();
    virtual void            PreExecute();
    virtual void            PostExecute();
    virtual void            CreateFinalOutput();
    virtual bool            ExecutionSuccessful();

    virtual bool            FilterSupportsTimeParallelization();
    virtual bool            DataCanBeParallelizedOverTime(void);
    int                     GetIndexFromDay(const int &t);

    vtkDataSet *outDS;
    int numTuples;
    bool nodeCenteredData, initialized;
    int idx0, idxN;
    //values[location][aggregation][i]
    std::vector<std::vector<std::vector<double> > >values;
    int numYears;
};

#endif
