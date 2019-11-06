.. _Tutorials_Scripting:

Scripting
=========

.. toctree::
       :maxdepth: 2

This section describes the VisIt_ Command Line Interface (CLI).

Command line interface overview
-------------------------------

VisIt_ includes a rich a command line interface that is based on Python 2.7.

There are several ways to use the *CLI*:

1) Launch VisIt_ in a batch mode and run scripts.

  * Linux: ``/path/to/visit/bin/visit -nowin -cli -s <script.py>``
  * OSX: ``/path/to/VisIt.app/Contents/Resources/bin/visit -nowin -cli -s <script.py>``

2) Launch VisIt_ so that a visualization window is visible and interactively issue *CLI* commands.
3) Use both the standard *GUI* and *CLI* simultaneously.

Launching the *CLI*
-------------------

We will focus on the use case where we have the graphical user interface and
*CLI* running simultaneously. 

To launch the *CLI* from the graphical user interface:

1) Go to *Controls->Command*.

This will bring up the Commands window. The Command window provides a text
editor with Python syntax highlighting and an *Execute* button that tells
VisIt_ to execute the script. Finally, the Command window lets you record
your *GUI* actions into Python code that you can use in your scripts.

A first action in the *CLI*
---------------------------

1) Open "example.silo" in the *GUI* if it not already open.
2) Cut-and-paste the following Python commands into the first tab of the Commands window. ::

      AddPlot("Pseudocolor", "temp")
      # You will see the active plots list in the GUI update, since the CLI and GUI communicate.
      DrawPlots()
      #You should see your plot.

3) Click *Execute*.

Tips about Python
-----------------

1) Python is whitespace sensitive! This is a pain, especially when you are cut-n-pasting things.
2) Python has great constructs for control and iteration, here are some examples: ::

    for i in range(100):
       # use i

    # strided range
    for i in range(0,100,10):
       # use i

    if (cond):
     # stmt

    import sys
    ...
    sys.exit()

Example scripts
---------------

We will be using Python scripts in each of the following sections:
You can get execute them by:

1) Cut-n-paste-ing them into a tab in the Commands window and executing it.

For all of these scripts, make sure "example.silo" is currently open unless
otherwise noted.

Setting attributes
~~~~~~~~~~~~~~~~~~

Each of VisIt_'s Plots and Operators expose a set of *attributes* that control
their behavior. In VisIt_'s *GUI*, these attributes are modified via options
windows. VisIt_'s *CLI* provides a set of simple Python objects that control
these attributes. Here is an example setting the minimum and maximum for the
Pseudocolor plot ::

    DeleteAllPlots()
    AddPlot("Pseudocolor", "temp")
    DrawPlots()
    p = PseudocolorAttributes()
    p.minFlag = 1
    p.maxFlag = 1
    p.min = 3.5
    p.max = 7.5
    SetPlotOptions(p)

Animating an isosurface
~~~~~~~~~~~~~~~~~~~~~~~

This example demonstrates sweeping an isosurface operator to animate the
display of a range of isovalues from "example.silo". ::

    DeleteAllPlots()
    AddPlot("Pseudocolor", "temp")
    iso_atts = IsosurfaceAttributes()
    iso_atts.contourMethod = iso_atts.Value
    iso_atts.variable = "temp"
    AddOperator("Isosurface")
    DrawPlots()
    for i in range(30):
       iso_atts.contourValue = (2 + 0.1*i)
       SetOperatorOptions(iso_atts)
       # For moviemaking, you'll need to save off the image
       # SaveWindow()

Using all of VisIt_'s building blocks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example uses a Pseudocolor plot with a ThreeSlice operator applied to
display *temp* on the exterior of the grid along with streamlines of the
gradient of *temp*.

Note that the script below may not work the first time you execute it. In
that case delete all the plots and execute the script again.

.. figure:: images/Scripting-StreamlineOutput.png

   Streamlines

::

    # Clear any previous plots
    DeleteAllPlots()
    # Create a plot of the scalar field 'temp'
    AddPlot("Pseudocolor","temp")
    # Slice the volume to show only three
    # external faces.
    AddOperator("ThreeSlice")
    tatts = ThreeSliceAttributes()
    tatts.x = -10
    tatts.y = -10
    tatts.z = -10
    SetOperatorOptions(tatts)
    DrawPlots()
    # Find the maximum value of the field 'temp'
    Query("Max")
    val = GetQueryOutputValue()
    print "Max value of 'temp' = ", val

    # Create a streamline plot that follows
    # the gradient of 'temp'
    DefineVectorExpression("g","gradient(temp)")
    AddPlot("Pseudocolor", "operators/IntegralCurve/g")
    iatts = IntegralCurveAttributes()
    iatts.sourceType = iatts.SpecifiedBox
    iatts.sampleDensity0 = 7
    iatts.sampleDensity1 = 7
    iatts.sampleDensity2 = 7
    iatts.dataValue = iatts.SeedPointID
    iatts.integrationType = iatts.DormandPrince
    iatts.issueStiffnessWarnings = 0
    iatts.issueCriticalPointsWarnings = 0
    SetOperatorOptions(iatts)

    # set style of streamlines
    patts = PseudocolorAttributes()
    patts.lineType = patts.Tube 
    patts.tailStyle = patts.Spheres
    patts.headStyle = patts.Cones
    patts.endPointRadiusBBox = 0.01
    SetPlotOptions(patts)

    DrawPlots()

