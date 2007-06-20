/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include <avtHohlraumFluxQuery.h>
#include <vtkCleanPolyData.h>
#include <vtkCellData.h>
#include <avtTerminatingSource.h>
#include <avtLineScanFilter.h>
#include <avtSourceFromAVTDataset.h>
#include <avtParallel.h>
#include <vectortypes.h>

#include <visitstream.h>

// ****************************************************************************
//  Method: avtHohlraumFluxQuery::avtHohlraumFluxQuery
//
//  Purpose:
//    Basic constructor
//
//  Programmer: David Bremer
//  Creation:   November 17, 2006
//
//  Modifications:
//    Dave Bremer, Tue Jun 19 18:33:26 PDT 2007
//    Lowered the number of lines per iteration.
//  
// ****************************************************************************

avtHohlraumFluxQuery::avtHohlraumFluxQuery():
    absVarName("absorbtivity"),
    emisVarName("emissivity")
{
    rayCenter[0] = 0.0f;
    rayCenter[1] = 0.0f;
    rayCenter[2] = 0.0f;
    theta  = 0.0f;
    phi    = 0.0f;
    radius = 1.0f;
    radBins = NULL;

    //lower the default number of lines, to work around a problem 
    //merging lines in vtkCleanPolyData.
    numLinesPerIteration = 1000;
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::~avtHohlraumFluxQuery
//
//  Purpose:
//    Basic destructor
//
//  Programmer: David Bremer
//  Creation:   November 17, 2006
//
// ****************************************************************************

avtHohlraumFluxQuery::~avtHohlraumFluxQuery()
{
    delete[] radBins;
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::SetVariableNames
//
//  Purpose:
//    Set the names of absorbtion and emission variables.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
// ****************************************************************************

void
avtHohlraumFluxQuery::SetVariableNames(const stringVector &names)
{
    absVarName  = names[0];
    emisVarName = names[1];
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::SetRayCenter
//
//  Purpose:
//    Set the center point of a column of randomly distributed rays to be
//    integrated through the data.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
// ****************************************************************************

void
avtHohlraumFluxQuery::SetRayCenter(float x, float y, float z)
{
    rayCenter[0] = x;
    rayCenter[1] = y;
    rayCenter[2] = z;
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::SetRadius
//
//  Purpose:
//    Set the radius of a column of randomly distributed rays to be
//    integrated through the data.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
// ****************************************************************************

void
avtHohlraumFluxQuery::SetRadius(float r)
{
    radius = r;
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::SetThetaPhi
//
//  Purpose:
//    Set the theta and phi offsets from the z-axis in 3D, or the x-axis in 2D.
//    In 3D, theta is a rotation about the x-axis, and phi is a rotation about
//    the z-axis.  In 2D, only theta is used.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
// ****************************************************************************

void
avtHohlraumFluxQuery::SetThetaPhi(float thetaInDegrees, float phiInDegrees)
{
    theta = thetaInDegrees * M_PI / 180.0;
    phi   = phiInDegrees * M_PI / 180.0;
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::ExecuteLineScan
//
//  Purpose:
//    Glue together a set of line segments intersecting the mesh, and then
//    integrate radiation along those lines in calls to IntegrateLine.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
//  Modifications:
//    Dave Bremer, Tue Jun 19 18:33:26 PDT 2007
//    Added code to query for the widths of bins.  If they are available, they
//    will be used later to compute temperature from flux.
// ****************************************************************************

void
avtHohlraumFluxQuery::ExecuteLineScan(vtkPolyData *pd)
{
    int extraMsg = 100;
    int totalProg = totalNodes * extraMsg;
    int i;
    UpdateProgress(extraMsg*currentNode, totalProg);

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

    //Get the field variables
    vtkDataArray *absorbtivityBins;
    vtkDataArray *emissivityBins;

    absorbtivityBins = output->GetCellData()->GetArray(absVarName.c_str());
    emissivityBins   = output->GetCellData()->GetArray(emisVarName.c_str());
    if (absorbtivityBins == NULL)
    {
        char msg[256];
        sprintf(msg, "Variable %s not found.", absVarName.c_str());
        EXCEPTION1(VisItException, msg);
    }
    if (emissivityBins == NULL)
    {
        char msg[256];
        sprintf(msg, "Variable %s not found.", emisVarName.c_str());
        EXCEPTION1(VisItException, msg);
    }
    if ( emissivityBins->GetNumberOfComponents() != 
         absorbtivityBins->GetNumberOfComponents() )
    {
        EXCEPTION1(VisItException, "Number of bins for absorption and "
                                   "emission did not match.");
    }
    
    if (radBins == NULL)
    {
        numBins = emissivityBins->GetNumberOfComponents();
        radBins = new double[numBins];
        for (i = 0 ; i < numBins ; i++)
        {
            radBins[i] = 0.0;
        }
        
        avtDataAttributes &dataAttr = GetInput()->GetInfo().GetAttributes();
        const std::vector<double> &binRange = dataAttr.GetVariableBinRanges( absVarName.c_str() );
    
        if (binRange.size() != 0 && binRange.size() != numBins+1)
        {
            EXCEPTION1(VisItException, "Number of array components must equal number of bins + 1");
        }
        if (binRange.size() != 0)
        {
            binWidths.resize(binRange.size() - 1);
            for (i = 0 ; i < numBins ; i++)
            {
                binWidths[i] = binRange[i+1] - binRange[i];
            }
        }
    }
    else
    {
        if (numBins != emissivityBins->GetNumberOfComponents())
        {
            EXCEPTION1(VisItException,
                       "On nth pass, different number of bins found.");
        }
    }
    //recompute the user-defined vector
    double cosT = cos(theta);
    double sinT = sin(theta);
    double cosP = cos(phi);
    double sinP = sin(phi);
    double dir[3] = { sinT*sinP, sinT*cosP, cosT };

    //output of cleanpolydata
    output->BuildLinks();
    output->BuildCells();

    int amtPerMsg = npts / extraMsg + 1;
    UpdateProgress(extraMsg*currentNode+2*extraMsg/3, totalProg);
    int lastMilestone = 0;

    //allocate and send into IntegrateLine, to save calls to new.
    double *tmpBins = new double[numBins];

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

        IntegrateLine(oneSide, otherSide, output, lineids, lineid, dir,
                      absorbtivityBins, emissivityBins, tmpBins);


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
    delete[] tmpBins;
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::IntegrateLine
//
//  Purpose:
//    Integrate radiation along a line, factoring in absorptivity and emissivity
//    per bin the whole way along.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
// ****************************************************************************

void
avtHohlraumFluxQuery::IntegrateLine(int oneSide, int otherSide,
                                    vtkPolyData *output, vtkIntArray *lineids,
                                    int lineid, double dir[3],
                                    vtkDataArray *absorbtivityBins,
                                    vtkDataArray *emissivityBins,
                                    double *tmpBins)
{
    double pt0[3], pt1[3];
    int startPt, endPt, ii;

    output->GetPoint(oneSide,   pt0);
    output->GetPoint(otherSide, pt1);

    double lineDir[3] = {pt1[0]-pt0[0], pt1[1]-pt0[1], pt1[2]-pt0[2]};
    double dot = dir[0]*lineDir[0] + dir[1]*lineDir[1] + dir[2]*lineDir[2];

    for (ii = 0 ; ii < numBins ; ii++)
    {
        tmpBins[ii] = 0.0;
    }

    // if the dot product is positive, the vec from pt0 to pt1 is in the
    // same direction as the user-defined vector, so pt0 is the start point
    if (dot > 0.0)
    {
        startPt = oneSide;
        endPt   = otherSide;
    }
    else
    {
        startPt = otherSide;
        endPt   = oneSide;
    }

    int currPt = startPt;
    int currSeg = 0, dummySeg = 0;
    int numMatches = GetCellsForPoint(startPt, output, lineids, -1, currSeg, dummySeg);
    if (numMatches != 1)
    {
        EXCEPTION1(VisItException, "Error integrating a line");
    }
    output->GetPoint(currPt, pt0);

    while (currPt != endPt)
    {
        int newPt = -1, newSeg = -1;
        WalkChain1(output, currPt, currSeg,
                   lineids, lineid,
                   newPt, newSeg);
        output->GetPoint(newPt, pt1);

        double segLen = sqrt( (pt1[0]-pt0[0])*(pt1[0]-pt0[0]) +
                              (pt1[1]-pt0[1])*(pt1[1]-pt0[1]) +
                              (pt1[2]-pt0[2])*(pt1[2]-pt0[2]) );
        double *a = absorbtivityBins->GetTuple(currSeg);
        double *e = emissivityBins->GetTuple(currSeg);
        
        for (ii = 0 ; ii < numBins ; ii++)
        {
            double tmp = exp(-a[ii]*segLen);
            tmpBins[ii] = tmpBins[ii] * tmp + e[ii] * (1.0 - tmp);
        }
        currPt  = newPt;
        currSeg = newSeg;
        pt0[0]  = pt1[0];
        pt0[1]  = pt1[1];
        pt0[2]  = pt1[2];
    }
    for (ii = 0 ; ii < numBins ; ii++)
    {
        radBins[ii] += tmpBins[ii];
    }
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::PostExecute
//
//  Purpose:
//    Gather the results over all processes and average the results over the
//    total number of lines.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
//  Modifications:
//    Dave Bremer, Tue Jun 19 18:33:26 PDT 2007
//    Added code to use bin widths, if they are available, to compute 
//    temperature from flux.
// ****************************************************************************

void
avtHohlraumFluxQuery::PostExecute(void)
{
    double *accumBins = new double[numBins];

    SumDoubleArrayAcrossAllProcessors(radBins, accumBins, numBins);
    delete[] radBins;
    radBins = accumBins;

    int ii;
    doubleVector result(numBins);
    double resultSum = 0.0;
    double resultSum2 = 0.0;
    double temperature = 0.0;
        
    for (ii = 0 ; ii < numBins ; ii++)
    {
        result[ii] = accumBins[ii] / (double)numLines;
        resultSum  += result[ii];
        
        if (binWidths.size() != 0)
            resultSum2 += result[ii] * binWidths[ii];
    }
    if (binWidths.size() != 0)
        temperature = pow( resultSum2 / 1028000.0, 0.25 );
    
    // Write all bins to an ultra file
    char name[256];
    if (PAR_Rank() == 0)
    {
        int times = 0;
        sprintf(name, "hf%d.ult", times++);

        for (times = 0 ; times < 10000 ; times++)
        {
            ifstream ifile(name);
            if (ifile.fail())
            {
                ifile.close();
                ofstream ofile(name);
                // I would like to stick all this in the file, but the whole thing is 
                // interpreted as the variable name. -DJB
                //ofile << "# Flux values in Eu / (ms*cm2*keV).  Sum is " << resultSum << ".";
                //if (binWidths.size() != 0)
                //{
                //    ofile << "Temperature is " << temperature << ".";
                //}
                //ofile << endl;
                ofile << "# Hohlraum Flux" << endl;
                
                for (ii = 0 ; ii < numBins ; ii++)
                {
                    ofile << ii << " " << result[ii] << endl;
                }
                ofile.close();
                break;
            } 
            else
            {
                ifile.close();
                sprintf(name, "hf%d.ult", times);
            }
        }
    }

    int   msgBufSize = 512;
    char *msg = (char *)malloc(msgBufSize);
    sprintf(msg, "The hohlraum flux query over %d energy groups "
                 "was written to %s.\n", numBins, name);
    sprintf(msg + strlen(msg), "Sum is %g.\n", resultSum);
    if (binWidths.size() != 0)
        sprintf(msg + strlen(msg), "Temperature is %g.\n", temperature);
    strcat(msg, "Energy group values are: ");
    
    for (ii = 0 ; ii < numBins ; ii++)
    {
        if (strlen(msg) > (msgBufSize - 20))
        {
            msg = (char *)realloc(msg, msgBufSize + 256);
            msgBufSize += 256;
        }
        sprintf(msg + strlen(msg), " %g", result[ii]);
        
        if (ii < numBins-1)
            strcat(msg, ",");
    }
    SetResultMessage(msg);
    free(msg);

    SetResultValue(resultSum);
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::CreateLineScanFilter
//
//  Purpose:
//    Create a line scan filter that will distribute rays uniformly within a
//    cylinder, rather than the default uniform random positions and
//    orientations.
//
//  Programmer: David Bremer
//  Creation:   Dec 8, 2006
//
// ****************************************************************************

avtLineScanFilter *
avtHohlraumFluxQuery::CreateLineScanFilter()
{
    avtLineScanFilter *r = new avtLineScanFilter;
    r->SetCylinderDistrib(rayCenter, theta, phi, radius);
    return r;
}


// ****************************************************************************
//  Method: avtHohlraumFluxQuery::GetSecondaryVars
//
//  Purpose:
//    Adds the user-specified absorption and emission variables, using those 
//    for the query rather than the currently plotted variable.
//
//  Programmer: David Bremer 
//  Creation:   January 4, 2007
//
// ****************************************************************************

void
avtHohlraumFluxQuery::GetSecondaryVars( std::vector<std::string> &outVars )
{
    outVars.clear();
    outVars.push_back( absVarName );
    outVars.push_back( emisVarName );
}
