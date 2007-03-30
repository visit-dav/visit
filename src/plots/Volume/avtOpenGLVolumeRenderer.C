// ************************************************************************* //
//                           avtOpenGLVolumeRenderer.C                       //
// ************************************************************************* //

#include "avtOpenGLVolumeRenderer.h"

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>

#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


// ****************************************************************************
//  Method:  avtOpenGLVolumeRenderer::Render
//
//  Purpose:
//    Render one image
//
//  Arguments:
//    ds      : the data set to render
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
// ****************************************************************************

void
avtOpenGLVolumeRenderer::Render(vtkDataSet *ds)
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
    }

    // Do other initialization
    if (!initialized)
    {
        Initialize(ds);
    }

    // get data set
    vtkRectilinearGrid  *grid = (vtkRectilinearGrid*)ds;
    vtkDataArray *data = NULL;
    vtkDataArray *opac = NULL;
    GetScalars(grid, data, opac);
    int dims[3];
    grid->GetDimensions(dims);

    // get attribute parameters
    int ncolors=256;
    int nopacities=256;
    unsigned char rgba[256*4];
    atts.GetTransferFunction(rgba);
    
    // Set up OpenGL parameters
    glDisable(GL_LIGHTING);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                 GRIDSIZE,GRIDSIZE,0, GL_ALPHA, GL_FLOAT, alphatex);
    glEnable(GL_TEXTURE_2D);

    // set up parameters
    vtkCamera *camera = vtkCamera::New();
    view.SetCameraFromView(camera);
    vtkMatrix4x4 *cameraMatrix = camera->GetViewTransformMatrix();
    vtkMatrix4x4 *I = vtkMatrix4x4::New();
    I->DeepCopy(cameraMatrix);
    I->Invert();

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



    // Set up splat size and the splat stuff
    vtkDataArray *xc = grid->GetXCoordinates();
    vtkDataArray *yc = grid->GetYCoordinates();
    vtkDataArray *zc = grid->GetZCoordinates();

    float width  = xc->GetTuple1(1) - xc->GetTuple1(0);
    float height = yc->GetTuple1(1) - yc->GetTuple1(0);
    float depth  = zc->GetTuple1(1) - zc->GetTuple1(0);

    float size = MAX(MAX(width,height),depth);

    float viewdir[4] = {0,0,1, 0};
    float V1[4] = {0,size*1.8,0};
    float V2[4] = {size*1.8,0,0};
    I->MultiplyPoint(viewdir, viewdir);
    I->MultiplyPoint(V1,V1);
    I->MultiplyPoint(V2,V2);

    float light[4] = {0,0,1, 0};
    I->MultiplyPoint(light, light);

    // Splat it!
    float color[4];
    glDepthMask(false);
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
                int index = grid->ComputePointId(ijk);
                float  v = data->GetTuple1(index);
                float  o = opac->GetTuple1(index);

                // drop empty ones
                if (v < -1e+37)
                    continue;

                // normalize the value
                v = (v < vmin ? vmin : v);
                v = (v > vmax ? vmax : v);
                v = (v-vmin)/vsize;
                o = (o < omin ? omin : o);
                o = (o > omax ? omax : o);
                o = (o-omin)/osize;

                // opactity map:
                float opacity;
                opacity = float(rgba[int(o*(nopacities-1))*4 + 3])*atts.GetOpacityAttenuation()/256.;
                opacity = MAX(0,MIN(1,opacity));

                // drop transparent splats 
                if (opacity < .0001)
                    continue;

                // get the point
                float *p = grid->GetPoint(index);

                float gi = gx[index];
                float gj = gy[index];
                float gk = gz[index];

                // do shading
                float brightness;
                const bool shading = atts.GetLightingFlag();
                if (shading)
                {
                    float grad[3] = {gi,gj,gk};
                    brightness = vtkMath::Dot(grad,light);
                    if (brightness<0) brightness *= -1;

                    // no advanced shading options
                    /* ** HRC, 11/19/01, if this code ever becomes uncommented,
                       ** it must be modified to address separate opacity and
                       ** color variables. 
                    if (shadingGradientScale)
                        brightness = 1*(1-data->gmn[index]) +
                            brightness*data->gmn[index];
                    if (shadingOpacityScale)
                        brightness = 1*(1-opacity) +
                            brightness*opacity;
                    */
                }
                else
                {
                    brightness=1;
                }
                
                float r,g,b;

                int colorindex = int(ncolors * v);
                if (colorindex < 0)
                    colorindex = 0;
                if (colorindex >= ncolors)
                    colorindex =  ncolors-1;
                int colorindex4 = colorindex*4;
                float scaledbrightness = brightness/255.;
                r = float(rgba[colorindex4 + 0])*scaledbrightness;
                g = float(rgba[colorindex4 + 1])*scaledbrightness;
                b = float(rgba[colorindex4 + 2])*scaledbrightness;

                // draw the splat
                color[0] = r;
                color[1] = g;
                color[2] = b;
                //color[3] = opacity*alphacorrection;
                color[3] = opacity;

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
        }
    }

    glEnd();

    glDepthMask(true);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    opac->Delete();
    data->Delete();
    camera->Delete();
    I->Delete();
}


