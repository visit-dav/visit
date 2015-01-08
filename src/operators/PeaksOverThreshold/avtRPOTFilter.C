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

// ************************************************************************* //
//  File: avtRPOTFilter.C
// ************************************************************************* //

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
#include <vtkRectilinearGrid.h>
#include <InvalidFilesException.h>
#include <FileWriter.h>
#include <TimingsManager.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

using namespace std;

string avtRPOTFilter::exceedencesStr = "exceedences";
string avtRPOTFilter::dayIndicesStr = "dayIndices";
string avtRPOTFilter::yearIndicesStr = "yearIndices";
string avtRPOTFilter::monthIndicesStr = "monthIndices";

// ****************************************************************************
// Method:  avtRPOTFilter::avtRPOTFilter
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

avtRPOTFilter::avtRPOTFilter()
{
    outDS = NULL;
    numTuples = 0;
    nodeCenteredData = false;
    numYears = 0;
    numBins = 0;
    outputValsPerLoc = 0;
    initialized = false;
    idx0 = idxN = 0;
    cycle0 = 0;
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
    int dayInYear = t % daysPerYear;

    int month = -1;
    for (int i = 0; i < 12; i++)
        if (dayInYear <= dayCountAtMonthEnd[i])
            return i;
    
    EXCEPTION1(ImproperUseException, "Date out of range.");
}

// ****************************************************************************
// Method:  avtRPOTFilter::GetYearFromDay
//
// Programmer:  Dave Pugmire
// Creation:    February  7, 2012
//
// ****************************************************************************

int
avtRPOTFilter::GetYearFromDay(int t)
{
    int x = t/daysPerYear;
    return t/daysPerYear;
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
    if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        return 0;
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
        return GetMonthFromDay(t);
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
        return GetMonthFromDay(t); //DRP GetSeasonFromDay(t);
}

// ****************************************************************************
// Method:  avtRPOTFilter::CalculateThresholds
//
// Programmer:  Dave Pugmire
// Creation:    May 22, 2012
//
// ****************************************************************************

vector<float>
avtRPOTFilter::CalculateThresholds(int loc)
{
    vector<float> thresholds;
    int nArrs = values[loc].size();
    
    thresholds.resize(nArrs);
    vtkRInterface *RI = vtkRInterface::New();

    //cout<<"CalculateThresholds("<<loc<<") nArrs= "<<nArrs<<endl;
    
    for (int i = 0; i < nArrs; i++)
    {
        int nVals = values[loc][i].size();
        if (nVals == 0)
        {
            cout<<"FAIL!!"<<endl;
            thresholds[i] = 0.0f;
            continue;
        }
        
        vtkDoubleArray *inData = vtkDoubleArray::New();
        inData->SetNumberOfComponents(1);
        inData->SetNumberOfTuples(nVals);
        for (int j = 0; j < nVals; j++)
            inData->SetValue(j, values[loc][i][j].val);

        RI->AssignVTKDataArrayToRVariable(inData, "inData");
        RI->EvalRscript(CreateQuantileCommand("q", "inData", i).c_str());
        vtkDoubleArray *Q = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("q"));

        thresholds[i] = (Q ? (float)Q->GetComponent(0,0) : 0.0f);

        inData->Delete();
    }
    
    RI->Delete();
    return thresholds;
}

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
// Modifications:
//
//   Dave Pugmire, Wed Jul 31 14:44:32 EDT 2013
//   Indexing for seasonal analysis was wrong.
//
// ****************************************************************************

