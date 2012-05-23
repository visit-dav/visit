#include <avtParallel.h>
#include <avtRExtremesFilter.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
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
// Method:  avtRExtremesFilter::avtRExtremesFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRExtremesFilter::avtRExtremesFilter()
{
    initialized = false;
    outDS = NULL;
    numTuples = 0;
    nodeCenteredData = false;
    aggregation = ExtremeValueAnalysisAttributes::ANNUAL;
    displaySeason = ExtremeValueAnalysisAttributes::WINTER;
    displayMonth = ExtremeValueAnalysisAttributes::January;
    numYears = 0;
    scalingVal =  1.0;
    dumpData = false;
    indexCounter = 0;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::~avtRExtremesFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRExtremesFilter::~avtRExtremesFilter()
{
}

// ****************************************************************************
// Method:  avtRExtremesFilter::FilterSupportsTimeParallelization
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRExtremesFilter::FilterSupportsTimeParallelization()
{
    return true;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::DataCanBeParallelizedOverTime
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRExtremesFilter::DataCanBeParallelizedOverTime()
{
    return true;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::GetIndexFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRExtremesFilter::GetIndexFromDay(const int &t)
{
    if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY ||
        aggregation == ExtremeValueAnalysisAttributes::SEASONAL)
        
    {
        int year = t/365;
        int dayMonth[12] = {31,59,90,120,151,181,213,244,274,304,334,365}; //no leap year
        int dayInYear = t % 365;
        int month = -1, ret = -1;
        for (int i = 0; i < 12; i++)
            if (dayInYear <= dayMonth[i])
            {
                month = i;
                ret = (year*12) + i; 
                break;
            }

        if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY)
            return ret;
        else
        {
            int season = -1;
            if (month == 11 || month == 0 || month == 1)
                season = (year*4) + 0;
            else if (month == 2 || month == 3 || month == 4)
                season = (year*4) + 1;
            else if (month == 5 || month == 6 || month == 7)
                season = (year*4) + 2;
            else if (month == 8 || month == 9 || month == 10)
                season = (year*4) + 3;
            return season;
        }
    }
    else if (aggregation == ExtremeValueAnalysisAttributes::ANNUAL)
        return t / 365;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::ExecutionSuccessful
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

bool
avtRExtremesFilter::ExecutionSuccessful()
{
    return true;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::Initialize
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::Initialize()
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

    int numAggregationArrays = 0;
    //Maxima aggregation.
    if (aggregation == ExtremeValueAnalysisAttributes::ANNUAL)
        numAggregationArrays = numYears;
    else if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY)
        numAggregationArrays = 12*numYears;
    else if (aggregation == ExtremeValueAnalysisAttributes::SEASONAL)
        numAggregationArrays = 4*numYears;
    
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
#endif

    values.resize(numAggregationArrays);
    for (int i = 0; i < numAggregationArrays; i++)
    {
        values[i].resize(numTuples);
        for (int j = 0; j < numTuples; j++)
            values[i][j] = -numeric_limits<double>::max();
    }
    
    initialized = true;
    delete [] leaves;


    //Setup outputs.
    SetupOutput("rv", "returnValue", 1);
    SetupOutput("se_rv", "se.returnValue", 1);
    SetupOutput("mle", "mle", 3);
    SetupOutput("se_mle", "se.mle", 3);
    /*
    for (int i=0; i < outputArr.size(); i++)
        cout<<i<<": "<<outputArr[i].name<<" = "<<outputArr[i].Rname<<" vidx= "<<outputArr[i].varIdx<<" "<<outputArr[i].aggrIdx<<" "<<outputArr[i].dumpFileName<<endl;
    */
}


// ****************************************************************************
// Method:  avtRExtremesFilter::PreExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::PreExecute()
{
    avtDatasetToDatasetFilter::PreExecute();
}

// ****************************************************************************
// Method:  avtRExtremesFilter::PostExecute
//
// Programmer:  Dave Pugmire
// Creation:    March 16, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::PostExecute()
{
    avtDatasetToDatasetFilter::PostExecute();
}

// ****************************************************************************
// Method:  avtRExtremesFilter::Execute
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

void
avtRExtremesFilter::Execute()
{
    debug1<<"avtRExtremesFilter::Execute() time= "<<currentTime<<endl;

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
    
    int index = GetIndexFromDay(currentTime);
    //cout<<"index= "<<index<<" "<<values.size()<<" "<<values[index].size()<<" nt= "<<scalars->GetNumberOfTuples()<<endl;

    for (int i = 0; i < numTuples; i++)
    {
        float v = vals[i];
        if (v > values[index][i])
            values[index][i] = v;
    }
}

// ****************************************************************************
// Method:  avtRExtremesFilter::CreateFinalOutput
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
avtRExtremesFilter::CreateFinalOutput()
{
    avtCallback::ResetTimeout(0);
    
    debug1<<"avtRExtremesFilter::CreateFinalOutput()"<<endl;
    
    //Unify maxima
#ifdef PARALLEL
    double *outvalues = new double[values[0].size()];
    for (int i = 0; i < values.size(); i++)
    {
        UnifyMaximumDoubleArrayAcrossAllProcessors(&values[i][0], outvalues, values[i].size());
        for (int j = 0; j < values[i].size(); j++)
            values[i][j] = outvalues[j];
    }
    delete [] outvalues;
    //cout<<PAR_Rank()<<" ["<<idx0<<" "<<idxN<<"]"<<endl;
#endif

    //Run R code.
    int numVals = idxN-idx0;

    //Results: $returnValue, $se.returnValue, $mle(x3), $se.mle(x3)
    int numResults = 8;
    //results[aggregation][location][var]
    vector<vector<vector<float> > > results;
    
    int numAggregationArrays = 0;
    //Maxima aggregation.
    if (aggregation == ExtremeValueAnalysisAttributes::ANNUAL)
        numAggregationArrays = 1;
    else if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY)
        numAggregationArrays = 12;
    else if (aggregation == ExtremeValueAnalysisAttributes::SEASONAL)
        numAggregationArrays = 4;

    results.resize(numAggregationArrays);
    for (int i = 0; i < numAggregationArrays; i++)
    {
        results[i].resize(numTuples);
        for (int j = 0; j < numTuples; j++)
        {
            results[i][j].resize(numResults);
            for (int k = 0; k < numResults; k++)
                results[i][j][k] = 0.0f;
        }
    }

    vtkRInterface *RI = vtkRInterface::New();
    vtkDoubleArray *inData = vtkDoubleArray::New();
    inData->SetNumberOfComponents(1);
    inData->SetNumberOfTuples((idxN-idx0)*values.size());
    int idx = 0, N = idxN-idx0;

    for (int i = 0; i < N; i++)
        for (int d = 0; d < values.size(); d++)
        {
            double v = scalingVal * values[d][idx0+i];
            inData->SetValue(idx, v);
            idx++;
        }
    RI->AssignVTKDataArrayToRVariable(inData, "inData");

    cout<<"inData : "<<inData->GetNumberOfTuples()<<" : "<<inData->GetNumberOfComponents()<<endl;
    cout<<"I have "<<idx0<<" "<<idxN<<endl;

    string cmd = GenerateCommand("inData");

    string command;
    char fileLoad[1024];
    sprintf(fileLoad, "source('%s/auxil.r')\n", codeDir.c_str());
    command += fileLoad;
    sprintf(fileLoad, "source('%s/gev.fit2.r')\n", codeDir.c_str());
    command += fileLoad;
    sprintf(fileLoad, "source('%s/gevVisit.r')\n", codeDir.c_str());
    command += fileLoad;
        
    command += cmd;
    cout<<command<<endl;
    RI->EvalRscript(command.c_str());
    //RI->EvalRscript("save.image(file='tmp.RData')\n");

    SetResults(results, RI);

    inData->Delete();
    RI->Delete();

#ifdef PARALLEL
    float *s = new float[results[0][0].size()];
    for (int i = 0; i < results.size(); i++)
    {
        for (int j = 0; j < results[i].size(); j++)
        {
            SumFloatArray(&(results[i][j][0]), s, results[0][0].size());
            if (PAR_Rank() == 0)
                memcpy(&(results[i][j][0]), s, results[0][0].size()*sizeof(float));
        }
    }
    delete [] s;
#endif

    //SumFloatArrayAcrossAllProcessors(result, s, 3*numTuples);

    if (PAR_Rank() == 0)
    {
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        int aggrIdx = 0;
        
        if (aggregation == ExtremeValueAnalysisAttributes::ANNUAL)
            aggrIdx = 0;
        else if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY)
            aggrIdx = (int)displayMonth;
        else if (aggregation == ExtremeValueAnalysisAttributes::SEASONAL)
            aggrIdx = (int)displaySeason;
        
        for (int i = 0; i < numTuples; i++, idx++)
        {
            outVar->SetValue(i, results[aggrIdx][idx][0]);
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
            for (int i=0; i < outputArr.size(); i++)
            {
                ofstream ofile(outputArr[i].dumpFileName.c_str());
                for (int j = 0; j < numTuples; j++)
                    ofile<<results[outputArr[i].aggrIdx][j][outputArr[i].varIdx]<<endl;
            }
        }
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);
}


