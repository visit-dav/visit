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
//                            IceTNetworkManager.C                           //
// ************************************************************************* //

#include "IceTNetworkManager.h"
#include <avtCallback.h>
#include <avtDebugDumpOptions.h>
#include <avtImage.h>
#include <avtParallel.h>
#include <avtSoftwareShader.h>
#include <avtSourceFromImage.h>
#include <avtTransparencyActor.h>
#include <DebugStream.h>
#include <DataNetwork.h>
#include <Engine.h>
#include <snprintf.h>
#include <StackTimer.h>
#include <UnexpectedValueException.h>
#include <VisWindow.h>

#include <vtkFloatArray.h>

#include <IceTGL.h>
#include <IceTMPI.h>
#include <mpi.h>
#include <vtkImageData.h>

// ****************************************************************************
// Debugging help.

#ifdef DEBUG_ICET
#   define DEBUG_ONLY(x) x
#else
#   define DEBUG_ONLY(x) /* nothing. */
#endif

#define ICET_CHECK_ERROR                                                    \
    do {                                                                    \
        GLenum err = icetGetError();                                        \
        const char *es = NULL;                                              \
        switch(err) {                                                       \
            case ICET_NO_ERROR: es = "no error"; break;                     \
            case ICET_SANITY_CHECK_FAIL: es = "sanity check failed"; break; \
            case ICET_INVALID_ENUM: es = "invalid enum"; break;             \
            case ICET_BAD_CAST: es = "bad cast"; break;                     \
            case ICET_OUT_OF_MEMORY: es = "out of memory"; break;           \
            case ICET_INVALID_OPERATION: es = "invalid operation"; break;   \
            case ICET_INVALID_VALUE: es = "invalid value"; break;           \
        }                                                                   \
        if(err != ICET_NO_ERROR) {                                          \
            debug1 << "[ICET_ERROR] " << es << std::endl;                   \
        }                                                                   \
    } while(0)

#define PR_ICET_MPI                                                        \
    do {                                                                   \
        GLint rank, nproc, ntiles;                                         \
        ICET(icetGetIntegerv(ICET_RANK, &rank));                           \
        ICET(icetGetIntegerv(ICET_NUM_PROCESSES, &nproc));                 \
        ICET(icetGetIntegerv(ICET_NUM_TILES, &ntiles));                    \
        debug1 << "IceT: (rank, procs, tiles): (" << rank << ", " << nproc \
               << ", " << ntiles << ")" << std::endl;                      \
    } while(0)

#ifdef DEBUG_ICET
#   define ICET(stmt)     \
    do {                  \
        stmt;             \
        ICET_CHECK_ERROR; \
    } while(0)
#else
#   define ICET(stmt) stmt
#endif

// ****************************************************************************
//  Method: IceTNetworkManager default constructor
//
//  Programmer: Tom Fogal
//  Creation:   June 17, 2008
//
//  Modifications:
//
//    Tom Fogal, Wed May 18 11:57:34 MDT 2011
//    Initialize 'renderings'.
//
// ****************************************************************************
IceTNetworkManager::IceTNetworkManager(void): NetworkManager(), 
    useIceTRenderGeometry(true), renderings(0), compositeOrder(NULL)
{
    this->comm = icetCreateMPICommunicator(VISIT_MPI_COMM);
    DEBUG_ONLY(ICET_CHECK_ERROR);
    this->context = icetCreateContext(comm);
    DEBUG_ONLY(ICET_CHECK_ERROR);

    ICET(icetGLInitialize());
    ICET(icetStrategy(ICET_STRATEGY_REDUCE));

    DEBUG_ONLY(PR_ICET_MPI);

#if defined(__APPLE__) || defined(_WIN32)
    batonTag = GetUniqueMessageTag();
#else
    batonTag = -1;
#endif
}

// ****************************************************************************
//  Method: IceTNetworkManager destructor
//
//  Programmer: Tom Fogal
//  Creation:   June 17, 2008
//
// ****************************************************************************
IceTNetworkManager::~IceTNetworkManager(void)
{
    ICET(icetDestroyContext(this->context));
    ICET(icetDestroyMPICommunicator(this->comm));
    if(compositeOrder != NULL)
        delete [] compositeOrder;
}