string
avtRPOTFilter::CreateQuantileCommand(const char *var, const char *in, int aggregationIdx)
{
    float percentile = 0.0;
    
    if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
        percentile = atts.GetMonthlyPercentile()[aggregationIdx];
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
    {
        //Winter
        if (aggregationIdx == 11 || aggregationIdx == 0 || aggregationIdx == 1)
            percentile = atts.GetSeasonalPercentile()[0];
        //Spring
        else if (aggregationIdx == 2 || aggregationIdx == 3 || aggregationIdx == 4)
            percentile = atts.GetSeasonalPercentile()[1];
        //Summer
        else if (aggregationIdx == 5 || aggregationIdx == 6 || aggregationIdx == 7)
            percentile = atts.GetSeasonalPercentile()[2];
        //Fall
        else if (aggregationIdx == 8 || aggregationIdx == 9 || aggregationIdx == 10)
            percentile = atts.GetSeasonalPercentile()[3];
    }
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        percentile = atts.GetAnnualPercentile();

    char str[128];
    sprintf(str, "%s = quantile(%s, %f)\n", var, in, percentile);
    string cmd = str;
    //cout<<"   CreateQuantile: "<<aggregationIdx<<" "<<cmd<<endl;

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
avtRPOTFilter::GetDumpFileName(int idx, int yr, int var)
{
    string nm;
    char str[128];
    
    char *varNm;
    if (var == 0)
        varNm = "returnValue";
    else if (var == 1)
        varNm = "se_returnValue";
    else if (var == 2)
        varNm = "paramValues";
    else if (var == 3)
        varNm = "se_paramValues";
      
    nm = "POT";
    if (atts.GetComputeCovariates() && yr != -1)
    {
        if (atts.GetComputeRVDifferences() && yr >= atts.GetCovariateReturnYears().size())
            sprintf(str, ".RVDiff");
        else
            sprintf(str, ".%d", atts.GetCovariateReturnYears()[yr]);
        nm += str;
    }
    
    if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        sprintf(str, ".annual_%s.txt", varNm);
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
    {
        const char *m[12] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        sprintf(str, ".%s_%s.txt", m[idx], varNm);
    }
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
    {
        const char *s[4] = {"WINTER", "SPRING", "SUMMER", "FALL"};
        sprintf(str, ".%s_%s.txt", s[idx], varNm);
    }
    
    nm += str;
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

    if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
    {
        daysPerYear = atts.GetDaysPerYear();
    }
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL||
             atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
    {
        daysPerYear = 0;
        for (int i = 0; i < 12; i++)
        {
            daysPerYear += atts.GetDaysPerMonth()[i];
            dayCountAtMonthEnd[i] = daysPerYear;
        }
    }

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
    numYears = numTimes/daysPerYear;

    int dsCycle = (float)GetInput()->GetInfo().GetAttributes().GetCycle();
#ifdef PARALLEL
    cycle0 = UnifyMinimumValue(dsCycle);
#else
    cycle0 = dsCycle;
#endif

    if (atts.GetDataAnalysisYearRangeEnabled())
    {
        numYears = atts.GetDataAnalysisYear2()-atts.GetDataAnalysisYear1()+1;
        numTimes = numYears*daysPerYear;
    }

    if (atts.GetEnsemble())
    {
        numTimes *= atts.GetNumEnsembles();
    }

    //cout<<"numTimes = "<<numTimes<<" : numYears = "<<numYears<<" daysPerYear= "<<daysPerYear<<endl;


    //How to compute maxes.
    //Monthly maxes.
    if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
        numBins = 12;
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
        numBins = 12; //DRP4;
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        numBins = 1;

    values.resize(numTuples);
    for (int i = 0; i < numTuples; i++)
        values[i].resize(numBins);

    //cout<<"values:"<<values.size()<<" x "<<values[0].size()<<" x "<<values[0][0].size()<<endl;
    //cout<<"values["<<numTuples<<"]["<<numBins<<"][numTimes]"<<endl;
    
    pair<int,int> locs = LocationsForRank(PAR_Rank());
    idx0 = locs.first;
    idxN = locs.second;
    
    if (atts.GetDumpData())
    {
        vtkRectilinearGrid *rg = vtkRectilinearGrid::SafeDownCast(inDS);
        if (inDS->GetDataObjectType() != VTK_RECTILINEAR_GRID || rg == NULL)
        {
            EXCEPTION1(ImproperUseException, "Can't output non-Rectilinear grid data.");
        }

        coordinates[0].resize(rg->GetDimensions()[0]);
        coordinates[1].resize(rg->GetDimensions()[1]);
        coordinates[2].resize(rg->GetDimensions()[2]);

        vtkDataArray *x = rg->GetXCoordinates();
        vtkDataArray *y = rg->GetYCoordinates();
        vtkDataArray *z = rg->GetZCoordinates();

        for (int i = 0; i < coordinates[0].size(); i++)
            coordinates[0][i] = x->GetTuple1(i);
        for (int i = 0; i < coordinates[1].size(); i++)
            coordinates[1][i] = y->GetTuple1(i);
        for (int i = 0; i < coordinates[2].size(); i++)
            coordinates[2][i] = z->GetTuple1(i);
    }

    GenerateOutputInfo();
    initialized = true;
    delete [] leaves;
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
    int t1 = visitTimer->StartTimer();

    if (!atts.GetEnsemble() && atts.GetDataAnalysisYearRangeEnabled())
    {
        int currYear = atts.GetDataYearBegin()+(currentTime/daysPerYear);
        if (currYear < atts.GetDataAnalysisYear1() ||
            currYear > atts.GetDataAnalysisYear2())
        {
            //cout<<"Skipping "<<currentTime<<" : year= "<<currYear<<endl;
            return;
        }
    }
    
    int nleaves;
    vtkDataSet **leaves = GetInputDataTree()->GetAllLeaves(nleaves);
    vtkDataSet *ds = leaves[0];
    vtkFloatArray *scalars = NULL;

    if (nodeCenteredData)
        scalars = (vtkFloatArray *)ds->GetPointData()->GetScalars();
    else
        scalars = (vtkFloatArray *)ds->GetCellData()->GetScalars();
    float *vals = (float *) scalars->GetVoidPointer(0);

    float dsTime = (float)GetInput()->GetInfo().GetAttributes().GetTime();
    int dsCycle = (float)GetInput()->GetInfo().GetAttributes().GetCycle();

    int timeNow = dsCycle - cycle0;

    int nTuples = scalars->GetNumberOfTuples();
    int index = GetIndexFromDay(timeNow); //currentTime);

    GetYearFromDay(timeNow); //currentTime);
    
    float scaling = atts.GetDataScaling(), cutoff = atts.GetCutoff();
    //cout<<"processing "<<currentTime<<" sv = "<<scaling<<" index= "<<index<<" nTuples= "<<nTuples<<endl;
    //cout<<"Processing: ct= "<<currentTime<<" dsTime= "<<dsTime<<" dsCycle= "<<dsCycle<<endl;
    //cout<<PAR_Rank()<<" currentTime= "<<currentTime<<" cycle0= "<<cycle0<<" timeNow= "<<timeNow<<endl;

    if (atts.GetCutoffMode() == PeaksOverThresholdAttributes::UPPER_TAIL)
    {
        for (int i = 0; i < nTuples; i++)
        {
            float v = vals[i]*scaling;
            if (v > cutoff)
                values[i][index].push_back(sample(v, timeNow)); //currentTime, dsTime));
        }
    }
    else
    {
        for (int i = 0; i < nTuples; i++)
        {
            float v = vals[i]*scaling;
            if (v < cutoff)
                values[i][index].push_back(sample(v,timeNow)); //currentTime, dsTime));
        }
    }

    debug1<<"Processing "<<timeNow<<" index= "<<index<<" loc0 sz= "<<values[0][0].size()<<endl;
    visitTimer->StopTimer(t1, "RPOT::Execute");
}

void
avtRPOTFilter::ExchangeData0()
{
#if PARALLEL
    float *tmp = new float[numTimes];
    float *res = new float[numTimes];
    int *flags = new int[numTimes];
    int *flagsRes = new int[numTimes];
    float *times = new float[numTimes];
    float *timesRes = new float[numTimes];
    
    for (int i=0; i<numTuples; i++)
    {
        for (int t=0; t<numTimes; t++)
        {
            tmp[t] = 0.0;//-numeric_limits<float>::min();
            flags[t] = 0;
            times[t] = 0.0;
        }
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t=0; t<nt; t++)
            {
                tmp[values[i][b][t].Cycle] = values[i][b][t].val;
                flags[values[i][b][t].Cycle] = 1;
                times[values[i][b][t].Cycle] = values[i][b][t].Cycle;
            }
        }
        MPI_Allreduce(tmp, res, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(times, timesRes, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
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
                    values[i][b].push_back(sample(res[t], t, timesRes[t]));
                }
            }
        }
    }
    
    delete [] tmp;
    delete [] res;
    delete [] flags;
    delete [] flagsRes;
#endif
}

