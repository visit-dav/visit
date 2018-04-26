/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
//                          vtkCurveMapper.C                                 //
// ************************************************************************* //

#include <vtkCurveMapper.h>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTrivialProducer.h>


using std::vector;


vtkStandardNewMacro( vtkCurveMapper )

//----------------------------------------------------------------------------
vtkCurveMapper::vtkCurveMapper()
  : DrawCurve(true), 
    DrawPoints(false), 
    SymbolType(0), 
    PointStride(1), 
    StaticPoints(true), 
    PointDensity(50), 
    PointSize(5.0), 
    LinesDataInitialized(false),
    PointsDataInitialized(false)
{
  this->LinesOutput->SetOutput(this->LinesPolyData.GetPointer());
  this->LinesMapper->SetInputConnection(this->LinesOutput->GetOutputPort());
  this->LinesMapper->ScalarVisibilityOff();
  this->LinesMapper->SetInterpolateScalarsBeforeMapping(0);

  this->PointsOutput->SetOutput(this->PointsPolyData.GetPointer());

  this->PointsMapper->SetInputConnection(this->PointsOutput->GetOutputPort());
  this->PointsMapper->ScalarVisibilityOff();
  this->PointsMapper->SetInterpolateScalarsBeforeMapping(0);
  this->FFScale[0] = this->FFScale[1] = this->FFScale[2] = 1.; 
}

//----------------------------------------------------------------------------
vtkCurveMapper::~vtkCurveMapper()
{
}

//----------------------------------------------------------------------------
void vtkCurveMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if (this->DrawCurve)
      this->LinesMapper->PrintSelf(os, indent);
  if (this->DrawPoints)
      this->PointsMapper->PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkCurveMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  if (this->DrawCurve)
  {
    this->UpdateLinesData(ren);
    this->LinesMapper->Render(ren,act);
  }
  if (this->DrawPoints)
  {
    this->UpdatePointsData(ren);
    this->PointsMapper->Render(ren,act);
  }
}

//----------------------------------------------------------------------------
void vtkCurveMapper::RenderPiece(vtkRenderer *ren, vtkActor *act)
{
  if (this->DrawCurve)
  {
    this->LinesMapper->RenderPiece(ren,act);
  }
  if (this->DrawPoints)
  {
    this->PointsMapper->RenderPiece(ren,act);
  }
}

//----------------------------------------------------------------------------
void vtkCurveMapper::ReleaseGraphicsResources(vtkWindow *w)
{
  this->LinesMapper->ReleaseGraphicsResources(w);
  this->PointsMapper->ReleaseGraphicsResources(w);
}

// from avtOpenGLCurveRenderer.h
//----------------------------------------------------------------------------
bool
vtkCurveMapper::GetAspect(vtkRenderer *ren, 
    int &bin_x_n, double &bin_x_size, double &bin_x_offset,
    int &bin_y_n, double &bin_y_size, double &bin_y_offset)
{
  bool success = false;
  vtkMatrix4x4 *mvtm = ren->GetActiveCamera()->GetModelViewTransformMatrix();
  if(mvtm)
  {
    double m[4][4];
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        m[i][j] = mvtm->GetElement(j, i);

    double m0 = m[0][0] * FFScale[0];
    double m5 = m[1][1] * FFScale[1];
    double lowerleft[3] = {-1., -1., 0.}; 
    ren->ViewToWorld(lowerleft[0], lowerleft[1], lowerleft[2]);
    double upperright[3] = {1., 1., 0.};
    ren->ViewToWorld(upperright[0], upperright[1], upperright[2]);
    lowerleft[0]  /= m0;
    upperright[0] /= m0;
    lowerleft[1]  /= m5;
    upperright[1] /= m5;

    //
    // figure out the size and number of bins that we want in X, Y.
    //
    double win_dx = upperright[0] - lowerleft[0];
    double win_dy = upperright[1] - lowerleft[1];
    int n_bins = this->PointDensity > 10 ? this->PointDensity : 10;
    double bin_size;
    if(win_dy > win_dx)
      bin_size = win_dy / n_bins;
    else
      bin_size = win_dx / n_bins;

    bin_x_n = int(win_dx / (bin_size / m0)) + 1;
    bin_x_size = bin_size / m0;
    bin_x_offset = lowerleft[0];

    bin_y_n = int(win_dy / (bin_size / m5)) + 1;
    bin_y_size = bin_size / m5;
    bin_y_offset = lowerleft[1];
    success = true;
  } 
  return success;
} 


