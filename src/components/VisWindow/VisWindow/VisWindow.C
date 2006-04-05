// ************************************************************************* //
//                               VisWindow.C                                 //
// ************************************************************************* //

#include <VisWindow.h>

#include <float.h> // for FLOAT_MAX

#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkMapper.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkVisItCellLocator.h>

#include <AnnotationObjectList.h>
#include <ColorAttribute.h>
#include <LineAttributes.h>

#include <avtCallback.h>
#include <avtPlot.h>
#include <avtLightList.h>
#include <VisitInteractor.h>
#include <VisWinAnnotations.h>
#include <VisWinAxes.h>
#include <VisWinAxes3D.h>
#include <VisWinBackground.h>
#include <VisWinFrame.h>
#include <VisWinInteractions.h>
#include <VisWinLegends.h>
#include <VisWinLighting.h>
#include <VisWinPlots.h>
#include <VisWinQuery.h>
#include <VisWinRenderingWithoutWindow.h>
#include <VisWinTools.h>
#include <VisWinTriad.h>
#include <VisWinUserInfo.h>
#include <VisWinView.h>
#include <BadColleagueException.h>
#include <BadWindowModeException.h>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

using std::string;
using std::vector;


//
// Static Function Prototypes
//

static void      start_render(vtkObject *, unsigned long, void*, void *);


// ****************************************************************************
//  Method: VisWindow constructor
//
//  Programmer: Hank Childs
//  Creation:   May 4, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Mar  4 08:43:15 PST 2004
//    Made the constructor just call an initialize routine.
//
// ****************************************************************************

VisWindow::VisWindow()
    : colleagueProxy(this), interactorProxy(this), renderProxy(this),
      lightList()
{
    Initialize(new VisWinRenderingWithoutWindow(colleagueProxy));
}


// ****************************************************************************
//  Method: VisWindow protected constructor
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2004
//
// ****************************************************************************

VisWindow::VisWindow(bool callInit)
    : colleagueProxy(this), interactorProxy(this), renderProxy(this),
      lightList()
{
    if (callInit)
    {
        Initialize(new VisWinRenderingWithoutWindow(colleagueProxy));
    }
}


// ****************************************************************************
//  Method: VisWindow::Initialize
//
//  Purpose:
//      Does the work of the constructor.  Separated into its own routine so
//      there will be a "single point of source".
//
//  Notes:   This routine was formerly the VisWindow constructor until that
//           constructor got split on March 4, 2004.  For this reason, some
//           modification comments will come before the creation date.
//
//  Programmer: Hank Childs
//  Creation:   March 4, 2004
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 10:18:22 PDT 2000
//    Added function calls to initialize whether we can accept updates or
//    we have plots.  Also added construction of new colleague, frame.
//
//    Brad Whitlock, Tue Nov 7 10:56:06 PDT 2000
//    Added initialization of callback pointers.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    Added initialization of annotionAtts, and construction of new 
//    colleague, axes3D.  
//
//    Eric Brugger, Fri Aug 17 09:42:17 PDT 2001
//    I added a callback to capture resize events.
//
//    Eric Brugger, Wed Aug 22 15:29:35 PDT 2001
//    I moved the initialization of the VisWinView colleague before the
//    call to ChangeMode, since ChangeMode now depends on the view.
//
//    Brad Whitlock, Tue Aug 28 12:35:58 PDT 2001
//    Added the background colleague and some internal attributes.
//
//    Brad Whitlock, Mon Oct 1 12:48:03 PDT 2001
//    Added the tools colleague.
//
//    Brad Whitlock, Fri Oct 19 22:29:14 PST 2001
//    Added the light list.
//
//    Hank Childs, Tue Mar 12 18:29:46 PST 2002
//    Added legends colleague.
//
//    Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002 
//    Added query colleague.
//
//    Eric Brugger, Tue Mar 26 16:13:08 PST 2002
//    Changed the default viewport to be 0 -> 1 in the x and y directions
//    since it shouldn't be related to the defaults set by VisIt's viewer.
//
//    Hank Childs, Fri Feb  1 09:57:52 PST 2002
//    Added argument doNoWinMode.
//
//    Kathleen Bonnell, Fri May 10 15:38:14 PDT 2002   
//    Added lineout callback, and flag specifiying that this window type
//    should/should no be considered 'curve type'.  Used mainly by plots
//    when adding a new plot, to determine whether or not to change the
//    window mode. (Cannot be determined by spatial dimensions alone).
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    Added lighting colleague.
//
//    Kathleen Bonnell, Thu Sep  5 09:10:08 PDT 2002 
//    Create lighting colleague before view colleague, as updating the view
//    also updates lighting.
//
//    Eric Brugger, Wed Aug 20 10:09:15 PDT 2003
//    Removed typeIsCurve.
//
//    Brad Whitlock, Wed Oct 29 17:06:18 PST 2003
//    Added the annotation colleague and initialized the new frameAndState
//    array.
//
//    Kathleen Bonnell, Thu Sep  2 13:40:25 PDT 2004 
//    Initialize pickForIntersectionOnly.
//
// ****************************************************************************

void
VisWindow::Initialize(VisWinRendering *ren)
{
    startRenderCallback = vtkCallbackCommand::New();
    startRenderCallback->SetClientData(&renderProxy);
    startRenderCallback->SetCallback(&start_render);
    //
    // Set up all of the non-colleague fields. 
    //
    frameAndState[0] = 1;
    frameAndState[1] = 0;
    frameAndState[2] = 0;
    frameAndState[3] = 0;
    frameAndState[4] = 0;
    frameAndState[5] = 0;
    frameAndState[6] = 0;
    mode = WINMODE_NONE;
    backgroundMode = 0;
    gradientBackground[0][0] = 0.;
    gradientBackground[0][1] = 0.;
    gradientBackground[0][2] = 1.;
    gradientBackground[1][0] = 0.;
    gradientBackground[1][1] = 1.;
    gradientBackground[1][2] = 1.;
    gradientBackgroundStyle = 0;
    SetBackgroundColor(1., 1., 1.);
    SetForegroundColor(0., 0., 0.);
    SetViewport(0., 0., 1., 1.);
    EnableUpdates();
    NoPlots();

    //
    // rendering must be the first colleague added since it must change window
    // modes before the rest.
    //
    rendering = ren;
    rendering->SetResizeEvent(ProcessResizeEvent, this);
    AddColleague(rendering);

    view         = new VisWinView(colleagueProxy);
    AddColleague(view);

    lighting     = new VisWinLighting(colleagueProxy);
    AddColleague(lighting);

    //
    // ChangeMode has a dependence on rendering and the view, so it cannot
    // be changed until it is set.
    //
    ChangeMode(WINMODE_3D);

    plots        = new VisWinPlots(colleagueProxy);
    AddColleague(plots);

    queries      = new VisWinQuery(colleagueProxy);
    AddColleague(queries);

    interactions = new VisWinInteractions(colleagueProxy, interactorProxy);
    AddColleague(interactions);

    triad        = new VisWinTriad(colleagueProxy);
    AddColleague(triad);

    userInfo     = new VisWinUserInfo(colleagueProxy);
    AddColleague(userInfo);

    axes         = new VisWinAxes(colleagueProxy);
    AddColleague(axes);

    frame        = new VisWinFrame(colleagueProxy);
    AddColleague(frame);

    axes3D       = new VisWinAxes3D(colleagueProxy);
    AddColleague(axes3D);

    windowBackground = new VisWinBackground(colleagueProxy);
    AddColleague(windowBackground);

    tools        = new VisWinTools(colleagueProxy);
    AddColleague(tools);

    legends      = new VisWinLegends(colleagueProxy);
    AddColleague(legends);

    annotations = new VisWinAnnotations(colleagueProxy);
    AddColleague(annotations);

    // Initialize the menu callbacks.
    showMenuCallback = 0;
    showMenuCallbackData = 0;
    hideMenuCallback = 0;
    hideMenuCallbackData = 0;

    performPickCallback= 0;
    ppInfo = 0;

    performLineoutCallback= 0;
    loInfo = 0;

    pickForIntersectionOnly = false;
}


// ****************************************************************************
//  Method: VisWindow destructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Fri Jul  7 11:23:25 PDT 2000
//    Added deletion of colleague frame.
//
//    Eric Brugger, Mon Aug 13 16:57:31 PDT 2001
//    I moved the deletion of the renderer to the end of the routine to fix
//    a crash.
//
//    Brad Whitlock, Tue Aug 28 12:36:50 PDT 2001
//    Added deletion of the background colleague.
//
//    Brad Whitlock, Mon Oct 1 12:49:00 PDT 2001
//    Added deletion of the tools colleague.
//
//    Hank Childs, Tue Mar 12 18:29:46 PST 2002
//    Added deletion of legends colleague.
//
//    Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002 
//    Added deletion of query colleague.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    Added deletion of lighting colleague.
//
//    Brad Whitlock, Wed Oct 29 17:06:59 PST 2003
//    Added deletion of the annotation colleague.
//
// ****************************************************************************

VisWindow::~VisWindow()
{
    if (plots != NULL)
    {
        delete plots;
        plots = NULL;
    }
    if (queries != NULL)
    {
        delete queries;
        queries = NULL;
    }
    if (interactions != NULL)
    {
        delete interactions;
        interactions = NULL;
    }
    if (userInfo != NULL)
    {
        delete userInfo;
        userInfo = NULL;
    }
    if (axes != NULL)
    {
        delete axes;
        axes = NULL;
    }
    if (axes3D != NULL)
    {
        delete axes3D;
        axes3D = NULL;
    }
    if (tools != NULL)
    {
        delete tools;
        tools = NULL;
    }
    if (triad != NULL)
    {
        delete triad;
        triad = NULL;
    }
    if (frame != NULL)
    {
        delete frame;
        frame = NULL;
    }
    if (legends != NULL)
    {
        delete legends;
        legends = NULL;
    }
    if (lighting != NULL)
    {
        delete lighting;
        lighting = NULL;
    }
    if (windowBackground != NULL)
    {
        delete windowBackground;
        windowBackground = NULL;
    }
    if (view != NULL)
    {
        delete view;
        view = NULL;
    }
    if (rendering != NULL)
    {
        delete rendering;
        rendering = NULL;
    }
    if (annotations != NULL)
    {
        delete annotations;
        annotations = NULL;
    }
    if (startRenderCallback != NULL)
    {
        startRenderCallback->Delete();
        startRenderCallback = NULL;
    }
}


// ****************************************************************************
//  Method: VisWindow::AddColleague
//
//  Purpose:
//      Adds a colleague to our vector of colleagues and populates it with
//      information general to all colleagues.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 10:18:22 PDT 2000
//    Added calls to show whether updates are enabled and there are plots.
//
//    Brad Whitlock, Wed Aug 29 15:23:59 PST 2001
//    Added code to set the gradient background if that is the type of
//    background currently being used.
//
//    Hank Childs, Tue Sep 18 12:02:29 PDT 2001
//    Added case to switch statement to get rid of compiler warning.
//
//    Kathleen Bonnell, Fri May 10 15:38:14 PDT 2002   
//    Added support for WINMODE_CURVE. 
//
// ****************************************************************************

void
VisWindow::AddColleague(VisWinColleague *col)
{
    //
    // Add this to our vector.
    //
    colleagues.push_back(col);
   
    //
    // Set all of the general colleague information.
    //
    col->SetForegroundColor(foreground[0], foreground[1], foreground[2]);
    col->SetBackgroundColor(background[0], background[1], background[2]);
    col->SetGradientBackgroundColors(gradientBackgroundStyle,
        gradientBackground[0][0],
        gradientBackground[0][1],
        gradientBackground[0][2],
        gradientBackground[1][0],
        gradientBackground[1][1],
        gradientBackground[1][2]);
    col->SetBackgroundMode(backgroundMode);
    col->SetViewport(viewportLeft, viewportBottom, viewportRight, viewportTop);

    if (hasPlots)
    {
        col->HasPlots();
    }
    else
    {
        col->NoPlots();
    }

    if (updatesEnabled)
    {
        col->EnableUpdates();
    }
    else
    {
        col->DisableUpdates();
    }

    switch (mode)
    {
      case WINMODE_2D:
        col->Start2DMode();
        break;
      case WINMODE_3D:
        col->Start3DMode();
        break;
      case WINMODE_CURVE:
        col->StartCurveMode();
        break;
      case WINMODE_NONE:
      default:
        break;
    }
}


// ****************************************************************************
//  Method: VisWindow::SetBackgroundColor
//
//  Purpose:
//      Sets the background color for this and all of its modules.
//
//  Arguments:
//      br      The red component (rgb) of the background.
//      bg      The green component (rgb) of the background.
//      bb      The blue component (rgb) of the background.
//
//  Programmer: Hank Childs
//  Creation:   May 4, 2000
//
// ****************************************************************************

void
VisWindow::SetBackgroundColor(double br, double bg, double bb)
{
    background[0] = br;
    background[1] = bg;
    background[2] = bb;

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetBackgroundColor(br, bg, bb);
    }
}

// ****************************************************************************
// Method: VisWindow::SetGradientBackgroundColors
//
// Purpose: 
//   Sets the gradient background style and colors and also does so for all
//   colleagues.
//
// Arguments:
//   gradStyle : The gradient style. A value of 0 means top to bottom. A value
//               of 1 means radial.
//   c1R       : The red component for color 1.
//   c1G       : The green component for color 1.
//   c1B       : The blue component for color 1.
//   c2R       : The red component for color 2.
//   c2G       : The green component for color 2.
//   c2B       : The blue component for color 2.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 15:14:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetGradientBackgroundColors(int gradStyle,
    double c1R, double c1G, double c1B, double c2R, double c2G, double c2B)
{
    // Set the VisWindow's internal state.
    gradientBackground[0][0] = c1R;
    gradientBackground[0][1] = c1G;
    gradientBackground[0][2] = c1B;
    gradientBackground[1][0] = c2R;
    gradientBackground[1][1] = c2G;
    gradientBackground[1][2] = c2B;
    gradientBackgroundStyle = gradStyle;

    // Set the state of the colleagues.
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetGradientBackgroundColors(gradientBackgroundStyle,
             gradientBackground[0][0],
             gradientBackground[0][1],
             gradientBackground[0][2],
             gradientBackground[1][0],
             gradientBackground[1][1],
             gradientBackground[1][2]);
    }
}

// ****************************************************************************
// Method: VisWindow::SetBackgroundMode
//
// Purpose: 
//   Sets the background mode for the VisWindow. This determines whether or
//   not we have a solid or a gradient background.
//
// Arguments:
//   mode : The new background mode. A value of 0 means that we have a solid
//          background. A value of 1 means that we have a gradient background.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 15:28:49 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetBackgroundMode(int mode)
{
    backgroundMode = mode;

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetBackgroundMode(backgroundMode);
    }
}

// ****************************************************************************
// Method: VisWindow::GetBackgroundMode
//
// Purpose: 
//   Returns the background mode for the VisWindow.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 29 15:31:11 PST 2001
//   
// ****************************************************************************

int
VisWindow::GetBackgroundMode() const
{
    return backgroundMode;
}

// ****************************************************************************
//  Method: VisWindow::GetBackgroundColor
//
//  Purpose:
//      Gets the background color.
//
//  Programmer: Brad Whitlock
//  Creation:   Aug 27, 2001
//
// ****************************************************************************

const double *
VisWindow::GetBackgroundColor() const
{
    return (const double *)background;
}

// ****************************************************************************
//  Method: VisWindow::SetForegroundColor
//
//  Purpose:
//      Sets the foreground color for this and all of its modules.
//
//  Arguments:
//      fr      The red component (rgb) of the foreground.
//      fg      The green component (rgb) of the foreground.
//      fb      The blue component (rgb) of the foreground.
//
//  Programmer: Hank Childs
//  Creation:   May 12, 2000
//
// ****************************************************************************

