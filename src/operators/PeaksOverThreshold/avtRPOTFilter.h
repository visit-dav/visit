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
//  File: avtRPOTFilter.h
// ************************************************************************* //

#ifndef AVT_R_EXTREMES_FILTER_H
#define AVT_R_EXTREMES_FILTER_H

#include <filters_exports.h>
#include <avtDatasetToDatasetFilter.h>
#include <avtTimeLoopFilter.h>
#include <PeaksOverThresholdAttributes.h>
#include <string>
#include <vector>

class vtkDataSet;
class vtkDoubleArray;
class vtkIntArray;

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

    std::string newVarName, codeDir;
    PeaksOverThresholdAttributes atts;
    
  protected:
    void            Initialize();
    virtual void            Execute();
    virtual void            CreateFinalOutput();
    virtual bool            ExecutionSuccessful();

    virtual bool            FilterSupportsTimeParallelization();
    virtual bool            DataCanBeParallelizedOverTime(void);

    void                    GenerateOutputInfo();
    void                    PrintOutputInfo();
    std::string             GenerateRCommand(const std::vector<float> &thresholds, int loc);
    void                    SetExceedenceData(int loc,
                                              const std::vector<float> &thresholds,
                                              vtkDoubleArray *exceedences,
                                              vtkIntArray *dayIndices, 
                                              vtkIntArray *yearIndices, 
                                              vtkIntArray *monthIndices);
    float                   CalculateThreshold(int loc, int arr);
    std::vector<float>      CalculateThresholds(int loc);
    int                     GetIndexFromDay(int t);
    int                     GetMonthFromDay(int t);
    int                     GetYearFromDay(int t);
    int                     GetSeasonFromDay(int t);
    std::string             CreateQuantileCommand(const char *var, const char *in, int aggregationIdx);
    std::string             GetDumpFileName(int idx, int yr, int var);

    vtkDataSet *outDS;
    std::vector<double> coordinates[3];
    int numTuples, numTimes, numYears, numBins;
    bool nodeCenteredData, initialized;
    int idx0, idxN;
    int cycle0;

    // array/data names.
    static std::string exceedencesStr;
    static std::string dayIndicesStr, yearIndicesStr, monthIndicesStr;

    class outputType
    {
    public:
        outputType(const std::string &nm, const std::string &rnm, int d, int i) : name(nm), rName(rnm), dim(d), index(i)
        {}
        outputType() {dim=1; index=0;}
        std::string name, rName;
        int dim, index;
    };
    std::vector<outputType> outputInfo;
    int outputValsPerLoc;
    
    class sample
    {
    public:
        sample() {val=-1; Cycle=-1; Time=-1;}
        sample(float v, int c, float t=-1.0f) {val=v; Cycle=c; Time=t;}
        float val, Time;
        int Cycle;
    };
    //values[location][aggregation][time_i]    
    std::vector<std::vector<std::vector<sample> > > values;
    //std::vector<sample> values;
    
    void DebugData(int loc, std::string nm);

    int daysPerYear, dayCountAtMonthEnd[12];

    int RankForLocation(int loc);
    std::pair<int,int> LocationsForRank(int rank);

    void ExchangeData();
    void ExchangeData0();
};

#endif
