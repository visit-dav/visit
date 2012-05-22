#include <avtParallel.h>
#include <avtRPOTFilter.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <limits>
#include <vtkRInterface.h>
#include <vtkRectilinearGrid.h>
#include <DebugStream.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <InvalidFilesException.h>

using namespace std;

// ****************************************************************************
// Method:  avtRPOTFilter::avtRPOTFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRPOTFilter::avtRPOTFilter()
{
    initialized = false;
    outDS = NULL;
    numTuples = 0;
    nodeCenteredData = false;
    aggregation = PeaksOverThresholdAttributes::ANNUAL;
    
    displaySeason = PeaksOverThresholdAttributes::WINTER;
    displayMonth = PeaksOverThresholdAttributes::JAN;

    numYears = 0;
    scalingVal =  86400.0;
    dumpData = false;
    cutoff = 0.0f;
}

// ****************************************************************************
// Method:  avtRPOTFilter::~avtRPOTFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRPOTFilter::~avtRPOTFilter()
{
}

// ****************************************************************************
// Method:  avtRPOTFilter::FilterSupportsTimeParallelization
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRPOTFilter::FilterSupportsTimeParallelization()
{
    return true;
}

// ****************************************************************************
// Method:  avtRPOTFilter::DataCanBeParallelizedOverTime
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRPOTFilter::DataCanBeParallelizedOverTime()
{
    return true;
}

// ****************************************************************************
// Method:  avtRPOTFilter::GetIndexFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRPOTFilter::GetIndexFromDay(int t)
{
    if (aggregation == PeaksOverThresholdAttributes::ANNUAL)
        return 0;
    else if (aggregation == PeaksOverThresholdAttributes::MONTHLY ||
             aggregation == PeaksOverThresholdAttributes::SEASONAL)
    {
        int year = t/365;
        int dayMonth[12] = {31,59,90,120,151,181,213,244,274,304,334,365}; //no leap year
        int dayInYear = t % 365;

        int month = -1;
        for (int i = 0; i < 12; i++)
            if (dayInYear <= dayMonth[i])
            {
                month = i;
                break;
            }

        if (aggregation == PeaksOverThresholdAttributes::SEASONAL)
        {
            if (month == 11 || month == 0 || month == 1) // WINTER
                return 0;
            else if (month == 2 || month == 3 || month == 4) //SPRING
                return 1;
            else if (month == 5 || month == 6 || month == 7) //SUMMER
                return 2;
            else if (month == 8 || month == 9 || month == 10) //FALL
                return 3;
        }
        else
            return month;
    }
}

// ****************************************************************************
// Method:  avtRPOTFilter::CalculateThreshold
//
// Programmer:  Dave Pugmire
// Creation:    May 22, 2012
//
// ****************************************************************************

float
avtRPOTFilter::CalculateThreshold(int loc, int arr)
{
    int nVals = numTimes;

    vtkDoubleArray *inData = vtkDoubleArray::New();
    inData->SetNumberOfComponents(1);

    if (cutoff != 0.0f)
    {
        nVals = 0;
        for (int i = 0; i < numTimes; i++)
        {
            if (values[loc][arr][i] > cutoff)
                nVals++;
        }
        
        inData->SetNumberOfTuples(nVals);
        int idx = 0;
        for (int i = 0; i < nVals; i++)
        {
            if (values[loc][arr][i] > cutoff)
            {
                inData->SetValue(idx, values[loc][arr][i]);
                idx++;
            }
        }
    }
    else
    {
        inData->SetNumberOfTuples(nVals);
        for (int i = 0; i < nVals; i++)
            inData->SetValue(i, values[loc][arr][i]);
    }

    vtkRInterface *RI = vtkRInterface::New();
    
    RI->AssignVTKDataArrayToRVariable(inData, "inData");
    RI->EvalRscript(CreateQuantileCommand("q", "inData", arr).c_str());
    vtkDoubleArray *Q = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("q"));
    float threshold = (float)Q->GetComponent(0,0);

    inData->Delete();
    RI->Delete();
    
    return threshold;
}