// ****************************************************************************
// Method: ConvertIceTImageToAVTImage
//
// Purpose:
//   Converts the IceTImage into an avtImage.
//
// Arguments:
//   iImage : The icet image (it may be empty)
//   w      : The width of the image.
//   h      : The height of the image.
//   keepZ  : Whether we need Z in the output image.
//   keepA  : Whether we need alpha in the output image.
//
// Returns:    An avtImage.
//
// Note:       Rank 0 gets a real composited image. Other ranks get a blank image.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 20 09:40:22 PDT 2017
//
// Modifications:
//
// ****************************************************************************

avtImage_p
ConvertIceTImageToAVTImage(IceTImage iImage, int w, int h, bool keepZ, bool keepA = false)
{
    const char *mName = "ConvertIceTImageToAVTImage: ";

    int npixels = w*h;
    debug5 << mName << "w=" << w << ", h=" << h
           << ", keepZ=" << keepZ << ", keepA=" << keepA << endl;
    debug5 << mName << "IceTImage: "
           << "rgba_ubyte=" << (icetImageGetColorFormat(iImage) == ICET_IMAGE_COLOR_RGBA_UBYTE)
           << ", z=" << (icetImageGetDepthFormat(iImage) == ICET_IMAGE_DEPTH_FLOAT)
           << endl;

    // Make a new avtImage.
    vtkImageData  *colors = avtImageRepresentation::NewImage(w, h, keepA ? 4 : 3);
    unsigned char *dest_color = (unsigned char *) colors->GetScalarPointer();
    avtImage_p img = new avtImage(NULL);
    img->GetImage().SetImageVTK(colors);
    colors->Delete();

    if(icetImageGetColorFormat(iImage) == ICET_IMAGE_COLOR_RGBA_UBYTE)
    {
        debug5 << mName << "Copying image data." << endl;
        // Copy the image colors into the image.
        const IceTUByte *src_rgba = icetImageGetColorub(iImage);
        if(keepA)
        {
            //memcpy(dest_color, src_rgba, 4 * sizeof(unsigned char) * npixels);
            icetImageCopyColorub(iImage, dest_color, ICET_IMAGE_COLOR_RGBA_UBYTE);
        }
        else
        {
            for (int i = 0 ; i < npixels ; i++)
            {
                *dest_color++ = *src_rgba++;
                *dest_color++ = *src_rgba++;
                *dest_color++ = *src_rgba++;
                src_rgba++; // Skip alpha
            }
        }
    }
    else
    {
        debug5 << mName << "incompatible image format." << endl;
    }

    if(keepZ)
    {
        vtkFloatArray *zbuffer = vtkFloatArray::New();
        zbuffer->SetNumberOfTuples(npixels);
        float *fptr = (float *)zbuffer->GetVoidPointer(0);

        if(icetImageGetDepthFormat(iImage) == ICET_IMAGE_DEPTH_FLOAT)
        {
            debug5 << mName << "Reading back zbuffer data." << endl;

            // Copy the z-buffer [0,1] data into the zbuffer array.
            icetImageCopyDepthf(iImage, (IceTFloat*)zbuffer->GetVoidPointer(0), ICET_IMAGE_DEPTH_FLOAT);
        }
        else
        {
            debug5 << mName << "Filling zbuffer data." << endl;
            for(int i = 0; i < npixels; ++i)
                fptr[i] = 1.f;
        }

        // Stash the zbuffer in the new image.
        img->GetImage().SetZBufferVTK(zbuffer);
        zbuffer->Delete();
    }
    else
    {
        debug5 << mName << "Not reading back zbuffer data" << endl;
    }

    return img;
}

// ****************************************************************************
// Method: IceTNetworkManager::BroadcastFinishedImage
//
// Purpose:
//   Sends a finished avtImage from rank 0 to the other ranks.
//
// Arguments:
//   img : The image to send/receive.
//
// Returns:    
//
// Note:       We need to do this for shadows to work as written.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 14 19:38:56 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
IceTNetworkManager::BroadcastFinishedImage(avtImage_p img, int ncomps, int width, int height) const
{
    // We need to send the image to the other ranks.
    MPI_Bcast(img->GetImage().GetRGBBuffer(), ncomps*width*height, MPI_UNSIGNED_CHAR, 0, VISIT_MPI_COMM);
    MPI_Bcast(img->GetImage().GetZBuffer(), width*height, MPI_FLOAT, 0, VISIT_MPI_COMM);
}