void
VisWindow::SetForegroundColor(double fr, double fg, double fb)
{
    foreground[0] = fr;
    foreground[1] = fg;
    foreground[2] = fb;

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetForegroundColor(fr, fg, fb);
    }
}

// ****************************************************************************
//  Method: VisWindow::GetForegroundColor
//
//  Purpose:
//      Gets the foreground color.
//
//  Programmer: Brad Whitlock
//  Creation:   Aug 27, 2001
//
// ****************************************************************************

const double *
VisWindow::GetForegroundColor() const
{
    return (const double *)foreground;
}

// ****************************************************************************
// Method: VisWindow::InvertBackgroundColor
//
// Purpose: 
//   Inverts the background color and the foreground color for the window.
//
// Note:       We invert both the internal bg/fg colors and the ones in the
//             state object.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 16:55:29 PST 2001
//
// Modifications:
//
//   Brad Whitlock, Thu Sep 20 16:04:23 PST 2001
//   Modified to conform to the new AnnotationAttributes.
//
//   Hank Childs, Mon Feb 10 16:05:37 PST 2003
//   Make sure that the window re-renders after swapping the colors.
//
// ****************************************************************************

void
VisWindow::InvertBackgroundColor()
{
    double tmp[3];
    tmp[0] = background[0];
    tmp[1] = background[1];
    tmp[2] = background[2];
    SetBackgroundColor(foreground[0], foreground[1], foreground[2]);
    SetForegroundColor(tmp[0], tmp[1], tmp[2]);

    ColorAttribute tmp2(annotationAtts.GetBackgroundColor());
    annotationAtts.SetBackgroundColor(annotationAtts.GetForegroundColor());
    annotationAtts.SetForegroundColor(tmp2);

    Render();
}

// ****************************************************************************
//  Method: VisWinRendering::SetViewport
//
//  Purpose:
//      Sets the viewport.
//
//  Arguments:
//      vl     The normalized left viewport.
//      vb     The normalized bottom viewport.
//      vr     The normalized right viewport.
//      vt     The normalized top viewport.
//
//  Programmer: Hank Childs
//  Creation:   May 4, 2000
//
// ****************************************************************************

void
VisWindow::SetViewport(double vl, double vb, double vr, double vt)
{
    //
    // The viewport coordinates should be numbers from 0-1.  Check to make
    // sure this is the case.
    //
    viewportLeft   = (vl < 0. ? 0. : (vl > 1. ? 1. : vl));
    viewportRight  = (vr < 0. ? 0. : (vr > 1. ? 1. : vr));
    viewportBottom = (vb < 0. ? 0. : (vb > 1. ? 1. : vb));
    viewportTop    = (vt < 0. ? 0. : (vt > 1. ? 1. : vt));

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetViewport(vl, vb, vr, vt);
    }
}


// ****************************************************************************
//  Method: VisWindow::UpdatePlotList
//
//  Purpose:
//      Tells each of the colleagues what the current plot list is.
//
//  Arguments:
//      lst     The list of current plots.
//
//  Programmer: Hank Childs
//  Creation:   March 12, 2002
//
// ****************************************************************************

void
VisWindow::UpdatePlotList(vector<avtActor_p> &lst)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->UpdatePlotList(lst);
    }
}


// ****************************************************************************
//  Method: VisWindow::ChangeMode
//
//  Purpose:
//      Allows the old mode to stopped gracefully and then sets up the new
//      mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//   Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//   Changed the exception keywords to macros.
//
//    Kathleen Bonnell, Fri May 10 15:38:14 PDT 2002   
//    Added support for WINMODE_CURVE. 
//
// ****************************************************************************

void
VisWindow::ChangeMode(WINDOW_MODE newMode)
{
    if (mode == newMode)
    {
        //
        // No reason to shut down this mode just to start it back up.
        //
        return;
    }

    //
    // Get the first renderer and remove our `start_render' callback that
    // we registered before.
    //
    if (ValidMode(mode) && rendering != NULL)
    {
        vtkRenderer *firstRenderer = rendering->GetFirstRenderer();
        if (firstRenderer != NULL)
        {
            firstRenderer->RemoveObserver(startRenderCallback);
        }
    }

    switch (mode)
    {
      case WINMODE_2D:
        Stop2DMode();
        break;
      case WINMODE_3D:
        Stop3DMode();
        break;
      case WINMODE_CURVE:
        StopCurveMode();
        break;
      case WINMODE_NONE:
        break;
      default:
        { EXCEPTION1(BadWindowModeException, mode); }
        /* NOTREACHED */ break;
    }

    //
    // We must set the mode to be the new mode, since some of the modules
    // query our `mode' value to determine correct behavior.
    //
    mode = newMode;

    //
    // If we are trying to set this to be a bad mode, catch that and set 
    // ourselves to be in WINMODE_NONE.
    //
    TRY
    {
        switch (mode)
        {
          case WINMODE_2D:
            Start2DMode();
            break;
          case WINMODE_3D:
            Start3DMode();
            break;
          case WINMODE_CURVE:
            StartCurveMode();
            break;
          default:
            { EXCEPTION1(BadWindowModeException, mode); }
            /* NOTREACHED */ break;
        }
    }
    CATCH(BadWindowModeException)
    {
        mode = WINMODE_NONE;
        RETHROW;
    }
    ENDTRY

    //
    // Register our `start_render' as the method that should be called before
    // rendering starts.
    //
    if (ValidMode(mode) && rendering != NULL)
    {
        vtkRenderer *firstRenderer = rendering->GetFirstRenderer();
        if (firstRenderer != NULL)
        {
            firstRenderer->AddObserver(
                vtkCommand::StartEvent, startRenderCallback);
        }
    }
}


// ****************************************************************************
//  Method: VisWindow::Start2DMode
//
//  Purpose:
//      Has all of its modules start 2D mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Eric Brugger, Fri Aug 17 09:42:17 PDT 2001
//    I added code to update the view.
//
// ****************************************************************************

void
VisWindow::Start2DMode(void)
{
    //
    // Update the view.  In the future this should probably go into
    // VisWinView's Start2DMode, but for now we will do it here.
    //
    UpdateView();

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->Start2DMode();
    }
}


// ****************************************************************************
//  Method: VisWindow::Start3DMode
//
//  Purpose:
//      Has all of its modules start 3D mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Eric Brugger, Fri Aug 17 09:42:17 PDT 2001
//    I added code to update the view.
//
//    Kathleen Bonnell, Thu Oct 28 17:37:40 PDT 2004 
//    Moved 'UpdateView' to after the colleagues have started 3D mode. 
//
// ****************************************************************************

void
VisWindow::Start3DMode(void)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->Start3DMode();
    }

    //
    // Update the view.  In the future this should probably go into
    // VisWinView's Start3DMode, but for now we will do it here.
    //
    UpdateView();
}

// ****************************************************************************
//  Method: VisWindow::StartCurveMode
//
//  Purpose:
//      Has all of its modules start Curve mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002
//
// ****************************************************************************

void
VisWindow::StartCurveMode(void)
{
    //
    // Update the view.  In the future this should probably go into
    // VisWinView's StartCurveMode, but for now we will do it here.
    //
    UpdateView();

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->StartCurveMode();
    }
}


// ****************************************************************************
//  Method: VisWindow::Stop2DMode
//
//  Purpose:
//      Has all of its modules stop 2D mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

void
VisWindow::Stop2DMode(void)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->Stop2DMode();
    }
}


// ****************************************************************************
//  Method: VisWindow::Stop3DMode
//
//  Purpose:
//      Has all of its modules stop 3D mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
// ****************************************************************************

void
VisWindow::Stop3DMode(void)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->Stop3DMode();
    }
}

// ****************************************************************************
//  Method: VisWindow::StopCurveMode
//
//  Purpose:
//      Has all of its modules stop Curve mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002
//
// ****************************************************************************

void
VisWindow::StopCurveMode(void)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->StopCurveMode();
    }
}

// ****************************************************************************
//  Method: VisWindow::EnableUpdates
//
//  Purpose:
//      Tells all of the colleagues that they can enable updates.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWindow::EnableUpdates(void)
{
    updatesEnabled = true;
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->EnableUpdates();
    }
}


// ****************************************************************************
//  Method: VisWindow::DisableUpdates
//
//  Purpose:
//      Tells all of the colleagues that they should disable updates.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWindow::DisableUpdates(void)
{
    updatesEnabled = false;
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->DisableUpdates();
    }
}

// ****************************************************************************
// Method: VisWindow::GetHotPoint
//
// Purpose: 
//   Returns a pointer to the hotpoint being clicked at point (x,y).
//
// Arguments:
//   x : The x location of the click in device coordinates.
//   y : The y location of the click in device coordinates.
//   h : A reference to a hotpoint that will store the active hotpoint.
//
// Returns:    true if a hotpoint was clicked, false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 14:17:35 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::GetHotPoint(int x, int y, HotPoint &h) const
{
    return tools->GetHotPoint(x, y, h);
}

// ****************************************************************************
// Method: VisWindow::SetHighlightEnabled
//
// Purpose: 
//   Turns hotpoint highlights on/off.
//
// Arguments:
//   val : Whether or not to display hotpoint highlights.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 3 00:05:38 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetHighlightEnabled(bool val)
{
    tools->SetHighlightEnabled(val);
}

// ****************************************************************************
// Method: VisWindow::GetToolName
//
// Purpose: 
//   Returns the name of the specified tool.
//
// Arguments:
//   index : The index of the tool.
//
// Returns:    The name of the specified tool.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 10:47:10 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

const char *
VisWindow::GetToolName(int index) const
{
    return tools->GetToolName(index);
}

// ****************************************************************************
// Method: VisWindow::GetNumTools
//
// Purpose: 
//   Returns the number of tools.
//
// Returns:    The number of tools.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 12 10:46:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

int
VisWindow::GetNumTools() const
{
    return tools->GetNumTools();
}

// ****************************************************************************
// Method: VisWindow::SetToolEnabled
//
// Purpose: 
//   Sets the enabled state of the specified tool.
//
// Arguments:
//   index : The index of the tool.
//   val   : Whether the tool is on or off.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 13:20:20 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetToolEnabled(int index, bool val)
{
    tools->SetToolEnabled(index, val);
}

// ****************************************************************************
// Method: VisWindow::GetToolEnabled
//
// Purpose: 
//   Returns whether or not the specified tool is enabled.
//
// Arguments:
//   index : The index of the tool.
//
// Returns:    Whether or not the specified tool is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 13:21:00 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::GetToolEnabled(int index) const
{
    return tools->GetToolEnabled(index);
}

// ****************************************************************************
// Method: VisWindow::GetToolAvailable
//
// Purpose: 
//   Returns whether or not the specified tool is available.
//
// Arguments:
//   index : The index of the tool.
//
// Returns:    Whether or not the specified tool is available.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 1 13:21:00 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::GetToolAvailable(int index) const
{
    return tools->GetToolAvailable(index);
}


// ****************************************************************************
// Method: VisWindow::GetToolInterface
//
// Purpose: 
//   Returns the i'th tool interface.
//
// Arguments:
//   index : The index of the interface we want.
//
// Returns:    the index'th tool interface.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 11 14:32:09 PST 2002
//
// Modifications:
//   
// ****************************************************************************

avtToolInterface &
VisWindow::GetToolInterface(int index) const
{
    return tools->GetToolInterface(index);
}


// ****************************************************************************
// Method: VisWindow::UpdateTool
//
// Purpose: 
//   Tells the index'th tool to update itself and re-renders the window if
//   told to do so.
//
// Arguments:
//   index  : The index of the tool to redraw.
//   redraw : A flag that indicates whether a redraw is needed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 09:39:42 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::UpdateTool(int index, bool redraw)
{
    tools->UpdateTool(index);

    if(redraw)
        Render();
}


// ****************************************************************************
// Method: VisWindow::UpdatesEnabled
//
// Purpose: 
//   Returns whether or not updates are enabled.
//
// Returns:    Whether or not updates are enabled.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 16:11:49 PST 2001
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::UpdatesEnabled() const
{
    return updatesEnabled;
}

// ****************************************************************************
// Method: VisWindow::Iconify
//
// Purpose: 
//   Iconifies the render window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:39:11 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::Iconify()
{
    rendering->Iconify();
}

// ****************************************************************************
// Method: VisWindow::DeIconify
//
// Purpose: 
//   De-iconifies the render window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:39:36 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::DeIconify()
{
    rendering->DeIconify();
}

// ****************************************************************************
// Method: VisWindow::GetRealized
//
// Purpose: 
//   Returns the realized state of the window.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:44:17 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
bool
VisWindow::GetRealized()
{
    return rendering->GetRealized();
}

// ****************************************************************************
// Method: VisWindow::Show
//
// Purpose: 
//   Shows the render window.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:44:17 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
VisWindow::Show()
{
    rendering->Show();
}

// ****************************************************************************
// Method: VisWindow::Raise
//
// Purpose: 
//   Raises the render window.
//
// Programmer: Sean Ahern
// Creation:   Mon May 20 13:29:38 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
VisWindow::Raise()
{
    rendering->Raise();
}

// ****************************************************************************
// Method: VisWindow::Lower
//
// Purpose: 
//   Lowers the render window.
//
// Programmer: Sean Ahern
// Creation:   Mon May 20 13:29:38 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::Lower()
{
    rendering->Lower();
}

// ****************************************************************************
// Method: VisWindow::Hide
//
// Purpose: 
//   Hides the render window.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:44:17 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::Hide()
{
    rendering->Hide();
}

// ****************************************************************************
// Method: VisWindow::IsVisible
//
// Purpose: 
//   Returns whether the vis window is visible.
//
// Returns:    Whether the vis window is visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 09:23:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::IsVisible() const
{
    return rendering->IsVisible();
}

// ****************************************************************************
//  Method: VisWindow::SetSize
//
//  Purpose:
//      Sets the size of the vis window.
//
//  Arguments:
//      w       The desired width (in pixels) of the vis window.
//      h       The desired height (in pixels) of the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWindow::SetSize(int w, int h)
{
    rendering->SetSize(w, h);
}

// ****************************************************************************
// Method: VisWindow::GetSize
//
// Purpose: 
//   Returns the renderable portion of the window size.
//
// Arguments:
//   w : A reference to an int that is used to return the window width.
//   h : A reference to an int that is used to return the window height.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 10:54:52 PDT 2001
//
// ****************************************************************************

void
VisWindow::GetSize(int &w, int &h) const
{
    rendering->GetSize(w, h);
}

// ****************************************************************************
// Method: VisWindow::GetWindowSize
//
// Purpose: 
//   Returns the window size.
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
VisWindow::GetWindowSize(int &w, int &h) const
{
    rendering->GetWindowSize(w, h);
}

// ****************************************************************************
//  Method: VisWindow::SetLocation
//
//  Purpose:
//      Sets the location of the vis window.
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
VisWindow::SetLocation(int x, int y)
{
    rendering->SetLocation(x, y);
}

// ****************************************************************************
// Method: VisWindow::GetLocation
//
// Purpose: 
//   Returns the window location.
//
// Arguments:
//   x : A reference to an int that is used to return the window x location.
//   y : A reference to an int that is used to return the window y location.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 2 10:54:52 PDT 2001
//
// ****************************************************************************

void
VisWindow::GetLocation(int &x, int &y) const
{
    rendering->GetLocation(x, y);
}

// ****************************************************************************
//  Method: VisWindow::Realize
//
//  Purpose:
//      Realizes the vis window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// **************************************************************************** 

void
VisWindow::Realize(void)
{
    rendering->Realize();
}


