// ************************************************************************* //
//                      avtOpenGL3DTextureVolumeRenderer.C                   //
// ************************************************************************* //

#include "avtOpenGL3DTextureVolumeRenderer.h"

#if defined(_WIN32)
#include <windows.h>
#endif
#include <GL/gl.h>

#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <BoundingBoxContourer.h>
#include <VolumeAttributes.h>
#include <avtViewInfo.h>
#include <avtCallback.h>
#include <LightList.h>

#include <float.h>

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


// ****************************************************************************
//  Method: avtOpenGL3DTextureVolumeRenderer::avtOpenGL3DTextureVolumeRenderer
//
//  Purpose:
//    Initialize the texture memory pointer and the OpenGL texture ID.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 30, 2003
//
// ****************************************************************************
avtOpenGL3DTextureVolumeRenderer::avtOpenGL3DTextureVolumeRenderer()
{
    volumetex = NULL;
    volumetexId = 0;
}


// ****************************************************************************
//  Method: avtOpenGL3DTextureVolumeRenderer::~avtOpenGL3DTextureVolumeRenderer
//
//  Purpose:
//    Destructor.  Free volumetex.
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
// ****************************************************************************
avtOpenGL3DTextureVolumeRenderer::~avtOpenGL3DTextureVolumeRenderer()
{
    delete[] volumetex;
    volumetex = NULL;

    // Assumes context is current!  This is the job of the container class.
    if (volumetexId != 0)
    {
        glDeleteTextures(1, &volumetexId);
        volumetexId = 0;
    }
}