void
avtRPOTFilter::ExchangeData()
{
#if PARALLEL
    //Determine how many samples need to be communicated.
    int nRanks = PAR_Size();
    int rank = PAR_Rank();
    int *localSenders = new int[nRanks];
    int *localMaxCnt = new int[nRanks];
    for (int i = 0; i < nRanks; i++)
        localSenders[i] = localMaxCnt[i] = 0;

    //Figure out the buffer sizes.
    for (int i = 0; i < nRanks; i++)
    {
        if (rank == i)
            continue;
        
        pair<int,int> locs = LocationsForRank(i);

        int cnt = 1; //For number of locs.
        for (int j = locs.first; j < locs.second; j++)
        {
            cnt++; // location.
            for (int b = 0; b < numBins; b++)
            {
                cnt++; //number in this bin.
                cnt += 2*values[j][b].size(); // num of val/cycles.
            }
        }
        if (cnt > 1)
        {
            localSenders[i] = 1;
            localMaxCnt[i] = cnt;
        }
    }
    
    // This tells each rank how many senders, and the max send buffer size.
    int *globalSenders = new int[nRanks];
    int *globalMaxCnt = new int[nRanks];
    MPI_Allreduce(localSenders, globalSenders, nRanks, MPI_INT, MPI_SUM, VISIT_MPI_COMM);
    MPI_Allreduce(localMaxCnt, globalMaxCnt, nRanks, MPI_INT, MPI_MAX, VISIT_MPI_COMM);
    
    if (rank == 0)
        for (int i = 0; i < nRanks; i++)
            debug1<<i<<": "<<globalSenders[i]<<" maxSz= "<<globalMaxCnt[i]<<endl;

    //Post recvs.
    int tag = 10001;
    int numToRecv = globalSenders[rank];
    MPI_Request *rReqs = new MPI_Request[numToRecv];
    float **rBuffs = new float*[numToRecv];
    int buffSz = globalMaxCnt[rank];
    for (int i = 0; i < numToRecv; i++)
    {
        rBuffs[i] = new float[buffSz];
        int x = MPI_Irecv(rBuffs[i], buffSz, MPI_FLOAT, MPI_ANY_SOURCE, tag, VISIT_MPI_COMM, &rReqs[i]);
        debug1<<rank<<": postRecv of sz= "<<buffSz<<" ret= "<<x<<endl;
    }
    
    //Post sends.
    int numToSend = 0;
    for (int i = 0; i < nRanks; i++)
        if (localSenders[i] > 0)
            numToSend++;

    MPI_Request *sReqs = new MPI_Request[numToSend];
    float **sBuffs = new float*[numToSend];
    int sCnt = 0;
    for (int r = 0; r < nRanks; r++)
    {
        if (r == rank || localSenders[r] == 0)
            continue;
        
        int buffSz = localMaxCnt[r];
        sBuffs[sCnt] = new float[buffSz];

        pair<int,int> locs = LocationsForRank(r);
        int cnt = 0;
        sBuffs[sCnt][cnt++] = (locs.second-locs.first);
        debug1<<rank<<": Send nLocs= "<<sBuffs[sCnt][0];
        for (int i = locs.first; i < locs.second; i++)
        {
            sBuffs[sCnt][cnt++] = i; //location.
            int numVals = 0;
            for (int b = 0; b < numBins; b++)
                numVals += values[i][b].size();
            sBuffs[sCnt][cnt++] = numVals;
            debug1<<" ("<<i<<","<<numVals<<") ";
            
            for (int b = 0; b < numBins; b++)
            {
                int nt = values[i][b].size();
                for (int t = 0; t < nt; t++)
                {
                    sBuffs[sCnt][cnt++] = values[i][b][t].Cycle;
                    sBuffs[sCnt][cnt++] = values[i][b][t].val;
                }
            }
        }
        debug1<<"CNT= "<<cnt<<endl;
        
        int x = MPI_Isend(sBuffs[sCnt], cnt, MPI_FLOAT, r, tag, VISIT_MPI_COMM, &sReqs[sCnt]);
        debug1<<"send "<<cnt<<" buffSz= "<<buffSz<<endl;
        debug1<<rank<<" ==> "<<r<<" buffSz= "<<buffSz<<" ret= "<<x<<endl;
        
        sCnt++;
    }

    MPI_Status *sendStats = new MPI_Status[numToSend];
    MPI_Status *recvStats = new MPI_Status[numToRecv];
    int ns = numToSend, nr = numToRecv;
    
    debug1<<endl<<endl;
    debug1<<"while: ns= "<<ns<<" nr= "<<nr<<endl;
    while (ns > 0 || nr > 0)
    {
        if (nr > 0)
        {
            int *idx = new int[numToRecv];
            MPI_Status *status = new MPI_Status[numToRecv];
            int num;
            MPI_Testsome(numToRecv, rReqs, &num, idx, status);
            if (num > 0)
            {
                debug1<<"RECV COMPLETE num= "<<num<<endl;
                nr -= num;
                for (int i = 0; i < num; i++)
                {
                    float *buff = rBuffs[idx[i]];
                    int nLocs = (int)buff[0];
                    int cnt = 1;
                    for (int j = 0; j < nLocs; j++)
                    {
                        int loc = (int)buff[cnt++];
                        int nt = (int)buff[cnt++];
                        debug1<<" ("<<loc<<","<<nt<<") ";
                        for (int t = 0; t < nt; t++)
                        {
                            int cycle = buff[cnt++];
                            float val = buff[cnt++];
                            int index = GetIndexFromDay(cycle);
                            values[loc][index].push_back(sample(val, cycle));
                            //if (cnt < 200)
                            //debug1<<" "<<cycle<<" "<<val<<" idx= "<<index<<endl;
                        }
                    }
                    delete [] buff;
                    rBuffs[idx[i]] = NULL;
                }
                debug1<<endl;
            }
            delete [] idx;
            delete [] status;
        }

        if (ns > 0)
        {
            int *idx = new int[numToSend];
            MPI_Status *status = new MPI_Status[numToSend];
            int num;
            MPI_Testsome(numToSend, sReqs, &num, idx, status);
            if (num > 0)
            {
                debug1<<"SEND COMPLETE num= "<<num<<endl;
                ns -= num;
                for (int i = 0; i < num; i++)
                {
                    float *buff = sBuffs[idx[i]];
                    delete [] buff;
                    sBuffs[idx[i]] = NULL;
                }
            }
            delete [] idx;
            delete [] status;
        }
    }
    delete [] sendStats;
    delete [] recvStats;
    delete [] sReqs;
    delete [] sBuffs;
    delete [] rReqs;
    delete [] rBuffs;
    delete [] localSenders;
    delete [] localMaxCnt;
#endif
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
    //avtCallback::ResetTimeout(0);

    int t1 = visitTimer->StartTimer();
    ExchangeData();
    visitTimer->StopTimer(t1, "RPOT::FinalOutput exchange data 1");

    int t2 = visitTimer->StartTimer();
    vtkRInterface *RI = vtkRInterface::New();

    double *outputData = new double[(idxN-idx0)*outputValsPerLoc];
    for (int i = 0; i < (idxN-idx0)*outputValsPerLoc; i++)
        outputData[i] = -1.0;

    for (int i = 0; i < (idxN-idx0); i++)
    {
        vector<float> tresholds = CalculateThresholds(i+idx0);
        
        vtkDoubleArray *exceedences = vtkDoubleArray::New();
        vtkIntArray *dayIndices = vtkIntArray::New();
        vtkIntArray *yearIndices = vtkIntArray::New();
        vtkIntArray *monthIndices = vtkIntArray::New();

        SetExceedenceData(i+idx0, tresholds, exceedences, dayIndices, yearIndices, monthIndices);
        
        RI->AssignVTKDataArrayToRVariable(exceedences, exceedencesStr.c_str());
        RI->AssignVTKDataArrayToRVariable(yearIndices, yearIndicesStr.c_str());
        RI->AssignVTKDataArrayToRVariable(monthIndices, monthIndicesStr.c_str());
        RI->AssignVTKDataArrayToRVariable(dayIndices, dayIndicesStr.c_str());

        string command = GenerateRCommand(tresholds, i+idx0);
        if (atts.GetDumpDebug())
        {
            cout<<"*************************************************************"<<endl;
            cout<<"location= "<<(idx0+i)<<endl;
            cout<<command;
            cout<<"*************************************************************"<<endl;
        }

        RI->EvalRscript(command.c_str());

        exceedences->Delete();
        dayIndices->Delete();
        yearIndices->Delete();
        monthIndices->Delete();

        int nVars = outputInfo.size();
        //PrintOutputInfo();
        for (int j = 0; j < outputInfo.size(); j++)
        {
            vtkDoubleArray *outArr = NULL;
            outArr = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray(outputInfo[j].name.c_str()));
            //cout<<outputInfo[j].name<<" : "<<outArr->GetNumberOfComponents()<<" x "<<outArr->GetNumberOfTuples()<<endl;
            for (int k = 0; k < outArr->GetNumberOfTuples(); k++)
            {
                outputData[i*outputValsPerLoc + outputInfo[j].index + k] = outArr->GetTuple1(k);
                //cout<<"    outputData["<<i*outputValsPerLoc + outputInfo[j].index + k<<"] = "<<outArr->GetTuple1(k)<<endl;
            }
        }
    }

    visitTimer->StopTimer(t2, "RPOT::FinalOutput Run Rcode");


    int t3 = visitTimer->StartTimer();