// ****************************************************************************
// Method: IceTNetworkManager::Render
//
// Purpose:
//   Renders the plots in the vis window and returns an image.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       Sets a flag whether IceT can be used and then calls the base
//             class' Render. We get back into IceT in RenderGeometry() if
//             we don't have transparent geometry.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb  1 11:42:28 PST 2018
//
// Modifications:
//
// ****************************************************************************

avtDataObject_p
IceTNetworkManager::Render(
    avtImageType imgT, bool getZBuffer,
    intVector networkIds,
    bool checkThreshold, int annotMode, int windowID,
    bool leftEye,
    int &outImgWidth, int &outImgHeight)
{
    const char *mName = "IceTNetworkManager::Render";
    StackTimer t0(mName);
    avtDataObject_p retval;

    TRY
    {
        // Get the vis window.
        EngineVisWinInfo &viswinInfo = viswinMap[windowID];
        VisWindow *viswin = viswinInfo.viswin;

        // Determine whether we have some transparent plots that do not use
        // the transparency actor.
        bool plotDoingTransparencyOutsideTransparencyActor = false;
        DataNetwork *origWorkingNet = workingNet;
        for(size_t i = 0 ; i < networkIds.size() ; i++)
        {
            workingNet = NULL;
            UseNetwork(networkIds[i]);
            if(this->workingNet->GetPlot()->ManagesOwnTransparency())
            {
                plotDoingTransparencyOutsideTransparencyActor = true;
            }
        }
        workingNet = origWorkingNet;

        // We can't easily figure out a compositing order, which IceT requires
        // in order to properly composite transparent geometry.  Thus if there
        // is some transparency, fallback to our parent implementation.
        avtTransparencyActor* trans = viswin->GetTransparencyActor();
        if (trans->TransparenciesExist() ||
            plotDoingTransparencyOutsideTransparencyActor)
        {
            debug2 << mName << ": Encountered transparency: falling back to old "
                      "SR / compositing routines." << std::endl;

            this->useIceTRenderGeometry = false;
        }
        else
        {
            // True for now. We may later decide to not use IceT once we
            // get to RenderGeometry.
            this->useIceTRenderGeometry = true;
        }

        retval = NetworkManager::Render(imgT, getZBuffer, networkIds,
                     checkThreshold, annotMode, windowID, leftEye,
                     outImgWidth, outImgHeight);
    }
    CATCHALL
    {
    }
    ENDTRY

    return retval;
}

#define TEST_WINDOW_MODE
#ifdef TEST_WINDOW_MODE
// ****************************************************************************
// Method: RevertBasedOnWindowMode
//
// Purpose:
//   Exclude certain window modes from icet compsositing.
//
// Arguments:
//   m : The window mode.
//
// Returns:    
//
// Note:       We currently exclude 2D windows because of 2 problems:
//             1. Label plots don't set Z so labels get clipped at mesh edges.
//             2. tests like operators/cart_proj.py are color summing badly,
//                making a bad image on Linux. It seems to be an IceT bug 
//                where there are colors of full intensity being added. It
//                happens on NETCDF because the basic reader can decompose on
//                the fly and for some reason, it ends up allowing the netcdf
//                mesh in the same spatial location to exist on multiple ranks,
//                so we run into bad color summing.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 14 14:51:14 PST 2018
//
// Modifications:
//
// ****************************************************************************

static bool
RevertBasedOnWindowMode(WINDOW_MODE m)
{
    return (m != WINMODE_3D && m != WINMODE_CURVE && m != WINMODE_PARALLELAXES);
}
#endif

// ****************************************************************************
//  Method: RenderGeometry
//
//  Purpose: Renders the geometry for a scene; this is always the opaque
//           objects, and may or may not include translucent objects (depending
//           on the current multipass rendering settings).
//           We override this method because we can avoid a readback in the
//           one-pass case (we'll read it back from IceT later anyway).
//
//  Programmer: Tom Fogal
//  Creation:   July 26, 2008
//
//  Modifications:
//
//    Tom Fogal, Mon Jul 28 14:57:01 EDT 2008
//    Need to return NULL in the single-pass case!
//
//    Burlen Loring, Tue Sep  1 14:26:30 PDT 2015
//    sync up with network manager(base class) order compositing refactor
//
//    Brad Whitlock, Thu Jan 25 12:24:03 PST 2018
//    Rewrite for IceT 2.x using icetCompositeImage.
//
// ****************************************************************************

