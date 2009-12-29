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
//                          avtOpenGLStreamlineRenderer.C                      //
// ************************************************************************* //

#include "avtOpenGLStreamlineRenderer.h"
#include <avtStreamlinePolyDataFilter.h>
#include <InvalidColortableException.h>
#include <LightList.h>
#include <StreamlineAttributes.h>
#include <avtCallback.h>
#include <avtColorTables.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkTubeFilter.h>
#include <vtkRibbonFilter.h>
#include <vtkPolyLine.h>
#include <vtkFloatArray.h>
#include <vtkAppendPolyData.h>
#include <ImproperUseException.h>
#include <LineAttributes.h>
#include <avtLookupTable.h>

#ifndef VTK_IMPLEMENT_MESA_CXX
  #if defined(__APPLE__) && (defined(VTK_USE_CARBON) || defined(VTK_USE_COCOA))
    #include <OpenGL/gl.h>
  #else
    #if defined(_WIN32)
       #include <windows.h>
    #endif
    #include <GL/gl.h>
  #endif
#else
  #include <GL/gl.h>
#endif


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int sphereQualityLevels[5][2] = {
    {6,3},
    {12,4},
    {24,12},
    {48,24},
    {96,48}
};


// ****************************************************************************
//  Constructor: avtOpenGLStreamlineRenderer::avtOpenGLStreamlineRenderer
//
//  Purpose:
//    Constructor
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

avtOpenGLStreamlineRenderer::avtOpenGLStreamlineRenderer()
{
    displaylistid = 0;

    colorTableName = "";
    colorTable.resize(0);
    levelsLUT = NULL;

    for (int i = 0; i < MAX_DETAIL_LEVELS; i++)
        spherePts[i] = NULL;
}


// ****************************************************************************
//  Destructor: avtOpenGLStreamlineRenderer::~avtOpenGLStreamlineRenderer
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

avtOpenGLStreamlineRenderer::~avtOpenGLStreamlineRenderer()
{
    if (displaylistid != 0)
    {
        glDeleteLists(displaylistid, 1);
        displaylistid = 0;
    }

    for (int i = 0; i < MAX_DETAIL_LEVELS; i++)
        if (spherePts[i])
        {
            delete [] spherePts[i];
            spherePts[i] = NULL;
        }
}



// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::InvalidateColors
//
//  Purpose:
//    
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// ****************************************************************************
void
avtOpenGLStreamlineRenderer::InvalidateColors()
{
    colorTableName = "";
    colorTable.resize(0);

    if (displaylistid != 0)
    {
        glDeleteLists(displaylistid, 1);
        displaylistid = 0;
    }
}