Creating a movie of animated streamline paths
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example extends the "Using all of VisIt_'s Building Blocks" example by 

* animating the paths of the streamlines
* saving images of the animation
* finally, encoding those images into a movie

(Note: Encoding requires ffmpeg is installed and available in your PATH)

.. This is a subset of the [[Seedme | Seedme]] sharing example.

::

    # import visit_utils, we will use it to help encode our movie
    from visit_utils import *
 
    # Set a better view
    ResetView()
    v = GetView3D()
    v.RotateAxis(0,44)
    v.RotateAxis(1,-23)
    SetView3D(v)
 
    # Disable annotations
    aatts = AnnotationAttributes()
    aatts.axes3D.visible = 0
    aatts.axes3D.triadFlag = 0
    aatts.axes3D.bboxFlag = 0
    aatts.userInfoFlag = 0
    aatts.databaseInfoFlag = 0
    aatts.legendInfoFlag = 0
    SetAnnotationAttributes(aatts)
 
    # Set basic save options
    swatts = SaveWindowAttributes()
    #
    # The 'family' option controls if visit automatically adds a frame number to 
    # the rendered files. For this example we will explicitly manage the output name.
    #
    swatts.family = 0
    #
    # select PNG as the output file format
    #
    swatts.format = swatts.PNG 
    #
    # set the width of the output image
    #
    swatts.width = 1024 
    #
    # set the height of the output image
    #
    swatts.height = 1024

 
    ####
    # Crop streamlines to render them at increasing time values over 50 steps
    ####
    iatts.cropValue = iatts.Time 
    iatts.cropEndFlag = 1
    iatts.cropBeginFlag = 1
    iatts.cropBegin = 0
    for ts in range(0,50):
        # set the integral curve attributes to change the where we crop the streamlines
        iatts.cropEnd = (ts + 1) * .5

        # update streamline attributes and draw the plot
        SetOperatorOptions(iatts)
        DrawPlots()
        #before we render the result, explicitly set the filename for this render
        swatts.fileName = "streamline_crop_example_%04d.png" % ts
        SetSaveWindowAttributes(swatts)
        # render the image to a PNG file
        SaveWindow()
 
    ################
    # use visit_utils.encoding to encode these images into a "wmv" movie
    #
    # The encoder looks for a printf style pattern in the input path to identify the frames of the movie. 
    # The frame numbers need to start at 0. 
    # 
    # The encoder selects a set of decent encoding settings based on the extension of the
    # the output movie file (second argument). In this case we will create a "wmv" file. 
    # 
    # Other supported options include ".mpg", ".mov". 
    #   "wmv" is usually the best choice and plays on all most all platforms (Linux ,OSX, Windows). 
    #   "mpg" is lower quality, but should play on any platform.
    #
    # 'fdup' controls the number of times each frame is duplicated. 
    #  Duplicating the frames allows you to slow the pace of the movie to something reasonable. 
    #
    ################
 
    input_pattern = "streamline_crop_example_%04d.png"
    output_movie = "streamline_crop_example.wmv"
    encoding.encode(input_pattern,output_movie,fdup=4)

Rendering each time step of a dataset to a movie
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example assumes the "aneurysm.visit" is already opened.

* Create a plot, render all timesteps and encode a movie.

(Note: Encoding requires that ffmpeg is installed and available in your PATH) ::

    # import visit_utils, we will use it to help encode our movie
    from visit_utils import *
    DeleteAllPlots()

    AddPlot("Pseudocolor","pressure")
    DrawPlots()
 
    # Set a better view
    ResetView()
    v = GetView3D()
    v.RotateAxis(1,90)
    SetView3D(v)
 
    # get the number of timesteps
    nts = TimeSliderGetNStates()
 
    # set basic save options
    swatts = SaveWindowAttributes()
    #
    # The 'family' option controls if visit automatically adds a frame number to 
    # the rendered files. For this example we will explicitly manage the output name.
    #
    swatts.family = 0
    #
    # select PNG as the output file format
    #
    swatts.format = swatts.PNG 
    #
    # set the width of the output image
    #
    swatts.width = 1024 
    #
    # set the height of the output image
    #
    swatts.height = 1024

    #the encoder expects file names with an integer sequence
    # 0,1,2,3 .... N-1

    file_idx = 0

    for ts in range(0,nts,10): # look at every 10th frame
        # Change to the next timestep
        TimeSliderSetState(ts)
        #before we render the result, explicitly set the filename for this render
        swatts.fileName = "blood_flow_example_%04d.png" % file_idx
        SetSaveWindowAttributes(swatts)
        # render the image to a PNG file
        SaveWindow()
        file_idx +=1

    ################
    # use visit_utils.encoding to encode these images into a "wmv" movie
    #
    # The encoder looks for a printf style pattern in the input path to identify the frames of the movie. 
    # The frame numbers need to start at 0. 
    # 
    # The encoder selects a set of decent encoding settings based on the extension of the
    # the output movie file (second argument). In this case we will create a "wmv" file. 
    # 
    # Other supported options include ".mpg", ".mov". 
    #   "wmv" is usually the best choice and plays on all most all platforms (Linux ,OSX, Windows). 
    #   "mpg" is lower quality, but should play on any platform.
    #
    # 'fdup' controls the number of times each frame is duplicated. 
    #  Duplicating the frames allows you to slow the pace of the movie to something reasonable. 
    #
    ################
 
    input_pattern = "blood_flow_example_%04d.png"
    output_movie = "blood_flow_example.wmv"
    encoding.encode(input_pattern,output_movie,fdup=4)