// ****************************************************************************
//  Method: VisWindow::ScreenCapture
//
//  Purpose:
//      Saves the window to the filename.  More arguments need to be added as
//      more options are added.
//
//  Returns:    The image from the screen capture.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2001
//
//  Modifications:
//    Mark C. Miller, Wed Mar 31 18:00:23 PST 2004
//    Added doViewportOnly arg
//
//    Chris Wojtan, Wed Jul 21 15:14:30 PDT 2004
//    Pass separate bools for opaque and translucent data
//
//    Chris Wojtan, Fri Jul 30 14:35:49 PDT 2004
//    Pass in an image reference and a boolean indicating which rendering pass
//
//    Jeremy Meredith, Thu Oct 21 15:39:54 PDT 2004
//    Removed the last boolean, as we really only needed to check if "input"
//    was NULL.
//
// ****************************************************************************

avtImage_p
VisWindow::ScreenCapture(bool doViewportOnly, bool doZBufferToo,
                         bool doOpaque, bool doTranslucent,
                         avtImage_p input)
{
    return rendering->ScreenCapture(doViewportOnly, doZBufferToo,
                                    doOpaque, doTranslucent, input);
}

// ****************************************************************************
//  Method: VisWindow::PostProcessScreenCapture
//
//  Purpose:
//      Does any necessary post-processing on a screen captured image. See
//      comments in VisWinRendering.C for more details 
//
//  Programmer: Mark C. Miller 
//  Creation:   July 26, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct  6 17:50:23 PDT 2004
//    Added arg indicating if zbuffer should be kept or not
//
// ****************************************************************************

avtImage_p
VisWindow::PostProcessScreenCapture(avtImage_p capturedImage,
                                    bool doViewportOnly, bool keepZBuffer)
{
    return rendering->PostProcessScreenCapture(capturedImage,
                                               doViewportOnly,
                                               keepZBuffer);
}

// ****************************************************************************
//  Method: VisWindow::GetAllDatasets
//
//  Purpose:
//      Gets all of the datasets that are added to this window.
//
//  Returns:    A dataset that contains all the other datasets.
//
//  Programmer: Hank Childs
//  Creation:   May 24, 2002
//
// ****************************************************************************

avtDataset_p
VisWindow::GetAllDatasets()
{
    return plots->GetAllDatasets();
}


// ****************************************************************************
//  Method: VisWindow::HasPlots
//
//  Purpose:
//      Decides whether the vis window should notify its colleagues that the
//      plot status (ie has 'em or doesn't have 'em) has changed.
//
//  Arguments:
//      b       true if the vis window has plots, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri May 10 15:38:14 PDT 2002   
//    Added test for Lineout interaction mode, so that the mode could
//    be reset if necessary.
//
//    Kathleen Bonnell, Thu May 16 09:12:57 PDT 2002  
//    Moved test for Lineout && 3d to VisWinInteractions. 
// ****************************************************************************

void
VisWindow::HasPlots(bool b)
{
    if (b != hasPlots)
    {
        if (b)
        {
            HasPlots();
        }
        else
        {
            NoPlots();
        }
    }
}


// ****************************************************************************
//  Method: VisWindow::MotionBegin
//
//  Purpose:
//      Tells all of the colleagues that motion just began.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2002
//
// ****************************************************************************

void
VisWindow::MotionBegin(void)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->MotionBegin();
    }
}


// ****************************************************************************
//  Method: VisWindow::MotionEnd
//
//  Purpose:
//      Tells all of the colleagues that motion just ended.
//
//  Programmer: Hank Childs
//  Creation:   July 11, 2002
//
// ****************************************************************************

void
VisWindow::MotionEnd(void)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->MotionEnd();
    }
}


// ****************************************************************************
//  Method: VisWindow::HasPlots
//
//  Purpose:
//      Tells all of the colleagues that the VisWindow has plots.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWindow::HasPlots(void)
{
    hasPlots = true;
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->HasPlots();
    }
}


// ****************************************************************************
//  Method: VisWindow::NoPlots
//
//  Purpose:
//      Tells all of the colleagues that there are no plots in the VisWindow.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWindow::NoPlots(void)
{
    hasPlots = false;
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->NoPlots();
    }
}


// ****************************************************************************
//  Method: VisWindow::AddPlot
//
//  Purpose:
//      Adds a plot to the VisWindow.
//
//  Arguments:
//      p      An avtActor.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    Reset bounds for axes3D after plot is added.
// 
// ****************************************************************************

void
VisWindow::AddPlot(avtActor_p &p)
{
    plots->AddPlot(p);
    double bnds[6];
    plots->GetBounds(bnds);
    axes3D->SetBounds(bnds);
}


// ****************************************************************************
//  Method: VisWindow::RemovePlot
//
//  Purpose:
//      Removes a plot to the VisWindow.
//
//  Arguments:
//      p      An avtActor.
//
//  Programmer: Hank Childs
//  Creation:   September 28, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    Reset bounds for axes3D after plot is removed.
//
// ****************************************************************************

void
VisWindow::RemovePlot(avtActor_p &p)
{
    plots->RemovePlot(p);
    double bnds[6];
    plots->GetBounds(bnds);
    axes3D->SetBounds(bnds);
}


// ****************************************************************************
//  Method: VisWindow::ClearPlots
//
//  Purpose:
//      Removes all of the plots from the VisWindow.  This is done by one of
//      its colleagues.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWindow::ClearPlots(void)
{
    plots->ClearPlots();
}


// ****************************************************************************
//  Method: VisWindow::StartBoundingBox
//
//  Purpose:
//      Places the VisWindow into bounding box mode.  This is done by one of
//      its colleagues.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue Sep 25 10:22:04 PDT 2001
//    Turn off 3d axes bbox.
//
//    Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002 
//    Added call to query's StartBoundingBox method. 
//
//    Kathleen Bonnell, Mon Mar 18 09:32:20 PST 2002   
//    Move query's method before plots, so that pick points are hidden
//    before the plots are hidden.  (aesthetically more pleasing).
//
// ****************************************************************************

void
VisWindow::StartBoundingBox(void)
{
    axes3D->SetBBoxVisibility(false);
    queries->StartBoundingBox();
    plots->StartBoundingBox();
}


// ****************************************************************************
//  Method: VisWindow::EndBoundingBox
//
//  Purpose:
//      Takes the VisWindow out of bounding box mode.  This is done by one of
//      its colleagues.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue Sep 25 10:22:04 PDT 2001
//    Turn on 3d axes bbox, according to its set flag.
//
//    Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002 
//    Added call to query's EndBoundingBox method. 
//
// ****************************************************************************

void
VisWindow::EndBoundingBox(void)
{
    axes3D->SetBBoxVisibility(annotationAtts.GetBboxFlag());
    plots->EndBoundingBox();
    queries->EndBoundingBox();
}


// ****************************************************************************
//  Method: VisWindow::SetBoundingBoxMode
//
//  Purpose:
//      Sets the VisWindow's bounding box mode.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 15:40:54 PST 2000
//
// ****************************************************************************

void
VisWindow::SetBoundingBoxMode(bool mode)
{
    if(mode != interactions->GetBoundingBoxMode())
    {
        interactions->SetBoundingBoxMode(mode);
    }
}


// ****************************************************************************
//  Method: VisWindow::GetBoundingBoxMode
//
//  Purpose:
//      Gets the current bounding box mode.
//
//  Returns:    true if the viswindow is in bounding box mode, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
// ****************************************************************************

bool
VisWindow::GetBoundingBoxMode() const
{
    return interactions->GetBoundingBoxMode();
}


// ****************************************************************************
//  Method: VisWindow::SetViewExtentsType
//
//  Purpose:
//      Sets the flavor of spatial extents to be used when setting the view.
//
//  Programmer: Hank Childs
//  Creation:   July 15, 2002
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct  6 17:50:23 PDT 2004
//    Added code to force explicit bounds passed in from the caller
//
// ****************************************************************************

void
VisWindow::SetViewExtentsType(avtExtentType vt, const double *const expbnds)
{
    double bnds[6];
    plots->SetViewExtentsType(vt);
    if ((vt != AVT_SPECIFIED_EXTENTS) || (expbnds == NULL))
        plots->GetBounds(bnds);
    else
    {
        for (int i = 0; i < 6; i++)
            bnds[i] = expbnds[i];
    }
    axes3D->SetBounds(bnds);
}


// ****************************************************************************
//  Method: VisWindow::SetSpinMode
//
//  Purpose:
//      Sets the VisWindow's spin mode.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

void
VisWindow::SetSpinMode(bool mode)
{
    if(mode != interactions->GetSpinMode())
    {
        interactions->SetSpinMode(mode);
    }
}


// ****************************************************************************
//  Method: VisWindow::GetSpinMode
//
//  Purpose:
//      Gets the current spin mode.
//
//  Returns:    true if the viswindow is in spin mode, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

bool
VisWindow::GetSpinMode() const
{
    return interactions->GetSpinMode();
}


// ****************************************************************************
//  Method: VisWindow::SetSpinModeSuspended
//
//  Purpose:
//      Sets the VisWindow's spin mode suspension flag.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 10 16:02:22 PST 2003
//
// ****************************************************************************

void
VisWindow::SetSpinModeSuspended(bool mode)
{
    if(mode != interactions->GetSpinModeSuspended())
    {
        interactions->SetSpinModeSuspended(mode);
    }
}


// ****************************************************************************
//  Method: VisWindow::GetSpinModeSuspended
//
//  Purpose:
//      Gets whether spin mode is suspended.
//
//  Returns:    true if spin mode is suspended, false otherwise.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 10 16:01:57 PST 2003
//
// ****************************************************************************

bool
VisWindow::GetSpinModeSuspended() const
{
    return interactions->GetSpinModeSuspended();
}


// ****************************************************************************
//  Method: VisWindow::ResetView
//
//  Purpose:
//      Resets the camera to be the canonical view.  This is done by one of the
//      VisWindow's colleagues.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
//  Modifications:
//    Eric Brugger, Wed Aug 22 13:26:10 PDT 2001
//    Temporarily disable until VisWinView is fixed.
//
//    Kathleen Bonnell, Thu Aug 29 10:30:59 PDT 2002 
//    Added call to UpdateLightPositions, so that lights get updated properly
//    to new camera postions. (In case this method ever gets enabled again.) 
//
// ****************************************************************************

void
VisWindow::ResetView(void)
{
#if 0
    // Disable until done properly (Eric).
    view->ResetView();
    UpdateLightPositions();
#endif
}


// ****************************************************************************
//  Method: VisWindow::SetView2D
//
//  Purpose:
//    Sets the 2D view for the window.
//
//  Arguments:
//    v         The new view.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 10 10:37:19 PST 2003
//    If the new view is the same as the old view, do nothing.  This is
//    especially helpful when locking views.
//
//    Kathleen Bonnell, Thu May 15 10:00:02 PDT 2003 
//    Scale the plots if necessary. 
//    
//    Kathleen Bonnell, Fri Jun  6 15:53:58 PDT 2003  
//    Removed call to ScalePlots.  Added calls to FullFramOn/Off so that all
//    colleagues can be notified when full-frame mode changes. 
//    
//    Kathleen Bonnell, Wed Jul 16 16:32:43 PDT 2003 
//    Allow FullFrameOn to be called when scale factor changes, not just
//    when full-frame turned on for first time. 
//    
//    Eric Brugger, Thu Oct 16 09:26:49 PDT 2003
//    Modified to match changes in avtView2D made to handle full frame
//    mode properly.
//
//    Kathleen Bonnell, Thu Feb 12 16:15:03 PST 2004 
//    Added call to Render after call to FullFrameOff, to ensure that
//    the changes shows up immediately on the screen.
//
// ****************************************************************************

void
VisWindow::SetView2D(const avtView2D &v)
{
    if (view2D == v)
        return;
    
    //
    // Determine if full-frame mode has changed. 
    //
    bool fullFrameChanged = false;

    if (v.fullFrame != view2D.fullFrame)
    { 
        fullFrameChanged = true;
    }

    //
    // In the future this should propably be done by the VisWinView
    // colleague.
    //
    view2D = v;

    UpdateView();

    //
    // Tell colleagues that full-frame mode has changed, if necessary. 
    //
    if (fullFrameChanged && !view2D.fullFrame)
    {
        FullFrameOff();
        Render();
    }
}


// ****************************************************************************
//  Method: VisWindow::GetView2D
//
//  Purpose:
//    Gets the 2D view from the window.
//
//  Returns:    The currrent avtView2D.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 25 16:13:44 PST 2002
//    Set the viewport as well.
//
//    Eric Brugger, Tue Mar 26 14:27:13 PST 2002
//    Undo setting of the viewport.  The viewport in the view2D is different
//    from the viewport in viewportLeft, viewportRight, ...
//
//    Eric Brugger, Thu Oct 16 09:26:49 PDT 2003
//    Modified to match changes in avtView2D made to handle full frame
//    mode properly.
//
// ****************************************************************************

const avtView2D &
VisWindow::GetView2D(void)
{
    return view2D;
}


// ****************************************************************************
//  Method: VisWindow::SetView3D
//
//  Purpose:
//    Sets the 3D view for the window.
//
//  Arguments:
//    v         The new view.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Feb 10 10:37:19 PST 2003
//    If the new view is the same as the old view, do nothing.  This is
//    especially helpful when locking views.
//
// ****************************************************************************

void
VisWindow::SetView3D(const avtView3D &v)
{
    if (view3D == v)
        return;

    //
    // In the future this should propably be done by the VisWinView
    // colleague.
    //
    view3D = v;

    UpdateView();
}


// ****************************************************************************
//  Method: VisWindow::GetView3D
//
//  Purpose:
//    Gets the 3D view from the window.
//
//  Returns:    The currrent avtViewInfo.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2001
//
//  Modifications:
//    Eric Brugger, Fri Mar 29 16:29:22 PST 2002
//    Modify the method to just return the internally stored avtView3D 
//    without first updating it from the vtk view state.
//
// ****************************************************************************

const avtView3D &
VisWindow::GetView3D(void)
{
    return view3D;
}

// ****************************************************************************
//  Method: VisWindow::SetViewCurve
//
//  Purpose:
//    Sets the Curve view for the window.
//
//  Arguments:
//    v         The new view.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Feb 10 10:37:19 PST 2003
//    If the new view is the same as the old view, do nothing.  This is
//    especially helpful when locking views.
//
// ****************************************************************************

void
VisWindow::SetViewCurve(const avtViewCurve &v)
{
    if (viewCurve == v)
        return;

    //
    // In the future this should propably be done by the VisWinView
    // colleague.
    //
    viewCurve = v;

    UpdateView();
}


// ****************************************************************************
//  Method: VisWindow::GetViewCurve
//
//  Purpose:
//    Gets the Curve view from the window.
//
//  Returns:    The currrent avtViewCurve.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 10, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 10:09:15 PDT 2003
//    Modified the routine to pass the window size to SetViewFromViewInfo
//    so that it could handle non-square windows.
//
//    Eric Brugger, Tue Nov 18 11:11:09 PST 2003
//    Modify the method to just return the internally stored avtViewCurve
//    without first updating it from the vtk view state.
//
// ****************************************************************************

const avtViewCurve &
VisWindow::GetViewCurve(void)
{
    return viewCurve;
}


// ****************************************************************************
//  Method: VisWindow::GetWindowMode
//
//  Purpose:
//    Gets the window mode from the window.
//
//  Returns:    The currrent window mode.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2003
//
// ****************************************************************************

WINDOW_MODE
VisWindow::GetWindowMode() const
{
    return mode;
}


