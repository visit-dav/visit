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
//                          avtMassDistributionQuery.C                       //
// ************************************************************************* //

#include <avtMassDistributionQuery.h>

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
#include <avtTotalRevolvedVolumeQuery.h>
#include <avtTotalSurfaceAreaQuery.h>
#include <avtTotalVolumeQuery.h>
#include <avtWeightedVariableSummationQuery.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtMassDistributionQuery constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

avtMassDistributionQuery::avtMassDistributionQuery()
{
    mass      = new double[numBins];
}


// ****************************************************************************
//  Method: avtMassDistributionQuery destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

avtMassDistributionQuery::~avtMassDistributionQuery()
{
    delete [] mass;
}


// ****************************************************************************
//  Method: avtMassDistributionQuery::PreExecute
//
//  Purpose:
//      Does some initialization work before the query executes.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

void
avtMassDistributionQuery::PreExecute(void)
{
    avtLineScanQuery::PreExecute();

    delete [] mass;
    mass = new double[numBins];
    for (int i = 0 ; i < numBins ; i++)
        mass[i] = 0.;
}


// ****************************************************************************
//  Method: avtMassDistributionQuery::PostExecute
//
//  Purpose:
//      Outputs the mass distribution.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
//  Modifications:
//
//    Hank Childs, Fri Sep 15 17:03:39 PDT 2006
//    Add support for RZ meshes.
//
// ****************************************************************************

void
avtMassDistributionQuery::PostExecute(void)
{
    int   i;

    avtWeightedVariableSummationQuery summer;
    avtDataObject_p dob = GetInput();
    summer.SetInput(dob);
    QueryAttributes qa;
    summer.PerformQuery(&qa);
    double totalMass = qa.GetResultsValue()[0];

    bool didVolume  = false;
    bool didRVolume = false;
    bool didSA      = false;
    if (totalMass == 0.)
    {
        if (dob->GetInfo().GetAttributes().GetTopologicalDimension() == 3)
        {
            avtTotalVolumeQuery tvq;
            avtDataObject_p dob = GetInput();
            tvq.SetInput(dob);
            QueryAttributes qa;
            tvq.PerformQuery(&qa);
            totalMass = qa.GetResultsValue()[0];
            didVolume = true;
        }
        else if (dob->GetInfo().GetAttributes().GetMeshCoordType() == AVT_RZ
              || dob->GetInfo().GetAttributes().GetMeshCoordType() == AVT_ZR)
        {
            avtTotalRevolvedVolumeQuery rvq;
            avtDataObject_p dob = GetInput();
            rvq.SetInput(dob);
            QueryAttributes qa;
            rvq.PerformQuery(&qa);
            totalMass = qa.GetResultsValue()[0];
            didRVolume = true;
        }
        else
        {
            avtTotalSurfaceAreaQuery saq;
            avtDataObject_p dob = GetInput();
            saq.SetInput(dob);
            QueryAttributes qa;
            saq.PerformQuery(&qa);
            totalMass = qa.GetResultsValue()[0];
            didSA = true;
        }
    }

    int times = 0;
    char name[1024];
    sprintf(name, "md%d.ult", times++);

    if (PAR_Rank() == 0)
    {
        bool lookingForUnused = true;
        while (lookingForUnused)
        {
            ifstream ifile(name);
            if (ifile.fail())
                lookingForUnused = false;
            else
                sprintf(name, "md%d.ult", times++);
        }
    }

    char msg[1024];
    const char *mass_string = (didVolume ? "volume" : 
                              (didRVolume ? "revolved volume" : 
                              (didSA ? "area" : "mass")));
    sprintf(msg, "The %s distribution has been outputted as an "
                 "Ultra file (%s), which can then be imported into VisIt.  The"
                 " total %s considered was %f\n", 
                 mass_string, name, mass_string, totalMass);
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
            double x1 = minLength + (i)*binWidth;
            double x2 = minLength + (i+1)*binWidth;
            double y = (totalMass*mass[i]) / (totalMassFromLines*binWidth); 
            ofile << x1 << " " << y << endl;
            ofile << x2 << " " << y << endl;
        }
    }
}


// ****************************************************************************
//  Method: avtMassDistributionQuery::ExecuteLineScan
//
//  Purpose:
//      Examines the input data.  Note that the line scan filter will organize
//      the data so that all of the contributions from a given line will be in
//      the same vtkPolyData input.
//
//  Programmer: Hank Childs
//  Creation:   July 20, 2006
//
// ****************************************************************************

void
avtMassDistributionQuery::ExecuteLineScan(vtkPolyData *pd)
{
    vtkIntArray *lineids = (vtkIntArray *) 
                                  pd->GetCellData()->GetArray("avtLineID");
    if (lineids == NULL)
        EXCEPTION0(ImproperUseException);
        
    int npts = pd->GetNumberOfPoints();
    vector<bool> usedPoint(npts, false);
    
    vtkDataArray *arr = pd->GetCellData()->GetArray(varname.c_str());

    pd->BuildLinks();
    pd->BuildCells();

    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;
    int amtPerMsg = npts / extraMsg + 1;
    UpdateProgress(extraMsg*currentNode+2*extraMsg/3, totalProg);
    int lastMilestone = 0;

    for (int i = 0 ; i < npts ; i++)
    {
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
        double dist = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                           (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                           (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));
        int bin = (int)((dist-minLength) / (maxLength-minLength) * numBins);
        if (bin < 0)
            bin = 0;
        if (bin >= numBins)
            bin = numBins-1;
        int curId = oneSide;
        numMatches = GetCellsForPoint(curId, pd, lineids, -1, seg1, seg2);
        int curCell = seg1;
        double curSegMass = 0.;
        while (curId != otherSide)
        {
           double curSegDen = (arr != NULL ? arr->GetTuple1(curCell) : 1.);
           int newPtId, newCellId;
           WalkChain1(pd, curId, curCell, lineids, lineid, 
                      newPtId, newCellId);
           pd->GetPoint(curId, pt1);
           pd->GetPoint(newPtId, pt2);
           double dist = sqrt((pt2[0]-pt1[0])*(pt2[0]-pt1[0]) + 
                              (pt2[1]-pt1[1])*(pt2[1]-pt1[1]) + 
                              (pt2[2]-pt1[2])*(pt2[2]-pt1[2]));
           curSegMass += curSegDen*dist;
           curId = newPtId;
           curCell = newCellId;
           if (curCell == -1 && curId != otherSide)
           {
               debug1 << "INTERNAL ERROR: path could not be reproduced." 
                      << endl;
               break;
           }
        }
        mass[bin] += curSegMass;

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


