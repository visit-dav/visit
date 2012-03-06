/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
//                      avtOpenGLSplattingVolumeRenderer.C                   //
// ************************************************************************* //

#include "avtOpenGLSplattingVolumeRenderer.h"

#include <avtGLEWInitializer.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDepthSortPolyData.h>
#include <vtkIntArray.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <VolumeAttributes.h>
#include <avtViewInfo.h>
#include <avtCallback.h>
#include <LightList.h>

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

// ****************************************************************************
//  Method: avtOpenGLSplattingVolumeRenderer::avtOpenGLSplattingVolumeRenderer
//
//  Purpose:
//    Initialize the texture memory and the OpenGL texture ID.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 30, 2003
//
// ****************************************************************************
avtOpenGLSplattingVolumeRenderer::avtOpenGLSplattingVolumeRenderer()
{
    alphatex = NULL;
    alphatexId = 0;
}


// ****************************************************************************
//  Method: avtOpenGLSplattingVolumeRenderer::~avtOpenGLSplattingVolumeRenderer
//
//  Purpose:
//    Destructor.  Free alphatex.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 30, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Oct  2 13:33:49 PDT 2003
//    Delete the texture object here since this class is no longer
//    an implementation of avtCustomRenderer, so it has no
//    ReleaseGraphicsResources method.
//
//    Thomas R. Treadway, Tue Feb  6 17:04:03 PST 2007
//    The gcc-4.x compiler no longer just warns about automatic type conversion.
//
// ****************************************************************************
avtOpenGLSplattingVolumeRenderer::~avtOpenGLSplattingVolumeRenderer()
{
    delete[] alphatex;
    // Assumes context is current
    if (alphatexId != 0)
    {
        glDeleteTextures(1, (GLuint*)&alphatexId);
        alphatexId = 0;
    }
}

// ****************************************************************************
// Method: DrawOneSplat
//
// Purpose: 
//   Draw one splat using OpenGL.
//
// Arguments:
//    props  : The renderer properties.
//    volume : The dataset that we'll render.
//    index  : The index of the point for which we're creating a splat.
//    V1     : The horizontal axis from the point
//    V2     : The vertical axis from the point (V1&V2 form an axis perp. to the camera).
//
// Returns:    
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   April  5, 2001
//
// Modifications:
//   Brad Whitlock, Mon Jan 30 11:44:02 PST 2012
//   I separated this code out into its own routine.
//
// ****************************************************************************