// ****************************************************************************
// Method: avtOpenGLStreamlineRenderer::SetLevelsLUT
//
// Purpose: 
//   Sets a lookup table to be used for resseq coloring.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::SetLevelsLUT(avtLookupTable *lut)
{
    levelsLUT = lut;
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::Render
//
//  Purpose:
//    Render one image
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::Render(vtkPolyData *data,
                                    const StreamlineAttributes &a,
                                    bool immediateModeRendering,
                                    double vMin, double vMax,
                                    float _ambient_coeff,
                                    float _spec_coeff, float _spec_power,
                                    float _spec_r, float _spec_g, float _spec_b,
                                    const int *winsize)
{
    // If nothing changed, and we have display lists, just replay it.
    if (immediateModeRendering &&
        displaylistid != 0 &&
        atts == a &&
        ambient_coeff == _ambient_coeff &&
        spec_coeff == _spec_coeff &&
        spec_power == _spec_power &&
        spec_r == _spec_r &&
        spec_g == _spec_g &&
        spec_b == _spec_b)
    {
        glCallList(displaylistid);
        return;
    }

    //Otherwise, we need to regenerate.
    atts = a;

    //Make a new display list, if needed.
    if (immediateModeRendering)
    {
        if (displaylistid != 0)
            glDeleteLists(displaylistid, 1);
        
        displaylistid = glGenLists(1);
        glNewList(displaylistid, GL_COMPILE);
    }

    varMin = vMin;
    varMax = vMax;
    varDiff = varMax-varMin;

    ambient_coeff = _ambient_coeff;
    spec_coeff    = _spec_coeff;
    spec_power    = _spec_power;
    spec_r        = _spec_r;
    spec_g        = _spec_g;
    spec_b        = _spec_b;

    glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT);
    if (!atts.GetLightingFlag())
        ambient_coeff = 1.0;

    float diff[] = {1,1,1,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
    if (ambient_coeff == 0)
    {
        float amb[] = {ambient_coeff, ambient_coeff, ambient_coeff, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    }
    else
    {
        float amb[] = {ambient_coeff, ambient_coeff, ambient_coeff, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }
    glEnable(GL_COLOR_MATERIAL);
    float spec[] = {spec_r * spec_coeff,
                    spec_g * spec_coeff,
                    spec_b * spec_coeff,
                    1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &spec_power);
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    TRY
    {
        InitColors();
        DrawStreamlines(data);
    }
    CATCH2(VisItException, e)
    {
        avtCallback::IssueWarning(e.Message().c_str());
    }
    ENDTRY

    glPopAttrib();

    if (immediateModeRendering)
    {
        glEndList();
        glCallList(displaylistid);
    }
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawStreamline
//
//  Purpose:
//    Call the appropriate rendering method, then display seeds if needed.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawStreamlines(vtkPolyData *data)
{
    if (atts.GetDisplayMethod() == StreamlineAttributes::Lines)
        DrawAsLines(data);
    else if (atts.GetDisplayMethod() == StreamlineAttributes::Tubes)
        DrawAsTubes(data);
    else if (atts.GetDisplayMethod() == StreamlineAttributes::Ribbons)
        DrawAsRibbons(data);

    if (atts.GetShowStart())
        DrawSeedPoints(data);
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawAsLines
//
//  Purpose:
//    Display streamlines as lines.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawAsLines(vtkPolyData *data)
{
    //Turn off lighting for lines.
    glDisable(GL_LIGHTING);

    glLineWidth(Int2LineWidth(atts.GetLineWidth()));
    
    vtkPoints *points = data->GetPoints();
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();
    float *scalar = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *opacity = NULL;
    if (data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str()))
        opacity = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str())->GetVoidPointer(0);

    int *segptr = segments;
    double pt[3];

    for (int i=0; i<data->GetNumberOfLines(); i++)
    {
        int nPts = *segptr;
        int idx0 = 0, idx1 = nPts;

        segptr++; //Now segptr points at vtx0.
        
        double t0=0.0, t1=0.0;
        GetEndPoints(data, segptr, nPts, idx0, idx1, t0, t1);

        //cout<<"   Draw: "<<idx0<<" to "<<idx1<<" ["<<t0<<" "<<t1<<"]"<<endl;

        float o = 1.0;
        glBegin(GL_LINE_STRIP);

        //If we have an interpolated start point, calculate it.
        if (idx0 > 0)
        {
            double prev[3];
            points->GetPoint(segptr[idx0-1], prev);
            points->GetPoint(segptr[idx0], pt);
            
            float p[3];
            p[0] = prev[0] + t0*(pt[0]-prev[0]);
            p[1] = prev[1] + t0*(pt[1]-prev[1]);
            p[2] = prev[2] + t0*(pt[2]-prev[2]);
            
            float  s0, s1, s, o;
            s0 = scalar[segptr[idx0-1]];
            s1 = scalar[segptr[idx0]];
            s = s0 + t0*(s1-s0);
            if (opacity)
            {
                s0 = scalar[segptr[idx0-1]];
                s1 = scalar[segptr[idx0]];
                o = s0 + t0*(s1-s0);
            }
            o = s;
            SetColor(s, o);

            glVertex3fv(p);
        }
        
        for (int j = idx0; j < idx1; j++)
        {
            points->GetPoint(segptr[j], pt);
            float p[3] = {pt[0], pt[1], pt[2]};
            
            float s = scalar[segptr[j]];
            if (opacity)
            {
                o = opacity[segptr[j]];
            }
            SetColor(s, o);
            glVertex3fv(p);
        }

        //If we have an interpolated end point, calculate it.
        if (idx1 < nPts)
        {
            double next[3];
            points->GetPoint(segptr[idx1], pt);
            points->GetPoint(segptr[idx1+1], next);
            
            float p[3];
            p[0] = pt[0] + t1*(next[0]-pt[0]);
            p[1] = pt[1] + t1*(next[1]-pt[1]);
            p[2] = pt[2] + t1*(next[2]-pt[2]);

            float  s0, s1, s, o;
            s0 = scalar[segptr[idx1]];
            s1 = scalar[segptr[idx1+1]];
            s = s0 + t1*(s1-s0);
            if (opacity)
            {
                s0 = scalar[segptr[idx1]];
                s1 = scalar[segptr[idx1+1]];
                o = s0 + t1*(s1-s0);
            }
            
            SetColor(s, o);
            
            glVertex3fv(p);
        }

        glEnd();

        segptr += nPts;
    }

    glEnable(GL_LIGHTING);
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawAsTubes
//
//  Purpose:
//    Display each streamline as a tube. Run a vtkTubeFilter on each streamline,
//    then render the polygons.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawAsTubes(vtkPolyData *data)
{
    vtkTubeFilter *tube = vtkTubeFilter::New();
    tube->SetRadius(atts.GetRadius());

    tube->SetNumberOfSides(atts.GetTubeDisplayDensity());
    tube->SetRadiusFactor(2.);
    tube->SetCapping(1);
    tube->ReleaseDataFlagOn();

    //Easy case, make tubes and we're done.
    if (!atts.GetDisplayBegin() && !atts.GetDisplayEnd())
        tube->SetInput(data);
    else
    {
        // If we need to trim either end, create a new trimmed polyline
        // and run the tube on this geometry.
        
        vtkPoints *points = data->GetPoints();
        vtkCellArray *lines = data->GetLines();
        vtkIdType *segments = lines->GetPointer();
    
        int *segptr = segments;
        vtkAppendPolyData *append = vtkAppendPolyData::New();
        
        for (int i=0; i<data->GetNumberOfLines(); i++)
        {
            vtkPolyData *pd = MakeNewPolyline(data, segptr);

            append->AddInput(pd);
            pd->Delete();
        }
        
        append->Update();
        tube->SetInput(append->GetOutput());
        append->Delete();
    }
    
    //Create the tube polydata, and draw.
    tube->Update();
    DrawPolyData(tube->GetOutput());
    tube->Delete();
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawAsRibbons
//
//  Purpose:
//    Display each streamline as a ribbon. Run a vtkRibbonFilter on each streamline,
//    then render the polygons.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawAsRibbons(vtkPolyData *data)
{
    vtkPoints *points = data->GetPoints();
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();
    float *t = NULL;
    if (data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::thetaArrayName.c_str()))
        t = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::thetaArrayName.c_str())->GetVoidPointer(0);
    else
        EXCEPTION1(ImproperUseException, "Expected a vorticity values for ribbon display.");
    
    int *segptr = segments;

    //Create new polylines and add the normals.
    for (int i=0; i<data->GetNumberOfLines(); i++)
    {
        vtkPolyData *pd = MakeNewPolyline(data, segptr);
        int nPts = pd->GetPointData()->GetNumberOfTuples();

        vtkIdList *ids = vtkIdList::New();
        vtkPoints *pts = vtkPoints::New();
        vtkCellArray *lines = vtkCellArray::New();
        for (int i = 0; i < nPts; i++)
        {
            vtkIdType id = pts->InsertNextPoint(pd->GetPoints()->GetPoint(i));
            ids->InsertNextId(id);
        }

        lines->InsertNextCell(ids);
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
            double theta = t[i];
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
            
            normals->GetTuple(i, normal);
            vtkMath::Normalize(tan);
            vtkMath::Normalize(normal);
            
            vtkMath::Cross(normal, tan, biNormal);
            double cosTheta = cos(theta);
            double sinTheta = sin(theta);
            for (int j = 0; j < 3; j++)
                normal[j] = cosTheta*normal[j] + sinTheta*biNormal[j];
            
            normals->SetTuple(i,normal);
        }
        
        ids->Delete();
        pts->Delete();
        lines->Delete();
        
        pd->GetPointData()->SetNormals(normals);
        normals->Delete();
        lineNormalGenerator->Delete();
        
        vtkRibbonFilter *ribbons = vtkRibbonFilter::New();
        ribbons->SetWidth(atts.GetRadius());
        ribbons->SetInput(pd);
        ribbons->Update();
        
        DrawPolyData(ribbons->GetOutput());
        pd->Delete();
        ribbons->Delete();
    }
}


// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawSeedPoints
//
//  Purpose:
//    Draw each seed point as a sphere.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawSeedPoints(vtkPolyData *data)
{
    CalculateSpherePts();
    double rad = atts.GetSeedDisplayRadius();
    int quality = atts.GetSeedDisplayDensity()-1;

    vtkPoints *points = data->GetPoints();
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();
    float *s = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *o = NULL;
    if (data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str()))
        o = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str())->GetVoidPointer(0);
    
    int *segptr = segments;
    double pt[3];
    
    for (int i=0; i<data->GetNumberOfLines(); i++)
    {
        int nPts = *segptr;
        segptr++; //Now segptr points at vtx0.

        points->GetPoint(segptr[0], pt);
        
        glBegin(GL_QUADS);
        SetColor(s[*segptr], (o?o[*segptr]:1.0));
        DrawSphereAsQuads(pt[0],pt[1], pt[2], rad, quality);
        glEnd();
        segptr += nPts;
    }
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::MakeNewPolyline
//
//  Purpose:
//    Helper function that creates a new polyline, trimmed at both ends as needed
//    and all arrays attached.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

vtkPolyData *
avtOpenGLStreamlineRenderer::MakeNewPolyline(vtkPolyData *data,
                                             int *&segptr)
{
    vtkPoints *points = data->GetPoints();
    float *s = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *t = NULL, *o = NULL;
    if (data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::thetaArrayName.c_str()))
        t = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::thetaArrayName.c_str())->GetVoidPointer(0);
    if (data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str()))
        o = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str())->GetVoidPointer(0);
    
    int nPts = *segptr;
    
    vtkPoints *pts = vtkPoints::New();
    vtkCellArray *cells = vtkCellArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetName(avtStreamlinePolyDataFilter::colorvarArrayName.c_str());

    vtkFloatArray *thetas = NULL;
    if (t)
    {
        thetas = vtkFloatArray::New();
        thetas->SetName(avtStreamlinePolyDataFilter::thetaArrayName.c_str());
    }
    vtkFloatArray *opacity = NULL;
    if (o)
    {
        opacity = vtkFloatArray::New();
        opacity->SetName(avtStreamlinePolyDataFilter::opacityArrayName.c_str());
    }
            
    segptr++; //Now segptr points at vtx0.

    double t0, t1, pt[3];
    int idx0 = 0, idx1 = nPts;
    GetEndPoints(data, segptr, nPts, idx0, idx1, t0, t1);


    int nNewPts = idx1-idx0;
    if (idx0 != 0)
        nNewPts++;
    if (idx1 != nPts)
        nNewPts++;
    //cout<<"   Draw: "<<idx0<<" to "<<idx1<<" ["<<t0<<" "<<t1<<"]"<<" pts= "<<nNewPts<<endl;

    pts->Allocate(nNewPts);
    scalars->Allocate(nNewPts);
    cells->InsertNextCell(nNewPts);


    int idx = 0;
    //If we have an interpolated start point, calculate it.
    if (idx0 > 0)
    {
        double prev[3];
        points->GetPoint(segptr[idx0-1], prev);
        points->GetPoint(segptr[idx0], pt);
        
        double p[3];
        p[0] = prev[0] + t0*(pt[0]-prev[0]);
        p[1] = prev[1] + t0*(pt[1]-prev[1]);
        p[2] = prev[2] + t0*(pt[2]-prev[2]);
        //cout<<"0"<<" "<<idx<<": "<<pt[0]<<" "<<pt[1]<<" "<<pt[2]<<endl;
        
        pts->InsertPoint(idx, p[0], p[1], p[2]);
        cells->InsertCellPoint(idx);

        double v0, v1, v;
        v0 = s[segptr[idx0-1]];
        v1 = s[segptr[idx0]];
        v = v0 + t0*(v1-v0);
        scalars->InsertTuple1(idx, v);

        if (t)
        {
            v0 = t[segptr[idx0-1]];
            v1 = t[segptr[idx0]];
            v = v0 + t0*(v1-v0);
            thetas->InsertTuple1(idx, v);
        }
        if (o)
        {
            v0 = o[segptr[idx0-1]];
            v1 = o[segptr[idx0]];
            v = v0 + t0*(v1-v0);
            opacity->InsertTuple1(idx, v);
        }
        
        idx++;
    }
    
    //Add all the interior points.
    for (int i = idx0; i < idx1; i++, idx++)
    {
        points->GetPoint(segptr[i], pt);
        
        pts->InsertPoint(idx, pt[0], pt[1], pt[2]);
        //cout<<i<<" "<<idx<<": "<<pt[0]<<" "<<pt[1]<<" "<<pt[2]<<endl;
        cells->InsertCellPoint(idx);
        
        scalars->InsertTuple1(idx, s[segptr[i]]);
        if (t)
            thetas->InsertTuple1(idx, t[segptr[i]]);
        if (o)
            opacity->InsertTuple1(idx, o[segptr[i]]);
    }

    //If we have an interpolated end point, calculate it.
    if (idx1 < nPts)
    {
        double next[3];
        points->GetPoint(segptr[idx1], pt);
        points->GetPoint(segptr[idx1+1], next);
        
        double p[3];
        p[0] = pt[0] + t1*(next[0]-pt[0]);
        p[1] = pt[1] + t1*(next[1]-pt[1]);
        p[2] = pt[2] + t1*(next[2]-pt[2]);
        //cout<<"N"<<" "<<idx<<": "<<pt[0]<<" "<<pt[1]<<" "<<pt[2]<<endl;
        
        pts->InsertPoint(idx, p[0], p[1], p[2]);
        cells->InsertCellPoint(idx);
        
        double v0, v1, v;
        v0 = s[segptr[idx1]];
        v1 = s[segptr[idx1+1]];
        v = v0 + t1*(v1-v0);
        scalars->InsertTuple1(idx, v);

        if (t)
        {
            v0 = t[segptr[idx1]];
            v1 = t[segptr[idx1+1]];
            v = v0 + t1*(v1-v0);
            thetas->InsertTuple1(idx, v);
        }
        if (o)
        {
            v0 = o[segptr[idx1]];
            v1 = o[segptr[idx1+1]];
            v = v0 + t1*(v1-v0);
            opacity->InsertTuple1(idx, v);
        }

        idx++;
    }
    
    segptr += nPts;

    vtkPolyData *pd = vtkPolyData::New();
    pd->SetPoints(pts);
    pd->SetLines(cells);
    pd->GetPointData()->AddArray(scalars);
    if (thetas)
    {
        pd->GetPointData()->AddArray(thetas);
        thetas->Delete();
    }
    if (opacity)
    {
        pd->GetPointData()->AddArray(opacity);
        opacity->Delete();
    }
    pts->Delete();
    scalars->Delete();

    return pd;
}


// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawPolyData
//
//  Purpose:
//    Draw a vtkPolyData.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawPolyData(vtkPolyData *poly)
{
    float *p = (float *)poly->GetPoints()->GetVoidPointer(0);
    float *n = (float *)poly->GetPointData()->GetNormals()->GetVoidPointer(0);
    float *s = (float *)poly->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *o = NULL;
    if (poly->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str()))
        o = (float *)poly->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str())->GetVoidPointer(0);
    
    vtkCellArray *strips = poly->GetStrips();
    vtkIdType *ptIds = strips->GetPointer();
    vtkIdType *endPtIds = ptIds + strips->GetNumberOfConnectivityEntries();
    
    while (ptIds < endPtIds)
    {
        int nPts = *ptIds;
        ++ptIds;
        glBegin(GL_TRIANGLE_STRIP);
        while (nPts > 0)
        {
            SetColor(s[*ptIds], (o?o[*ptIds]:1.0));
            glNormal3fv(n + 3*(*ptIds));
            glVertex3fv(p + 3*(*ptIds));
            
            ++ptIds;
            --nPts;
        }
        glEnd();
    }
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::SetColor
//
//  Purpose:
//    Set color and opacity based on a scalar and opacity.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::SetColor(const float &scalar,
                                      const float &opacity) const
{
    // If solid and no variable opacity, coloring already set.
    if (atts.GetColoringMethod() == StreamlineAttributes::Solid &&
        atts.GetOpacityType() != StreamlineAttributes::VariableRange)
    {
        return;
    }

    unsigned char rgba[4];

    //Set color.
    if (atts.GetColoringMethod() == StreamlineAttributes::Solid)
        memcpy(rgba, atts.GetSingleColor().GetColor(), 3);
    else
    {
        int index = (int)((scalar-varMin)/varDiff * 255.0f);
        if (index < 0)
            index = 0;
        if ( index > 255)
            index = 255;

        memcpy(rgba, &colorTable[4*index], 3);
    }
    
    // Figure out opacity, if needed.
    if (atts.GetOpacityType() != StreamlineAttributes::None)
    {
        float alpha = atts.GetOpacity();
        
        if (atts.GetOpacityType() == StreamlineAttributes::VariableRange)
        {
            if (atts.GetOpacityVarMinFlag() && opacity < atts.GetOpacityVarMin())
                alpha = 0.0;
            if (atts.GetOpacityVarMaxFlag() && opacity > atts.GetOpacityVarMax())
                alpha = 0.0;
        }
        rgba[3] = (unsigned char)(alpha*255.0f);
    }
    else
        rgba[3] = (unsigned char)255;
    
    glColor4ubv(rgba);
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::InitColors
//
//  Purpose:
//    Initialize color tables.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::InitColors()
{
    if (atts.GetColoringMethod() == StreamlineAttributes::Solid)
    {
        unsigned char rgba[4];
        
        memcpy(rgba, atts.GetSingleColor().GetColor(), 4);
        if (atts.GetOpacityType() == StreamlineAttributes::None)
            rgba[3] = (unsigned char)255;
        else if (atts.GetOpacityType() == StreamlineAttributes::Constant)
            rgba[3] = (unsigned char)(atts.GetOpacity()*255.0f);
        
        glColor4ubv(rgba);
        return;
    }
    if (atts.GetColorTableName() == colorTableName)
        return;
    
    colorTableName = atts.GetColorTableName();
    avtColorTables *ct = avtColorTables::Instance();
    
    if (colorTableName == "Default")
        colorTableName = string(ct->GetDefaultContinuousColorTable());

    // Make sure color table exists.
    if (!ct->ColorTableExists(colorTableName.c_str()))
        EXCEPTION1(InvalidColortableException, colorTableName);


    const int numColors = 256, opacity = 255;
    
    colorTable.resize(numColors*4);

    //Fill in the colorTable.
    unsigned char rgb[3] = {0,0,0};
    
    vector<unsigned char>::iterator iter = colorTable.begin();
    if (ct->IsDiscrete(colorTableName.c_str()))
    {
        for (int i = 0; i < numColors; i++)
        {
            ct->GetControlPointColor(colorTableName.c_str(), i, rgb);
            *iter++ = rgb[0];
            *iter++ = rgb[1];
            *iter++ = rgb[2];
            *iter++ = opacity;
        }
    }
    else
    {
        unsigned char *rgb = ct->GetSampledColors(colorTableName.c_str(), numColors);
        if (rgb)
        {
            vector<unsigned char>::iterator iter = colorTable.begin();
            for (int i = 0; i < numColors; i++)
            {
                *iter++ = rgb[i*3 +0];
                *iter++ = rgb[i*3 +1];
                *iter++ = rgb[i*3 +2];
                *iter++ = opacity;
            }
            delete [] rgb;
        }
    }
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::CalculateSpherePts
//
//  Purpose:
//    Precalculate points for sphere geometry.
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

void avtOpenGLStreamlineRenderer::CalculateSpherePts()
{
    if (spherePts[0] != NULL)
        return;

    for (int detail=0; detail<MAX_DETAIL_LEVELS; detail++)
    {
        int qdetail = sphereQualityLevels[detail][0];
        int hdetail = sphereQualityLevels[detail][1];
        spherePts[detail] = new float[(qdetail+1)*(hdetail+1)*3];

        for (int a=0; a<=qdetail; a++)
        {
            float theta = 2*M_PI * float(a) / float(qdetail);
            for (int b=0; b<=hdetail; b++)
            {
                float phi = M_PI * float(b) / float(hdetail);

                float dx = sin(phi) * cos(theta);
                float dy = sin(phi) * sin(theta);
                float dz = cos(phi);

                spherePts[detail][(a * (hdetail+1) + b)*3 + 0] = dx;
                spherePts[detail][(a * (hdetail+1) + b)*3 + 1] = dy;
                spherePts[detail][(a * (hdetail+1) + b)*3 + 2] = dz;
            }
        }
    }
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::GetEndPoints
//
//  Purpose:
//    If displayBegin and/or displayEnd is set, figure out the index of the nearest
//    vertex (j0, j1) and the percentage between the neighboring point (t0, t1).
//
//  Programmer:  Dave Pugmire
//  Creation:    December 29, 2009
//
//  Modifications:
//
// ****************************************************************************

bool
avtOpenGLStreamlineRenderer::GetEndPoints(vtkPolyData *data, 
                                          int *segptr,
                                          int nPts,
                                          int &j0, 
                                          int &j1,
                                          double &t0,
                                          double &t1)
{
    if (! atts.GetDisplayBeginFlag() && !atts.GetDisplayEndFlag())
        return false;
    
    vtkPoints *points = data->GetPoints();
    vtkDataArray *param = data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::paramArrayName.c_str());
    
    t0 = 0.0;
    t1 = 0.0;
    
    bool modifiedStartEnd = false;
    
    //Find the begining
    double beg = atts.GetDisplayBegin();
    if (atts.GetDisplayBeginFlag() && beg > 0.0)
    {
        for (int i = 0; i < nPts; i++)
        {
            double v = param->GetTuple1(segptr[i]);
            if (v >= beg)
            {
                j0 = i;

                if (i > 0)
                {
                    double v0 = param->GetTuple1(segptr[i-1]);
                    t0 = (beg-v0) / (v-v0);
                }
                modifiedStartEnd = true;
                break;
            }
        }
    }
    
    // Find the end.
    double end = atts.GetDisplayEnd();
    if (atts.GetDisplayEndFlag() && end < atts.GetTermination())
    {
        for (int i = nPts-1; i >= 0; i--)
        {
            double v = param->GetTuple1(segptr[i]);
            if (v <= end)
            {
                j1 = i;
                if (i < nPts-1)
                {
                    double v1 = param->GetTuple1(segptr[i+1]);
                    t1 = (end-v) / (v1-v);
                }
                modifiedStartEnd = true;
                break;
            }
        }
    }

    return modifiedStartEnd;
}



// ****************************************************************************
//  Method:  SetColor3ubv
//
//  Purpose:
//    Okay, so it's a little silly right now, but there was some
//    debate about using glMaterialColor(...), so having things call
//    this instead of the GL call directly made trying it easier.
//    It's staying because I might still change my mind.
//
//  Arguments:
//    c          the pointer to the uchar color
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2006
//
// ****************************************************************************
static inline void
SetColor3ubv(const unsigned char *c)
{
    glColor3ubv(c);
}

/*void
avtOpenGLAnimStreamlineRenderer::DrawAtomsAsSpheres(vtkPolyData *data,
                                              const AnimStreamlineAttributes &atts)
{
    vtkPoints *points = data->GetPoints();

    vtkDataArray *primary = data->GetPointData()->GetScalars();
    if (!primary)
    {
        // Let's just assume we don't want to plot the spheres for
        // a cell-centered variable
        return;
    }
    if (primary && !primary->IsA("vtkFloatArray"))
    {
        debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
        return;
    }
    float *scalar = (float*)primary->GetVoidPointer(0);

    glBegin(GL_QUADS);
    for (int i=0; i<data->GetNumberOfPoints(); i++)
    {
        // Determine radius
        float radius = 0.3;

        // Determine color
        {
            float alpha;
            if (varmax == varmin)
                alpha = 0.5;
            else
                alpha = (scalar[i] - varmin) / (varmax - varmin);
            
            int color = int((float(numcolors)-.01) * alpha);
            if (color < 0)
                color = 0;
            if (color > numcolors-1)
                color = numcolors-1;
            SetColor3ubv(&colors[4*color]);
        }

        {
            // Plot spheres
            double *pt = points->GetPoint(i);
            DrawSphereAsQuads(pt[0],
                              pt[1],
                              pt[2],
                              radius,
                              1//atts.GetAtomSphereQuality()
                              );
        }
    }
    glEnd();
}*/


#if 0
void
avtOpenGLStreamlineRenderer::DrawAtomsAsSpheres(vtkPolyData *data,
                                                const StreamlineAttributes &atts)
{
    vtkPoints *points = data->GetPoints();

    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();

    vtkDataArray *primary = data->GetPointData()->GetScalars();
    if (!primary)
    {
        // Let's just assume we don't want to plot the spheres for
        // a cell-centered variable
        return;
    }
    if (primary && !primary->IsA("vtkFloatArray"))
    {
        debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
        return;
    }
    float *scalar = (float*)primary->GetVoidPointer(0);

#if 1
    glBegin(GL_QUADS);
    int *segptr = segments;
    int segctr = 0;
    int nframes = 10;
    int nextball = positionIndex % nframes;
    for (int j=0; j<data->GetNumberOfLines(); j++)
    {
        int nseg = *segptr;
        int step = positionIndex % nseg;

        while (nextball < segctr + nseg)
        {
            int index = *(segptr + 1 + nextball-segctr);

            // Determine radius
            float radius = 0.15;

            // Determine color
            {
                float alpha;
                if (varmax == varmin)
                    alpha = 0.5;
                else
                    alpha = (scalar[index] - varmin) / (varmax - varmin);
            
                int color = int((float(numcolors)-.01) * alpha);
                if (color < 0)
                    color = 0;
                if (color > numcolors-1)
                    color = numcolors-1;
                SetColor3ubv(&colors[4*color]);
            }

            {
                // Plot spheres
                double *pt = points->GetPoint(index);
                DrawSphereAsQuads(pt[0],
                                  pt[1],
                                  pt[2],
                                  radius,
                                  1/*atts.GetAtomSphereQuality()*/);
            }

            nextball += nframes;
        }

        segctr += nseg;
        segptr += (*segptr) + 1;
    }
    glEnd();
#else
    glBegin(GL_QUADS);
    int *segptr = segments;
    for (int j=0; j<data->GetNumberOfLines(); j++)
    {
        int nseg = *segptr;
        int step = positionIndex % nseg;

        int index = *(segptr + 1 + step);

        // Determine radius
        float radius = 0.3;

        // Determine color
        {
            float alpha;
            if (varmax == varmin)
                alpha = 0.5;
            else
                alpha = (scalar[index] - varmin) / (varmax - varmin);
            
            int color = int((float(numcolors)-.01) * alpha);
            if (color < 0)
                color = 0;
            if (color > numcolors-1)
                color = numcolors-1;
            SetColor3ubv(&colors[4*color]);
        }

        {
            // Plot spheres
            double *pt = points->GetPoint(index);
            DrawSphereAsQuads(pt[0],
                              pt[1],
                              pt[2],
                              radius,
                              atts.GetSeedDisplayDensity());
        }

        segptr += (*segptr) + 1;
    }
    glEnd();
#endif

}
#endif

#if 0

void
avtOpenGLStreamlineRenderer::DrawBonds(vtkPolyData *data,
                                       const StreamlineAttributes &atts)
{
    vtkPoints *points = data->GetPoints();
    int numverts = data->GetNumberOfVerts();
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();

    bool primary_is_cell_centered = false;
    vtkDataArray *primary = data->GetPointData()->GetScalars();
    if (!primary)
    {
        primary = data->GetCellData()->GetScalars();
        primary_is_cell_centered = true;
    }
    if (!primary)
    {
        // eh? no variable at all?  that's a logic error....
        EXCEPTION1(ImproperUseException, "Expected a variable of some sort.");
    }
    if (primary && !primary->IsA("vtkFloatArray"))
    {
        debug4 << "avtOpenGLMoleculeRenderer: found a non-float array\n";
        return;
    }
    float *scalar = (float*)primary->GetVoidPointer(0);

    glBegin(GL_LINE_STRIP);

    int *segptr = segments;
    for (int i=0; i<data->GetNumberOfLines(); i++)
    {
        int nseg = *segptr;
        for (int step=0; step<nseg-1; step++)
        {
            int v0 = *(segptr+step+1);
            int v1 = *(segptr+step+2);

            double pt_0[3];
            double pt_1[3];
            points->GetPoint(v0, pt_0);
            points->GetPoint(v1, pt_1);

            double pt_mid[3] = {(pt_0[0]+pt_1[0])/2.,
                                (pt_0[1]+pt_1[1])/2.,
                                (pt_0[2]+pt_1[2])/2.};


            int atom  = v0;
            double *pt_a = pt_0;
            double *pt_b = pt_1;

            float radius = 0.1;

            float scalarval;
            if (primary_is_cell_centered)
                scalarval = scalar[i + numverts];
            else
                scalarval = scalar[atom];

            {
                float alpha;
                if (varmax == varmin)
                    alpha = 0.5;
                else
                    alpha = (scalarval - varmin) / (varmax - varmin);
            
                int color = int((float(numcolors)-.01) * alpha);
                if (color < 0)
                    color = 0;
                if (color > numcolors-1)
                    color = numcolors-1;

                glEnable(GL_BLEND);
                glEnable(GL_ALPHA_TEST);
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(1.0, 1.0, 1.0, 0.025);
                //SetColor3ubv(&colors[4*color]);
            }

            DrawCylinderBetweenTwoPoints(pt_a, pt_b, radius,
                                         1/*atts.GetBondCylinderQuality()*/);
        }

        segptr += (*segptr) + 1;
    }
    glEnd();
}
#endif



// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::Render
//
//  Purpose:
//    Render one image
//
//  Arguments:
//    ds      : the data set to render
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  3, 2006
//
//  Modifications:
//    Brad Whitlock, Mon Mar 27 14:56:06 PST 2006
//    I made sure that we use immediate mode when we need to when we use 
//    imposter rendering,
//
// ****************************************************************************

#if 0
void
avtOpenGLStreamlineRenderer::Render(vtkPolyData *data,
                                    const StreamlineAttributes &atts,
                                 bool immediateModeRendering,
                                 float _varmin, float _varmax,
                                 float _ambient_coeff,
                                 float _spec_coeff, float _spec_power,
                                 float _spec_r, float _spec_g, float _spec_b,
                                 const int *winsize)
{
    if (!data->GetCellData()->GetScalars() &&
        !data->GetPointData()->GetScalars())
        return;

    immediateModeRendering = true;
    positionIndex++;

    if (immediateModeRendering)
    {
        if (displaylistid != 0)
        {
            glDeleteLists(displaylistid, 1);
            displaylistid = 0;
        }
    }
    else
    {
        // Check to see if we need to regenerate lists
        if (displaylistid != 0 &&
            atts_for_displaylist != atts)
        {
            glDeleteLists(displaylistid, 1);
            displaylistid = 0;
        }

        // If the list we have is valid, just call it
        if (displaylistid != 0)
        {
            glCallList(displaylistid);
            return;
        }

        atts_for_displaylist = atts;
        displaylistid = glGenLists(1);
        glNewList(displaylistid, GL_COMPILE);
    }

    varmin = _varmin;
    varmax = _varmax;

    immediatemode = immediateModeRendering;
    ambient_coeff = _ambient_coeff;
    spec_coeff    = _spec_coeff;
    spec_power    = _spec_power;
    spec_r        = _spec_r;
    spec_g        = _spec_g;
    spec_b        = _spec_b;

    glPushAttrib(GL_COLOR_BUFFER_BIT |
                 GL_DEPTH_BUFFER_BIT |
                 GL_ENABLE_BIT);
    glEnable(GL_LIGHTING);

#if 0
    glDisable(GL_COLOR_MATERIAL);
#else
    float diff[] = {1,1,1,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diff);
    ambient_coeff = 1.0;
    if (ambient_coeff == 0)
    {
        float amb[] = {ambient_coeff, ambient_coeff, ambient_coeff, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
    }
    else
    {
        float amb[] = {ambient_coeff, ambient_coeff, ambient_coeff, 1};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }
    glEnable(GL_COLOR_MATERIAL);
#endif
    float spec[] = {spec_r * spec_coeff,
                    spec_g * spec_coeff,
                    spec_b * spec_coeff,
                    1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &spec_power);
    glShadeModel(GL_SMOOTH);

    TRY
    {
        //SetColors(data, atts);
        DrawAtomsAsSpheres(data, atts);
        DrawBonds(data, atts);
    }
    CATCH2(VisItException, e)
    {
        // Note: we should not need to intercept this here, except
        // that it's otherwise uncaught, as of version 1.5.
        // Also note: if we decide to let the exception reach up
        // the call chain, we STILL need at least catch it here,
        // make sure we've cleaned up OpenGL correctly
        // (e.g. popattrib and endlist), and then re-throw.
        avtCallback::IssueWarning(e.Message().c_str());
    }
    ENDTRY

    glPopAttrib();


    if (immediateModeRendering)
    {
    }
    else
    {
        glEndList();
        glCallList(displaylistid);
    }
}
#endif


// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::CalculateCylPts
//
//  Purpose:
//    Precalculate points for cylinder geometry.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//
// ****************************************************************************

#if 0
void avtOpenGLStreamlineRenderer::CalculateCylPts()
{
    if (cylinders_calculated)
        return;

    cylinders_calculated = true;

    for (int detail=0; detail<MAX_DETAIL_LEVELS; detail++)
    {
        int cdetail = cylinder_quality_levels[detail];
        cyl_pts[detail] = new float[(cdetail+1)*4];

        for (int b=0; b<=cdetail; b++)
        {
            float theta = 2*M_PI * float(b) / float(cdetail);

            float dx = cos(theta);
            float dy = sin(theta);
            float dz = 0;

            cyl_pts[detail][b*4+0] = dx;
            cyl_pts[detail][b*4+1] = dy;
            cyl_pts[detail][b*4+2] = dz;
            cyl_pts[detail][b*4+3] = 0;
        }
    }
}
#endif

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawSphereAsQuads
//
//  Purpose:
//    Make the OpenGL calls to draw a sphere with the
//    given center, radius, and detail level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//
// ****************************************************************************
void
avtOpenGLStreamlineRenderer::DrawSphereAsQuads(float x0,
                                             float y0,
                                             float z0,
                                             float r,
                                             int detail)
{
    CalculateSpherePts();

    int qdetail = sphereQualityLevels[detail][0];
    int hdetail = sphereQualityLevels[detail][1];

    for (int a=0; a<qdetail; a++)
    {
        int a0 = a;
        int a1 = a+1;
        for (int b=0; b<hdetail; b++)
        {
            int b0 = b;
            int b1 = b+1;

            float *v00, *v01, *v10, *v11;

            v00 = &(spherePts[detail][(a0 * (hdetail+1) + b0)*3]);
            v01 = &(spherePts[detail][(a0 * (hdetail+1) + b1)*3]);
            v10 = &(spherePts[detail][(a1 * (hdetail+1) + b0)*3]);
            v11 = &(spherePts[detail][(a1 * (hdetail+1) + b1)*3]);

            glNormal3fv(v00);
            glVertex3f(x0 + r*v00[0], y0 + r*v00[1], z0 + r*v00[2]);

            glNormal3fv(v01);
            glVertex3f(x0 + r*v01[0], y0 + r*v01[1], z0 + r*v01[2]);

            glNormal3fv(v11);
            glVertex3f(x0 + r*v11[0], y0 + r*v11[1], z0 + r*v11[2]);

            glNormal3fv(v10);
            glVertex3f(x0 + r*v10[0], y0 + r*v10[1], z0 + r*v10[2]);
        }
    }
}

// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawCylinderBetweenTwoPoints
//
//  Purpose:
//    Make the OpenGL calls to draw a cylinder with the given begin
//    and end points, radius, and detail level.
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 10, 2006
//
//  Modifications:
//    Jeremy Meredith, Mon Aug 28 18:25:02 EDT 2006
//    Point locations are now doubles, not floats.
//
// ****************************************************************************

#if 0
void
avtOpenGLStreamlineRenderer::DrawCylinderBetweenTwoPoints(double *p0,
                                                        double *p1,
                                                        float r,
                                                        int detail)
{
    CalculateCylPts();

    float vc[3] = {p1[0]-p0[0], p1[1]-p0[1], p1[2]-p0[2]};
    float va[3];
    float vb[3];

    float vc_len = vtkMath::Normalize(vc);
    if (vc_len == 0)
        return;

    vtkMath::Perpendiculars(vc, va,vb, 0);

    float v0[4];
    float v1[4];
    int cdetail = cylinder_quality_levels[detail];
    for (int b=0; b<cdetail; b++)
    {
        int b0 = b;
        int b1 = b+1;

        float *u0, *u1;
        u0 = &(cyl_pts[detail][b0*4]);
        u1 = &(cyl_pts[detail][b1*4]);

        v0[0] = va[0]*u0[0] + vb[0]*u0[1];
        v0[1] = va[1]*u0[0] + vb[1]*u0[1];
        v0[2] = va[2]*u0[0] + vb[2]*u0[1];

        v1[0] = va[0]*u1[0] + vb[0]*u1[1];
        v1[1] = va[1]*u1[0] + vb[1]*u1[1];
        v1[2] = va[2]*u1[0] + vb[2]*u1[1];

        glNormal3fv(v0);
        glVertex3f(p1[0] + r*v0[0], p1[1] + r*v0[1], p1[2] + r*v0[2]);

        glNormal3fv(v0);
        glVertex3f(p0[0] + r*v0[0], p0[1] + r*v0[1], p0[2] + r*v0[2]);

        glNormal3fv(v1);
        glVertex3f(p0[0] + r*v1[0], p0[1] + r*v1[1], p0[2] + r*v1[2]);

        glNormal3fv(v1);
        glVertex3f(p1[0] + r*v1[0], p1[1] + r*v1[1], p1[2] + r*v1[2]);
    }
}
#endif
