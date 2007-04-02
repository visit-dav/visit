/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                    avtAggregateRayLengthDistributionQuery.C               //
// ************************************************************************* //

#include <avtAggregateRayLengthDistributionQuery.h>

#include <stdio.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkExecutive.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>

#include <avtLineScanFilter.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>
#include <avtWeightedVariableSummationQuery.h>

#include <DebugStream.h>
#include <VisItException.h>


// ****************************************************************************
//  Method: avtAggregateRayLengthDistributionQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

avtAggregateRayLengthDistributionQuery::avtAggregateRayLengthDistributionQuery()
{
    count = new double[numBins];
}


// ****************************************************************************
//  Method: avtAggregateRayLengthDistributionQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

avtAggregateRayLengthDistributionQuery::~avtAggregateRayLengthDistributionQuery()
{
    delete [] count;
}


// ****************************************************************************
//  Method: avtAggregateRayLengthDistributionQuery::PreExecute
//
//  Purpose:
//      Does some initialization work before the query executes.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

void
avtAggregateRayLengthDistributionQuery::PreExecute(void)
{
    avtLineScanQuery::PreExecute();

    if (minLength != 0.)
    {
        EXCEPTION1(VisItException, "This query only makes sense with minimum "
                     "lengths of 0.  Please try again with that length.");
    }

    delete [] count;
    count = new double[numBins];
    for (int i = 0 ; i < numBins ; i++)
        count[i] = 0.;
}


// ****************************************************************************
//  Method: avtAggregateRayLengthDistributionQuery::PostExecute
//
//  Purpose:
//      Outputs the probability distribution of encountering specific masses.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