#if PARALLEL
#if 1
    int nVals = (idxN-idx0)*outputValsPerLoc;
    
    if (PAR_Rank() == 0)
    {
        int sz = numTuples*outputValsPerLoc;
        double *result = new double[sz];

        int *recvCnts = new int[PAR_Size()];
        int *displs = new int[PAR_Size()];

        int disp = 0;
        for (int r = 0; r < PAR_Size(); r++)
        {
            pair<int,int> locs = LocationsForRank(r);
            recvCnts[r] = (locs.second-locs.first)*outputValsPerLoc;
            displs[r] = disp;
            disp += recvCnts[r];
        }

        MPI_Gatherv(outputData, nVals, MPI_DOUBLE, result, recvCnts, displs, MPI_DOUBLE, 0, VISIT_MPI_COMM);
        delete [] recvCnts;
        delete [] displs;

        delete [] outputData;
        outputData = result;
    }
    else
    {
        MPI_Gatherv(outputData, nVals, MPI_DOUBLE, NULL, NULL, NULL, MPI_DOUBLE, 0, VISIT_MPI_COMM);
    }

#endif

#if 0
    int sz = numTuples*outputValsPerLoc;
    
    double *in = new double[sz], *sum = new double[sz];
    for (int i = 0; i < sz; i++)
    {
        in[i] = 0.0;
        sum[i] = 0.0;
    }
    
    for (int i=0; i<(idxN-idx0); i++)
    {
        for (int j = 0; j < outputValsPerLoc; j++)
        {
            in[(i+idx0)*outputValsPerLoc +j] = outputData[i*outputValsPerLoc+j];
        }
    }
    SumDoubleArray(in, sum, sz);

    delete outputData;
    outputData = new double[sz];
    for (int i = 0; i < sz; i++)
        outputData[i] = sum[i];
    delete [] in;
    delete [] sum;
#endif
#endif

    if (PAR_Rank() == 0 && atts.GetDumpData())
    {
        //PrintOutputInfo();
        vector<string> meshDimNms;
        vector<vector<double> > meshDims;
        vector<POTFilterWriteData::varInfo> vars;
        vector<int> arrShape;

        //TODO: These should be swapped in the writer...
        meshDimNms.push_back("Lat");        
        meshDimNms.push_back("Lon");
        meshDimNms.push_back("Aggregation");
        meshDims.resize(3);
        meshDims[0] = coordinates[1];
        meshDims[1] = coordinates[0];
        for (int i = 0; i < numBins; i++)
            meshDims[2].push_back(i);
        int nVars = outputInfo.size();
        int nLocs = meshDims[0].size()*meshDims[1].size();
        int nVals = nLocs * outputValsPerLoc;
        
        int idx = 0;
        vector<string> varnames;
        for (int i = 0; i < nVars; i++)
            varnames.push_back(outputInfo[i].name);
        
        //cout<<"DUMP: nVars= "<<nVars<<" nVals= "<<numTuples*outputValsPerLoc<<endl;
        POTFilterWriteData::writeNETCDFData("POT.output.nc",
                                            meshDimNms, meshDims,
                                            varnames, outputData);
    }
    
    //Set the return dataset.
    if (PAR_Rank() == 0)
    {
        int displayIdx = 0;
        if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
            displayIdx = 0;
        else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplayMonth() - (int)(PeaksOverThresholdAttributes::JAN);
        else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplaySeason() - (int)(PeaksOverThresholdAttributes::WINTER);
        
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
            outVar->SetValue(i, outputData[i*outputValsPerLoc + displayIdx]);
    
        if (nodeCenteredData)
            outDS->GetPointData()->SetScalars(outVar);
        else
            outDS->GetPointData()->SetScalars(outVar);
        
        outVar->Delete();

        avtDataTree_p outputTree = new avtDataTree(outDS, 0);
        SetOutputDataTree(outputTree);
        outDS->Delete();
    }
    else
        SetOutputDataTree(new avtDataTree());

    RI->Delete();
    visitTimer->StopTimer(t3, "RPOT::FinalOutput exchange data 2");

    //avtCallback::ResetTimeout(5*60);
    //avtCallback::ResetTimeout(0);
}

#if 0

    if (0)
    {
        vector<string> meshDimNms;
        vector<vector<double> > meshDims;
        vector<POTFilterWriteData::varInfo> vars;
        vector<int> arrShape;
        vtkDoubleArray *array = vtkDoubleArray::New();
        
        meshDimNms.push_back("lat");
        meshDimNms.push_back("lon");
        meshDimNms.push_back("index");
        int nx = 4, ny = 5, nz = 1;
        meshDims.push_back(makeArray(nx));
        meshDims.push_back(makeArray(ny));
        meshDims.push_back(makeArray(nz));
        
        int nVals = nx*ny*nz;
        int nVars = 2;
        arrShape.push_back(nVals);
        arrShape.push_back(nVars);
        
        POTFilterWriteData::varInfo v0, v1;
        v0.name = "var1";
        v0.dims.push_back("lat");
        v0.dims.push_back("lon");
        v0.indices.push_back(0);

        v1.name = "var2";
        v1.dims.push_back("lat");
        v1.dims.push_back("lon");
        v1.indices.push_back(1);
        vars.push_back(v0);
        vars.push_back(v1);
        
        vector<double> d = makeArray(nVals*nVars);
        array->SetNumberOfTuples(d.size());
        for (int i = 0; i < d.size(); i++)
            array->SetTuple1(i, d[i]);
        
        POTFilterWriteData::writeNETCDFData("output.nc",
                                            meshDimNms, meshDims,
                                            vars, arrShape, array);

        SetOutputDataTree(new avtDataTree());
        return;
    }
    


    //Exchange data....
    if (atts.GetEnsemble())
    {
        numTimes *= atts.GetNumEnsembles();
    }