// ****************************************************************************
//  Method: VisWindow::GetCanvas
//
//  Purpose:
//      Returns the renderer acting as the canvas for the vis window.
//
//  Returns:    The vtkRenderer that acts as the canvas.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
// ****************************************************************************

vtkRenderer *
VisWindow::GetCanvas(void)
{
    return rendering->GetCanvas();
}


// ****************************************************************************
//  Method: VisWindow::GetBackground
//
//  Purpose:
//      Returns the renderer acting as the background for the vis window.
//
//  Returns:    The vtkRenderer that acts as the background.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
// ****************************************************************************

vtkRenderer *
VisWindow::GetBackground(void)
{
    return rendering->GetBackground();
}


// ****************************************************************************
//  Method: VisWindow::GetForeground
//
//  Purpose:
//     Returns the renderer acting as the foreground for the vis window.
//
//  Returns:     The vtkRenderer that acts as the foreground.
//
//  Programmer: Hank Childs
//  Creation:   July 26, 2000
//
// ****************************************************************************

vtkRenderer *
VisWindow::GetForeground(void)
{
    return rendering->GetForeground();
}


// ****************************************************************************
//  Method: VisWindow::Render
//
//  Purpose:
//      Renders the VisWindow explicitly.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
// ****************************************************************************

void
VisWindow::Render(void)
{
    rendering->Render();
}


// ****************************************************************************
//  Method: VisWindow::UpdateView
//
//  Purpose:
//    Set the view based on mode (dimensionality) of the window.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Mar 26 11:29:53 PST 2002
//    If appropriate, call a routine indicating that the viewport has changed.
//
//    Kathleen Bonnell, Fri May 10 15:38:14 PDT 2002   
//    Added support for WINMODE_CURVE. 
//
//    Eric Brugger, Wed Aug 20 10:09:15 PDT 2003
//    Added code to scale the plot if setting a curve view.  Pass the size
//    to avtViewCurve::SetViewInfoFromView so that it can handle non-square
//    windows.
//
//    Eric Brugger, Thu Oct 16 09:26:49 PDT 2003
//    Modified to match changes in avtView2D and avtViewCurve made to handle
//    full frame mode properly.
//
//    Kathleen Bonnell, Thu Feb 12 16:15:03 PST 2004 
//    Added call to Render after call to FullFrameOn in 2d, to ensure that
//    the changes shows up immediately on the screen.
// 
//    Kathleen Bonnell, Tue Apr 27 13:29:46 PDT 2004 
//    Added call to Render after call to FullFrameOn in Curve mode, to ensure 
//    that the changes show up immediately on the screen.
// 
// ****************************************************************************

void
VisWindow::UpdateView()
{
    if (mode == WINMODE_2D)
    {
        double    viewport[4];
        avtViewInfo viewInfo;

        int *size=rendering->GetFirstRenderer()->GetSize();

        view2D.GetActualViewport(viewport, size[0], size[1]);
        if (viewport[0] != viewportLeft || viewport[1] != viewportRight ||
            viewport[2] != viewportBottom || viewport[3] != viewportTop)
        {
            SetViewport(viewport[0], viewport[2], viewport[1], viewport[3]);
        }

        view2D.SetViewInfoFromView(viewInfo, size);
        view->SetViewInfo(viewInfo);

        if (view2D.fullFrame)
        {
            FullFrameOff();
            FullFrameOn(view2D.GetScaleFactor(size), 1);
            Render();
        }
    }
    else if (mode == WINMODE_3D)
    {
        avtViewInfo viewInfo;

        view3D.SetViewInfoFromView(viewInfo);
        view->SetViewInfo(viewInfo);
    }
    else if (mode == WINMODE_CURVE)
    {
        avtViewInfo viewInfo;

        int *size=rendering->GetFirstRenderer()->GetSize();

        if (viewCurve.viewport[0] != viewportLeft ||
            viewCurve.viewport[1] != viewportRight ||
            viewCurve.viewport[2] != viewportBottom ||
            viewCurve.viewport[3] != viewportTop)
        {
            SetViewport(viewCurve.viewport[0], viewCurve.viewport[2],
                        viewCurve.viewport[1], viewCurve.viewport[3]);
        }

        viewCurve.SetViewInfoFromView(viewInfo, size);
        view->SetViewInfo(viewInfo);

        FullFrameOff();
        FullFrameOn(viewCurve.GetScaleFactor(size), 1);
        Render();
    }
}


// ****************************************************************************
//  Method: VisWindow::SetInteractionMode
//
//  Purpose:
//      Sets the interaction mode of the VisWindow.
//
//  Arguments:
//      m           The new interaction mode.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
//  Modifications:
//    Kathleen Bonnell, Fri May 10 15:38:14 PDT 2002   
//    Added support for LINEOUT. 
//
//    Kathleen Bonnell, Fri Jun 27 16:30:26 PDT 2003  
//    Removed calls to queries->SetQueryType, no longer necessary. 
//
//    Brad Whitlock, Wed Jan 7 14:38:38 PST 2004
//    Added code to tell the renderer colleague to set the right cursor
//    for the interaction mode.
//
// ****************************************************************************

void
VisWindow::SetInteractionMode(INTERACTION_MODE m)
{
    interactions->SetInteractionMode(m);
    rendering->SetCursorForMode(m);
}

// ****************************************************************************
//  Method: VisWindow::GetInteractionMode
//
//  Purpose:
//      Returns the VisWindow's current interaction mode.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 16:08:40 PST 2000
//
// ****************************************************************************

INTERACTION_MODE
VisWindow::GetInteractionMode() const
{
    return interactions->GetInteractionMode();
}

// ****************************************************************************
//  Method: VisWindow::SetInteractor
//
//  Purpose:
//      Sets the interactor.
//
//  Arguments:
//      i      The new interactor.
//
//  Note:      This is a protected method and is only meant to be accessed
//             by its colleagues through the proxy.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
// ****************************************************************************

void
VisWindow::SetInteractor(VisitInteractor *i)
{
    rendering->SetInteractor(i);
}


// ****************************************************************************
//  Method: VisWindow::GetForegroundColor
//
//  Purpose:
//      Copies the foreground color into the argument array.
//
//  Arguments:
//      fg       An array to copy the foreground color into.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
// ****************************************************************************

void
VisWindow::GetForegroundColor(double *fg)
{
    fg[0] = foreground[0];
    fg[1] = foreground[1];
    fg[2] = foreground[2];
}


// ****************************************************************************
//  Method: VisWindow::GetViewport
//
//  Purpose:
//      Copies the viewport into the argument array.  Copies in left, bottom,
//      right, top order.
//
//  Arguments:
//      vport      An array to copy the viewport into.
//
//  Programmer: Hank Childs
//  Creation:   June 5, 2000
//
// ****************************************************************************

void
VisWindow::GetViewport(double *vport)
{
    vport[0] = viewportLeft;
    vport[1] = viewportBottom;
    vport[2] = viewportRight;
    vport[3] = viewportTop;
}


// ****************************************************************************
//  Method: VisWindow::RecalculateRenderOrder
//
//  Purpose:
//      Recalculates the order in which the plots should be rendered.
//
//  Programmer: Hank Childs
//  Creation:   August 4, 2002
//
//  Modifications:
//    Kathleen Bonnell, Wed May 28 16:25:37 PDT 2003
//    Added ReAddColleagesToRenderWindow. 
//
//    Kathleen Bonnell, Tue Jul  8 20:06:37 PDT 2003 
//    Always allow colleagues to re-add themselves (not just for antialiasing).
// 
//    Kathleen Bonnell, Mon Sep 29 13:15:20 PDT 2003 
//    Pass the antialiasing flag to the plots. 
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Pushed call to ReAddColleaguesToRenderWindow down into OrderPlots
// 
// ****************************************************************************

void
VisWindow::RecalculateRenderOrder(void)
{
    plots->OrderPlots();
}


// ****************************************************************************
//  Method: VisWindow::StartRender
//
//  Purpose:
//      This method is called when VTK wants to start it rendering.  Many
//      of the VisWindow's colleagues are view dependent and this gives
//      them a chance to update.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 27 10:02:18 PST 2002
//    Add support for timings.
//
// ****************************************************************************

void
VisWindow::StartRender(void)
{
    static int timingIndex = -1;
    static int numFrames = 0;
    if (timingIndex == -1)
    {
        timingIndex = visitTimer->StartTimer();
    }
    numFrames++;

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->UpdateView();
    }

    if (numFrames >= 50)
    {
        visitTimer->StopTimer(timingIndex,
                              "Time elapsed over rendering of last 50 frames");
        visitTimer->DumpTimings();
        timingIndex = visitTimer->StartTimer();
        numFrames = 0;
    }
}


// ****************************************************************************
//  Method: VisWindow::SetBounds
//
//  Purpose:
//      Tells the VisWindow that the initial view should contain these bounds.
//      This is used for animations.
//
//  Arguments:
//      bounds    The desired bounds as <xmin, xmax, ymin, ymax, zmin, zmax>.
//
//  Programmer: Hank Childs
//  Creation:   August 6, 2000
//
// ****************************************************************************

void
VisWindow::SetBounds(const double bounds[6])
{
    plots->SetBounds(bounds);
}


// ****************************************************************************
//  Method: VisWindow::UnsetBounds
//
//  Purpose:
//      Tells the VisWindow that the bounds should no longer conform to the
//      previously specified bounds.
//
//  Programmer: Hank Childs
//  Creation:   August 6, 2000
//
// ****************************************************************************

void
VisWindow::UnsetBounds()
{
    plots->UnsetBounds();
}


// ****************************************************************************
//  Method: VisWindow::GetBounds
//
//  Purpose:
//      Gets the bounds of the plots in the VisWindow.
//
//  Arguments:
//      bounds      A place to copy the bounds.
//
//  Programmer:  Hank Childs
//  Creation:    November 8, 2000
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct  6 17:50:23 PDT 2004
//    Added const qualification
//
// ****************************************************************************

void
VisWindow::GetBounds(double bounds[6]) const
{
    plots->GetBounds(bounds);
}


// ****************************************************************************
//  Method: VisWindow::SetTitle
//
//  Purpose:
//      Sets the title of the VisWindow.
//
//  Arguments:
//      title   The title of the VisWindow.
//
//  Programmer: Hank Childs
//  Creation:   September 14, 2000
//
// ****************************************************************************

void
VisWindow::SetTitle(const char *title)
{
    rendering->SetTitle(title);
}

// ****************************************************************************
// Method: VisWindow::ShowMenu
//
// Purpose: 
//   Executes the callback that tells the VisWindow's menu to show itself.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 3 13:49:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::ShowMenu()
{
    if(showMenuCallback == 0)
        return;

    // Execute the callback.
    (*showMenuCallback)(showMenuCallbackData);
}

// ****************************************************************************
// Method: VisWindow::SetShowMenu
//
// Purpose: 
//   Sets the callback to use to show the popup menu.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 3 13:49:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetShowMenu(VisCallback *cb, void *data)
{
    showMenuCallback = cb;
    showMenuCallbackData = data;
}

// ****************************************************************************
// Method: VisWindow::HideMenu
//
// Purpose: 
//   Executes the callback that tells the VisWindow's menu to hide itself.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 3 13:49:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::HideMenu()
{
    if(hideMenuCallback == 0)
        return;

    // Execute the callback.
    (*hideMenuCallback)(hideMenuCallbackData);
}

// ****************************************************************************
// Method: VisWindow::SetHideMenu
//
// Purpose: 
//   Sets the callback to use to hide the popup menu.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 3 13:49:15 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetHideMenu(VisCallback *cb, void *data)
{
    hideMenuCallback = cb;
    hideMenuCallbackData = data;
}

// ****************************************************************************
// Method: VisWindow::SetCloseCallback
//
// Purpose: 
//   Sets the callback function that is called when the window is closed.
//
// Arguments:
//   cb   : A pointer to the callback function.
//   data : The data to be passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 22 11:59:57 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetCloseCallback(VisCallback *cb, void *data)
{
    rendering->SetCloseCallback(cb, data);
}

// ****************************************************************************
// Method: VisWindow::SetHideCallback
//
// Purpose: 
//   Sets the callback function that is called when the window is hidden.
//
// Arguments:
//   cb   : A pointer to the callback function.
//   data : The data to be passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 09:57:38 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetHideCallback(VisCallback *cb, void *data)
{
    rendering->SetHideCallback(cb, data);
}

// ****************************************************************************
// Method: VisWindow::SetShowCallback
//
// Purpose: 
//   Sets the callback function that is called when the window is shown.
//
// Arguments:
//   cb   : A pointer to the callback function.
//   data : The data to be passed to the callback.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 12 09:57:38 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetShowCallback(VisCallback *cb, void *data)
{
    rendering->SetShowCallback(cb, data);
}

// ****************************************************************************
// Method: VisWindow::SetAnnotationAtts
//
// Purpose: 
//   Sets the annotation attributes used to control axes, etc. 
//
//  Arguments:
//    atts     The annotation attributes to use. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 18, 2001 
//
// Modifications:
//   Brad Whitlock, Mon Aug 27 15:42:38 PST 2001
//   I added code to set the fg/bg colors and background style.
//
//   Brad Whitlock, Thu Sep 20 16:05:16 PST 2001
//   Modified to conform to the new AnnotationAttributes.
//
//   Eric Brugger, Fri Nov  2 14:04:00 PST 2001
//   I added a const qualifier for atts.
//
//   Brad Whitlock, Thu Jan 10 08:20:30 PDT 2002
//   I added a call to UpdateTextAnnotations.
//
// ****************************************************************************

void
VisWindow::SetAnnotationAtts(const AnnotationAttributes *atts)
{
    bool changed = (annotationAtts != *atts);

    if (changed)
    {
        // Set the background and foreground colors.
        double bg[3], fg[3], gbg1[3], gbg2[3];
        bg[0] = double(atts->GetBackgroundColor().Red()) / 255.;
        bg[1] = double(atts->GetBackgroundColor().Green()) / 255.;
        bg[2] = double(atts->GetBackgroundColor().Blue()) / 255.;
        fg[0] = double(atts->GetForegroundColor().Red()) / 255.;
        fg[1] = double(atts->GetForegroundColor().Green()) / 255.;
        fg[2] = double(atts->GetForegroundColor().Blue()) / 255.;
        gbg1[0] = double(atts->GetGradientColor1().Red()) / 255.;
        gbg1[1] = double(atts->GetGradientColor1().Green()) / 255.;
        gbg1[2] = double(atts->GetGradientColor1().Blue()) / 255.;
        gbg2[0] = double(atts->GetGradientColor2().Red()) / 255.;
        gbg2[1] = double(atts->GetGradientColor2().Green()) / 255.;
        gbg2[2] = double(atts->GetGradientColor2().Blue()) / 255.;
        SetBackgroundColor(bg[0], bg[1], bg[2]);
        SetGradientBackgroundColors(atts->GetGradientBackgroundStyle(),
            gbg1[0], gbg1[1], gbg1[2], gbg2[0], gbg2[1], gbg2[2]);
        SetForegroundColor(fg[0], fg[1], fg[2]);
        SetBackgroundMode(atts->GetBackgroundMode());

        // Copy the annotation attributes.
        annotationAtts = *atts;

        // Update the axes using the new annotation attributes.
        UpdateAxes2D();
        UpdateAxes3D();
        UpdateTextAnnotations();

        // Re-render the window.
        Render();
    }
}

// ****************************************************************************
// Method: VisWindow::GetAnnotationAttributes()
//
// Purpose: 
//   Returns a pointer to the window's annotation attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 08:42:48 PDT 2001
//   
// ****************************************************************************

const AnnotationAttributes *
VisWindow::GetAnnotationAtts() const
{
    return (const AnnotationAttributes *)&annotationAtts;
}