void
avtAggregateRayLengthDistributionQuery::PostExecute(void)
{
    int   i;

    int times = 0;
    char name[1024];
    sprintf(name, "rld_a%d.ult", times++);

    if (PAR_Rank() == 0)
    {
        bool lookingForUnused = true;
        while (lookingForUnused)
        {
            ifstream ifile(name);
            if (ifile.fail())
                lookingForUnused = false;
            else
                sprintf(name, "rld_a%d.ult", times++);
        }
    }

    char msg[1024];
    sprintf(msg, "The ray length distribution "
                 "has been outputted as an "
                 "Ultra file (%s), which can then be imported into VisIt.",
                 name);
    SetResultMessage(msg);
    SetResultValue(0.);

    double *m2 = new double[numBins];
    SumDoubleArrayAcrossAllProcessors(count, m2, numBins);
    delete [] count;
    count = m2;

    double    totalCount = 0;
    for (i = 0 ; i < numBins ; i++)
        totalCount += count[i];

    if (PAR_Rank() == 0)
    {
        if (totalCount == 0.)
        {
            sprintf(msg, "The ray length distribution could not be calculated"
                    " because none of the lines intersected the data set."
                    "  If you have used a fairly large number of lines, then "
                    "this may be indicative of an error state.");
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
        ofile << "# Ray length distribution (aggregate)" << endl;
        double binWidth = (maxLength) / numBins;
        for (int i = 0 ; i < numBins ; i++)
        {
            //double x = (i+0.5)*binWidth;
            double x1 = (i)*binWidth;
            double x2 = (i+1)*binWidth;
            double y = (count[i]) / (totalCount*binWidth); 
            ofile << x1 << " " << y << endl;
            ofile << x2 << " " << y << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtAggregateRayLengthDistributionQuery::ExecuteLineScan
//
//  Purpose:
//      Examines the input data.  Note that the line scan filter will organize
//      the data so that all of the contributions from a given line will be in
//      the same vtkPolyData input.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

void
avtAggregateRayLengthDistributionQuery::ExecuteLineScan(vtkPolyData *pd)
{
    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;

    //Get array of cast lines
    vtkIntArray *lineids = (vtkIntArray *) 
                                  pd->GetCellData()->GetArray("avtLineID");
    if (lineids == NULL)
        EXCEPTION0(ImproperUseException);
        
    int npts = pd->GetNumberOfPoints();
    vector<bool> usedPoint(npts, false);
    
    // This may be NULL, but, if so, we will just treat it as uniform density.
    vtkDataArray *arr = pd->GetCellData()->GetArray(varname.c_str());

    pd->BuildLinks();
    pd->BuildCells();

    int amtPerMsg = npts / extraMsg + 1;
    UpdateProgress(extraMsg*currentNode+2*extraMsg/3, totalProg);
    int lastMilestone = 0;

    vector<double> pts;
    vector<double> mass;
    vector<int>    seg_lineid;

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
    vector< vector<int> >    hashed_lineid_lookup(1000);

    for (int i = 0 ; i < npts ; i++)
    {
        // glue segments into one long line
        if (usedPoint[i])
            continue;
        int seg1 = 0, seg2 = 0;
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
        pts.push_back(pt1[0]);
        pts.push_back(pt1[1]);
        pts.push_back(pt1[2]);
        pts.push_back(pt2[0]);
        pts.push_back(pt2[1]);
        pts.push_back(pt2[2]);

        double m = GetMass(oneSide, otherSide, pd, lineids, lineid, arr);
        mass.push_back(m);
        int seg_id = seg_lineid.size();
        seg_lineid.push_back(lineid);
        hashed_lineid_lookup[lineid % 1000].push_back(seg_id);
    }

    usedPoint.resize(npts, false);
    int segId = 0;
    for (int i = 0 ; i < npts ; i++)
    {
        // glue segments into one long line
        if (usedPoint[i])
            continue;
        int seg1 = 0, seg2 = 0;
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

        // Look over all other line segments and see which side they are on.
        // Then split up the mass accordingly.
        double massFromOneToOther = 0.;
        double massFromOtherToOne = 0.;

        double pt1[3];
        double pt2[3];
        pd->GetPoint(oneSide, pt1);
        pd->GetPoint(otherSide, pt2);

        double dir[3];
        dir[0] = pt2[0] - pt1[0];
        dir[1] = pt2[1] - pt1[1];
        dir[2] = pt2[2] - pt1[2];

        // Check for degenerate
        if (dir[0] == 0. && dir[1] == 0. && dir[2] == 0.)
            continue;
        int dim = 0;
        while (dir[dim] == 0.)
            dim++;

        int num_segs = pts.size() / 6;
        int entry = lineid % 1000;
        for (int j = 0 ; j < hashed_lineid_lookup[entry].size() ; j++)
        {
            int candidate = hashed_lineid_lookup[entry][j];
            if (seg_lineid[candidate] != lineid)
                continue;
            if (candidate == segId)
                continue;

            // Determine which side the segment is on
            double p1 = pts[6*candidate + dim];
            double p2 = pts[6*candidate + dim + 3];
            double t1 = (pt1[dim] - p1) / dir[dim];
            double t2 = (pt1[dim] - p2) / dir[dim];
            double t3 = (pt2[dim] - p1) / dir[dim];
            double t4 = (pt2[dim] - p2) / dir[dim];

            // The segment crosses our current segment.
            if (t1*t2 <= 0. || t1*t3 <= 0. || t2*t4 <= 0.)
                continue;

            if (t1 > 0) // the segment is on the opposite side of oneSide
                        // as otherSide
                massFromOneToOther += mass[candidate];
            else        // the segment is on the same side of oneSide
                        // as otherSide
                massFromOtherToOne += mass[candidate];
        }

        // Drop values from the line into the appropriate mass bins
        WalkLine(oneSide, otherSide, pd, lineids, lineid, arr,
                 massFromOneToOther);
        WalkLine(otherSide, oneSide, pd, lineids, lineid, arr,
                 massFromOtherToOne);

        segId++;

        int currentMilestone = (int)(((float) i) / amtPerMsg);
        if (currentMilestone > lastMilestone)
        {
            UpdateProgress((int)(
                        extraMsg*currentNode+2*extraMsg/3.+currentMilestone/3),
                           extraMsg*totalNodes);
            lastMilestone = currentMilestone;
        }
    }
}



// ****************************************************************************
//  Method: avtAggregateRayLengthDistributionQuery::WalkLine
//
//  Purpose:
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

void
avtAggregateRayLengthDistributionQuery::WalkLine(int startPtId, 
                 int endPtId, vtkPolyData *output, vtkIntArray *lineids, 
                 int lineid, vtkDataArray *arr, double additionalMass)
{
    int curPtId = startPtId;
    int curCellId, dummyCellId;
    double pt1[3];
    double pt2[3];

    GetCellsForPoint(curPtId, output, lineids, -1, curCellId, dummyCellId);
    output->GetPoint(startPtId, pt1);
    output->GetPoint(endPtId, pt2);

    // So we will initialize "massEncounteredSoFar" with "additionalMass".
    // This is because we know that there is "additionalMass" located 
    // before "startPtId".
    double massEncounteredSoFar = additionalMass;
    double binSize = (maxLength) / (double)numBins;

    // Walk segments in the line
    while (curPtId != endPtId)
    {
        double curSegDen = (arr != NULL ? arr->GetTuple1(curCellId) : 1.);
        int newPtId, newCellId;
        WalkChain1(output, curPtId, curCellId, lineids, lineid, 
                   newPtId, newCellId);
        output->GetPoint(curPtId, pt1);
        output->GetPoint(newPtId, pt2);
        double dist = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                           (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                           (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));

        // Walk bins that overlap this segment, first with respect to the 
        // start point, then with respect to the end point.
        double massForThisSegment = dist*curSegDen;
        double massAfterThisSegment = massEncounteredSoFar+massForThisSegment;
    
        int startBin = (int)floor(massEncounteredSoFar / binSize);
        int endBin   = (int)floor(massAfterThisSegment / binSize);

        for (int currBin = startBin ; currBin <= endBin ; currBin++) 
        {
            int bin = (currBin < 0 ? 0 : (currBin >= numBins ? numBins-1
                            : currBin));
            if (currBin == startBin && startBin == endBin)
            {
                count[bin] += curSegDen*dist;
            }
            else if (currBin == startBin)
            {
                double massForNextBin = (currBin+1)*binSize;
                double massContrib = massForNextBin - massEncounteredSoFar;
                count[bin] += massContrib;
            }
            else if (currBin == endBin) 
            {
                double massThroughStartOfBin = (currBin)*binSize;
                double massLeft = massAfterThisSegment - massThroughStartOfBin;
                count[bin] += massLeft;
            }
            else
            {
                count[bin] += curSegDen*binSize;
            }
        }
        massEncounteredSoFar += massForThisSegment;

        curPtId   = newPtId;
        curCellId = newCellId;

        if (curCellId == -1 && curPtId != endPtId)
        {
            debug1 << "INTERNAL ERROR: path could not be reproduced." 
                   << endl;
            return;
        }
    }
}


// ****************************************************************************
//  Method: avtAggregateRayLengthDistributionQuery::GetMass
//
//  Purpose:
//      Gets the mass along a line segment.
//
//  Programmer: Hank Childs
//  Creation:   August 28, 2006
//
// ****************************************************************************

double
avtAggregateRayLengthDistributionQuery::GetMass(int startPtId, 
                 int endPtId, vtkPolyData *output, vtkIntArray *lineids, 
                 int lineid, vtkDataArray *arr)
{
    int curPtId = startPtId;
    int curCellId, dummyCellId;
    double pt1[3];
    double pt2[3];

    GetCellsForPoint(curPtId, output, lineids, -1, curCellId, dummyCellId);
    output->GetPoint(startPtId, pt1);
    output->GetPoint(endPtId, pt2);

    // Walk segments in the line
    double totalMass = 0.;
    while (curPtId != endPtId)
    {
        double curSegDen = (arr != NULL ? arr->GetTuple1(curCellId) : 1.);
        int newPtId, newCellId;
        WalkChain1(output, curPtId, curCellId, lineids, lineid, 
                   newPtId, newCellId);
        output->GetPoint(curPtId, pt1);
        output->GetPoint(newPtId, pt2);
        double dist = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                           (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                           (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));
        double massForThisSegment = dist*curSegDen;
        totalMass += massForThisSegment;

        curPtId   = newPtId;
        curCellId = newCellId;

        if (curCellId == -1 && curPtId != endPtId)
        {
            debug1 << "INTERNAL ERROR: path could not be reproduced." 
                   << endl;
            return 0.;
        }
    }

    return totalMass;
}