// ****************************************************************************
//  Method:  avtOpenGL3DTextureVolumeRenderer::Render
//
//  Purpose:
//    Render one image using a 3D texture and view-aligned slice planes.
//
//  Arguments:
//    grid      : the data set to render
//    data,opac : the color/opacity variables
//    view      : the viewing information
//    atts      : the current volume plot attributes
//    vmin/max/size : the min/max/range of the color variable
//    omin/max/size : the min/max/range of the opacity variable
//    gx/gy/gz      : the gradient of the opacity variable
//    gmn           : the gradient magnitude, normalized to the max grad mag
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
//  Modifications:
//
// ****************************************************************************
void
avtOpenGL3DTextureVolumeRenderer::Render(vtkRectilinearGrid *grid,
                                         vtkDataArray *data,
                                         vtkDataArray *opac,
                                         const avtViewInfo &view,
                                         const VolumeAttributes &atts,
                                         float vmin, float vmax, float vsize,
                                         float omin, float omax, float osize,
                                         float *gx, float *gy, float *gz,
                                         float *gmn)
{
    // Get the transfer function
    int ncolors=256;
    int nopacities=256;
    unsigned char rgba[256*4];
    atts.GetTransferFunction(rgba);
    
    // Get the dimensions
    int dims[3];
    grid->GetDimensions(dims);

    int nx=dims[0];
    int ny=dims[1];
    int nz=dims[2];

    // Find the smallest power of two in each dimension 
    // that will accomodate this data set
    int newnx = MAX(int(pow(2,1+int(log(nx-1)/log(2)))),1);
    int newny = MAX(int(pow(2,1+int(log(ny-1)/log(2)))),1);
    int newnz = MAX(int(pow(2,1+int(log(nz-1)/log(2)))),1);

    // Get the new lighting parameters
    LightList lights = avtCallback::GetCurrentLightList();

    // Determine if we need to invalidate the old texture
    if (volumetex && (atts != oldAtts || lights != oldLights))
    {
        glDeleteTextures(1, &volumetexId);
        volumetexId = 0;
        delete[] volumetex;
        volumetex = NULL;
    }
    oldAtts = atts;
    oldLights = lights;


    //
    // Extract the lighting information from the actual light list
    //

    float light[4] = {0,0,1, 0};
    float ambient = 0.0;

    // Find an ambient light
    for (int i=0; i<lights.NumLights(); i++)
    {
        const LightAttributes &l = lights.GetLight(i);
        if (l.GetEnabledFlag() && l.GetType()==LightAttributes::Ambient)
        {
            // Take it's overall brightness
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
            // Take it's direction
            const double *dir = l.GetDirection();
            light[0] = dir[0];
            light[1] = dir[1];
            light[2] = dir[2];
            break;
        }
    }

    // If we want to transform the light so it is attached to the camera:
    //I->MultiplyPoint(light, light);


    //
    // Create the 3D texture if we need to
    //

    if (!volumetex)
    {
        int nels=newnx*newny*newnz;
        volumetex = new unsigned char[4*nels];
        int outindex = -1;
        for (int k=0; k<newnz; k++)
        {
            for (int j=0; j<newny; j++)
            {
                for (int i=0; i<newnx; i++)
                {
                    int ijk[3]={i,j,k};
                    outindex++;
                    volumetex[outindex*4 + 0] = 0;
                    volumetex[outindex*4 + 1] = 0;
                    volumetex[outindex*4 + 2] = 0;
                    volumetex[outindex*4 + 3] = 0;

                    if (i>=nx || j>=ny || k>=nz)
                    {
                        // out of bounds data
                        continue;
                    }

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

                    // opactity map
                    float opacity;
                    opacity = float(rgba[int(o*(nopacities-1))*4 + 3])*
                        atts.GetOpacityAttenuation()/256.;
                    opacity = MAX(0,MIN(1,opacity));

                    // drop transparent splats 
                    //if (opacity < .0001)
                    //    continue;

                    // do shading
                    float brightness;
                    const bool shading = atts.GetLightingFlag();
                    if (shading)
                    {
                        // Get the gradient
                        float gi = gx[index];
                        float gj = gy[index];
                        float gk = gz[index];

                        // Amount of shading should be somewhat proportional
                        // to the magnitude of the gradient
                        float gm = pow(gmn[index], 0.25);

                        // Get the base lit brightness based on the 
                        // light direction and the gradient
                        float grad[3] = {gi,gj,gk};
                        float lightdir[3] = {light[0],light[1],light[2]};
                        brightness = vtkMath::Dot(grad,lightdir);
                        if (brightness<0) brightness *= -1;

                        // Modulate by the gradient magnitude
                        brightness = (1.0 - gm)*1.0 + gm*brightness;

                        // Modulate by the amount of ambient lighting
                        brightness = (1.0 - ambient)*brightness + ambient;
                    }
                    else
                    {
                        // No shading ata ll
                        brightness=1;
                    }
                
                    // Determine the actual color and opacity now
                    int colorindex = int(ncolors * v);
                    if (colorindex < 0)
                        colorindex = 0;
                    if (colorindex >= ncolors)
                        colorindex =  ncolors-1;
                    int colorindex4 = colorindex*4;
                    float scaledbrightness = brightness/255.;
                    float r,g,b;
                    r = float(rgba[colorindex4 + 0])*scaledbrightness;
                    g = float(rgba[colorindex4 + 1])*scaledbrightness;
                    b = float(rgba[colorindex4 + 2])*scaledbrightness;

                    volumetex[outindex*4 + 0] = (unsigned char)(r*255);
                    volumetex[outindex*4 + 1] = (unsigned char)(g*255);
                    volumetex[outindex*4 + 2] = (unsigned char)(b*255);
                    volumetex[outindex*4 + 3] = (unsigned char)(opacity*255);
                }
            }
        }

        // Create the texture
        //glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glGenTextures(1, &volumetexId);
        glBindTexture(GL_TEXTURE_3D, volumetexId);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, newnx, newny, newnz,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, volumetex);
    }

    //
    // We have the texture; now draw with it
    //

    // Set up OpenGL parameters
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_3D, volumetexId);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_3D);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDepthMask(false);

    // Set up camera parameters
    vtkCamera *camera = vtkCamera::New();
    view.SetCameraFromView(camera);
    vtkMatrix4x4 *cameraMatrix = camera->GetViewTransformMatrix();

    //
    // Contour the bounding box at a user specified number of depths
    //

    BoundingBoxContourer bbox;

    // Extract the depth values at the corners of our bounding box
    int mapStructuredToUnstructured[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    int bbox_index = 0;
    float minz =  FLT_MAX;
    float maxz = -FLT_MAX;
    for (int k=0; k<nz; k+=nz-1)
    {
        for (int j=0; j<ny; j+=ny-1)
        {
            for (int i=0; i<nx; i+=nx-1)
            {
                int ijk[] = {i,j,k};
                float worldpt[4] = {0,0,0,1};
                grid->GetPoint(grid->ComputePointId(ijk), worldpt);

                // Get the world space coordinates

                // The contourer expects an unstructured hex
                int pt_index = mapStructuredToUnstructured[bbox_index];

                bbox.x[pt_index] = worldpt[0];
                bbox.y[pt_index] = worldpt[1];
                bbox.z[pt_index] = worldpt[2];

                // Get the texture coordinates
                bbox.r[pt_index] = float(i) / float(newnx + 0) + (0.5 / float(newnx));
                bbox.s[pt_index] = float(j) / float(newny + 0) + (0.5 / float(newny));
                bbox.t[pt_index] = float(k) / float(newnz + 0) + (0.5 / float(newnz));

                // Get the camera space coordinates
                float viewpt[4];
                cameraMatrix->MultiplyPoint(worldpt, viewpt);
                float dist = viewpt[2];

                bbox.v[pt_index] = dist;

                if (dist < minz)
                    minz = dist;
                if (dist > maxz)
                    maxz = dist;

                bbox_index++;
            }
        }
    }

    // Determine the depth values we need
    int pt0 = mapStructuredToUnstructured[0];
    int ptn = mapStructuredToUnstructured[7];
    float dx = bbox.x[pt0] - bbox.x[ptn];
    float dy = bbox.y[pt0] - bbox.y[ptn];
    float dz = bbox.z[pt0] - bbox.z[ptn];
    float dist = sqrt(dx*dx + dy*dy + dz*dz);

    // Do the actual contouring 
    glBegin(GL_TRIANGLES);
    glColor4f(1.,1.,1.,1.);
    int ns = atts.GetNum3DSlices();
    if (ns < 1)
        ns = 1;
    if (ns > 500)
        ns = 500;
    for (int z=0; z<ns; z++)
    {
        float value = (float(z)+0.5)/float(ns);
        bbox.ContourTriangles(minz + dist*value);
    }
    glEnd();

    // Set some GL parameters back to their expected values and free memory
    glDepthMask(true);
    glDisable(GL_TEXTURE_3D);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    opac->Delete();
    data->Delete();
    camera->Delete();
}