// ****************************************************************************
// Method: VisWindow::AddAnnotationObject
//
// Purpose: 
//   Tells the annotation colleague to create a new annotation.
//
// Arguments:
//   annotType : The type of annotation to create.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:32:59 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::AddAnnotationObject(int annotType)
{
    annotations->AddAnnotationObject(annotType);
}

// ****************************************************************************
// Method: VisWindow::HideActiveAnnotationObjects
//
// Purpose: 
//   Hides the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:33:29 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::HideActiveAnnotationObjects()
{
    annotations->HideActiveAnnotationObjects();
}

// ****************************************************************************
// Method: VisWindow::DeleteActiveAnnotationObjects
//
// Purpose: 
//   Deletes the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:33:43 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::DeleteActiveAnnotationObjects()
{
    annotations->DeleteActiveAnnotationObjects();
}

// ****************************************************************************
// Method: VisWindow::DeleteAllAnnotationObjects
//
// Purpose: 
//   Deletes all annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:34:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::DeleteAllAnnotationObjects()
{
    annotations->DeleteAllAnnotationObjects();
}

// ****************************************************************************
// Method: VisWindow::RaiseActiveAnnotationObjects
//
// Purpose: 
//   Raises the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:34:21 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::RaiseActiveAnnotationObjects()
{
    annotations->RaiseActiveAnnotationObjects();
}

// ****************************************************************************
// Method: VisWindow::LowerActiveAnnotationObjects
//
// Purpose: 
//   Lowers the active annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::LowerActiveAnnotationObjects()
{
    annotations->LowerActiveAnnotationObjects();
}

// ****************************************************************************
// Method: VisWindow::SetAnnotationObjectOptions
//
// Purpose: 
//   Sets the annotation object options.
//
// Arguments:
//   al : The annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:35:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetAnnotationObjectOptions(const AnnotationObjectList &al)
{
    annotations->SetAnnotationObjectOptions(al);
}

// ****************************************************************************
// Method: VisWindow::UpdateAnnotationObjectList
//
// Purpose: 
//   Updates the annotation object list.
//
// Arguments:
//   al : The annotation object list that we're going to update.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:35:45 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::UpdateAnnotationObjectList(AnnotationObjectList &al)
{
    annotations->UpdateAnnotationObjectList(al);
}

// ****************************************************************************
// Method: VisWindow::CreateAnnotationObjectsFromList
//
// Purpose: 
//   Creates the annotation objects recorded in the annotation object list.
//
// Arguments:
//   al : The annotation object list used to create annotations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:36:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::CreateAnnotationObjectsFromList(const AnnotationObjectList &al)
{
    annotations->CreateAnnotationObjectsFromList(al);
}

// ****************************************************************************
// Method: VisWindow::SetFrameAndState
//
// Purpose: 
//   Sets the vis window's frame and state so they are available for colleagues
//   that need that information.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:37:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetFrameAndState(int nFrames,
    int startFrame, int curFrame, int endFrame,
    int startState, int curState, int endState)
{
    frameAndState[0] = nFrames;
    frameAndState[1] = startFrame;
    frameAndState[2] = curFrame;
    frameAndState[3] = endFrame;
    frameAndState[4] = startState;
    frameAndState[5] = curState;
    frameAndState[6] = endState;

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetFrameAndState(nFrames, startFrame, curFrame, endFrame,
                                startState, curState, endState);
    }
}

// ****************************************************************************
// Method: VisWindow::GetFrameAndState
//
// Purpose: 
//   Gets the vis window's frame and state information.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 2 15:37:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::GetFrameAndState(int &nFrames,
    int &startFrame, int &curFrame, int &endFrame,
    int &startState, int &curState, int &endState) const
{
    nFrames    = frameAndState[0];
    startFrame = frameAndState[1];
    curFrame   = frameAndState[2];
    endFrame   = frameAndState[3];
    startState = frameAndState[4];
    curState   = frameAndState[5];
    endState   = frameAndState[6];
}

// ****************************************************************************
// Method: VisWindow::SetLightList
//
// Purpose: 
//   Sets the light list and re-renders the window.
//
// Arguments:
//   ll : The new light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:31:24 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//   Fleshed out this method with new lighting colleague.  Changed parameter
//   to LightList from avtLightList.
//   
//   Kathleen Bonnell, Tue Nov  5 08:32:59 PST 2002 
//   Allow tools to update lighting conditions. 
// 
//   Kathleen Bonnell, Tue Oct 26 15:58:06 PDT 2004 
//   Call UpdateView so that CameraLights' transform matrix gets recomputed. 
// 
// ****************************************************************************

void
VisWindow::SetLightList(const LightList *ll)
{
    bool changed = (lightList != *ll);
    if (changed)
    {
        lightList = *ll;
        avtLightList aLL(*ll); 
        lighting->SetLightList(aLL);

        if (lighting->GetNumLightsEnabled() > 0)  
        {
            // Set Ambient to 0, Diffuse to 1
            plots->TurnLightingOn();
            tools->TurnLightingOn();
        }
        else
        {
            // Set Ambient to 1, Diffuse to 0
            plots->TurnLightingOff();
            tools->TurnLightingOff();
        }
        if (lighting->GetAmbientOn())
        {
            //
            //  If ambient lighting is on, the lighting coefficient
            //  may need to be changed.
            //
            plots->SetAmbientCoefficient(lighting->GetAmbientCoefficient());
        }
        UpdateView();
        Render();
    }
}

// ****************************************************************************
// Method: VisWindow::GetLightList
//
// Purpose: 
//   Returns a reference to the window's light list.
//
// Returns:    A reference to the window's light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 22:31:54 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//   Change return type to LightList from avtLightList.
//   
// ****************************************************************************

const LightList *
VisWindow::GetLightList() const
{
    return (const LightList *)&lightList;
}

// ****************************************************************************
// Method: VisWindow::UpdateAxes2D
//
// Purpose: 
//   Updates necessary aspects of VisWinAxes.
//
// Arguments:
//   atts     The annotation attributes to use. 
//
// Programmer: Kathleen Bonnell 
// Creation:   June 18, 2001 
//
// Modifications:
//   Kathleen Bonnell, Fri Jul  6 14:48:53 PDT 2001
//   Added setting of gridlines.
//
//   Eric Brugger, Wed Nov  5 14:03:46 PST 2002
//   Added more user control over the axes and tick marks.
//
//   Eric Brugger, Fri Jan 24 11:22:43 PST 2003
//   Changed the way the font size is set.
//
//   Eric Brugger, Wed Jun 25 15:45:22 PDT 2003
//   Added the setting of the line width.
//
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003 
//   Added the setting of the label scaling.
//
//   Brad Whitlock, Thu Jul 28 08:40:31 PDT 2005
//   Added ability to set axis titles and units.
//
// ****************************************************************************

void
VisWindow::UpdateAxes2D()
{
    //
    // Axes visibility
    //
    axes->SetVisibility(annotationAtts.GetAxesFlag2D());
    frame->SetVisibility(annotationAtts.GetAxesFlag2D());

    //
    // Labels
    //
    int xLabel = annotationAtts.GetXAxisLabels2D();
    int yLabel = annotationAtts.GetYAxisLabels2D();
    axes->SetLabelsVisibility(xLabel, yLabel);

    axes->SetLabelScaling(annotationAtts.GetLabelAutoSetScaling2D(), 
                          annotationAtts.GetXLabelScaling2D(),
                          annotationAtts.GetYLabelScaling2D());

    //
    // Titles
    //
    axes->SetTitleVisibility(annotationAtts.GetXAxisTitle2D(),
                             annotationAtts.GetYAxisTitle2D());
    axes->SetXTitle(annotationAtts.GetXAxisUserTitle2D(),
                    annotationAtts.GetXAxisUserTitleFlag2D());
    axes->SetXUnits(annotationAtts.GetXAxisUserUnits2D(),
                    annotationAtts.GetXAxisUserUnitsFlag2D());
    axes->SetYTitle(annotationAtts.GetYAxisUserTitle2D(),
                    annotationAtts.GetYAxisUserTitleFlag2D());
    axes->SetYUnits(annotationAtts.GetYAxisUserUnits2D(),
                    annotationAtts.GetYAxisUserUnitsFlag2D());

    //
    // GridLines
    //
    axes->SetXGridVisibility(annotationAtts.GetXGridLines2D());
    axes->SetYGridVisibility(annotationAtts.GetYGridLines2D());

    //
    // Ticks
    //
    switch (annotationAtts.GetAxesTicks2D())
    {
        case 0 : // off 
                 axes->SetXTickVisibility(0, xLabel); 
                 axes->SetYTickVisibility(0, yLabel); 
                 frame->SetTopRightTickVisibility(0); 
                 break;
        case 1 : // bottom 
                 axes->SetXTickVisibility(1, xLabel); 
                 axes->SetYTickVisibility(0, yLabel); 
                 frame->SetTopRightTickVisibility(0); 
                 break;
        case 2 : // left
                 axes->SetXTickVisibility(0, xLabel); 
                 axes->SetYTickVisibility(1, yLabel); 
                 frame->SetTopRightTickVisibility(0); 
                 break;
        case 3 : //bottom-left
                 axes->SetXTickVisibility(1, xLabel);
                 axes->SetYTickVisibility(1, yLabel); 
                 frame->SetTopRightTickVisibility(0); 
                 break;
        case 4 : //all
                 axes->SetXTickVisibility(1, xLabel); 
                 axes->SetYTickVisibility(1, yLabel); 
                 frame->SetTopRightTickVisibility(1); 
                 break;
    }
    axes->SetTickLocation(annotationAtts.GetAxesTickLocation2D());
    frame->SetTickLocation(annotationAtts.GetAxesTickLocation2D());

    axes->SetAutoSetTicks(annotationAtts.GetAxesAutoSetTicks2D());
    axes->SetMajorTickMinimum(annotationAtts.GetXMajorTickMinimum2D(),
                              annotationAtts.GetYMajorTickMinimum2D());
    axes->SetMajorTickMaximum(annotationAtts.GetXMajorTickMaximum2D(),
                              annotationAtts.GetYMajorTickMaximum2D());
    axes->SetMajorTickSpacing(annotationAtts.GetXMajorTickSpacing2D(),
                              annotationAtts.GetYMajorTickSpacing2D());
    axes->SetMinorTickSpacing(annotationAtts.GetXMinorTickSpacing2D(),
                              annotationAtts.GetYMinorTickSpacing2D());
    frame->SetAutoSetTicks(annotationAtts.GetAxesAutoSetTicks2D());
    frame->SetMajorTickMinimum(annotationAtts.GetXMajorTickMinimum2D(),
                               annotationAtts.GetYMajorTickMinimum2D());
    frame->SetMajorTickMaximum(annotationAtts.GetXMajorTickMaximum2D(),
                               annotationAtts.GetYMajorTickMaximum2D());
    frame->SetMajorTickSpacing(annotationAtts.GetXMajorTickSpacing2D(),
                               annotationAtts.GetYMajorTickSpacing2D());
    frame->SetMinorTickSpacing(annotationAtts.GetXMinorTickSpacing2D(),
                               annotationAtts.GetYMinorTickSpacing2D());

    //
    // Font size
    //
    axes->SetXLabelFontHeight(annotationAtts.GetXLabelFontHeight2D());
    axes->SetYLabelFontHeight(annotationAtts.GetYLabelFontHeight2D());
    axes->SetXTitleFontHeight(annotationAtts.GetXTitleFontHeight2D());
    axes->SetYTitleFontHeight(annotationAtts.GetYTitleFontHeight2D());

    //
    // Line width
    //
    axes->SetLineWidth(LineWidth2Int(Int2LineWidth(
        annotationAtts.GetAxesLineWidth2D())));
    frame->SetLineWidth(LineWidth2Int(Int2LineWidth(
        annotationAtts.GetAxesLineWidth2D())));
}


// ****************************************************************************
// Method: VisWindow::UpdateAxes3D
//
// Purpose: 
//   Updates the VisWinAxes3D.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 20, 2001 
//
// Modifications:
//   Kathleen Bonnell, Fri Aug  3 15:29:04 PDT 2001
//   Set visibility of gridlines.
//
//   Brad Whitlock, Fri Sep 21 15:53:34 PST 2001
//   Changed the code so the bbox can be drawn even when the axes are not.
//
//   Kathleen Bonnell, Fri Nov  2 16:43:14 PST 2001 
//   Fix SetTickLocation so that the arg is only 
//   annotationAtts.GetAxes3DTickLocation().
//
//   Eric Brugger, Wed Nov  5 14:03:46 PST 2002
//   Change the names of some of the fields in annotationAtts.
//
//   Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003 
//   Added the setting of the label scaling.
//
//   Brad Whitlock, Thu Jul 28 10:16:47 PDT 2005
//   Added code to set the titles and units.
//
// ****************************************************************************

void
VisWindow::UpdateAxes3D()
{
    //
    // Axes visibility. 
    //
    bool a = annotationAtts.GetAxesFlag();
    axes3D->SetVisibility(a || annotationAtts.GetBboxFlag());

    //
    // Labels
    //
    axes3D->SetXLabelVisibility(a && annotationAtts.GetXAxisLabels());
    axes3D->SetYLabelVisibility(a && annotationAtts.GetYAxisLabels());
    axes3D->SetZLabelVisibility(a && annotationAtts.GetZAxisLabels());
    axes3D->SetLabelScaling(annotationAtts.GetLabelAutoSetScaling(), 
                            annotationAtts.GetXLabelScaling(),
                            annotationAtts.GetYLabelScaling(),
                            annotationAtts.GetZLabelScaling());
    axes3D->SetXTitle(annotationAtts.GetXAxisUserTitle(),
                      annotationAtts.GetXAxisUserTitleFlag());
    axes3D->SetXUnits(annotationAtts.GetXAxisUserUnits(),
                      annotationAtts.GetXAxisUserUnitsFlag());
    axes3D->SetYTitle(annotationAtts.GetYAxisUserTitle(),
                      annotationAtts.GetYAxisUserTitleFlag());
    axes3D->SetYUnits(annotationAtts.GetYAxisUserUnits(),
                      annotationAtts.GetYAxisUserUnitsFlag());
    axes3D->SetZTitle(annotationAtts.GetZAxisUserTitle(),
                      annotationAtts.GetZAxisUserTitleFlag());
    axes3D->SetZUnits(annotationAtts.GetZAxisUserUnits(),
                      annotationAtts.GetZAxisUserUnitsFlag());

    //
    // Ticks
    //
    axes3D->SetXTickVisibility(a && annotationAtts.GetXAxisTicks(),
                               a && annotationAtts.GetXAxisLabels());
    axes3D->SetYTickVisibility(a && annotationAtts.GetYAxisTicks(),
                               a && annotationAtts.GetYAxisLabels());
    axes3D->SetZTickVisibility(a && annotationAtts.GetZAxisTicks(),
                               a && annotationAtts.GetZAxisLabels());

    axes3D->SetTickLocation(annotationAtts.GetAxesTickLocation());

    //
    // Fly Mode (axes3D type).
    //
    axes3D->SetFlyMode(annotationAtts.GetAxesType());

    //
    // Triad 
    //
    triad->SetVisibility(annotationAtts.GetTriadFlag());

    //
    // Bounding Box 
    //
    axes3D->SetBBoxVisibility(annotationAtts.GetBboxFlag());

    //
    // Gridlines 
    //  
    axes3D->SetXGridVisibility(a && annotationAtts.GetXGridLines());
    axes3D->SetYGridVisibility(a && annotationAtts.GetYGridLines());
    axes3D->SetZGridVisibility(a && annotationAtts.GetZGridLines());
}


