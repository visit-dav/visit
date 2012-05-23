#ifndef AVT_R_EXTREMES_FILTER_H
#define AVT_R_EXTREMES_FILTER_H

#include <filters_exports.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>
#include <ExtremeValueAnalysisAttributes.h>
#include <string>
#include <vector>

class vtkDataSet;
class vtkDoubleArray;
class vtkRInterface;

// ****************************************************************************
// Class:  avtRExtremesFilter
//
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

class AVTFILTERS_API avtRExtremesFilter : virtual public avtDatasetToDatasetFilter,
                                          virtual public avtTimeLoopFilter
{
  public:
    avtRExtremesFilter();
    virtual ~avtRExtremesFilter();
    virtual const char* GetType() {return "avtRExtremesFilter";}

    std::string newVarName;
    
    ExtremeValueAnalysisAttributes::AggregationType aggregation;
    ExtremeValueAnalysisAttributes::SeasonType displaySeason;
    ExtremeValueAnalysisAttributes::MonthType displayMonth;

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
    int                     GetIndexFromDay(const int &t);
    std::string             GenerateCommand(const char *var);
    void                    SetResults(std::vector<std::vector<std::vector<float> > > &results,
                                       vtkRInterface *RI);

    void                    SetupOutput(const char *nm, const char *Rnm, int dim);

    vtkDataSet *outDS;
    int numTuples, numTimes, numYears;
    bool nodeCenteredData, initialized;
    int idx0, idxN;
    //values[aggregation][location]
    std::vector<std::vector<double> > values;

    struct outputType
    {
        std::string name, Rname, dumpFileName;
        int aggrIdx, varIdx;
    };

    std::vector<outputType> outputArr;
    int indexCounter;
};

#endif