// ****************************************************************************
// Method:  avtRExtremesFilter::GenerateCommand
//
// Programmer:  Dave Pugmire
// Creation:    Wed May 23 15:21:06 EDT 2012
//
// ****************************************************************************

string
avtRExtremesFilter::GenerateCommand(const char *var)
{
    string aggrType;
    int n;
    if (aggregation == ExtremeValueAnalysisAttributes::ANNUAL)
    {
        aggrType = "annual";
        n = 1;
    }
    else if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY)
    {
        aggrType = "monthly";
        n = 12;
    }
    else if (aggregation == ExtremeValueAnalysisAttributes::SEASONAL)
    {
        aggrType = "seasonal";
        n = 4;
    }

    char str[512];
    sprintf(str,
            "output=gevFit(data=%s,aggregation='%s', nYears=%d, nLocations=%d, returnParams=TRUE)\n",
            var, aggrType.c_str(), numYears, (idxN-idx0));
    
    string command = str;
    for (int i=0; i < outputArr.size(); i++)
    {
        sprintf(str, "%s = %s\n", outputArr[i].name.c_str(), outputArr[i].Rname.c_str());
        command = command + str;
    }
    
    return command;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::SetResults
//
// Programmer:  Dave Pugmire
// Creation:    Wed May 23 15:21:06 EDT 2012
//
// ****************************************************************************