// ****************************************************************************
// Method: VisWindow::UpdateTextAnnotations
//
// Purpose: 
//   Makes actors that manage certain text annotations update themselves.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 10 08:21:21 PDT 2002
//
// Modifications:
//   Brad Whitlock, Thu Apr 11 12:09:50 PDT 2002
//   Added code to update the database visibility.
//
// ****************************************************************************

void
VisWindow::UpdateTextAnnotations()
{
    // Set the visibility of the user information.
    userInfo->SetVisibility(annotationAtts.GetUserInfoFlag());
    legends->SetVisibility(annotationAtts.GetDatabaseInfoFlag(),
                           annotationAtts.GetLegendInfoFlag());
    plots->TriggerPlotListUpdate();
}

// ****************************************************************************
//  Function: start_render
//
//  Purpose:
//      A function that is accessible for a "VTK callback".  It calls the
//      VisWindow's equivalent function.
//
//  Arguments:
//      p       A pointer to the VisWindowRenderProxy
//
//  Programmer: Hank Childs
//  Creation:   May 4, 2000
//
// ****************************************************************************

static void
start_render(vtkObject *, unsigned long, void *p, void*)
{
    VisWindowRenderProxy  *rp = (VisWindowRenderProxy *) p;
    rp->StartRender();
}


// ****************************************************************************
//  Method: VisWindow::ProcessResizeEvent
//
//  Purpose:
//    A function that is called when a vis window is resized.
//
//  Arguments:
//    data      A pointer to the VisWindow.
//
//  Programmer: Eric Brugger
//  Creation:   August 20, 2001
//
// ****************************************************************************

void
VisWindow::ProcessResizeEvent(void *data)
{
    VisWindow *visWindow = (VisWindow *) data;

    visWindow->UpdateView();
}


// ****************************************************************************
// Method: VisWindow::Pick
//
// Purpose: 
//   Executes the callback that tells the VisWindow's to perform a pick. 
//
// Arguments:
//   x, y      The screen coordinates of the picked point.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 9, 2001 
//
// Modifications:
//   Kathleen Bonnell, Tue Mar  5 09:27:51 PST 2002
//   Changed tolerance value for picker, so that points close to boundaries
//   of the dataset will still yield good results.
//
//   Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002
//   Use queries method, not plots. Change tolerance back to original.  Previous
//   fix was very data-dependent.
//   
//   Kathleen Bonnell, Tue Mar 26 10:43:23 PST 2002 
//   Remove unsued variables xs, ys. 
//   
//   Kathleen Bonnell, Mon May 20 17:01:31 PDT 2002  
//   Retrieve domain from OriginalCells array.  Test for pick position outside
//   of dataset bounds. 
//   
//   Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002  
//   Removed call that set the pick Letter from queries->GetNextDesignator. 
// 
//   Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003 
//   Reworked code to remove picking from the renderer.  Ray-endpoints are 
//   calculated from the screen coordinates,  and passed to the viewer for
//   handling by the engine. 
// 
//   Eric Brugger, Wed Jun 18 17:50:24 PDT 2003
//   I modified the method so that pick worked properly with the new pan
//   and zoom mechanism.
//
//   Kathleen Bonnell, Thu Sep  2 13:40:25 PDT 2004 
//   Added code to call the 'FindIntersection' method if the flag is set. 
//
//   Kathleen Bonnell, Tue Mar  7 08:27:25 PST 2006 
//   Expanded 'intersection only' to handle SR mode.
//
// ****************************************************************************

void
VisWindow::Pick(int x, int y)
{
    if(performPickCallback == 0)
        return;

    if (pickForIntersectionOnly)
    {
        ppInfo->intersectionOnly = true; 
        if (!GetScalableRendering())
        {
            double isect[3];
            ppInfo->validPick = FindIntersection(x, y, isect); 
            if (ppInfo->validPick)
            {
                ppInfo->rayPt1[0] = isect[0];
                ppInfo->rayPt1[1] = isect[1];
                ppInfo->rayPt1[2] = isect[2];
            }
        }
        else
        {
            //        
            // SR mode, and intersect only, need to go to the engine for this
            // so tell pick what the x, y points are by setting them in rayPt.
            //        
            ppInfo->rayPt1[0] = (double) x;
            ppInfo->rayPt1[1] = (double) y;
            ppInfo->validPick = false;
        }
        // Execute the callback.
        (*performPickCallback)((void*)ppInfo);
        return;    
    }

    ppInfo->intersectionOnly = false; 
    double cameraPos[4];
    double cameraFocal[4];
    double pickPos[3];
    double cameraDOP[3];
    double ray[3];
    double tF, tB; 
    double rayPt1[4], rayPt2[4];
    double *displayCoords;
    double *clipRange;
    double rayLength;
    vtkRenderer *ren = GetCanvas();
    vtkCamera *cam = ren->GetActiveCamera();

    cam->GetPosition(cameraPos);
    cam->GetFocalPoint(cameraFocal);
    cameraPos[3] = cameraFocal[3] = 1.0;
    bool validPick;
    int i;


    ren->SetWorldPoint(cameraFocal);
    ren->WorldToDisplay();
    displayCoords = ren->GetDisplayPoint();

    //
    // Transform the point from display to view coordinates.
    //
    double viewPoint[4];

    ren->SetDisplayPoint(x, y, displayCoords[2]);
    ren->DisplayToView();
    double *viewCoords = ren->GetViewPoint();

    viewPoint[0] = viewCoords[0];
    viewPoint[1] = viewCoords[1];
    viewPoint[2] = viewCoords[2];
    viewPoint[3] = 1.0;

    // Compensate for window centering and scaling.
    double *windowCenter = cam->GetWindowCenter();
    double focalDisk = cam->GetFocalDisk();
    double *aspect = ren->GetAspect();

    viewPoint[0] = viewPoint[0] -
        (aspect[0] - 1.) * windowCenter[0] * focalDisk;
    viewPoint[1] = viewPoint[1] -
        (aspect[1] - 1.) * windowCenter[1] * focalDisk;

    //
    // Transform the point from view to world coordinates.
    //
    double worldCoords[4];
    vtkMatrix4x4 *mat = vtkMatrix4x4::New();
 
    // get the perspective transformation from the active camera
    mat->DeepCopy(cam->GetCompositePerspectiveTransformMatrix(1,0,1));
 
    // use the inverse matrix
    mat->Invert();
 
    mat->MultiplyPoint(viewPoint, worldCoords);
 
    // Get the transformed vector & set WorldPoint
    // while we are at it try to keep w at one
    if (worldCoords[3])
    {
        worldCoords[0] = worldCoords[0] / worldCoords[3];
        worldCoords[1] = worldCoords[1] / worldCoords[3];
        worldCoords[2] = worldCoords[2] / worldCoords[3];
        worldCoords[3] = 1;
    }
    mat->Delete();

    if ( worldCoords[3] == 0.0 )
    {
        debug5 << "vtkCellPicker could not calculate pick ray."  << endl;
        validPick = false;
    }
    else
    {
        for (i=0; i < 3; i++)
        {
            pickPos[i] = worldCoords[i] / worldCoords[3];
        }
        //  
        //  Compute the ray endpoints.  The ray is along the line running from
        //  the camera position to the selection point, starting where this line
        //  intersects the front clipping plane, and terminating where this
        //  line intersects the back clipping plane.
        //  
        for (i=0; i<3; i++)
        {
            ray[i] = pickPos[i] - cameraPos[i];
        }
        for (i=0; i<3; i++)
        {
            cameraDOP[i] = cameraFocal[i] - cameraPos[i];
        }
 
        vtkMath::Normalize(cameraDOP);

 
        if (( rayLength = vtkMath::Dot(cameraDOP,ray)) == 0.0 )
        {
            debug5 << "vtkCellPicker could not calculate pick ray."  << endl;
            validPick = false;
        }
        else
        {
            clipRange = cam->GetClippingRange();

            validPick = true;
 
            if ( cam->GetParallelProjection() )
            {
                tF = clipRange[0] - rayLength;
                tB = clipRange[1] - rayLength;
                for (i=0; i<3; i++)
                {
                    rayPt1[i] = pickPos[i] + tF*cameraDOP[i];
                    rayPt2[i] = pickPos[i] + tB*cameraDOP[i];
                }
            }
            else
            {
                tF = clipRange[0] / rayLength;
                tB = clipRange[1] / rayLength;
                for (i=0; i<3; i++)
                {
                    rayPt1[i] = cameraPos[i] + tF*ray[i];
                    rayPt2[i] = cameraPos[i] + tB*ray[i];
                }
            }
            rayPt1[3] = rayPt2[3] = 1.0;
        }
    }

    ppInfo->validPick = validPick;
    if (validPick)
    {
        ppInfo->rayPt1[0] = rayPt1[0];
        ppInfo->rayPt1[1] = rayPt1[1];
        ppInfo->rayPt1[2] = rayPt1[2];
        ppInfo->rayPt2[0] = rayPt2[0];
        ppInfo->rayPt2[1] = rayPt2[1];
        ppInfo->rayPt2[2] = rayPt2[2];
    }

    // Execute the callback.
    (*performPickCallback)((void*)ppInfo);
}

// ****************************************************************************
// Method: VisWindow::Pick
//
// Purpose: 
//   Calls the Pick method with window coordinates [0,1]
//
// Arguments:
//   sx, sy : The point on which to pick.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 5 14:43:11 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::Pick(double sx, double sy)
{
    vtkRenderer *ren = GetCanvas();
    
    if(ren->GetRenderWindow())
    {
        int *windowSize = ren->GetRenderWindow()->GetSize();
        if(windowSize)
        {
            int x = int(sx * windowSize[0]);
            int y = int(sy * windowSize[1]);
            Pick(x, y);
        }
    }
}

// ****************************************************************************
// Method: VisWindow::SetPickCB
//
// Purpose: 
//   Sets the callback to use to perform a pick. 
//
// Arguments:
//   cb        The callback method.
//   data      The callback data. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 9, 2001 
//
// ****************************************************************************

void
VisWindow::SetPickCB(VisCallback *cb, void *data)
{
    performPickCallback= cb;
    ppInfo = (PICK_POINT_INFO *)data;
}


// ****************************************************************************
// Method: VisWindow::ClearPickPoints
//
// Purpose: 
//   Tell the plots to clear pick points.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 27, 2001 
//
// Modifications:
//   Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002
//   Call queries method, not plots.
//   
//   Kathleen Bonnell, Wed Mar 26 14:29:23 PST 2003   
//   Force a render so pick points always disappear from window.
//   
// ****************************************************************************

void
VisWindow::ClearPickPoints()
{
    queries->ClearPickPoints();
    Render();
}


// ****************************************************************************
// Method: VisWindow::Lineout
//
// Purpose: 
//   Executes the callback that tells the VisWindow's to perform a line-out. 
//
// Arguments:
//   x1, y1, x2, y2      The screen coordinates of the endpoints of the line.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 17, 2001 
//
// Modifications:
//   Kathleen Bonnell, Wed May 15 16:40:35 PDT 2002
//   Catch exeception thrown by GetDataExtents. (Indicates bad variable for Lineout).
//
//   Kathleen Bonnell, Thu Jun 20 14:58:18 PDT 2002  
//   Moved code for determing YScale to ViewerQuery. 
//
//   Kathleen Bonnell, Tue Jul 23 15:01:55 PDT 2002 
//   Removed code for determining glyphSize. 
//
//   Kathleen Bonnell, Tue Jul 30 13:35:04 PDT 2002   
//   Set z-coord to 0 (for 2d) BEFORE the points are set for VisWinQuery. 
//
//   Kathleen Bonnell, Fri Dec 20 09:48:48 PST 2002  
//   Remove call to queries->GetNextDesignator.
//
//    Kathleen Bonnell, Fri Jun 27 16:30:26 PDT 2003 
//    Removed call to queries->SetAttachmentPoint, queries->SetSecondaryPoint. 
//
// ****************************************************************************

void
VisWindow::Lineout(int x1, int y1, int x2, int y2)
{
    if(performLineoutCallback == 0)
        return;
 
    //
    // Retrieve the objects we want to modify.
    //
    vtkRenderer *canvas = GetCanvas();

    double pt1[3], pt2[3];
    pt1[2] = pt2[2] = 0.;

    pt1[0] = (double) x1;
    pt1[1] = (double) y1;
    pt2[0] = (double) x2;
    pt2[1] = (double) y2;
   
    //
    // Have the canvas translate our endpoints in display coordinates
    // to endpoints in world coordinate.  Sadly, this takes five
    // operations each.
    //
    canvas->DisplayToNormalizedDisplay(pt1[0], pt1[1]);
    canvas->NormalizedDisplayToViewport(pt1[0], pt1[1]);
    canvas->ViewportToNormalizedViewport(pt1[0], pt1[1]);
    canvas->NormalizedViewportToView(pt1[0], pt1[1], pt1[2]);
    canvas->ViewToWorld(pt1[0], pt1[1], pt1[2]);

    canvas->DisplayToNormalizedDisplay(pt2[0], pt2[1]);
    canvas->NormalizedDisplayToViewport(pt2[0], pt2[1]);
    canvas->ViewportToNormalizedViewport(pt2[0], pt2[1]);
    canvas->NormalizedViewportToView(pt2[0], pt2[1], pt2[2]);
    canvas->ViewToWorld(pt2[0], pt2[1], pt2[2]);

    // We want the z-coord to be 0., because we should only be working
    // with 2D data.  We don't get a 0. for z-coord when we translate the
    // endpoints because set them in the Foreground which is closer to the
    // camera than the canvas.

    if (mode == WINMODE_2D)
    {
        pt1[2] = pt2[2] = 0.;
    }

    double dpt1[3] = {pt1[0], pt1[1], pt1[2]};
    double dpt2[3] = {pt2[0], pt2[1], pt2[2]};
    loInfo->atts.SetPoint1(dpt1);
    loInfo->atts.SetPoint2(dpt2);

    // Execute the callback.
    (*performLineoutCallback)((void*)loInfo);
}

// ****************************************************************************
// Method: VisWindow::GetVisualCues
//
// Purpose: 
//   Returns a vector of visual cues currently in the VisWindow 
//
// Programmer: Mark C. Miller 
// Creation:   June 7, 2004 
//
// ****************************************************************************
void
VisWindow::GetVisualCues(const VisualCueInfo::CueType cueType,
    std::vector<const VisualCueInfo*>& cues) const
{
    queries->GetVisualCues(cueType, cues);
}


// ****************************************************************************
// Method: VisWindow::SetExternalRenderCallback
//
// Purpose: 
//   Forward a request to register an external rendering callback function to
//   the VisWinPlots.
//
// Programmer: Mark C. Miller
// Creation:   January 13, 2003 
//
// ****************************************************************************

// new name for same function
void
VisWindow::SetExternalRenderCallback(VisCallbackWithDob *cb, void *data)
{
   plots->SetExternalRenderCallback(cb, data);
}

// ****************************************************************************
// Method: VisWindow::EnableExternalRenderRequests
//
// Purpose: 
//   Forward a request to enable external render requests to the VisWinPlots
//
// Programmer: Mark C. Miller
// Creation:   February 5, 2003 
//
// ****************************************************************************

bool
VisWindow::EnableExternalRenderRequests(void)
{
   return plots->EnableExternalRenderRequests();
}

// ****************************************************************************
// Method: VisWindow::DisableExternalRenderRequests
//
// Purpose: 
//   Forward a request to enable external render requests to the VisWinPlots
//
// Programmer: Mark C. Miller
// Creation:   February 5, 2003 
//
// ****************************************************************************

bool
VisWindow::DisableExternalRenderRequests(void)
{
   return plots->DisableExternalRenderRequests();
}