// ****************************************************************************
// Method:  avtRPOTFilter::ExecutionSuccessful
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRPOTFilter::ExecutionSuccessful()
{
    return true;
}

// ****************************************************************************
// Method:  avtRPOTFilter::CreateQuantileCommand
//
// Programmer:  Dave Pugmire
// Creation:    May 22, 2012
//
// ****************************************************************************

string
avtRPOTFilter::CreateQuantileCommand(const char *var, const char *in, int aggregationIdx)
{
    float percentile = 0.0;
    
    if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
        percentile = monthlyPercentile[aggregationIdx];
    else if (aggregation == PeaksOverThresholdAttributes::SEASONAL)
        percentile = seasonalPercentile[aggregationIdx];
    else if (aggregation == PeaksOverThresholdAttributes::ANNUAL)
        percentile = annualPercentile;

    char str[128];
    sprintf(str, "%s = quantile(%s, %f)\n", var, in, percentile);
    
    string cmd = str;
    return cmd;
}


// ****************************************************************************
// Method:  avtRPOTFilter::GetDumpFileName
//
// Programmer:  Dave Pugmire
// Creation:    May 22, 2012
//
// ****************************************************************************

string
avtRPOTFilter::GetDumpFileName(int idx, int var)
{
    string nm;
    char str[128];
    
    char *varNm;
    if (var == 0)
        varNm = "returnValue";
    else if (var == 1)
        varNm = "se_returnValue";
      
    //cout<<"varNm= "<<varNm<<endl;
    
    if (aggregation == PeaksOverThresholdAttributes::ANNUAL)
        sprintf(str, "POT.annual_%s.txt", varNm);
    else if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
    {
        const char *m[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        sprintf(str, "POT.%s_%s.txt", m[idx], varNm);
    }
    else if (aggregation == PeaksOverThresholdAttributes::SEASONAL)
    {
        const char *s[4] = {"WINTER", "SPRING", "SUMMER", "FALL"};
        sprintf(str, "POT.%s_%s.txt", s[idx], varNm);
    }
    
    nm = str;
    return nm;
}

// ****************************************************************************
// Method:  avtRPOTFilter::Initialize
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRPOTFilter::Initialize()
{
    if (initialized)
        return;

    int nleaves;
    vtkDataSet **leaves = GetInputDataTree()->GetAllLeaves(nleaves);

    if (nleaves != 1)
    {
        EXCEPTION1(ImproperUseException, "Multi-domain not supported yet.");
    }

    vtkDataSet *inDS = leaves[0];
    outDS = (vtkDataSet *)(inDS->NewInstance());
    outDS->CopyStructure(inDS);

    nodeCenteredData = (GetInput()->GetInfo().GetAttributes().GetCentering() == AVT_NODECENT);
    if (nodeCenteredData)
        numTuples = inDS->GetPointData()->GetScalars()->GetNumberOfTuples();
    else
        numTuples = inDS->GetCellData()->GetScalars()->GetNumberOfTuples();
    
    //cout<<"Initialize: numTuples= "<<numTuples<<endl;
    int t0 = GetStartTime();
    int t1 = GetEndTime();
    numTimes = t1-t0 + 1;
    numYears = numTimes/365;

    int numArrays = 0;
    //How to compute maxes.
    //Monthly maxes.
    if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
        numArrays = 12;
    else if (aggregation == PeaksOverThresholdAttributes::SEASONAL)
        numArrays = 4;
    else if (aggregation == PeaksOverThresholdAttributes::ANNUAL)
        numArrays = 1;
    
    values.resize(numTuples);
    for (int i = 0; i < numTuples; i++)
    {
        values[i].resize(numArrays);
        for (int j = 0; j < numArrays; j++)
        {
            values[i][j].resize(numTimes);
            for (int k = 0; k < numTimes; k++)
                values[i][j][k] = 0.0;
        }
    }

    //cout<<"values:"<<values.size()<<" x "<<values[0].size()<<" x "<<values[0][0].size()<<endl;
    //cout<<"values["<<numTuples<<"]["<<numArrays<<"][numTimes]"<<endl;
    
    idx0 = 0;
    idxN = numTuples;
    
#ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();

    int nSamplesPerProc = (numTuples / nProcs);
    int oneExtraUntil = (numTuples % nProcs);
    if (rank < oneExtraUntil)
    {
        idx0 = (rank)*(nSamplesPerProc+1);
        idxN = (rank+1)*(nSamplesPerProc+1);
    }
    else
    {
        idx0 = (rank)*(nSamplesPerProc) + oneExtraUntil;
        idxN = (rank+1)*(nSamplesPerProc) + oneExtraUntil;
    }
    debug1<<"I have: ["<<idx0<<" "<<idxN<<"]"<<endl;
#endif
    
    initialized = true;
    delete [] leaves;
}


// ****************************************************************************
// Method:  avtRPOTFilter::PreExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRPOTFilter::PreExecute()
{
    avtDatasetToDatasetFilter::PreExecute();
}

// ****************************************************************************
// Method:  avtRPOTFilter::PostExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRPOTFilter::PostExecute()
{
    avtDatasetToDatasetFilter::PostExecute();
}

// ****************************************************************************
// Method:  avtRPOTFilter::Execute
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRPOTFilter::Execute()
{
    debug1<<"avtRPOTFilter::Execute() time= "<<currentTime<<endl;

    Initialize();
    int nleaves;
    vtkDataSet **leaves = GetInputDataTree()->GetAllLeaves(nleaves);
    vtkDataSet *ds = leaves[0];
    vtkFloatArray *scalars = NULL;

    if (nodeCenteredData)
        scalars = (vtkFloatArray *)ds->GetPointData()->GetScalars();
    else
        scalars = (vtkFloatArray *)ds->GetCellData()->GetScalars();
    float *vals = (float *) scalars->GetVoidPointer(0);

    int nTuples = scalars->GetNumberOfTuples();
    int index = GetIndexFromDay(currentTime);

    //cout<<"processing "<<currentTime<<" sv = "<<scalingVal<<" index= "<<index<<endl;
    for (int i = 0; i < nTuples; i++)
        values[i][index][currentTime] = vals[i]*scalingVal;
}

// ****************************************************************************
// Method:  avtRPOTFilter::CreateFinalOutput
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// Modifications:
//
//   Dave Pugmire, Wed Apr 11 17:07:26 EDT 2012
//   Remove the processing of locations in rounds. No longer needed.
//
// ****************************************************************************

void
avtRPOTFilter::CreateFinalOutput()
{
    avtCallback::ResetTimeout(0);

    //Exchange data....
    int numArrays = values[0].size();
#ifdef PARALLEL
    double *tmp = new double[numTimes];

    for (int i = 0; i < numTuples; i++)
    {
        for (int j = 0; j < numArrays; j++)
        {
            UnifyMaximumDoubleArrayAcrossAllProcessors(&(values[i][j][0]), tmp, numTimes);
            for (int k = 0; k < numTimes; k++)
                values[i][j][k] = tmp[k];
        }
    }
    delete [] tmp;
#endif
    
    vtkRInterface *RI = vtkRInterface::New();
    char fileLoad[1024];
    sprintf(fileLoad,
            "source('%s/auxil.r')\n"    \
            "source('%s/pp.fit2.r')\n" \
            "source('%s/potVisit.r')\n",
            codeDir.c_str(),
            codeDir.c_str(),
            codeDir.c_str());
    
    char potCmd[1024];
            
    
    for (int j = 0; j < numArrays; j++)
    {
        for (int i = idx0; i < idxN; i++)
        {
            float threshold = CalculateThreshold(i, j);

            int numExceedences = 0;
            for (int k = 0; k < numTimes; k++)
                if (values[i][j][k] > threshold)
                    numExceedences++;

            vtkDoubleArray *exceedences = vtkDoubleArray::New();
            vtkIntArray *dayIndices = vtkIntArray::New();
            exceedences->SetNumberOfComponents(1);
            exceedences->SetNumberOfTuples(numExceedences);
            dayIndices->SetNumberOfComponents(1);
            dayIndices->SetNumberOfTuples(numExceedences);
            
            int idx = 0;
            for (int k = 0; k < numTimes; k++)
            {
                if (values[i][j][k] > threshold)
                {
                    exceedences->SetValue(idx, values[i][j][k]);
                    dayIndices->SetValue(idx, k);
                    idx++;
                    if (idx == numExceedences)
                        break;
                }
            }
            RI->AssignVTKDataArrayToRVariable(exceedences, "exceedences");
            RI->AssignVTKDataArrayToRVariable(dayIndices, "dayIndices");
            sprintf(potCmd,
                    "require(ismev)\n" \
                    "output = potFit(data = exceedences, day = dayIndices, aggregation = 'annual', nYears = %d, threshold = %f)\n"
                    /*
                    "save.image(file='tmp.RData')\n"  \
                    */
                    "rv = output$returnValue\n"\
                    "se_rv = output$se.returnValue\n",
                    numYears, threshold);
            string command = fileLoad;
            command += potCmd;
            //cout<<"command="<<command<<endl;
            RI->EvalRscript(command.c_str());

            vtkDoubleArray *out_rv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rv"));
            vtkDoubleArray *out_serv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_rv"));

            if (dumpData)
            {
                values[i][j].resize(2);
                values[i][j][0] = (float)out_rv->GetComponent(0,0);
                values[i][j][1] = (float)out_serv->GetComponent(0,0);
            }
            else
            {
                values[i][j].resize(1);
                values[i][j][0] = (float)out_rv->GetComponent(0,0);
            }
        }
    }

#if PARALLEL
    float *in = new float[numTuples];
    float *sum = new float[numTuples];
    
    for (int i = 0; i < numTuples; i++)
        in[i] = 0.0f;
    
    for (int k = 0; k < values[0][0].size(); k++)
    {
        for (int j = 0; j < numArrays; j++)
        {
            for (int i = idx0; i < idxN; i++)
                in[i] = values[i][j][k];
        
            SumFloatArray(in, sum, numTuples);
            if (PAR_Rank() == 0)
            {
                for (int i = 0; i < numTuples; i++)
                    values[i][j][k] = sum[i];
            }
        }
    }

    delete [] in;
    delete [] sum;
#endif

    if (PAR_Rank() == 0)
    {
        int displayIdx = 0;
        if (aggregation == PeaksOverThresholdAttributes::ANNUAL)
            displayIdx = 0;
        else if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
            displayIdx = (int)displayMonth - (int)(PeaksOverThresholdAttributes::JAN);
        else if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
            displayIdx = (int)displaySeason - (int)(PeaksOverThresholdAttributes::WINTER);
        
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
        {
            outVar->SetValue(i, values[i][displayIdx][0]);
        }
    
        if (nodeCenteredData)
            outDS->GetPointData()->SetScalars(outVar);
        else
            outDS->GetPointData()->SetScalars(outVar);
        
        outVar->Delete();

        avtDataTree_p outputTree = new avtDataTree(outDS, 0);
        SetOutputDataTree(outputTree);
        outDS->Delete();

        if (dumpData)
        {
            for (int k = 0; k < values[0][0].size(); k++)
            {
                for (int j = 0; j < numArrays; j++)
                {
                    string nm = GetDumpFileName(j, k);
                    ofstream ofile(nm.c_str());
                    
                    for (int i = 0; i < numTuples-1; i++)
                        ofile<<values[i][j][k]<<", ";
                    ofile<<values[numTuples-1][j][k]<<endl;
                }
            }
        }
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);
}
