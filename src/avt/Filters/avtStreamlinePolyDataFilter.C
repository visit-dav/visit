/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
//                        avtStreamlinePolyDataFilter.C                      //
// ************************************************************************* //

#include <avtStreamlinePolyDataFilter.h>

#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkCleanPolyData.h>

#include <avtCallback.h>
#include <avtParallel.h>
#include <avtStateRecorderIntegralCurve.h>
#include <avtStreamlineIC.h>

std::string avtStreamlinePolyDataFilter::colorvarArrayName = "colorVar";
std::string avtStreamlinePolyDataFilter::paramArrayName = "params";
std::string avtStreamlinePolyDataFilter::opacityArrayName = "opacity";
std::string avtStreamlinePolyDataFilter::thetaArrayName = "theta";
std::string avtStreamlinePolyDataFilter::tangentsArrayName = "tangents";

// ****************************************************************************
//  Method: avtStreamlineFilter::CreateIntegralCurveOutput
//
//  Purpose:
//      Create the VTK poly data output from the streamline.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Add dataSpatialDimension.
//
//   Hank Childs, Tue Dec  2 13:51:19 PST 2008
//   Removed this method from avtStreamlineFilter to 
//   avtStreamlinePolyDataFilter.  The motivation for this refactoring was to
//   allow other modules (Poincare) to inherit from avtStreamlineFilter and
//   use its parallel-aware goodness.
//
//   Dave Pugmire, Tue Dec 23 13:52:42 EST 2008
//   Removed ReportStatistics from this method.
//
//   Dave Pugmire, Tue Feb  3 11:00:54 EST 2009
//   Changed debugs.
//
//   Dave Pugmire, Tue Dec 29 14:37:53 EST 2009
//   Add custom renderer and lots of appearance options to the streamlines plots.
//
//   Hank Childs, Fri Jun  4 19:58:30 CDT 2010
//   Use avtStreamlines, not avtStreamlineWrappers.
//
//   Hank Childs, Sun Jun  6 12:21:30 CDT 2010
//   Rename this method to reflect the new emphasis in particle advection, as
//   opposed to streamlines.
//
//   Dave Pugmire, Tue Sep 28 10:41:00 EDT 2010
//   Optimize the creation of vtkPolyData.
//
//   Dave Pugmire, Wed Sep 29 14:57:59 EDT 2010
//   Initialize scalar array if coloring by solid.
//
//   Hank Childs, Wed Oct  6 20:07:28 PDT 2010
//   Initialize referenceTypeForDisplay.
//
//   Hank Childs, Fri Oct  8 14:57:13 PDT 2010
//   Check to see if any curves terminated because of the steps criteria.
//
//   Dave Pugmire, Thu Dec  2 12:49:33 EST 2010
//   Can't early return until after collective communication.
//
//   Hank Childs, Sun Dec  5 10:43:57 PST 2010
//   Issue warnings for more problems.
//
// ****************************************************************************