// ****************************************************************************
// Method: VisWindow::ComputeVectorTextScaleFactor
//
// Purpose: 
//   Tell the queries to clear ref lines (from LineOut).
//
// Programmer: Kathleen Bonnell 
// Creation:   May 8, 2002 
//
// ****************************************************************************

double
VisWindow::ComputeVectorTextScaleFactor(const double *pos, const double *vp)
{
    return rendering->ComputeVectorTextScaleFactor(pos, vp);
}




// ****************************************************************************
// Method: VisWindow::ClearRefLines
//
// Purpose: 
//   Tell the queries to clear ref lines (from LineOut).
//
// Programmer: Kathleen Bonnell 
// Creation:   January 14, 2002 
//
// Modifications:
//   Kathleen Bonnell, Wed Mar 26 14:29:23 PST 2003   
//   Force a render so reflines always disappear from window.
//   
// ****************************************************************************

void
VisWindow::ClearRefLines()
{
    queries->ClearLineouts();
    Render();
}


// ****************************************************************************
// Method: VisWindow::SetLineoutCB
//
// Purpose: 
//   Sets the callback to use to perform a lineout. 
//
// Arguments:
//   cb        The callback method.
//   data      The callback data. 
//
// Programmer: Kathleen Bonnell 
// Creation:   January 14, 2002 
//
// ****************************************************************************

void
VisWindow::SetLineoutCB(VisCallback *cb, void *data)
{
    performLineoutCallback= cb;
    loInfo = (LINE_OUT_INFO *)data;
}


// ****************************************************************************
//  Method: VisWindow::QueryIsValid
//
//  Purpose:
//    Tells the query colleague that the lastt query was valid.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 15, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002
//    Changed argument to Line*, to convey more information than just color.
//
//    Kathleen Bonnell, Thu Dec 19 13:32:47 PST 2002  
//    Added argument designator.
//
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003 
//    Replaced argument designator with PickAttributes.
//
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Changed interface to use VisualCueInfo object
//
// ****************************************************************************

void
VisWindow::QueryIsValid(const VisualCueInfo *pickCue, const VisualCueInfo *lineCue) 
{
    queries->QueryIsValid(pickCue, lineCue);
}


// ****************************************************************************
//  Method: VisWindow::GetScaleFactorAndType
//
//  Purpose:
//      Gets the VisWindow's axis scale factor and type. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 13, 2003 
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 10:09:15 PDT 2003
//    I modified the routine to calculate the scale in the curve case
//    from the domain and range.
//
//    Eric Brugger, Thu Oct 16 09:26:49 PDT 2003
//    Modified to match changes in avtView2D and avtViewCurve made to handle
//    full frame mode properly.
//
// ****************************************************************************

void
VisWindow::GetScaleFactorAndType(double &s, int &t) 
{
    if (mode == WINMODE_2D)
    {
        int *size=rendering->GetFirstRenderer()->GetSize();

        s = view2D.GetScaleFactor(size);
    }
    else if (mode == WINMODE_CURVE)
    {
        int *size=rendering->GetFirstRenderer()->GetSize();

        s = viewCurve.GetScaleFactor(size);
    }
    else // this really doesn't apply, set scale to 0. 
    {
        s = 0.; // no scaling will happen.
    }
    t = 1; // y_axis
}


// ****************************************************************************
//  Method: VisWindow::UpdateQuery
//
//  Purpose:
//    Tells the query colleague to do an update. 
//
//  Arguments:
//    lineAtts  Attributes that specify which query is to be updated.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 18, 2002 
//
//  Modifications:
//
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Changed interface to use VisualCueInfo object
// ****************************************************************************
 
void
VisWindow::UpdateQuery(const VisualCueInfo *lineCue)
{
    queries->UpdateQuery(lineCue);
}


// ****************************************************************************
//  Method: VisWindow::DeleteQuery
//
//  Purpose:
//    Tells the query colleague to do delete the query specifed by lineAtts. 
//
//  Arguments:
//    lineAtts  Attributes that specify which query is to be deleted.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 18, 2002 
//
//  Modifications:
//
//    Mark C. Miller Wed Jun  9 17:44:38 PDT 2004
//    Changed interface to use VisualCueInfo object
// ****************************************************************************
 
void
VisWindow::DeleteQuery(const VisualCueInfo *lineCue)
{
    queries->DeleteQuery(lineCue);
}


// ****************************************************************************
//  Method: VisWindow::ScalePlots
//
//  Purpose:
//    Tells the plots colleague to do scale the plots as specifed by vec. 
//
//  Arguments:
//    vec   The vector by which plots are to be scaled. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 11, 2002 
//
// ****************************************************************************
 
void
VisWindow::ScalePlots(const double vec[3])
{
    plots->ScalePlots(vec);
}


// ****************************************************************************
//  Method: VisWindow::GetAmbientOn
//
//  Purpose:
//    Retrieves the lighting flag specifying where ambient light is on or off. 
//
//  Returns:
//    True if ambient lighting is in effect, false otherwise. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 12, 2002 
//
// ****************************************************************************
 
bool
VisWindow::GetAmbientOn()
{
    return lighting->GetAmbientOn();
}


// ****************************************************************************
//  Method: VisWindow::GetAmbientCoefficient
//
//  Purpose:
//    Retrieves the ambient lighting coefficient. 
//
//  Returns:
//    The ambient lighting coefficient. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 12, 2002 
//
// ****************************************************************************
 
double
VisWindow::GetAmbientCoefficient()
{
    return lighting->GetAmbientCoefficient();
}

// ****************************************************************************
//  Method: VisWindow::GetLighting
//
//  Purpose:
//    Retrieves the flag specifying whether lighing is on or off. 
//
//  Returns:
//    True if lighting is in effect, false otherwise. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 12, 2002 
//
// ****************************************************************************
 
bool
VisWindow::GetLighting()
{
    return (lighting->GetNumLightsEnabled() > 0);
}


// ****************************************************************************
//  Method: VisWindow::UpdateLightPositions
//
//  Purpose:
//    Tells the lighting colleague that light positions need updating.
//    This occurs mainly when plots updpate the camera based on new bounds. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   August 13, 2002 
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug 29 09:49:36 PDT 2002
//    Removed arguments from lighting->UpdateLightPositions.  Added test
//    for NULL. 
//
// ****************************************************************************

void
VisWindow::UpdateLightPositions()
{
    if (lighting!=NULL)
    {
        lighting->UpdateLightPositions();
    }
}

// ****************************************************************************
// Method: VisWindow::SetRenderInfoCallback
//
// Purpose: 
//   Sets the rendering information callback function.
//
// Arguments:
//   cb   : The function pointer to call when rendering information is updated.
//   data : The data to pass to the callback function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:05:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetRenderInfoCallback(VisCallback *cb, void *data)
{
    rendering->SetRenderInfoCallback(cb, data);
}

// ****************************************************************************
// Method: VisWindow::SetAntialiasing
//
// Purpose: 
//   Sets the window's antialiasing mode.
//
// Arguments:
//   enabled : Whether or not antialiasing is enabled.
//   frames  : The number of frames to use for AA.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:06:02 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Dec  4 17:29:34 PST 2002  
//   Removed frames argument.
//   
//   Kathleen Bonnell, Mon Sep 29 13:15:20 PDT 2003 
//   Added call to RecalculateRenderOrder. 
//   
// ****************************************************************************

void
VisWindow::SetAntialiasing(bool enabled)
{
    if (enabled != rendering->GetAntialiasing())
    {
        rendering->SetAntialiasing(enabled);
        RecalculateRenderOrder();
    }
}

// ****************************************************************************
// Method: VisWindow::GetAntialiasing
//
// Purpose: 
//   Returns the window's AA mode.
//
// Returns:    The window's AA mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:06:46 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::GetAntialiasing() const
{
    return rendering->GetAntialiasing();
}

// ****************************************************************************
// Method: VisWindow::GetRenderTimes
//
// Purpose: 
//   Returns the window's min,avg, and max render times.
//
// Arguments:
//   times : The return array for the render times.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:07:40 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//   Added 3 most recent rendering times to set of times returned
//   
// ****************************************************************************

void
VisWindow::GetRenderTimes(double times[6]) const
{
    rendering->GetRenderTimes(times);
}

// ****************************************************************************
// Method: VisWindow::SetStereoRendering
//
// Purpose: 
//   Sets the window's stereo mode.
//
// Arguments:
//   enabled : Whether or not stereo is enabled.
//   type    : The type of stereo rendering to do.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:08:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetStereoRendering(bool enabled, int type)
{
    rendering->SetStereoRendering(enabled, type);
}

// ****************************************************************************
// Method: VisWindow::GetStereo
//
// Purpose: 
//   Returns whether or not stereo rendering is enabled.
//
// Returns:    Whether or not stereo rendering is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:09:04 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::GetStereo() const
{
    return rendering->GetStereo();
}

// ****************************************************************************
// Method: VisWindow::GetStereoType
//
// Purpose: 
//   Returns the window's stereo rendering type.
//
// Returns:    The window's stereo rendering type.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:09:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
VisWindow::GetStereoType() const
{
    return rendering->GetStereoType();
}

// ****************************************************************************
// Method: VisWindow::GetImmediateModeRendering
//
// Purpose: 
//   Declares whether or not we should do immediate mode rendering.
//
// Programmer: Hank Childs
// Creation:   May 9, 2004
//
// ****************************************************************************

bool
VisWindow::GetImmediateModeRendering(void)
{
    int mode = GetDisplayListMode();
    if (mode == 0)
        return true;
    if (mode == 1)
        return false;

    return IsDirect();
}


// ****************************************************************************
// Method: VisWindow::SetDisplayListMode
//
// Purpose: 
//   Tells the window what mode to use for display lists.
//
// Arguments:
//   mode : The new display list mode
//
// Programmer: Hank Childs
// Creation:   May 9, 2004
//
// ****************************************************************************

void
VisWindow::SetDisplayListMode(int mode)
{
    rendering->SetDisplayListMode(mode);
    bool immediateMode = GetImmediateModeRendering();

    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetImmediateModeRendering(immediateMode);
    }
}

// ****************************************************************************
// Method: VisWindow::GetDisplayListMode
//
// Purpose: 
//   Returns the display list mode for the window.
//
// Programmer: Hank Childs
// Creation:   May 10, 2004
//
// ****************************************************************************

int
VisWindow::GetDisplayListMode(void) const
{
    return rendering->GetDisplayListMode();
}

// ****************************************************************************
// Method: VisWindow::IsDirect
//
// Purpose: 
//     Returns whether or not the window is rendering directly to a GPU or
//     whether it is going through an X-Server.
//
// Programmer: Hank Childs
// Creation:   May 9, 2004
//
// ****************************************************************************

bool
VisWindow::IsDirect(void)
{
    return rendering->IsDirect();
}

// ****************************************************************************
// Method: VisWindow::SetSurfaceRepresentation
//
// Purpose: 
//   Sets the window's surface representation.
//
// Arguments:
//   rep : The window's new surface representation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:11:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetSurfaceRepresentation(int rep)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetSurfaceRepresentation(rep);
    }
}

// ****************************************************************************
// Method: VisWindow::GetSurfaceRepresentation
//
// Purpose: 
//   Gets the window's surface representation.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:11:50 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
VisWindow::GetSurfaceRepresentation() const
{
    return rendering->GetSurfaceRepresentation();
}

// ****************************************************************************
// Method: VisWindow::SetSpecularProperties
//
// Purpose: 
//   Sets the window's specular properties.
//
// Arguments:
//      coeff :  the new specular coefficient
//      power :  the new specular power
//      color :  the new specular color
//
// Programmer: Jeremy Meredith
// Creation:   November 14, 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetSpecularProperties(bool flag, double coeff, double power,
                                 const ColorAttribute &color)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->SetSpecularProperties(flag, coeff, power, color);
    }
}


// ****************************************************************************
//  Method:  VisWindow::GetSpecularCoeff
//
//  Purpose:
//    Get the window's specular coefficient.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

double
VisWindow::GetSpecularCoeff()
{
    return rendering->GetSpecularCoeff();
}

// ****************************************************************************
//  Method:  VisWindow::GetSpecularFlag
//
//  Purpose:
//    Get the window's specular flag.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

bool
VisWindow::GetSpecularFlag()
{
    return rendering->GetSpecularFlag();
}

// ****************************************************************************
//  Method:  VisWindow::GetSpecularPower
//
//  Purpose:
//    Get the window's specular power.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

double
VisWindow::GetSpecularPower()
{
    return rendering->GetSpecularPower();
}

// ****************************************************************************
//  Method:  VisWindow::GetSpecularColor
//
//  Purpose:
//    Get the window's specular color.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 14, 2003
//
// ****************************************************************************

const ColorAttribute &
VisWindow::GetSpecularColor()
{
    return rendering->GetSpecularColor();
}

// ****************************************************************************
// Method: VisWindow::GetNumPrimitives
//
// Purpose: 
//   Gets the approximate number of triangles that were rendered.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:12:15 PST 2002
//
// Modifications:
//
//   Mark C. Miller, Thu Mar  3 17:38:36 PST 2005
//   Changed name from GetNumTriangles to GetNumPrimitives
//   
// ****************************************************************************

int
VisWindow::GetNumPrimitives() const
{
    return rendering->GetNumPrimitives();
}

// ****************************************************************************
// Method: VisWindow::SetNotifyForEachRender
//
// Purpose: 
//   Sets a flag that tells the window whether or not it should report
//   rendering information after each render.
//
// Arguments:
//   val : The new notify mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:12:46 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetNotifyForEachRender(bool val)
{
    rendering->SetNotifyForEachRender(val);
}

// ****************************************************************************
// Method: VisWindow::GetNotifyForEachRender
//
// Purpose: 
//   Gets the window's rendering information notification mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 23 14:13:25 PST 2002
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::GetNotifyForEachRender() const
{
    return rendering->GetNotifyForEachRender();
}

// ****************************************************************************
// Method: VisWindow::SetScalableRendering
//
// Purpose: 
//   Enables (true) or disables (false) scalable rendering mode 
//
// Programmer: Mark C. Miller 
// Creation:   Tue Dec  3 19:15:37 PST 2002 
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetScalableRendering(bool mode)
{
    rendering->SetScalableRendering(mode);
}

// ****************************************************************************
// Method: VisWindow::GetScalableRendering
//
// Purpose: 
//   returns true if scalable rendering is enabled, false if not 
//
// Programmer: Mark C. Miller 
// Creation:   Tue Dec  3 19:15:37 PST 2002 
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::GetScalableRendering() const
{
    return rendering->GetScalableRendering();
}

// ****************************************************************************
// Method: VisWindow::GetScalableThreshold
//
// Purpose: 
//   returns scalable rendering threshold
//
// Programmer: Mark C. Miller 
// Creation:   Tue Dec  3 19:15:37 PST 2002 
//
// Modifications:
//   
// ****************************************************************************

int
VisWindow::GetScalableThreshold() const
{
    return rendering->GetScalableThreshold();
}


// ****************************************************************************
// Method: VisWindow::SetScalableActivationMode
//
// Programmer: Mark C. Miller 
// Creation:   May 11, 2004 
//
// ****************************************************************************

void
VisWindow::SetScalableActivationMode(int mode)
{
    rendering->SetScalableActivationMode(mode);
}

// ****************************************************************************
// Method: VisWindow::GetScalableActivationMode
//
// Programmer: Mark C. Miller 
// Creation:   May 11, 2004 
//
// ****************************************************************************

int
VisWindow::GetScalableActivationMode() const
{
    return rendering->GetScalableActivationMode();
}

// ****************************************************************************
// Method: VisWindow::SetScalableAutoThreshold
//
// Programmer: Mark C. Miller 
// Creation:   May 11, 2004 
//   
// ****************************************************************************

