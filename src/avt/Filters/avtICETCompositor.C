// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtICETCompositor.C                                //
// ************************************************************************* //

#include <avtICETCompositor.h>
#include <avtParallel.h>

#include <DebugStream.h>

#include <memory>
#include <map>
#include <vector>

const void *avtICETCompositor::s_colorBuffer = nullptr;
int avtICETCompositor::s_nColorChannels = 4;
int avtICETCompositor::s_width = 0;
int avtICETCompositor::s_height = 0;

// ****************************************************************************
//  Method: avtICETCompositor Default Constructor
//
//  Programmer: Kevin Griffin
//  Creation:   March 4, 2021
//
//  Modifications:
//
// ****************************************************************************

avtICETCompositor::avtICETCompositor()
{
    avtICETCompositor(0);
}

// ****************************************************************************
//  Method: avtICETCompositor Constructor
//
//  Arguments:
//      z       The average geometry z-depth
//      bgColor The background color
//
//  Programmer: Kevin Griffin
//  Creation:   March 4, 2021
//
//  Modifications:
//
// ****************************************************************************

avtICETCompositor::avtICETCompositor(float z, float * bgColor) :
  m_zDepth(z)
{
    if( bgColor )
    {
        m_bgColor[0] = bgColor[0];
        m_bgColor[1] = bgColor[1];
        m_bgColor[2] = bgColor[2];
        m_bgColor[3] = 1.0;
    }

    // Initialize IceT
    #if defined(PARALLEL) && defined(HAVE_ICET)
        InitIceT(z);
    #endif
}

// ****************************************************************************
//  Method: Destructor
//
//  Programmer: Kevin Griffin
//  Creation:   March 4, 2021
//
//  Modifications:
//
// ****************************************************************************

avtICETCompositor::~avtICETCompositor()
{
    #if defined(PARALLEL) && defined(HAVE_ICET)
        icetDestroyContext(m_icetContext);
        icetSetContext(m_prevIceTContext);
    #endif
}

#if defined(PARALLEL) && defined(HAVE_ICET)

// ****************************************************************************
//  Method: avtICETCompositor::InitIceT
//
//  Purpose:
//      Initialize the IceT state.
//
//  Arguments:
//      depth       The average geometry z-depth
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

void
avtICETCompositor::InitIceT(float depth)
{
    // Initialize IceT
    m_prevIceTContext = icetGetContext();
    auto icetMPIComm = icetCreateMPICommunicator(VISIT_MPI_COMM);
    m_icetContext = icetCreateContext(icetMPIComm);
    icetSetContext(m_icetContext);
    icetDestroyMPICommunicator(icetMPIComm);

    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);
    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);

    icetEnable(ICET_ORDERED_COMPOSITE);
    icetEnable(ICET_CORRECT_COLORED_BACKGROUND); //TODO: test this out
    // To prevent IceT from changing the perspective matrix when in single tile mode.
    // TOTRY: icetDisable(ICET_FLOATING_VIEWPORT);

    // Image composition strategy
    icetStrategy(ICET_STRATEGY_SEQUENTIAL);
    icetSingleImageStrategy(ICET_SINGLE_IMAGE_STRATEGY_AUTOMATIC);  // Default
    // icetBoundingBoxf() ??
    icetDrawCallback(IceTDrawCallback);

    // IceT Composite Order
    int mpiSize = PAR_Size();
    std::unique_ptr<IceTInt[]> processRanks(new IceTInt[mpiSize]);
    GetProcessRanks(depth, mpiSize, processRanks.get());
    icetCompositeOrder(processRanks.get());

    // TODO: remove diagnostics
    icetDiagnostics(ICET_DIAG_FULL);
}

// ****************************************************************************
//  Method: avtICETCompositor::GetProcessRanks
//
//  Purpose:
//      Get the visibility order of the geometry assoiated with each process.
//
//  Arguments:
//      depth       The average geometry z-depth
//      mpiSize     The number of processors
//      rankOrder   This is an output variable with the processor rank placed
//                  in the position of the array according to its composite
//                  order.
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

void
avtICETCompositor::GetProcessRanks(float depth, const int mpiSize, IceTInt * const rankOrder)
{
    std::vector<float> allDepths(mpiSize);
    MPI_Allgather(&depth, 1, MPI_FLOAT, allDepths.data(), 1, MPI_FLOAT, VISIT_MPI_COMM);

    // Create a sorted map of depth values to processor rank
    int i;
    std::multimap<float,int> depthToRankMap;
    for(i=0; i<mpiSize; i++)
    {
        depthToRankMap.insert(std::pair<float,int>(allDepths[i], i));
    }

    // Place process ranks in order from lowest depth to highest depth in output var
    i = 0;
    for(auto it = depthToRankMap.begin(); it != depthToRankMap.end(); ++it)
    {
        rankOrder[i] = (*it).second;
        ++i;
    }
}

// ****************************************************************************
//  Method: avtICETCompositor::IceTDrawCallback
//
//  Purpose:
//     Drawing callback used by IceT.
//
//  Arguments:
//      projMatrix 16-value array representing a 4x4 transformation of homo-
//                 geneous coordinates. The matricess are stored in column-
//                 major order.
//      modelViewMatrix Same as the projMatrix except it stores values for the
//                      model/view transformation.
//      bgColor Backdrop should be initialized to this value which may be
//              differentthan the background color passed to icetDrawFrame.
//      readBackViewport Contains 4 integers specifying a region of pixels
//                       that IceT will use. The first two specify the lower-
//                       left corner of the region and the next two specify
//                       the width and height of the region.
//      result The resulting image should be rendered or copied into here.
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