avtImage_p
IceTNetworkManager::RenderGeometry()
{
    StackTimer t0("IceTNetworkManager::RenderGeometry");
    if (renderState.transparencyInPass2 || !useIceTRenderGeometry)
    {
        debug5 << "Using NetworkManager::RenderGeometry instead of IceT." << endl;
        return NetworkManager::RenderGeometry();
    }
#ifdef TEST_WINDOW_MODE
    if (RevertBasedOnWindowMode(renderState.window->GetWindowMode()))
    {
        // Right now, some plots where we have no Z-buffer are 
        // acting screwy with IceT and how we're doing alpha compositing.
        // Revert to base class' RenderGeometry method.
        //
        // Examples:
        //  operators/ops_cart.py - color sum overflow in 8-bit?
        //  queries/queriesOverTime.py - shifted results for multicurve
        //                               on Linux.
        debug5 << "Using NetworkManager::RenderGeometry instead of IceT. "
               << "winmode="
               << WINDOW_MODE_ToString(renderState.window->GetWindowMode())
               << endl;
        return NetworkManager::RenderGeometry();
    }
#endif

    avtImage_p output(NULL);
    if (PAR_Size() < 2)
    {
        // If we're returning alpha then don't put the background in.
        bool disableBackground = renderState.getAlpha;

        // don't bother with the compositing code if not in parallel
        renderState.window->ScreenRender(renderState.imageType,
            renderState.viewportedMode,
            /*doZBbuffer=*/true, /*opaque on=*/true,
            /*translucent on=*/renderState.transparencyInPass1,
            disableBackground, /*input image=*/NULL);

        output = renderState.window->ScreenReadBack(renderState.viewportedMode,
            /*read z=*/renderState.getZBuffer, /*read a=*/renderState.getAlpha);

        CallProgressCallback("NetworkManager", "render pass 1", 1, 1);
        CallProgressCallback("NetworkManager", "composite pass 1", 0, 1);
    }
    else
    {
        bool saveImages = avtDebugDumpOptions::DumpEnabled();
        // We want Z compositing under these circumstances, alpha compositing otherwise.
#ifdef TEST_WINDOW_MODE
        bool doZComposite = renderState.window->GetWindowMode() == WINMODE_3D;
#else
        bool doZComposite = renderState.threeD || renderState.needZBufferToCompositeEvenIn2D;
#endif
        // We do not want the background in the pixels if:
        //   a) we are expected to return the alpha channel to the client
        //   b) we have a window that is not 3D (we use alpha compositing)
        //   c) we have a non-solid background that can interfere with compositing.
        bool solidBackground     = renderState.window->GetBackgroundMode() == AnnotationAttributes::Solid;
        bool compositeBackground = (!doZComposite || !solidBackground) && 
                                   !renderState.getAlpha;
        bool disableBackground   = renderState.getAlpha || compositeBackground;

#if defined(__APPLE__) || defined(_WIN32)
        // On these systems, we're using the GPU offscreen for rendering. Let's throttle
        // access to it a little to make rendering/readback faster.
        int nbatons = 3;
        int baton = PAR_Rank();
        if((PAR_Rank() / nbatons) > 0)
        {
            MPI_Status status;
            MPI_Recv(&baton, 1, MPI_INT, PAR_Rank()-nbatons, batonTag, MPI_COMM_WORLD, &status);
        }
#endif
        //
        // Render the geometry.
        //
        {
            StackTimer t1("ScreenRender");
            renderState.window->ScreenRender(renderState.imageType,
                renderState.viewportedMode,
                /*doZBbuffer=*/true, /*opaque on=*/true,
                /*translucent on=*/renderState.transparencyInPass1,
                disableBackground, /*input image=*/NULL);
        }

        //
        // Read back the pixels.
        //
        avtImage_p pixels(NULL);
        {
            StackTimer t2("ScreenReadback");
            bool getZBuffer = doZComposite;
            bool getAlpha = true;
            pixels = renderState.window->ScreenReadBack(renderState.viewportedMode,
                /*read z=*/getZBuffer, /*read a=*/getAlpha);

            if(saveImages)
                this->DumpImage(pixels, "icet-render-geom");
        }
#if defined(__APPLE__) || defined(_WIN32)
        // Throttling. pass the baton.
        if((PAR_Rank() + nbatons) < PAR_Size())
        {
            baton = PAR_Rank();
            MPI_Send(&baton, 1, MPI_INT, PAR_Rank()+nbatons, batonTag, MPI_COMM_WORLD);
        }
#endif

        avtImage_p bgImage(NULL);
        if(PAR_Rank() == 0 && compositeBackground && !solidBackground)
        {
            StackTimer t3("Getting background image");
            bgImage = renderState.window->BackgroundReadback(renderState.viewportedMode);
            if(saveImages)
                this->DumpImage(bgImage, "icet-render-geom-bg");
        }

        // Now, make an IceT image out of it.
        TRY
        {
            // The output image here has width/height backwards for some reason.
            int imgW = 0, imgH = 0;
            pixels->GetSize(&imgW, &imgH);
            debug5 << "GetSize: " << imgW << ", " << imgH << endl;

            // Come up with a range of valid pixels.
            IceTInt valid_pixels_viewport[4];
            valid_pixels_viewport[0] = 0;
            valid_pixels_viewport[1] = 0;
            valid_pixels_viewport[2] = imgW;
            valid_pixels_viewport[3] = imgH;
            ShrinkValidPixelsViewport(pixels, valid_pixels_viewport);

            //
            // IceT Compositing
            //
            IceTImage comp = icetImageNull();
            double bgColor[3];
            {
                StackTimer t3("IceT Compositing");
                memcpy(bgColor, renderState.window->GetBackgroundColor(), 3*sizeof(double));

                ICET(icetResetTiles());
                const GLint this_mpi_rank_gets_an_image = 0;
                ICET(icetAddTile(0, 0, imgW, imgH, this_mpi_rank_gets_an_image));

                ICET(icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE));
                IceTVoid *color_buffer = (IceTVoid *)pixels->GetImage().GetRGBBuffer();

                IceTVoid *depth_buffer = NULL;
                IceTFloat background_color[4];
                if(doZComposite)
                {
                    ICET(icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT));
                    depth_buffer = (IceTVoid *)pixels->GetImage().GetZBuffer();
                    ICET(icetCompositeMode(ICET_COMPOSITE_MODE_Z_BUFFER));
                    ICET(icetDisable(ICET_ORDERED_COMPOSITE));

                    // If we are compositing in the background, use a clear background here.
                    background_color[0] = IceTFloat(compositeBackground ? 0. : bgColor[0]);
                    background_color[1] = IceTFloat(compositeBackground ? 0. : bgColor[1]);
                    background_color[2] = IceTFloat(compositeBackground ? 0. : bgColor[2]);
                    background_color[3] = IceTFloat(0.);
                }
                else
                {
                    ICET(icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE));
                    ICET(icetCompositeMode(ICET_COMPOSITE_MODE_BLEND));
                    ICET(icetEnable(ICET_ORDERED_COMPOSITE));
                    ICET(icetEnable(ICET_CORRECT_COLORED_BACKGROUND));
                    if(compositeOrder == NULL)
                    {
                        int nprocs = PAR_Size();
                        compositeOrder = new int[nprocs];
                        for(int i = 0; i < nprocs; ++i)
                            compositeOrder[i] = i;
                    }
                    ICET(icetCompositeMode(ICET_COMPOSITE_MODE_BLEND));
                    ICET(icetCompositeOrder(compositeOrder));

                    // Force a clear background so colors are summed correctly.
                    background_color[0] = IceTFloat(0.);
                    background_color[1] = IceTFloat(0.);
                    background_color[2] = IceTFloat(0.);
                    background_color[3] = IceTFloat(0.);
                }

                if(renderState.getZBuffer)
                {
                    ICET(icetDisable(ICET_COMPOSITE_ONE_BUFFER));
                }
                else
                {
                    ICET(icetEnable(ICET_COMPOSITE_ONE_BUFFER));
                }
                ICET(icetEnable(ICET_FLOATING_VIEWPORT));

                IceTDouble *modelview_matrix = NULL;
                IceTDouble *projection_matrix = NULL;
                comp = icetCompositeImage(color_buffer,
                                          depth_buffer,
                                          valid_pixels_viewport,
                                          projection_matrix,
                                          modelview_matrix,
                                          background_color);
            }

            if(PAR_Rank() == 0)
            {
                StackTimer t4("IceTImage to avtImage");
                // Set output with the IceT image.
                if(icetImageIsNull(comp))
                {
                    debug5 << "IceT returned a NULL image." << endl;                        
                    return output; // return null image.
                }
                else
                {
                    output = ConvertIceTImageToAVTImage(comp, imgW, imgH,
                                 renderState.getZBuffer,
                                 compositeBackground || renderState.getAlpha);

                    if(saveImages)
                        this->DumpImage(output, "icet-render-geom-composited");
                }
            }

            // Background compositing
            int desiredComponents = renderState.getAlpha ? 4 : 3;
            if(PAR_Rank() == 0 && compositeBackground)
            {
                StackTimer tb("CompositeBackground");

                unsigned char ucbgColor[3];
                ucbgColor[0] = static_cast<unsigned char>(static_cast<int>(bgColor[0]*255.));
                ucbgColor[1] = static_cast<unsigned char>(static_cast<int>(bgColor[1]*255.));
                ucbgColor[2] = static_cast<unsigned char>(static_cast<int>(bgColor[2]*255.));
                if(solidBackground)
                    this->CompositeSolidBackground(output, ucbgColor);
                else
                    this->CompositeBackground(output, bgImage);
                if(saveImages)
                    this->DumpImage(output, "icet-render-geom-composited-bg");

                // If we're here then we composited in the background and we 
                // simply want a 3 channel image.
                this->ConvertChannels4to3(output);

                if(saveImages)
                    this->DumpImage(output, "icet-render-geom-final");
            }

            // If all ranks need the final image, send it.
            if (renderState.allReducePass1)
            {
                StackTimer t5("Image broadcast");
                if(*output == NULL)
                {
                    output = new avtImage(NULL);
                    output->GetImage() = avtImageRepresentation(imgW, imgH, desiredComponents);
                }
                this->BroadcastFinishedImage(output, desiredComponents, imgW, imgH);
            }
        }
        CATCH(VisItException)
        {
        }
        ENDTRY
    }

    this->renderings++;

    return output;
}

