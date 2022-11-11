# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  basics.py
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files. Exec'ing in this way avoid
# an import hell caused by the interdependency of functions used in
# vqr_utils.py but nonetheless defined in other parts of VisIt's test harness.
# This small block of code needs to be copied into any .py file being used
# to demonstrate a quick recipe.
#
with open(tests_path('quickrecipes','vqr_utils.py'), 'r') as f: 
    prog = compile(''.join(f.readlines()),'load_vqr_utils.err','exec')
    exec(prog)

def using_session_files():
 
  try:
    # using session files {
    # Import a session file from the current working directory. 
    RestoreSession('/home/juan/visit/visit.session', 0) 
    # Now that VisIt has restored the session, animate through time.
    for state in range(TimeSliderGetNStates()): 
      TimeSliderSetState(state) 
      SaveWindow() 
    # using session files }
    TestValueEQ('using session files error message',GetLastError(),'')
    TestPOA('using session files no exceptions')
  except:
    TestFOA('using session files exception', LINE())
    pass
  vqr_cleanup()

def getting_something_on_the_screen():

  try:
    # getting something on the screen {
    # Step 1: Open a database 
    OpenDatabase('~juanita/silo/stuff/wave.visit') 

    # Step 2: Add plots with default properties
    AddPlot('Pseudocolor', 'pressure') 
    AddPlot('Mesh', 'quadmesh') 

    # Step 3: Draw the plots with default view
    DrawPlots() 

    # Step 4: Iterate through time and save images 
    for state in range(0,TimeSliderGetNStates(),10): 
      TimeSliderSetState(state) 
      SaveWindow() 
    # getting something on the screen }
    TestValueEQ('getting something on the screen error message',GetLastError(),'')
    TestPOA('getting something on the screen no exceptions')
  except:
    TestFOA('getting something on the screen exception', LINE())
    pass
  vqr_cleanup()

using_session_files()
getting_something_on_the_screen()

Exit()