#if 0
// ****************************************************************************
//  Method:  avtOpenGL3DTextureVolumeRenderer::Render
//
//  Purpose:
//    Render one image using axis-aligned slices.  This code is
//    disabled for now, but kept as a reference if we want to include
//    a 2D texturing volume renderer
//
//  Arguments:
//    grid      : the data set to render
//    data,opac : the color/opacity variables
//    view      : the viewing information
//    atts      : the current volume plot attributes
//    vmin/max/size : the min/max/range of the color variable
//    omin/max/size : the min/max/range of the opacity variable
//    gx/gy/gz      : the gradient of the opacity variable
//    gmn           : the gradient magnitude, normalized to the max grad mag
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  1, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtOpenGL3DTextureVolumeRenderer::Render(vtkRectilinearGrid *grid,
                                         vtkDataArray *data,
                                         vtkDataArray *opac,
                                         const avtViewInfo &view,
                                         const VolumeAttributes &atts,
                                         float vmin, float vmax, float vsize,
                                         float omin, float omax, float osize,
                                         float *gx, float *gy, float *gz,
                                         float *gmn)
{
    int dims[3];
    grid->GetDimensions(dims);

    // get attribute parameters
    int ncolors=256;
    int nopacities=256;
    unsigned char rgba[256*4];
    atts.GetTransferFunction(rgba);
    
    int nx=dims[0];
    int ny=dims[1];
    int nz=dims[2];

    int newnx = MAX(int(pow(2,1+int(log(nx-1)/log(2)))),1);
    int newny = MAX(int(pow(2,1+int(log(ny-1)/log(2)))),1);
    int newnz = MAX(int(pow(2,1+int(log(nz-1)/log(2)))),1);

    LightList lights = avtCallback::GetCurrentLightList();

    // Create the 3D texture
    if (volumetex && 
        (atts != oldAtts ||
         lights != oldLights))
    {
        glDeleteTextures(1, &volumetexId);
        volumetexId = 0;
        delete[] volumetex;
        volumetex = NULL;
    }
    oldAtts = atts;
    oldLights = lights;

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

    if (!volumetex)
    {
        int nels=newnx*newny*newnz;
        volumetex = new unsigned char[4*nels];
        int outindex = -1;
        for (int k=0; k<newnz; k++)
        {
            for (int j=0; j<newny; j++)
            {
                for (int i=0; i<newnx; i++)
                {
                    int ijk[3]={i,j,k};
                    outindex++;
                    volumetex[outindex*4 + 0] = 0;
                    volumetex[outindex*4 + 1] = 0;
                    volumetex[outindex*4 + 2] = 0;
                    volumetex[outindex*4 + 3] = 0;

                    if (i>=nx || j>=ny || k>=nz)
                    {
                        // out of bounds
                        continue;
                    }

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
                    opacity = float(rgba[int(o*(nopacities-1))*4 + 3])*
                        atts.GetOpacityAttenuation()/256.;
                    opacity = MAX(0,MIN(1,opacity));

                    // drop transparent splats 
                    //if (opacity < .0001)
                    //    continue;

                    // do shading
                    float brightness;
                    const bool shading = atts.GetLightingFlag();
                    if (shading)
                    {
                        float gi = gx[index];
                        float gj = gy[index];
                        float gk = gz[index];

                        // Amount of shading should be somewhat proportional
                        // to the magnitude of the gradient
                        float gm = pow(gmn[index], 0.25);

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

                    volumetex[outindex*4 + 0] = (unsigned char)(r*255);
                    volumetex[outindex*4 + 1] = (unsigned char)(g*255);
                    volumetex[outindex*4 + 2] = (unsigned char)(b*255);
                    volumetex[outindex*4 + 3] = (unsigned char)(opacity*255);
                }
            }
        }

        // Create the texture
        //glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glGenTextures(1, &volumetexId);
        glBindTexture(GL_TEXTURE_3D, volumetexId);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, newnx, newny, newnz,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, volumetex);
    }

    // Set up OpenGL parameters
    glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_3D, volumetexId);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_3D);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

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

    float tci, tcj, tck;
    float *tc1, *tc2, *tc3;
    float tcimax = float(nx)/float(newnx);
    float tcjmax = float(ny)/float(newny);
    float tckmax = float(nz)/float(newnz);
    float *tc1max, *tc2max, *tc3max;

    if (avx>=avy && avx>=avz)
    {
        c1    = &i;
        c1min = &imin;
        c1max = &imax;
        c1s   = &svx;
        tc1   = &tci;
        tc1max= &tcimax;
        if (avy>avz)
        {
            c2    = &j;
            c2min = &jmin;
            c2max = &jmax;
            c2s   = &svy;
            tc2   = &tcj;
            tc2max= &tcjmax;
            c3    = &k;
            c3min = &kmin;
            c3max = &kmax;
            c3s   = &svz;
            tc3   = &tck;
            tc3max= &tckmax;
        }
        else
        {
            c3    = &j;
            c3min = &jmin;
            c3max = &jmax;
            c3s   = &svy;
            tc3   = &tcj;
            tc3max= &tcjmax;
            c2    = &k;
            c2min = &kmin;
            c2max = &kmax;
            c2s   = &svz;
            tc2   = &tck;
            tc2max= &tckmax;
        }
    }
    else if (avy>=avx && avy>=avz)
    {
        c1    = &j;
        c1min = &jmin;
        c1max = &jmax;
        c1s   = &svy;
        tc1   = &tcj;
        tc1max= &tcjmax;
        if (avx>avz)
        {
            c2    = &i;
            c2min = &imin;
            c2max = &imax;
            c2s   = &svx;
            tc2   = &tci;
            tc2max= &tcimax;
            c3    = &k;
            c3min = &kmin;
            c3max = &kmax;
            c3s   = &svz;
            tc3   = &tck;
            tc3max= &tckmax;
        }
        else
        {
            c3    = &i;
            c3min = &imin;
            c3max = &imax;
            c3s   = &svx;
            tc3   = &tci;
            tc3max= &tcimax;
            c2    = &k;
            c2min = &kmin;
            c2max = &kmax;
            c2s   = &svz;
            tc2   = &tck;
            tc2max= &tckmax;
        }
    }
    else if (avz>=avx && avz>=avy)
    {
        c1    = &k;
        c1min = &kmin;
        c1max = &kmax;
        c1s   = &svz;
        tc1   = &tck;
        tc1max= &tckmax;
        if (avx>avy)
        {
            c2    = &i;
            c2min = &imin;
            c2max = &imax;
            c2s   = &svx;
            tc2   = &tci;
            tc2max= &tcimax;
            c3    = &j;
            c3min = &jmin;
            c3max = &jmax;
            c3s   = &svy;
            tc3   = &tcj;
            tc3max= &tcjmax;
        }
        else
        {
            c3    = &i;
            c3min = &imin;
            c3max = &imax;
            c3s   = &svx;
            tc3   = &tci;
            tc3max= &tcimax;
            c2    = &j;
            c2min = &jmin;
            c2max = &jmax;
            c2s   = &svy;
            tc2   = &tcj;
            tc2max= &tcjmax;
        }
    }

    glDepthMask(false);

    glBegin(GL_QUADS);
    glColor4f(1.,1.,1.,1.);
    for (*c1 = *c1min; *c1 != *c1max; *c1 += *c1s)
    {
        *tc1 = *tc1max * float(*c1 - *c1min) / float(*c1max - *c1min);
        if (*c1s < 0)
            *tc1 = *tc1max  - *tc1;

        *c2 = *c2min;
        *c3 = *c3min;
        int ijk1[]={i,j,k};
        float p1[3];
        grid->GetPoint(grid->ComputePointId(ijk1),p1);

        *c2 = *c2max-*c2s;
        *c3 = *c3min;
        int ijk2[]={i,j,k};
        float p2[3];
        grid->GetPoint(grid->ComputePointId(ijk2),p2);

        *c2 = *c2max-*c2s;
        *c3 = *c3max-*c3s;
        int ijk3[]={i,j,k};
        float p3[3];
        grid->GetPoint(grid->ComputePointId(ijk3),p3);

        *c2 = *c2min;
        *c3 = *c3max-*c3s;
        int ijk4[]={i,j,k};
        float p4[3];
        grid->GetPoint(grid->ComputePointId(ijk4),p4);

        *tc2 = (*c2s > 0) ? 0 : *tc2max; 
        *tc3 = (*c3s > 0) ? 0 : *tc3max; 
        glTexCoord3f(tci, tcj, tck);
        glVertex3fv(p1);

        *tc2 = (*c2s > 0) ? *tc2max : 0; 
        *tc3 = (*c3s > 0) ? 0 : *tc3max; 
        glTexCoord3f(tci, tcj, tck);
        glVertex3fv(p2);

        *tc2 = (*c2s > 0) ? *tc2max : 0; 
        *tc3 = (*c3s > 0) ? *tc3max : 0; 
        glTexCoord3f(tci, tcj, tck);
        glVertex3fv(p3);

        *tc2 = (*c2s > 0) ? 0 : *tc2max; 
        *tc3 = (*c3s > 0) ? *tc3max : 0; 
        glTexCoord3f(tci, tcj, tck);
        glVertex3fv(p4);
    }

    glEnd();

    glDepthMask(true);

    glDisable(GL_TEXTURE_3D);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    opac->Delete();
    data->Delete();
    camera->Delete();
    I->Delete();
}
#endif

