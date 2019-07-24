// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.


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

#ifndef BALL_CUE_VERTS
#define BALL_CUE_VERTS 100
#endif

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
    ViewScale(1.0),
    TimeForTimeCue(0.0),
    DoBallTimeCue(false),
    TimeCueBallSize(0.0),
    DoLineTimeCue(false),
    TimeCueLineWidth(0.0),
    DoCropTimeCue(false),
    LinesDataInitialized(false),
    PointsDataInitialized(false),
    BallCueDataInitialized(false),
    LineCueDataInitialized(false)
{
  this->LinesOutput->SetOutput(this->LinesPolyData.GetPointer());
  this->LinesMapper->SetInputConnection(this->LinesOutput->GetOutputPort());
  this->LinesMapper->ScalarVisibilityOff();
  this->LinesMapper->SetInterpolateScalarsBeforeMapping(0);

  this->PointsOutput->SetOutput(this->PointsPolyData.GetPointer());

  this->PointsMapper->SetInputConnection(this->PointsOutput->GetOutputPort());
  this->PointsMapper->ScalarVisibilityOff();
  this->PointsMapper->SetInterpolateScalarsBeforeMapping(0);

  this->BallCueOutput->SetOutput(this->BallCuePolyData.GetPointer());
  this->BallCueMapper->SetInputConnection(this->BallCueOutput->GetOutputPort());
  this->BallCueMapper->ScalarVisibilityOff();
  this->BallCueMapper->SetInterpolateScalarsBeforeMapping(0);

  this->LineCueMapper->SetInputConnection(this->LineCueData->GetOutputPort());
  this->LineCueMapper->ScalarVisibilityOff();
  this->LineCueMapper->SetInterpolateScalarsBeforeMapping(0);

  this->FFScale[0] = this->FFScale[1] = this->FFScale[2] = 1.;
  this->TimeCueBallColor[0] =
    this->TimeCueBallColor[1] = this->TimeCueBallColor[2] = 1.;
  this->TimeCueLineColor[0] =
    this->TimeCueLineColor[1] = this->TimeCueLineColor[2] = 1.;
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
  if (this->DoBallTimeCue)
      this->BallCueMapper->PrintSelf(os, indent);
  if (this->DoLineTimeCue)
      this->LineCueMapper->PrintSelf(os, indent);
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
  if (this->DoBallTimeCue)
  {
    this->UpdateBallCueData(ren);

    //
    // We need to temporarily change the actor color for
    // the ball cue.
    //
    double prevColor[3];
    act->GetProperty()->GetColor(prevColor);
    act->GetProperty()->SetColor(this->TimeCueBallColor);

    this->BallCueMapper->Render(ren, act);

    act->GetProperty()->SetColor(prevColor);
  }
  if (this->DoLineTimeCue)
  {
    this->UpdateLineCueData(ren);

    //
    // We need to temporarily change the actor color and
    // line width for the line cue.
    //
    double prevColor[3];
    double prevWidth;
    act->GetProperty()->GetColor(prevColor);
    prevWidth = act->GetProperty()->GetLineWidth();

    act->GetProperty()->SetColor(this->TimeCueLineColor);
    act->GetProperty()->SetLineWidth(this->TimeCueLineWidth);

    this->LineCueMapper->Render(ren, act);

    act->GetProperty()->SetColor(prevColor);
    act->GetProperty()->SetLineWidth(prevWidth);
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
  if (this->DoBallTimeCue)
  {
    double prevColor[3];
    act->GetProperty()->GetColor(prevColor);
    act->GetProperty()->SetColor(this->TimeCueBallColor);
    this->BallCueMapper->RenderPiece(ren, act);
    act->GetProperty()->SetColor(prevColor);
  }
  if (this->DoLineTimeCue)
  {
    double prevColor[3];
    double prevWidth;
    act->GetProperty()->GetColor(prevColor);
    prevWidth = act->GetProperty()->GetLineWidth();

    act->GetProperty()->SetColor(this->TimeCueLineColor);
    act->GetProperty()->SetLineWidth(this->TimeCueLineWidth);

    this->LineCueMapper->RenderPiece(ren, act);

    act->GetProperty()->SetColor(prevColor);
    act->GetProperty()->SetLineWidth(prevWidth);
  }
}

//----------------------------------------------------------------------------
void vtkCurveMapper::ReleaseGraphicsResources(vtkWindow *w)
{
  this->LinesMapper->ReleaseGraphicsResources(w);
  this->PointsMapper->ReleaseGraphicsResources(w);
  this->BallCueMapper->ReleaseGraphicsResources(w);
  this->LineCueMapper->ReleaseGraphicsResources(w);
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
    double m5 = m[1][1] * FFScale[1] * ViewScale;
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

        if (DoCropTimeCue && TimeForTimeCue < pts[0])
          continue;

        FILL_PTS_AND_CELLS
      }
      // add the last point.
      inPts->GetPoint(nPts-1, pts);

      if (!DoCropTimeCue ||
       (DoCropTimeCue && TimeForTimeCue >= pts[0]))
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

          if (DoCropTimeCue && TimeForTimeCue < pts[0])
            continue;

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

// from avtOpenGLCurveRenderer
//----------------------------------------------------------------------------
//  Modifications:
//
//    Kathleen Biagas, Thu Dec 27 08:14:09 PST 2018
//    Don't display the ball if TimeForTimeCue is out of range.
//
//    Kathleen Biagas, Thu Feb 28 08:19:42 PST 2019
//    Per ticket #3266, display the ball even if TimeForTimeCue is out of 
//    range.  Use pt[0] for < and pt[nPts-1] for >.
//
//----------------------------------------------------------------------------

void
vtkCurveMapper::SetUpBallCue(vtkRenderer *ren)
{
  int    bin_x_n,      bin_y_n;
  double bin_x_size,   bin_y_size;
  double bin_x_offset, bin_y_offset;
  bool haveAspect = GetAspect(ren, bin_x_n, bin_x_size, bin_x_offset,
                              bin_y_n, bin_y_size, bin_y_offset);

  if (haveAspect)
  {
    vtkPoints *inPts = this->GetInput()->GetPoints();
    vtkIdType nPts   = inPts->GetNumberOfPoints();
    vtkCellArray *ca = vtkCellArray::New();
    vtkPoints *newPts = this->BallCuePolyData->GetPoints()->NewInstance();

    double ix = this->TimeForTimeCue;
    double iy = 0.;
    if (this->TimeForTimeCue < inPts->GetPoint(0)[0])
        iy = inPts->GetPoint(0)[1];
    else if (this->TimeForTimeCue > inPts->GetPoint(nPts-1)[0])
        iy = inPts->GetPoint(nPts-1)[1];
    else
    {
        for(vtkIdType i = 0; i < nPts-1 ; i++)
        {
          double ptr[6];
          inPts->GetPoint(i, ptr);
          inPts->GetPoint(i+1, ptr+3);
          if (ptr[0] <= this->TimeForTimeCue && this->TimeForTimeCue <= ptr[3])
          {
            double lastX = ptr[0];
            double curX = ptr[3];
            double lastY = ptr[1];
            double curY = ptr[4];
            iy = (ix-lastX)/(curX-lastX)*(curY-lastY) + lastY;
            break;
          }
        }
    }

    std::vector<vtkIdType> ids;
    double pt[3];
    pt[0] = ix;
    pt[1] = iy;
    pt[2] = 0.;
    vtkIdType id = newPts->InsertNextPoint(pt);
    ids.push_back(id);
    for(int i = 0; i < BALL_CUE_VERTS-1; ++i)
    {
      double t = double(i) / double(BALL_CUE_VERTS-1-1);
      double angle = 2. * M_PI * t;
      pt[0] = ix + cos(angle) * this->TimeCueBallSize * bin_x_size / 2.;
      pt[1] = iy + sin(angle) * this->TimeCueBallSize * bin_y_size / 2.;
      pt[2] = 0.;

      vtkIdType id = newPts->InsertNextPoint(pt);
      ids.push_back(id);
    }

    for (size_t k = 1; k < ids.size()-1; ++k)
    {
      ca->InsertNextCell(3);
      ca->InsertCellPoint(ids[0]);
      ca->InsertCellPoint(ids[k]);
      ca->InsertCellPoint(ids[k+1]);
    }
    ca->InsertNextCell(3);
    ca->InsertCellPoint(ids[0]);
    ca->InsertCellPoint(ids[ids.size()-1]);
    ca->InsertCellPoint(ids[1]);

    this->BallCuePolyData->SetPoints(newPts);
    this->BallCuePolyData->SetPolys(ca);
  }
}

// from avtOpenGLCurveRenderer
//----------------------------------------------------------------------------
void
vtkCurveMapper::SetUpLineCue(vtkRenderer *ren)
{
  vtkPolyData *input = this->GetInput();
  double max = -1e+30;
  double min = +1e+30;
  vtkIdType nPts = input->GetPoints()->GetNumberOfPoints();
  for(vtkIdType i = 0; i < nPts ; i++)
  {
      double pt[3];
      input->GetPoints()->GetPoint(i, pt);
      max = (max > pt[1] ? max : pt[1]);
      min = (min < pt[1] ? min : pt[1]);
  }
  double diff = max-min;
  max += diff;
  min -= diff;
  double pt1[3] = { this->TimeForTimeCue, min, 0. };
  double pt2[3] = { this->TimeForTimeCue, max, 0. };

  LineCueData->SetPoint1(pt1);
  LineCueData->SetPoint2(pt2);
}

//----------------------------------------------------------------------------
//  Modifications:
//    Kathleen Biagas, Thu Dec 27 08:40:48 PST 2018
//    Update cell count for polyLines, may be less than allocated due to crop.
//    Compare TimeForTimeCue with point(i)'s x value, not i.
//
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
    int nCellPtsAdded = 0;
    for (int i = 0; i < input->GetNumberOfPoints(); ++i)
    {
        if (this->DoCropTimeCue &&
            this->TimeForTimeCue < input->GetPoints()->GetPoint(i)[0])
        {
            break;
        }
        polyLines->InsertCellPoint(i);
        ++nCellPtsAdded;
    }
    polyLines->UpdateCellCount(nCellPtsAdded);

    this->LinesPolyData->SetLines(polyLines);
    polyLines->Delete();
    this->LinesPolyData->GetPointData()->PassData(input->GetPointData());
    // We have changed a bunch of line segment cells to a single polyline cell,
    // so we shouldn't pass the cell data along???
    if (DoCropTimeCue)
        this->LinesPolyData->GetCellData()->PassData(input->GetCellData());
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

//----------------------------------------------------------------------------
void vtkCurveMapper::UpdateBallCueData(vtkRenderer *ren)
{
  vtkPolyData *input = this->GetInput();
  if (!this->BallCueDataInitialized ||
      ( (input->GetMTime() > this->BallCuePolyData->GetMTime() ||
         this->GetMTime()  > this->BallCuePolyData->GetMTime()) ) ||
      ( ren->GetActiveCamera()->GetMTime() > this->BallCuePolyData->GetMTime()))
  {
    this->BallCuePolyData->Initialize();

    vtkPoints *newPts = input->GetPoints()->NewInstance();
    newPts->Allocate(BALL_CUE_VERTS);
    this->BallCuePolyData->GetCellData()->Allocate(BALL_CUE_VERTS);

    this->BallCuePolyData->SetPoints(newPts);
    newPts->Delete();

    this->SetUpBallCue(ren);
    this->BallCueDataInitialized = true;
    this->BallCueMapper->Update();
  }
}

//----------------------------------------------------------------------------
void vtkCurveMapper::UpdateLineCueData(vtkRenderer *ren)
{
  vtkPolyData *input = this->GetInput();
  if (!this->LineCueDataInitialized ||
      ( (input->GetMTime() > this->LineCueData->GetMTime() ||
         this->GetMTime()  > this->LineCueData->GetMTime()) ) ||
      ( ren->GetActiveCamera()->GetMTime() > this->LineCueData->GetMTime()))
  {
    this->SetUpLineCue(ren);
    this->LineCueDataInitialized = true;
    this->LineCueMapper->Update();
  }
}

