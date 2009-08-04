/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//                          avtLineScanTransformQuery.C                       //
// ************************************************************************* //

#include <avtLineScanTransformQuery.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkExecutive.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>

#include <avtLineScanFilter.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtOriginatingSource.h>
#include <avtWeightedVariableSummationQuery.h>
#include <avtVector.h>

#include <DebugStream.h>

static int qsort_compare_seglen(const void *a, const void *b);



// ****************************************************************************
//  Method: avtLineScanTransformQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

avtLineScanTransformQuery::avtLineScanTransformQuery()
{
    lengths = new double[numBins];
    numLineIntersections = 0;
}


// ****************************************************************************
//  Method: avtLineScanTransformQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

avtLineScanTransformQuery::~avtLineScanTransformQuery()
{
    delete [] lengths;
}


// ****************************************************************************
//  Method: avtLineScanTransformQuery::PreExecute
//
//  Purpose:
//      Does some initialization work before the query executes.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

void
avtLineScanTransformQuery::PreExecute(void)
{
    avtLineScanQuery::PreExecute();

    delete [] lengths;
    lengths = new double[numBins];
    for (int i = 0 ; i < numBins ; i++)
        lengths[i] = 0.;
    numLineIntersections = 0;
}


// ****************************************************************************
//  Method: avtLineScanTransformQuery::PostExecute
//
//  Purpose:
//      Outputs the length distribution.
//
//  Programmer: David Bremer
//  Creation:   August 8, 2006
//
// ****************************************************************************