void
avtRExtremesFilter::SetResults(vector<vector<vector<float> > > &results,
                               vtkRInterface *RI)
{
    int numAggregationArrays = 0;
    if (aggregation == ExtremeValueAnalysisAttributes::ANNUAL)
        numAggregationArrays = 1;
    else if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY)
        numAggregationArrays = 12;
    else if (aggregation == ExtremeValueAnalysisAttributes::SEASONAL)
        numAggregationArrays = 4;

    vtkDoubleArray *arr;
    for (int i=0; i < outputArr.size(); i++)
    {
        arr = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray(outputArr[i].name.c_str()));
        int j = 0;
        //cout<<"results: "<<results.size()<<" x "<<results[0].size()<<" x "<<results[0][0].size()<<endl;
        for (int idx = idx0; idx < idxN; idx++, j++)
        {
            //cout<<arr->GetComponent(0,j)<<endl;
            //cout<<"results["<<outputArr[i].aggrIdx<<"]["<<idx<<"]["<<outputArr[i].varIdx<<"]"<<endl;
            results[outputArr[i].aggrIdx][idx][outputArr[i].varIdx] = (float)arr->GetComponent(0, j);
        }
    }
}


static string
makeFileName(const char *nm, int i, int d, int N, int DIM)
{
    string fname;
    char str[512];
    const char *a[1] = {"ANNUAL"};
    const char *s[4] = {"WINTER", "SPRING", "SUMMER", "FALL"};
    const char *m[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    const char **t;
    if (N == 1)
        t = a;
    else if (N == 4)
        t = s;
    else if (N == 12)
        t = m;

    
    if (DIM == 1)
        sprintf(str, "GEV.%s_%s.txt", t[i], nm);
    else
        sprintf(str, "GEV.%s_%s_%d.txt", t[i], nm, d);
    fname = str;

    return fname;
}

// ****************************************************************************
// Method:  avtRExtremesFilter::SetupOutput
//
// Programmer:  Dave Pugmire
// Creation:    Wed May 23 15:21:06 EDT 2012
//
// ****************************************************************************

void
avtRExtremesFilter::SetupOutput(const char *nm, const char *Rnm, int dim)
{
    char str[512];
    int n;
    char *fileNamePrefix;
    if (aggregation == ExtremeValueAnalysisAttributes::ANNUAL)
    {
        n = 1;
        fileNamePrefix = "GEV.annual_";
    }
    else if (aggregation == ExtremeValueAnalysisAttributes::MONTHLY)
    {
        n = 12;
        fileNamePrefix = "GEV.%s_";
    }
    else if (aggregation == ExtremeValueAnalysisAttributes::SEASONAL)
    {
        n = 4;
        fileNamePrefix = "GEV.%s_";
    }

    for (int i = 0; i < n; i++)
    {
        for (int d = 0; d < dim; d++)
        {
            outputType o;
            o.varIdx = indexCounter+d;
            o.aggrIdx = i;
            
            if (dim == 1)
                sprintf(str, "%s_%d", nm, i);
            else
                sprintf(str, "%s_%d_%d", nm, i, d);
            o.name = str;
            
            if (dim == 1)
            {
                if (n == 1)
                    sprintf(str, "output$%s", Rnm);
                else
                    sprintf(str, "output$%s[%d,]", Rnm, i+1);
            }
            else
            {
                if (n == 1)
                    sprintf(str, "output$%s[%d,]", Rnm, d+1);
                else
                    sprintf(str, "output$%s[%d,%d,]", Rnm, d+1, i+1);
            }
            o.Rname = str;

            o.dumpFileName = makeFileName(nm, i, d, n, dim);

            outputArr.push_back(o);
        }
    }
    indexCounter += dim;
}