void
VisWindow::SetScalableAutoThreshold(int threshold)
{
    rendering->SetScalableAutoThreshold(threshold);
}

// ****************************************************************************
// Method: VisWindow::GetScalableAutoThreshold
//
// Programmer: Mark C. Miller 
// Creation:   May 11, 2004 
//
// ****************************************************************************

int
VisWindow::GetScalableAutoThreshold() const
{
    return rendering->GetScalableAutoThreshold();
}

// ****************************************************************************
// Method: VisWindow::CreateToolbar
//
// Purpose: 
//   Creates a toolbar widget and returns a pointer to it.
//
// Arguments:
//   name : The name of the toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 14:34:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void *
VisWindow::CreateToolbar(const char *name)
{
    return rendering->CreateToolbar(name);
}

// ****************************************************************************
// Method: VisWindow::SetLargeIcons
//
// Purpose: 
//   Tells the window to use large icons.
//
// Arguments:
//   val : Whether we should use large icons.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:51:44 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::SetLargeIcons(bool val)
{
    rendering->SetLargeIcons(val);
}

// ****************************************************************************
// Method: VisWindow::ReAddColleaguesToRenderWindow
//
// Purpose: 
//   Allow colleagues to re-add themselves to the render window, in order
//   to be rendered after plots when in antialiasing mode. 
//
// Programmer: Kathleen Bonnell
// Creation:   May 28, 2003
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::ReAddColleaguesToRenderWindow(void)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->ReAddToWindow();
    }
}


// ****************************************************************************
//  Method: VisWindow::FullFrameOff
//
//  Purpose:
//    Tells colleagues that FullFrameMode has been turned off. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 6, 2003 
//
// ****************************************************************************
 
void
VisWindow::FullFrameOff()
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->FullFrameOff();
    }
}


// ****************************************************************************
//  Method: VisWindow::FullFrameOn
//
//  Purpose:
//    Tells colleagues that FullFrameMode has been turned on. 
//
//  Arguments:
//    scale     The axis scale factor.
//    type      The axis scale type.
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 6, 2003 
//
// ****************************************************************************

void
VisWindow::FullFrameOn(const double scale, const int type)
{
    std::vector< VisWinColleague * >::iterator it;
    for (it = colleagues.begin() ; it != colleagues.end() ; it++)
    {
        (*it)->FullFrameOn(scale, type);
    }
}


// ****************************************************************************
//  Method: VisWindow::GetFullFrameMode
//
//  Purpose:
//    Returns the status of full-frame mode.
//
//  Returns: true if window mode is 2d, and full frame is on, false otherwise. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   June 6, 2003 
//
//  Modifications:
//    Eric Brugger, Thu Oct 16 09:26:49 PDT 2003
//    Modified to match changes in avtView2D made to handle full frame
//    mode properly.
//
//    Kathleen  Bonnell, Tue Dec  2 16:36:00 PST 2003 
//    CurveMode is always in FullFrameMode. 
//
// ****************************************************************************

bool
VisWindow::GetFullFrameMode()
{
    if ((mode == WINMODE_2D && view2D.fullFrame) ||
        (mode == WINMODE_CURVE))
        return true;
    else 
        return false; 
}

// ****************************************************************************
// Method: VisWindow::TransparenciesExist
//
// Purpose: 
//   Returns whether or not there are transparent actors in the plots. 
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2003 
//
// Modifications:
//   
// ****************************************************************************

bool
VisWindow::TransparenciesExist()
{
    return plots->TransparenciesExist();
}


// ****************************************************************************
// Method: VisWindow::ReAddToolsToRenderWindow
//
// Purpose: 
//   Allow tools to re-add themselves to the render window, in order
//   to be rendered after plots but before tranparent actors. 
//
// Programmer: Kathleen Bonnell
// Creation:   December 3, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
VisWindow::ReAddToolsToRenderWindow(void)
{
    tools->ReAddToolsToWindow();
}


// ****************************************************************************
// Method: VisWindow::SetInteractorAtts
//
// Purpose: 
//   Sets the interactor attributes used to control zoom, etc.
//
//  Arguments:
//    atts     The interactor attributes to use. 
//
// Programmer: Kathleen Bonnell 
// Creation:   July 16, 2004 
//
// Modifications:
//   Eric Brugger, Fri Nov 12 14:51:13 PST 2004
//   Add code to update the interactors if the attributes changed.
//
// ****************************************************************************

void
VisWindow::SetInteractorAtts(const InteractorAttributes *atts)
{
    bool changed = (interactorAtts != *atts);

    if (changed)
    {
        interactorAtts = *atts;

        //
        // Update the interactors.  This is necessary to handle a change
        // in navigation mode.
        //
        interactions->SetInteractionMode(interactions->GetInteractionMode());
    }
}

// ****************************************************************************
// Method: VisWindow::GetInteractorAttributes()
//
// Purpose: 
//   Returns a pointer to the window's interactor attributes.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004 
//   
// ****************************************************************************

const InteractorAttributes *
VisWindow::GetInteractorAtts() const
{
    return (const InteractorAttributes *)&interactorAtts;
}


// ****************************************************************************
//  Method: VisWindow::FindIntersection
//
//  Purpose: 
//    Uses a vtkCellPicker to find the world coordinate corresponding to the
//    specified screen coordinates. 
//
//  Arguments:
//    x         The screen x-coordinate.
//    y         The screen y-coordinate.
//    isect     A place to store the intersection point.
//
//  Returns:    True if an intersection with rendered data was found, 
//              false otherwise.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//   
//  Modifications:
//    Kathleen Bonnell, Thu Nov  4 16:46:31 PST 2004
//    Make plots pickable before performing intersection, and unpickable after.
// ****************************************************************************

bool
VisWindow::FindIntersection(const int x, const int y, double isect[3])
{
    vtkRenderer *ren = GetCanvas();
    if(!ren->GetRenderWindow())
    {
        return false;
    }

    plots->MakeAllPickable();
    bool success;
    vtkCellPicker *picker = vtkCellPicker::New();
    picker->SetTolerance(1.0e-6);
    picker->Pick(x, y, 0, ren);
   
    int cell = picker->GetCellId();

    if ( cell < 0 )
    {
       debug5 << "vtkCellPicker found no intersection with surface."  << endl;
       success = false;
    }
    else
    {
        vtkDataSet *ds = picker->GetDataSet();
        if (ds == NULL)
        {
            success = false;
            debug5 << "vtkCellPicker returned NULL dataset." << endl;
        }
        else 
        {
            success = true;
            isect[0] = picker->GetPickPosition()[0];
            isect[1] = picker->GetPickPosition()[1];
            isect[2] = picker->GetPickPosition()[2];
        }
    }
    picker->Delete();
    plots->MakeAllUnPickable();
    return success;
}


// ****************************************************************************
//  Method: VisWindow::SetPickTypeToIntersection
//
//  Purpose: 
//    Sets a flag specifiying that the PickMethod should perform an
//    intersection calculation. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//   
//  Modifications:
//
// ****************************************************************************

void
VisWindow::SetPickTypeToIntersection()
{
    pickForIntersectionOnly = true;
}


// ****************************************************************************
//  Method: VisWindow::SetPickTypeToNormal
//
//  Purpose: 
//    Sets a flag specifiying that the PickMethod should NOT perform an
//    intersection calculation, but perform in the default manner, constructing
//    ray points from the screen coordinates. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   September 2, 2004 
//   
//  Modifications:
//
// ****************************************************************************

void
VisWindow::SetPickTypeToNormal()
{
    pickForIntersectionOnly = false;
}


// ****************************************************************************
//  Method: VisWindow::SuspendOpaqueGeometry
//
//  Purpose:
//    Make opaque geometry invisible
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:43:16 PDT 2004
//
// ***************************************************************************

void
VisWindow::SuspendOpaqueGeometry()
{
    plots->SuspendOpaqueGeometry();
}


// ****************************************************************************
//  Method: VisWindow::SuspendTranslucentGeometry
//
//  Purpose:
//    Make translucent geometry invisible
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:43:22 PDT 2004
//
// ***************************************************************************

void
VisWindow::SuspendTranslucentGeometry()
{
    plots->SuspendTranslucentGeometry();
}


// ****************************************************************************
//  Method: VisWindow::ResumeOpaqueGeometry
//
//  Purpose:
//    Make opaque geometry visible again
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:43:04 PDT 2004
//
// ***************************************************************************

void
VisWindow::ResumeOpaqueGeometry()
{
    plots->ResumeOpaqueGeometry();
}

// ****************************************************************************
//  Method: VisWinPlots::ResumeTranslucentGeometry
//
//  Purpose:
//    Make tranlucent geometry visible again
//
//  Programmer: Chris Wojtan
//  Creation:   Mon Jul 26 15:42:57 PDT 2004
//
// ***************************************************************************

void
VisWindow::ResumeTranslucentGeometry()
{
    plots->ResumeTranslucentGeometry();
}


// ****************************************************************************
// Method: VisWindow::GlyphPick
//
// Purpose: 
//   Finds the domain and cell/node number intersected by a ray.
//
// Arguments:
//   rp1       The origin of the ray.
//   rp2       The endpoint of the ray.
//   dom       A place to store the intersected domain number. 
//   elNum     A place to store the intersected cell/node number. 
//   forCell   A flag returned that specifies whether Pick shoulde be
//             a cell pick or node pick. 
//   doRender  A flag specifying if a Render request should be made.
//
// Programmer: Kathleen Bonnell 
// Creation:   October 11, 2004 
//
// Modifications:
//   Kathleen Bonnell, Tue Nov  2 10:18:16 PST 2004
//   Added support for full-frame mode.
//
//   Kathleen Bonnell, Mon Nov  8 15:47:32 PST 2004
//   Moved support for full-frame mode to ViewerQueryManager.
//
//   Jeremy Meredith, Wed Aug 17 16:37:03 PDT 2005
//   Added support for unglyphed point meshes.  This simply returns
//   the point closest to the pick ray, independent of depth.
//
// ****************************************************************************
void
VisWindow::GlyphPick(const double *rp1, const double *rp2, int &dom, int &elNum, 
                     bool &forCell, const bool doRender)
{
    double dummy  = +FLT_MAX;
    GlyphPick(rp1, rp2, dom, elNum, forCell, dummy, doRender);
}

void
VisWindow::GlyphPick(const double *rp1, const double *rp2, int &dom, 
                     int &elNum, bool &forCell, double & d, const bool doRender)
{
    //
    // First things first, we must make sure that the datasets have been
    // truly rendered.
    //
    if (doRender)
        GetCanvas()->GetRenderWindow()->Render();

    int i, cell = -1;
    double *bnds = NULL;
    double dir[3];
    double r1[3] = {rp1[0], rp1[1], rp1[2]};
    double r2[3] = {rp2[0], rp2[1], rp2[2]};

    for (i = 0; i < 3; i++)
        dir[i] = r2[i] - r1[i];
    double dummy1[3], dummy2;
    vtkActorCollection *actors = GetCanvas()->GetActors();
    vtkActor *actor = NULL;
    vtkDataSet *ds = NULL;
    vtkDataSet *good_ds = NULL;
    double minDist = FLT_MAX;
    double dist;

    for (actors->InitTraversal(); (actor = actors->GetNextActor());) 
    {
        if (actor->GetPickable() && actor->GetVisibility())
        {
            ds = actor->GetMapper()->GetInput();
            if (ds == NULL)
            {
                debug5 << "GlyphPick strangeness, dataset from mapper"
                       << " is NULL!" << endl;
                continue;
            }
            bnds = ds->GetBounds();
            if (vtkBox::IntersectBox(bnds, r1, dir, dummy1, dummy2))
            {
                if (ds->GetNumberOfPoints() == 0)
                    continue;

                bool unglyphedDataset = false;
                if (ds->GetDataObjectType() == VTK_POLY_DATA &&
                    (((vtkPolyData*)ds)->GetNumberOfVerts() ==
                     ((vtkPolyData*)ds)->GetNumberOfCells()))
                {
                    unglyphedDataset = true;
                }

                vtkVisItCellLocator *cellLocator = vtkVisItCellLocator::New();
                cellLocator->SetIgnoreGhosts(true);
                cellLocator->SetIgnoreLines(true);
                cellLocator->SetDataSet(ds);
                cellLocator->BuildLocator();
                double pcoords[3] = {0., 0., 0.}, ptLine[3] = {0., 0., 0.};
                double isect[3] = {0., 0., 0.};
                int subId = 0, success = 0;
                vtkIdType foundCell; 
                if (r1[0] == r2[0] &&
                    r1[1] == r2[1] &&
                    r1[2] == r2[2])
                { /* WORLD COORD LOCATE */
                    cellLocator->FindClosestPoint(r1, ptLine, foundCell,
                                                  subId, dist);
                    if (foundCell >= 0 && dist >= 0)
                    {
                        success = 1;
                    }
                }
                else if (unglyphedDataset)
                { /* RAY CLOSEST POINT LOCATE */
                    cellLocator->FindClosestPointToLine(r1, r2,
                                                        dist, foundCell);
                    if (foundCell >= 0 && dist >= 0)
                    {
                        success = 1;
                    }
                }
                else
                { /* RAY-INTERSECT LOCATE */
                    success = cellLocator->IntersectWithLine(r1, r2, 
                                  dist, isect, pcoords, subId, foundCell);
                }
                cellLocator->Delete();

                if (success && dist < minDist)
                {
                    cell = foundCell;
                    good_ds = ds;
                    minDist = dist;
                }
            }
        }
    }

    d = minDist; 
    if ( cell < 0 )
    {
       debug5 << "GlyphPick:  no valid cell was intersected!" << endl; 
    }
    else
    {
        vtkDataSet *ds = good_ds;
        if (ds == NULL)
        {
            debug5 << "GlyphPick:  locator returned a NULL dataset. " << endl;
        }
        else 
        {
            vtkDataArray *oc = 
                ds->GetCellData()->GetArray("avtOriginalCellNumbers");
            if (oc != NULL)
            {
                elNum = (int) oc->GetComponent(cell, 1);
                dom   = (int) oc->GetComponent(cell, 0);
                forCell = true;
            }
            else
            {
                oc = ds->GetCellData()->GetArray("avtOriginalNodeNumbers");
                if (oc != NULL)
                {
                    elNum = (int) oc->GetComponent(cell, 1);
                    dom   = (int) oc->GetComponent(cell, 0);
                    forCell = false;
                }
                else
                {
                    // In the unglyphed point mesh case, the
                    // avtOriginalNodeNumbers array remains at the
                    // points instead of being copied to the cell
                    // data.  Simply make sure to check for it there.

                    oc = ds->GetPointData()->GetArray("avtOriginalNodeNumbers");
                    if (oc != NULL)
                    {
                        elNum = (int) oc->GetComponent(cell, 1);
                        dom   = (int) oc->GetComponent(cell, 0);
                        forCell = false;
                    }
                    else
                    {
                        debug5 << "GlyphPick: Data does not have "
                               << "avtOriginalCellsArray or avtOriginaNodesArray."
                               << endl;
                    }
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: VisWinPlots::GetMaxPlotZShift
//
//  Purpose:
//    Returns the maximum that have plots have been shifted in Z.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 27, 2005 
//
// ****************************************************************************

double
VisWindow::GetMaxPlotZShift()
{
    return plots->GetMaxZShift();
}

// ****************************************************************************
//  Method:  VisWindow::DoAllPlotsAxesHaveSameUnits
//
//  Programmer:  Mark C. Miller
//  Creation:    April 5, 2006 
//
// ****************************************************************************
bool
VisWindow::DoAllPlotsAxesHaveSameUnits()
{
    return plots->DoAllPlotsAxesHaveSameUnits();
}
