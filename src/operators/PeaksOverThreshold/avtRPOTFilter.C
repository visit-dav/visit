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

#ifdef PARALLEL
  #include <mpi.h>
#endif

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
    numBins = 0;
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
// Method:  avtRPOTFilter::GetMonthFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRPOTFilter::GetMonthFromDay(int t)
{
    int year = t/365;
    int dayMonth[12] = {31,59,90,120,151,181,213,244,274,304,334,365}; //no leap year
    int dayInYear = t % 365;

    int month = -1;
    for (int i = 0; i < 12; i++)
        if (dayInYear <= dayMonth[i])
            return i;
    
    EXCEPTION1(ImproperUseException, "Date out of range.");
}

// ****************************************************************************
// Method:  avtRPOTFilter::GetSeasonFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRPOTFilter::GetSeasonFromDay(int t)
{
    int month = GetMonthFromDay(t);
    if (month == 11 || month == 0 || month == 1) // WINTER
        return 0;
    else if (month == 2 || month == 3 || month == 4) //SPRING
        return 1;
    else if (month == 5 || month == 6 || month == 7) //SUMMER
        return 2;
    else if (month == 8 || month == 9 || month == 10) //FALL
        return 3;
    
    EXCEPTION1(ImproperUseException, "Date out of range.");
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
    else if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
        return GetMonthFromDay(t);
    else if (aggregation == PeaksOverThresholdAttributes::SEASONAL)
        return GetSeasonFromDay(t);
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
    int nVals = values[loc][arr].size();
    if (nVals == 0)
        return 0.0f;
    
    vtkDoubleArray *inData = vtkDoubleArray::New();
    inData->SetNumberOfComponents(1);
    inData->SetNumberOfTuples(nVals);
    for (int i = 0; i < nVals; i++)
        inData->SetValue(i, values[loc][arr][i].val);

    vtkRInterface *RI = vtkRInterface::New();
    RI->AssignVTKDataArrayToRVariable(inData, "inData");
    RI->EvalRscript(CreateQuantileCommand("q", "inData", arr).c_str());
    vtkDoubleArray *Q = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("q"));

    float threshold = 0.0f;
    if (Q)
        threshold = (float)Q->GetComponent(0,0);

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

    //How to compute maxes.
    //Monthly maxes.
    if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
        numBins = 12;
    else if (aggregation == PeaksOverThresholdAttributes::SEASONAL)
        numBins = 4;
    else if (aggregation == PeaksOverThresholdAttributes::ANNUAL)
        numBins = 1;
    
    values.resize(numTuples);
    for (int i = 0; i < numTuples; i++)
        values[i].resize(numBins);

    //cout<<"values:"<<values.size()<<" x "<<values[0].size()<<" x "<<values[0][0].size()<<endl;
    //cout<<"values["<<numTuples<<"]["<<numBins<<"][numTimes]"<<endl;
    
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
    {
        float v = vals[i]*scalingVal;
        if (v > cutoff)
            values[i][index].push_back(sample(v,currentTime));
    }
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
#ifdef PARALLEL
    float *tmp = new float[numTimes];
    float *res = new float[numTimes];
    int *flags = new int[numTimes];
    int *flagsRes = new int[numTimes];
    
    for (int i=0; i<numTuples; i++)
    {
        for (int t=0; t<numTimes; t++)
        {
            tmp[t] = 0.0;//-numeric_limits<float>::min();
            flags[t] = 0;
        }
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t=0; t<nt; t++)
            {
                if(PAR_Rank()== 0 ) cout<<values[i][b][t].time<<endl;
                tmp[values[i][b][t].time] = values[i][b][t].val;
                flags[values[i][b][t].time] = 1;
            }
        }
        MPI_Allreduce(tmp, res, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(flags, flagsRes, numTimes, MPI_INT, MPI_SUM, VISIT_MPI_COMM);

        for (int b = 0; b < numBins; b++)
            values[i][b].resize(0);

        //See if it's one of my locations.
        if (i >= idx0 && i < idxN)
        {
            for (int t=0; t<numTimes; t++)
            {
                if (flagsRes[t])
                {
                    int b = GetIndexFromDay(t);
                    values[i][b].push_back(sample(res[t], t));
                }
            }
        }
    }
    
    delete [] tmp;
    delete [] res;
    delete [] flags;
    delete [] flagsRes;