// ****************************************************************************
// Method: IceTNetworkManager::ShrinkValidPixelsViewport
//
// Purpose:
//   Shrink the valid pixels viewport by examining the Z-buffer.
//
// Arguments:
//   img : The input image.
//   valid_pixels_viewport : The extents of the valid pixels in the image.
//
// Returns:    
//
// Note:       We just look at the Z-buffer. If plots don't set the Z-buffer
//             then we should be using the entire image for compositing. We
//             could be checking alpha != 0 too.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 26 11:57:32 PST 2018
//
// Modifications:
//
// ****************************************************************************

void
IceTNetworkManager::ShrinkValidPixelsViewport(avtImage_p img, 
    IceTInt valid_pixels_viewport[4]) const
{
    // Contract the viewport some using the Z-buffer. This might be
    // cost-effective at higher scales. It takes like 0.005 sec on 
    // my Mac. All rank produce the same viewport when the bbox annotation
    // is enabled.
    if(img->GetImage().GetZBuffer() != NULL)
    {
        int imgW = valid_pixels_viewport[2];
        int imgH = valid_pixels_viewport[3];

        StackTimer tvpt("ShrinkValidPixelsViewport");
        int xmin = imgW, ymin = imgH, xmax = -1, ymax = -1;
        const float *zptr = img->GetImage().GetZBuffer();
        for(int j = 0; j < imgH; ++j)
        {
            for(int i = 0; i < imgW; ++i)
            {
                if(*zptr++ < 1.f)
                {
                    xmin = std::min(i, xmin);
                    xmax = std::max(i, xmax);
                    ymin = std::min(j, ymin);
                    ymax = std::max(j, ymax);
                }
            }
        }
        if(xmin == imgW)
            xmin = 0;
        if(xmax == -1)
            xmax = imgW-1;
        if(ymin == imgH)
            ymin = 0;
        if(ymax == -1)
            ymax = imgH-1;

        valid_pixels_viewport[0] = xmin;
        valid_pixels_viewport[1] = ymin;
        valid_pixels_viewport[2] = xmax - xmin + 1;
        valid_pixels_viewport[3] = ymax - ymin + 1;
        debug5 << PAR_Rank() << ": viewport: x=" << valid_pixels_viewport[0]
               << ", y=" << valid_pixels_viewport[1]
               << ", w=" << valid_pixels_viewport[2]
               << ", h=" << valid_pixels_viewport[3]
               << endl;
    }
}