void
avtICETCompositor::IceTDrawCallback(const IceTDouble *projMatrix,
                                        const IceTDouble *modelViewMatrix,
                                        const IceTFloat *bgColor,
                                        const IceTInt *readBackViewport,
                                        IceTImage result)
{
    // int width = icetImageGetWidth(result);
    // int height = icetImageGetHeight(result);
    size_t bytesToCopy = s_width * s_height * s_nColorChannels;
    u_char *icetColorBuffer = icetImageGetColorub(result);

    if(s_colorBuffer != nullptr)
    {
        memcpy(icetColorBuffer, s_colorBuffer, bytesToCopy);
    }
    else
    {
        // u_char *opaqueImageColorBuffer = opaqueImage->GetImage().GetRGBBuffer();
        // memcpy(icetColorBuffer, opaqueImageColorBuffer, bytesToCopy);
        memset(icetColorBuffer, 0, bytesToCopy);
    }
}

// ****************************************************************************
//  Method: avtICETCompositor::CompositeIceT
//
//  Purpose:
//      Renders and composites the final image using IceT.
//
//  Arguments:
//      outBuffer   the buffer to copy the composited image into.
//      width       image width
//      height      image height
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

void
avtICETCompositor::CompositeIceT(void * const outBuffer,
                                     const int width,
                                     const int height)
{
    IceTFloat bgColor[4];
    bgColor[0] = m_bgColor[0];
    bgColor[1] = m_bgColor[1];
    bgColor[2] = m_bgColor[2];
    bgColor[3] = m_bgColor[3];

    const IceTDouble identity[16] =
    {
        IceTDouble(1.0), IceTDouble(0.0), IceTDouble(0.0), IceTDouble(0.0),
        IceTDouble(0.0), IceTDouble(1.0), IceTDouble(0.0), IceTDouble(0.0),
        IceTDouble(0.0), IceTDouble(0.0), IceTDouble(1.0), IceTDouble(0.0),
        IceTDouble(0.0), IceTDouble(0.0), IceTDouble(0.0), IceTDouble(1.0)
    };

    // Add Tiles
    icetResetTiles();

    // Create a single image that's accessible from the root process
    int retVal = icetAddTile(0, 0, width, height, 0);
    if(retVal == -1)
    {
        debug5 << "[ICETCompositor] IceT Error - tile could not be created" << std::endl;
    }

    // Size of the image VisItVTK will generate
    icetPhysicalRenderSize(width, height);

    // Composite - All processes in the current IceT context must call
    // this method for it to complete
    IceTImage icetImage = icetDrawFrame(identity, identity, m_bgColor);

    if(PAR_Rank() == 0)
    {
        if(!icetImageIsNull(icetImage))
        {
            u_char *finalImageColorBuffer = (u_char *)outBuffer;
            icetImageCopyColorub(icetImage, finalImageColorBuffer, ICET_IMAGE_COLOR_RGBA_UBYTE);

            // Timing
            double totalTime;    // in seconds
            icetGetDoublev(ICET_RENDER_TIME, &totalTime);
            debug5 << "[ICETCompositor] IceT Render Time = " << totalTime << std::endl;

            icetGetDoublev(ICET_TOTAL_DRAW_TIME, &totalTime);
            debug5 << "[ICETCompositor] IceT Total Draw Time = " << totalTime << std::endl;
        }
        else
        {
            debug5 << "[ICETCompositor] IceT returned NULL image" << std::endl;
        }
    }
}
#endif

// ****************************************************************************
//  Method: avtICETCompositor::Composite
//
//  Purpose:
//      Render and composites the final image
//
//  Arguments:
//      colorBuffer the VisItVTK rendered frame buffer
//      outBuffer   the final image buffer
//      width       final image width
//      height      final image height
//      nChan       number of color channels (e.g. RGBA = 4)
//
//  Programmer: Kevin Griffin
//  Creation:   March 8, 2021
//
// ****************************************************************************

void
avtICETCompositor::Composite(const void *colorBuffer,
                                 void * const outBuffer,
                                 const int width,
                                 const int height,
                                 const int nChan)
{
    s_colorBuffer = colorBuffer;
    s_nColorChannels = nChan;
    s_width = width;
    s_height = height;

    debug5 << "[ICETCompositor] width = " << width << " height = " << height << std::endl;

#if defined(PARALLEL) && defined(HAVE_ICET)
    debug5 << "[ICETCompositor] Info - Compositing with IceT" << std::endl;
    CompositeIceT(outBuffer, height, width);
#elif PARALLEL
    // TODO: custom compositing solution
#else
    debug5 << "[ICETCompositor] Info - Serial Compositing" << std::endl;
    size_t bytesToCopy = width * height * nChan;

    if(colorBuffer != nullptr)
    {
        memcpy(outBuffer, colorBuffer, bytesToCopy);
	// Debug code
        // avtVisItVTKRenderer::WriteArrayToPPM("test", renderedFrameBuffer, screen[0], screen[1]);
    }
    else
    {
        memset(outBuffer, 0, bytesToCopy);
    }
#endif
}