#ifdef PARALLEL
    float *tmp = new float[numTimes];
    float *res = new float[numTimes];
    int *flags = new int[numTimes];
    int *flagsRes = new int[numTimes];
    float *times = new float[numTimes];
    float *timesRes = new float[numTimes];
    
    for (int i=0; i<numTuples; i++)
    {
        for (int t=0; t<numTimes; t++)
        {
            tmp[t] = 0.0;//-numeric_limits<float>::min();
            flags[t] = 0;
            times[t] = 0.0;
        }
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t=0; t<nt; t++)
            {
                //if(PAR_Rank()== 0 ) cout<<values[i][b][t].time<<endl;
                tmp[values[i][b][t].Cycle] = values[i][b][t].val;
                flags[values[i][b][t].Cycle] = 1;
                times[values[i][b][t].Cycle] = values[i][b][t].Time;
            }
        }
        MPI_Allreduce(tmp, res, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
        MPI_Allreduce(times, timesRes, numTimes, MPI_FLOAT, MPI_SUM, VISIT_MPI_COMM);
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
                    values[i][b].push_back(sample(res[t], t, timesRes[t]));
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
    
    int nValsPerOut = 1;
    if (atts.GetComputeCovariates())
    {
        nValsPerOut *= atts.GetCovariateReturnYears().size();
        if (atts.GetComputeRVDifferences())
            nValsPerOut++;
    }
    
    vector<vector<vector<float> > > outputs_rv, outputs_serv;
    outputs_rv.resize(nValsPerOut);
    outputs_serv.resize(nValsPerOut);
    for (int n = 0; n < nValsPerOut; n++)
    {
        outputs_rv[n].resize(numBins);
        outputs_serv[n].resize(numBins);
        for (int b = 0; b < numBins; b++)
        {
            outputs_rv[n][b].resize(numTuples);
            outputs_serv[n][b].resize(numTuples);
        }
    }

    vector<vector<vector<float> > > outputs_mle, outputs_semle;
    if (atts.GetComputeParamValues())
    {
        outputs_mle.resize(4);
        outputs_semle.resize(4);
        for (int n = 0; n < 4; n++)
        {
            outputs_mle[n].resize(numBins);
            outputs_semle[n].resize(numBins);
            for (int b = 0; b < numBins; b++)
            {
                outputs_mle[n][b].resize(numTuples);
                outputs_semle[n][b].resize(numTuples);
            }
        }
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
            if (atts.GetCutoffMode() == PeaksOverThresholdAttributes::UPPER_TAIL)
            {
                for (int t = 0; t < nt; t++)
                    if (values[i][b][t].val > thresholds[b])
                        numExceedences++;
            }
            else
            {
                for (int t = 0; t < nt; t++)
                    if (values[i][b][t].val < thresholds[b])
                        numExceedences++;
            }
        }
        
        //cout<<"numExceedences: "<<numExceedences<<endl;
        vtkDoubleArray *exceedences = vtkDoubleArray::New();
        vtkIntArray *dayIndices = vtkIntArray::New();
        vtkIntArray *monthIndices = vtkIntArray::New();
        vtkIntArray *yearIndices = vtkIntArray::New();
        exceedences->SetNumberOfComponents(1);
        exceedences->SetNumberOfTuples(numExceedences);
        dayIndices->SetNumberOfComponents(1);
        dayIndices->SetNumberOfTuples(numExceedences);
        monthIndices->SetNumberOfComponents(1);
        monthIndices->SetNumberOfTuples(numExceedences);
        yearIndices->SetNumberOfComponents(1);
        yearIndices->SetNumberOfTuples(numExceedences);
        
        int idx = 0;
        for (int b = 0; b < numBins; b++)
        {
            int nt = values[i][b].size();
            for (int t = 0; t < nt; t++)
            {
                if (values[i][b][t].val > thresholds[b])
                {
                    exceedences->SetValue(idx, values[i][b][t].val);
                    dayIndices->SetValue(idx, values[i][b][t].Time+1);
                    if (atts.GetAggregation() != PeaksOverThresholdAttributes::ANNUAL)
                        monthIndices->SetValue(idx, GetMonthFromDay(values[i][b][t].Time)+1);
                    yearIndices->SetValue(idx, GetYearFromDay(values[i][b][t].Time));//+1);
                 
                    idx++;
                    if (idx == numExceedences)
                        break;
                }
            }
        }
        RI->AssignVTKDataArrayToRVariable(exceedences, "exceedences");
        RI->AssignVTKDataArrayToRVariable(dayIndices, "dayIndices");
        RI->AssignVTKDataArrayToRVariable(monthIndices, "monthIndices");
        string threshStr, aggrStr;
        if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        {
            aggrStr = "'annual'";
            char str[32];
            sprintf(str, "%f", thresholds[0]);
            threshStr = str;
        }
        else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
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
        else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
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
        
        int nCovariates = 0;
        string yearIndicesNm = "NULL", monthIndicesNm = "NULL";
        string covByYear = "NULL", newDataStr = "NULL", rvDiffStr = "NULL";
        string useLocModelStr = "NULL", useShapeModelStr = "NULL", useScaleModelStr = "NULL";
        string returnParamsStr = "FALSE";

        if (atts.GetComputeParamValues())
            returnParamsStr = "TRUE";
        if (atts.GetAggregation() != PeaksOverThresholdAttributes::ANNUAL)
            monthIndicesNm = "monthIndices";
        
        if (atts.GetComputeCovariates())
        {
            nCovariates = 1;
            yearIndicesNm = "yearIndices";
            covByYear = "covByYear";
            
            vtkIntArray *covByYearArr = vtkIntArray::New();
            covByYearArr->SetNumberOfComponents(1);
            
            if (0)//atts.GetEnsemble())
            {
                int N = numYears * atts.GetNumEnsembles();
                covByYearArr->SetNumberOfTuples(N);
                int idx = 0;
                for (int e = 0; e < atts.GetNumEnsembles(); e++)
                {
                    for (int y = 0; y < numYears; y++, idx++)
                    {
                        covByYearArr->SetValue(idx, y + atts.GetDataYearBegin());
                    }
                }
            }
            else
            {
                covByYearArr->SetNumberOfTuples(numYears);
                for (int y = 0; y < numYears; y++)
                    covByYearArr->SetValue(y, y + atts.GetDataYearBegin());
            }
            RI->AssignVTKDataArrayToRVariable(yearIndices, yearIndicesNm.c_str());
            RI->AssignVTKDataArrayToRVariable(covByYearArr, covByYear.c_str());
            covByYearArr->Delete();
            
            newDataStr = "newData";
            vtkIntArray *newData = vtkIntArray::New();
            newData->SetNumberOfComponents(1);
            newData->SetNumberOfTuples(atts.GetCovariateReturnYears().size());
            for (int y = 0; y < atts.GetCovariateReturnYears().size(); y++)
            {
                newData->SetValue(y, atts.GetCovariateReturnYears()[y]);//-atts.GetDataYearBegin() +1);
            }
            RI->AssignVTKDataArrayToRVariable(newData, newDataStr.c_str());
            newData->Delete();

            if (atts.GetComputeRVDifferences())
            {
                rvDiffStr = "rvDiff";
                vtkIntArray *rvDiff = vtkIntArray::New();
                rvDiff->SetNumberOfComponents(1);
                rvDiff->SetNumberOfTuples(2);
                rvDiff->SetValue(0, atts.GetRvDifference1());//-atts.GetDataYearBegin() +1);
                rvDiff->SetValue(1, atts.GetRvDifference2());//-atts.GetDataYearBegin() +1);
                RI->AssignVTKDataArrayToRVariable(rvDiff, rvDiffStr.c_str());
                rvDiff->Delete();
            }
            if (atts.GetCovariateModelLocation())
                useLocModelStr = "1";
            if (atts.GetCovariateModelShape())
                useShapeModelStr = "1";
            if (atts.GetCovariateModelScale())
                useScaleModelStr = "1";

        }
        string dumpStr0 = "", dumpStr1 = "";
        if (1)
        {
            char tmp[128];
            sprintf(tmp, "save.image(file='input_%d.RData')\n", PAR_Rank());
            dumpStr0 = tmp;
            sprintf(tmp, "save.image(file='output_%d.RData')\n", PAR_Rank());
            dumpStr1 = tmp;
        }
        string outputStr = "rv = output$returnValue\n";
        outputStr += "se_rv = output$se.returnValue\n";
        if (atts.GetComputeCovariates() && atts.GetComputeRVDifferences())
        {
            outputStr += "rvDiff = output$returnValueDiff;\n";
            outputStr += "se_rvDiff = output$se.returnValueDiff;\n";
        }
        if (atts.GetComputeParamValues())
        {
            outputStr += "mle = output$mle;\n";
            outputStr += "se_mle = output$se.mle;\n";
        }
        string optimMethod = (atts.GetOptimizationMethod() == PeaksOverThresholdAttributes::NELDER_MEAD ?
                              "Nelder-Mead" : "BFGS");
        string upper_tail = (atts.GetCutoffMode() == PeaksOverThresholdAttributes::UPPER_TAIL ?
                             "TRUE" : "FALSE");
        string multiDayEventHandling = (atts.GetNoConsecutiveDay() ? "'norun'" : "NULL");
        char tmp[64];
        sprintf(tmp, "%d", atts.GetDataYearBegin());
        string initialYear = tmp;

        string numPerYearStr;
        if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        {
            sprintf(tmp, "%d", atts.GetDaysPerYear());
            numPerYearStr = tmp;
        }
        else
        {
            numPerYearStr = "c(";
            for (int m = 0; m < 12; m++)
            {
                sprintf(tmp, "%d", atts.GetDaysPerMonth()[m]);
                numPerYearStr = numPerYearStr + tmp;
                if (m != 11)
                    numPerYearStr = numPerYearStr + ",";
            }
            numPerYearStr = numPerYearStr + ")";
        }

        int nReplicates = 1;
        if (atts.GetEnsemble())
            nReplicates = atts.GetNumEnsembles();
        sprintf(potCmd,
                "%s" \
                "require(ismev)\n" \
                "output = potFit(data = exceedences, "\
                "day = dayIndices, month = %s, year=%s, initialYear=%s, "\
                "nYears=%d, threshold=%s, aggregation=%s, " \
                "numPerYear=%s, nReplicates=%d, " \
                "nCovariates=%d, covariatesByYear=%s, "\
                "locationModel=%s, scaleModel=%s, shapeModel=%s, "\
                "newData=%s, rvDifference=%s, " \
                "returnParams=%s, "\
                "optimMethod=%s, multiDayEventHandling=%s, upper.tail=%s" \
                ")\n"\
                "%s" \
                "%s",
                dumpStr0.c_str(),
                monthIndicesNm.c_str(),
                yearIndicesNm.c_str(), 
                initialYear.c_str(),
                numYears, threshStr.c_str(), aggrStr.c_str(),
                numPerYearStr.c_str(), nReplicates,
                nCovariates, covByYear.c_str(),
                useLocModelStr.c_str(), useShapeModelStr.c_str(), useScaleModelStr.c_str(),
                newDataStr.c_str(), rvDiffStr.c_str(),
                returnParamsStr.c_str(),
                optimMethod.c_str(), multiDayEventHandling.c_str(), upper_tail.c_str(),
                dumpStr1.c_str(),
                outputStr.c_str());
        
        string command = fileLoad;
        command += potCmd;
        cout<<command<<endl;
        RI->EvalRscript(command.c_str());

        cout<<"*************************************************************"<<endl;
        cout<<GenerateRCommand(i);
        cout<<"*************************************************************"<<endl;

        vtkDoubleArray *out_rv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rv"));
        vtkDoubleArray *out_serv = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_rv"));
        //cout<<"valsPerLoc: "<<nValsPerOut<<endl;
        //cout<<"out_rv  : "<<out_rv->GetNumberOfComponents()<<" x "<<out_rv->GetNumberOfTuples()<<endl;
        //cout<<"out_serv: "<<out_serv->GetNumberOfComponents()<<" x "<<out_serv->GetNumberOfTuples()<<endl;

        int N = nValsPerOut;
        if (atts.GetComputeRVDifferences())
            N--;
        
        for (int b = 0; b < numBins; b++)
        {
            for (int n = 0; n < N; n++)
            {
                outputs_rv[n][b][i] = (float)out_rv->GetComponent(b,n);
                outputs_serv[n][b][i] = (float)out_serv->GetComponent(b,n);
            }
        }
        
        vtkDoubleArray *out_rvDiff = NULL, *out_servDiff = NULL;
        if (atts.GetComputeCovariates() && atts.GetComputeRVDifferences())
        {
            out_rvDiff = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("rvDiff"));
            out_servDiff = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_rvDiff"));
            //cout<<"out_rvDiff  : "<<out_rvDiff->GetNumberOfComponents()<<" x "<<out_rvDiff->GetNumberOfTuples()<<endl;
            //cout<<"out_servDiff: "<<out_servDiff->GetNumberOfComponents()<<" x "<<out_servDiff->GetNumberOfTuples()<<endl;
            int N = nValsPerOut-1;
            for (int n = nValsPerOut-1; n < nValsPerOut; n++)
            {
                for (int b = 0; b < numBins; b++)
                {
                    outputs_rv[n][b][i] = (float)out_rvDiff->GetComponent(b,0);
                    outputs_serv[n][b][i] = (float)out_servDiff->GetComponent(b,0);
                }
            }
        }
        if (atts.GetComputeParamValues())
        {
            vtkDoubleArray *out_mle, *out_semle;
            out_mle = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("mle"));
            out_semle = vtkDoubleArray::SafeDownCast(RI->AssignRVariableToVTKDataArray("se_mle"));
            for (int n = 0; n < 4; n++)
            {
                for (int b = 0; b < numBins; b++)
                {
                    outputs_mle[n][b][i] = (float)out_mle->GetComponent(b,n);
                    outputs_semle[n][b][i] = (float)out_semle->GetComponent(b,n);
                }
            }
        }
        
        exceedences->Delete();
        dayIndices->Delete();
        monthIndices->Delete();
        yearIndices->Delete();
    }

