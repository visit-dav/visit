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
        vtkPolyData *pd = GetVTKPolyData(slSeg->sl, slSeg->id, thetas);
        
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

        if (showStart)
        {
            float val = pd->GetPointData()->GetScalars()->GetTuple1(0);
            double *pt = NULL;
            if (slSeg->dir == avtStreamlineWrapper::FWD)
                pt = pd->GetPoints()->GetPoint(0);
            else
                pt = pd->GetPoints()->GetPoint(
                                       pd->GetPoints()->GetNumberOfPoints()-1);
            vtkPolyData *ball = StartSphere(val, pt);
            
            append->AddInput(ball);
            ball->Delete();
        }
        
        if (doTubes)
        {
            vtkTubeFilter* tubes = vtkTubeFilter::New();
            tubes->SetRadius(radius);
            tubes->SetNumberOfSides(8);
            tubes->SetRadiusFactor(2.);
            tubes->SetCapping(1);
            tubes->ReleaseDataFlagOn();
            tubes->SetInput(pd);
            tubes->Update();
            
            pd->Delete();
            pd = tubes->GetOutput();
            pd->Register(NULL);
            pd->SetSource(NULL);
            tubes->Delete();
            
            append->AddInput(pd);
        }
        else if (doRibbons)
        {
            vtkRibbonFilter* ribbons = vtkRibbonFilter::New();
            ribbons->SetWidth(radius);

            int nPts = pd->GetPointData()->GetNumberOfTuples();
            
            vtkIdList *ids = vtkIdList::New();
            vtkPoints *pts = vtkPoints::New();
            vtkCellArray *lines = vtkCellArray::New();
            for (int i = 0; i < nPts; i++)
            {
                vtkIdType id = pts->InsertNextPoint(
                                                 pd->GetPoints()->GetPoint(i));
                ids->InsertNextId(id);
            }

            lines->InsertNextCell(ids);
            //Create normals, initialize them. (Remove the init later....)
            vtkFloatArray *normals = vtkFloatArray::New();
            normals->SetNumberOfComponents(3);
            normals->SetNumberOfTuples(nPts);

            vtkPolyLine *lineNormalGenerator = vtkPolyLine::New();
            lineNormalGenerator->GenerateSlidingNormals(pts, lines, normals);
            
            //Now, rotate the normals according to the vorticity..
            //double normal[3], local1[3], local2[3],length,costheta, sintheta;
            double normal[3], tan[3], biNormal[3], p0[3], p1[3];
            for (int i = 0; i < nPts; i++)
            {
                double theta = thetas[i];

                pts->GetPoint(i, p0);
                if (i < nPts-1)
                    pts->GetPoint(i+1, p1);
                else
                {
                    pts->GetPoint(i-1, p0);
                    pts->GetPoint(i, p1);
                }
                for (int j = 0; j < 3; j++)
                    tan[j] = p1[j]-p0[j];

                //cout<<i<<": p= ["<<p0[0]<<" "<<p0[1]<<" "<<p0[2]<<"] ["
                //    <<p1[0]<<" "<<p1[1]<<" "<<p1[2]<<"]\n";
                //cout<<i<<": T=["<<tan[0]<<" "<<tan[1]<<" "<<tan[2]<<"]\n\n";
                normals->GetTuple(i, normal);
                vtkMath::Normalize(tan);
                vtkMath::Normalize(normal);

                vtkMath::Cross(normal, tan, biNormal);
                double cosTheta = cos(theta);
                double sinTheta = sin(theta);
                for (int j = 0; j < 3; j++)
                    normal[j] = cosTheta*normal[j] + sinTheta*biNormal[j];
                
                //cout<<i<<": T=["<<tan[0]<<" "<<tan[1]<<" "<<tan[2]<<"] N= ["
                //    <<normal[0]<<" "<<normal[1]<<" "<<normal[2]<<endl;
                normals->SetTuple(i,normal);
            }

            ids->Delete();
            pts->Delete();
            lines->Delete();
            
            pd->GetPointData()->SetNormals(normals);
            normals->Delete();
            lineNormalGenerator->Delete();

            ribbons->SetInput(pd);
            ribbons->Update();
            
            pd->Delete();
            pd = ribbons->GetOutput();
            pd->Register(NULL);
            pd->SetSource(NULL);

            ribbons->Delete();
            append->AddInput(pd);
        }
        else
        {
            append->AddInput(pd);
            pd->Delete();
        }
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
// ****************************************************************************

vtkPolyData *
avtStreamlinePolyDataFilter::GetVTKPolyData(avtStreamline *sl,
                                            int id,
                                            vector<float> &thetas)
{
    if (sl == NULL || sl->size() == 0)
        return NULL;

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *cells = vtkCellArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();
    
    cells->InsertNextCell(sl->size());
    scalars->Allocate(sl->size());
    avtStreamline::iterator siter;
    
    unsigned int i = 0;
    float val = 0.0, theta = 0.0;
    debug5<<"Create vtkPolyData\n";
    for(siter = sl->begin(); siter != sl->end(); ++siter, i++)
    {
        debug5<<i<<": "<< (*siter)->front()<<endl;
        points->InsertPoint(i, (*siter)->front()[0], (*siter)->front()[1], 
                            (dataSpatialDimension > 2 ? (*siter)->front()[2] : 0.0));
        cells->InsertCellPoint(i);

        avtIVPStep *step = (*siter);

        // Set the speed/vorticity.
        if (coloringMethod == STREAMLINE_COLOR_SPEED)
        {
            avtVec deriv = step->velEnd;
            val = deriv.values()[0]*deriv.values()[0] 
                + deriv.values()[1]*deriv.values()[1];
            if (dataSpatialDimension == 3)
                val += deriv.values()[2]*deriv.values()[2];
            val = sqrt(val);
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
        
        if (coloringMethod == STREAMLINE_COLOR_VORTICITY || 
            displayMethod == STREAMLINE_DISPLAY_RIBBONS)
        {
            double dT = (step->tEnd - step->tStart);
            float scaledVort = step->vorticity * dT;
            theta += scaledVort;
            thetas.push_back(theta);
            if (coloringMethod == STREAMLINE_COLOR_VORTICITY)
                val = scaledVort;
        }
        
        scalars->InsertTuple1(i, val);
    }
    
    //Create the polydata.
    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(points);
    pd->SetLines(cells);
    scalars->SetName("colorVar");
    pd->GetPointData()->SetScalars(scalars);

    points->Delete();
    cells->Delete();
    scalars->Delete();

    return pd;
}