#endif

    vtkRInterface *RI = vtkRInterface::New();
    char fileLoad[1024];
    sprintf(fileLoad,
            "source('%s/auxil.r')\n"   \
            "source('%s/pp.fit2.r')\n" \
            "source('%s/potVisit.r')\n",
            codeDir.c_str(),
            codeDir.c_str(),
            codeDir.c_str());
    
    char potCmd[1024];
            
    vector<float> thresholds(numBins);
    
    vector<vector<float> > outputs[2];
    outputs[0].resize(numBins);
    outputs[1].resize(numBins);
    for (int b = 0; b < numBins; b++)
    {
        outputs[0][b].resize(numTuples);
        outputs[1][b].resize(numTuples);
    }
    for (int i = idx0; i < idxN; i++)
    {
        //DebugData(i, "./dumps/x0");
        for (int j = 0; j < numBins; j++)
            thresholds[j] = CalculateThreshold(i, j);

        int numExceedences = 0;
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t = 0; t < nt; t++)
                if (values[i][b][t].val > thresholds[b])
                    numExceedences++;
        }
        vtkDoubleArray *exceedences = vtkDoubleArray::New();
        vtkIntArray *dayIndices = vtkIntArray::New();
        vtkIntArray *monthIndices = vtkIntArray::New();
        exceedences->SetNumberOfComponents(1);
        exceedences->SetNumberOfTuples(numExceedences);
        dayIndices->SetNumberOfComponents(1);
        dayIndices->SetNumberOfTuples(numExceedences);
        monthIndices->SetNumberOfComponents(1);
        monthIndices->SetNumberOfTuples(numExceedences);
        int idx = 0;
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t = 0; t < nt; t++)
                if (values[i][b][t].val > thresholds[b])
                {
                    exceedences->SetValue(idx, values[i][b][t].val);
                    dayIndices->SetValue(idx, values[i][b][t].time+1);
                    monthIndices->SetValue(idx, GetMonthFromDay(values[i][b][t].time)+1);
                    idx++;
                    if (idx == numExceedences)
                        break;
                }
        }
        RI->AssignVTKDataArrayToRVariable(exceedences, "exceedences");
        RI->AssignVTKDataArrayToRVariable(dayIndices, "dayIndices");
        RI->AssignVTKDataArrayToRVariable(monthIndices, "monthIndices");
        string threshStr, aggrStr;
        if (aggregation == PeaksOverThresholdAttributes::ANNUAL)
        {
            aggrStr = "'annual'";
            char str[32];
            sprintf(str, "%f", thresholds[0]);
            threshStr = str;
        }
        else if (aggregation == PeaksOverThresholdAttributes::MONTHLY)
        {
            aggrStr = "'monthly'";
            threshStr = "thresholds";
            vtkDoubleArray *thresh = vtkDoubleArray::New();
            thresh->SetNumberOfComponents(1);
            thresh->SetNumberOfTuples(12);
            for (int t = 0; t < 12; t++)
                thresh->SetValue(t, thresholds[t]);
            RI->AssignVTKDataArrayToRVariable(thresh, "thresholds");
        }
        else if (aggregation == PeaksOverThresholdAttributes::SEASONAL)
        {
            aggrStr = "'seasonal'";
            threshStr = "thresholds";
            vtkDoubleArray *thresh = vtkDoubleArray::New();
            thresh->SetNumberOfComponents(1);
            thresh->SetNumberOfTuples(4);
            for (int t = 0; t < 4; t++)
                thresh->SetValue(t, thresholds[t]);
            RI->AssignVTKDataArrayToRVariable(thresh, "thresholds");
        }
        char dumpStr[128];
        sprintf(dumpStr, "'tmp_%d.RData'", PAR_Rank());
        sprintf(potCmd,
                "require(ismev)\n" \
                "output = potFit(data = exceedences, day = dayIndices, month = monthIndices, "\
                "nYears=%d, threshold=%s, aggregation=%s)\n"\
                "save.image(file=%s)\n"     \
                "rv = output$returnValue\n" \
                "se_rv = output$se.returnValue\n",
                numYears, threshStr.c_str(), aggrStr.c_str(), dumpStr);
        string command = fileLoad;
        command += potCmd;
        //cout<<"command="<<command<<endl;
        RI->EvalRscript(command.c_str());

        vtkDoubleArray *out_rv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rv"));
        vtkDoubleArray *out_serv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_rv"));
        
        for (int b = 0; b < numBins; b++)
        {
            outputs[0][b][i] = (float)out_rv->GetComponent(b,0);
            outputs[1][b][i] = (float)out_serv->GetComponent(b,0);
        }
    }

#if PARALLEL
    float *in = new float[numTuples];
    float *sum = new float[numTuples];

    for (int v = 0; v < 2; v++)
    {
        for (int b = 0; b<numBins; b++)
        {
            for (int i=0; i<numTuples; i++)
            {
                if (i >= idx0 && i < idxN)
                    in[i] = outputs[v][b][i];
                else
                    in[i] = 0.0f;
            }
            
            SumFloatArray(in, sum, numTuples);
            if (PAR_Rank() == 0)
            {
                for (int i=0; i<numTuples; i++)
                    outputs[v][b][i] = sum[i];
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
            outVar->SetValue(i, outputs[0][displayIdx][i]);
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
            for (int v = 0; v < 2; v++)
            {
                for (int b = 0; b < numBins; b++)
                {
                    string nm = GetDumpFileName(b, v);
                    ofstream ofile(nm.c_str());
                    
                    for (int i = 0; i < numTuples-1; i++)
                        ofile<<outputs[v][b][i]<<", ";
                    ofile<<outputs[v][b][numTuples]<<endl;
                }
            }
        }
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);
}


void
avtRPOTFilter::DebugData(int loc, std::string nm)
{
    char filename[128];
    sprintf(filename, "%s_r%02d_%04d.txt", nm.c_str(), PAR_Rank(), loc);
    
    ofstream ofile(filename);
    for (int b = 0; b < numBins; b++)
    {
        ofile<<"["<<b<<"] ("<<values[loc][b].size()<<") : ";
        for (int t = 0; t < values[loc][b].size(); t++)
            ofile<<values[loc][b][t].val<<" ";    
        ofile<<endl;
    }
}