# BEGIN Commenting out most code {
"""
def save_window_attributes():

    # save_window_attributes {

    # Prepare to save a BMP file at 1024x768 resolution 
    s = SaveWindowAttributes() 
    s.format = s.BMP 
    s.fileName = 'mybmpfile' 
    s.width, s.height = 1024,768 
    s.screenCapture = 0 
    SetSaveWindowAttributes(s) 
    # Subsequent calls to SaveWindow() will use these settings

    # save_window_attributes }

def open_database_variations():

    # open_database_variations {

    # Open a database at time state 0, the default
    OpenDatabase('/usr/local/visit/data/allinone00.pdb') 

    # Open a database at time state 17 to pick up variables 
    # that exist only for time states 17 and later
    OpenDatabase('/usr/local/visit/data/wave.visit', 17) 

    # Opening first file in series wave0000.silo, wave0010.silo, ... 
    OpenDatabase('/usr/local/visit/data/wave0000.silo') 

    # Opening a virtual database representing all wave*.silo files. 
    OpenDatabase('/usr/local/visit/data/wave*.silo database') 

    # Opening a file on a remote computer by giving a host name 
    # Also, open the database at a later time slice (17)
    OpenDatabase('thunder:/usr/local/visit/data/wave.visit', 17)

    # open_database_variations }


    # Save images of all timesteps and add each image filename to a list. 
    names = [] 
    for state in range(TimeSliderGetNStates()): 
      SetTimeSliderState(state) 
      # Save the image 
      n = SaveWindow() 
      names = names + [n] 
    print(names)


def open_compute_engine_variations():

    # open_compute_engine_variations {

    # Open a local, parallel compute engine before opening a database 
    # Use 4 processors on 2 nodes
    OpenComputeEngine('localhost', ('-np', '4', '-nn', '2'))
    OpenDatabase('/usr/local/visit/data/multi_ucd3d.silo') 

    # Use the 'srun' job launcher, the 'batch' partition, the 'mybank' bank,
    # 72 processors on 2 nodes and a time limit of 1 hour
    OpenComputeEngine('localhost',('-l', 'srun',
                                   '-p', 'batch',
                                   '-b', 'mybank',
                                   '-np', '72',
                                   '-nn', '2',
                                   '-t', '1:00:00'))

    # Set the user name to 'user1' and use the third profile,
    # overriding a few of its properties
    p = GetMachineProfile('quartz.llnl.gov')
    p.userName='user1'
    p.activeProfile = 2
    p.GetLaunchProfiles(2).numProcessors = 72
    p.GetLaunchProfiles(2).numNodes = 2
    p.GetLaunchProfiles(2).timeLimit = '00:30:00'
    OpenComputeEngine(p)

    # open_compute_engine_variations }

::

    # Names of all available plot plugins 
    print(PlotPlugins())

    # Create plots 
    AddPlot('Pseudocolor', 'pressure') 
    AddPlot('Mesh', 'quadmesh') 
    # Draw the plots 
    DrawPlots() 

::

    # Plot material boundaries 
    AddPlot('Boundary', 'mat1') 
    # Plot materials 
    AddPlot('FilledBoundary', 'mat1') 

::

    # Creating a Pseudocolor plot and setting min/max values. 
    AddPlot('Pseudocolor', 'pressure') 
    p = PseudocolorAttributes() 
    # Look in the object 
    print(p)
    # Set the min/max values 
    p.min, p.minFlag = 0.0, 1 
    p.max, p.maxFlag = 10.0, 1 
    SetPlotOptions(p) 

::

    # Create more than 1 plot of the same type 
    AddPlot('Pseudocolor', 'pressure') 
    AddPlot('Pseudocolor', 'density') 

    # List the plots. The second plot should be active. 
    ListPlots() 

    # Draw the plots 
    DrawPlots() 

    # Hide the first plot 
    SetActivePlots(0) 
    HideActivePlots() 

    # Set both plots' color table to 'hot' 
    p = PseudocolorAttributes() 
    p.colorTableName = 'hot' 
    SetActivePlots((0,1)) 
    SetPlotOptions(p) 

    # Show the first plot again. 
    SetActivePlots(0) 
    HideActivePlots()

    # Delete the second plot 
    SetActivePlots(1) 
    DeleteActivePlots() 
    ListPlots() 

::

    # Save an image and take care of plots that entered the error state. 
    drawThePlots = 0 
    for state in range(TimeSliderGetNStates()): 
      if SetTimeSliderState(state) == 0: 
        drawThePlots = 1 
      if drawThePlots == 1: 
        if DrawPlots() == 0: 
          print('VisIt could not draw plots for state: %d')% state 
        else: 
          drawThePlots = 0 
      SaveWindow() 

::

    # Print available operators 
    print(OperatorPlugins())
    # Create a plot 
    AddPlot('Pseudocolor') 
    # Add an Isovolume operator and a Slice operator 
    AddOperator('Isovolume') 
    AddOperator('Slice') 
    DrawPlots() 

::

    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hardyglobal') 
    AddOperator('Slice') 
    s = SliceAttributes() 
    s.originType = s.Percent 
    s.project2d = 0 
    SetOperatorOptions(s) 
    DrawPlots() 

    nSteps = 20 
    for axis in (0,1,2): 
      s.axisType = axis 
      for step in range(nSteps): 
        t = float(step) / float(nSteps - 1) 
        s.originPercent = t * 100. 
        SetOperatorOptions(s) 
        SaveWindow()

::

    # Creating a new expression 
    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hardyglobal') 
    DrawPlots() 
    DefineScalarExpression('newvar', 'sin(hardyglobal) + cos(shepardglobal') 
    ChangeActivePlotsVar('newvar') 

::

    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hgslice') 
    DrawPlots() 
    s = [] 
    # Pick by a node id 
    PickbyNode(300) 
    s = s + [GetPickOutput()] 
    # Pick by a cell id 
    PickByZone(250) 
    s = s + [GetPickOutput()] 
    # Pick on a cell using a 3d point 
    Pick((-2., 2., 0.)) 
    s = s + [GetPickOutput()] 
    # Pick on the node closest to (-2,2,0) 
    NodePick((-2,2,0)) 
    s = s + [GetPickOutput()] 
    # Print all pick results 
    print(s)

::

    p0 = (-5,-3)
    p1 = ( 5, 8)
    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hgslice') 
    DrawPlots() 
    Lineout(p0, p1)
    # Specify 65 sample points 
    Lineout(p0, p1, 65)
    # Do three variables ('default' is 'hgslice')
    Lineout(p0, p1, ('default', 'var1', 'var2'))

::

    # Set active window to one containing Lineout curve plots (typically #2)
    SetActiveWindow(2)
    # Get array of x,y pairs for first curve plot in window
    SetActivePlots(0)
    hgslice_vals = GetPlotInformation()['Curve']
    # Get array of x,y pairs for second curve plot in window
    SetActivePlots(1)
    var1_vals = GetPlotInformation()['Curve']
    # Get array of x,y pairs for third curve plot in window
    SetActivePlots(2)
    var2_vals = GetPlotInformation()['Curve']

    # Write it as CSV data to a file
    for i in range(len(hgslice_vals) / 2):
        idx = i*2+1 # take only y-values in each array
        print '%g,%g,%g' % (hgslice_vals[idx], var1_vals[idx], var2_vals[idx])

::

    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hardyglobal') 
    DrawPlots() 
    Query('NumNodes') 
    print('The float value is: %g')% GetQueryOutputValue() 
    Query('NumNodes') 

::

    # Define a helper function to get the id's of the MinMax query. 
    def GetMinMaxIds(): 
      Query('MinMax') 
      import string 
      s = string.split(GetQueryOutputString(), ' ') 
      retval = [] 
      nextGood = 0 
      idType = 0 
      for token in s: 
        if token == '(zone' or token == '(cell': 
          idType = 1 
          nextGood = 1 
          continue 
        elif token == '(node': 
          idType = 0 
          nextGood = 1 
          continue 
        if nextGood == 1: 
           nextGood = 0 
           retval = retval + [(idType, int(token))] 
      return retval

    # Set up a plot 
    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hgslice') 
    DrawPlots() 

    # Do picks on the ids that were returned by MinMax. 
    for ids in GetMinMaxIds(): 
      idType = ids[0] 
      id = ids[1] 
      if idType == 0: 
        PickByNode(id) 
      else: 
        PickByZone(id) 

::

    OpenDatabase('/usr/local/visit/data/multi_rect2d.silo') 
    AddPlot('Pseudocolor', 'd') 
    DrawPlots() 
    # Turning off all but the last domain 
    d = GetDomains() 
    for dom in d[:-1]: 
      TurnDomainsOff(dom) 
    # Turn all domains off 
    TurnDomainsOff() 
    # Turn on domains 3,5,7 
    TurnDomainsOn((d[3], d[5], d[7]))

::

    OpenDatabase('/usr/local/visit/data/multi_rect2d.silo') 
    AddPlot('FilledBoundary', 'mat1') 
    DrawPlots() 
    # Print the materials are: 
    GetMaterials() 
    # Turn off material 2
    TurnMaterialsOff('2') 

::

    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hgslice') 
    AddPlot('Mesh', 'Mesh2D') 
    AddPlot('Label', 'hgslice') 
    DrawPlots() 
    print('The current view is:', GetView2D())
    # Get an initialized 2D view object. 
    v = GetView2D() 
    v.windowCoords = (-7.67964, -3.21856, 2.66766, 7.87724) 
    SetView2D(v) 

::

    OpenDatabase('/usr/local/visit/data/noise.silo') 
    AddPlot('Pseudocolor', 'hardyglobal') 
    AddPlot('Mesh', 'Mesh') 
    DrawPlots() 
    v = GetView3D() 
    print('The view is: ', v) 
    v.viewNormal = (-0.571619, 0.405393, 0.713378) 
    v.viewUp = (0.308049, 0.911853, -0.271346) 
    SetView3D(v)

::

    # Do a pseudocolor plot of u. 
    OpenDatabase('/usr/local/visit/data/globe.silo') 
    AddPlot('Pseudocolor', 'u') 
    DrawPlots() 
        
    # Create the control points for the views. 
    c0 = View3DAttributes() 
    c0.viewNormal = (0, 0, 1) 
    c0.focus = (0, 0, 0) 
    c0.viewUp = (0, 1, 0) 
    c0.viewAngle = 30 
    c0.parallelScale = 17.3205 
    c0.nearPlane = 17.3205 
    c0.farPlane = 81.9615 
    c0.perspective = 1 
        
    c1 = View3DAttributes() 
    c1.viewNormal = (-0.499159, 0.475135, 0.724629) 
    c1.focus = (0, 0, 0) 
    c1.viewUp = (0.196284, 0.876524, -0.439521) 
    c1.viewAngle = 30 
    c1.parallelScale = 14.0932 
    c1.nearPlane = 15.276 
    c1.farPlane = 69.917 
    c1.perspective = 1 
        
    c2 = View3DAttributes() 
    c2.viewNormal = (-0.522881, 0.831168, -0.189092) 
    c2.focus = (0, 0, 0) 
    c2.viewUp = (0.783763, 0.556011, 0.27671) 
    c2.viewAngle = 30 
    c2.parallelScale = 11.3107 
    c2.nearPlane = 14.8914 
    c2.farPlane = 59.5324 
    c2.perspective = 1 
        
    c3 = View3DAttributes()
    c3.viewNormal = (-0.438771, 0.523661, -0.730246) 
    c3.focus = (0, 0, 0) 
    c3.viewUp = (-0.0199911, 0.80676, 0.590541) 
    c3.viewAngle = 30 
    c3.parallelScale = 8.28257 
    c3.nearPlane = 3.5905 
    c3.farPlane = 48.2315 
    c3.perspective = 1 
        
    c4 = View3DAttributes() 
    c4.viewNormal = (0.286142, -0.342802, -0.894768) 
    c4.focus = (0, 0, 0) 
    c4.viewUp = (-0.0382056, 0.928989, -0.36813) 
    c4.viewAngle = 30 
    c4.parallelScale = 10.4152 
    c4.nearPlane = 1.5495 
    c4.farPlane = 56.1905 
    c4.perspective = 1 
        
    c5 = View3DAttributes() 
    c5.viewNormal = (0.974296, -0.223599, -0.0274086) 
    c5.focus = (0, 0, 0) 
    c5.viewUp = (0.222245, 0.97394, -0.0452541) 
    c5.viewAngle = 30 
    c5.parallelScale = 1.1052 
    c5.nearPlane = 24.1248 
    c5.farPlane = 58.7658 
    c5.perspective = 1 
        
    c6 = c0 
        
    # Create a tuple of camera values and x values. The x values 
    # determine where in [0,1] the control points occur. 
    cpts = (c0, c1, c2, c3, c4, c5, c6) 
    x=[] 
    for i in range(7): 
      x = x + [float(i) / float(6.)] 
        
    # Animate the view using EvalCubicSpline. 
    nsteps = 100 
    for i in range(nsteps): 
      t = float(i) / float(nsteps - 1) 
      c = EvalCubicSpline(t, x, cpts) 
      c.nearPlane = -34.461 
      c.farPlane = 34.461 
      SetView3D(c)

::

    # Set a blue/black, radial, gradient background. 
    a = AnnotationAttributes() 
    a.backgroundMode = a.Gradient 
    a.gradientBackgroundStyle = a.Radial 
    a.gradientColor1 = (0,0,255,255) # Blue 
    a.gradientColor2 = (0,0,0,255) # Black 
    SetAnnotationAttributes(a) 


::

    # Create a text object that we'll use to indicate that our 
    # visualization is unclassified. 
    banner = CreateAnnotationObject('Text2D') 
    banner.text = 'Unclassified' 
    banner.position = (0.37, 0.95) 
    banner.fontBold = 1 
    # print the attributes that you can set in the banner object. 
    print(banner)

::

    # Add a time slider in the lower left corner 
    slider = CreateAnnotationObject('TimeSlider') 
    slider.height = 0.07 
    # Print the options that are available in the time slider object 
    print(slider)

::

    # Incorporate LLNL logo image (llnl.jpeg) as an annotation 
    image = CreateAnnotationObject('Image') 
    image.image = 'llnl.jpeg' 
    image.position = (0.02, 0.02) 
    # Print the other image annotation options 
    print(image)

::

    # Open a file and make a plot
    OpenDatabase('/usr/gapps/visit/data/noise.silo')
    AddPlot('Mesh', 'Mesh')
    AddPlot('Pseudocolor', 'hardyglobal')
    DrawPlots()
    # Get the legend annotation object for the Pseudocolor plot, the second
    # plot in the list (0-indexed).
    plotName = GetPlotList().GetPlots(1).plotName 
    legend = GetAnnotationObject(plotName)
    # See if we can scale the legend.
    legend.xScale = 3.
    legend.yScale = 3.
    # the bounding box.
    legend.drawBoundingBox = 1
    legend.boundingBoxColor = (180,180,180,230)
    # Make it horizontal
    legend.orientation = legend.HorizontalBottom
    # moving the legend
    legend.managePosition = 0
    legend.position = (0.7,0.15)
    # text color
    InvertBackgroundColor()
    legend.useForegroundForTextColor = 0
    legend.textColor = (255, 0, 0, 255)
    # number format
    legend.numberFormat = '%1.4e'
    # the font.
    legend.fontFamily = legend.Arial
    legend.fontBold = 1
    legend.fontItalic = 1
    # turning off the labels.
    legend.fontItalic = 0
    legend.drawLabels = legends.None 
    legend.drawMinMax = 0
    # turning off the title.
    legend.drawTitle = 0
    # Use user-supplied labels, rather than numeric values.
    legend.controlTicks=0
    legend.drawLabels = legend.Labels
    # suppliedLabels must be strings, only valid when controlTicks is 0
    legend.suppliedLabels=('A', 'B', 'C', 'D', 'E')
    # Give the legend a custom title
    legend.useCustomTitle=1
    legend.customTitle='my custom title'
    # Print the legend object so you can see the other properties
    # that you can set in order to modify the legend.
    print(legend)


::

    hotCT = GetColorTable('hot')
    print(hotCT)
    # results of print
    GetControlPoints(0).colors = (0, 0, 255, 255)
    GetControlPoints(0).position = 0
    GetControlPoints(1).colors = (0, 255, 255, 255)
    GetControlPoints(1).position = 0.25
    GetControlPoints(2).colors = (0, 255, 0, 255)
    GetControlPoints(2).position = 0.5
    GetControlPoints(3).colors = (255, 255, 0, 255)
    GetControlPoints(3).position = 0.75
    GetControlPoints(4).colors = (255, 0, 0, 255)
    GetControlPoints(4).position = 1
    smoothing = Linear  # NONE, Linear, CubicSpline
    equalSpacingFlag = 0
    discreteFlag = 0
"""
# END Commenting out most code }
