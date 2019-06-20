/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                             VisWinRendering.C                             //
// ************************************************************************* //

#include <VisWinRendering.h>

#include <vtkCullerCollection.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include <vtkMapper.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyle.h>
#include <vtkToolkits.h>
#include <vtkInformation.h>

#include <RenderingAttributes.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <avtCallback.h>
#include <avtSourceFromImage.h>

#include <ImproperUseException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtk_glew.h>

// We'd do it another way in VTK8
//#define VALUE_IMAGE_RENDERING_PRE_VTK8
#ifdef VALUE_IMAGE_RENDERING_PRE_VTK8
#include <vtkVisItDataSetMapper.h>
#include <vtkProperty.h>
#endif

#ifdef VISIT_OSPRAY
#include <vtkOSPRayRendererNode.h>
#include <vtkOSPRayPass.h>
#include <vtkViewNodeFactory.h>
#include <vtkVisItViewNodeFactory.h>
#endif

#include <limits>
using std::numeric_limits;

static void RemoveCullers(vtkRenderer *);

bool VisWinRendering::stereoEnabled = false;

// ****************************************************************************
//  Method: VisWinRendering constructor
//
//  Arguments:
//      p    A proxy that allows this colleague friend access to the VisWindow.
//      c    The application context that the window should be associated with.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modification:
//
//    Hank Childs, Thu Jul  6 14:06:15 PDT 2000
//    Initialized needsUpdate, realized and set size and location.  Cached 
//    context.  Removed call to SetUpViewport.  Pushed code to create rendering
//    pipeline up to this function.
//
//    Hank Childs, Tue Aug  1 16:24:09 PDT 2000
//    Added initialization of foreground renderer.  Added code to have 
//    transparent renderers instead of "sensitive" setup I had before.
//
//    Hank Childs, Fri Aug  4 15:00:17 PDT 2000
//    Removed code to have renderers be set to specific viewports and changed
//    way the layers were established to meet new interface that doesn't
//    partition the z-buffer.
//
//    Hank Childs, Fri Aug 31 09:18:25 PDT 2001
//    Removed the cullers to allow for small domains.
//
//    Brad Whitlock, Thu Sep 19 14:26:49 PST 2002
//    Initialized some new data members.
//
//    Kathleen Bonnell, Wed Dec  4 17:05:24 PST 2002  
//    Removed numAntialiasingFrames, no longer required. 
//
//    Eric Brugger, Thu Dec 12 13:27:42 PST 2002
//    Initialized some new data members added to for scalable rendering.
//
//    Hank Childs, Fri Oct 17 21:52:51 PDT 2003
//    Set scalable thresholding to be very high.  Another bug ['3922] allows
//    for scalable rendering even when never is set.  The threshold it uses
//    is the one from this module.  So I am setting it to be very high until
//    '3922 is fixed.
//
//    Mark C. Miller, Tue Nov  4 17:01:09 PST 2003
//    Set scalableThreshold to be default value obtiained from
//    RenderingAttributes.
//
//    Jeremy Meredith, Fri Nov 14 11:29:05 PST 2003
//    Added specular properties.
//
//    Hank Childs, Mon May 10 08:27:32 PDT 2004
//    Initialize displayListMode.
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Replaced scalableThreshold member with scalableAutoThreshold
//    Added scalableActivationMode member
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added initialization for 3 most recent rendering times
//
//    Brad Whitlock, Mon Sep 18 11:07:54 PDT 2006
//    Added colorTexturingFlag.
//
//   Dave Pugmire, Tue Aug 24 11:28:02 EDT 2010
//   Added compact domains mode.
//
//   Burlen Loring, Thu Aug 13 10:07:59 PDT 2015
//   Added depth peeling support
//
//   Burlen Loring, Wed Aug 26 12:12:00 PDT 2015
//   Fix use of unitialized member variable
//
//   Burlen Loring, Sun Sep  6 09:03:17 PDT 2015
//   Added option to disable ordered compositing
//
//   Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//   Added optional OSPRay initialization to constructor
// ****************************************************************************

VisWinRendering::VisWinRendering(VisWindowColleagueProxy &p) :
    VisWinColleague(p), background(NULL), foreground(NULL), needsUpdate(false),
    realized(false), antialiasing(false), stereo(false), stereoType(2),
    surfaceRepresentation(0), specularFlag(false),
    specularCoeff(0.6), specularPower(10.0),
    specularColor(ColorAttribute(255,255,255,255)), colorTexturingFlag(true),
    orderComposite(true), depthCompositeThreads(2), depthCompositeBlocking(65536),
    alphaCompositeThreads(2), alphaCompositeBlocking(65536), depthPeeling(false),
    occlusionRatio(0.01), numberOfPeels(32), multiSamples(8), renderInfo(NULL),
    renderInfoData(NULL), renderEvent(NULL), renderEventData(NULL),
    notifyForEachRender(false), inMotion(false),
    minRenderTime(numeric_limits<double>::max()),
    maxRenderTime(-numeric_limits<double>::max()), summedRenderTime(0.0),
    nRenders(0.0), curRenderTimes(), // stereoEnabled(false),
    scalableRendering(false),
    scalableActivationMode(RenderingAttributes::DEFAULT_SCALABLE_AUTO_THRESHOLD),
    scalableAutoThreshold(RenderingAttributes::DEFAULT_SCALABLE_ACTIVATION_MODE),
    compactDomainsActivationMode(RenderingAttributes::DEFAULT_COMPACT_DOMAINS_ACTIVATION_MODE),
    compactDomainsAutoThreshold(RenderingAttributes:: DEFAULT_COMPACT_DOMAINS_AUTO_THRESHOLD),
    setRenderUpdate(true)
{
    canvas = vtkRenderer::New();
    canvas->SetInteractive(1);
    canvas->SetLayer(1);

    background = vtkRenderer::New();
    background->SetInteractive(0);
    background->SetLayer(0);

    foreground = vtkRenderer::New();
    foreground->SetInteractive(0);
    foreground->SetLayer(2);

    RemoveCullers(canvas);
    RemoveCullers(background);
    RemoveCullers(foreground);

    curRenderTimes[0] = curRenderTimes[1] = curRenderTimes[2] = 0.0;

#ifdef VISIT_OSPRAY
    osprayRendering = false;
    ospraySPP = 1;
    osprayAO = 0;
    osprayShadows = false;
    modeIsPerspective = true;
    canvas->SetPass(0);

    osprayPass = vtkOSPRayPass::New();
    vtkViewNodeFactory* factory = osprayPass->GetViewNodeFactory();
    factory->RegisterOverride("vtkDataSetMapper",
                              vtkVisItViewNodeFactory::pd_maker);
    factory->RegisterOverride("vtkPointGlyphMapper",
                              vtkVisItViewNodeFactory::pd_maker);
    factory->RegisterOverride("vtkMultiRepMapper",
                              vtkVisItViewNodeFactory::pd_maker);
    factory->RegisterOverride("vtkMeshPlotMapper",
                              vtkVisItViewNodeFactory::pd_maker);
    factory->RegisterOverride("vtkOpenGLMeshPlotMapper",
                              vtkVisItViewNodeFactory::pd_maker);
    factory->RegisterOverride("vtkVisItCubeAxesActor",
                              vtkVisItViewNodeFactory::cube_axes_act_maker);
    factory->RegisterOverride("vtkVisItAxisActor",
                              vtkVisItViewNodeFactory::axis_act_maker);
#endif
}


// ****************************************************************************
//  Method: VisWinRendering destructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Fri Jul  7 09:06:17 PDT 2000
//    Removed destruction of axis data members.
//
//    Eric Brugger, Mon Aug 13 17:00:32 PDT 2001
//    Moved the deletion of the render window to the end to the end of the
//    routine to avoid a crash.
//
// ****************************************************************************