#if PARALLEL
    float *in0 = new float[numTuples], *in1 = new float[numTuples];
    float *sum0 = new float[numTuples], *sum1 = new float[numTuples];
    int N = outputs_rv.size();
    
    for (int n = 0; n < N; n++)
        in0[n] = in1[n] = sum0[n] = sum1[n] = 0.0;
    for (int n = 0; n < N; n++)
    {
        for (int b = 0; b<numBins; b++)
        {
            for (int i=0; i<numTuples; i++)
            {
                if (i >= idx0 && i < idxN)
                {
                    in0[i] = outputs_rv[n][b][i];
                    in1[i] = outputs_serv[n][b][i];
                }
                else
                    in0[i] = in1[i] = 0.0f;
            }
            SumFloatArray(in0, sum0, numTuples);
            SumFloatArray(in1, sum1, numTuples);
            if (PAR_Rank() == 0)
            {
                for (int i=0; i<numTuples; i++)
                {
                    outputs_rv[n][b][i] = sum0[i];
                    outputs_serv[n][b][i] = sum1[i];
                }
            }
        }
    }
    if (atts.GetComputeParamValues())
    {
        for (int n = 0; n < N; n++)
            in0[n] = in1[n] = sum0[n] = sum1[n] = 0.0;
        N = outputs_mle.size();
        for (int n = 0; n < N; n++)
        {
            for (int b = 0; b<numBins; b++)
            {
                for (int i=0; i<numTuples; i++)
                {
                    if (i >= idx0 && i < idxN)
                    {
                        in0[i] = outputs_mle[n][b][i];
                        in1[i] = outputs_semle[n][b][i];
                    }
                    else
                        in0[i] = in1[i] = 0.0f;
                }
                SumFloatArray(in0, sum0, numTuples);
                SumFloatArray(in1, sum1, numTuples);
                if (PAR_Rank() == 0)
                {
                    for (int i=0; i<numTuples; i++)
                    {
                        outputs_mle[n][b][i] = sum0[i];
                        outputs_semle[n][b][i] = sum1[i];
                    }
                }
            }
        }
    }

    delete [] in0;
    delete [] in1;
    delete [] sum0;
    delete [] sum1;

