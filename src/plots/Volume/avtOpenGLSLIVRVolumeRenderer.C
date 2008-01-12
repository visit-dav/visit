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
//                        avtOpenGLSLIVRVolumeRenderer.C                     //
// ************************************************************************* //

#include "avtOpenGLSLIVRVolumeRenderer.h"
#ifdef HAVE_LIBSLIVR

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <avtViewInfo.h>
#include <avtCallback.h>
#include <LightList.h>

#include <DebugStream.h>
//#define debug5 cerr
//#define DEBUG_PRINT

#include <slivr/VideoCardInfo.h>
#include <slivr/ShaderProgramARB.h>
#include <slivr/CM2Widget.h>

// Convert the float data into a uchar nrrd until I figure out why float
// nrrds don't create a picture. They instead create a constant colored 
// blob.
#define CREATE_UCHAR_NRRD

// When we resample to a regular grid, we add NO_DATA values for regions that
// were not covered by the original volume. In order to make these void areas
// void by default, we reserve the zeroeth entry in the colormap for the void
// values -- a totally transparent color.
#define TAKE_ENTRY_ZERO_FOR_NO_DATA
#define NO_DATA -1e+37


// Static
bool avtOpenGLSLIVRVolumeRenderer::slivrInit = false;

// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::avtOpenGLSLIVRVolumeRenderer
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 17 18:01:43 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtOpenGLSLIVRVolumeRenderer::avtOpenGLSLIVRVolumeRenderer() : oldAtts()
{
    context = 0;
}

// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::~avtOpenGLSLIVRVolumeRenderer
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 17 18:02:04 PST 2007
//
// Modifications:
//   
// ****************************************************************************

avtOpenGLSLIVRVolumeRenderer::~avtOpenGLSLIVRVolumeRenderer()
{
    FreeContext();
}

// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::FreeContext
//
// Purpose: 
//   Frees the SlivrContext that we keep around.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 17 18:30:01 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLSLIVRVolumeRenderer::FreeContext()
{
    if(context != 0)
    {
        debug5 << "avtOpenGLSLIVRVolumeRenderer::FreeContext" << endl;
        delete context;
    }
    context = 0;
}

#ifdef DEBUG_PRINT
// ****************************************************************************
// Function: PrintAxisInfo
//
// Purpose: 
//   Prints nrrd axis info
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 10 16:18:23 PST 2008
//
// Modifications:
//   
// ****************************************************************************

static void
PrintAxisInfo(int axis, const NrrdAxisInfo &info)
{
  printf("Axis %d:\n"
         "\tsize=%d\n"
         "\tspacing=%g\n"
         "\tthickness=%g\n"
         "\tmin=%g\n"
         "\tmax=%g\n"
         "\tcenter=%d\n"
         "\tkind=%d\n"
         "\tlabel=%s\n"
         "\tunits=%s\n", 
      axis, info.size, info.spacing, info.thickness, info.min, info.max, 
      info.center, info.kind, info.label, info.units);
}
#endif

// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::CreateColormap
//
// Purpose: 
//   Creates a SLIVR::ColorMap from the VisIt transfer function definition.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 20 13:40:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLSLIVRVolumeRenderer::CreateColormap(const VolumeAttributes &atts, bool &cmap2D)
{
    // Blow away the 1D transfer function
    if(context->cm != 0)
    {
#ifdef DEBUG_PRINT
        cout << "Delete cm" << endl;
#endif
        delete context->cm;
        context->cm = 0;
    }
    // Blow away the 2D transfer function
    for(int c = 0; c < context->cmap2.size(); ++c)
    {
        vector<SLIVR::CM2Widget*> &widgets = context->cmap2[c]->widgets();
        for(int w = 0; w < widgets.size(); ++w)
        {
#ifdef DEBUG_PRINT
            cout << "Deleting widget " << w << endl;
#endif
            delete widgets[w];
        }
#ifdef DEBUG_PRINT
        cout << "Deleting colormap " << c << endl;
#endif
        delete context->cmap2[c];
    }
#ifdef DEBUG_PRINT
    cout << "Cleared cmap2 " << endl;
#endif
    context->cmap2.clear();

    const float opacityScaling = 0.002;

    // Look in the atts. Does the cmap look 2D?
    cmap2D = atts.GetOpacityVariable() != "default"; // || atts.DoMultivariate()
    if(cmap2D)
    {
        // We're doing a 2D transfer function so set it up.
        if(atts.GetOpacityVariable() != "default")
        {
            // Single variable but we need to make a 2D transfer function.

            // Create a float nrrd for the transfer function.
            unsigned char rgba[256*4];
            atts.GetTransferFunction(rgba);
            float *rgba_f = new float[256*256*4];
            float *fptr = rgba_f;
            for(int j = 0; j < 256; ++j)
            {
                const unsigned char *cptr = rgba;
                float opacity = (float(cptr[j*4+3]) / 255.f) * 
                      atts.GetOpacityAttenuation() * opacityScaling;
                for(int i = 0; i < 256; ++i)
                {
                    *fptr++ = (float(*cptr++) / 255.f);
                    *fptr++ = (float(*cptr++) / 255.f);
                    *fptr++ = (float(*cptr++) / 255.f);
                    *fptr++ = opacity;
                }
            }
            size_t size[3] = {4, 256, 256};
            context->cmap2_image = nrrdNew();
            nrrdWrap_nva(context->cmap2_image, rgba_f, nrrdTypeFloat, 
                3, size);

            // Make a ColorMap2 from the nrrd.
            SLIVR::ColorMap2 *c = new SLIVR::ColorMap2;
            c->widgets().push_back(new SLIVR::ImageCM2Widget(context->cmap2_image));

            context->cmap2.push_back(c);
        }
        else
        {
            // Multivariate, complex cmap2 with widgets
        }
    }
    else
    {
        unsigned char rgba[256*4];
        atts.GetTransferFunction(rgba);
        float rgba_f[256*4];
        const unsigned char *cptr = rgba;
        float *fptr = rgba_f;
        for(int i = 0; i < 256; ++i)
        {
            *fptr++ = (float(*cptr++) / 255.f);
            *fptr++ = (float(*cptr++) / 255.f);
            *fptr++ = (float(*cptr++) / 255.f);
            *fptr++ = (float(*cptr++) / 255.f) * 
                      atts.GetOpacityAttenuation() * opacityScaling;
        }
#ifdef TAKE_ENTRY_ZERO_FOR_NO_DATA
        rgba_f[0] = 0.;
        rgba_f[1] = 0.;
        rgba_f[2] = 0.;
        rgba_f[3] = 0.;
#endif
        context->cm = new SLIVR::ColorMap(rgba_f);
    }
}

// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::OnlyLightingFlagIsDifferent
//
// Purpose: 
//   Determines whether only the lighting flag is different.   
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 10 16:13:18 PST 2008
//
// Modifications:
//   
// ****************************************************************************

bool
avtOpenGLSLIVRVolumeRenderer::OnlyLightingFlagIsDifferent(
    const VolumeAttributes &lhs, const VolumeAttributes &rhs) const
{
    VolumeAttributes compareAtts(lhs);
    compareAtts.SetLightingFlag(rhs.GetLightingFlag());
    // If the attributes only differ in the lighting flag, return true.
    return (lhs != rhs && compareAtts == rhs);
}


// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::Render
//
// Purpose: 
//   Render the dataset
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 17 18:04:07 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtOpenGLSLIVRVolumeRenderer::Render(vtkRectilinearGrid *grid,
                                     vtkDataArray *data,
                                     vtkDataArray *opac,
                                     const avtViewInfo &view,
                                     const VolumeAttributes &atts,
                                     float vmin, float vmax, float vsize,
                                     float omin, float omax, float osize,
                                     float *gx, float *gy, float *gz,
                                     float *gmn, bool reducedDetail)
{
    const char *mName = "avtOpenGLSLIVRVolumeRenderer::Render: ";

    // Initializes SLIVR shaders and GLEW.
    if(!slivrInit)
    {
        debug5 << mName << "Initializing SLIVR" << endl;
        SLIVR::ShaderProgramARB::init_shaders_supported();
        slivrInit = true;
    }

    // Get the sampling rate that the renderer will use.
    float samplingRate = atts.GetRendererSamples();
    if(samplingRate < 1.f)
        samplingRate = 1.f;
    else if(samplingRate > 20.f) // 20 taking from show_volume example program
        samplingRate = 20.f;

    // Ignore the legend flag.
    oldAtts.SetLegendFlag(atts.GetLegendFlag());
    // Ignore the renderSamples value
    oldAtts.SetRendererSamples(atts.GetRendererSamples());

    // Change the renderer attributes, or free the context if the changes
    // are too great.
    if(oldAtts != atts)
    {
        if(context != 0)
        {
            for(int i = 0; i < oldAtts.NumAttributes(); ++i)
                 if(!oldAtts.FieldsEqual(i, &atts))
                     debug5 << "Field " << i << " differs" << endl;

            if(oldAtts.AnyNonTransferFunctionMembersAreDifferent(atts))
            {
                if(OnlyLightingFlagIsDifferent(oldAtts, atts))
                {
                    // Only lighting is different.
                    debug5 << mName << "Set the shading flag" << endl;
                    context->renderer->set_shading(atts.GetLightingFlag());
                }
                else
                {
                    // Something besides the transfer function is different so 
                    // blow away the context
                    debug5 << mName << "oldAtts != atts, freeing the context." << endl;
                    FreeContext();
                }
            }
            else // Only transfer function is different
            {
                // The transfer function is different so install a new colormap
                // based on the updated transfer function.
                debug5 << mName << "Installing new transfer function." << endl;
                bool cmap2d = false;
                CreateColormap(atts, cmap2d);
                if(cmap2d)
                    context->renderer->set_colormap2(context->cmap2);
                else
                    context->renderer->set_colormap1(context->cm);
            }
        }
        oldAtts = atts;
    }

    // If the context needs to be created, do so now.
    if(context == 0)
    {
        debug5 << mName << "Creating new context" << endl;
        context = new SlivrContext();
        if(context == 0)
            return;

        bool opacityDifferent = atts.GetOpacityVariable() != "default";

        // Create the nrrd.
        size_t size[4];
        size[0] = grid->GetDimensions()[0];
        size[1] = grid->GetDimensions()[1];
        size[2] = grid->GetDimensions()[2];
        size[3] = opacityDifferent ? 2 : 1;
        debug5 << mName << "data size: " << size[0] << " " << size[1] << " " << size[2] << endl;
        context->data = nrrdNew();

        if(opacityDifferent)
        {
            debug5 << mName << "Creating nrrd for 2 variables." << endl;
            // Create uchar versions. Put var1 first then opac as var2.
            int nvals = size[0]*size[1]*size[2];
            unsigned char *cdata = new unsigned char[nvals * size[3]];
            float *dptr[2];
            dptr[0] = (float *)data->GetVoidPointer(0);
            dptr[1] = (float *)opac->GetVoidPointer(0);
            float d = vmax - vmin;
            float inv_d = 1.f / d;
            unsigned char *ccdata = cdata;
            for(int c = 0; c < 2; ++c)
            {
                float *fdata = dptr[c];
                for(int k = 0; k < nvals; ++k)
                {
                    if(fdata[k] < NO_DATA)
                        ccdata[k] = 0;
                    else
                    {
                        float t = (fdata[k] - vmin) * inv_d;
                        ccdata[k] = (unsigned char)(((int)(t * 254.)) + 1);
                    }
                }
                ccdata += nvals;
            }

            nrrdWrap_nva(context->data, cdata, nrrdTypeUChar, 
                4, size);

            // Store the pointer to the data in the nrrd so we can free it later.
            context->data->ptr = (void*)cdata;
        }
        else
        {
#ifdef CREATE_UCHAR_NRRD
            int nvals = size[0]*size[1]*size[2];
            unsigned char *cdata = new unsigned char[nvals];
            float *fdata = (float *)data->GetVoidPointer(0);
            float d = vmax - vmin;
#ifdef TAKE_ENTRY_ZERO_FOR_NO_DATA
            float inv_d = 1.f / d;
            for(int k = 0; k < nvals; ++k)
            {
                if(fdata[k] < NO_DATA)
                    cdata[k] = 0;
                else
                {
                    float t = (fdata[k] - vmin) * inv_d;
                    cdata[k] = (unsigned char)(((int)(t * 254.)) + 1);
                }
            }
#else
            for(int k = 0; k < nvals; ++k)
            {
                cdata[k] = (unsigned char)(int)((fdata[k] - vmin) * 255. / d);
            }
#endif
            nrrdWrap_nva(context->data, cdata, nrrdTypeUChar, 
                3, size);

            // Store the pointer to the data in the nrrd so we can free it later.
            context->data->ptr = (void*)cdata;
#else
            nrrdWrap_nva(context->data, data->GetVoidPointer(0), nrrdTypeFloat, 
                3, size);
#endif
        }
        // Set the data min, max values.
        context->data->oldMin = vmin;
        context->data->oldMax = vmax;

        // Wrap the gradient magnitude
        context->gm_data = nrrdNew();
        nrrdWrap_nva(context->gm_data, gmn, nrrdTypeFloat, 
            3, size);

        vtkDataArray *x = grid->GetXCoordinates();
        vtkDataArray *y = grid->GetYCoordinates();
        vtkDataArray *z = grid->GetZCoordinates();
        Nrrd *data[2];
        data[0] = context->data;
        data[1] = context->gm_data;
        for(int i = 0; i < 2; ++i)
        {
            // Set spacing.
            data[i]->axis[0].spacing = 1;
            data[i]->axis[1].spacing = 1;
            data[i]->axis[2].spacing = 1;

            // Set the nrrd's axis sizes.
            data[i]->axis[0].min = x->GetTuple1(0);
            data[i]->axis[0].max = x->GetTuple1(x->GetNumberOfTuples()-1);
            data[i]->axis[1].min = y->GetTuple1(0);
            data[i]->axis[1].max = y->GetTuple1(y->GetNumberOfTuples()-1);
            data[i]->axis[2].min = z->GetTuple1(0);
            data[i]->axis[2].max = z->GetTuple1(z->GetNumberOfTuples()-1);
        }

#ifdef DEBUG_PRINT
        debug5 << "\ttype=" << context->data->type << endl
               << "\tdim=" << context->data->dim << endl
               << "\tspace=" << context->data->space << endl
               << "\tspaceDim=" << context->data->spaceDim << endl
               << "\tblockSize=" << context->data->blockSize << endl
               << "\toldMin=" << context->data->oldMin << endl
               << "\toldMax=" << context->data->oldMax << endl;
        debug5 << mName << "xmin=" << context->data->axis[0].min
               << " xmax=" << context->data->axis[0].max << endl;
        debug5 << mName << "ymin=" << context->data->axis[1].min
               << " ymax=" << context->data->axis[1].max << endl;
        debug5 << mName << "zmin=" << context->data->axis[2].min
               << " zmax=" << context->data->axis[2].max << endl;
        debug5 << mName << "Built nrrd" << endl;

        PrintAxisInfo(0, context->data->axis[0]);
        PrintAxisInfo(1, context->data->axis[1]);
        PrintAxisInfo(2, context->data->axis[2]);
#endif
        // Create the texture.
        context->tex = new SLIVR::Texture();
        context->tex->build(context->data, 0, //context->gm_data, 
            vmin, vmax, 
            0., 1., // gmin, gmax -- gradient
            video_card_memory_size());
        debug5 << mName << "Built texture" << endl;

        // Initialize colors using VisIt's transfer function.
        bool cmap2d = false;
        CreateColormap(atts, cmap2d);
        debug5 << mName << "Built colormap" << endl;

        // Create the renderer.
        int vcm = video_card_memory_size();
        debug5 << "video_card_memory_size = " << vcm << endl;
        context->renderer = new SLIVR::VolumeRenderer(context->tex, 
            context->cm, context->cmap2, context->planes, vcm*1024*1024);
        context->renderer->set_sampling_rate(samplingRate);
        context->renderer->set_shading(atts.GetLightingFlag());
//        context->renderer->set_blend_num_bits(32);
//        context->renderer->set_mode(SLIVR::VolumeRenderer::MODE_MIP);
        debug5 << mName << "Built renderer" << endl;
    }

    // Render the context.
    debug5 << mName << "Rendering..." << endl;
    if(reducedDetail)
    {
        if(atts.GetLightingFlag())
             context->renderer->set_shading(false);

        context->renderer->set_sampling_rate(1.);

        context->renderer->draw(false, true);

        context->renderer->set_sampling_rate(samplingRate);

        if(atts.GetLightingFlag())
             context->renderer->set_shading(true);
    }
    else
    {
        context->renderer->set_sampling_rate(samplingRate);
        context->renderer->draw(false, false);
    }
}    

// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::SlivrContext::SlivrContext
//
// Purpose: 
//   Constructor for the SlivrContext class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 10 16:12:05 PST 2008
//
// Modifications:
//   
// ****************************************************************************

avtOpenGLSLIVRVolumeRenderer::SlivrContext::SlivrContext() : cmap2(), planes()
{
    data = 0;
    gm_data = 0;
    tex = 0;
    cm = 0;
    renderer = 0;
    cmap2_image = 0;
}

// ****************************************************************************
// Method: avtOpenGLSLIVRVolumeRenderer::SlivrContext::~SlivrContext
//
// Purpose: 
//   Destructor for the SlivrContext class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 10 16:12:05 PST 2008
//
// Modifications:
//   
// ****************************************************************************

avtOpenGLSLIVRVolumeRenderer::SlivrContext::~SlivrContext()
{
    if(data != 0)
    {
#ifdef CREATE_UCHAR_NRRD
        // Free the data we created as a workaround.
        unsigned char *cdata = (unsigned char *)data->ptr;
        delete [] cdata;
#endif
        nrrdNix(data);
    }
    if(gm_data != 0)
        nrrdNix(gm_data);
    if(tex != 0)
        delete tex;
    if(cm != 0)
        delete cm;
    if(cmap2_image != 0)
        nrrdNuke(cmap2_image);
    if(renderer != 0)
        delete renderer;
}

#endif // HAVE_LIBSLIVR