VisWinRendering::~VisWinRendering()
{
    if (canvas != NULL)
    {
        canvas->Delete();
        canvas = NULL;
    }
    if (background != NULL)
    {
        background->Delete();
        background = NULL;
    }
    if (foreground != NULL)
    {
        foreground->Delete();
        foreground = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinRendering::InitializeRenderWindow
//
//  Purpose:
//      Initializes the render window (this cannot be done in the constructor
//      because we cannot call virtual functions in the constructor).
//
//  Programmer: Hank Childs
//  Creation:   January 29, 2002
//
//  Modifications:
//    Jeremy Meredith, Tue Nov 19 17:12:33 PST 2002
//    Added check of a flag before forcing stereo.
//
//    Tom Fogal, Tue Nov 24 11:24:57 MST 2009
//    Force a render so that we can initialize GLEW.
//
//    Tom Fogal, Tue Dec 15 10:53:21 MST 2009
//    Remove GLEW initialization from here, move it elsewhere.
//
//    Burlen Loring, Thu Oct  8 10:38:18 PDT 2015
//    fix a compiler warning
//
// ****************************************************************************

static void renderEventCallback(vtkObject*, unsigned long, void* clientdata, void* calldata)
{
    (void)calldata;
    VisWinRendering* renWin = (VisWinRendering*)clientdata;
    renWin->InvokeRenderCallback();
}

void
VisWinRendering::InitializeRenderWindow(vtkRenderWindow *renWin)
{
    renWin->SetNumberOfLayers(3);

    renWin->AddRenderer(background);
    renWin->AddRenderer(canvas);
    renWin->AddRenderer(foreground);

    if (stereoEnabled)
    {
        renWin->SetStereoCapableWindow(1);
    }

    vtkSmartPointer<vtkCallbackCommand> command = vtkCallbackCommand::New();
    command->SetCallback(renderEventCallback);
    command->SetClientData(this);

    renWin->AddObserver(vtkCommand::EndEvent, command);

}


// ****************************************************************************
//  Method: VisWinRendering::GetCanvas
//
//  Purpose:
//      Gets the canvas renderer.
//
//  Returns:    The canvas renderer.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  1 16:24:09 PDT 2000
//    Removed logic for which renderer is acting as the canvas.  They are now
//    all layered in the render window.
//
// ****************************************************************************

vtkRenderer *
VisWinRendering::GetCanvas(void)
{
    return canvas;
}
       

// ****************************************************************************
//  Method: VisWinRendering::GetBackground
//
//  Purpose:
//      Gets the background renderer.
//
//  Returns:    The background renderer.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  1 16:24:09 PDT 2000
//    Removed logic for which renderer is acting as the background.  They are
//    now all layered in the render window.
//
// ****************************************************************************

vtkRenderer *
VisWinRendering::GetBackground(void)
{
    return background;
}
       

// ****************************************************************************
//  Method: VisWinRendering::GetForeground
//
//  Purpose:
//      Gets the foreground renderer.
//
//  Returns:    The foreground renderer.
//
//  Programmer: Hank Childs
//  Creation:   July 26, 2000
//
// ****************************************************************************

vtkRenderer *
VisWinRendering::GetForeground(void)
{
    return foreground;
}


// ****************************************************************************
//  Method: VisWinRendering::SetViewport
//
//  Purpose:
//      Sets the viewport.
//
//  Arguments:
//      vl      The left viewport in normalized device coordinates.
//      vb      The bottom viewport in normalized device coordinates.
//      vr      The right viewport in normalized device coordinates.
//      vt      The top viewport in normalized device coordinates.
//
//  Programmer: Hank Childs
//  Creation:   May 4, 2000
//
//  Modifications:
//    Hank Childs, Fri Aug  4 14:51:20 PDT 2000
//    Only change the canvas viewport if we are in 2D mode since the canvas is 
//    also used for 3D mode.
//
//    Eric Brugger, Fri Aug 24 09:15:28 PDT 2001
//    I added a call to compute the aspect ratio after setting the view.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002 
//    Added support for curve mode. 
//
//    Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//    Added new AxisArray window mode.
//
//    Eric Brugger, Tue Dec  9 14:19:59 PST 2008
//    Added the ParallelAxes window mode.
//
//    Eric Brugger, Mon Nov  5 15:32:19 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ****************************************************************************

void
VisWinRendering::SetViewport(double vl, double vb, double vr, double vt)
{
    if (mediator.GetMode() == WINMODE_2D ||
        mediator.GetMode() == WINMODE_CURVE ||
        mediator.GetMode() == WINMODE_AXISARRAY ||
        mediator.GetMode() == WINMODE_PARALLELAXES ||
        mediator.GetMode() == WINMODE_VERTPARALLELAXES)
    {
        canvas->SetViewport(vl, vb, vr, vt);
        canvas->ComputeAspect();
    }
}


// ****************************************************************************
//  Method: VisWindowRendering::SetBackgroundColor
//
//  Purpose:
//      Sets the background color for the renderers.
//
//  Arguments:
//      br      The red component (rgb) of the background.
//      bg      The green component (rgb) of the background.
//      bb      The blue component (rgb) of the background.
//
//  Programmer: Hank Childs
//  Creation:   May 4, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  1 16:24:09 PDT 2000
//    Set the foreground's background color.
//
// ****************************************************************************

void
VisWinRendering::SetBackgroundColor(double br, double bg, double bb)
{
    canvas->SetBackground(br, bg, bb);
    background->SetBackground(br, bg, bb);
    foreground->SetBackground(br, bg, bb);
}

// ****************************************************************************
//  Method: VisWinRendering::EnabledDepthPeeling
//
//  Purpose:
//      Enables VTK's depth peeling for order independent rendering of
//      transparent geometry for the next render
//
//  Arguments:
//
//  Programmer: Burlen Loring
//  Creation:   Wed Aug 12 11:49:37 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void
VisWinRendering::EnableDepthPeeling()
{
    vtkRenderWindow *rwin = GetRenderWindow();

    // save window settings
    multiSamples = rwin->GetMultiSamples();

    // configure window
    rwin->SetAlphaBitPlanes(1);
    rwin->SetMultiSamples(0);

    // configure renderer
    canvas->SetUseDepthPeeling(true);
    canvas->SetMaximumNumberOfPeels(numberOfPeels);
    canvas->SetOcclusionRatio(occlusionRatio);
}

// ****************************************************************************
//  Method: VisWinRendering::DisableDepthPeeling
//
//  Purpose:
//      Disables VTK's depth peeling for order independent rendering of
//      transparent geometry
//
//  Arguments:
//
//  Programmer: Burlen Loring
//  Creation:   Wed Aug 12 11:49:37 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
void
VisWinRendering::DisableDepthPeeling()
{
    // restore window settings
    vtkRenderWindow *rwin = GetRenderWindow();
    rwin->SetAlphaBitPlanes(0);
    rwin->SetMultiSamples(multiSamples);

    // configure renderer
    canvas->SetUseDepthPeeling(false);
}

// ****************************************************************************
//  Method: VisWinRendering::EnabledAlphaChannel
//
//  Purpose:
//      Enables VTK's depth peeling for order independent rendering of
//      transparent geometry for the next render
//
//  Arguments:
//
//  Programmer: Burlen Loring
//  Creation:   Wed Aug 12 11:49:37 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
void
VisWinRendering::EnableAlphaChannel()
{
    vtkRenderWindow *rwin = GetRenderWindow();
    rwin->SetAlphaBitPlanes(1);
}

// ****************************************************************************
//  Method: VisWinRendering::DisableAlphaChannel
//
//  Purpose:
//      Disables VTK's depth peeling for order independent rendering of
//      transparent geometry
//
//  Arguments:
//
//  Programmer: Burlen Loring
//  Creation:   Wed Aug 12 11:49:37 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
void
VisWinRendering::DisableAlphaChannel()
{
    vtkRenderWindow *rwin = GetRenderWindow();
    rwin->SetAlphaBitPlanes(0);
}

// ****************************************************************************
//  Method: VisWinRendering::Start2DMode
//
//  Purpose:
//      Puts the rendering module in 2D mode.  This means that the camera 
//      should have orthographic projection.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Hank Childs, Fri Aug  4 14:51:20 PDT 2000
//    Make the canvas be on a smaller viewport.
//
//    Eric Brugger, Fri Aug 24 09:15:28 PDT 2001
//    I added a call to compute the aspect ratio after setting the view.
//
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::Start2DMode(void)
{
    //
    // The canvas should now be snapped to a smaller viewport.
    //
    double vport[4];
    mediator.GetViewport(vport);
    canvas->SetViewport(vport);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY 
    SetModePerspective(false);
#endif
}


// ****************************************************************************
//  Method: VisWinRendering::Stop2DMode
//
//  Purpose:
//      Takes the rendering module out of 2D mode.  This means that the camera
//      should be put in perspective projection mode if it was in that mode
//      previously.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2000
//
//  Modifications:
//    Hank Childs, Fri Aug  4 14:51:20 PDT 2000
//    Restore the size of the canvas' viewport.
//
//    Eric Brugger, Fri Aug 24 09:15:28 PDT 2001
//    I added a call to compute the aspect ratio after setting the view.
//
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::Stop2DMode(void)
{
    //
    // We made the canvas' viewport when we entered 2D mode.  Make it be the
    // whole screen again.
    //
    canvas->SetViewport(0., 0., 1., 1.);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY 
    SetModePerspective(true);
#endif
}

// ****************************************************************************
//  Method: VisWinRendering::StartCurveMode
//
//  Purpose:
//      Puts the rendering module in Curve mode.  This means that the camera 
//      should have orthographic projection.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2002 
//
//  Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::StartCurveMode(void)
{
    //
    // The canvas should now be snapped to a smaller viewport.
    //
    double vport[4];
    mediator.GetViewport(vport);
    canvas->SetViewport(vport);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY 
    SetModePerspective(false);
#endif
}


// ****************************************************************************
//  Method: VisWinRendering::StopCurveMode
//
//  Purpose:
//      Takes the rendering module out of Curve mode.  This means that the camera
//      should be put in perspective projection mode if it was in that mode
//      previously.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 7, 2002 
//
//  Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::StopCurveMode(void)
{
    //
    // We made the canvas' viewport when we entered Curve mode.  Make it be the
    // whole screen again.
    //
    canvas->SetViewport(0., 0., 1., 1.);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY
    SetModePerspective(true);
#endif
}
     
// ****************************************************************************
//  Method: VisWinRendering::StartAxisArrayMode
//
//  Purpose:
//      Puts the rendering module in AxisArray mode.  This means that the 
//      camera should have orthographic projection.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 30, 2008
//
//  Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::StartAxisArrayMode(void)
{
    //
    // The canvas should now be snapped to a smaller viewport.
    //
    double vport[4];
    mediator.GetViewport(vport);
    canvas->SetViewport(vport);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY 
    SetModePerspective(false);
#endif
}


// ****************************************************************************
//  Method: VisWinRendering::StopAxisArrayMode
//
//  Purpose:
//      Takes the rendering module out of AxisArray mode.  This means that
//      the camera should be put in perspective projection mode if it was in
//      that mode previously.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 30, 2008
//
//  Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::StopAxisArrayMode(void)
{
    //
    // We made the canvas' viewport when we entered 2D mode.  Make it be the
    // whole screen again.
    //
    canvas->SetViewport(0., 0., 1., 1.);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY 
    SetModePerspective(true);
#endif
}

// ****************************************************************************
//  Method: VisWinRendering::StartParallelAxesMode
//
//  Purpose:
//      Puts the rendering module in ParallelAxes mode.  This means that the 
//      camera should have orthographic projection.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::StartParallelAxesMode(void)
{
    //
    // The canvas should now be snapped to a smaller viewport.
    //
    double vport[4];
    mediator.GetViewport(vport);
    canvas->SetViewport(vport);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY 
    SetModePerspective(false);
#endif
}


// ****************************************************************************
//  Method: VisWinRendering::StopParallelAxesMode
//
//  Purpose:
//      Takes the rendering module out of ParallelAxes mode.  This means that
//      the camera should be put in perspective projection mode if it was in
//      that mode previously.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Added state tracking for non-perspective modes needed by OSPRay
//
// ****************************************************************************

void
VisWinRendering::StopParallelAxesMode(void)
{
    //
    // We made the canvas' viewport when we entered 2D mode.  Make it be the
    // whole screen again.
    //
    canvas->SetViewport(0., 0., 1., 1.);
    canvas->ComputeAspect();

#ifdef VISIT_OSPRAY 
    SetModePerspective(true);
#endif
}

// ****************************************************************************
//  Method: VisWinRendering::EnableUpdates
//
//  Purpose:
//      If a render was requested was updating (rendering) was suspended, do
//      that now.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Aug 11 08:08:18 PDT 2004
//    Add timings code.
//
// ****************************************************************************

void
VisWinRendering::EnableUpdates(void)
{
    if (needsUpdate)
    {
        int t1 = visitTimer->StartTimer();
        Render();
        visitTimer->StopTimer(t1, "Time for first render after adding plots to"
                                  " this window.");
        needsUpdate = false;
    }
}


// ****************************************************************************
//  Method: VisWinRendering::Render
//
//  Purpose:
//      Causes the render window to render.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 13:52:19 PDT 2000
//    Added a check to see if updates are enabled and if the window is 
//    realized.
//
//    Eric Brugger, Mon Nov  5 13:48:48 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Wed Sep 18 10:17:04 PDT 2002
//    Do not issue the render if we are in no-win mode.  That only slows things
//    down.  When are saving an image, it already forces a render.
//
//    Brad Whitlock, Thu Sep 19 17:04:58 PST 2002
//    I added code to send the rendering time back to the client.
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added args to Start/Stop timer to force acquisition of timing information
//    even if timings were not enabled on the command-line.
//    Added 3 most recent rendering times to set of times returned
//
//    Hank Childs, Wed Mar  1 10:05:25 PST 2006
//    Look for exceptions that occurred during a Render.
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added logic for an 'extra' render so that we can render the 'in-progress'
//    visual queue while in SR mode.
//
// ****************************************************************************

void
VisWinRendering::Render()
{
    if (realized)
    {
        if (mediator.UpdatesEnabled() && (!avtCallback::GetNowinMode() ||
                                           avtCallback::GetNowinInteractionMode()))
        {
            const bool forceTiming = true;
            const bool timingEnabled = visitTimer->Enabled(); 
            const int timingsIndex = visitTimer->StartTimer(forceTiming);
            double rt = 0.;

            // Do an extra render for 'in progress' visual queue if
            // average time to ender is more than 2 seconds
            if (mediator.IsMakingExternalRenderRequests() &&
                mediator.GetAverageExternalRenderingTime() > 2.0 &&
                !avtCallback::GetNowinMode())
            {
                int w, h;
                double color[3];
                GetSize(w, h);
                mediator.GetForegroundColor(color);
                mediator.DoNextExternalRenderAsVisualQueue(w,h,color);
                RenderRenderWindow();
            }

            avtCallback::ClearRenderingExceptions();
            RenderRenderWindow();
            std::string errorMsg = avtCallback::GetRenderingException();
            if (errorMsg != "")
            {
                EXCEPTION1(VisItException, errorMsg.c_str());
            }

            // Determine the time taken to render the image.
            rt = (double)visitTimer->StopTimer(timingsIndex, "Render one frame", forceTiming);
            if(timingEnabled)
            {
                // VisIt's timer is going so use its return value.
                // Dump the timings to the timings file.
                visitTimer->DumpTimings();
            }

            // Update the render times and call the renderer information callback
            // if we need to.
            summedRenderTime += (rt >= 0.) ? rt : 0.;
            minRenderTime = (rt < minRenderTime) ? rt : minRenderTime;
            maxRenderTime = (rt > maxRenderTime) ? rt : maxRenderTime;
            curRenderTimes[2] = curRenderTimes[1];
            curRenderTimes[1] = curRenderTimes[0];
            curRenderTimes[0] = (rt >= 0.) ? rt : 0.;
            ++nRenders;

            // Call the rendering information callback
            if(notifyForEachRender && !inMotion && renderInfo != 0)
            {
                (*renderInfo)(renderInfoData);
                ResetCounters();
            }
        }
        else
        {
            needsUpdate = true;
        }
    }
}

// ****************************************************************************
// Method: VisWinRendering::ResetCounters
//
// Purpose: 
//   Resets counters used by the renderer to keep track of the fps.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 11:43:14 PDT 2002
//
// Modifications:
//   
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added reseting of 3 most recent rendering times
// ****************************************************************************

void
VisWinRendering::ResetCounters()
{
    nRenders = 0;
    summedRenderTime = 0.;
    maxRenderTime = 0.;
    minRenderTime = 1.e6;
    curRenderTimes[0] = 0.0;
    curRenderTimes[1] = 0.0;
    curRenderTimes[2] = 0.0;
}

// ****************************************************************************
// Method: VisWinRendering::MotionBegin
//
// Purpose: 
//   Resets the rendering time variables.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 19 17:07:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::MotionBegin(void)
{
    ResetCounters();
    inMotion = true;
}

// ****************************************************************************
// Method: VisWinRendering::MotionEnd
//
// Purpose: 
//   Calls the rendering information callback.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 11:46:23 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::MotionEnd(void)
{
    inMotion = false;
    if(renderInfo != 0)
    {
        (*renderInfo)(renderInfoData);
        ResetCounters();
    }

    /// call end to render event..
    InvokeRenderCallback();
}

// ****************************************************************************
//  Method: VisWinRendering::Realize
//
//  Purpose:
//      Realizes the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  1 16:24:09 PDT 2000
//    Added check to make sure it hasn't already been realized.
//
//    Brad Whitlock, Wed Nov 1 15:22:21 PST 2000
//    Added code to show the vtkQtRenderWindow.
//
//    Jeremy Meredith, Thu Sep 13 15:42:06 PDT 2001
//    Added a call to wait for the window manager to grab the window.
//
//    Tom Fogal, Tue Dec 15 10:52:51 MST 2009
//    Move GLEW initialization here, causing initialization to be done
//    a bit more lazily.
//
// ****************************************************************************

void
VisWinRendering::Realize(void)
{
    if (realized == false)
    {
        RealizeRenderWindow();
        realized = true;
    }
}

// ****************************************************************************
// Method: VisWinRendering::RenderRenderWindow
//
// Purpose: 
//   Render the render window.
//
// Arguments:
//
// Returns:    
//
// Note:       We put this into a method call so we can prevent it from getting
//             called in subclasses where sometimes we can't render reliably.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 13 16:04:23 PDT 2013
//
// Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Force-disable shadows when not using OSPRay to prevent a crash caused
//    by VTK getting into a strange state when switching back to GL rendering.
//   
// ****************************************************************************

void
VisWinRendering::RenderRenderWindow(void)
{
#ifdef VISIT_OSPRAY
    if (GetOsprayRendering() && modeIsPerspective)
    {
        canvas->SetPass(osprayPass);
    }
    else
    {
        canvas->SetUseShadows(false);
        canvas->SetPass(0);
    }
#endif

    GetRenderWindow()->Render();
}

// ****************************************************************************
//  Method: VisWinRendering::GetCaptureRegion
//
//  Purpose:
//      Computes the size of the region to capture based in image mode and
//      request to do the viewport only or not
//
//  Programmer: Mark C. Miller
//  Creation:   July 26, 2004 
//
//  Modifications:
//     Jeremy Meredith, Thu Jan 31 14:41:50 EST 2008
//     Added new AxisArray window mode.
//
//     Eric Brugger, Tue Dec  9 14:19:59 PST 2008
//     Added the ParallelAxes window mode.
//
//    Eric Brugger, Mon Nov  5 15:32:19 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ****************************************************************************
void
VisWinRendering::GetCaptureRegion(int& r0, int& c0, int& w, int& h,
    bool doViewportOnly)
{
    vtkRenderWindow *renWin = GetRenderWindow();

    r0 = 0;
    c0 = 0;
    w = renWin->GetSize()[0];
    h = renWin->GetSize()[1];

    if (doViewportOnly)
    {
        bool haveViewport = false;
        double viewPort[4];

        if (mediator.GetMode() == WINMODE_2D)
        {
            VisWindow *vw = mediator;
            avtView2D v = vw->GetView2D();
            v.GetActualViewport(viewPort, w, h);
            haveViewport = true;
        }
        else if (mediator.GetMode() == WINMODE_CURVE)
        {
            VisWindow *vw = mediator;
            avtViewCurve v = vw->GetViewCurve();
            v.GetViewport(viewPort);
            haveViewport = true;
        }
        else if (mediator.GetMode() == WINMODE_AXISARRAY ||
                 mediator.GetMode() == WINMODE_PARALLELAXES ||
                 mediator.GetMode() == WINMODE_VERTPARALLELAXES)
        {
            VisWindow *vw = mediator;
            avtViewAxisArray v = vw->GetViewAxisArray();
            v.GetViewport(viewPort);
            haveViewport = true;
        }

        if (haveViewport)
        {
            int neww = (int) ((viewPort[1] - viewPort[0]) * w + 0.5);
            int newh = (int) ((viewPort[3] - viewPort[2]) * h + 0.5);

            c0 = (int) (viewPort[0] * w + 0.5);
            r0 = (int) (viewPort[2] * h + 0.5);
            w = neww;
            h = newh;
        }
    }
}


// ****************************************************************************
//  Method: VisWinRendering::ScreenRender
//
//  Purpose:
//      Forces the render window to render something.
//
//  Programmer: Tom Fogal (modification of a method by Hank Childs)
//  Creation:   July 24, 2008
//
//  Modifications:
//
//    Mark C. Miller, 06May03
//    Added code to get zbuffer data including setting layers so that
//    zbuffer data is obtained correctly
//
//    Hank Childs, Wed Jun 25 09:30:59 PDT 2003
//    Fix memory leak.
//
//    Hank Childs, Sun Nov 16 16:04:52 PST 2003
//    Fix (another) memory leak -- this time with zbuffer.
//
//    Mark C. Miller, Wed Mar 31 17:47:20 PST 2004
//    Added doViewportOnly arg and code to support getting only the viewport
//
//    Mark C. Miller, Fri Apr  2 09:54:10 PST 2004
//    Fixed problem where used 2D view to control region selection for 
//    window in CURVE mode
//
//    Mark C. Miller, Mon Jul 26 15:08:39 PDT 2004
//    Moved code to compute size and origin of region to capture to
//    GetCaptureRegion. Also, changed code to instead of swapping canvas
//    and foreground layer order to just remove foreground layer and re-add
//    it at the end.
//
//    Chris Wojtan, Wed Jul 21 15:17:52 PDT 2004
//    Created separate passes for opaque and translucent data
//
//    Chris Wojtan, Fri Jul 30 14:37:06 PDT 2004
//    Load data from the first rendering pass in the second rendering pass
//
//    Jeremy Meredith, Thu Oct 21 17:14:42 PDT 2004
//    Fixed the stuffing of rgb/z data into the second pass.  Turned off
//    erasing if we are in the second pass.  Put in a big hack to allow
//    this to work even if we are doing two-pass rendering with OpenGL
//    (which isn't supported right now anyway).
//
//    Hank Childs, Wed Mar  1 11:26:15 PST 2006
//    Add some exception handling.
//
//    Brad Whitlock, Wed Jun 10 12:26:48 PDT 2009
//    Don't use mgl functions unless we have mangled mesa.
//
//    Burlen Loring, Fri Aug 28 13:41:29 PDT 2015
//    Clear the alpha channel, it might be needed for ordered
//    compositing.
//
//    Burlen Loring, Mon Aug 31 07:20:57 PDT 2015
//    add a flag to disable uploading the background rendering
//    because it will interfere with ordered compositing. background
//    is handled by the blending compositer. support upload of
//    both rgb and rgba images.
//
//    Burlen Loring, Mon Aug 31 07:33:52 PDT 2015
//    move throw to the end of the method so that the window is not
//    left in a bad state when an exception occurs.
//
//    Burlen Loring, Fri Sep 11 04:16:19 PDT 2015
//    Disable writes to the depth buffer during upload of
//    pass 1 composited image. clear the alpha channel
//    of the pass 1 image. these are needed for ordered
//    compositing.
//
//    Brad Whitlock, Wed Sep 27 11:43:08 PDT 2017
//    I added imgT and code that lets us render different image types.
//    I'd do these things differently after VTK8 vs now with VTK 6.1 so
//    that's why these things are conditionally compiled. They'll need to
//    be changed to render passes for VTK 8.
//
// ****************************************************************************

void
VisWinRendering::ScreenRender(avtImageType imgT,
    bool doViewportOnly, bool doCanvasZBufferToo,
    bool doOpaque, bool doTranslucent,
    bool disableBackground, avtImage_p input)
{
    avtCallback::ClearRenderingExceptions();

    vtkRenderWindow *renWin = GetRenderWindow();

    // If we want zbuffer for the canvas, we need to take care that
    // the canvas is rendered last. To achieve this, we temporarily
    // remove the foreground renderer.
    bool removeForeground = false;
    if(imgT == LuminanceImage || imgT == ValueImage || doCanvasZBufferToo)
        removeForeground = true;

    // Remove the foreground renderer if needed.
    if (removeForeground)
        renWin->RemoveRenderer(foreground);

    // hide the appropriate geometry here
    if(!doOpaque)
        mediator.SuspendOpaqueGeometry();

    if(!doTranslucent)
        mediator.SuspendTranslucentGeometry();

    // Set region origin/size to be captured
    int r0, c0, w, h;
    GetCaptureRegion(r0, c0, w, h, doViewportOnly);

#ifdef VALUE_IMAGE_RENDERING_PRE_VTK8
    double oldBG[3] = {0., 0., 0.};
    int nActors = 0;
    double *actorColors = NULL;
    bool *actorLighting = NULL;
    double *actorAmbient = NULL;
    double *actorDiffuse = NULL;
    if(imgT == ColorRGBImage || imgT == ColorRGBAImage)
        vtkVisItDataSetMapper::SetRenderingMode(vtkVisItDataSetMapper::RENDERING_MODE_NORMAL);
    else if(imgT == LuminanceImage)
    {
        vtkVisItDataSetMapper::SetRenderingMode(vtkVisItDataSetMapper::RENDERING_MODE_LUMINANCE);
        background->GetBackground(oldBG);
        background->SetBackground(0.,0.,0.);
        // TODO: Turn off gradient background.
        vtkActorCollection *actors = canvas->GetActors();
        nActors = actors->GetNumberOfItems();
        actorColors = new double[nActors * 4];
        int i = 0;
        actors->InitTraversal();
        vtkActor *actor = NULL;
        while((actor = actors->GetNextActor()) != NULL)
        {
            // Save the color and opacity.
            actor->GetProperty()->GetColor(actorColors[4*i],actorColors[4*i+1],actorColors[4*i+2]);
            actorColors[4*i+3] = actor->GetProperty()->GetOpacity();

            // Override the color with white. Make opaque.
            actor->GetProperty()->SetColor(1., 1., 1.);
            actor->GetProperty()->SetOpacity(1.);
            i++;
        }
    }
    else if(imgT == ValueImage)
    {
        vtkVisItDataSetMapper::SetRenderingMode(vtkVisItDataSetMapper::RENDERING_MODE_VALUE);
        background->GetBackground(oldBG);
        background->SetBackground(0.,0.,0.);
        // TODO: Turn off gradient background.

        vtkActorCollection *actors = canvas->GetActors();
        nActors = actors->GetNumberOfItems();
        actorLighting = new bool[nActors];
        actorAmbient = new double[nActors];
        actorDiffuse = new double[nActors];
        int i = 0;
        actors->InitTraversal();
        vtkActor *actor = NULL;
        while((actor = actors->GetNextActor()) != NULL)
        {
            // Save the lighting.
            actor->GetProperty()->GetLighting();
            actorLighting[i] = actor->GetProperty()->GetOpacity();
            actorAmbient[i] = actor->GetProperty()->GetAmbient();
            actorDiffuse[i] = actor->GetProperty()->GetDiffuse();

            // Override the lighting.
            actor->GetProperty()->SetLighting(false);
            actor->GetProperty()->SetAmbient(1);
            actor->GetProperty()->SetDiffuse(0.);
            i++;
        }
    }
#endif

    // render
    if (input)
    {
        // given an image to upload. we are in the second render pass
        // for translucent geometry
        if (disableBackground)
        {
            // note: clear color must be 0,0,0,0 is set by caller using
            // visit's api because caller also must set the background
            // mode to solid using visit's api
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            // while uploading we need to disable depth write.
            glDepthMask(GL_FALSE);

            // upload the composited image from pass 1 opaque geometry
            // render
            unsigned char *rgbbuf = input->GetImage().GetRGBBuffer();
            int nchan = input->GetImage().GetNumberOfColorChannels();

            if (nchan == 3)
                renWin->SetPixelData(r0,c0,w-1,h-1,rgbbuf, 1);
            else
                renWin->SetRGBACharPixelData(r0,c0,w-1,h-1, rgbbuf, 1);

            glDepthMask(GL_TRUE);
        }

        // upload the compositied depth buffer from opaque render in
        // pass 1
        float *zbuf = input->GetImage().GetZBufferVTKDirect()->GetPointer(0);
        glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
        renWin->SetZbufferData(r0,c0,w-1,h-1,zbuf);
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

        // disable erase, we would lose uploaded depth/color buffer
        renWin->EraseOff();
        RenderRenderWindow();
        renWin->EraseOn();
    }
    else
    {
        // not given any image to upload. just render.
        RenderRenderWindow();
    }

    // If we removed the foreground layer, put it back before we leave
    if (removeForeground)
        renWin->AddRenderer(foreground);

    // return geometry from hidden status
    if(!doOpaque)
        mediator.ResumeOpaqueGeometry();
    if(!doTranslucent)
        mediator.ResumeTranslucentGeometry();

#ifdef VALUE_IMAGE_RENDERING_PRE_VTK8
    // If we changed the background color, restore it.
    if(imgT == LuminanceImage)
    {
        background->SetBackground(oldBG);

        vtkActorCollection *actors = canvas->GetActors();
        int i = 0;
        actors->InitTraversal();
        vtkActor *actor = NULL;
        while((actor = actors->GetNextActor()) != NULL)
        {
            // Restore the color.
            actor->GetProperty()->SetColor(actorColors[4*i],actorColors[4*i+1],actorColors[4*i+2]);
            actor->GetProperty()->SetOpacity(actorColors[4*i+3]);
            i++;
        }
        delete [] actorColors;
    }
    else if(imgT == ValueImage)
    {
        background->SetBackground(oldBG);

        vtkActorCollection *actors = canvas->GetActors();
        int i = 0;
        actors->InitTraversal();
        vtkActor *actor = NULL;
        while((actor = actors->GetNextActor()) != NULL)
        {
            // Restore the lighting.
            actor->GetProperty()->SetLighting(actorLighting[i]);
            actor->GetProperty()->SetAmbient(actorAmbient[i]);
            actor->GetProperty()->SetDiffuse(actorDiffuse[i]);

            i++;
        }
        delete [] actorLighting;
        delete [] actorAmbient;
        delete [] actorDiffuse;
    }
#endif

    std::string errorMsg = avtCallback::GetRenderingException();
    if (!errorMsg.empty())
    {
        EXCEPTION1(VisItException, errorMsg.c_str());
    }
}

// ****************************************************************************
//  Method: VisWinRendering::ScreenReadback
//
//  Purpose:
//      Reads back an image from our render window.
//
//  Returns:    The image on the screen.
//
//  Programmer: Tom Fogal (copy of a method by Hank Childs)
//  Creation:   July 24, 2008
//
//  Modifications:
//
//    Hank Childs, Wed Jun  6 16:17:17 PDT 2001
//    Used new specification in Update.
//
//    Mark C. Miller, 06May03
//    Added code to get zbuffer data including setting layers so that
//    zbuffer data is obtained correctly
//
//    Hank Childs, Thu Jun 19 17:28:50 PDT 2003
//    Stop using window-to-image filter, since it does not play well with
//    the new camera modifications.
//
//    Hank Childs, Wed Jun 25 09:30:59 PDT 2003
//    Fix memory leak.
//
//    Hank Childs, Sun Nov 16 16:04:52 PST 2003
//    Fix (another) memory leak -- this time with zbuffer.
//
//    Mark C. Miller, Tue Feb  3 20:46:20 PST 2004
//    Moved call to delete [] zb to after Update of SourceFromImage
//
//    Mark C. Miller, Wed Mar 31 17:47:20 PST 2004
//    Added doViewportOnly arg and code to support getting only the viewport
//
//    Mark C. Miller, Fri Apr  2 09:54:10 PST 2004
//    Fixed problem where used 2D view to control region selection for 
//    window in CURVE mode
//
//    Mark C. Miller, Mon Jul 26 15:08:39 PDT 2004
//    Moved code to compute size and origin of region to capture to
//    GetCaptureRegion.
//
//    Hank Childs, Thu Aug 28 13:04:44 PDT 2008
//    Read RGBA data instead of RGB.  First and foremost, this bypasses a Mesa
//    bug on Fedora where RGB data is not served up correctly.  Second, by
//    requesting RGBA data, we get onto Mesa's fast track routines for read
//    back.  (Note: timing shows that this read back routine is fairly quick
//    even when not fast tracked, so this isn't a huge result.)
//
//    Hank Childs, Sun Feb 14 16:21:03 CST 2010
//    Put in explicit timing for Z-buffer readback.
//
//    Burlen Loring, Mon Aug 24 14:35:50 PDT 2015
//    Add support for capturing the alpha channel. when alpha channel is
//    requested we can skip a memcpy. zero copy z-buffer. move disable update
//    request up the stack so that render/readback sequences can occur safely
//    without udpates.
//
//    Brad Whitlock, Thu Sep 21 16:01:03 PDT 2017
//    Fix problem with zbuffer read. It wasn't setting the number of tuples.
//
//    Alister Maguire, Tue Jun 18 09:43:21 PDT 2019
//    Handling opengl error within vtk that occurs while retrieving the 
//    zbuffer.  
//
//    Alister Maguire, Thu Jun 20 09:12:24 PDT 2019
//    Disabling zbuffer error catch until we figure out how to better handle
//    it. 
//
// ****************************************************************************

avtImage_p
VisWinRendering::ScreenReadback(
    bool doViewportOnly, bool readZ, bool readAlpha)
{
    // Set region origin/size to be captured
    int r0, c0, w, h;
    GetCaptureRegion(r0, c0, w, h, doViewportOnly);

    vtkRenderWindow *renWin = GetRenderWindow();
    vtkFloatArray *zbuffer = NULL;

    // Read the pixels from the window and copy them over.
    unsigned char *pixels =
        renWin->GetRGBACharPixelData(c0,r0,c0+w-1,r0+h-1,/*front=*/1);
    const int numPix = w*h;

    if (readZ)
    {
        // get zbuffer data for the canvas
        zbuffer = vtkFloatArray::New();
        zbuffer->SetNumberOfComponents(1);
        zbuffer->SetNumberOfValues(numPix);

        int zStatus = renWin->GetZbufferData(c0,r0,c0+w-1,r0+h-1, zbuffer);

        // FIXME: 
        // For some reason, an opengl error frequently occurs when 
        // retrieving the zbuffer. If this happens, it's unclear what 
        // to do. We need to retain the true zbuffer values, but 
        // copying them over create memory errors every time they 
        // are touched. 
        //
        //if (zStatus == VTK_ERROR)
        //{
        //    HOW SHOULD WE HANDLE THIS??
        //}
    }

    vtkImageData *image = NULL;
    if (readAlpha)
    {
        // keeping the alpha channel and we can do zero copy
        vtkUnsignedCharArray *is = vtkUnsignedCharArray::New();
        is->SetNumberOfComponents(4);
        is->SetName("ImageScalars");
        is->SetArray(pixels, 4*numPix, /*keep=*/0, /*use delete []=*/1);

        image = vtkImageData::New();
        image->SetDimensions(w,h,1);
        image->GetPointData()->SetScalars(is);

        is->Delete();
    }
    else
    {
        // in this case we need to make a copy and strip out
        // the alpha channel
        image = avtImageRepresentation::NewImage(w, h);

        unsigned char *img_pix
            = (unsigned char *)image->GetScalarPointer(0, 0, 0);

        for (int j = 0; j < 3; ++j)
        {
            unsigned char *po = img_pix + j;
            unsigned char *pi = pixels + j;
            for (int i = 0; i < numPix; ++i)
                po[3*i] = pi[4*i];
        }

        delete [] pixels;
    }

    avtImage_p output = new avtImage(NULL);
    output->SetImage(avtImageRepresentation(image, zbuffer));

    image->Delete();
    if (zbuffer)
        zbuffer->Delete();

    return output;
}

// ****************************************************************************
// Method: VisWinRendering::BackgroundReadback
//
// Purpose:
//   Reads back the window's background as an avtImage.
//
// Arguments:
//   doViewportOnly : Whether to read for viewport only.
//
// Returns:    An avtImage containing the background.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 14 19:46:53 PDT 2017
//
// Modifications:
//
// ****************************************************************************

avtImage_p
VisWinRendering::BackgroundReadback(bool doViewportOnly)
{
    // temporarily remove canvas and foreground renderers
    vtkRenderWindow *renWin = GetRenderWindow();
    renWin->RemoveRenderer(canvas);
    renWin->RemoveRenderer(foreground);

    // render (background layer only)
    RenderRenderWindow();

    avtImage_p img = ScreenReadback(doViewportOnly, false, false);

    // add canvas and foreground renderers back in
    renWin->AddRenderer(canvas);
    renWin->AddRenderer(foreground);

    return img;
}

// ****************************************************************************
//  Method: VisWinRendering::PostProcessScreenCapture
//
//  Purpose:
//      Does any necessary post-processing on a screen captured image. This
//      is necessary when the engine is doing ALL rendering as in a
//      non-"Screen Capture" (GUI terminology) save window that includes any
//      annotations that are rendered in the foreground layer. Ordinarily,
//      the engine never renders the foreground layer and it can only do so
//      correctly after it has done a parallel image composite. 
//
//  Programmer: Mark C. Miller
//  Creation:   July 26, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct  6 17:50:23 PDT 2004
//    Added args for viewport only and keeping zbuffer
//
//    Hank Childs, Wed Jan 14 17:45:06 CST 2009
//    Beef up debug message in error condition.
//
//    Brad Whitlock, Tue Sep 27 16:03:21 PDT 2011
//    Removed extra call to set pixel data.
//
//    Burlen Loring, Tue Sep 15 10:42:31 PDT 2015
//    Eliminate a memcpy by reading directly into the output
//    image buffer.
//
//    Brad Whitlock, Thu Sep 21 17:17:11 PDT 2017
//    If we get 4 channel data, output 4 channel data.
//
// ****************************************************************************

avtImage_p
VisWinRendering::PostProcessScreenCapture(avtImage_p input,
    bool doViewportOnly, bool keepZBuffer)
{
    // compute size of region we'll be writing back to the frame buffer
    int r0, c0, w, h;
    GetCaptureRegion(r0, c0, w, h, doViewportOnly);

    // confirm image passed in is the correct size
    int iw, ih;
    input->GetSize(&iw, &ih);
    if ((iw != w) || (ih != h))
    {
        debug1 << "Error condition in screen capture save window" << endl
            << "Captured image is " << iw << "x" << ih << endl
            << "But we believe it should be " << w << "x" << h << endl;
        EXCEPTION1(ImproperUseException, "size of image passed for "
            "PostProcessScreenCapture does not match vtkRenderWindow size");
    }

    // temporarily remove canvas and background renderers
    vtkRenderWindow *renWin = GetRenderWindow();
    renWin->RemoveRenderer(canvas);
    renWin->RemoveRenderer(background);

    // set pixel data
    unsigned char *pixels = input->GetImage().GetRGBBuffer();
    int nChannels = input->GetImage().GetNumberOfColorChannels();
    if(nChannels == 4)
        renWin->SetRGBACharPixelData(c0, r0, c0+w-1, r0+h-1, pixels, /*front=*/1);
    else
        renWin->SetPixelData(c0, r0, c0+w-1, r0+h-1, pixels, /*front=*/1);

    // render (foreground layer only)
    RenderRenderWindow();

    // capture the whole image now
    GetCaptureRegion(r0, c0, w, h, false);

    size_t npix = w*h;

    vtkUnsignedCharArray *pix = vtkUnsignedCharArray::New();
    pix->SetNumberOfComponents(nChannels);
    pix->SetNumberOfTuples(npix);
    pix->SetName("ImageScalars");

    if(nChannels == 4)
        renWin->GetRGBACharPixelData(c0,r0,c0+w-1,r0+h-1, /*front=*/1, pix);
    else
        renWin->GetPixelData(c0,r0,c0+w-1,r0+h-1, /*front=*/1, pix);

    // construct the output image
    vtkImageData *im = vtkImageData::New();
    im->SetDimensions(w, h, 1);
    im->GetPointData()->SetScalars(pix);
    pix->Delete();

    avtImage_p output = new avtImage(NULL);

    output->SetImage(avtImageRepresentation(im,
            keepZBuffer ? input->GetImage().GetZBufferVTK() : 0));

    im->Delete();

    // add canvas and background renderers back in
    renWin->AddRenderer(background);
    renWin->AddRenderer(canvas);

    return output;
}

// ****************************************************************************
// Method: VisWinRendering::ScreenCaptureValues
//
// Purpose:
//   Renders the window as a value image and returns the values in an avtImage.
//
// Returns:    A value image.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 25 15:35:27 PDT 2017
//
// Modifications:
//
// ****************************************************************************

//#define SCREEN_CAPTURE_VALUES_DEBUG
#ifdef SCREEN_CAPTURE_VALUES_DEBUG
#include <vtkPNGWriter.h>
#endif

avtImage_p
VisWinRendering::ScreenCaptureValues(bool readZ)
{
#ifdef VALUE_IMAGE_RENDERING_PRE_VTK8
    bool doViewportOnly = false;
    bool doCanvasZBufferToo = true;
    bool doOpaque = 1;
    bool doTranslucent = true;
    bool disableBackground = true;
    avtImage_p input = NULL;

    // Render as a value image. We just install a linear gray lookup 
    // table and render. (For now).
    ScreenRender(ValueImage,
        doViewportOnly, doCanvasZBufferToo,
        doOpaque, doTranslucent,
        disableBackground, input);

    //
    // We have to have custom read-back.
    //
    vtkRenderWindow *renWin = GetRenderWindow();

    // read the pixels.
    int r0, c0, w, h;
    GetCaptureRegion(r0, c0, w, h, false);
    size_t npix = w*h;
    vtkUnsignedCharArray *pix = vtkUnsignedCharArray::New();
    pix->SetNumberOfComponents(3);
    pix->SetNumberOfTuples(npix);
    renWin->GetPixelData(c0,r0,c0+w-1,r0+h-1, /*front=*/1, pix);

#ifdef SCREEN_CAPTURE_VALUES_DEBUG
    vtkImageData *grayImage = vtkImageData::New();
    grayImage->SetDimensions(w,h,1);
    pix->SetName("ImageScalars");
    grayImage->GetPointData()->SetScalars(pix);

    vtkPNGWriter *writer = vtkPNGWriter::New();
    writer->SetFileName("screencapturevalues_gray.png");
    writer->SetInputData(grayImage);
    writer->Write();
    writer->Delete(); 
#endif

    // read z back. We use it to mask.
    vtkFloatArray *zbuffer = vtkFloatArray::New();
    renWin->GetZbufferData(c0,r0,c0+w-1,r0+h-1, zbuffer);

    // Get the minval and scale for the values. 
    double ext[2] = {0., 1.};
    mediator.GetExtents(ext);
    float minval = ext[0];
    float scale = (ext[1] - ext[0]) / 255.f;
//cout << "ScreenCaptureValues: min=" << ext[0] << ", max=" << ext[1] << endl;
    // Create the values. We get the grayscale image and scale the values.
    vtkFloatArray *values = vtkFloatArray::New();
    values->SetName("ImageScalars");
    values->SetNumberOfTuples(npix);
    float *v = (float *)values->GetVoidPointer(0);
    float *z = (float *)zbuffer->GetVoidPointer(0);
    const unsigned char *byteval = (const unsigned char *)pix->GetVoidPointer(0);
    for(int j = 0; j < h; j++)
    {
        for(int i = 0; i < w; ++i)
        {
            if(*z >= 1.f)
                *v = 256.f;
            else
                *v = minval + scale * float(*byteval);
            z++;
            v++;
            byteval += 3;
        }
    }

    // Package it up as an avtImage.
    vtkImageData *image = vtkImageData::New();
    image->SetDimensions(w,h,1);
    image->GetPointData()->SetScalars(values);
    values->Delete();
#ifdef SCREEN_CAPTURE_VALUES_DEBUG
    grayImage->Delete();
#else
    pix->Delete();
#endif

    // If don't need Z, delete it.
    if(!readZ)
    {
        zbuffer->Delete();
        zbuffer = NULL;
    }

    avtImage_p output = new avtImage(NULL);
    output->SetImage(avtImageRepresentation(image, zbuffer));
    image->Delete();
    if (zbuffer)
        zbuffer->Delete();

    return output;
#else
    // VTK 8 has a far superior way of doing this.

    // Remove bg/fg renderers
    // Make a vtkValuePass and add it to the canvas renderer.
    // Force a render
    // Get the float values from the value pass.
    // wrap up the floats as an avtImage.
    // restore bg/fg renderers.
    avtImage_p output = new avtImage(NULL);
    return output;
#endif
}

// ****************************************************************************
//  Method: VisWinRendering::SetSize
//
//  Purpose:
//      Sets the size of the renderable portion of the window.
//
//  Arguments:
//      w       The desired width (in pixels) of the vis window.
//      h       The desired height (in pixels) of the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Aug  1 16:24:09 PDT 2000
//    Added a render to pick up the size change.
//
//    Kathleen Bonnell, Tue Aug 26 09:01:33 PDT 2003 
//    Only set the size if different than what was previously set.  Calling
//    SetSize all the time in ScalableRendering mode causes the rendering
//    Context to be destroyed, invalidating any display lists created. 
//
// ****************************************************************************

void
VisWinRendering::SetSize(int w, int h)
{
    int *size = GetRenderWindow()->GetSize();
    if (size[0] != w || size[1] != h)
    {
        GetRenderWindow()->SetSize(w, h);
        Render();
    }
}

// ****************************************************************************
// Method: VisWinRendering::SetWindowSize
//
// Purpose: 
//      Sets the size of the renderable portion of the window.
//
//  Arguments:
//      w       The desired width (in pixels) of the vis window.
//      h       The desired height (in pixels) of the vis window.
//
// Programmer: Gunther H. Weber
// Creation:   July 15, 2011
//
// ****************************************************************************

void
VisWinRendering::SetWindowSize(int w, int h)
{
   SetSize(w,h);
   Render();
}

// ****************************************************************************
// Method: VisWinRendering::IsDirect
//
// Purpose: 
//   Determines if the vis window is direct connection to the GPU or if it
//   goes through the X-server.
//
// Programmer: Hank Childs
// Creation:   May 9, 2004
//
// ****************************************************************************

bool
VisWinRendering::IsDirect(void)
{
    return (GetRenderWindow()->IsDirect() ? true : false);
}

// ****************************************************************************
// Method: VisWinRendering::GetSize
//
// Purpose: 
//   Returns the size of the renderable portion of the window.
//
// Arguments:
//   w : A reference to an int that is used to return the window width.
//   h : A reference to an int that is used to return the window height.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 11:02:51 PDT 2001
//
// ****************************************************************************

void
VisWinRendering::GetSize(int &w, int &h)
{
    int *size = GetRenderWindow()->GetSize();
    w = size[0];
    h = size[1];
}

// ****************************************************************************
// Method: VisWinRendering::GetWindowSize
//
// Purpose: 
//   Returns the size of the window.
//
// Arguments:
//   w : A reference to an int that is used to return the window width.
//   h : A reference to an int that is used to return the window height.
//
// Programmer: Mark C. Miller
// Creation:   07Jul03 
//   
// ****************************************************************************

void
VisWinRendering::GetWindowSize(int &w, int &h)
{
   GetSize(w,h);
}

// ****************************************************************************
//  Method: VisWinRendering::SetLocation
//
//  Purpose:
//      Sets the location of the window.
//
//  Arguments:
//      x       The desired x-coordinate of the vis window.
//      y       The desired y-coordinate of the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWinRendering::SetLocation(int x, int y)
{
    GetRenderWindow()->SetPosition(x, y);
}

// ****************************************************************************
// Method: VisWinRendering::GetLocation
//
// Purpose: 
//   Returns the location of the window.
//
// Arguments:
//   x : A reference to an int that is used to return the window x location.
//   y : A reference to an int that is used to return the window y location.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 11:02:51 PDT 2001
//   
// ****************************************************************************

void
VisWinRendering::GetLocation(int &x, int &y)
{
    vtkRenderWindow *renWin = GetRenderWindow();
    x = renWin->GetPosition()[0];
    y = renWin->GetPosition()[1];
}


// ****************************************************************************
//  Method: VisWinRendering::SetInteractor
//
//  Purpose:
//      Sets the interactor style.
//
//  Arguments:
//      style   The new interactor style.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    Now that there is lighting colleage, light positions will be
//    managed by vtkRenderer.  Turn off duplicate interactor functionality. 
//
//    Kathleen Bonnell, Tue Feb 11 11:28:03 PST 2003  
//    Test for existence of Interactor before using it. (No interactor
//    in noWin mode.)
//
// ****************************************************************************

void
VisWinRendering::SetInteractor(vtkInteractorStyle *style)
{
    vtkRenderWindowInteractor *iren = GetRenderWindowInteractor();
    if (iren)
    {
        GetRenderWindowInteractor()->SetInteractorStyle(style);
        GetRenderWindowInteractor()->LightFollowCameraOff();
    }
}


// ****************************************************************************
//  Method: VisWinRendering::GetFirstRenderer
//
//  Purpose:
//      Returns the renderer that will render first.  The VisWindow needs to
//      know this so it can alter some of its plots before they are rendered.
//      The background is added first, so it is the first renderer.
//
//  Returns:    The first renderer.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

vtkRenderer *
VisWinRendering::GetFirstRenderer(void)
{
    return background;
}


// ****************************************************************************
//  Method: VisWinRendering::SetTitle
//
//  Purpose:
//      Sets the title of a vis window.
//
//  Arguments:
//      title      The title of the vis window.
//
//  Programmer: Hank Childs
//  Creation:   September 14, 2000
//
// ****************************************************************************

void
VisWinRendering::SetTitle(const char *title)
{
    GetRenderWindow()->SetWindowName((char *)title);
}


// ****************************************************************************
//  Function: RemoveCullers
//
//  Purpose:
//      Removes the cullers automatically added to vtkRenderers.  The offensive
//      culler is vtkFrustumCoverageCuller.  It removes domains that are too
//      small to be seen (this is of course view dependent).
//
//  Arguments:
//      ren     The renderer to remove the cullers from.
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2001
//
// ****************************************************************************

static void 
RemoveCullers(vtkRenderer *ren)
{
    vtkCullerCollection *cullers = ren->GetCullers();
    cullers->InitTraversal();
    vtkCuller *cull = NULL;
    do
    {
        cull = cullers->GetNextItem();
        if (cull != NULL)
        {
            ren->RemoveCuller(cull);
        }
    } while (cull != NULL);
}

// ****************************************************************************
//  Method: VisWinRendering::ComputeVectorTextScaleFactor
//
//  Purpose:
//      Computes a scaling factor to be used with vector text.  The scaling
//      factor is viewport dependednt.
//
//  Arguments:
//      pos     A world-coordinate position to use in the calculation 
//      vp      An alternate viewport (optional).  Components should
//              be in the vtk viewport order: min-x, min_y, max_x, max_y.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002  (Moved and slightly modified from avtPickActor).
//
// ****************************************************************************

double
VisWinRendering::ComputeVectorTextScaleFactor(const double *pos, const double *vp)
{
    double currVP[4];
    if (vp != NULL)
    {
        canvas->GetViewport(currVP);
        //
        // temporarily set vp to that which user specified 
        // 
        canvas->SetViewport(const_cast<double *>(vp));
    }

    //
    //  Convert our world-space position to NormalizedDisplay coordinates.
    //
    double newX = pos[0], newY = pos[1], newZ = pos[2];
    canvas->WorldToView(newX, newY, newZ);
    canvas->ViewToNormalizedViewport(newX, newY, newZ);
    canvas->NormalizedViewportToViewport(newX, newY);
    canvas->ViewportToNormalizedDisplay(newX, newY);

    //
    //  Assuming NormalizedDisplay coordinates run from 0 .. 1 in both
    //  x and y directions, then the normalized dispaly creates a square, whose
    //  diagonal length is sqrt(2) or 1.4142134624.  
    // 
    const double displayDiag = 1.4142135624; 

    // 
    //  We want to find a position P2, that creates a diagonal with the 
    //  original point that is 1/20th of the display diagonal.
    //

    const double target = displayDiag * 0.0125;

    //
    //  Since we are dealing with a right-isosceles-triangle the new position
    //  will be offset in both x and y directions by target * cos(45); 
    //
  
    const double cos45 = 0.7604059656;

    double offset = target * cos45;

    newX += offset;
    newY += offset;

    //
    // Now convert our new position from NormalizedDisplay to World Coordinates.
    //
 
    canvas->NormalizedDisplayToViewport(newX, newY);
    canvas->ViewportToNormalizedViewport(newX, newY);
    canvas->NormalizedViewportToView(newX, newY, newZ);
    canvas->ViewToWorld(newX, newY, newZ);

    //
    // Experimental results, using vtkVectorText and vtkPolyDataMapper, 
    // smallest 'diagonal size' is from the letter 'r' at 0.917883
    // largest  'diagonal size' is from the letter 'W' at 1.78107
    // thus, the average diagonal size of vtkVectorText is:  1.3494765
    // (for alpha text only, in world coordinats, with scale factor of 1.0).  
    // THIS MAY BE A DISPLAY-DEPENDENT RESULT! 
    // 

    const double avgTextDiag = 1.3494765;

    //
    //  Calculate our scale factor, by determining the new target and using
    //  the avgTextDiag to normalize the results across all the pick letters.
    //

    double dxsqr = (newX - pos[0]) * (newX - pos[0]);
    double dysqr = (newY - pos[1]) * (newY - pos[1]);
    double dzsqr = (newZ - pos[2]) * (newZ - pos[2]);
    double worldTarget = sqrt(dxsqr + dysqr + dzsqr); 

    if (vp != NULL)
    {
        //
        // Reset the vp to what it should be. 
        //
        canvas->SetViewport(currVP);
    }
    return (worldTarget / avgTextDiag);
}

// ****************************************************************************
// Method: VisWinRendering::SetRenderInfoCallback
//
// Purpose: 
//   Sets the callback function used to report information back to the client.
//
// Arguments: 
//   callback : The information callback function.
//   data     : Data that is passed to the information callback function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:20:49 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetRenderInfoCallback(void(*callback)(void *), void *data)
{
    renderInfo = callback;
    renderInfoData = data;
}

void
VisWinRendering::SetRenderEventCallback(void(*callback)(void *,bool), void *data)
{
    renderEvent = callback;
    renderEventData = data;
}
// ****************************************************************************
// Method: VisWinRendering::SetAntialiasing
//
// Purpose: 
//   Sets the antialiasing mode.
//
// Arguments:
//   enabled : Whether or not antialiasing is enabled.
//   frames : The number of frames to use.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:21:39 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  4 17:05:24 PST 2002 
//   Remove frames, perform antialiasing via line-smoothing.
//   
// ****************************************************************************

void
VisWinRendering::SetAntialiasing(bool enabled)
{
    if(enabled != antialiasing )
    {
        antialiasing = enabled;
        GetRenderWindow()->SetLineSmoothing(enabled);
    }
}

// ****************************************************************************
// Method: VisWinRendering::GetRenderTimes
//
// Purpose: 
//   Returns the rendering times.
//
// Arguments:
//   times : The return array for the rendering times.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:22:34 PST 2002
//
// Modifications:
//   
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added 3 most recent rendering times to set of times returned
// ****************************************************************************

void
VisWinRendering::GetRenderTimes(double times[6]) const
{
    times[0] = minRenderTime;
    times[1] = (nRenders > 0) ? (summedRenderTime / double(nRenders)) : 0.;
    times[2] = maxRenderTime;
    times[3] = curRenderTimes[0];
    times[4] = curRenderTimes[1];
    times[5] = curRenderTimes[2];
}

// ****************************************************************************
// Method: VisWinRendering::SetStereoRendering
//
// Purpose: 
//   Sets the stereo mode.
//
// Arguments:
//   enabled : Whether or not stereo is enabled.
//   type    : The type of stereo rendering that should be done.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:23:06 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Thu Jun 30 15:29:55 PDT 2005
//   Support red-green stereo type.
//   
//   Hank Childs, Sun Dec  4 18:50:46 PST 2005
//   Issue a warning if the user tried to start stereo without putting
//   "-stereo" on the command line ['4432].
//
//   Mark C. Miller, Sat Jul 22 17:53:43 PDT 2006
//   Added left/right overrides to support stereo SR
//
// ****************************************************************************

void
VisWinRendering::SetStereoRendering(bool enabled, int type)
{
    if (enabled && !stereoEnabled && (type == 2))
    {
        avtCallback::IssueWarning("To use crystal eyes stereo, you need "
           "to re-start VisIt with the \"-stereo\" flag.  VisIt does not "
           "automatically have stereo functionality enabled, because enabling "
           " it can incur severe performance penalties in non-stereo mode.  We"
           " apologize for any inconvience.");
        return;
    }
    if (enabled != stereo || type != stereoType)
    {
        stereo = enabled;
        stereoType = type;
        if(stereo)
        {
            if(stereoType == 0)
                GetRenderWindow()->SetStereoType(VTK_STEREO_RED_BLUE);
            else if(stereoType == 1)
                GetRenderWindow()->SetStereoType(VTK_STEREO_INTERLACED);
            else if(stereoType == 2)
                GetRenderWindow()->SetStereoType(VTK_STEREO_CRYSTAL_EYES);
            else if(stereoType == 4)
                GetRenderWindow()->SetStereoType(VTK_STEREO_LEFT);
            else if(stereoType == 5)
                GetRenderWindow()->SetStereoType(VTK_STEREO_RIGHT);
            else 
            {
                //GetRenderWindow()->SetStereoType(VTK_STEREO_RED_GREEN);
                GetRenderWindow()->SetStereoType(7);
            }
            GetRenderWindow()->SetStereoRender(1);
        }
        else
        {
            GetRenderWindow()->SetStereoRender(0);
        }
    }
}


// ****************************************************************************
// Method: VisWinRendering::SetSurfaceRepresentation
//
// Purpose: 
//   Sets the surface representation.
//
// Arguments:
//   rep : The new surface representation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:26:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetSurfaceRepresentation(int rep)
{
    surfaceRepresentation = rep;
}

// ****************************************************************************
// Method: VisWinRendering::SetSpecularProperties
//
// Purpose: 
//   Sets the specular properties for all of the actors in the canvas
//   renderer.
//
// Arguments:
//   coeff:   the new coefficient
//   power:   the new power
//   color:   the new color
//
// Programmer: Jeremy Meredith
// Creation:   November 14, 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetSpecularProperties(bool flag, double coeff, double power,
                                       const ColorAttribute &color)
{
    specularFlag  = flag;
    specularCoeff = coeff;
    specularPower = power;
    specularColor = color;
}

// ****************************************************************************
// Method: VisWinRendering::SetColorTexturingFlag
//
// Purpose: 
//   Sets the color texturing flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 11:09:39 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetColorTexturingFlag(bool val)
{
    colorTexturingFlag = val;
}

// ****************************************************************************
// Method: VisWinRendering::GetColorTexturingFlag
//
// Purpose: 
//   Returns the color texturing flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 11:09:23 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
VisWinRendering::GetColorTexturingFlag() const
{
    return colorTexturingFlag;
}

// ****************************************************************************
// Method: VisWinRendering::GetNumPrimitives
//
// Purpose: 
//   Counts the number of graphics primitives drawn by the actors in the canvas
//   renderer.
//
// Returns:    A graphics primitive count.
//
// Note:       Polygons in a vtkDataSet, if any, can be rendered in several
//             different ways that affect the actual number of primitives
//             sent to the GPU. We are not using vtkPolyDataMapper because
//             it does not provide such information. We instead have to count
//             cells. I could do a better job at determining a count of the 
//             number of primitives by checking for number of sides on each cell
//             but that would probably be too slow so I'm just using the cell
//             count.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:28:57 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Thu Mar  3 17:38:36 PST 2005
//   Modified to count all types of primitives, not just polygons
//   
// ****************************************************************************

int
VisWinRendering::GetNumPrimitives() const
{
    int sum = 0;

    // Iterate through each actor in the canvas renderer and determine
    // the rough number of triangles that are in it.
    vtkActorCollection *actors = canvas->GetActors();
    actors->InitTraversal();
    vtkActor *actor = NULL;
    do
    {
        actor = actors->GetNextItem();
        if (actor != NULL)
        {
            vtkMapper *m = actor->GetMapper();
            if(m != NULL)
            {
                vtkDataSet *data = m->GetInput();
                if(data != NULL)
                {
                    sum += data->GetNumberOfCells();
                }
            }
        }
    } while (actor != NULL);

    return sum;
}

// ****************************************************************************
// Method: VisWinRendering::GetScalableThreshold
//
// Programmer: Mark C. Miller 
// Creation:   May 11, 2004
//
// ****************************************************************************

int
VisWinRendering::GetScalableThreshold() const
{
    return RenderingAttributes::GetEffectiveScalableThreshold(
        (RenderingAttributes::TriStateMode) scalableActivationMode,
                                            scalableAutoThreshold);
}

// ****************************************************************************
// Method: VisWinRendering::SetScalableRendering
//
// Programmer: Mark C. Miller 
// Creation:   February 5, 2003 
//
// ****************************************************************************

bool
VisWinRendering::SetScalableRendering(bool mode)
{
    bool oldMode = scalableRendering;
    scalableRendering = mode;
    if (scalableRendering)
        mediator.EnableExternalRenderRequests();
    else
        mediator.DisableExternalRenderRequests(true);
    return oldMode;
}

// ****************************************************************************
// Method: VisWinRendering::SetScalableAutoThreshold
//
// Programmer: Mark C. Miller 
// Creation:   May 11, 2004 
//
// ****************************************************************************

int
VisWinRendering::SetScalableAutoThreshold(int autoThreshold)
{
    int oldVal = scalableAutoThreshold;
    scalableAutoThreshold = autoThreshold;
    return oldVal;
}

// ****************************************************************************
// Method: VisWinRendering::SetScalableActivationMode
//
// Programmer: Mark C. Miller 
// Creation:   May 11, 2004
//
// ****************************************************************************

int
VisWinRendering::SetScalableActivationMode(int mode)
{
    int oldVal = scalableActivationMode;
    scalableActivationMode = mode;
    return oldVal;
}

// ****************************************************************************
// Method:  VisWinRendering::SetCompactDomainsActivationMode
//
// Purpose: Set compact domains activation.
//   
//
// Programmer:  Dave Pugmire
// Creation:    August 24, 2010
//
// ****************************************************************************

int
VisWinRendering::SetCompactDomainsActivationMode(int mode)
{
    int oldMode = compactDomainsActivationMode;
    compactDomainsActivationMode = mode;
    return oldMode;
}

// ****************************************************************************
// Method:  VisWinRendering::SetCompactDomainsAutoThreshold
//
// Purpose: Set compact domains threshold.
//   
//
// Programmer:  Dave Pugmire
// Creation:    August 24, 2010
//
// ****************************************************************************

int
VisWinRendering::SetCompactDomainsAutoThreshold(int val)
{
    int oldVal = compactDomainsAutoThreshold;
    compactDomainsAutoThreshold = val;
    return oldVal;
}

void
VisWinRendering::InvokeRenderCallback() {
    if(renderEvent && GetRenderUpdate() == true) {
        renderEvent(renderEventData,inMotion);
    }
}

void
VisWinRendering::UpdateMouseActions(std::string action, double start_dx, double start_dy,
                                    double end_dx, double end_dy,
                                    bool ctrl, bool shift) {

    vtkRenderWindow* win = GetRenderWindow();
    vtkRenderWindowInteractor* iren = GetRenderWindowInteractor();

    if(win == NULL || iren == NULL) {
        return;
    }

    enum MouseButton { LeftButton, MiddleButton, RightButton };

    MouseButton mb = LeftButton;

    if(action.find("Middle") != std::string::npos)
        mb = MiddleButton;
    else if(action.find("Right") != std::string::npos)
        mb = RightButton;

    double width, height;
    int* size = win->GetSize();

    width = (double)size[0];
    height = (double)size[1];

    /// Handle wheel events.
    if(action == "WheelUp") {
        iren->MouseWheelForwardEvent();
    }

    if(action == "WheelDown") {
        iren->MouseWheelBackwardEvent();
    }

    if(action.find("Press") != std::string::npos) {

        iren->SetEventInformationFlipY(width*start_dx, height*start_dy,
                                       ctrl ? 1 : 0, shift ? 1 : 0);

        switch(mb) {
            case MiddleButton: iren->MiddleButtonPressEvent(); break;
            case RightButton: iren->RightButtonPressEvent(); break;
            default: iren->LeftButtonPressEvent();
        };
    }

    if(action.find("Move") != std::string::npos) {

        iren->SetEventInformationFlipY(width*end_dx, height*end_dy,
                                       ctrl ? 1 : 0, shift ? 1 : 0);
        iren->MouseMoveEvent();
    }

    if(action.find("Release") != std::string::npos) {

        iren->SetEventInformationFlipY(width*end_dx, height*end_dy,
                                       ctrl ? 1 : 0, shift ? 1 : 0);

        switch(mb) {
            case MiddleButton: iren->MiddleButtonReleaseEvent(); break;
            case RightButton: iren->RightButtonReleaseEvent(); break;
            default: iren->LeftButtonReleaseEvent();
        };
    }
}

#ifdef VISIT_OSPRAY
// ****************************************************************************
// Method: VisWinRendering::SetModePerspective
//
// Purpose: 
//   Stores rendering mode state information needed by OSPRay
//
// Arguments:
//   enabled : Whether or not we're using a perspective rendering mode
//
// Programmer: Garrett Morrison
// Creation:   Wed 2 May 2018 08:39:06 PM PDT
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetModePerspective(bool modePerspective)
{
    modeIsPerspective = modePerspective;
}


// ****************************************************************************
// Method: VisWinRendering::SetOsprayRendering
//
// Purpose: 
//   Sets the OSPRay rendering flag
//
// Arguments:
//   enabled : Whether or not OSPRay rendering is enabled.
//
// Programmer: Alok Hota
// Creation:   Tue 24 Apr 2018 11:22:05 AM EDT
//
// Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Force-disable shadows when not using OSPRay to prevent a crash caused
//    by VTK getting into a strange state when switching back to GL rendering.
//   
// ****************************************************************************

void
VisWinRendering::SetOsprayRendering(bool enabled)
{
    osprayRendering = enabled;

    if (GetOsprayRendering() && modeIsPerspective)
    {
        canvas->SetPass(osprayPass);
    }
    else
    {
        SetOsprayShadows(false);
        canvas->SetPass(0);
    }
}

// ****************************************************************************
// Method: VisWinRendering::SetOspraySPP
//
// Purpose: 
//   Sets the OSPRay samples per pixel
//
// Arguments:
//   val : The new number of samples per pixel
//
// Programmer: Alok Hota
// Creation:   Tue 24 Apr 2018 11:22:05 AM EDT
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetOspraySPP(int val)
{
    if(val != ospraySPP)
    {
        ospraySPP = val;
        vtkOSPRayRendererNode::SetSamplesPerPixel(val, canvas);
    }
}

// ****************************************************************************
// Method: VisWinRendering::SetOsprayAO
//
// Purpose: 
//   Sets the OSPRay ambient occlusion samples
//
// Arguments:
//   val : the new number of ambient occlusion samples
//
// Programmer: Alok Hota
// Creation:   Tue 24 Apr 2018 11:22:05 AM EDT
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetOsprayAO(int val)
{
    if(val != osprayAO)
    {
        osprayAO = val;
        vtkOSPRayRendererNode::SetAmbientSamples(val, canvas);
    }
}

// ****************************************************************************
// Method: VisWinRendering::SetOsprayShadows
//
// Purpose: 
//   Sets the OSPRay shadows flag
//
// Arguments:
//   enabled : The new shadows boolean flag
//
// Programmer: Alok Hota
// Creation:   Wed 02 May 2018 09:41:20 AM EDT
//
// Modifications:
//    Garrett Morrison, Fri May 11 17:57:47 PDT 2018
//    Force-disable shadows when not using OSPRay to prevent a crash caused
//    by VTK getting into a strange state when switching back to GL rendering.
//   
// ****************************************************************************

void
VisWinRendering::SetOsprayShadows(bool enabled)
{
    osprayShadows = enabled;
    
    if(osprayShadows && modeIsPerspective)
    {
        canvas->SetUseShadows(true);
    }
    else
    {
        canvas->SetUseShadows(false);
    }
}
#endif
