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
#include <vtkDepthSortPolyData.h>
#include <vtkPolyLine.h>
#include <vtkFloatArray.h>
#include <vtkAppendPolyData.h>
#include <ImproperUseException.h>
#include <LineAttributes.h>
#include <avtLookupTable.h>
#include <avtGLEWInitializer.h>
#include <avtGLSLProgram.h>
#include <vtkCamera.h>
#include <vtkStripper.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int sphereQualityLevels[5][2] = {
    {12,4},
    {24,12},
    {48,24},
    {96,48}
};

static const char *GLSL_illuminated_lines_vertex_program_source = 
"varying vec3 L, T, V;"
"void main(void)"
"{"
"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"    L = vec3(gl_LightSource[0].position) - vec3(gl_ModelViewMatrix * gl_Vertex);"
"    T = normalize( gl_NormalMatrix * gl_Normal );"
"    V = vec3(gl_ModelViewMatrix * gl_Vertex);"
"    gl_FrontColor = gl_Color;"
"}";

static const char *GLSL_illuminated_lines_fragment_program_source = 
"varying vec3 L, T, V;"
"void main(void)"
"{"
"    float LT =  dot( normalize(L), normalize(T) );"
"    float VT = -dot( normalize(V), normalize(T) );"
""
"    float kd = sqrt( 1. - LT*LT );"
"    float ks = pow( max( 0., kd * sqrt(1. - VT*VT) ), gl_FrontMaterial.shininess );"
""
"    gl_FragColor = gl_FrontMaterial.ambient  * gl_LightSource[0].ambient +" 
"                   gl_Color                  * gl_LightSource[0].diffuse * kd +" 
"                   gl_FrontMaterial.specular * gl_LightSource[0].specular * ks;"
"    gl_FragColor.a = gl_Color.a;"
"}";


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
//   Dave Pugmire (for Christoph Garth), Wed Jan 20 09:28:59 EST 2010
//   Add illuminated lighting model for lines.
//
// ****************************************************************************

