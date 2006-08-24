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
//                          avtDistanceFromBoundaryQuery.C                       //
// ************************************************************************* //

#include <avtDistanceFromBoundaryQuery.h>

#include <stdio.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkExecutive.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>

#include <avtLineScanFilter.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>
#include <avtWeightedVariableSummationQuery.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtDistanceFromBoundaryQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

avtDistanceFromBoundaryQuery::avtDistanceFromBoundaryQuery()
{
    mass = new double[numBins];
}


// ****************************************************************************
//  Method: avtDistanceFromBoundaryQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

avtDistanceFromBoundaryQuery::~avtDistanceFromBoundaryQuery()
{
    delete [] mass;
}


// ****************************************************************************
//  Method: avtDistanceFromBoundaryQuery::PreExecute
//
//  Purpose:
//      Does some initialization work before the query executes.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

void
avtDistanceFromBoundaryQuery::PreExecute(void)
{
    avtLineScanQuery::PreExecute();

    delete [] mass;
    mass = new double[numBins];
    for (int i = 0 ; i < numBins ; i++)
        mass[i] = 0.;
}


// ****************************************************************************
//  Method: avtDistanceFromBoundaryQuery::PostExecute
//
//  Purpose:
//      Outputs the mass distribution.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

void
avtDistanceFromBoundaryQuery::PostExecute(void)
{
    int   i;

    avtWeightedVariableSummationQuery summer;
    avtDataObject_p dob = GetInput();
    summer.SetInput(dob);
    QueryAttributes qa;
    summer.PerformQuery(&qa);
    double totalMass = qa.GetResultsValue()[0];

    int times = 0;
    char name[1024];
    sprintf(name, "dfb%d.ult", times++);

    if (PAR_Rank() == 0)
    {
        bool lookingForUnused = true;
        while (lookingForUnused)
        {
            ifstream ifile(name);
            if (ifile.fail())
                lookingForUnused = false;
            else
                sprintf(name, "dfb%d.ult", times++);
        }
    }

    char msg[1024];
    sprintf(msg, "The distribution of mass over distance from the boundary "
                 "has been outputted as an "
                 "Ultra file (%s), which can then be imported into VisIt.  The"
                 " total mass considered was %f\n", 
                 name, totalMass);
    SetResultMessage(msg);
    SetResultValue(0.);

    double *m2 = new double[numBins];
    SumDoubleArrayAcrossAllProcessors(mass, m2, numBins);
    delete [] mass;
    mass = m2;

    double totalMassFromLines = 0.;
    for (i = 0 ; i < numBins ; i++)
        totalMassFromLines += mass[i];

    if (PAR_Rank() == 0)
    {
        if (totalMassFromLines == 0.)
        {
            sprintf(msg, "The mass distribution could not be calculated "
                    "becuase none of the lines intersected the data set."
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
        ofile << "# Mass distribution" << endl;
        double binWidth = (maxLength-minLength) / numBins;
        for (int i = 0 ; i < numBins ; i++)
        {
            double x = minLength + (i+0.5)*binWidth;
            double y = (totalMass*mass[i]) / (totalMassFromLines*binWidth); 
            ofile << x << " " << y << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtDistanceFromBoundaryQuery::ExecuteLineScan
//
//  Purpose:
//      Examines the input data.  Note that the line scan filter will organize
//      the data so that all of the contributions from a given line will be in
//      the same vtkPolyData input.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

void
avtDistanceFromBoundaryQuery::ExecuteLineScan(vtkPolyData *pd)
{
    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;
    UpdateProgress(extraMsg*currentNode, totalProg);

    //
    // Check to make sure we have a mass variable before running clean
    // poly data, since that takes a *long* time.
    //
    if (pd->GetCellData()->GetArray(varname.c_str()) == NULL)
    {
        EXCEPTION1(VisItException, "This query is only set up for zonal"
                                   " variables.  You should be applying this "
                                   "query to a pseudocolor plot of density.");
    }

    vtkCleanPolyData *cpd = vtkCleanPolyData::New();  //Glue together intersecting segments
    cpd->SetToleranceIsAbsolute(0);
    cpd->SetTolerance(1e-7);
    cpd->SetInput(pd);
    vtkPolyData *output = cpd->GetOutput();
    output->Update();

    UpdateProgress(extraMsg*currentNode+extraMsg/3, totalProg);

    //Get array of cast lines
    vtkIntArray *lineids = (vtkIntArray *) 
                                  output->GetCellData()->GetArray("avtLineID");
    if (lineids == NULL)
        EXCEPTION0(ImproperUseException);
        
    int npts = output->GetNumberOfPoints();
    vector<bool> usedPoint(npts, false);
    
    vtkDataArray *arr = output->GetCellData()->GetArray(varname.c_str());
    if (arr == NULL)
    {
        EXCEPTION1(VisItException, "This query is only set up for zonal"
                                   " variables.  You should be applying this "
                                   "query to a pseudocolor plot of density.");
    }

    //output of cleanpolydata
    output->BuildLinks();
    output->BuildCells();

    int amtPerMsg = npts / extraMsg + 1;
    UpdateProgress(extraMsg*currentNode+2*extraMsg/3, totalProg);
    int lastMilestone = 0;

    for (int i = 0 ; i < npts ; i++)
    {
        // glue segments into one long line
        if (usedPoint[i])
            continue;
        int seg1 = 0, seg2 = 0;
        int numMatches = GetCellsForPoint(i, output, lineids, -1, seg1, seg2);
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
            otherSide = WalkChain(output, i, seg1, usedPoint, lineids, lineid);
        }
        else if (numMatches == 2)
        {
            oneSide   = WalkChain(output, i, seg1, usedPoint, lineids, lineid);
            otherSide = WalkChain(output, i, seg2, usedPoint, lineids, lineid);
        }
        if (oneSide == -1 || otherSide == -1)
        {
            // We found an error condition.  Give up on this line.  This
            // happens infrequently enough that it should not affect our
            // statistics.
            continue;
        }

        // Drop values from the line into the appropriate mass bins
        WalkLine(oneSide, otherSide, output, lineids, lineid, arr);

        int currentMilestone = (int)(((float) i) / amtPerMsg);
        if (currentMilestone > lastMilestone)
        {
            UpdateProgress((int)(
                        extraMsg*currentNode+2*extraMsg/3.+currentMilestone/3),
                           extraMsg*totalNodes);
            lastMilestone = currentMilestone;
        }
    }
    vtkCellArray *lines = output->GetLines();

    cpd->Delete();
}



// ****************************************************************************
//  Method: avtDistanceFromBoundaryQuery::WalkLine
//
//  Purpose:
//      Process all the segments in a line.  For each segment, find the 
//      distance bins that overlap the segment, and increment the values
//      in each bin by the amount of segment in the bin * the density 
//      for that segment.  Iterate over bins twice per segment, to cover
//      the distance both from both endpoints of the line.
//
//  Programmer: David Bremer
//  Creation:   August 23, 2006
//
// ****************************************************************************

void
avtDistanceFromBoundaryQuery::WalkLine(int startPtId, int endPtId, vtkPolyData *output, 
                                       vtkIntArray *lineids, int lineid, vtkDataArray *arr)
{
    int curPtId = startPtId;
    int curCellId, dummyCellId;
    double pt1[3];
    double pt2[3];

    GetCellsForPoint(curPtId, output, lineids, -1, curCellId, dummyCellId);
    output->GetPoint(startPtId, pt1);
    output->GetPoint(endPtId, pt2);
    double distToEndPt = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                              (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                              (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));

    double distToStartPt = 0.0;
    double binSize = (maxLength-minLength) / (double)numBins;

    // Walk segments in the line
    while (curPtId != endPtId)
    {
        double curSegDen = arr->GetTuple1(curCellId);
        int newPtId, newCellId;
        WalkChain1(output, curPtId, curCellId, lineids, lineid, 
                   newPtId, newCellId);
        output->GetPoint(curPtId, pt1);
        output->GetPoint(newPtId, pt2);
        double dist = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                           (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                           (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));
        distToEndPt -= dist;

        // Walk bins that overlap this segment, first with respect to the start point,
        // then with respect to the end point.
        int ii;
        for (ii=0; ii<2; ii++)
        {
            double distToEdge;
            if (ii==0)
                distToEdge = distToStartPt;
            else
                distToEdge = distToEndPt;

            int currBin;
            int startBin = (int)floor((distToEdge - minLength) / binSize);
            int endBin   = (int)floor((distToEdge + dist - minLength) / binSize);

            for (currBin = startBin ; currBin <= endBin ; currBin++) 
            {
                double distToContrib = binSize;
                if (currBin == startBin) 
                {
                    double lenUnusedPartOfBin 
                                    = distToEdge - minLength - binSize*currBin;
                    distToContrib -= lenUnusedPartOfBin;
                }
                if (currBin == endBin) 
                {
                    double lenUnusedPartOfBin = binSize*(currBin+1) 
                                              - (distToEdge + dist - minLength);
                    distToContrib -= lenUnusedPartOfBin;
                }
                // Should I clamp the bin number, or exclude lengths 
                //outside the range of interest?
                int clampedBin = (currBin < 0) ? 0: 
                                     (currBin >= numBins) ? numBins-1: currBin;
                mass[clampedBin] += distToContrib * curSegDen;
            }
        }

        curPtId   = newPtId;
        curCellId = newCellId;
        distToStartPt += dist;

        if (curCellId == -1 && curPtId != endPtId)
        {
            debug1 << "INTERNAL ERROR: path could not be reproduced." 
                   << endl;
            return;
        }
    }
}