void
avtLineScanTransformQuery::PostExecute(void)
{
    int times = 0;
    char name[1024];
    sprintf(name, "lst%d.ult", times++);

    if (PAR_Rank() == 0)
    {
        bool lookingForUnused = true;
        while (lookingForUnused)
        {
            ifstream ifile(name);
            if (ifile.fail())
                lookingForUnused = false;
            else
                sprintf(name, "lst%d.ult", times++);
        }
    }

    double *accumLengths = new double[numBins];
    SumDoubleArrayAcrossAllProcessors(lengths, accumLengths, numBins);
    delete [] lengths;
    lengths = accumLengths;

    int  totalNumLineIntersections;
    SumIntArrayAcrossAllProcessors( &numLineIntersections, &totalNumLineIntersections, 1);
    
    char msg[1024];
    sprintf(msg, "The line scan transform has been outputted as an "
                 "Ultra file (%s), which can then be imported into VisIt."
                 "There were %d total line intersections.", name, totalNumLineIntersections);
    SetResultMessage(msg);
    SetResultValue(0.);

    if (PAR_Rank() == 0)
    {
        ofstream ofile(name);
        if (ofile.fail())
        {
            sprintf(msg, "Unable to write out file containing distribution.");
            SetResultMessage(msg);
            return;
        }
        ofile << "# Line Scan Transform" << endl;
        double binWidth = (maxLength-minLength) / (numBins-1);
        for (int i = 0 ; i < numBins ; i++)
        {
            double x = minLength + i*binWidth;
            double y = lengths[i] / (double)totalNumLineIntersections; 
            ofile << x << " " << y << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtLineScanTransformQuery::ExecuteLineScan
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
avtLineScanTransformQuery::ExecuteLineScan(vtkPolyData *pd)
{
    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;

    double pt1[3];   //Endpoints of current segment
    double pt2[3];

    bool bHaveLineEq = false;

    double binSize = (maxLength - minLength) / (numBins - 1);

    UpdateProgress(extraMsg*currentNode, totalProg);

    //
    // Check to make sure we have a mass variable before running clean
    // poly data, since that takes a *long* time.
    //
/*
    if (pd->GetCellData()->GetArray(varname.c_str()) == NULL)
    {
        EXCEPTION1(VisItException, "This query is only set up for zonal"
                                   " variables.  You should be applying this "
                                   "query to a pseudocolor plot of density.");
    }
*/

    // Glue together intersecting segments
    vtkCleanPolyData *cpd = vtkCleanPolyData::New();  
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
    
    /*
    vtkDataArray *arr = output->GetCellData()->GetArray(varname.c_str());
    if (arr == NULL)
    {
        EXCEPTION1(VisItException, "This query is only set up for zonal"
                                   " variables.  You should be applying this "
                                   "query to a pseudocolor plot of density.");
    }*/

    //output of cleanpolydata
    output->BuildLinks();
    output->BuildCells();

    int amtPerMsg = npts / extraMsg + 1;
    UpdateProgress(extraMsg*currentNode+2*extraMsg/3, totalProg);
    int lastMilestone = 0;
    int i, j, k;

    int hashSize = 10000;
    vector< vector<int> >    hashedLineidLookup(hashSize);
    vector< vector<double> > hashedSegmentEndPoints(hashSize);


    // After this loop completes, a set of line endpoints and ids will fill 
    // the hash tables.
    for (i = 0 ; i < npts ; i++)
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

        output->GetPoint(oneSide,   pt1);
        output->GetPoint(otherSide, pt2);

        int hashid = lineid % hashSize;
        hashedLineidLookup[hashid].push_back(lineid);
        hashedSegmentEndPoints[hashid].push_back(pt1[0]);
        hashedSegmentEndPoints[hashid].push_back(pt1[1]);
        hashedSegmentEndPoints[hashid].push_back(pt1[2]);
        hashedSegmentEndPoints[hashid].push_back(pt2[0]);
        hashedSegmentEndPoints[hashid].push_back(pt2[1]);
        hashedSegmentEndPoints[hashid].push_back(pt2[2]);

        // Drop values from the line into the appropriate mass bins
        //WalkLine(oneSide, otherSide, output, lineids, lineid, arr);

        int currentMilestone = (int)(((float) i) / amtPerMsg);
        if (currentMilestone > lastMilestone)
        {
            UpdateProgress((int)(
                        extraMsg*currentNode+2*extraMsg/3.+currentMilestone/3),
                           extraMsg*totalNodes);
            lastMilestone = currentMilestone;
        }
    }
    //vtkCellArray *lines = output->GetLines();

    vector< double > projectedSegments;

    for (i = 0 ; i < hashSize ; i++)
    {
        while (1)
        {
            // Look for a valid line id for this hash element.
            // If none are left, move to the next hash bin.
            int currLineID = -1;
            for (j = 0 ; j < hashedLineidLookup[i].size() ; j++)
            {
                if (hashedLineidLookup[i][j] != -1)
                {
                    currLineID = hashedLineidLookup[i][j];
                    break;
                }
            }
            if (currLineID == -1)
                break;
    
            projectedSegments.resize(0);
            avtVector pt( lines[currLineID*6],   lines[currLineID*6+2], lines[currLineID*6+4]);
            avtVector dir(lines[currLineID*6+1], lines[currLineID*6+3], lines[currLineID*6+5]);
            dir -= pt;
            dir.normalize();
    
            //Pull out all the segments with id == currLineID, 
            //and store their projected lengths.
            for (j = 0 ; j < hashedLineidLookup[i].size() ; j++)
            {
                if (hashedLineidLookup[i][j] == currLineID)
                {
                    hashedLineidLookup[i][j] = -1;
                    avtVector p1(hashedSegmentEndPoints[i][j*6],
                                 hashedSegmentEndPoints[i][j*6+1],
                                 hashedSegmentEndPoints[i][j*6+2]);
                    avtVector p2(hashedSegmentEndPoints[i][j*6+3],
                                 hashedSegmentEndPoints[i][j*6+4],
                                 hashedSegmentEndPoints[i][j*6+5]);
                    double d1 = (p1 - pt) * dir;
                    double d2 = (p2 - pt) * dir;
                    if (d1 < d2) 
                    {
                        projectedSegments.push_back(d1);
                        projectedSegments.push_back(d2);
                    }
                    else
                    {
                        projectedSegments.push_back(d2);
                        projectedSegments.push_back(d1);
                    }
                }
            }
            //Sort the list of segments.  TODO:  See if std has a preferred sort method.
            qsort(&(projectedSegments[0]), projectedSegments.size()/2, sizeof(double)*2, qsort_compare_seglen);
        
            double min = projectedSegments[0];
            //printf("----line %f,%f  to  %f,%f  min=%f----\n", lines[currLineID*6], lines[currLineID*6+2], lines[currLineID*6+1], lines[currLineID*6+3], min);
            for (j = 0; j < projectedSegments.size(); j++ )
            {
                projectedSegments[j] -= min;
            }
            //for (j = 0; j < projectedSegments.size(); j+=2 )
            //{
            //    printf("%f %f\n", projectedSegments[j], projectedSegments[j+1]);
            //}

            for ( k = 2 ; k <= projectedSegments.size() ; k++ )
            {
                for ( i = 1 ; i <= k - 1 ; i++ )
                {
                    int incr = 1;
                    if (((i+k+1) & 0x1) == 1)
                        incr = -1;

                    double segLen = projectedSegments[k-1] - projectedSegments[i-1];

                    //Increment or decrement all bins that fall in the range 0..segLen
                    int startBin = 0, endBin = numBins - 1;
                    if (minLength < 0.0)
                    {
                        startBin = (int)ceil(-minLength / binSize);
                    }
                    if (segLen < maxLength) 
                    {
                        endBin = (int)floor( (segLen - minLength) / binSize );
                    }
                    for ( j = startBin ; j <= endBin ; j++ )
                    {
                        lengths[j] += incr;
                    }
                }
            }
            numLineIntersections++;

            //for ( i = 0 ; i < numBins ; i++ )
            //{
            //    printf("%f ", lengths[i]);
            //}
            //printf("\n");
        }
    }

    cpd->Delete();
}


static int qsort_compare_seglen(const void *a, const void *b)
{
    double p1 = *((double *)a);
    double p2 = *((double *)b);
    if (p1 < p2)
        return -1;
    else if (p1 > p2) 
        return 1;
    else
        return 0;
}