void
avtStreamlinePolyDataFilter::CreateIntegralCurveOutput(vector<avtIntegralCurve *> &ics)
{
    debug5 << "::CreateIntegralCurveOutput " << ics.size() << endl;
    int numICs = ics.size(), numPts = 0;
    int numEarlyTerminators = 0;
    int numStiff = 0;
    int numCritPts = 0;

    //See how many pts, ics we have so we can preallocate everything.
    for (int i = 0; i < numICs; i++)
    {
        avtStreamlineIC *ic = dynamic_cast<avtStreamlineIC*>(ics[i]);
        size_t numSamps = (ic ? ic->GetNumberOfSamples() : 0);
        if (numSamps > 1)
            numPts += numSamps;

        if (ic->TerminatedBecauseOfMaxSteps())
        {
            if (ic->SpeedAtTermination() <= criticalPointThreshold)
                numCritPts++;
            else
                numEarlyTerminators++;
        }
        if (ic->EncounteredNumericalProblems())
            numStiff++;
    }

    if ((doDistance || doTime) && issueWarningForMaxStepsTermination)
    {
        SumIntAcrossAllProcessors(numEarlyTerminators);
        if (numEarlyTerminators > 0)
        {
            char str[1024];
            SNPRINTF(str, 1024, 
               "%d of your streamlines terminated because they "
               "reached the maximum number of steps.  This may be indicative of your "
               "time or distance criteria being too large or of other attributes being "
               "set incorrectly (example: your step size is too small).  If you are "
               "confident in your settings and want the particles to advect farther, "
               "you should increase the maximum number of steps.  If you want to disable "
               "this message, you can do this under the Advaced tab of the streamline plot."
               "  Note that this message does not mean that an error has occurred; it simply "
               "means that VisIt stopped advecting particles because it reached the maximum "
               "number of steps. (That said, this case happens most often when other attributes "
               "are set incorrectly.)", numEarlyTerminators);
            avtCallback::IssueWarning(str);
        }
    }
    if (issueWarningForCriticalPoints)
    {
        SumIntAcrossAllProcessors(numCritPts);
        if (numCritPts > 0)
        {
            char str[1024];
            SNPRINTF(str, 1024, 
               "%d of your streamlines circled round and round a critical point (a zero"
               " velocity location).  Normally, VisIt is able to advect the particle "
               "to the critical point location and terminate.  However, VisIt was not able "
               "to do this for these particles due to numerical issues.  In all likelihood, "
               "additional steps will _not_ help this problem and only cause execution to "
               "take longer.  If you want to disable this message, you can do this under "
               "the Advanced tab of the streamline plot.", numCritPts);
            avtCallback::IssueWarning(str);
        }
    }
    if (issueWarningForStiffness)
    {
        SumIntAcrossAllProcessors(numStiff);
        if (numStiff > 0)
        {
            char str[1024];
            SNPRINTF(str, 1024, 
               "%d of your streamlines were unable to advect because of \"stiffness\".  "
               "When one component of a velocity field varies quickly and another stays "
               "relatively constant, then it is not possible to choose step sizes that "
               "remain within tolerances.  This condition is referred to as stiffness and "
               "VisIt stops advecting in this case.  If you want to disable this message, "
               "you can do this under the Advanced tab of the streamline plot.", numStiff);
            avtCallback::IssueWarning(str);
        }
    }

    if (numICs == 0)
        return;

    //Make a polydata.
    vtkPoints     *points   = vtkPoints::New();
    vtkCellArray  *lines    = vtkCellArray::New();
    vtkFloatArray *scalars  = vtkFloatArray::New();
    vtkFloatArray *params   = vtkFloatArray::New();
    vtkFloatArray *tangents = vtkFloatArray::New();
    vtkFloatArray *thetas   = NULL;
    vtkFloatArray *opacity  = NULL;

    lines->Allocate(numICs);
    points->Allocate(numPts);
    scalars->Allocate(numPts);
    params->Allocate(numPts);
    tangents->SetNumberOfComponents(3);
    tangents->SetNumberOfTuples(numPts);
    
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->SetLines(lines);
    scalars->SetName(colorvarArrayName.c_str());
    params->SetName(paramArrayName.c_str());
    tangents->SetName(tangentsArrayName.c_str());

    pd->GetPointData()->AddArray(scalars);
    pd->GetPointData()->AddArray(params);
    pd->GetPointData()->AddArray(tangents);

    if (displayMethod == STREAMLINE_DISPLAY_RIBBONS)
    {
        thetas = vtkFloatArray::New();
        thetas->Allocate(numPts);
        thetas->SetName(thetaArrayName.c_str());
        pd->GetPointData()->AddArray(thetas);
    }
    if (opacityVariable != "")
    {
        opacity = vtkFloatArray::New();
        opacity->Allocate(numPts);
        opacity->SetName(opacityArrayName.c_str());
        pd->GetPointData()->AddArray(opacity);
    }

    vtkIdType pIdx = 0, idx = 0;
    for (int i = 0; i < numICs; i++)
    {
        avtStateRecorderIntegralCurve *ic = dynamic_cast<avtStateRecorderIntegralCurve*>(ics[i]);
        size_t numSamps = (ic ? ic->GetNumberOfSamples() : 0);
        if (numSamps <= 1)
            continue;
        
        vtkPolyLine *line = vtkPolyLine::New();
        line->GetPointIds()->SetNumberOfIds(numSamps);

        float theta = 0.0, prevT = 0.0;
        avtVector lastPos;

        cerr << phiFactor << "  " << (phiFactor == 0.0) << endl;

        for (int j = 0; j < numSamps; j++)
        {
            avtStateRecorderIntegralCurve::Sample s = ic->GetSample(j);
            line->GetPointIds()->SetId(j, pIdx);

            if( coordinateSystem == 0 )
              points->InsertPoint(pIdx, s.position.x, s.position.y, s.position.z);
            else if( coordinateSystem == 1 )
              points->InsertPoint(pIdx, 
                                  s.position.x*cos(s.position.y),
                                  s.position.x*sin(s.position.y),
                                  s.position.z);
            else if( coordinateSystem == 2 )
              points->InsertPoint(pIdx, 
                                  sqrt(s.position.x*s.position.x+
                                       s.position.y*s.position.y),
                                  (phiFactor == 0.0 ?
                                   atan2( s.position.y, s.position.x ) :
                                   (double) (j) / phiFactor),
                                  s.position.z);
            
            float speed = s.velocity.length();
            if (speed > 0)
                s.velocity *= 1.0f/speed;
            tangents->InsertTuple3(pIdx, s.velocity.x, s.velocity.y, s.velocity.z);

            // color scalars
            switch (coloringMethod)
            {
              case STREAMLINE_COLOR_TIME:
                scalars->InsertTuple1(pIdx, s.time);
                break;
              case STREAMLINE_COLOR_SPEED:
                scalars->InsertTuple1(pIdx, speed);
                break;
              case STREAMLINE_COLOR_VORTICITY:
                scalars->InsertTuple1(pIdx, s.vorticity);
                break;
              case STREAMLINE_COLOR_ARCLENGTH:
                scalars->InsertTuple1(pIdx, s.arclength);
                break;
              case STREAMLINE_COLOR_VARIABLE:
                scalars->InsertTuple1(pIdx, s.scalar0);
                break;
              case STREAMLINE_COLOR_ID:
                scalars->InsertTuple1(pIdx, ic->id);
                break;
              case STREAMLINE_COLOR_SOLID:
                scalars->InsertTuple1(pIdx, 0.0f);
            }

            // parameter scalars
            switch (referenceTypeForDisplay)
            {
              case 0: // Distance
                params->InsertTuple1(pIdx, s.arclength);
                break;
              case 1: // Time
                params->InsertTuple1(pIdx, s.time);
                break;
              case 2: // Steps
                params->InsertTuple1(pIdx, j);
                break;
            }
            
            // opacity/theta scalars
            if (opacity)
                opacity->InsertTuple1(pIdx, s.scalar1);
            if (thetas)
            {
                float scaledVort = s.vorticity * (prevT-s.time);
                theta += scaledVort;
                thetas->InsertTuple1(pIdx, theta);
                prevT = s.time;
            }
            
            pIdx++;
            lastPos = s.position;
        }
        
        lines->InsertNextCell(line);
        line->Delete();
        idx++;
    }
    
    points->Delete();
    lines->Delete();
    scalars->Delete();
    params->Delete();
    tangents->Delete();
    if (thetas)
        thetas->Delete();
    if (opacity)
        opacity->Delete();

    vtkCleanPolyData *clean = vtkCleanPolyData::New();
    clean->ConvertLinesToPointsOff();
    clean->ConvertPolysToLinesOff();
    clean->ConvertStripsToPolysOff();
    clean->PointMergingOn();
    clean->SetInput(pd);
    clean->Update();
    pd->Delete();

    vtkPolyData *cleanPD = clean->GetOutput();

    avtDataTree *dt = new avtDataTree(cleanPD, 0);
    SetOutputDataTree(dt);

    clean->Delete();

    /*
    if (1)
    {
        char f[51];
        sprintf(f, "streamlines_%03d.txt", PAR_Rank());
        FILE *fp = fopen(f, "w");
        for (int i = 0; i < numICs; i++)
        {
            avtStateRecorderIntegralCurve *ic = dynamic_cast<avtStateRecorderIntegralCurve*>(ics[i]);
            size_t numSamps = (ic ? ic->GetNumberOfSamples() : 0);
            if (numSamps == 0)
                continue;

            fprintf(fp, "%d\n", (int)numSamps);
            for (int j = 0; j < numSamps; j++)
            {
                avtStateRecorderIntegralCurve::Sample s = ic->GetSample(j);
                fprintf(fp, "%lf %lf %lf %lf %lf\n", s.position.x, s.position.y, s.position.z, s.time, s.scalar0);
            
            }
        }
        fflush(fp);
        fclose(fp);
    }
    */
}