inline unsigned char colorblend(float t, unsigned char a, unsigned char b)
{
    float A = static_cast<float>(a);
    float B = static_cast<float>(b);
    float C = A + t * (B - A);
    return static_cast<unsigned char>(static_cast<int>(C));
}

// ****************************************************************************
// Method: IceTNetworkManager::CompositeBackground
//
// Purpose:
//   Replaces background color pixels in the input image with background 
//   color pixels (like a gradient or image background).
//
// Arguments:
//   img     : The image we are changing.
//   bgColor : The background color to add.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 14 19:30:35 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
IceTNetworkManager::CompositeSolidBackground(avtImage_p img, unsigned char bgColor[3]) const
{
    const char *mName = "IceTNetworkManager::CompositeSolidBackground: ";
    StackTimer tb("CompositeSolidBackground");

    if(*img == NULL)
    {
        debug1 << mName << "img == NULL!" << endl;
        return;
    }

    int imw, imh;
    img->GetImage().GetSize(&imh, &imw);
    unsigned char *im_pix = (unsigned char *)img->GetImage().GetRGBBuffer();
    int npixels = imw * imh;
    if(img->GetImage().GetNumberOfColorChannels() == 4)
    {
        debug5 << mName << "Blend 4 component image with alpha." << endl;
        for(int j = 0; j < npixels; ++j)
        {
            if(im_pix[3] == 0)
            {
                // replace with background color.
                im_pix[0] = bgColor[0];
                im_pix[1] = bgColor[1];
                im_pix[2] = bgColor[2];
                im_pix[3] = 255;
            }

            im_pix += 4;
        }
    }
}

