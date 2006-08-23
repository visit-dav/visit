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
//                      avtChordLengthDistributionQuery.C                    //
// ************************************************************************* //

#include <avtChordLengthDistributionQuery.h>

#include <stdio.h>
#include <math.h>

#include <vtkCellData.h>
#include <vtkCleanPolyData.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>

#include <avtLineScanFilter.h>
#include <avtParallel.h>
#include <avtSourceFromAVTDataset.h>
#include <avtTerminatingSource.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtChordLengthDistributionQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2006
//
// ****************************************************************************

avtChordLengthDistributionQuery::avtChordLengthDistributionQuery()
{
    numChords = new int[numBins];
}


// ****************************************************************************
//  Method: avtChordLengthDistributionQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2006
//
// ****************************************************************************

avtChordLengthDistributionQuery::~avtChordLengthDistributionQuery()
{
    delete [] numChords;;
}


// ****************************************************************************
//  Method: avtChordLengthDistributionQuery::PreExecute
//
//  Purpose:
//      Does some initialization work before the query executes.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2006
//
// ****************************************************************************

void
avtChordLengthDistributionQuery::PreExecute(void)
{
    avtLineScanQuery::PreExecute();

    delete [] numChords;
    numChords = new int[numBins];
    for (int i = 0 ; i < numBins ; i++)
        numChords[i] = 0;
}


// ****************************************************************************
//  Method: avtChordLengthDistributionQuery::PostExecute
//
//  Purpose:
//      Outputs the chord counts.
//
//  Programmer: Hank Childs
//  Creation:   July 8, 2006
//
// ****************************************************************************

void
avtChordLengthDistributionQuery::PostExecute(void)
{
    int   i;

    int times = 0;
    char name[1024];
    sprintf(name, "cld%d.ult", times++);

    if (PAR_Rank() == 0)
    {
        bool lookingForUnused = true;
        while (lookingForUnused)
        {
            ifstream ifile(name);
            if (ifile.fail())
                lookingForUnused = false;
            else
                sprintf(name, "cld%d.ult", times++);
        }
    }

    char msg[1024];
    sprintf(msg, "The chord length distribution has been outputted as an "
                 "Ultra file (%s), which can then be imported into VisIt.", 
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
        ofile << "# Chord length distribution" << endl;

        for (int i = 0 ; i < numBins ; i++)
        {
            double x = minLength + (i+0.5)*binWidth;
            double y = numChords[i] / totalArea; // Make it be a distribution ...
                                                 // the area under the curve: 1
            ofile << x << " " << y << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtChordLengthDistributionQuery::ExecuteLineScan
//
//  Purpose:
//      Examines the input data.  Note that the line scan filter will organize
//      the data so that all of the contributions from a given line will be in
//      the same vtkPolyData input.
//
//  Programmer: Hank Childs
//  Creation:   July 8, 2006
//
// ****************************************************************************

void
avtChordLengthDistributionQuery::ExecuteLineScan(vtkPolyData *pd)
{
    vtkCleanPolyData *cpd = vtkCleanPolyData::New();
    cpd->SetInput(pd);
    vtkPolyData *output = cpd->GetOutput();
    output->Update();

    vtkIntArray *lineids = (vtkIntArray *) 
                                  output->GetCellData()->GetArray("avtLineID");
    if (lineids == NULL)
        EXCEPTION0(ImproperUseException);
        
    int npts = output->GetNumberOfPoints();
    vector<bool> usedPoint(npts, false);
    
    output->BuildLinks();
    output->BuildCells();
    for (int i = 0 ; i < npts ; i++)
    {
        if (usedPoint[i])
            continue;
        int seg1, seg2;
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
        double pt1[3];
        double pt2[3];
        output->GetPoint(oneSide, pt1);
        output->GetPoint(otherSide, pt2);
        double dist = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                           (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                           (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));
        int bin = (int)((dist-minLength) / (maxLength-minLength) * numBins);
        if (bin < 0)
            bin = 0;
        if (bin >= numBins)
            bin = numBins-1;
        numChords[bin]++;
    }
    vtkCellArray *lines = output->GetLines();

    cpd->Delete();
}