#endif
    if (PAR_Rank() == 0)
    {
        int displayIdx = 0;
        if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
            displayIdx = 0;
        else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplayMonth() - (int)(PeaksOverThresholdAttributes::JAN);
        else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
            displayIdx = (int)atts.GetDisplaySeason() - (int)(PeaksOverThresholdAttributes::WINTER);
        
        //cout<<"Create output.....("<<numOutputComponents<<" x "<<numTuples<<")"<<endl;
        vtkFloatArray *outVar = vtkFloatArray::New();
        outVar->SetNumberOfComponents(1);
        outVar->SetNumberOfTuples(numTuples);
        int idx = 0;
        for (int i = 0; i < numTuples; i++, idx++)
        {
            outVar->SetValue(i, outputs_rv[0][displayIdx][i]);
        }
    
        if (nodeCenteredData)
            outDS->GetPointData()->SetScalars(outVar);
        else
            outDS->GetPointData()->SetScalars(outVar);
        
        outVar->Delete();

        avtDataTree_p outputTree = new avtDataTree(outDS, 0);
        SetOutputDataTree(outputTree);
        outDS->Delete();

        //cout<<"outputs: "<<outputs_rv.size()<<" "<<outputs_rv[0].size()<<" "<<outputs_rv[0][0].size()<<endl;
        if (atts.GetDumpData())
        {
            int N = outputs_rv.size();
            for (int b = 0; b < numBins; b++)
            {
                for (int n = 0; n < N; n++)
                {
                    string nm0 = GetDumpFileName(b, n, 0);
                    string nm1 = GetDumpFileName(b, n, 1);
                    ofstream ofile0(nm0.c_str());
                    ofstream ofile1(nm1.c_str());
                    //cout<<nm0<<": "<<N<<" x "<<numTuples<<endl;
                    //cout<<nm1<<": "<<N<<" x "<<numTuples<<endl;
                    for (int i = 0; i < numTuples; i++)
                    {
                        ofile0<<outputs_rv[n][b][i]<<" ";
                        ofile1<<outputs_serv[n][b][i]<<" ";
                    }
                    ofile0<<endl;
                    ofile1<<endl;
                }
            }
            if (atts.GetComputeParamValues())
            {
                int N = outputs_mle.size();
                for (int b = 0; b < numBins; b++)
                {
                    string nm0 = GetDumpFileName(b, -1, 2);
                    string nm1 = GetDumpFileName(b, -1, 3);
                    ofstream ofile0(nm0.c_str());
                    ofstream ofile1(nm1.c_str());
                    //cout<<nm0<<": "<<N<<" x "<<numTuples<<endl;
                    //cout<<nm1<<": "<<N<<" x "<<numTuples<<endl;
                    for (int i = 0; i < numTuples; i++)
                    {
                        for (int n = 0; n < N; n++)
                        {
                            //ofile0<<outputs_mle[n][b][i]<<" ";
                            //ofile1<<outputs_semle[n][b][i]<<" ";
                            ofile0<<outputs_mle[n][b][i]<<endl;
                            ofile1<<outputs_semle[n][b][i]<<endl;
                        }
                    }
                    ofile0<<endl;
                    ofile1<<endl;
                }
            }
        }
    }
    else
        SetOutputDataTree(new avtDataTree());

    avtCallback::ResetTimeout(5*60);
}
#endif


void
avtRPOTFilter::SetExceedenceData(int loc,
                                 const std::vector<float> &thresholds,
                                 vtkDoubleArray *exceedences,
                                 vtkIntArray *dayIndices,
                                 vtkIntArray *yearIndices,
                                 vtkIntArray *monthIndices)
{
    int numExceedences = 0;
    for (int b = 0; b < numBins; b++)
    {
        int nt = values[loc][b].size();
        if (atts.GetCutoffMode() == PeaksOverThresholdAttributes::UPPER_TAIL)
        {
            for (int t = 0; t < nt; t++)
                if (values[loc][b][t].val > thresholds[b])
                    numExceedences++;
        }
        else
        {
            for (int t = 0; t < nt; t++)
                if (values[loc][b][t].val < thresholds[b])
                    numExceedences++;
        }
    }

    exceedences->SetNumberOfComponents(1);
    exceedences->SetNumberOfTuples(numExceedences);
    dayIndices->SetNumberOfComponents(1);
    dayIndices->SetNumberOfTuples(numExceedences);
    monthIndices->SetNumberOfComponents(1);
    monthIndices->SetNumberOfTuples(numExceedences);
    yearIndices->SetNumberOfComponents(1);
    yearIndices->SetNumberOfTuples(numExceedences);

    int idx = 0;
    for (int b = 0; b < numBins; b++)
    {
        int nt = values[loc][b].size();
        for (int t = 0; t < nt; t++)
        {
            if ((atts.GetCutoffMode() == PeaksOverThresholdAttributes::UPPER_TAIL &&
                 values[loc][b][t].val > thresholds[b]) ||
                (atts.GetCutoffMode() == PeaksOverThresholdAttributes::LOWER_TAIL &&
                 values[loc][b][t].val < thresholds[b]))
            {
                exceedences->SetValue(idx, values[loc][b][t].val);
                dayIndices->SetValue(idx, values[loc][b][t].Cycle +1);
                if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY ||
                    atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
                {
                    monthIndices->SetValue(idx, GetMonthFromDay(values[loc][b][t].Cycle) +1);
                }
                else
                    monthIndices->SetValue(idx, -1);
                yearIndices->SetValue(idx, GetYearFromDay(values[loc][b][t].Cycle)+atts.GetDataYearBegin());

                idx++;
                if (idx == numExceedences)
                    break;
            }
        }
    }
}

void
avtRPOTFilter::GenerateOutputInfo()
{
    int index = 0;
    char tmp1[64], tmp2[64];

    if (!atts.GetComputeCovariates() && !atts.GetComputeCovariates())
    {
        outputInfo.push_back(outputType("rv", "output$returnValue", numBins, index));
        index += numBins;
        outputInfo.push_back(outputType("se_rv", "output$se.returnValue", numBins, index));
        index += numBins;
    }

    if (atts.GetComputeCovariates() && atts.GetComputeCovariates())
    {
        int nv = atts.GetCovariateReturnYears().size();
        for (int i = 0; i < nv; i++)
        {
            sprintf(tmp1, "rv_%d", (atts.GetCovariateReturnYears()[i]));
            if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
                sprintf(tmp2, "output$returnValue[%d]", (i+1));
            else
            {
                if (nv == 1)
                    sprintf(tmp2, "output$returnValue");
                else
                    sprintf(tmp2, "output$returnValue[%d,]", (i+1));
            }
            outputInfo.push_back(outputType(tmp1, tmp2, numBins, index));
            index += numBins;

            sprintf(tmp1, "se_rv_%d", (atts.GetCovariateReturnYears()[i]));
            if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
                sprintf(tmp2, "output$se.returnValue[%d]", (i+1));
            else
            {
                if (nv == 1)
                    sprintf(tmp2, "output$se.returnValue");
                else
                    sprintf(tmp2, "output$se.returnValue[%d,]", (i+1));             
            }
            outputInfo.push_back(outputType(tmp1, tmp2, numBins, index));
            index += numBins;
        }
    }
    
    if (atts.GetComputeCovariates() && atts.GetComputeRVDifferences())
    {
        sprintf(tmp1, "rvDiff_%d_%d", atts.GetRvDifference1(), atts.GetRvDifference2());
        outputInfo.push_back(outputType(tmp1, "output$returnValueDiff", numBins, index));
        index += numBins;

        sprintf(tmp1, "se_rvDiff_%d_%d", atts.GetRvDifference1(), atts.GetRvDifference2());
        outputInfo.push_back(outputType(tmp1, "output$se.returnValueDiff", numBins, index));
        index += numBins;
    }

    outputValsPerLoc = index;
}