// ****************************************************************************
// Method: IceTNetworkManager::CompositeBackground
//
// Purpose:
//   Replaces background color pixels in the input image with background 
//   color pixels (like a gradient or image background).
//
// Arguments:
//   img     : The image we are changing.
//   bgImage : The image that contains the background.
//   bgColor : The color to replace (when we have no alpha or Z).
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 14 19:30:35 PDT 2017
//
// Modifications:
//
// ****************************************************************************

void
IceTNetworkManager::CompositeBackground(avtImage_p img, avtImage_p bgImage) const
{
    const char *mName = "IceTNetworkManager::CompositeBackground: ";
    StackTimer tb("CompositeBackground");

    if(*img == NULL)
    {
        debug1 << mName << "img == NULL!" << endl;
        return;
    }
    if(*bgImage == NULL)
    {
        debug1 << mName << "bgImage == NULL!" << endl;
        return;
    }
    if(img->GetImage().GetNumberOfColorChannels() != 4)
    {
        debug1 << mName << "img ncomps != 4" << endl;
        return;
    }
    if(bgImage->GetImage().GetNumberOfColorChannels() != 3)
    {
        debug1 << mName << "bgImage ncomps != 3" << endl;
        return;
    }

    int bgw, bgh, imw, imh;
    img->GetImage().GetSize(&imh, &imw);
    bgImage->GetImage().GetSize(&bgh, &bgw);
    debug5 << mName << "imw=" << imw << ", imh=" << imh << ", bgw=" << bgw << ", bgh=" << bgh << endl;

    if(bgw == imw && bgh == imh)
    {
        unsigned char *im_pix = (unsigned char *)img->GetImage().GetRGBBuffer();
        const unsigned char *bg_pix = (const unsigned char *)bgImage->GetImage().GetRGBBuffer();
        int npixels = imw * imh;

        debug5 << mName << "Blend 4 component image with alpha." << endl;
        // We have alpha.
        for(int j = 0; j < npixels; ++j)
        {
             if(im_pix[3] == 0)
             {
                 // replace with background color.
                 im_pix[0] = bg_pix[0];
                 im_pix[1] = bg_pix[1];
                 im_pix[2] = bg_pix[2];
                 im_pix[3] = 255;
             }
#if 0
             else if(im_pix[3] < 255)
             {
                 // blend with background color.
                 float t = static_cast<float>(im_pix[3]) * inv_255;
                 im_pix[0] = colorblend(t, im_pix[0], bg_pix[0]);
                 im_pix[1] = colorblend(t, im_pix[1], bg_pix[1]);
                 im_pix[2] = colorblend(t, im_pix[2], bg_pix[2]);
                 im_pix[3] = 255;
             }
#endif
             im_pix += 4;
             bg_pix += 3;
        }
    }
    else
    {
        debug5 << mName << "Do nothing. image sizes do not match." << endl;
    }
}