inline void
DrawOneSplat(const avtVolumeRendererImplementation::RenderProperties &props,
    const avtVolumeRendererImplementation::VolumeData &volume,
    vtkIdType index, const unsigned char *rgba, const float *light, float ambient,
    const float *V1, const float *V2)
{
    float  v = volume.data.data->GetTuple1(index);
    // drop empty ones
    if (v < -1e+37)
        return;

    int ncolors=256;
    int nopacities=256;

    // normalize the value
    v = (v < volume.data.min ? volume.data.min : v);
    v = (v > volume.data.max ? volume.data.max : v);
    v = (v-volume.data.min)/volume.data.size;
    float  o = volume.opacity.data->GetTuple1(index);
    o = (o < volume.opacity.min ? volume.opacity.min : o);
    o = (o > volume.opacity.max ? volume.opacity.max : o);
    o = (o-volume.opacity.min)/volume.opacity.size;

    // opactity map:
    float opacity;
    opacity = float(rgba[int(o*(nopacities-1))*4 + 3])*props.atts.GetOpacityAttenuation()/256.;
    opacity = MAX(0,MIN(1,opacity));

    // drop transparent splats 
    if (opacity < .0001)
        return;

    // get the point
    double p[3];
    volume.grid->GetPoint(index, p);

    // do shading
    float brightness = 1.f;              
    if (props.atts.GetLightingFlag())
    {
        //std::cout<<"Getting g values"<<endl;
        float gi = volume.gx[index];
        float gj = volume.gy[index];
        float gk = volume.gz[index];

        // Amount of shading should be somewhat proportional
        // to the magnitude of the gradient
        float gm = 1.0;
        if (props.atts.GetLowGradientLightingReduction() !=
            VolumeAttributes::Off)
        {
            double lp = 1.0;
            switch (props.atts.GetLowGradientLightingReduction())
            {
              case VolumeAttributes::Lowest:  lp = 1./16.;break;
              case VolumeAttributes::Lower:   lp = 1./8.; break;
              case VolumeAttributes::Low:     lp = 1./4.; break;
              case VolumeAttributes::Medium:  lp = 1./2.; break;
              case VolumeAttributes::High:    lp = 1.;    break;
              case VolumeAttributes::Higher:  lp = 2.;    break;
              case VolumeAttributes::Highest: lp = 4.;    break;
              default: break;
            }
            if (props.atts.GetLowGradientLightingClampFlag())
            {
                gm = volume.gm[index] /
                    props.atts.GetLowGradientLightingClampValue();
            }
            else
            {
                gm = volume.gmn[index];
            }
            gm = pow((double)gm, lp);
        }
        if (gm < 0)
            gm = 0;
        if (gm > 1)
            gm = 1;

        // Get the base lit brightness 
        float grad[3] = {gi,gj,gk};
        float lightdir[3] = {light[0],light[1],light[2]};
        brightness = vtkMath::Dot(grad,lightdir);
        if (brightness<0) brightness *= -1;

        // Modulate by the gradient magnitude
        brightness = (1.0 - gm)*1.0 + gm*brightness;

        // Modulate by the amount of ambient lighting
        brightness = (1.0 - ambient)*brightness + ambient;
    }

    // Figure out the color.
    int colorindex = int(ncolors * v);
    if (colorindex < 0)
        colorindex = 0;
    if (colorindex >= ncolors)
        colorindex =  ncolors-1;
    int colorindex4 = colorindex*4;
    float scaledbrightness = brightness/255.;
    float color[4];
    color[0] = float(rgba[colorindex4 + 0])*scaledbrightness;
    color[1] = float(rgba[colorindex4 + 1])*scaledbrightness;
    color[2] = float(rgba[colorindex4 + 2])*scaledbrightness;
    //color[3] = opacity*alphacorrection;
    color[3] = opacity;

    // draw the splat
    glColor4fv(color);

    glTexCoord2f(0,0);
    glVertex3f(p[0]-V1[0]-V2[0],p[1]-V1[1]-V2[1],p[2]-V1[2]-V2[2]);

    glTexCoord2f(0,1);
    glVertex3f(p[0]-V1[0]+V2[0],p[1]-V1[1]+V2[1],p[2]-V1[2]+V2[2]);

    glTexCoord2f(1,1);
    glVertex3f(p[0]+V1[0]+V2[0],p[1]+V1[1]+V2[1],p[2]+V1[2]+V2[2]);

    glTexCoord2f(1,0);
    glVertex3f(p[0]+V1[0]-V2[0],p[1]+V1[1]-V2[1],p[2]+V1[2]-V2[2]);
}

// ****************************************************************************
//  Method:  avtOpenGLSplattingVolumeRenderer::Render
//
//  Purpose:
//    Render one image
//
//  Arguments:
//    props  : The renderer properties.
//    volume : The dataset that we'll render.
//
//  Programmer:  Jeremy Meredith
//  Creation:    April  5, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Apr  6 11:00:04 PDT 2001
//    Changed the calculation of the splat size due to a change in assumptions.
//
//    Brad Whitlock, Thu Sep 6 11:11:31 PDT 2001
//    Modified the code to use the new interface to the VolumeAttributes.
//
//    Kathleen Bonnell, Mon Nov 19 16:05:37 PST 2001 
//    VTK 4.0 API changes require use of vtkDataArray in place of
//    vtkScalars for rgrid coordinates. 
//
//    Hank Childs, Mon Jan  7 16:36:11 PST 2002
//    Fix memory leak.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Hank Childs, Wed Apr 24 09:27:36 PDT 2002
//    Transferred from to this class from avtVolumeRenderer.
//
//    Jeremy Meredith, Tue Sep 30 11:53:44 PDT 2003
//    Added calls to make an opengl texture object so we don't have to
//    keep sending it.  Only get values of gradient if we are doing lighting.
//
//    Jeremy Meredith, Thu Oct  2 13:33:02 PDT 2003
//    Changed the way lighting is handled.
//    Made this class not inherit from avtVolumeRenderer, so it now receives
//    most of its state in the Render arguments every frame.
//
//    Hank Childs, Tue May 11 15:24:45 PDT 2004
//    Turn off blending so transparent surfaces can work afterwards.
//
//    Eric Brugger, Tue Jul 27 11:46:25 PDT 2004
//    Change a double constant to a float constant to fix a compile error.
//
//    Thomas R. Treadway, Tue Feb  6 17:04:03 PST 2007
//    The gcc-4.x compiler no longer just warns about automatic type conversion.
//
//    Brad Whitlock, Wed Apr 22 12:01:26 PDT 2009
//    I changed the interface.
//
//    Jeremy Meredith, Tue Jan  5 15:49:43 EST 2010
//    Added ability to reduce amount of lighting for low-gradient-mag areas.
//    This was already enabled, to some degree, by default, but it's now both
//    optional and configurable.
//
//    Allen Harvey, Brad Whitlock, Mon Jan 30 15:49:33 PST 2012
//    Add support for rendering point meshes.
//
// ****************************************************************************