avtOpenGLStreamlineRenderer::avtOpenGLStreamlineRenderer()
{
    displaylistid = 0;

    colorTableName = "";
    colorTable.resize(0);
    levelsLUT = NULL;
    appendForTranspPolys = NULL;
    
    for (int i = 0; i < MAX_DETAIL_LEVELS; i++)
        spherePts[i] = NULL;
        
    shader = new avtGLSLProgram("Illuminated Lines");
    shader->AttachShaderFromString(GL_VERTEX_SHADER,
                                   GLSL_illuminated_lines_vertex_program_source);
    shader->AttachShaderFromString(GL_FRAGMENT_SHADER,
                                   GLSL_illuminated_lines_fragment_program_source);
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
        
    delete shader;
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
//    Christoph Garth, Tue Jan 20 10:17:39 PST 2010 
//    Removed the display list generation since Mesa 7.5 does not support
//    shader calls inside display lists.
//
//  Dave Pugmire, Fri Feb 12 14:02:57 EST 2010
//  Pass in camera to do transparency sorting.
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::Render(vtkPolyData *data,
                                    const StreamlineAttributes &a,
                                    bool immediateModeRendering,
                                    double vMin, double vMax,
                                    vtkCamera *cam,
                                    float _ambient_coeff,
                                    float _spec_coeff, float _spec_power,
                                    float _spec_r, float _spec_g, float _spec_b,
                                    const int *winsize)
{
    camera = cam;
#if 0
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
#endif

    //Otherwise, we need to regenerate.
    atts = a;

#if 0
    //Make a new display list, if needed.
    if (immediateModeRendering)
    {
        if (displaylistid != 0)
            glDeleteLists(displaylistid, 1);
        
        displaylistid = glGenLists(1);
        glNewList(displaylistid, GL_COMPILE);
    }
#endif

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

#if 0
    if (immediateModeRendering)
    {
        glEndList();
        glCallList(displaylistid);
    }
#endif
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
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Add drawHead geom.
//
//  Dave Pugmire, Fri Feb 12 14:02:57 EST 2010
//  Support for transparency sorting.
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawStreamlines(vtkPolyData *data)
{
    if (atts.GetOpacityType() != StreamlineAttributes::None)
        appendForTranspPolys = vtkAppendPolyData::New();
    
    if (atts.GetShowSeeds())
        DrawSeedPoints(data);
    if (atts.GetShowHeads())
        DrawHeadGeom(data);
    if (atts.GetDisplayMethod() == StreamlineAttributes::Lines)
        DrawAsLines(data);
    else if (atts.GetDisplayMethod() == StreamlineAttributes::Tubes)
        DrawAsTubes(data);
    else if (atts.GetDisplayMethod() == StreamlineAttributes::Ribbons)
        DrawAsRibbons(data);

    if (appendForTranspPolys)
    {
        appendForTranspPolys->Update();
        DrawPolyData(appendForTranspPolys->GetOutput());
        appendForTranspPolys->Delete();
        appendForTranspPolys = NULL;
    }
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
//
//   Dave Pugmire (for Christoph Garth), Wed Jan 20 09:28:59 EST 2010
//   Illuminated lighting model for lines..
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawAsLines(vtkPolyData *data)
{
    bool illuminated = atts.GetLightingFlag() && shader->Enable();

    //Turn off lighting for lines.
    glDisable(GL_LIGHTING);
    glLineWidth(Int2LineWidth(atts.GetLineWidth()));
    
    vtkPoints *points = data->GetPoints();
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();
    float *scalar = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *param = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::paramArrayName.c_str())->GetVoidPointer(0);
    float *opacity = NULL;
    if (data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str()))
        opacity = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str())->GetVoidPointer(0);
    float *tangents = NULL;
    if (illuminated && data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::tangentsArrayName.c_str()))
        tangents = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::tangentsArrayName.c_str())->GetVoidPointer(0);

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

        float o = 1.0, v[3];
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
            
            if (atts.GetOpacityType() == StreamlineAttributes::Ramp)
                o = 0.0;
            else if (opacity)
            {
                s0 = scalar[segptr[idx0-1]];
                s1 = scalar[segptr[idx0]];
                o = s0 + t0*(s1-s0);
            }
            SetColor(s, o);

            if (tangents)
            {
                float* v0 = tangents + segptr[idx0-1];
                float* v1 = tangents + segptr[idx0];

                v[0] = v0[0] + t0*(v1[0]-v0[0]);
                v[1] = v0[1] + t0*(v1[1]-v0[1]);
                v[2] = v0[2] + t0*(v1[2]-v0[2]);

                glNormal3fv(v);
            }

            glVertex3fv(p);
        }
        
        for (int j = idx0; j < idx1; j++)
        {
            points->GetPoint(segptr[j], pt);
            float p[3] = {pt[0], pt[1], pt[2]};
            
            float s = scalar[segptr[j]];
            if (atts.GetOpacityType() == StreamlineAttributes::Ramp)
            {
                o = ComputeRampOpacity(param[segptr[j]]);
            }
            else if (opacity)
            {
                o = opacity[segptr[j]];
            }
            SetColor(s, o);
            if (tangents)
                glNormal3fv(tangents+3*segptr[j]);
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
            
            if (atts.GetOpacityType() == StreamlineAttributes::Ramp)
                o = 1.0;
            else if (opacity)
            {
                s0 = scalar[segptr[idx1]];
                s1 = scalar[segptr[idx1+1]];
                o = s0 + t1*(s1-s0);
            }
            
            SetColor(s, o);
            
            if (tangents)
            {
                float* v0 = tangents + segptr[idx1];
                float* v1 = tangents + segptr[idx1+1];
            
                v[0] = v0[0] + t1*(v1[0]-v0[0]);
                v[1] = v0[1] + t1*(v1[1]-v0[1]);
                v[2] = v0[2] + t1*(v1[2]-v0[2]);
                
                glNormal3fv(v);
            }
            
            glVertex3fv(p);
        }

        glEnd();

        segptr += nPts;
    }

    glEnable(GL_LIGHTING);

    if (illuminated)
        shader->Disable();
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
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Changed some attribute names.
//
//  Dave Pugmire, Fri Feb 12 14:02:57 EST 2010
//  Support for transparency sorting.
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawAsTubes(vtkPolyData *data)
{
    vtkTubeFilter *tube = vtkTubeFilter::New();
    tube->SetRadius(atts.GetTubeRadius());

    tube->SetNumberOfSides(atts.GetTubeDisplayDensity());
    tube->SetCapping(1);
    tube->ReleaseDataFlagOn();

    //Easy case, make tubes and we're done.
    if (!atts.GetDisplayBeginFlag() && !atts.GetDisplayEndFlag())
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

    if (appendForTranspPolys)
        appendForTranspPolys->AddInput(tube->GetOutput());
    else
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
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Changed some attribute names.
//
//  Dave Pugmire, Fri Feb 12 14:02:57 EST 2010
//  Support for transparency sorting.
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
        ribbons->SetWidth(atts.GetRibbonWidth());
        ribbons->SetInput(pd);
        ribbons->Update();
        
        if (appendForTranspPolys)
            appendForTranspPolys->AddInput(ribbons->GetOutput());
        else
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
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Changed some attribute names.
//
//  Dave Pugmire, Fri Feb 12 14:02:57 EST 2010
//  Support for transparency sorting.
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawSeedPoints(vtkPolyData *data)
{
    CalculateSpherePts();
    double rad = atts.GetSeedDisplayRadius();
    int quality = (int)(atts.GetGeomDisplayQuality());

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

        if (appendForTranspPolys)
        {
            vtkPolyData *pd = GenerateSpherePolys(pt[0], pt[1], pt[2], rad, quality,
                                                  s[*segptr], (o?o[*segptr]:1.0), 0.0);
            appendForTranspPolys->AddInput(pd);
            pd->Delete();
        }
        else
        {
            glBegin(GL_QUADS);
            SetColor(s[*segptr], (o?o[*segptr]:1.0));
            DrawSphereAsQuads(pt[0],pt[1], pt[2], rad, quality);
            glEnd();
        }
        segptr += nPts;
    }
}
// ****************************************************************************
//  Method:  avtOpenGLStreamlineRenderer::DrawHeadGeom
//
//  Purpose:
//    Draw each seed point as a sphere.
//
//  Programmer:  Dave Pugmire
//  Creation:    January 20, 2010
//
//  Modifications:
//
//  Dave Pugmire, Fri Feb 12 14:02:57 EST 2010
//  Support for transparency sorting.
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawHeadGeom(vtkPolyData *data)
{
    CalculateSpherePts();
    double rad = atts.GetHeadDisplayRadius();
    int quality = (int)(atts.GetGeomDisplayQuality());

    vtkPoints *points = data->GetPoints();
    vtkCellArray *lines = data->GetLines();
    vtkIdType *segments = lines->GetPointer();
    float *s = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *o = NULL;
    if (data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str()))
        o = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str())->GetVoidPointer(0);
    
    int *segptr = segments;
    double endPt[3];
    unsigned char rgba[4];
    float scalar;

    for (int i=0; i<data->GetNumberOfLines(); i++)
    {
        int nPts = *segptr;
        segptr++; //Now segptr points at vtx0.

        int idx0 = 0, idx1 = nPts;
        double t0=0.0, t1=0.0;
        GetEndPoints(data, segptr, nPts, idx0, idx1, t0, t1);
        
        
        if (idx1 < nPts)
        {
            double next[3], pt[3];
            points->GetPoint(segptr[idx1], pt);
            points->GetPoint(segptr[idx1+1], next);
            
            endPt[0] = pt[0] + t1*(next[0]-pt[0]);
            endPt[1] = pt[1] + t1*(next[1]-pt[1]);
            endPt[2] = pt[2] + t1*(next[2]-pt[2]);
            
            float  s0, s1;
            s0 = s[segptr[idx1]];
            s1 = s[segptr[idx1+1]];
            scalar = s0 + t1*(s1-s0);
        }
        else
        {
            points->GetPoint(segptr[nPts-1], endPt);
            scalar = s[*(segptr+nPts-1)];
        }
        
        if (appendForTranspPolys)
        {
            float param = atts.GetTermination();
            if (atts.GetDisplayEndFlag())
                param = atts.GetDisplayEnd();
            vtkPolyData *pd = GenerateSpherePolys(endPt[0], endPt[1], endPt[2], rad, quality,
                                                  scalar, (o?o[*segptr]:1.0), param);
            appendForTranspPolys->AddInput(pd);
            pd->Delete();
        }
        else
        {
            glBegin(GL_QUADS);
            SetColor(scalar, (o?o[*segptr]:1.0));
            DrawSphereAsQuads(endPt[0],endPt[1], endPt[2], rad, quality);
            glEnd();
        }
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
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Copy over the param array.
//
// ****************************************************************************

vtkPolyData *
avtOpenGLStreamlineRenderer::MakeNewPolyline(vtkPolyData *data,
                                             int *&segptr)
{
    vtkPoints *points = data->GetPoints();
    float *s = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *p = (float *)data->GetPointData()->GetArray(avtStreamlinePolyDataFilter::paramArrayName.c_str())->GetVoidPointer(0);
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
    vtkFloatArray *params = vtkFloatArray::New();
    params->SetName(avtStreamlinePolyDataFilter::paramArrayName.c_str());

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
    params->Allocate(nNewPts);
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

        v0 = p[segptr[idx0-1]];
        v1 = p[segptr[idx0]];
        v = v0 + t0*(v1-v0);
        params->InsertTuple1(idx, v);

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
        params->InsertTuple1(idx, p[segptr[i]]);
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

        v0 = p[segptr[idx1]];
        v1 = p[segptr[idx1+1]];
        v = v0 + t1*(v1-v0);
        params->InsertTuple1(idx, v);

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
    pd->GetPointData()->AddArray(params);
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
    params->Delete();

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
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Ramp opacity.
//
//  Dave Pugmire, Fri Feb 12 14:02:57 EST 2010
//  Support for transparency sorting.
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::DrawPolyData(vtkPolyData *input)
{
    if (input->GetPoints() == NULL)
        return;

    input->Update();
    vtkPolyData *poly = input;
    vtkDepthSortPolyData *sorter = NULL;

    if (appendForTranspPolys)
    {
        sorter = vtkDepthSortPolyData::New();
        sorter->SetCamera(camera);
        sorter->SortScalarsOn();
        sorter->SetDirectionToFrontToBack();
        sorter->SetDepthSortModeToBoundsCenter();
        
        sorter->SetInput(input);
        sorter->Update();
        poly = sorter->GetOutput();
    }


    float *p = (float *)poly->GetPoints()->GetVoidPointer(0);
    float *n = (float *)poly->GetPointData()->GetNormals()->GetVoidPointer(0);
    float *s = (float *)poly->GetPointData()->GetArray(avtStreamlinePolyDataFilter::colorvarArrayName.c_str())->GetVoidPointer(0);
    float *param = (float *)poly->GetPointData()->GetArray(avtStreamlinePolyDataFilter::paramArrayName.c_str())->GetVoidPointer(0);
    float *o = NULL;
    if (poly->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str()))
        o = (float *)poly->GetPointData()->GetArray(avtStreamlinePolyDataFilter::opacityArrayName.c_str())->GetVoidPointer(0);
    
    vtkCellArray *strips = poly->GetStrips();
    vtkIdType *ptIds = strips->GetPointer();
    vtkIdType *endPtIds = ptIds + strips->GetNumberOfConnectivityEntries();
    float alpha = 1.0;

    while (ptIds < endPtIds)
    {
        int nPts = *ptIds;
        ++ptIds;
        glBegin(GL_TRIANGLE_STRIP);
        while (nPts > 0)
        {
            if (atts.GetOpacityType() == StreamlineAttributes::Ramp)
            {
                alpha = ComputeRampOpacity(param[*ptIds]);
            }
            else if (o)
                alpha = o[*ptIds];
            
            SetColor(s[*ptIds], alpha);
            glNormal3fv(n + 3*(*ptIds));
            glVertex3fv(p + 3*(*ptIds));
            
            ++ptIds;
            --nPts;
        }
        glEnd();
    }

    if (sorter)
        sorter->Delete();
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
//   Dave Pugmire, Wed Jan 20 09:28:59 EST 2010
//   Ramp opacity.
//
// ****************************************************************************

void
avtOpenGLStreamlineRenderer::SetColor(const float &scalar,
                                      const float &opacity) const
{
    // If solid and no variable opacity, coloring already set.
    if (atts.GetColoringMethod() == StreamlineAttributes::Solid &&
        atts.GetOpacityType() != StreamlineAttributes::VariableRange &&
        atts.GetOpacityType() != StreamlineAttributes::Ramp)
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
    if (atts.GetOpacityType() == StreamlineAttributes::Ramp)
    {
        rgba[3] = (unsigned char)(opacity*atts.GetOpacity()*255.0f);
    }
    else if (atts.GetOpacityType() == StreamlineAttributes::Constant ||
             atts.GetOpacityType() == StreamlineAttributes::VariableRange)
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
//  Method:  avtOpenGLStreamlineRenderer::GenerateSpherePolys
//
//  Purpose:
//    
//  Programmer:  Dave Pugmire
//  Creation:    February 12, 2010
//
//  Modifications:
//
// ****************************************************************************

vtkPolyData *
avtOpenGLStreamlineRenderer::GenerateSpherePolys(float x0,
                                                 float y0,
                                                 float z0,
                                                 float r,
                                                 int detail,
                                                 float scalar,
                                                 float opacity,
                                                 float param)
{
    CalculateSpherePts();

    int qdetail = sphereQualityLevels[detail][0];
    int hdetail = sphereQualityLevels[detail][1];

    vtkPoints *pts = vtkPoints::New();
    pts->Allocate(5000,10000);
    vtkCellArray *polys = vtkCellArray::New();
    vtkFloatArray *norms = vtkFloatArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();
    scalars->SetName(avtStreamlinePolyDataFilter::colorvarArrayName.c_str());
    vtkFloatArray *params = vtkFloatArray::New();
    params->SetName(avtStreamlinePolyDataFilter::paramArrayName.c_str());
    
    norms->SetNumberOfComponents(3);
    norms->SetName("Normals");
    polys->Allocate(10000,20000);
    vtkIdType tri[3], ids[4];

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

            float pt[3];
            pt[0] = x0 + r*v00[0]; pt[1] = y0 + r*v00[1]; pt[2] = z0 + r*v00[2];
            ids[0] = pts->InsertNextPoint(pt);

            pt[0] = x0 + r*v01[0]; pt[1] = y0 + r*v01[1]; pt[2] = z0 + r*v01[2];
            ids[1] = pts->InsertNextPoint(pt);
            
            pt[0] = x0 + r*v11[0]; pt[1] = y0 + r*v11[1]; pt[2] = z0 + r*v11[2];
            ids[2] = pts->InsertNextPoint(pt);

            pt[0] = x0 + r*v10[0]; pt[1] = y0 + r*v10[1]; pt[2] = z0 + r*v10[2];
            ids[3] = pts->InsertNextPoint(pt);

            norms->InsertNextTuple(v00);
            norms->InsertNextTuple(v01);
            norms->InsertNextTuple(v11);
            norms->InsertNextTuple(v10);

            for (int i = 0; i < 4; i++)
            {
                scalars->InsertNextTuple1(scalar);
                params->InsertNextTuple1(param);
            }
            
            tri[0] = ids[1];
            tri[1] = ids[2];
            tri[2] = ids[0];
            polys->InsertNextCell(3, tri);
            
            tri[0] = ids[2];
            tri[1] = ids[3];
            tri[2] = ids[0];
            polys->InsertNextCell(3, tri);
        }
    }
    
    vtkPolyData *pd = vtkPolyData::New();
    polys->Squeeze();

    pd->SetPoints(pts);
    pd->SetPolys(polys);
    pd->GetPointData()->SetNormals(norms);
    pd->GetPointData()->AddArray(scalars);
    pd->GetPointData()->AddArray(params);

    pts->Delete();
    polys->Delete();
    scalars->Delete();
    params->Delete();
    norms->Delete();

    //tri strip them.
    vtkStripper *stripper = vtkStripper::New();
    stripper->SetInput(pd);
    stripper->Update();
    
    pd->Delete();
    vtkPolyData *output = stripper->GetOutput();
    output->Register(NULL);
    stripper->Delete();
    
    return output;
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
//  Method:  avtOpenGLStreamlineRenderer::ComputueRampOpacity
//
//  Purpose:
//    
//  Programmer:  Dave Pugmire
//  Creation:    February 12, 2010
//
//  Modifications:
//
// ****************************************************************************

float
avtOpenGLStreamlineRenderer::ComputeRampOpacity(const float &p) const
{
    float p0 = 0, p1 = atts.GetTermination();
    if (atts.GetDisplayBeginFlag())
        p0 = atts.GetDisplayBegin();
    if (atts.GetDisplayEndFlag())
        p1 = atts.GetDisplayEnd();
    
    float o = (p-p0) / (p1-p0);
    //cout<<"p: "<<p<<" --> "<<o<<" ["<<p0<<" "<<p1<<"]"<<endl;
    //if (o < 0.0 || o > 1.0)

    if (o < 0.0)
        o = 0.0;
    if (o > 1.0)
        o = 1.0;
    
    return o;
}