// ****************************************************************************
// Method: IceTNetworkManager::ConvertChannels4to3
//
// Purpose:
//   Convert from 4 to 3 color channels.
//
// Arguments:
//   img : The 4 channel image to alter.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 26 14:34:33 PST 2018
//
// Modifications:
//
// ****************************************************************************

void
IceTNetworkManager::ConvertChannels4to3(avtImage_p img) const
{
    StackTimer t0("ConvertChannels4to3");

    if(img->GetImage().GetNumberOfColorChannels() == 3)
        return;

    int imw, imh;
    img->GetImage().GetSize(&imh, &imw);
    debug5 << "ConvertChannels4to3: imw=" << imw << ", imh=" << imh << endl;

    vtkImageData *colors = avtImageRepresentation::NewImage(imw, imh, 3);
    unsigned char *dest_color = (unsigned char *)colors->GetScalarPointer(0, 0, 0);
    unsigned char *src_color = (unsigned char *)img->GetImage().GetRGBBuffer();

    for(int j = 0; j < imh; ++j)
    {
        for(int i = 0; i < imw; ++i)
        {
            dest_color[0] = src_color[0];
            dest_color[1] = src_color[1];
            dest_color[2] = src_color[2];

            src_color += 4;
            dest_color += 3;
        }
    }

    // Replace the old pixels.
    img->GetImage().SetImageVTK(colors);
    colors->Delete();
}

// ****************************************************************************
//  Method: StopTimer
//
//  Purpose: Time the IceT rendering process.
//           IceT includes its own timing code that we might consider using at
//           some point ...
//
//  Programmer: Tom Fogal
//  Creation:   July 14, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Jan 15 11:07:53 CST 2009
//    Changed GetSize call to GetCaptureRegion, since that works for 2D.
//
//    Tom Fogal, Fri May 29 20:37:59 MDT 2009
//    Remove an unused variable.
//
//    Burlen Loring, Tue Sep  1 14:26:30 PDT 2015
//    sync up with network manager(base class) order compositing refactor
//
// ****************************************************************************
void
IceTNetworkManager::StopTimer()
{
    char msg[1024];
    VisWindow *viswin = renderState.window;
    int rows,cols, width_start, height_start;
    // This basically gets the width and the height.
    // The distinction is for 2D rendering, where we only want the
    // width and the height of the viewport.
    viswin->GetCaptureRegion(width_start, height_start, rows,cols,
                             renderState.viewportedMode);

    SNPRINTF(msg, 1023, "IceTNM::Render %lld cells %d pixels",
             renderState.cellCountTotal, rows*cols);
    visitTimer->StopTimer(renderState.timer, msg);
    renderState.timer = -1;
}

// ****************************************************************************
//  Method: FormatDebugImage
//
//  Purpose: Figure out a name for our debug image.  This is complicated in the
//           IceT case, because IceT can call our render function as much as it
//           wants.
//
//  Programmer: Tom Fogal
//  Creation:   May 18, 2011
//
// ****************************************************************************
void IceTNetworkManager::FormatDebugImage(char* out, size_t len,
                                          const char* prefix) const
{
  SNPRINTF(out, len, "%s-%03d-%03u", prefix, PAR_Rank(), this->renderings);
}

