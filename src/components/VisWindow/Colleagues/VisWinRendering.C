// ************************************************************************* //
//                             VisWinRendering.C                             //
// ************************************************************************* //

#include <VisWinRendering.h>

#include <vtkCullerCollection.h>
#include <vtkImageData.h>
#include <vtkMapper.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <vtkQtRenderWindow.h>
#include <vtkQtRenderWindowInteractor.h>

#include <RenderingAttributes.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>

#include <avtCallback.h>
#include <avtSourceFromImage.h>

#include <BadWindowModeException.h>
#include <DebugStream.h>
#include <TimingsManager.h>


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
// ****************************************************************************

VisWinRendering::VisWinRendering(VisWindowColleagueProxy &p) 
    : VisWinColleague(p)
{
    needsUpdate                    = false;
    realized                       = false;
    antialiasing                   = false;
    nRenders                       = 0;
    summedRenderTime               = 0.;
    maxRenderTime                  = 0.;
    minRenderTime                  = 1.e6;
    stereo                         = false;
    stereoType                     = 2;
    immediateMode                  = false;
    surfaceRepresentation          = 0;
    renderInfo                     = 0;
    renderInfoData                 = 0;
    notifyForEachRender            = false;
    inMotion                       = false;
    scalableRendering              = false;
    scalableThreshold              = RenderingAttributes::DEFAULT_SCALABLE_THRESHOLD;

    canvas       = vtkRenderer::New();
    canvas->SetInteractive(1);
    canvas->SetLayer(1);

    background   = vtkRenderer::New();
    background->SetInteractive(0);
    background->SetLayer(2);

    foreground   = vtkRenderer::New();
    foreground->SetInteractive(0);
    foreground->SetLayer(0);

    RemoveCullers(canvas);
    RemoveCullers(background);
    RemoveCullers(foreground);
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
// ****************************************************************************
 
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
// ****************************************************************************

void
VisWinRendering::SetViewport(float vl, float vb, float vr, float vt)
{
    if (mediator.GetMode() == WINMODE_2D || mediator.GetMode() == WINMODE_CURVE)
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
VisWinRendering::SetBackgroundColor(float br, float bg, float bb)
{
    canvas->SetBackground(br, bg, bb);
    background->SetBackground(br, bg, bb);
    foreground->SetBackground(br, bg, bb);
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
// ****************************************************************************

void
VisWinRendering::Start2DMode(void)
{
    //
    // The canvas should now be snapped to a smaller viewport.
    //
    float vport[4];
    mediator.GetViewport(vport);
    canvas->SetViewport(vport);
    canvas->ComputeAspect();
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
// ****************************************************************************

void
VisWinRendering::StartCurveMode(void)
{
    //
    // The canvas should now be snapped to a smaller viewport.
    //
    float vport[4];
    mediator.GetViewport(vport);
    canvas->SetViewport(vport);
    canvas->ComputeAspect();
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
// ****************************************************************************

void
VisWinRendering::EnableUpdates(void)
{
    if (needsUpdate)
    {
        Render();
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
// ****************************************************************************

void
VisWinRendering::Render()
{
    int timingsIndex = visitTimer->StartTimer();
    bool timingEnabled = (timingsIndex != -1);
    float rt = 0.;

    if (realized)
    {
        if (mediator.UpdatesEnabled() && !avtCallback::GetNowinMode())
            GetRenderWindow()->Render();
        else
        {
            needsUpdate = true;
        }
    }

    // Determine the time taken to render the image.
    if(timingEnabled)
    {
        // VisIt's timer is going so use its return value.
        rt = (float)visitTimer->StopTimer(timingsIndex, "Render one frame");
        // Dump the timings to the timings file.
        visitTimer->DumpTimings();
    }
    else
    {
        // The timer is not going, use the render time for all of the
        // renderers.
        rt = background->GetLastRenderTimeInSeconds() + 
            canvas->GetLastRenderTimeInSeconds() + 
            foreground->GetLastRenderTimeInSeconds();
    }

    // Update the render times and call the renderer information callback
    // if we need to.
    summedRenderTime += (rt >= 0.) ? rt : 0.;
    ++nRenders;
    minRenderTime = (rt < minRenderTime) ? rt : minRenderTime;
    maxRenderTime = (rt > maxRenderTime) ? rt : maxRenderTime;

    // Call the rendering information callback
    if(notifyForEachRender && !inMotion && renderInfo != 0)
    {
        (*renderInfo)(renderInfoData);
        ResetCounters();
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
// ****************************************************************************

void
VisWinRendering::ResetCounters()
{
    nRenders = 0;
    summedRenderTime = 0.;
    maxRenderTime = 0.;
    minRenderTime = 1.e6;
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
//  Method: VisWinRendering::ScreenCapture
//
//  Purpose:
//      Performs a screen capture and creates an image from the output image.
//
//  Returns:    The image on the screen.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2001
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
// ****************************************************************************

avtImage_p
VisWinRendering::ScreenCapture(bool doCanvasZBufferToo)
{
    float *zb = NULL;
    vtkRenderWindow *renWin = GetRenderWindow();
    bool extRequestMode = false;

    if (doCanvasZBufferToo)
    {
        //
        // If we want zbuffer for the canvas, we need to take care that
        // the canvas is rendered last 
        //
        foreground->SetLayer(1);
        canvas->SetLayer(0);
    }

    //
    // Make sure that the window is up-to-date.
    //
    renWin->Render();
    int w, h;
    w = renWin->GetSize()[0];
    h = renWin->GetSize()[1];

    if (doCanvasZBufferToo)
    {
        // get zbuffer data for the canvas
        zb = renWin->GetZbufferData(0,0,w-1,h-1);

        // temporarily disable external render requests
        extRequestMode = mediator.DisableExternalRenderRequests();
    }

    //
    // Read the pixels from the window and copy them over.  Sadly, it wasn't
    // very easy to avoid copying the buffer.
    //
    int readFrontBuffer = 1;
    unsigned char *pixels = renWin->GetPixelData(0,0,w-1,h-1,readFrontBuffer);

    vtkImageData *image = avtImageRepresentation::NewImage(w, h);
    unsigned char *img_pix = (unsigned char *)image->GetScalarPointer(0, 0, 0); 

    memcpy(img_pix, pixels, 3*w*h);
    delete [] pixels;

    //
    // Force some updates so we can let screenCaptureSource be destructed.
    // The img->Update forces the window to render, so we explicitly 
    // disable external render requests
    //
    avtSourceFromImage screenCaptureSource(image, zb);
    image->Delete();
    avtImage_p img = screenCaptureSource.GetTypedOutput();
    img->Update(screenCaptureSource.GetGeneralPipelineSpecification());
    img->SetSource(NULL);

    //
    // If we swapped the foreground & canvas layers to get the canvas' zbuffer,
    // swap them back before we leave
    //
    if (doCanvasZBufferToo)
    {
       canvas->SetLayer(1);
       foreground->SetLayer(0);
       if (extRequestMode)
          mediator.EnableExternalRenderRequests();
    }

    return img;
}


// ****************************************************************************
//  Method: VisWinRendering::SetSize
//
//  Purpose:
//      Sets the size of the window.
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
// Method: VisWinRendering::GetSize
//
// Purpose: 
//   Returns the size of the renderable portion of thewindow.
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

float
VisWinRendering::ComputeVectorTextScaleFactor(const float *pos, const float *vp)
{
    float currVP[4];
    if (vp != NULL)
    {
        canvas->GetViewport(currVP);
        //
        // temporarily set vp to that which user specified 
        // 
        canvas->SetViewport(const_cast<float *>(vp));
    }

    //
    //  Convert our world-space position to NormalizedDisplay coordinates.
    //
    float newX = pos[0], newY = pos[1], newZ = pos[2];
    canvas->WorldToView(newX, newY, newZ);
    canvas->ViewToNormalizedViewport(newX, newY, newZ);
    canvas->NormalizedViewportToViewport(newX, newY);
    canvas->ViewportToNormalizedDisplay(newX, newY);

    //
    //  Assuming NormalizedDisplay coordinates run from 0 .. 1 in both
    //  x and y directions, then the normalized dispaly creates a square, whose
    //  diagonal length is sqrt(2) or 1.4142134624.  
    // 
    const float displayDiag = 1.4142135624; 

    // 
    //  We want to find a position P2, that creates a diagonal with the 
    //  original point that is 1/20th of the display diagonal.
    //

    const float target = displayDiag * 0.0125;

    //
    //  Since we are dealing with a right-isosceles-triangle the new position
    //  will be offset in both x and y directions by target * cos(45); 
    //
  
    const float cos45 = 0.7604059656;

    float offset = target * cos45;

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

    const float avgTextDiag = 1.3494765;

    //
    //  Calculate our scale factor, by determining the new target and using
    //  the avgTextDiag to normalize the results across all the pick letters.
    //

    float dxsqr = (newX - pos[0]) * (newX - pos[0]);
    float dysqr = (newY - pos[1]) * (newY - pos[1]);
    float dzsqr = (newZ - pos[2]) * (newZ - pos[2]);
    float worldTarget = sqrt(dxsqr + dysqr + dzsqr); 

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
// ****************************************************************************

void
VisWinRendering::GetRenderTimes(float times[3]) const
{
    times[0] = minRenderTime;
    times[1] = (nRenders > 0) ? (summedRenderTime / float(nRenders)) : 0.;
    times[2] = maxRenderTime;
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
//   
// ****************************************************************************

void
VisWinRendering::SetStereoRendering(bool enabled, int type)
{
    if(enabled != stereo || type != stereoType)
    {
        stereo = enabled;
        stereoType = type;
        if(stereo)
        {
            if(stereoType == 0)
                GetRenderWindow()->SetStereoType(VTK_STEREO_RED_BLUE);
            else if(stereoType == 1)
                GetRenderWindow()->SetStereoType(VTK_STEREO_INTERLACED);
            else
                GetRenderWindow()->SetStereoType(VTK_STEREO_CRYSTAL_EYES);
            GetRenderWindow()->SetStereoRender(1);
        }
        else
        {
            GetRenderWindow()->SetStereoRender(0);
        }
    }
}

// ****************************************************************************
// Method: VisWinRendering::SetImmediateModeRendering
//
// Purpose: 
//   Sets the immediate rendering mode for all of the actors in the canvas
//   renderer.
//
// Arguments:
//   mode : The new immediate rendering mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:26:10 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWinRendering::SetImmediateModeRendering(bool mode)
{
    immediateMode = mode;
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
// Method: VisWinRendering::GetNumTriangles
//
// Purpose: 
//   Counts the number of triangles drawn by the actors in the canvas renderer.
//
// Returns:    A triangle count.
//
// Note:       vtkPolyData can be rendered in several different ways that
//             affect the rendered triangle count. We are not using the
//             polydata's mapper because it does not provide such information.
//             We instead have to count cells in the polydata. I could do a
//             better job at determining the triangle count by checking for
//             the number of sides on each cell but that would probably be
//             too slow so I'm just using the cell count.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:28:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
VisWinRendering::GetNumTriangles() const
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
                    // Try and downcast to a polydata object.
                    vtkPolyData *pd = vtkPolyData::SafeDownCast(data);
                    if(pd != NULL)
                    {
                        // This underestimates the number of triangles
                        // if the polydata contains quads.
                        sum += pd->GetNumberOfPolys();
                        sum += pd->GetNumberOfStrips();
                    }
                }
            }
        }
    } while (actor != NULL);

    return sum;
}

// ****************************************************************************
// Method: VisWinRendering::SetScalableRendering
//
// Purpose: 
//   Sets the scalable rendering mode 
//
// Arguments:
//   mode : The new mode
//
// Return:
//   the old mode
//
// Programmer: Mark C. Miller 
// Creation:   February 5, 2003 
//
// ****************************************************************************

bool
VisWinRendering::SetScalableRendering(bool mode)
{  bool oldMode = scalableRendering;
   scalableRendering = mode;
   if (scalableRendering)
      mediator.EnableExternalRenderRequests();
   else
      mediator.DisableExternalRenderRequests();
   return oldMode;
}

// ****************************************************************************
// Method: VisWinRendering::SetScalableThreshold
//
// Purpose: 
//   Sets the scalable rendering threshold
//
// Arguments:
//   threshold: The new threshold
//
// Return:
//   the old threshold
//
// Programmer: Mark C. Miller 
// Creation:   February 5, 2003 
//
// ****************************************************************************

int
VisWinRendering::SetScalableThreshold(int threshold)
{  int oldVal = scalableThreshold;
   scalableThreshold = threshold;
   return oldVal;
}