// from avtOpenGLCurveRenderer
void
vtkCurveMapper::SetUpPoints(vtkRenderer *ren)
{
  int bin_x_n = 0,          bin_y_n = 0;
  double bin_x_size = 0.,   bin_y_size = 0.;
  double bin_x_offset = 0., bin_y_offset =0.;
  bool haveAspect = GetAspect(ren, bin_x_n, bin_x_size, bin_x_offset,
                              bin_y_n, bin_y_size, bin_y_offset);

  if(haveAspect)
  {
#define SWAP(a,b) { int tmp = a; a = b; b = tmp; }
#define ONE_THIRD_ROUND (2. * M_PI / 3.)
#define REDUCE_SCALE (0.8 * this->PointSize / 5.0)
#define MAX_SYMBOL_VERTS 25
#define FILL_PTS_AND_CELLS \
    if (this->SymbolType  < 4 ) /* point, triangle(s), square */ \
      ca->InsertNextCell(symbolNVerts); \
    std::vector<vtkIdType> ids; \
    for(int j = 0; j < symbolNVerts; ++j) \
    { \
      double pt[3]; \
      pt[0] = pts[0] + symbolPoints[j][0]; \
      pt[1] = pts[1] + symbolPoints[j][1]; \
      pt[2] = 0.; \
      vtkIdType id = newPts->InsertNextPoint(pt); \
      if (this->SymbolType < 4)  /* point, triangle(s), square */ \
        ca->InsertCellPoint(id); \
      else \
        ids.push_back(id); \
    } \
    if (this->SymbolType > 4) /* plus, x */ \
    { \
      ca->InsertNextCell(2); \
      ca->InsertCellPoint(ids[0]); \
      ca->InsertCellPoint(ids[1]); \
      ca->InsertNextCell(2); \
      ca->InsertCellPoint(ids[2]); \
      ca->InsertCellPoint(ids[3]); \
    } \
    else if (this->SymbolType == 4) \
    { \
      /* VTK doesn't have triagle fan, triangle strip won't work */ \
      /* so create individual triangles */ \
      for (size_t k = 1; k < ids.size()-1; ++k) \
      { \
        ca->InsertNextCell(3); \
        ca->InsertCellPoint(ids[0]); \
        ca->InsertCellPoint(ids[k]); \
        ca->InsertCellPoint(ids[k+1]); \
      } \
      ca->InsertNextCell(3); \
      ca->InsertCellPoint(ids[0]); \
      ca->InsertCellPoint(ids[ids.size()-1]); \
      ca->InsertCellPoint(ids[1]); \
    }
#define SET_PD_CELLS \
    if (this->SymbolType == 0 ) \
      this->PointsPolyData->SetVerts(ca); \
    else if (this->SymbolType > 4) \
      this->PointsPolyData->SetLines(ca); \
    else \
      this->PointsPolyData->SetPolys(ca);

    double symbolPoints[MAX_SYMBOL_VERTS][2];
    int symbolNVerts = 0;
    if (this->SymbolType == 0) // Point
    {
      symbolNVerts = 1;
      symbolPoints[0][0] = 0.;
      symbolPoints[0][1] = 0.;
    }
    else if(this->SymbolType == 1) // TriangleUp
    {
      symbolNVerts = 3;
      symbolPoints[0][0] = cos(M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[0][1] = sin(M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[1][0] = cos(M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[1][1] = sin(M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[2][0] = cos(M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[2][1] = sin(M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
    }
    else if(this->SymbolType == 2) // TriangleDown
    {
      symbolNVerts = 3;
      symbolPoints[0][0] = cos(3*M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[0][1] = sin(3*M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[1][0] = cos(3*M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[1][1] = sin(3*M_PI/2. + ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[2][0] = cos(3*M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[2][1] = sin(3*M_PI/2. + 2.*ONE_THIRD_ROUND) * REDUCE_SCALE * bin_y_size / 2.;
    }
    else if(this->SymbolType == 3) // Square
    {
      symbolNVerts = 4;
      symbolPoints[0][0] = REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[0][1] = REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[1][0] = -REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[1][1] = REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[2][0] = -REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[2][1] = -REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[3][0] = REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[3][1] = -REDUCE_SCALE * bin_y_size / 2.;
    }
    else if(this->SymbolType == 4) // Circle
    {
      symbolNVerts = MAX_SYMBOL_VERTS;
      symbolPoints[0][0] = 0.f;
      symbolPoints[0][1] = 0.f;
      for(int i = 0; i < MAX_SYMBOL_VERTS-1; ++i)
      {
        double t = double(i) / double(MAX_SYMBOL_VERTS-1-1);
        double angle = 2. * M_PI * t;
        symbolPoints[i+1][0] = cos(angle) * REDUCE_SCALE * bin_x_size / 2.;
        symbolPoints[i+1][1] = sin(angle) * REDUCE_SCALE * bin_y_size / 2.;
      }
    }
    else if(this->SymbolType == 5) // Plus
    {
      symbolNVerts = 4;
      symbolPoints[0][0] = 0.f;
      symbolPoints[0][1] = -REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[1][0] = 0.f;
      symbolPoints[1][1] = REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[2][0] = -REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[2][1] = 0.f;
      symbolPoints[3][0] = REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[3][1] = 0.f;
    }
    else if(this->SymbolType == 6) // X
    {
      symbolNVerts = 4;
      symbolPoints[0][0] = cos(M_PI/4. + M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[0][1] = sin(M_PI/4. + M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[1][0] = cos(M_PI/4. + M_PI + M_PI/2.) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[1][1] = sin(M_PI/4. + M_PI + M_PI/2.) * REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[2][0] = cos(M_PI/4. + 0.) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[2][1] = sin(M_PI/4. + 0.) * REDUCE_SCALE * bin_y_size / 2.;
      symbolPoints[3][0] = cos(M_PI/4. + M_PI) * REDUCE_SCALE * bin_x_size / 2.;
      symbolPoints[3][1] = sin(M_PI/4. + M_PI) * REDUCE_SCALE * bin_y_size / 2.;
    }
    else // shouldn't happen, but let's default to Point, for completeness
    {
      vtkWarningMacro("vtkCurveMapper encountered invalid symbol type, "
                      "reverting to Point\n");
      symbolNVerts = 1;
      symbolPoints[0][0] = 0.;
      symbolPoints[0][1] = 0.;
    }

    vtkCellArray *ca = vtkCellArray::New();
    vtkPoints *inPts = this->PointsPolyData->GetPoints();
    vtkPoints *newPts = inPts->NewInstance();
    vtkIdType nPts = inPts->GetNumberOfPoints();
    // static case
    if (this->StaticPoints)
    {
      double pts[3] = {0., 0., 0.};
      for(vtkIdType i = 0; i < nPts-1; i+=this->PointStride)
      {
        inPts->GetPoint(i, pts);

#if 0
        if (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() < pts[0])
          continue;
#endif
        FILL_PTS_AND_CELLS
      }
      // add the last point.
      inPts->GetPoint(nPts-1, pts);
#if 0
      if (!atts.GetDoCropTimeCue() || 
       (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() >= pts[0]))
#endif
      {
        FILL_PTS_AND_CELLS
      }
      this->PointsPolyData->SetPoints(newPts);
      SET_PD_CELLS
    }
    else
    {
      vtkPoints *inPts = this->PointsPolyData->GetPoints();
      int nPts = inPts->GetNumberOfPoints();
      vtkCellArray *ca = vtkCellArray::New();
      for (int i = 1; i < nPts; ++i)
      {
        double A[3], B[3];
        inPts->GetPoint(i-1, A);
        inPts->GetPoint(i,   B);

        // Determine the grid cells that contain the points.
        int x0 = int((A[0] - bin_x_offset) / bin_x_size);
        int y0 = int((A[1] - bin_y_offset) / bin_y_size);

        int x1 = int((B[0] - bin_x_offset) / bin_x_size);
        int y1 = int((B[1] - bin_y_offset) / bin_y_size);

        // Use Bresenham's line algorithm to produce a number of
        // cells encountered along the line segment.
        bool steep = abs(y1 - y0) > abs(x1 - x0);
        if(steep)
        {
          SWAP(x0, y0);
          SWAP(x1, y1);
        }
        if(x0 > x1)
        {
          SWAP(x0, x1);
          SWAP(y0, y1);
        }
        int deltax = x1 - x0;
        int deltay = y1 - y0;
        if(deltay < 0)
          deltay = -deltay;
        int err = 0;
        int y = y0;
        int ystep = (y0 < y1) ? 1 : -1;
        int cells_in_line = 0;
        for(int x = x0; x <= x1; ++x)
        {
          cells_in_line++;
          err = err + deltay;
          if((err << 1) >= deltax)
          {
            y += ystep;
            err -= deltax;
          }
        }

        // Use the number of cells along the way between the line
        // end points to calculate intermediate points at which to 
        // put symbols.
        double pts[2];
        for(int pindex = 0; pindex < cells_in_line; ++pindex)
        {
          double t = (cells_in_line == 1) ? 0. : (double(pindex) / double(cells_in_line-1));

          pts[0] = (1.-t)*A[0] + t*B[0];
#if 0
          if (atts.GetDoCropTimeCue() && atts.GetTimeForTimeCue() < pts[0]])
            continue;
#endif
          pts[1] = (1.-t)*A[1] + t*B[1];
          FILL_PTS_AND_CELLS
        }
      }
      this->PointsPolyData->SetPoints(newPts);
      SET_PD_CELLS
      ca->Delete();
    } // dynamic
  }
}


//----------------------------------------------------------------------------
void vtkCurveMapper::UpdateLinesData(vtkRenderer *ren)
{
  vtkPolyData *input = this->GetInput();
  if (!this->LinesDataInitialized || (
      (input->GetMTime() > this->LinesPolyData->GetMTime() ||
      this->GetMTime() > this->LinesPolyData->GetMTime()) ))
  {
    this->LinesPolyData->Initialize();
    this->LinesPolyData->SetPoints(input->GetPoints());
    vtkCellArray *polyLines = vtkCellArray::New();
    polyLines->InsertNextCell(input->GetNumberOfPoints());
    for (int i = 0; i < input->GetNumberOfPoints(); ++i)
        polyLines->InsertCellPoint(i);

    this->LinesPolyData->SetLines(polyLines);
    polyLines->Delete();
    this->LinesPolyData->GetPointData()->PassData(input->GetPointData());
    // We have changed a bunch of line segment cells to a single polyline cell,
    // so we shouldn't pass the cell data along???
    //this->LinesPolyData->GetCellData()->PassData(input->GetCellData());
    this->LinesDataInitialized = true;
    this->LinesMapper->Update();
  }
}

//----------------------------------------------------------------------------
void vtkCurveMapper::UpdatePointsData(vtkRenderer *ren)
{
  vtkPolyData *input = this->GetInput();
  if (!this->PointsDataInitialized || 
      ( (input->GetMTime() > this->PointsPolyData->GetMTime() ||
         this->GetMTime()  > this->PointsPolyData->GetMTime()) ) ||
      ( ren->GetActiveCamera()->GetMTime() > this->PointsPolyData->GetMTime()))
  {
    this->PointsPolyData->Initialize();

    vtkPoints *newPts = input->GetPoints()->NewInstance();
    newPts->DeepCopy(input->GetPoints());

    this->PointsPolyData->SetPoints(newPts);
    newPts->Delete();

    this->SetUpPoints(ren);
    this->PointsPolyData->GetPointData()->PassData(input->GetPointData());
    this->PointsPolyData->GetCellData()->PassData(input->GetCellData());
    this->PointsDataInitialized = true;
    this->PointsMapper->Update();
  }
}