void
avtOpenGLSplattingVolumeRenderer::Render(
    const avtVolumeRendererImplementation::RenderProperties &props,
    const avtVolumeRendererImplementation::VolumeData &volume)
{
    // Create the texture for a gaussian splat
    const int GRIDSIZE=32;
    if (alphatex == NULL)
    {
        alphatex = new float[GRIDSIZE*GRIDSIZE];
        for (int i=0; i<GRIDSIZE; i++)
        {
            for (int j=0; j<GRIDSIZE; j++)
            {
                float u = float(i)/float(GRIDSIZE-1);
                float v = float(j)/float(GRIDSIZE-1);

                float gu = exp(-pow((4*(u - .5)), 2));
                float gv = exp(-pow((4*(v - .5)), 2));
                alphatex[i*GRIDSIZE+j] = gu*gv;
            }
        }
        glGenTextures(1, (GLuint*)&alphatexId);
        glBindTexture(GL_TEXTURE_2D, alphatexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                     GRIDSIZE,GRIDSIZE,0, GL_ALPHA, GL_FLOAT, alphatex);
    }

    // Determine whether we should treat the data as a rectilinear grid or 
    // as an unstructured grid.
    vtkRectilinearGrid *grid = NULL;
    if(volume.grid->GetDataObjectType() == VTK_RECTILINEAR_GRID)
        grid = (vtkRectilinearGrid *)volume.grid;

    int dims[3] = {0,0,0};
    if(grid != NULL)
    {
        grid->GetDimensions(dims);
    }

    // get attribute parameters
    unsigned char rgba[256*4];
    props.atts.GetTransferFunction(rgba);
    
    // Set up OpenGL parameters
    glDisable(GL_LIGHTING);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, alphatexId);
    glEnable(GL_TEXTURE_2D);
    bool alreadyBlending = glIsEnabled(GL_BLEND);
    if (!alreadyBlending)
        glEnable(GL_BLEND);

    // set up parameters
    vtkCamera *camera = vtkCamera::New();
    props.view.SetCameraFromView(camera);
    vtkMatrix4x4 *cameraMatrix = camera->GetViewTransformMatrix();
    vtkMatrix4x4 *I = vtkMatrix4x4::New();
    I->DeepCopy(cameraMatrix);
    I->Invert();

    // Set up splat size and the splat stuff
    float size = 0.0;
    if(grid != NULL)
    {
        vtkDataArray *xc = grid->GetXCoordinates();
        vtkDataArray *yc = grid->GetYCoordinates();
        vtkDataArray *zc = grid->GetZCoordinates();
        float width  = xc->GetTuple1(1) - xc->GetTuple1(0);
        float height = yc->GetTuple1(1) - yc->GetTuple1(0);
        float depth  = zc->GetTuple1(1) - zc->GetTuple1(0);
        size = MAX(MAX(width,height),depth);
    }
    else
    {
        double p0[3], p1[3];
        volume.grid->GetPoint(0,p0);
        volume.grid->GetPoint(1,p1);
        size = sqrt( (p0[0]-p1[0])*(p0[0]-p1[0]) + (p0[1]-p1[1])*(p0[1]-p1[1]) + (p0[2]-p1[2])*(p0[2]-p1[2]) );
    }

    float viewdir[4] = {0,0,1, 0};
    float V1[4] = {0,size*1.8,0};
    float V2[4] = {size*1.8,0,0};
    I->MultiplyPoint(viewdir, viewdir);
    I->MultiplyPoint(V1,V1);
    I->MultiplyPoint(V2,V2);

    LightList lights = avtCallback::GetCurrentLightList();

    float light[4] = {0,0,1, 0};
    float ambient = 0.0;

    // Find an ambient light
    for (int i=0; i<lights.NumLights(); i++)
    {
        const LightAttributes &l = lights.GetLight(i);
        if (l.GetEnabledFlag() && l.GetType()==LightAttributes::Ambient)
        {
            double rgba[4];
            l.GetColor().GetRgba(rgba);
            ambient = l.GetBrightness() * (rgba[0]+rgba[1]+rgba[2])/3.;
            break;
        }
    }

    // Find a directional (object or camera) light
    for (int i=0; i<lights.NumLights(); i++)
    {
        const LightAttributes &l = lights.GetLight(i);
        if (l.GetEnabledFlag() && l.GetType()!=LightAttributes::Ambient)
        {
            const double *dir = l.GetDirection();
            light[0] = dir[0];
            light[1] = dir[1];
            light[2] = dir[2];
            break;
        }
    }

    // If we want to transform the light so it is attached to the camera:
    //I->MultiplyPoint(light, light);

    // Splat it!
    glDepthMask(false);

    if(grid != NULL)
    {
        // Structured case

        // Set traversal order across and within each dimension
        int  svx,svy,svz;
        int  i,j,k;
        int  imin,imax;
        int  jmin,jmax;
        int  kmin,kmax;
        int *c1,*c2,*c3;
        int *c1min,*c2min,*c3min;
        int *c1max,*c2max,*c3max;
        int *c1s,*c2s,*c3s;

        float vx[4] = {-1,0,0, 0};
        float vy[4] = {0,-1,0, 0};
        float vz[4] = {0,0,-1, 0};

        cameraMatrix->MultiplyPoint(vx,vx);
        cameraMatrix->MultiplyPoint(vy,vy);
        cameraMatrix->MultiplyPoint(vz,vz);

        float view_dir[4] = {0,0,1, 0};

        float vxm = vtkMath::Dot(vx,view_dir);
        float vym = vtkMath::Dot(vy,view_dir);
        float vzm = vtkMath::Dot(vz,view_dir);
        float avx = fabs(vxm);
        float avy = fabs(vym);
        float avz = fabs(vzm);
        svx = (vxm>0 ? -1 : 1);
        svy = (vym>0 ? -1 : 1);
        svz = (vzm>0 ? -1 : 1);

        imin = (svx>0) ? 0 : dims[0]-1;
        jmin = (svy>0) ? 0 : dims[1]-1;
        kmin = (svz>0) ? 0 : dims[2]-1;
        imax = (svx<0) ? -1 : dims[0];
        jmax = (svy<0) ? -1 : dims[1];
        kmax = (svz<0) ? -1 : dims[2];
    
        if (avx>=avy && avx>=avz)
        {
            c1    = &i;
            c1min = &imin;
            c1max = &imax;
            c1s   = &svx;
            if (avy>avz)
            {
                c2    = &j;
                c2min = &jmin;
                c2max = &jmax;
                c2s   = &svy;
                c3    = &k;
                c3min = &kmin;
                c3max = &kmax;
                c3s   = &svz;
            }
            else
            {
                c3    = &j;
                c3min = &jmin;
                c3max = &jmax;
                c3s   = &svy;
                c2    = &k;
                c2min = &kmin;
                c2max = &kmax;
                c2s   = &svz;
            }
        }
        else if (avy>=avx && avy>=avz)
        {
            c1    = &j;
            c1min = &jmin;
            c1max = &jmax;
            c1s   = &svy;
            if (avx>avz)
            {
                c2    = &i;
                c2min = &imin;
                c2max = &imax;
                c2s   = &svx;
                c3    = &k;
                c3min = &kmin;
                c3max = &kmax;
                c3s   = &svz;
            }
            else
            {
                c3    = &i;
                c3min = &imin;
                c3max = &imax;
                c3s   = &svx;
                c2    = &k;
                c2min = &kmin;
                c2max = &kmax;
                c2s   = &svz;
            }
        }
        else if (avz>=avx && avz>=avy)
        {
            c1    = &k;
            c1min = &kmin;
            c1max = &kmax;
            c1s   = &svz;
            if (avx>avy)
            {
                c2    = &i;
                c2min = &imin;
                c2max = &imax;
                c2s   = &svx;
                c3    = &j;
                c3min = &jmin;
                c3max = &jmax;
                c3s   = &svy;
            }
            else
            {
                c3    = &i;
                c3min = &imin;
                c3max = &imax;
                c3s   = &svx;
                c2    = &j;
                c2min = &jmin;
                c2max = &jmax;
                c2s   = &svy;
            }
        }

        //Using the resampled data
        glBegin(GL_QUADS);
        for (*c1 = *c1min; *c1 != *c1max; *c1 += *c1s)
        {
            // skipping the opacity correction for now
            /* ** HRC, 11/19/01, if this code ever becomes uncommented, it must
               ** be modified to address separate opacity and color variables. 
            float dist;
            if (dir==1)
                dist = fabs(data->x[i]-data->x[i-svx]);
            else if (dir==2)
                dist = fabs(data->y[j]-data->y[j-svy]);
            else // (dir==3)
                dist = fabs(data->z[k]-data->z[k-svz]);
            float hyp = dist/dp;
            float alphacorrection = (1-exp(-hyp))/(1-exp(-dist));
            */

            for (*c2 = *c2min; *c2 != *c2max; *c2 += *c2s)
            {
                for (*c3 = *c3min; *c3 != *c3max; *c3 += *c3s)
                {
                    int ijk[3]={i,j,k};
                    vtkIdType index = grid->ComputePointId(ijk);

                    DrawOneSplat(props, volume, index, rgba, light, ambient, V1, V2);
                }
            }
        }
        glEnd();
    }
    else
    { // Unstructured case

        // If our input is a point set derived class then let's sort the points 
        // according to depth. Set up a polydata containing the points and we'll depth
        // sort it so we can draw it back to front. We attach an index called "OriginalIndex"
        // so we can index into the original arrays when drawing back to front.
        vtkPointSet *psgrid = (vtkPointSet *)volume.grid;
        vtkPolyData *pd = vtkPolyData::New();
        vtkCellArray *vertices = vtkCellArray::New();
        vertices->Allocate(psgrid->GetNumberOfPoints());
        pd->SetPoints(psgrid->GetPoints());
        pd->SetVerts(vertices);
        vertices->Delete();
        vtkIntArray *iarr = vtkIntArray::New();
        iarr->SetName("OriginalIndex");
        iarr->SetNumberOfTuples(psgrid->GetNumberOfPoints());
        for (vtkIdType index = 0; index < psgrid->GetNumberOfPoints(); index++)
        {
            vertices->InsertNextCell(1, &index);
            iarr->SetTuple1(index, (double)index);
        }
        pd->GetCellData()->AddArray(iarr);
        iarr->Delete();
        vtkDepthSortPolyData *dspd = vtkDepthSortPolyData::New();
        dspd->SetDepthSortModeToFirstPoint();
        dspd->SetDirectionToBackToFront();
        dspd->SetCamera(camera);
        dspd->SetInput(pd);
        dspd->Update();

        iarr = (vtkIntArray *)dspd->GetOutput()->GetCellData()->GetArray("OriginalIndex");
        glPointSize(100.0f);
        glEnable(GL_POINT_SMOOTH);
        glBegin(GL_QUADS);
        if(iarr != NULL)
        {
            // Draw back to front.
            for (vtkIdType i = 0; i < volume.grid->GetNumberOfPoints(); i++)
            {
                vtkIdType index = (vtkIdType)iarr->GetTuple1(i);

                //Ensure the circles drawn will always face the viewer
                size = volume.hs[index];
                V1[0] = V1[2] = 0.0;
                V2[1] = V2[2] = 0.0;
                V1[1] = V2[0] = size/2.0;

                I->MultiplyPoint(V1,V1);
                I->MultiplyPoint(V2,V2);

                DrawOneSplat(props, volume, index, rgba, light, ambient, V1, V2);
            }
        }
        else
        {
            for (vtkIdType index = 0; index < volume.grid->GetNumberOfPoints(); index++)
            {
                //Ensure the circles drawn will always face the viewer
                size = volume.hs[index];
                V1[0] = V1[2] = 0.0;
                V2[1] = V2[2] = 0.0;
                V1[1] = V2[0] = size/2.0;

                I->MultiplyPoint(V1,V1);
                I->MultiplyPoint(V2,V2);

                DrawOneSplat(props, volume, index, rgba, light, ambient, V1, V2);
            }
        }
        glEnd();

        if(dspd != NULL)
            dspd->Delete();
        if(pd != NULL)
            pd->Delete();
    }  //Done Splatting

    glDepthMask(true);

    glDisable(GL_TEXTURE_2D);
    if (!alreadyBlending)
        glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    
    camera->Delete();
    I->Delete();
}
