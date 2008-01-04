/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                  avtAggregateChordLengthDistributionQuery.C               //
// ************************************************************************* //

#include <avtAggregateChordLengthDistributionQuery.h>

#include <stdio.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>

#include <avtLineScanFilter.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtAggregateChordLengthDistributionQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2006
//
// ****************************************************************************

avtAggregateChordLengthDistributionQuery::avtAggregateChordLengthDistributionQuery()
{
    numChords = new int[numBins];
}


// ****************************************************************************
//  Method: avtAggregateChordLengthDistributionQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2006
//
// ****************************************************************************

avtAggregateChordLengthDistributionQuery::~avtAggregateChordLengthDistributionQuery()
{
    delete [] numChords;;
}


// ****************************************************************************
//  Method: avtAggregateChordLengthDistributionQuery::PreExecute
//
//  Purpose:
//      Does some initialization work before the query executes.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2006
//
// ****************************************************************************

void
avtAggregateChordLengthDistributionQuery::PreExecute(void)
{
    avtLineScanQuery::PreExecute();

    delete [] numChords;
    numChords = new int[numBins];
    for (int i = 0 ; i < numBins ; i++)
        numChords[i] = 0;
}


// ****************************************************************************
//  Method: avtAggregateChordLengthDistributionQuery::PostExecute
//
//  Purpose:
//      Outputs the chord counts.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2006
//
// ****************************************************************************

void
avtAggregateChordLengthDistributionQuery::PostExecute(void)
{
    int   i;

    int times = 0;
    char name[1024];
    sprintf(name, "cld_a%d.ult", times++);

    if (PAR_Rank() == 0)
    {
        bool lookingForUnused = true;
        while (lookingForUnused)
        {
            ifstream ifile(name);
            if (ifile.fail())
                lookingForUnused = false;
            else
                sprintf(name, "cld_a%d.ult", times++);
        }
    }

    char msg[1024];
    sprintf(msg, "The aggregate chord length distribution has been outputted "
            "as an Ultra file (%s), which can then be imported into VisIt.", 
            name);
    SetResultMessage(msg);
    SetResultValue(0.);

    int *nc2 = new int[numBins];
    SumIntArrayAcrossAllProcessors(numChords, nc2, numBins);
    delete [] numChords;
    numChords = nc2;

    if (PAR_Rank() == 0)
    {
        double binWidth = (maxLength-minLength) / numBins;
        double totalArea = 0.;
        for (i = 0 ; i < numBins ; i++)
            totalArea += binWidth*numChords[i];
        if (totalArea == 0.)
        {
            sprintf(msg, "The chord length distribution could not be "
                    "calculated because none of the lines intersected the data"
                    " set.  If you have used a fairly large number of lines, "
                    "then this may be indicative of an error state.");
            SetResultMessage(msg);
            return;
        }

        ofstream ofile(name);
        if (ofile.fail())
        {
            sprintf(msg, "Unable to write out file containing distribution.");
            SetResultMessage(msg);
            return;
        }
        ofile << "# Chord length distribution (aggregate)" << endl;

        for (int i = 0 ; i < numBins ; i++)
        {
            //double x = minLength + (i+0.5)*binWidth;
            double x1 = minLength + (i)*binWidth;
            double x2 = minLength + (i+1)*binWidth;
            double y = numChords[i] / totalArea; // Make it be 
                            // a distribution ... the area under the curve: 1
            ofile << x1 << " " << y << endl;
            ofile << x2 << " " << y << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtAggregateChordLengthDistributionQuery::ExecuteLineScan
//
//  Purpose:
//      Examines the input data.  Note that the line scan filter will organize
//      the data so that all of the contributions from a given line will be in
//      the same vtkPolyData input.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2006
//
// ****************************************************************************

void
avtAggregateChordLengthDistributionQuery::ExecuteLineScan(vtkPolyData *pd)
{
    vtkIntArray *lineids = (vtkIntArray *) 
                                  pd->GetCellData()->GetArray("avtLineID");
    if (lineids == NULL)
        EXCEPTION0(ImproperUseException);
        
    int npts = pd->GetNumberOfPoints();
    vector<bool> usedPoint(npts, false);
    
    pd->BuildLinks();
    pd->BuildCells();

    // When we determine which line segments are on one side of another,
    // we need to examine the other segments.  But we only want to consider
    // segments that are from the same line (i.e. have the same lineid).
    // So we are using this data structure to keep track of which segments
    // come from which line.  (Instead of having to iterate through all the
    // other segments each time we examine a segment.)
    //
    // So why hash?
    // The lines are spread out over many processors.  So proc X may have
    // lines 100,000 - 100,500.  So we either have to have vectors that are
    // way too big (100,500 entries with the first 100,000 empty),
    // or we have to do some clever indexing.  So we are doing
    // some clever indexing.  In this case, hashing.
    int hashSize = 10000;
    vector< vector<int> >    hashed_lineid_lookup(hashSize);
    vector< vector<double> > hashed_segment_length(hashSize);

    for (int i = 0 ; i < npts ; i++)
    {
        if (usedPoint[i])
            continue;
        int seg1, seg2;
        int numMatches = GetCellsForPoint(i, pd, lineids, -1, seg1, seg2);
        if (numMatches == 0)
            continue;
        if (numMatches > 2)
        {
            // We found an error condition.  Give up on this line.  This
            // happens infrequently enough that it should not affect our
            // statistics.
            continue;
        }
        int oneSide = i;
        int otherSide = i;
        int lineid = lineids->GetValue(seg1);
        if (numMatches == 1)
        {
            oneSide   = i;
            otherSide = WalkChain(pd, i, seg1, usedPoint, lineids, lineid);
        }
        else if (numMatches == 2)
        {
            oneSide   = WalkChain(pd, i, seg1, usedPoint, lineids, lineid);
            otherSide = WalkChain(pd, i, seg2, usedPoint, lineids, lineid);
        }
        if (oneSide == -1 || otherSide == -1)
        {
            // We found an error condition.  Give up on this line.  This
            // happens infrequently enough that it should not affect our
            // statistics.
            continue;
        }
        double pt1[3];
        double pt2[3];
        pd->GetPoint(oneSide, pt1);
        pd->GetPoint(otherSide, pt2);
        double dist = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                           (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                           (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));
 
        int hashid = lineid % hashSize;
        hashed_lineid_lookup[hashid].push_back(lineid);
        hashed_segment_length[hashid].push_back(dist);
    }

    for (int i = 0 ; i < hashSize ; i++)
    {
        vector<int> already_considered;
        for (int j = 0 ; j < hashed_lineid_lookup[i].size() ; j++)
        {
             bool alreadyDoneLineId = false;
             int  k;
             for (k = 0 ; k < already_considered.size() ; k++)
                 if (hashed_lineid_lookup[i][j] == already_considered[k])
                     alreadyDoneLineId = true;
             if (alreadyDoneLineId)
                 continue;

             int lineid = hashed_lineid_lookup[i][j];
             already_considered.push_back(lineid);
             double length = 0.;
             for (k = j ; k < hashed_lineid_lookup[i].size() ; k++)
             {
                 if (hashed_lineid_lookup[i][k] == lineid)
                     length += hashed_segment_length[i][k];
             }

             int bin = (int)((length-minLength)/(maxLength-minLength) * numBins);
             if (bin < 0)
                 bin = 0;
             if (bin >= numBins)
                 bin = numBins-1;
             numChords[bin]++;
        }
    }
}