void
avtRPOTFilter::PrintOutputInfo()
{
    cout<<"OUTPUTINFO: "<<endl;
    for (int i = 0; i < outputInfo.size(); i++)
    {
        cout<<i<<": "<<outputInfo[i].name<<" "<<outputInfo[i].rName<<" idx= "<<outputInfo[i].index<<" dim= "<<outputInfo[i].dim<<endl;
    }
}

string
avtRPOTFilter::GenerateRCommand(const vector<float> &t, int loc)
{
    string command;
    char tmp[512];

    if (atts.GetDumpDebug())
    {
        sprintf(tmp, "save.image(file='input_%d.RData')\n", loc);
        command += tmp;
    }

    //load files and packages.
    sprintf(tmp, "source('%s/auxil.r')\nsource('%s/pp.fit2.r')\nsource('%s/potVisit.r')\n",
            codeDir.c_str(), codeDir.c_str(), codeDir.c_str());
    command += tmp;
    command += "require(ismev)\n";
    command += "output = potFit(";
    if (atts.GetDataAnalysisYearRangeEnabled())
        sprintf(tmp, "initialYear=%d", atts.GetDataAnalysisYear1());
    else
        sprintf(tmp, "initialYear=%d", atts.GetDataYearBegin());
    command += tmp;

    sprintf(tmp, ", nYears=%d", numYears);
    command += tmp;
    
    if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
    {
        sprintf(tmp, ", numPerYear=%d", atts.GetDaysPerYear());
        command += tmp;
    }
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL ||
             atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
    {
        int *d = atts.GetDaysPerMonth();
        sprintf(tmp, ", numPerYear=c(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
                d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11]);
        command += tmp;
    }

    int nReplicates = 1;
    if (atts.GetEnsemble())
        nReplicates = atts.GetNumEnsembles();
    sprintf(tmp, ", nReplicates=%d", nReplicates);
    command += tmp;
    
    command += ", data=" + exceedencesStr;
    command += ", day=" + dayIndicesStr;
    command += ", year="+yearIndicesStr;
    command += ", month="+monthIndicesStr;
    
    if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        command += ", aggregation='annual'";
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
        command += ", aggregation='seasonal'";
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
        command += ", aggregation='monthly'";
    
    if (atts.GetAggregation() == PeaksOverThresholdAttributes::ANNUAL)
        sprintf(tmp, "%f", t[0]);
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::SEASONAL)
        sprintf(tmp, "c(%f, %f, %f, %f)", t[0], t[1], t[2], t[3]);
    else if (atts.GetAggregation() == PeaksOverThresholdAttributes::MONTHLY)
        sprintf(tmp, "c(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)", 
                t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7], t[8], t[9], t[10], t[11]);
    command += ", threshold=";
    command += tmp;
    
    if (atts.GetComputeCovariates())
    {
        command += ", nCovariates=1";
        command += ", covariatesByYear=c(";
        for (int y = 0; y < numYears; y++)
        {
            sprintf(tmp, "%d", y + atts.GetDataYearBegin());
            command += tmp;
            if (y < (numYears-1))
                command += ", ";
        }
        command += ")";
        command += ", newData=c(";
        int n = atts.GetCovariateReturnYears().size();
        for (int i = 0; i < n; i++)
        {
            sprintf(tmp, "%d", atts.GetCovariateReturnYears()[i]);
            command += tmp;
            if (i != (n-1))
                command += ",";
        }
        command += ")";

        if (atts.GetComputeRVDifferences())
        {
            sprintf(tmp, ", rvDifference=c(%d, %d)", atts.GetRvDifference1(), atts.GetRvDifference2());
            command += tmp;
        }
        
        if (atts.GetCovariateModelLocation())
            command += ", locationModel=1";
        if (atts.GetCovariateModelScale())
            command += ", scaleModel=1";
        if (atts.GetCovariateModelShape())
            command += ", shapeModel=1";

        if (atts.GetComputeParamValues())
            command += ", returnParams=TRUE";
    }

    if (atts.GetNoConsecutiveDay())
        command += ", multiDayEventHandling='norun'";
    if (atts.GetOptimizationMethod() == PeaksOverThresholdAttributes::NELDER_MEAD)
        command += ", optimMethod='Nelder-Mead'";
    else if (atts.GetOptimizationMethod() == PeaksOverThresholdAttributes::BFGS)
        command += ", optimMethod='BFGS'";
    if (atts.GetCutoffMode() == PeaksOverThresholdAttributes::UPPER_TAIL)
        command += ", upper.tail=TRUE";
    else
        command += ", upper.tail=FALSE";

    command += ")\n";

    for (int i = 0; i < outputInfo.size(); i++)
        command += outputInfo[i].name + "=" + outputInfo[i].rName + "\n";

    if (atts.GetDumpDebug())
    {
        sprintf(tmp, "save.image(file='output_%d.RData')\n", loc);
        command += tmp;
    }

    return command;
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


int
avtRPOTFilter::RankForLocation(int loc)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
    int nProcs = PAR_Size();

    int nSamplesPerProc = (numTuples / nProcs);
    int oneExtraUntil = (numTuples % nProcs);

    if (loc < (oneExtraUntil*(nSamplesPerProc+1)))
        return loc/(nSamplesPerProc+1);
    else
    {
        loc -= (oneExtraUntil*(nSamplesPerProc+1));
        return loc/nSamplesPerProc + oneExtraUntil;
    }
#else
    return 0;
#endif
}

std::pair<int,int>
avtRPOTFilter::LocationsForRank(int rank)
{
    int i0 = 0;
    int i1 = numTuples;
#ifdef PARALLEL
    int nProcs = PAR_Size();

    int nSamplesPerProc = (numTuples / nProcs);
    int oneExtraUntil = (numTuples % nProcs);
    if (rank < oneExtraUntil)
    {
        i0 = (rank)*(nSamplesPerProc+1);
        i1 = (rank+1)*(nSamplesPerProc+1);
    }
    else
    {
        i0 = (rank)*(nSamplesPerProc) + oneExtraUntil;
        i1 = (rank+1)*(nSamplesPerProc) + oneExtraUntil;
    }
#else
#endif
    
    pair<int, int> locs(i0,i1);
    return locs;
}
