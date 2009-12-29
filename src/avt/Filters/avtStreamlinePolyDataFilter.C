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
//                        avtStreamlinePolyDataFilter.C                      //
// ************************************************************************* //

#include <avtStreamlinePolyDataFilter.h>
#include "avtStreamlineWrapper.h"
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCleanPolyData.h>
#include <vtkFloatArray.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyLine.h>
#include <vtkRibbonFilter.h>
#include <vtkTubeFilter.h>


std::string avtStreamlinePolyDataFilter::colorvarArrayName = "colorVar";
std::string avtStreamlinePolyDataFilter::paramArrayName = "params";
std::string avtStreamlinePolyDataFilter::opacityArrayName = "opacity";
std::string avtStreamlinePolyDataFilter::thetaArrayName = "theta";


// ****************************************************************************
//  Method: avtStreamlineFilter::CreateStreamlineOutput
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
// ****************************************************************************

void
avtStreamlinePolyDataFilter::CreateStreamlineOutput(
                                   vector<avtStreamlineWrapper *> &streamlines)
{
    debug5 << "::CreateStreamlineOutput " << streamlines.size() << endl;

    bool doTubes = displayMethod == STREAMLINE_DISPLAY_TUBES;
    bool doRibbons  = displayMethod == STREAMLINE_DISPLAY_RIBBONS;
    
    if (streamlines.size() == 0)
        return;

    // Join all the streamline pieces.
    vtkAppendPolyData *append = vtkAppendPolyData::New();
    for (int i = 0; i < streamlines.size(); i++)
    {
        avtStreamlineWrapper *slSeg = (avtStreamlineWrapper *) streamlines[i];
        vector<float> thetas;
        vtkPolyData *pd = GetVTKPolyData(slSeg->sl, slSeg->id);

        if (pd == NULL)
            continue;

        vtkCleanPolyData *clean = vtkCleanPolyData::New();
        clean->SetInput(pd);
        clean->Update();
        pd->Delete();

        pd = clean->GetOutput();
        pd->Register(NULL);
        pd->SetSource(NULL);
        clean->Delete();

        append->AddInput(pd);

        pd->Delete();
    }

    append->Update();
    vtkPolyData *outPD = append->GetOutput();
    outPD->Register(NULL);
    outPD->SetSource(NULL);
    append->Delete();

    avtDataTree *dt = new avtDataTree(outPD, 0);
    SetOutputDataTree(dt);
}


// ****************************************************************************
//  Method: avtStreamlinePolyDataFilter::GetVTKPolyData
//
//  Purpose:
//      Converts the avtStreamline into a VTK poly data object.
//
//  Programmer: Dave Pugmire
//  Creation:   June 16, 2008
//
//  Modifications:
//
//   Dave Pugmire, Wed Aug 13 14:11:04 EST 2008
//   Step derivative is not giving the right answer. So, use the velEnd vector
//   for coloring by speed.
//
//   Dave Pugmire, Fri Aug 22 14:47:11 EST 2008
//   Add new coloring methods, length, time and ID.
//
//   Dave Pugmire, Mon Feb  2 14:39:35 EST 2009
//   Moved this method from avtStreamlineWrapper to avtStreamlinePolyDataFilter.
//
//   Dave Pugmire, Mon Jun 8 2009, 11:44:01 EDT 2009
//   Handle color by variable.
//
// ****************************************************************************

vtkPolyData *
avtStreamlinePolyDataFilter::GetVTKPolyData(avtStreamline *sl, int id)
{
    if (sl == NULL || sl->size() == 0)
        return NULL;

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *cells = vtkCellArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();
    vtkFloatArray *params = vtkFloatArray::New();
    vtkFloatArray *thetas = NULL;
    vtkFloatArray *opacity = NULL;

    if (displayMethod == STREAMLINE_DISPLAY_RIBBONS)
        thetas = vtkFloatArray::New();

    int opacityIdx = -1;
    if (opacityVariable != "")
    {
        opacityIdx = sl->GetVariableIdx(opacityVariable);
        if (opacityIdx == -1)
            EXCEPTION1(ImproperUseException, "Unknown opacity variable.");
        opacity = vtkFloatArray::New();
    }
    
    cells->InsertNextCell(sl->size());
    scalars->Allocate(sl->size());
    params->Allocate(sl->size());
    avtStreamline::iterator siter;
    
    unsigned int i = 0;
    float val = 0.0, theta = 0.0, param = 0.0;
    for(siter = sl->begin(); siter != sl->end(); ++siter, i++)
    {
        points->InsertPoint(i, (*siter)->front()[0], (*siter)->front()[1], 
                            (dataSpatialDimension > 2 ? (*siter)->front()[2] : 0.0));
        cells->InsertCellPoint(i);

        avtIVPStep *step = (*siter);

        // Set the color-by scalar.
        if (coloringMethod == STREAMLINE_COLOR_SPEED)
        {
            val = step->speed;
        }
        else if (coloringMethod == STREAMLINE_COLOR_VARIABLE)
        {
            val = step->scalarValue;
        }
        else if (coloringMethod == STREAMLINE_COLOR_VORTICITY)
        {
            double dT = (step->tEnd - step->tStart);
            val = step->vorticity * dT;
        }
        else if (coloringMethod ==  STREAMLINE_COLOR_ARCLENGTH)
        {
            val += step->length();
        }
        else if (coloringMethod ==  STREAMLINE_COLOR_TIME)
        {
            val = step->tEnd;
        }
        else if (coloringMethod ==  STREAMLINE_COLOR_ID)
        {
            val = (float)id;
        }

        if (terminationType == avtIVPSolver::TIME)
            param = step->tEnd;
        else if (terminationType == avtIVPSolver::DISTANCE)
            param += step->length();
        else if (terminationType == avtIVPSolver::STEPS)
            param = param+1.0;
        else
            param = param+1.0;
        
        //Ribbon display, record the angle.
        if (displayMethod == STREAMLINE_DISPLAY_RIBBONS)
        {
            double dT = (step->tEnd - step->tStart);
            float scaledVort = step->vorticity * dT;
            theta += scaledVort;
            thetas->InsertTuple1(i,theta);
        }
        if (opacity)
        {
            opacity->InsertTuple1(i, step->scalarValues[opacityIdx]);
        }
        
        scalars->InsertTuple1(i, val);
        params->InsertTuple1(i, param);
    }
    
    //Create the polydata.
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->SetLines(cells);
    scalars->SetName(colorvarArrayName.c_str());
    params->SetName(paramArrayName.c_str());
    
    pd->GetPointData()->AddArray(scalars);
    pd->GetPointData()->AddArray(params);
    
    if (thetas)
    {
        thetas->SetName(thetaArrayName.c_str());
        pd->GetPointData()->AddArray(thetas);
        thetas->Delete();
    }
    if (opacity)
    {
        opacity->SetName(thetaArrayName.c_str());
        pd->GetPointData()->AddArray(opacity);
        opacity->Delete();
    }

    points->Delete();
    cells->Delete();
    scalars->Delete();
    params->Delete();

    return pd;
}