Animating the camera
~~~~~~~~~~~~~~~~~~~~

See `Python fly through <http://visitusers.org/index.php?title=Visit-tutorial-python-fly>`_.

Automating data analysis
~~~~~~~~~~~~~~~~~~~~~~~~

See `Python analysis curves <http://visitusers.org/index.php?title=Visit-tutorial-python-analysis>`_.


Extracting a per-material aggregate value at each timestep
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

See example `Python aggregate curves <http://visitusers.org/index.php?title=Visit-tutorial-python-agg-curve>`_.

.. _Recording_GUI_actions_to_Python_scripts:

Recording *GUI* actions to Python scripts
-----------------------------------------

VisIt_'s Commands window provides a mechanism to translate *GUI* actions into their equivalent Python commands.

1) Open the Commands Window by selecting ''Controls Menu->Command''

.. figure:: images/Scripting-Commands.png

   The Commands window

2) Click the *Record* button.
3) Perform *GUI* actions.
4) Return to the Commands Window.
5) Select a tab to hold the python script of your recorded actions.
6) Click the *Stop* button.
7) The equivalent Python script will be placed in the tab in the Commands window.

* Note that the scripts are very verbose and contain some unnecessary commands, which can be edited out.

Learning the *CLI*
------------------

Here are some tips to help you quickly learn how to use VisIt_'s *CLI*:

.. highlight:: bash

1) From within VisIt_'s python *CLI*, you can type "dir()" to see the list of all commands.

  * Sometimes, the output from "dir()" within VisIt_'s python *CLI* is a little hard to look through. So, a useful thing on Linux to get a nicer list of methods is the following shell command (typed from *outside* VisIt_'s python *CLI*)... ::

      echo "dir()" | visit -cli -nowin -forceinteractivecli | tr ',' '\n' | tr -d " '" | sort

  * Or, if you are looking for *CLI* functions having to do with a specific thing... ::

      echo "dir()" | visit -cli -nowin -forceinteractivecli | tr ',' '\n' | tr -d " '" | grep -i material

2) You can learn the syntax of a given method by typing "help(MethodName)"

  * Type "help(AddPlot)" in the Python interpreter.

3) Use the *GUI* to Python recording featured outlined in :ref:`Recording_GUI_actions_to_Python_scripts`. 

4) Use ''WriteScript()'' function, which will create a python script that describes all of your current plots.

  * For more details, see `WriteScript <http://visitusers.org/index.php?title=WriteScript>`_.

.. highlight:: python

5) When you have a Python object, you can see all of its attributes by printing it. ::

    s = SliceAttributes()
    print s
    # Output:
    originType = Intercept  # Point, Intercept, Percent, Zone, Node
    originPoint = (0, 0, 0)
    originIntercept = 0
    originPercent = 0
    originZone = 0
    originNode = 0
    normal = (0, -1, 0)
    axisType = YAxis  # XAxis, YAxis, ZAxis, Arbitrary, ThetaPhi
    upAxis = (0, 0, 1)
    project2d = 1
    interactive = 1
    flip = 0
    originZoneDomain = 0
    originNodeDomain = 0
    meshName = "default"
    theta = 0
    phi = 0

Tips for searching for help
~~~~~~~~~~~~~~~~~~~~~~~~~~~

VisIt_'s *CLI* provides a large set of functions. To can limit the scope of your
search using a helper functions. One such helper is the *lsearch()* function
in the visit_utils module: ::

    from visit_utils.common import lsearch
    lsearch(dir(),"Material")

*lsearch()* returns a python list of strings with the names that match the
given pattern. Here is another example that prints each of the result strings
on a separate line. ::

    from visit_utils.common import lsearch
    for value in lsearch(dir(),"Material"):
        print value

Advanced features
-----------------

1) You can set up your own buttons in the VisIt_ gui using the *CLI*. See `visitrc file <http://visitusers.org/index.php?title=Visitrc_file>`_.
2) You can set up callbacks in the *CLI* that get called whenever events happen in VisIt_. See `Python callbacks <http://visitusers.org/index.php?title=Python_callbacks>`_.
3) You can create your own custom Qt *GUI* that uses VisIt_ for plotting. See `PySide recipes <http://visitusers.org/index.php?title=PySide_Recipes>`_.
