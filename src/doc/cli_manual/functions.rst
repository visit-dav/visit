

Functions
=========

Many functions return an integer where 1 means success and 0 means failure.
This behavior is represented by the type CLI_return_t in an attempt to
distinguish it from functions that may utilize the full range of integers.


ActivateDatabase
----------------

**Synopsis:**

::

  ActivateDatabase(argument) -> integer


argument : string
    The name of the database to be activated.

return type : CLI_return_t
    ActivateDatabase returns 1 on success and 0 on failure.


**Description:**

    The ActivateDatabase function is used to set the active database to a
    database that has been previously opened. The ActivateDatabase function
    only works when you are using it to activate a database that you have
    previously opened. You do not need to use this function unless you
    frequently toggle between more than one database when making plots or
    changing time states. While the OpenDatabase function can also be used
    to set the active database, the ActivateDatabase function does not have any
    side effects that would cause the time state for the new active database
    to be changed.


**Example:**

::

  #% visit -cli
  dbs = ("/usr/gapps/visit/data/wave.visit", "/usr/gapps/visit/data/curv3d.silo")
  OpenDatabase(dbs[0], 17)
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  OpenDatabase(dbs[1])
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Let's add another plot from the first database.
  ActivateDatabase(dbs[0])
  AddPlot("Mesh", "quadmesh")
  DrawPlots()


AddArgument
-----------

**Synopsis:**

::

  AddArgument(argument)


argument : string
    A string object that is added to the viewer's command line argument list.


**Description:**

    The AddArgument function is used to add extra command line arguments to
    VisIt's viewer. This is only useful when VisIt's Python interface is
    imported into a stand-alone Python interpreter because the AddArgument
    function must be called before the viewer is launched. The AddArgument
    function has no effect when used in VisIt's cli program because the viewer
    is automatically launched before any commands are processed.


**Example:**

::

  import visit
  visit.AddArgument("-nowin") # Add the -nowin argument to the viewer.


AddMachineProfile
-----------------

**Synopsis:**

::

  AddMachineProfile(MachineProfile) -> integer


MachineProfile : MachineProfile object


**Description:**

    Sets the input machine profile in the HostProfileList, replaces if one already exists
    Otherwise adds to the list


AddOperator
-----------

**Synopsis:**

::

  AddOperator(operator) -> integer
  AddOperator(operator, all) -> integer


operator : string
    The name of the operator to be applied.

all : integer
    This is an optional integer argument that applies the operator to all
    plots if the value of the argument is not zero.

return type : CLI_return_t
    The AddOperator function returns an integer value of 1 for success and 0
    for failure.


**Description:**

    The AddOperator function adds a VisIt operator to the active plots. The
    operator argument is a string containing the name of the operator to be
    added to the active plots. The operatore name must be a valid operator
    plugin name that is a member of the tuple returned by the OperatorPlugins
    function. The all argument is an integer that determines
    whether or not the operator is applied to all plots. If the all argument is
    not provided, the operator is only added to active plots. Once the
    AddOperator function is called, the desired operator is added to all
    active plots unless the all argument is a non-zero value. When the all
    argument is a non-zero value, the operator is applied to all plots
    regardless of whether or not they are selected. Operator attributes are set
    through the SetOperatorOptions function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddPlot("Mesh", "mesh1")
  AddOperator("Slice", 1) # Slice both plots
  DrawPlots()


AddPlot
-------

**Synopsis:**

::

  AddPlot(plotType, variableName) -> integer
  AddPlot(plotType, variableName, inheritSIL) -> integer
  AddPlot(plotType, variableName, inheritSIL, applyOperators) -> integer


plotType : string
    The name of a valid plot plugin type.

variableName : string
    A valid variable name for the open database.

inheritSIL : integer
    An integer flag indicating whether the plot should inherit the
    active plot's SIL restriction.

applyOperators : integer
    An integer flag indicating whether the operators from the active
    plot should be applied to the new plot.

return type : CLI_return_t
    The AddPlot function returns an integer value of 1 for success and 0 for
    failure.


**Description:**

    The AddPlot function creates a new plot of the specified type using a
    variable from the open database. The plotType argument is a string that
    contains the name of a valid plot plugin type which must be a member of the
    string tuple that is returned by the PlotPlugins function.
    The variableName argument is a string that contains the name of a variable
    in the open database. After the AddPlot function is called, a new plot is
    created and it is made the sole active plot.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Subset", "mat1") # Create a subset plot
  DrawPlots()


AddWindow
---------

**Synopsis:**

::

  AddWindow()


**Description:**

    The AddWindow function creates a new visualization window and makes it the
    active window. This function can be used to create up to 16 visualization
    windows. After that, the AddWindow function has no effect.


**Example:**

::

  import visit
  visit.Launch()
  visit.AddWindow() # Create window #2
  visit.AddWindow() # Create window #3


AlterDatabaseCorrelation
------------------------

**Synopsis:**

::

  AlterDatabaseCorrelation(name, databases, method) -> integer


name : string
    The name of the database correlation to be altered.

databases : tuple or list of strings
    The databases argument must be a tuple or list of strings containing the
    fully qualified database names to be used in the database correlation.

method : integer
    The method argument must be an integer in the range [0,3].
    
    
    =========================  =====
    **Correlation method**     Value
    =========================  =====
    IndexForIndexCorrelation   0
    StretchedIndexCorrelation  1
    TimeCorrelation            2
    CycleCorrelation           3
    =========================  =====

return type : CLI_return_t
    The AlterDatabaseCorrelation function returns 1 on success and 0 on
    failure.


**Description:**

    The AlterDatabaseCorrelation method alters an existing database
    correlation. A database correlation is a VisIt construct that relates the
    time states for two or more databases in some way. You would use the
    AlterDatabaseCorrelation function if you wanted to change the list of
    databases used in a database correlation or if you wanted to change how the
    databases are related - the correlation method. The name argument is a
    string that is the name of the database correlation to be altered. If the
    name that you pass is not a valid database correlation then the
    AlterDatabaseCorrelation function fails. The databases argument is a list
    or tuple of string objects containing the fully-qualified
    (host:/path/filename) names of the databases to be involved in the database
    query. The method argument allows you to specify a database correlation
    method.


**Example:**

::

  dbs = ("/usr/gapps/visit/data/wave.visit", "/usr/gapps/visit/data/wave*.silo database")
  OpenDatabase(dbs[0])
  AddPlot("Pseudocolor", "pressure")
  OpenDatabase(dbs[1])
  AddPlot("Pseudocolor", "d")
  # VisIt created an index for index database correlation but we
  # want a cycle correlation.
  AlterDatabaseCorrelation("Correlation01", dbs, 3)


ApplyNamedSelection
-------------------

**Synopsis:**

::

  ApplyNamedSelection(name) -> integer


name : string
    The name of a named selection.  (This should have been previously created
    with a CreateNamedSelection call.)

return type : CLI_return_t
    The ApplyNamedSelection function returns 1 for success and 0 for failure.


**Description:**

    Named Selections allow you to select a group of elements (or particles).
    One typically creates a named selection from a group of elements and then
    later applies the named selection to another plot (thus reducing the
    set of elements displayed to the ones from when the named selection was
    created).


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/wave*.silo database"
  OpenDatabase(db)
  AddPlot("Pseudocolor", "pressure")
  AddOperator("Clip")
  c = ClipAttributes()
  c.plane1Origin = (0,0.6,0)
  c.plane1Normal = (0,-1,0)
  SetOperatorOption(c)
  DrawPlots()
  CreateNamedSelection("els_above_at_time_0")
  SetTimeSliderState(40)
  RemoveLastOperator()
  ApplyNamedSelection("els_above_at_time_0")


ChangeActivePlotsVar
--------------------

**Synopsis:**

::

  ChangeActivePlotsVar(variableName) -> integer


variableName : string
    The name of the new plot variable.

return type : CLI_return_t
    The ChangeActivePlotsVar function returns an integer value of 1 for
    success and 0 for failure.


**Description:**

    The ChangeActivePlotsVar function changes the plotted variable for the
    active plots. This is a useful way to change what is being visualized
    without having to delete and recreate the current plots. The variableName
    argument is a string that contains the name of a variable in the open
    database.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  SaveWindow()
  ChangeActivePlotsVar("v")


CheckForNewStates
-----------------

**Synopsis:**

::

  CheckForNewStates(name) -> integer


name : string
    The name of a database that has been opened previously.

return type : CLI_return_t
    The CheckForNewStates function returns 1 for success and 0 for failure.


**Description:**

    Calculations are often run at the same time as some of the preliminary
    visualization work is being performed. That said, you might be visualizing
    the leading time states of a database that is still being created. If you
    want to force VisIt to add any new time states that were added since you
    opened the database, you can use the CheckForNewStates function. The name
    argument must contain the name of a database that has been opened before.


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/wave*.silo database"
  OpenDatabase(db)
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  SetTimeSliderState(TimeSliderGetNStates() - 1)
  # More files appear on disk
  CheckForNewStates(db)
  SetTimeSliderState(TimeSliderGetNStates() - 1)


ChooseCenterOfRotation
----------------------

**Synopsis:**

::

  ChooseCenterOfRotation() -> integer
  ChooseCenterOfRotation(screenX, screenY) -> integer


screenX : double
    A double that is the X coordinate of the pick point in normalized [0,1]
    screen space.

screenY : double
    A double that is the Y coordinate of the pick point in normalized [0,1]
    screen space.

return type : CLI_return_t
    The ChooseCenterOfRotation function returns 1 if successful and 0 if it
    fails.


**Description:**

    The ChooseCenterOfRotation function allows you to pick a new center of
    rotation, which is the point about which plots are rotated when you
    interactively rotate plots. The function can either take zero arguments, in
    which case you must interactively pick on plots, or it can take two
    arguments that correspond to the X and Y coordinates of the desired pick
    point in normalized screen space. When using the two argument version of
    the ChooseCenterOfRotation function, the X and Y values are floating point
    values in the range [0,1]. If the ChooseCenterOfRotation function is able
    to actually pick on plots, yes there must be plots in the vis window, then
    the center of rotation is updated and the new value is printed to the
    console.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlots("Pseudocolor", "u")
  DrawPlots()
  # Interactively choose the center of rotation
  ChooseCenterOfRotation()
  # Choose a center of rotation using normalized screen
  # coordinates and print the value.
  ResetView()
  ChooseCenterOfRotation(0.5, 0.3)
  print("The new center of rotation is:{}".format(GetView3D().centerOfRotation))


ClearAllWindows
---------------

**Synopsis:**

::

  ClearAllWindows() -> integer

return type : CLI_return_t
    1 on success, 0 on failure.


**Description:**

    The ClearWindow function is used to clear out the plots from the active
    visualization window. The plots are removed from the visualization window
    but are left in the plot list so that subsequent calls to the DrawPlots
    function regenerate the plots in the plot list. The ClearAllWindows
    function preforms the same work as the ClearWindow function except that all
    windows are cleared of their plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  AddWindow()
  SetActiveWindow(2) # Make window 2 active
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Subset", "mat1")
  DrawPlots()
  ClearWindow() # Clear the plots in window 2.
  DrawPlots() # Redraw the plots in window 2.
  ClearAllWindows() # Clear the plots from all windows.


ClearCache
----------

**Synopsis:**

::

  ClearCache(host) -> integer
  ClearCache(host, simulation) -> integer


host : string
    The name of the computer where the compute engine is running.

simulation : string
    The name of the simulation being processed by the compute engine.

return type : CLI_return_t
    1 on success and 0 on failure.


**Description:**

    Sometimes during extended VisIt runs, you might want to periodically clear
    the compute engine's network cache to reduce the amount of memory being
    used by the compute engine. Clearing the network cache is also useful when
    you want to change what the compute engine is working on. For example, you
    might process a large database and then decide to process another large
    database. Clearing the network cache beforehand will free up more resources
    for the compute engine so it can more efficiently process the new database.
    The host argument is a string object containing the name of the computer on
    which the compute engine is running. The simulation argument is optional
    and only applies to when you want to instruct a simulation that is acting
    as a VisIt compute engine to clear its network cache. If you want to tell
    more than one compute engine to clear its cache without having to call
    ClearCache multiple times, you can use the ClearCacheForAllEngines function.


**Example:**

::

  #%visit -cli
  OpenDatabase("localhost:very_large_database")
  # Do a lot of work
  ClearCache("localhost")
  OpenDatabase("localhost:another_large_database")
  # Do more work
  OpenDatabase("remotehost:yet_another_database")
  # Do more work
  ClearCacheForAllEngines()


ClearCacheForAllEngines
-----------------------

**Synopsis:**

::

  ClearCacheForAllEngines() -> integer


return type : CLI_return_t
    1 on success and 0 on failure.


**Description:**

    Sometimes during extended VisIt runs, you might want to periodically clear
    the compute engine's network cache to reduce the amount of memory being
    used by the compute engine. Clearing the network cache is also useful when
    you want to change what the compute engine is working on. For example, you
    might process a large database and then decide to process another large
    database. Clearing the network cache beforehand will free up more resources
    for the compute engine so it can more efficiently process the new database.
    The host argument is a string object containing the name of the computer on
    which the compute engine is running. The simulation argument is optional
    and only applies to when you want to instruct a simulation that is acting
    as a VisIt compute engine to clear its network cache. If you want to tell
    more than one compute engine to clear its cache without having to call
    ClearCache multiple times, you can use the ClearCacheForAllEngines function.


**Example:**

::

  #%visit -cli
  OpenDatabase("localhost:very_large_database")
  # Do a lot of work
  ClearCache("localhost")
  OpenDatabase("localhost:another_large_database")
  # Do more work
  OpenDatabase("remotehost:yet_another_database")
  # Do more work
  ClearCacheForAllEngines()


ClearMacros
-----------

**Synopsis:**

::

  ClearMacros()



**Description:**

    The ClearMacros function clears out the list of registered macros and sends
    a message to the gui to clear the buttons from the Macros window.


**Example:**

::

  ClearMacros()


ClearPickPoints
---------------

**Synopsis:**

::

  ClearPickPoints()


**Description:**

    The ClearPickPoints function removes pick points from the active
    visualization window. Pick points are the letters that are added to the
    visualization window where the mouse is clicked when the visualization
    window is in pick mode.


**Example:**

::

  #% visit -cli
  # Put the visualization window into pick mode using the popup
  # menu and add some pick points.
  # Clear the pick points.
  ClearPickPoints()


ClearReferenceLines
-------------------

**Synopsis:**

::

  ClearReferenceLines()


**Description:**

    The ClearReferenceLines function removes reference lines from the active
    visualization window. Reference lines are the lines that are drawn on a
    plot to show where you have performed lineouts.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "d")
  Lineout((-3.0, 2.0), (2.0, 4.0), ("default", "u", "v"))
  ClearReferenceLines()


ClearViewKeyframes
------------------

**Synopsis:**

::

  ClearViewKeyframes() -> integer

return type : CLI_return_t
    The ClearViewKeyframes function returns 1 on success and 0 on failure.


**Description:**

    The ClearViewKeyframes function clears any view keyframes that may have
    been set. View keyframes are used to create complex view behavior such as
    fly-throughs when VisIt is in keyframing mode.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  k = KeyframeAttributes()
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1,10,1
  SetKeyframeAttributes(k)
  DrawPlots()
  SetViewKeyframe()
  v1 = GetView3D()
  v1.viewNormal = (-0.66609, 0.337227, 0.665283)
  v1.viewUp = (0.157431, 0.935425, -0.316537)
  SetView3D(v1)
  SetTimeSliderState(9)
  SetViewKeyframe()
  ToggleCameraViewMode()
  for i in range(10):
      SetTimeSliderState(i)
  ClearViewKeyframes()


ClearWindow
-----------

**Synopsis:**

::

  ClearWindow() -> integer

return type : CLI_return_t
    1 on success, 0 on failure.


**Description:**

    The ClearWindow function is used to clear out the plots from the active
    visualization window. The plots are removed from the visualization window
    but are left in the plot list so that subsequent calls to the DrawPlots
    function regenerate the plots in the plot list. The ClearAllWindows
    function preforms the same work as the ClearWindow function except that all
    windows are cleared of their plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  AddWindow()
  SetActiveWindow(2) # Make window 2 active
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Subset", "mat1")
  DrawPlots()
  ClearWindow() # Clear the plots in window 2.
  DrawPlots() # Redraw the plots in window 2.
  ClearAllWindows() # Clear the plots from all windows.


CloneWindow
-----------

**Synopsis:**

::

  CloneWindow() -> integer

return type : CLI_return_t
    The CloneWindow function returns an integer value of 1 for success and 0
    for failure.


**Description:**

    The CloneWindow function tells the viewer to create a new window, based on
    the active window, that contains the same plots, annotations, lights, and
    view as the active window. This function is useful for when you have a
    window set up like you want and then want to do the same thing in another
    window using a different database. You can first clone the window and then
    replace the database.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  v = ViewAttributes()
  v.camera = (-0.505893, 0.32034, 0.800909)
  v.viewUp = (0.1314, 0.946269, -0.295482)
  v.parallelScale = 14.5472
  v.nearPlane = -34.641
  v.farPlane = 34.641
  v.perspective = 1
  SetView3D() # Set the view
  a = AnnotationAttributes()
  a.backgroundColor = (0, 0, 255, 255)
  SetAnnotationAttributes(a) # Set the annotation properties
  CloneWindow() # Create a clone of the active window
  DrawPlots() # Make the new window draw its plots


Close
-----

**Synopsis:**

::

  Close()



**Description:**

    The Close function terminates VisIt's viewer. This is useful for Python
    scripts that only need access to VisIt's capabilties for a short time
    before closing VisIt.


**Example:**

::

  import visit
  visit.Launch()
  visit.Close() # Close the viewer


CloseComputeEngine
------------------

**Synopsis:**

::

  CloseComputeEngine() -> integer
  CloseComputeEngine(hostName) -> integer
  CloseComputeEngine(hostName, simulation) -> integer


hostName : string
    Optional name of the computer on which the compute engine is running.

simulation : string
    Optional name of a simulation.

return type : CLI_return_t
    The CloseComputeEngine function returns an integer value of 1 for success
    and 0 for failure.


**Description:**

    The CloseComputeEngine function tells the viewer to close the compute
    engine running a specified host. The hostName argument is a string that
    contains the name of the computer where the compute engine is running. The
    hostName argument can also be the name "localhost" if you want to close
    the compute engine on the local machine without having to specify its name.
    It is not necessary to provide the hostName argument. If the argument is
    omitted, the first compute engine in the engine list will be closed. The
    simulation argument can be provided if you want to close a connection to a
    simulation that is acting as a VisIt compute engine. A compute engine can
    be launched again by creating a plot or by calling the OpenComputeEngine
    function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo") # Launches an engine
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  CloseComputeEngine() # Close the compute engine


CloseDatabase
-------------

**Synopsis:**

::

  CloseDatabase(name) -> integer


name : string
    The name of the database to close.

return type : CLI_return_t
    The CloseDatabase function returns 1 on success and 0 on failure.


**Description:**

    The CloseDatabase function is used to close a specified database and free
    all resources that were devoted to keeping the database open. This function
    has an effect similar to ClearCache but it does more in that
    in addition to clearing the compute engine's cache, which it only does for
    the specified database, it also removes all references to the specified
    database from tables of cached metadata, etc. Note that the CloseDatabase
    function will fail and the database will not be closed if any plots
    reference the specified database.


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/globe.silo"
  OpenDatabase(db)
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  print("This won't work: retval = %d" % CloseDatabase(db))
  DeleteAllPlots()
  print("Now it works: retval = %d" % CloseDatabase(db))


ColorTableNames
---------------

**Synopsis:**

::

  ColorTableNames() -> tuple

return type : tuple
    The ColorTableNames function returns a tuple of strings containing the
    names of the color tables that have been defined.


**Description:**

    The ColorTableNames function returns a tuple of strings containing the
    names of the color tables that have been defined. This method can be used
    in case you want to iterate over several color tables.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  for ct in ColorTableNames():
      p = PseudocolorAttributes()
      p.colorTableName = ct
      SetPlotOptions(p)


ConstructDataBinning
--------------------

**Synopsis:**

::

  ConstructDataBinning(options) -> integer


options : ConstructDataBinningAttributes object
    An object of type ConstructDataBinningAttributes. This object specifies
    the options for constructing a data binning.

return type : CLI_return_t
    Returns 1 on success, 0 on failure.


**Description:**

    The ConstructDataBinning function creates a data binning function for the active
    plot. Data Binnings place data from a data set into bins and reduce that data.
    They are used to either be incorporated with expressions to make new derived quantities
    or to be directly visualized.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv3d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set the construct data binning attributes.
  i = ConstructDataBinningAttributes()
  i.name = "db1"
  i.binningScheme = i.Uniform
  i.varnames = ("u", "w")
  i.binBoundaries = (-1, 1, -1, 1) # minu, maxu, minw, maxw
  i.numSamples = (25, 25)
  i.reductionOperator = i.Average
  i.varForReductionOperator = "v"
  ConstructDataBinning(i)
  # Example of binning using spatial coordinates
  i.varnames = ("X", "u") # X is added as a placeholder to maintain indexing
  i.binType = (1, 0) # 1 = X, 2 = Y, 3 = Z, 0 = variable


CopyAnnotationsToWindow
-----------------------

**Synopsis:**

::

  CopyAnnotationsToWindow(source, dest) -> integer


source : integer
    The index (an integer from 1 to 16) of the source window.

dest : integer
    The index (an integer from 1 to 16) of the destination window.

return type : CLI_return_t
    1 for success and 0 for failure.


**Description:**

    The Copy functions copy attributes from one visualization window to
    another visualization window. The CopyAnnotationsToWindow function copies
    the annotations from a source visualization window to a destination
    visualization window.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  AddWindow()
  SetActiveWindow(2)
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Mesh", "mesh1")
  # Copy window 1's Pseudocolor plot to window 2.
  CopyPlotsToWindow(1, 2)
  DrawPlots() # Window 2 will have 2 plots
  # Spin the plots around in window 2 using the mouse.
  CopyViewToWindow(2, 1) # Copy window 2's view to window 1.


CopyLightingToWindow
--------------------

**Synopsis:**

::

  CopyLightingToWindow(source, dest) -> integer


source : integer
    The index (an integer from 1 to 16) of the source window.

dest : integer
    The index (an integer from 1 to 16) of the destination window.

return type : CLI_return_t
    1 for success and 0 for failure.


**Description:**

    The Copy functions copy attributes from one visualization window to
    another visualization window. The CopyLightingAttributes function copies
    lighting.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  AddWindow()
  SetActiveWindow(2)
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Mesh", "mesh1")
  # Copy window 1's Pseudocolor plot to window 2.
  CopyPlotsToWindow(1, 2)
  DrawPlots() # Window 2 will have 2 plots
  # Spin the plots around in window 2 using the mouse.
  CopyViewToWindow(2, 1) # Copy window 2's view to window 1.


CopyPlotsToWindow
-----------------

**Synopsis:**

::

  CopyPlotsToWindow(source, dest) -> integer


source : integer
    The index (an integer from 1 to 16) of the source window.

dest : integer
    The index (an integer from 1 to 16) of the destination window.

return type : CLI_return_t
    1 for success and 0 for failure.


**Description:**

    The Copy functions copy attributes from one visualization window to
    another visualization window. The CopyPlotsToWindow function copies
    the plots from one visualization window to another visualization
    window but does not also force plots to generate so after copying
    plots with the CopyPlotsToWindow function, you should also call the
    DrawPlots function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  AddWindow()
  SetActiveWindow(2)
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Mesh", "mesh1")
  # Copy window 1's Pseudocolor plot to window 2.
  CopyPlotsToWindow(1, 2)
  DrawPlots() # Window 2 will have 2 plots
  # Spin the plots around in window 2 using the mouse.
  CopyViewToWindow(2, 1) # Copy window 2's view to window 1.


CopyViewToWindow
----------------

**Synopsis:**

::

  CopyViewToWindow(source, dest) -> integer


source : integer
    The index (an integer from 1 to 16) of the source window.

dest : integer
    The index (an integer from 1 to 16) of the destination window.

return type : CLI_return_t
    The Copy functions return an integer value of 1 for success and 0 for
    failure.


**Description:**

    The Copy functions copy attributes from one visualization window to
    another visualization window. The CopyViewToWindow function copies
    the view.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  AddWindow()
  SetActiveWindow(2)
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Mesh", "mesh1")
  # Copy window 1's Pseudocolor plot to window 2.
  CopyPlotsToWindow(1, 2)
  DrawPlots() # Window 2 will have 2 plots
  # Spin the plots around in window 2 using the mouse.
  CopyViewToWindow(2, 1) # Copy window 2's view to window 1.


CreateAnnotationObject
----------------------

**Synopsis:**

::

  CreateAnnotationObject(annotType[,annotName,visibleFlag]) -> annotation object


annotType : string
    The name of the type of annotation object to create.
    
    ======================  ==========
    **Annotation type**     String
    ======================  ==========
    2D text annotation      Text2D
    3D text annotation      Text3D
    Time slider annotation  TimeSlider
    Image annotation        Image
    Line/arrow annotation   Line2D
    ======================  ==========

annotName : string
    A user-defined name of the annotation object to create.
    By default, VisIt creates names like 'newObject0', 'newObject1', ....

visibleFlag : integer
    An optional integer to indicate if the annotation object should be created
    with initial visibility on or off. Pass 0 for off and non-zero for on.
    By default, VisIt creates annotation objects with visibility on. If you
    wish only to pass the visibleFlag argument, there is no need to also pass
    the annotName argument.

return type : annotation object
    CreateAnnotationObject is a factory function that creates annotation
    objects of different types. The return value, if a valid annotation type is
    provided, is an annotation object. If the function fails, VisItException is
    raised.


**Description:**

    CreateAnnotationObject is a factory function that creates different kinds
    of annotation objects. The annotType argument is a string containing the
    name of the type of annotation object to create. Each type of annotation
    object has different properties that can be set. Setting the different
    properties of an Annotation objects directly modifes annotations in the vis
    window. Currently there are 5 types of annotation objects:


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit", 17)
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  slider = CreateAnnotationObject("TimeSlider")
  print(slider)
  slider.startColor = (255,0,0,255)
  slider.endColor = (255,255,0,255)


CreateDatabaseCorrelation
-------------------------

**Synopsis:**

::

  CreateDatabaseCorrelation(name, databases, method) -> integer


name : string
    The name of the database correlation to be created.

databases : tuple or list of strings
    Tuple or list of strings containing the names of the databases to involve
    in the database correlation.

method : integer
    An integer in the range [0,3] that determines the correlation method.
    
    =========================  =====
    **Correlation method**     Value
    =========================  =====
    IndexForIndexCorrelation   0
    StretchedIndexCorrelation  1
    TimeCorrelation            2
    CycleCorrelation           3
    =========================  =====

return type : CLI_return_t
    The CreateDatabaseCorrelation function returns 1 on success and 0 on
    failure.


**Description:**

    The CreateDatabaseCorrelation function creates a database correlation,
    which is a VisIt construct that relates the time states for two or more
    databases in some way. You would use the CreateDatabaseCorrelation function
    if you wanted to put plots from more than one time-varying database in the
    same vis window and then move them both through time in some synchronized
    way. The name argument is a string that is the name of the database
    correlation to be created. You will use the name of the database
    correlation to set the active time slider later so that you can change time
    states. The databases argument is a list or tuple of string objects
    containing the fully-qualified (host:/path/filename) names of the databases
    to be involved in the database query. The method argument allows you to
    specify a database correlation method.
    Each database correlation has its own time slider that can be used to set
    the time state of databases that are part of a database correlation.
    Individual time-varying databases have their own trivial database
    correlation, consisting of only 1 database. When you call the
    CreateDatabaseCorrelation function, VisIt creates a new time slider with
    the same name as the database correlation and makes it be the active time
    slider.


**Example:**

::

  #% visit -cli
  dbs = ("/usr/gapps/visit/data/dbA00.pdb",
  "/usr/gapps/visit/data/dbB00.pdb")
  OpenDatabase(dbs[0])
  AddPlot("FilledBoundary", "material(mesh)")
  OpenDatabase(dbs[1])
  AddPlot("FilledBoundary", "material(mesh)")
  DrawPlots()
  CreateDatabaseCorrelation("common", dbs, 1)
  # Creating a new database correlation also creates a new time
  # slider and makes it be active.
  w = GetWindowInformation()
  print("Active time slider: %s" % w.timeSliders[w.activeTimeSlider])
  # Animate through time using the "common" database correlation's
  # time slider.
  for i in range(TimeSliderGetNStates()):
      SetTimeSliderState(i)


CreateNamedSelection
--------------------

**Synopsis:**

::

  CreateNamedSelection(name) -> integer
  CreateNamedSelection(name, properties) -> integer


name : string
    The name of a named selection.

properties : SelectionProperties object
    This optional argument lets you pass a SelectionProperties object containing
    the properties that will be used to create the named selection. When this
    argument is omitted, the named selection will always be associated with
    the active plot. You can use this argument to set up more complex named
    selections that may be associated with plots or databases.

return type : CLI_return_t
    The CreateNamedSelection function returns 1 for success and 0 for failure.


**Description:**

    Named Selections allow you to select a group of elements (or particles).
    One typically creates a named selection from a group of elements and then
    later applies the named selection to another plot (thus reducing the
    set of elements displayed to the ones from when the named selection was
    created).


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/wave*.silo database"
  OpenDatabase(db)
  AddPlot("Pseudocolor", "pressure")
  AddOperator("Clip")
  c = ClipAttributes()
  c.plane1Origin = (0,0.6,0)
  c.plane1Normal = (0,-1,0)
  SetOperatorOption(c)
  DrawPlots()
  CreateNamedSelection("els_above_at_time_0")
  SetTimeSliderState(40)
  RemoveLastOperator()
  ApplyNamedSelection("els_above_at_time_0")


DatabasePlugins
---------------

**Synopsis:**

::

  DatabasePlugins() -> dictionary
  DatabasePlugins(host) -> dictionary


host : string
    The name of the host for which we want database plugins.

return type : dictionary
    The DatabasePlugins functions returns a dictionary.


**Description:**

    The DatabasePlugins function returns a dictionary containing the names of
    the database plugins for the specified host. If no host is given, localhost
    is assumed. The dictionary contains two keys: "host" and "plugins".


**Example:**

::

  #% visit -cli
  dbp = DatabasePlugins("localhost")
  print(dbp["host"])
  print(dbp["plugins"])


DeIconifyAllWindows
-------------------

**Synopsis:**

::

  DeIconifyAllWindows()


**Description:**

    The DeIconifyAllWindows function unhides all of the hidden visualization
    windows. This function is usually called after IconifyAllWindows as a way
    of making all of the hidden visualization windows visible.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4) # Have 4 windows
  IconifyAllWindows()
  DeIconifyAllWindows()


DefineArrayExpression
---------------------

**Synopsis:**

::

  DefineArrayExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    DefineArrayExpression creates new array variables.
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DefineCurveExpression
---------------------

**Synopsis:**

::

  DefineCurveExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    DefineCurveExpression creates new curve variables.
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DefineMaterialExpression
------------------------

**Synopsis:**

::

  DefineMaterialExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    The DefineMaterialExpression function creates new material variables.
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DefineMeshExpression
--------------------

**Synopsis:**

::

  DefineMeshExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    The DefineMeshExpression creates new mesh variables.
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DefinePythonExpression
----------------------

**Synopsis:**

::

  DefinePythonExpression(myvar,args,source)
  DefinePythonExpression(myvar,args,source,type)
  DefinePythonExpression(myvar,args,file)


myvar : string
    The name of the variable to be created.

args : tuple
    A tuple (or list) of strings providing the variable names of the
    arguments to the Python Expression.

source : string
    A string containing the source code for a Python Expression Filter .

file : string
    A string containing the path to a Python Expression Filter script file.

type : string
    An optional string defining the output type of the expression.
    Default type - 'scalar'
    Avalaible types - 'scalar','vector','tensor','array','curve'
    Note - Use only one of the 'source' or 'file' arguments.
    If both are used the 'source' argument overrides 'file'.


**Description:**

    Used to define a Python Filter Expression.


DefineScalarExpression
----------------------

**Synopsis:**

::

  DefineScalarExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    The DefineScalarExpression function creates a new scalar variable based on
    other variables from the open database.
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DefineSpeciesExpression
-----------------------

**Synopsis:**

::

  DefineSpeciesExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    The DefineSpeciesExpression creates new species variables.
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DefineTensorExpression
----------------------

**Synopsis:**

::

  DefineTensorExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    The DefineTensorExpression creates new tensor variables.
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DefineVectorExpression
----------------------

**Synopsis:**

::

  DefineVectorExpression(variableName, expression) -> integer


variableName : string
    The name of the variable to be created.

expression : string
    The expression definition as a string.

return type : CLI_return_t
    The DefineExpression functions return 1 on success and 0 on failure.


**Description:**

    The DefineVectorExpression creates new vector variables
    Expression variables can be plotted like any other variable.
    The variableName argument is a string that contains the name of the new
    variable. You can pass the name of an existing expression if you want
    to provide a new expression definition.
    The expression argument is a string that contains the definition of the
    new variable in terms of math operators and pre-existing variable names
    Reference the VisIt User's Manual if you want more information on
    creating expressions, such as expression syntax, or a list of built-in
    expression functions.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  # Plot the scalar expression variable.
  AddPlot("Pseudocolor", "myvar")
  DrawPlots()
  # Plot a vector expression variable.
  DefineVectorExpression("myvec", "{u,v,w}")
  AddPlot("Vector", "myvec")
  DrawPlots()


DeleteActivePlots
-----------------

**Synopsis:**

::

  DeleteActivePlots() -> integer

return type : CLI_return_t
    The Delete functions return an integer value of 1 for success and 0 for
    failure.


**Description:**

    The Delete functions delete plots from the active window's plot list. The
    DeleteActivePlots function deletes all of the active plots from the plot
    list. There is no way to retrieve a plot once it has been deleted from the
    plot list. The active plots are set using the SetActivePlots function. The
    DeleteAllPlots function deletes all plots from the active window's plot
    list regardless of whether or not they are active.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "d")
  AddPlot("Contour", "u")
  AddPlot("Mesh", "curvmesh2d")
  DrawPlots()
  DeleteActivePlots() # Delete the mesh plot
  DeleteAllPlots() # Delete the pseudocolor and contour plots.


DeleteAllPlots
--------------

**Synopsis:**

::

  DeleteAllPlots() -> integer

return type : CLI_return_t
    The Delete functions return an integer value of 1 for success and 0 for
    failure.


**Description:**

    The Delete functions delete plots from the active window's plot list. The
    DeleteActivePlots function deletes all of the active plots from the plot
    list. There is no way to retrieve a plot once it has been deleted from the
    plot list. The active plots are set using the SetActivePlots function. The
    DeleteAllPlots function deletes all plots from the active window's plot
    list regardless of whether or not they are active.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "d")
  AddPlot("Contour", "u")
  AddPlot("Mesh", "curvmesh2d")
  DrawPlots()
  DeleteActivePlots() # Delete the mesh plot
  DeleteAllPlots() # Delete the pseudocolor and contour plots.


DeleteDatabaseCorrelation
-------------------------

**Synopsis:**

::

  DeleteDatabaseCorrelation(name) -> integer


name : string
    The name of the database correlation to delete.

return type : CLI_return_t
    The DeleteDatabaseCorrelation function returns 1 on success and 0 on
    failure.


**Description:**

    The DeleteDatabaseCorrelation function deletes a specific database
    correlation and its associated time slider. If you delete a database
    correlation whose time slider is being used for the current time slider,
    the time slider will be reset to the time slider of the next best suited
    database correlation. You can use the DeleteDatabaseCorrelation function to
    remove database correlations that you no longer need such as when you
    choose to examine databases that have nothing to do with your current
    databases.


**Example:**

::

  #% visit -cli
  dbs = ("dbA00.pdb", "dbB00.pdb")
  OpenDatabase(dbs[0])
  AddPlot("FilledBoundary", "material(mesh)")
  OpenDatabase(dbs[1])
  AddPlot("FilledBoundary", "material(mesh)")
  DrawPlots()
  CreateDatabaseCorrelation("common", dbs, 1)
  SetTimeSliderState(10)
  DeleteAllPlots()
  DeleteDatabaseCorrelation("common")
  CloseDatabase(dbs[0])
  CloseDatabase(dbs[1])


DeleteExpression
----------------

**Synopsis:**

::

  DeleteExpression(variableName) -> integer


variableName : string
    The name of the expression variable to be deleted.

return type : CLI_return_t
    The DeleteExpression function returns 1 on success and 0 on failure.


**Description:**

    The DeleteExpression function deletes the definition of an expression. The
    variableName argument is a string containing the name of the variable
    expression to be deleted. Any plot that uses an expression that has been
    deleted will fail to regenerate if its attributes are changed.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  DefineScalarExpression("myvar", "sin(u) + cos(w)")
  AddPlot("Pseudocolor", "myvar") # Plot the expression variable.
  DrawPlots()
  DeleteExpression("myvar") # Delete the expression variable myvar.


DeleteNamedSelection
--------------------

**Synopsis:**

::

  DeleteNamedSelection(name) -> integer


name : string
    The name of a named selection.

return type : CLI_return_t
    The DeleteNamedSelection function returns 1 for success and 0 for failure.


**Description:**

    Named Selections allow you to select a group of elements (or particles).
    One typically creates a named selection from a group of elements and then
    later applies the named selection to another plot (thus reducing the
    set of elements displayed to the ones from when the named selection was
    created).  If you have created a named selection that you are no longer
    interested in, you can delete it with the DeleteNamedSelection function.


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/wave*.silo database"
  OpenDatabase(db)
  AddPlot("Pseudocolor", "pressure")
  AddOperator("Clip")
  c = ClipAttributes()
  c.plane1Origin = (0,0.6,0)
  c.plane1Normal = (0,-1,0)
  SetOperatorOption(c)
  DrawPlots()
  CreateNamedSelection("els_above_y")
  SetTimeSliderState(40)
  DeleteNamedSelection("els_above_y")
  CreateNamedSelection("els_above_y")


DeletePlotDatabaseKeyframe
--------------------------

**Synopsis:**

::

  DeletePlotDatabaseKeyframe(plotIndex, frame)


plotIndex : integer
    A zero-based integer value corresponding to a plot's index in the plot
    list.

frame : integer
    A zero-based integer value corresponding to a database keyframe at a
    particular animation frame.


**Description:**

    The DeletePlotDatabaseKeyframe function removes a database keyframe from a
    specific plot. A database keyframe represents the database time state that
    will be used at a given animation frame when VisIt's keyframing mode is
    enabled. The plotIndex argument is a zero-based integer that is used to
    identify a plot in the plot list. The frame argument is a zero-based
    integer that is used to identify the frame at which a database keyframe is
    to be removed for the specified plot.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  k = GetKeyframeAttributes()
  k.enabled,k.nFrames,k.nFramesWasUserSet = 1,20,1
  SetKeyframeAttributes(k)
  AddPlot("Pseudocolor", "pressure")
  SetPlotDatabaseState(0, 0, 60)
  # Repeat time state 60 for the first few animation frames by adding a
  # keyframe at frame 3.
  SetPlotDatabaseState(0, 3, 60)
  SetPlotDatabaseState(0, 19, 0)
  DrawPlots()
  ListPlots()
  # Delete the database keyframe at frame 3.
  DeletePlotDatabaseKeyframe(0, 3)
  ListPlots()


DeletePlotKeyframe
------------------

**Synopsis:**

::

  DeletePlotKeyframe(plotIndex, frame)


plotIndex : integer
    A zero-based integer value corresponding to a plot's index in the plot
    list.

frame : integer
    A zero-based integer value corresponding to a plot keyframe at a
    particular animation frame.


**Description:**

    The DeletePlotKeyframe function removes a plot keyframe from a specific
    plot. A plot keyframe is the set of plot attributes at a specified frame.
    Plot keyframes are used to determine what plot attributes will be used at a
    given animation frame when VisIt's keyframing mode is enabled. The
    plotIndex argument is a zero-based integer that is used to identify a plot
    in the plot list. The frame argument is a zero-based integer that is used
    to identify the frame at which a keyframe is to be removed.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  k = GetKeyframeAttributes()
  k.enabled,k.nFrames,k.nFramesWasUserSet = 1,20,1
  SetKeyframeAttributes(k)
  AddPlot("Pseudocolor", "pressure")
  # Set up plot keyframes so the Pseudocolor plot's min will change
  # over time.
  p0 = PseudocolorAttributes()
  p0.minFlag,p0.min = 1,0.0
  p1 = PseudocolorAttributes()
  p1.minFlag,p1.min = 1, 0.5
  SetPlotOptions(p0)
  SetTimeSliderState(19)
  SetPlotOptions(p1)
  SetTimeSliderState(0)
  DrawPlots()
  ListPlots()
  # Iterate over all animation frames and wrap around to the first one.
  for i in list(range(TimeSliderGetNStates())) + [0]:
      SetTimeSliderState(i)
  # Delete the plot keyframe at frame 19 so the min won't
  # change anymore.
  DeletePlotKeyframe(19)
  ListPlots()
  SetTimeSliderState(10)


DeleteViewKeyframe
------------------

**Synopsis:**

::

  DeleteViewKeyframe(frame)


frame : integer
    A zero-based integer value corresponding to a view keyframe at a
    particular animation frame.


**Description:**

    The DeleteViewKeyframe function removes a view keyframe at a specified
    frame. View keyframes are used to determine what view will be used at a
    given animation frame when VisIt's keyframing mode is enabled. The frame
    argument is a zero-based integer that is used to identify the frame at
    which a keyframe is to be removed.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  k = KeyframeAttributes()
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1,10,1
  SetKeyframeAttributes(k)
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Set some view keyframes
  SetViewKeyframe()
  v1 = GetView3D()
  v1.viewNormal = (-0.66609, 0.337227, 0.665283)
  v1.viewUp = (0.157431, 0.935425, -0.316537)
  SetView3D(v1)
  SetTimeSliderState(9)
  SetViewKeyframe()
  ToggleCameraViewMode()
  # Iterate over the animation frames to watch the view change.
  for i in list(range(10)) + [0]:
      SetTimeSliderState(i)
  # Delete the last view keyframe, which is on frame 9.
  DeleteViewKeyframe(9)
  # Iterate over the animation frames again. The view should stay
  # the same.
  for i in range(10):
      SetTimeSliderState(i)


DeleteWindow
------------

**Synopsis:**

::

  DeleteWindow() -> integer

return type : CLI_return_t
    The DeleteWindow function returns an integer value of 1 for success and 0
    for failure.


**Description:**

    The DeleteWindow function deletes the active visualization window and
    makes the visualization window with the smallest window index the new
    active window. This function has no effect when there is only one remaining
    visualization window.


**Example:**

::

  #% visit -cli
  DeleteWindow() # Does nothing since there is only one window
  AddWindow()
  DeleteWindow() # Deletes the new window.


DemoteOperator
--------------

**Synopsis:**

::

  DemoteOperator(opIndex) -> integer
  DemoteOperator(opIndex, applyToAllPlots) -> integer


opIndex : integer
    A zero-based integer corresponding to the operator that should be demoted.

applyToAllPlots : integer
    An integer flag that causes all plots in the plot list to be affected when
    it is non-zero.

return type : CLI_return_t
    DemoteOperator returns 1 on success and 0 on failure.


**Description:**

    The DemoteOperator function moves an operator closer to the database in
    the visualization pipeline. This allows you to change the order of
    operators that have been applied to a plot without having to remove them
    from the plot. For example, consider moving a Slice to before a Reflect
    operator when it had been the other way around. Changing the order of
    operators can result in vastly different results for a plot. The opposite
    function is PromoteOperator.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hardyglobal")
  AddOperator("Slice")
  s = SliceAttributes()
  s.project2d = 0
  s.originPoint = (0,5,0)
  s.originType=s.Point
  s.normal = (0,1,0)
  s.upAxis = (-1,0,0)
  SetOperatorOptions(s)
  AddOperator("Reflect")
  DrawPlots()
  # Now reflect before slicing. We'll only get 1 slice plane
  # instead of 2.
  DemoteOperator(1)
  DrawPlots()


DisableRedraw
-------------

**Synopsis:**

::

  DisableRedraw()


**Description:**

    The DisableRedraw function prevents the active visualization window from
    ever redrawing itself. This is a useful function to call when performing
    many operations that would cause unnecessary redraws in the visualization
    window. The effects of this function are undone by calling the RedrawWindow
    function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Contour", "u")
  AddPlot("Pseudocolor", "w")
  DrawPlots()
  DisableRedraw()
  AddOperator("Slice")
  # Set the slice operator attributes
  # Redraw now that thw operator attributes are set. This will
  # prevent 1 redraw.
  RedrawWindow()


DrawPlots
---------

**Synopsis:**

::

  DrawPlots() -> integer

return type : CLI_return_t
    The DrawPlots function returns an integer value of 1 for success and 0 for
    failure.


**Description:**

    The DrawPlots function forces all new plots in the plot list to be drawn.
    Plots are added and then their attributes are modified. Finally, the
    DrawPlots function is called to make sure all of the new plots draw
    themselves in the visualization window. This function has no effect if all
    of the plots in the plot list are already drawn.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots() # Draw the new pseudocolor plot.


EnableTool
----------

**Synopsis:**

::

  EnableTool(toolIndex, activeFlag)


toolIndex : integer
    This is an integer that corresponds to an interactive tool.
    (Plane tool = 0, Line tool = 1, Plane tool = 2, Box tool = 3,
    Sphere tool = 4, Axis Restriction tool = 5)

activeFlag : integer
    An integer value of 1 enables the tool while a value of 0 disables the tool.

return : CLI_return_t
    The EnableToole function returns 1 on success and 0 on failure.


**Description:**

    The EnableTool function is used to set the enabled state of an interactive
    tool in the active visualization window. The toolIndex argument is an
    integer index that corresponds to a certain tool. The activeFlag argument
    is an integer value (0 or 1) that indicates whether to turn the tool on or
    off.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  EnableTool(0, 1) # Turn on the line tool.
  EnableTool(1,1) # Turn on the plane tool.
  EnableTool(2,1) # Turn on the sphere tool.
  EnableTool(2,0) # Turn off the sphere tool.


EvalCubic
---------

**Synopsis:**

::

  EvalCubic(t, c0, c1, c2, c3) -> f(t)


t : double
    A floating point number in the range [0., 1.] that represents the distance
    from c0 to c3.

c0 : arithmetic expression object
    The first control point. f(0) = c0. Any object that can be used in an
    arithmetic expression can be passed for c0.

c1 : arithmetic expression object
    The second control point. Any object that can be used in an arithmetic
    expression can be passed for c1.

c2 : arithmetic expression object
    The third control point. Any object that can be used in an arithmetic
    expression can be passed for c2.

c3 : arithmetic expression object
    The last control point. f(1) = c3. Any object that can be used in an
    arithmetic expression can be passed for c3.

return : double
    The EvalCubic function returns the interpolated value for t taking into
    account the control points that were passed in.


**Description:**

    The EvalCubic function takes in four objects and blends them using a cubic
    polynomial and returns the blended value.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  v0 = GetView3D()
  # rotate the plots
  v1 = GetView3D()
  # rotate the plots again.
  v2 = GetView3D()
  # rotate the plots one last time.
  v3 = GetView3D()
  # Fly around the plots using the views that have been specified.
  nSteps = 100
  for i in range(nSteps):
      t = float(i) / float(nSteps - 1)
      newView = EvalCubic(t, v0, v1, v2, v3)
      SetView3D(newView)


EvalCubicSpline
---------------

**Synopsis:**

::

  EvalCubicSpline(t, weights, values) -> f(t)


t : double
    A floating point number in the range [0., 1.] that represents the distance
    from the first control point to the last control point.

weights : tuple of doubles
    A tuple of N floating point values in the range [0., 1.] that represent
    how far along in parameterized space, the values will be located.

values : tuple of arithmetic expression object
    A tuple of N objects to be blended. Any objects that can be used in
    arithmetic expressions can be passed in.

return : double
    The EvalCubicSpline function returns the interpolated value for t
    considering the objects that were passed in.


**Description:**
    The EvalCubicSpline function takes in N objects to be blended and blends
    them using piece-wise cubic polynomials and returns the blended value.


EvalLinear
----------

**Synopsis:**

::

  EvalLinear(t, value1, value2) -> f(t)


t : double
    A floating point value in the range [0., 1.] that represents the distance
    between the first and last control point in parameterized space.

value1 : arithmetic expression object
    Any object that can be used in an arithmetic expression. f(0) = value1.

value2 : arithmetic expression object
    Any object that can be used in an arithmetic expression. f(1) = value2.

return : double
    The EvalLinear function returns an interpolated value for t based on the
    objects that were passed in.


**Description:**
    The EvalLinear function linearly interpolates between two values and
    returns the result.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  c0 = GetView3D()
  c1 = GetView3D()
  c1.viewNormal = (-0.499159, 0.475135, 0.724629)
  c1.viewUp = (0.196284, 0.876524, -0.439521)
  nSteps = 100
  for i in range(nSteps):
      t = float(i) / float(nSteps - 1)
      v = EvalLinear(t, c0, c1)
      SetView3D(v)


EvalQuadratic
-------------

**Synopsis:**

::

  EvalQuadratic(t, c0, c1, c2) -> f(t)


t : double
    A floating point number in the range [0., 1.] that represents the distance
    from c0 to c3.

c0 : arithmetic expression object
    The first control point. f(0) = c0. Any object that can be used in an
    arithmetic expression can be passed for c0.

c1 : arithmetic expression object
    The second control point. Any object that can be used in an arithmetic
    expression can be passed for c1.

c2 : arithmetic expression object
    The last control point. f(1) = c2. Any object that can be used in an
    arithmetic expression can be passed for c2.

return : double
    The EvalQuadratic function returns the interpolated value for t taking
    into account the control points that were passed in.


**Description:**
    The EvalQuadratic function takes in four objects and blends them using a
    cubic polynomial and returns the blended value.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  v0 = GetView3D()
  # rotate the plots 
  v1 = GetView3D()
  # rotate the plots one last time.
  v2 = GetView3D()
  # Fly around the plots using the views that have been specified.
  nSteps = 100
  for i in range(nSteps):
      t = float(i) / float(nSteps - 1)
      newView = EvalQuadratic(t, v0, v1, v2)
      SetView3D(newView)


ExecuteMacro
------------

**Synopsis:**

::

  ExecuteMacro(name) -> value


name : string
    The name of the macro to execute.

return type : value
    The ExecuteMacro function returns the value returned from the user's macro function.


**Description:**

    The ExecuteMacro function lets you call a macro function that was previously
    registered using the RegisterMacro method. Once macros are registered with a
    name, this function can be called whenever the macro function associated with
    that name needs to be called. The VisIt gui uses this function to tell the
    Python interface when macros need to be executed in response to user button
    clicks.


**Example:**

::

  def SetupMyPlots():
      OpenDatabase('noise.silo')
      AddPlot('Pseudocolor', 'hardyglobal')
      DrawPlots()
      
  RegisterMacro('Setup My Plots', SetupMyPlots)
  ExecuteMacro('Setup My Plots')


ExportDatabase
--------------

**Synopsis:**

::

  ExportDatabase(e) -> integer
  ExportDatabase(e, o) -> integer


e : ExportDBAttributes object
    An object of type ExportDBAttributes.  This object specifies the options
    for exporting the database.

o : dictionary
    A dictionary containing a key/value mapping to set options needed by the
    database exporter.  The default values can be obtained in the appropriate
    format using GetExportOptions('plugin').

return type : CLI_return_t
    Returns 1 on success, 0 on failure.


**Description:**

    The ExportDatabase function exports the active plot for the current window
    to a file.  The format of the file, name, and variables to be saved are
    specified using the ExportDBAttributes argument.
    Note that this functionality is distinct from the geometric formats of
    SaveWindow, such as STL.  SaveWindow can only save surfaces (triangle
    meshes), while ExportDatabase can export an entire three dimensional data
    set.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv3d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set the export database attributes.
  e = ExportDBAttributes()
  e.db_type = "Silo"
  e.variables = ("u", "v")
  e.filename = "test_ex_db"
  ExportDatabase(e)


Expressions
-----------

**Synopsis:**

::

  Expressions() -> tuple of expression tuples


return type : tuple of expression tuples
    The Expressions function returns a tuple of tuples that contain two
    strings that give the expression name and definition.


**Description:**

    The Expressions function returns a tuple of tuples that contain two
    strings that give the expression name and definition. This function is
    useful for listing the available expressions or for iterating through a
    list of expressions in order to create plots.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4)
  DefineScalarExpression("sin_u", "sin(u)")
  DefineScalarExpression("cos_u", "cos(u)")
  DefineScalarExpression("neg_u", "-u")
  DefineScalarExpression("bob", "sin_u + cos_u")
  for i in range(1,5):
      SetActiveWindow(i)
      OpenDatabase("/usr/gapps/visit/data/globe.silo")
      exprName = Expressions()[i-1][0]
      AddPlot("Pseudocolor", exprName)
      DrawPlots()


GetActiveContinuousColorTable
-----------------------------

**Synopsis:**

::

  GetActiveContinuousColorTable() -> string


return type : string
    Both functions return a string object containing the name of a color table.


**Description:**

    A color table is a set of color values that are used as the colors for
    plots. VisIt supports two flavors of color table: continuous and discrete.
    A continuous color table is defined by a small set of color control points
    and the colors specified by the color control points are interpolated
    smoothly to fill in any gaps. Continuous color tables are used for plots
    that need to be colored smoothly by a variable (e.g. Pseudocolor plot). A
    discrete color table is a set of color control points that are used to
    color distinct regions of a plot (e.g. Subset plot). VisIt supports the
    notion of default continuous and default discrete color tables so plots can
    just use the "default" color table. This lets you change the color table
    used by many plots by just changing the "default" color table. The
    GetActiveContinuousColorTable function returns the name of the default
    continuous color table. The GetActiveDiscreteColorTable function returns
    the name of the default discrete color table.


**Example:**

::

  #% visit -cli
  print("Default continuous color table: %s" % GetActiveContinuousColorTable())
  print("Default discrete color table: %s" % GetActiveDiscreteColorTable())


GetActiveDiscreteColorTable
---------------------------

**Synopsis:**

::

  GetActiveDiscreteColorTable() -> string


return type : string
    Both functions return a string object containing the name of a color table.


**Description:**

    A color table is a set of color values that are used as the colors for
    plots. VisIt supports two flavors of color table: continuous and discrete.
    A continuous color table is defined by a small set of color control points
    and the colors specified by the color control points are interpolated
    smoothly to fill in any gaps. Continuous color tables are used for plots
    that need to be colored smoothly by a variable (e.g. Pseudocolor plot). A
    discrete color table is a set of color control points that are used to
    color distinct regions of a plot (e.g. Subset plot). VisIt supports the
    notion of default continuous and default discrete color tables so plots can
    just use the "default" color table. This lets you change the color table
    used by many plots by just changing the "default" color table. The
    GetActiveContinuousColorTable function returns the name of the default
    continuous color table. The GetActiveDiscreteColorTable function returns
    the name of the default discrete color table.


**Example:**

::

  #% visit -cli
  print("Default continuous color table: %s" % GetActiveContinuousColorTable())
  print("Default discrete color table: %s" % GetActiveDiscreteColorTable())


GetActiveTimeSlider
-------------------

**Synopsis:**

::

  GetActiveTimeSlider() -> string

return type : string
    The GetActiveTimeSlider function returns a string containing the name of
    the active time slider.


**Description:**

    VisIt can support having multiple time sliders when you have opened more
    than one time-varying database. You can then use each time slider to
    independently change time states for each database or you can use a
    database correlation to change time states for all databases
    simultaneously. Every time-varying database has a database correlation and
    every database correlation has its own time slider. If you want to query to
    determine which time slider is currently the active time slider, you can
    use the GetActiveTimeSlider function.


**Example:**

::

  #% visit -cli
  OpenDatabase("dbA00.pdb")
  AddPlot("FilledBoundary", "material(mesh)")
  OpenDatabase("dbB00.pdb")
  AddPlot("FilledBoundary", "materials(mesh)")
  print("Active time slider: %s" % GetActiveTimeSlider())
  CreateDatabaseCorrelation("common", ("dbA00.pdb", "dbB00.pdb"), 2)
  print("Active time slider: %s" % GetActiveTimeSlider())


GetAnimationAttributes
----------------------

**Synopsis:**

::

  GetAnimationAttributes() -> AnimationAttributes object


return type : AnimationAttributes object
    The GetAnimationAttributes function returns an AnimationAttributes object.


**Description:**

    This function returns the current animation attributes, which contain the
    animation mode, increment, and playback speed.


**Example:**

::

  a = GetAnimationAttributes()
  print(a)


GetAnimationTimeout
-------------------

**Synopsis:**

::

  GetAnimationTimeout() -> integer

return type : CLI_return_t
    The GetAnimationTimeout function returns an integer that contains the time
    interval, measured in milliseconds, between the rendering of animation
    frames.


**Description:**

    The GetAnimationTimeout returns an integer that contains the time
    interval, measured in milliseconds, between the rendering of animation
    frames.


**Example:**

::

  #% visit -cli
  print("Animation timeout = %d" % GetAnimationTimeout())


GetAnnotationAttributes
-----------------------

**Synopsis:**

::

  GetAnnotationAttributes() -> AnnotationAttributes object

return type : AnnotationAttributes object
    The GetAnnotationAttributes function returns an AnnotationAttributes
    object that contains the annotation settings for the active visualization
    window.


**Description:**

    The GetAnnotationAttributes function returns an AnnotationAttributes
    object that contains the annotation settings for the active visualization
    window. It is often useful to retrieve the annotation settings and modify
    them to suit the visualization.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  a = GetAnnotationAttributes()
  print(a)
  a.backgroundMode = a.BACKGROUNDMODE_GRADIENT
  a.gradientColor1 = (0, 0, 255)
  SetAnnotationAttributes(a)


GetAnnotationObject
-------------------

**Synopsis:**

::

  GetAnnotationObject(string) -> Annotation object


string : string
    The name of the annotation object as returned by GetAnnotationObjectNames.

return type : Annotation object
    GetAnnotationObject returns a reference to an annotation object that was
    created using the CreateAnnotationObject function.


**Description:**

    GetAnnotationObject returns a reference to an annotation object that was
    created using the CreateAnnotationObject function. The string
    argument specifies the name of the desired annotation object. It must be
    one of the names returned by GetAnnotationObjectNames. This function is not
    currently necessary unless the annotation object that you used to create an
    annotation has gone out of scope and you need to create another reference
    to the object to set its properties. Also note that although this function
    will apparently also accept an integer index, that mode of access is not
    reliably and should be avoided.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  a = CreateAnnotationObject("TimeSlider")
  GetAnnotationObjectNames()
  ["plot0000", "TimeSlider1"]
  ref = GetAnnotationObject("TimeSlider1")
  print(ref)


GetAnnotationObjectNames
------------------------

**Synopsis:**

::

  GetAnnotationObjectNames() -> tuple of strings

return type : tuple of strings
    GetAnnotationObjectNames returns a tuple of strings of the names of all
    annotation objects defined for the currently active window.



**Example:**

::

  names = GetAnnotationObjectNames()
  names
  ["plot0000", "Line2D1", "TimeSlider1"]


GetCallbackArgumentCount
------------------------

**Synopsis:**

::

  GetCallbackArgumentCount(callbackName) -> integer


callbackName : string
    The name of a callback function. This name is a member of the tuple
    returned by GetCallbackNames().

return type : CLI_return_t
    The GetCallbackArgumentCount function returns the number of arguments
    associated with a particular callback function.



**Example:**

::

  cbName = 'OpenDatabaseRPC'
  count = GetCallbackArgumentCount(cbName)
  print('The number of arguments for %s is: %d' % (cbName, count))


GetCallbackNames
----------------

**Synopsis:**

::

  GetCallbackNames() -> tuple of string objects

return type : tuple of string objects
    GetCallbackNames returns a tuple containing the names of valid callback
    function identifiers for use in RegisterCallback().


**Description:**

    The GetCallbackNames function returns a tuple containing the names of valid
    callback function identifiers for use in RegisterCallback().


**Example:**

::

  import visit
  print(visit.GetCallbackNames())


GetDatabaseNStates
------------------

**Synopsis:**

::

  GetDatabaseNStates() -> integer

return type : CLI_return_t
    Returns the number of time states in the active database or 0 if there is
    no active database.


**Description:**

    GetDatabaseNStates returns the number of time states in the active
    database, which is not the same as the number of states in the active time
    slider. Time sliders can have different lengths due to database
    correlations and keyframing. Use this function when you need the actual
    number of time states in the active database.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave*.silo database")
  print("Number of time states: %d" % GetDatabaseNStates())


GetDebugLevel
-------------

**Synopsis:**

::

  GetDebugLevel() -> integer

return type : CLI_return_t
    The GetDebugLevel function returns the debug level of the VisIt module.


**Description:**

    The GetDebugLevel and SetDebugLevel functions are used when debugging
    VisIt Python scripts. The GetDebugLevel function can be used in Python
    scripts to alter the behavior of the script. For instance, the debug
    level can be used to selectively print values to the console.


**Example:**

::

  #% visit -cli -debug 2
  print("VisIt's debug level is: %d" % GetDebugLevel())


GetDefaultFileOpenOptions
-------------------------

**Synopsis:**

::

  GetDefaultFileOpenOptions(pluginName) -> dictionary


pluginName : string
    The name of a plugin.

return type : dictionary
    Returns a dictionary containing the options.


**Description:**

    GetDefaultFileOpenOptions returns the current options used to open new
    files when a specific plugin is triggered.


**Example:**

::

  #% visit -cli
  OpenMDServer()
  opts = GetDefaultFileOpenOptions("VASP")
  opts["Allow multiple timesteps"] = 1
  SetDefaultFileOpenOptions("VASP", opts)
  OpenDatabase("CHGCAR")


GetDomains
----------

**Synopsis:**

::

  GetDomains() -> tuple of strings


return type : tuple of strings
    GetDomains returns a tuple of strings.


**Description:**

    GetDomains returns a tuple containing the names of all of the domain
    subsets for a plot that was created using a database with multiple domains.
    This function can be used in specialized logic that iterates over domains
    to turn them on or off in some programmed way.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/multi_ucd3d.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  doms = GetDomains()
  print(doms)
  # Turn off all but the last domain, one after the other.
  for d in doms[:-1]:
      TurnDomainsOff(d)


GetEngineList
-------------

**Synopsis:**

::

  GetEngineList() -> tuple of strings
  GetEngineList(flag) -> tuple of tuples of strings


flag : integer
    If flag is a non-zero integer then the function returns a tuple of tuples
    with information about simulations.

return type : tuple of strings
    GetEngineList returns a tuple of strings that contain the names of the
    computers on which compute engines are running. If flag is a non-zero
    integer argument then the function returns a tuple of tuples where each
    tuple is of length 2. Element 0 contains the names of the computers where
    the engines are running. Element 1 contains the names of the simulations
    being run.


**Description:**

    The GetEngineList function returns a tuple of strings containing the names
    of the computers on which compute engines are running. This function can be
    useful if engines are going to be closed and opened explicitly in the
    Python script. The contents of the tuple can be used to help determine
    which compute engines should be closed or they can be used to determine if
    a compute engine was successfully launched.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  OpenDatabase("mcr:/usr/gapps/visit/data/globe.silo")
  AddPlot("Mesh", "mesh1")
  DrawPlots()
  for name in GetEngineList():
      print("VisIt has a compute engine running on %s" % name)
      CloseComputeEngine(GetEngineList()[1])


GetEngineProperties
-------------------

**Synopsis:**

::

  GetEngineProperties()            -> EngineProperties object
  GetEngineProperties(engine)      -> EngineProperties object
  GetEngineProperties(engine, sim) -> EngineProperties object


engine
    When engine is passed and it matches one of the computer names returned
    from GetEngineList() then the EngineProperties object for that engine is
    returned.

sim
    When both engine and sim arguments are passed, then the EngineProperties
    object for the simulation is returned.

return type : EngineProperties object
    The EngineProperties object for the specified compute engine/sim.


**Description:**

    GetEngineProperties returns an EngineProperties object containing the properties
    for the specified compute engine/sim. The EngineProperties let you discover
    information such as number of processors, etc for a compute engine/sim.


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/globe.silo"
  OpenDatabase(db)
  props = GetEngineProperties(GetEngineList()[0])


GetGlobalAttributes
-------------------

**Synopsis:**

::

  GetGlobalAttributes() -> GlobalAttributes object

return type : GlobalAttributes object
    Returns a GlobalAttributes object that has been initialized.


**Description:**

    The GetGlobalAttributes function returns a GlobalAttributes object that
    has been initialized with the current state of the viewer proxy's
    GlobalAttributes object. The GlobalAttributes object contains read-only
    information about the list of sources, the list of windows, and various
    flags that can be queried.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  g = GetGlobalAttributes()
  print(g)


GetGlobalLineoutAttributes
--------------------------

**Synopsis:**

::

  GetGlobalLineoutAttributes() -> GlobalLineoutAttributes object

return type : GlobalLineoutAttributes object
    Returns an initialized GlobalLineoutAttributes object.


**Description:**

    The GetGlobalLineoutAttributes function returns an initialized
    GlobalLineoutAttributes object. The GlobalLineoutAttributes, as suggested
    by its name, contains global properties that apply to all lineouts. You can
    use the GlobalLineoutAttributes object to turn on lineout sampling, specify
    the destination window, etc. for curve plots created as a result of
    performing lineouts. Once you make changes to the object by setting its
    properties, use the SetGlobalLineoutAttributes function to make VisIt use
    the modified global lineout attributes.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4)
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  g = GetGlobalLineoutAttributes()
  print(g)
  g.samplingOn = 1
  g.windowId = 4
  g.createWindow = 0
  g.numSamples = 100
  SetGlobalLineoutAttributes(g)
  Lineout((-3,2),(3,3),("default"))


GetInteractorAttributes
-----------------------

**Synopsis:**

::

  GetInteractorAttributes() -> InteractorAttributes object

return type : InteractorAttributes object
    Returns an initialized InteractorAttributes object.


**Description:**

    The GetInteractorAttributes function returns an initialized
    InteractorAttributes object. The InteractorAttributes object can be used to
    set certain interactor properties. Interactors, can be thought of as how
    mouse clicks and movements are translated into actions in the vis window.
    To set the interactor attributes, first get the interactor attributes using
    the GetInteractorAttributes function. Once you've set the object's
    properties, call the SetInteractorAttributes function to make VisIt use the
    new interactor attributes.


**Example:**

::

  #% visit -cli
  ia = GetInteractorAttributes()
  print(ia)
  ia.showGuidelines = 0
  SetInteractorAttributes(ia)


GetKeyframeAttributes
---------------------

**Synopsis:**

::

  GetKeyframeAttributes() -> KeyframeAttributes object

return type : KeyframeAttributes object
    GetKeyframeAttributes returns an initialized KeyframeAttributes object.


**Description:**

    Use the GetKeyframeAttributes function when you want to examine a
    KeyframeAttributes object so you can determine VisIt's state when it is in
    keyframing mode. The KeyframeAttributes object allows you to see whether
    VisIt is in keyframing mode and, if so, how many animation frames are in
    the current keyframe animation.


**Example:**

::

  #% visit -cli
  k = GetKeyframeAttributes()
  print(k)
  k.enabled,k.nFrames,k.nFramesWasUserSet = 1, 100, 1
  SetKeyframeAttributes(k)


GetLastError
------------

**Synopsis:**

::

  GetLastError() -> string

return type : string
    GetLastError returns a string containing the last error message that VisIt
    issued.


**Description:**

    The GetLastError function returns a string containing the last error
    message that VisIt issued.


**Example:**

::

  #% visit -cli
  OpenDatabase("/this/database/does/not/exist")
  print("VisIt Error: %s" % GetLastError())


GetLight
--------

**Synopsis:**

::

  GetLight(index) -> LightAttributes object


index : integer
    A zero-based integer index into the light list. Index can be in the range
    [0,7].

return type : LightAttributes object
    GetLight returns a LightAttributes object.


**Description:**

    The GetLight function returns a LightAttributes object containing the
    attributes for a specific light. You can use the LightAttributes object
    that GetLight returns to set light properties and then you can pass the
    object to SetLight to make VisIt use the light properties that you've set.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "w")
  p = PseudocolorAttributes()
  p.colorTableName = "xray"
  SetPlotOptions(p)
  DrawPlots()
  InvertBackgroundColor()
  light = GetLight(0)
  print(light)
  light.enabledFlag = 1
  light.direction = (0,-1,0)
  light.color = (255,0,0,255)
  SetLight(0, light)
  light.color,light.direction = (0,255,0,255), (-1,0,0)
  SetLight(1, light)


GetLocalHostName
----------------

**Synopsis:**

::

  GetLocalHostName() -> string


return type : string
    Both functions return a string.


**Description:**

    The GetLocalHostName function returns a string that contains the name of
    the local computer.


**Example:**

::

  #% visit -cli
  print("Local machine name is: %s" % GetLocalHostName())
  print("My username: %s" % GetLocalUserName())


GetLocalUserName
----------------

**Synopsis:**

::

  GetLocalUserName() -> string


return type : string
    Both functions return a string.


**Description:**

    The GetLocalUserName function returns a string containing the name of
    the user running VisIt.


**Example:**

::

  #% visit -cli
  print("Local machine name is: %s" % GetLocalHostName())
  print("My username: %s" % GetLocalUserName())


GetMachineProfile
-----------------

**Synopsis:**

::

  GetMachineProfile(hostname) -> MachineProfile object


hostname : string


return type : MachineProfile object
    MachineProfile for hostname.


**Description:**

    Gets the MachineProfile for a given hostname


GetMachineProfileNames
----------------------

**Synopsis:**

::

  GetMachineProfileNames() -> [hostname1, hostname2, ...]

return type : list of strings
    A list of MachineProfile hostnames


**Description:**

    Returns a list of hostnames that can be used to get a specific MachineProfile


GetMaterialAttributes
---------------------

**Synopsis:**

::

  GetMaterialAttributes() -> MaterialAttributes object


return type : MaterialAttributes object
    Returns a MaterialAttributes object.


**Description:**

    The GetMaterialAttributes function returns a MaterialAttributes object
    that contains VisIt's current material interface reconstruction settings.
    You can set properties on the MaterialAttributes object and then pass it to
    SetMaterialAttributes to make VisIt use the new material attributes that
    you've specified:


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/allinone00.pdb")
  AddPlot("Pseudocolor", "mesh/mixvar")
  p = PseudocolorAttributes()
  p.min,p.minFlag = 4.0, 1
  p.max,p.maxFlag = 13.0, 1
  SetPlotOptions(p)
  DrawPlots()
  # Tell VisIt to always do material interface reconstruction.
  m = GetMaterialAttributes()
  m.forceMIR = 1
  SetMaterialAttributes(m)
  ClearWindow()
  # Redraw the plot forcing VisIt to use the mixed variable information.
  DrawPlots()


GetMaterials
------------

**Synopsis:**

::

  GetMaterials() -> tuple of strings


return type : tuple of strings
    The GetMaterials function returns a tuple of strings.


**Description:**

    The GetMaterials function returns a tuple of strings containing the names
    of the available materials for the current plot's database. Note that the
    active plot's database must have materials for this function to return a
    tuple that has any string objects in it. Also, you must have at least one
    plot. You can use the materials returned by the GetMaterials function for a
    variety of purposes including turning materials on or off.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/allinone00.pdb")
  AddPlot("Pseudocolor", "mesh/mixvar")
  DrawPlots()
  mats = GetMaterials()
  for m in mats[:-1]:
      TurnMaterialOff(m)


GetMeshManagementAttributes
---------------------------

**Synopsis:**

::

  GetMeshManagementAttributes() -> MeshmanagementAttributes object

return type : MeshmanagementAttributes object
    Returns a MeshmanagementAttributes object.


**Description:**

    The GetMeshmanagementAttributes function returns a MeshmanagementAttributes object
    that contains VisIt's current mesh discretization settings.
    You can set properties on the MeshManagementAttributes object and then pass it to
    SetMeshManagementAttributes to make VisIt use the new material attributes that
    you've specified:


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/csg.silo")
  AddPlot("Mesh", "csgmesh")
  DrawPlots()
  # Tell VisIt to always do material interface reconstruction.
  mma = GetMeshManagementAttributes()
  mma.discretizationTolernace = (0.01, 0.025)
  SetMeshManagementAttributes(mma)
  ClearWindow()
  # Redraw the plot forcing VisIt to use the mixed variable information.
  DrawPlots()


GetMetaData
-----------

**Synopsis:**

::

  GetMetaData(db) -> avtDatabaseMetaData object
  GetMetaData(db, ts) -> avtDatabaseMetaData object


db : string
    The name of the database for which to return metadata.

ts : integer
    An optional integer indicating the time state at which to open the database.

return type : avtDatabaseMetaData object
    The GetMetaData function returns an avtDatabaseMetaData object.


**Description:**

    VisIt relies on metadata to populate its variable menus and make important
    decisions. Metadata can be used to create complex scripts whose behavior
    adapts based on the contents of the database.


**Example:**

::

  md = GetMetaData('noise.silo')
  for i in range(md.GetNumScalars()):
      AddPlot('Pseudocolor', md.GetScalars(i).name)
  DrawPlots()


GetNumPlots
-----------

**Synopsis:**

::

  GetNumPlots() -> integer

return type : CLI_return_t
    Returns the number of plots in the active window.


**Description:**

    The GetNumPlots function returns the number of plots in the active window.


**Example:**

::

  #% visit -cli
  print("Number of plots", GetNumPlots())
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "d")
  print("Number of plots", GetNumPlots())
  AddPlot("Mesh", "curvmesh2d")
  DrawPlots()
  print("Number of plots", GetNumPlots())


GetOperatorOptions
------------------

**Synopsis:**

::

  GetOperatorOptions(index) -> operator attributes object


index : integer
    The integer index of the operator within the plot's list of operators.

return type : operator attributes object
    The GetOperatorOptions function returns an operator attributes object.


**Description:**

    This function is provided to make it easy to probe the current attributes for
    a specific operator on the active plot.


**Example:**

::

  AddPlot('Pseudocolor', 'temperature')
  AddOperator('Transform')
  AddOperator('Transform')
  t = GetOperatorOptions(1)
  print('Attributes for the 2nd Transform operator:', t)


GetPickAttributes
-----------------

**Synopsis:**

::

  GetPickAttributes() -> PickAttributes object

return type : PickAttributes object
    GetPickAttributes returns a PickAttributes object.


**Description:**

    The GetPickAttributes object returns the pick settings that VisIt is
    currently using when it performs picks. These settings mainly determine
    which pick information is displayed when pick results are printed out but
    they can also be used to select auxiliary variables and generate time
    curves. You can examing the settings and you can set properties on the
    returned object. Once you've changed pick settings by setting properties on
    the object, you can pass the altered object to the SetPickAttributes
    function to force VisIt to use the new pick settings.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/allinone00.pdb")
  AddPlot("Pseudocolor", "mesh/ireg")
  DrawPlots()
  p = GetPickAttributes()
  print(p)
  p.variables = ("default", "mesh/a", "mesh/mixvar")
  SetPickAttributes(p)
  # Now do some interactive picks and you'll see pick information
  # for more than 1 variable.
  p.doTimeCurve = 1
  SetPickAttributes(p)
  # Now do some interactive picks and you'll get time-curves in
  # a new window.


GetPickOutput
-------------

**Synopsis:**

::

  GetPickOutput() -> string

return type : string
    GetPickOutput returns a string containing the output from the last pick.


**Description:**

    The GetPickOutput returns a string object that contains the output from
    the last pick.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  ZonePick(coord=(0.4, 0.6, 0), vars=("default", "u", "v"))
  s = GetPickOutput()
  print(s)


GetPickOutputObject
-------------------

**Synopsis:**

::

  GetPickOutputObject() -> dictionary

return type : dictionary
    GetPickOutputObject returns a dictionary produced by the last pick.


**Description:**

    GetPickOutputObject returns a dictionary object containing output from the
    last pick.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  ZonePick(coord=(0.4, 0.6, 0), vars=("default", "u", "v"))
  o = GetPickOutputObject()
  print(o)


GetPipelineCachingMode
----------------------

**Synopsis:**

::

  GetPipelineCachingMode() -> integer

return type : CLI_return_t
    The GetPipelineCachingMode function returns 1 if pipelines are being
    cached and 0 otherwise.


**Description:**

    The GetPipelineCachingMode function returns whether or not pipelines are
    being cached in the viewer. For animations of long time sequences, it is
    often useful to turn off pipeline caching so the viewer does not run out of
    memory.


**Example:**

::

  #%visit -cli
  offon = ("off", "on")
  print("Pipeline caching is %s" % offon[GetPipelineCachingMode()])


GetPlotInformation
------------------

**Synopsis:**

::

  GetPlotInformation() -> dictionary

return type : dictionary
    GetPlotInformation returns a dictionary.


**Description:**

    The GetPlotInformation function returns information about the active plot.
    For example, a Curve plot will return the xy pairs that comprise the
    curve.  The tuple is arranged <x1, y1, x2, y2, ..., xn, yn>.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  Lineout((0, 0), (1, 1))
  SetActiveWindow(2)
  info = GetPlotInformation()
  lineout = info["Curve"]
  print("The first lineout point is: [%g, %g] " % lineout[0], lineout[1])


GetPlotList
-----------

**Synopsis:**

::

  GetPlotList() -> PlotList object


return type : PlotList object
    The GetPlotList function returns a PlotList object.


**Description:**

    The GetPlotList function returns a copy of the plot list that gets exchanged
    between VisIt's viewer and its clients. The plot list object contains the list
    of plots, along with the databases, and any operators that are applied to each
    plot. Changing this object has NO EFFECT but it can be useful when writing
    complex functions that need to know about the plots and operators that exist
    within a visualization window


**Example:**

::

  # Copy plots (without operators to window 2)
  pL = GetPlotList()
  AddWindow()
  for i in range(pL.GetNumPlots()):
      AddPlot(PlotPlugins()[pL.GetPlots(i).plotType], pL.GetPlots(i).plotVar)
  DrawPlots()


GetPlotOptions
--------------

**Synopsis:**

::

  GetPlotOptions() -> plot attributes object


return type : plot attributes object
    The GetPlotOptions function returns a plot attributes object whose type varies
    depending the selected plots.


**Description:**

    This function is provided to make it easy to probe the current attributes for
    the selected plot.


**Example:**

::

  pc = GetPlotOptions()
  pc.legend = 0
  SetPlotOptions(pc)


GetPreferredFileFormats
-----------------------

**Synopsis:**

::

  GetPreferredFileFormats() -> tuple of strings

return type : tuple of strings
    The GetPreferredFileFormats returns the current list of preferred plugins.


**Description:**

    The GetPreferredFileFormats method is a way to get the list of
    file format reader plugins which are tried before any others.
    These IDs are full IDs, not just names, and are tried in order.


**Example:**

::

  GetPreferredFileFormats()
  # returns ('Silo_1.0',)


GetQueryOutputObject
--------------------

**Synopsis:**

::

  GetQueryOutputObject() -> dictionary or value

return type : dictionary or value
    GetQueryOutputObject returns an xml string produced by the last query.


**Description:**

    Both the GetQueryOutputString and GetQueryOutputValue functions return
    information about the last query to be executed but the type of information
    returns differs. GetQueryOutputString returns a string containing the
    output of the last query. GetQueryOutputValue returns a single number or
    tuple of numbers, depending on the nature of the last query to be executed.
    GetQueryOutputXML and GetQueryOutputObject expose more complex query output.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  Query("MinMax")
  print(GetQueryOutputString())
  print("The min is: %g and the max is: %g" % GetQueryOutputValue())


GetQueryOutputString
--------------------

**Synopsis:**

::

  GetQueryOutputString() -> string

return type : string
    GetQueryOutputString returns a string.


**Description:**

    Both the GetQueryOutputString and GetQueryOutputValue functions return
    information about the last query to be executed but the type of information
    returns differs. GetQueryOutputString returns a string containing the
    output of the last query. GetQueryOutputValue returns a single number or
    tuple of numbers, depending on the nature of the last query to be executed.
    GetQueryOutputXML and GetQueryOutputObject expose more complex query output.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  Query("MinMax")
  print(GetQueryOutputString())
  print("The min is: %g and the max is: %g" % GetQueryOutputValue())


GetQueryOutputValue
-------------------

**Synopsis:**

::

  GetQueryOutputValue() -> double, tuple of doubles

return type : double, tuple of doubles
    GetQueryOutputValue returns a single double precision number or a tuple of
    double precision numbers.


**Description:**

    Both the GetQueryOutputString and GetQueryOutputValue functions return
    information about the last query to be executed but the type of information
    returns differs. GetQueryOutputString returns a string containing the
    output of the last query. GetQueryOutputValue returns a single number or
    tuple of numbers, depending on the nature of the last query to be executed.
    GetQueryOutputXML and GetQueryOutputObject expose more complex query output.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  Query("MinMax")
  print(GetQueryOutputString())
  print("The min is: %g and the max is: %g" % GetQueryOutputValue())


GetQueryOutputXML
-----------------

**Synopsis:**

::

  GetQueryOutputXML() -> string

return type : string
    GetQueryOutputXML returns an xml string produced by the last query.


**Description:**

    Both the GetQueryOutputString and GetQueryOutputValue functions return
    information about the last query to be executed but the type of information
    returns differs. GetQueryOutputString returns a string containing the
    output of the last query. GetQueryOutputValue returns a single number or
    tuple of numbers, depending on the nature of the last query to be executed.
    GetQueryOutputXML and GetQueryOutputObject expose more complex query output.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  Query("MinMax")
  print(GetQueryOutputString())
  print("The min is: %g and the max is: %g" % GetQueryOutputValue())


GetQueryOverTimeAttributes
--------------------------

**Synopsis:**

::

  GetQueryOverTimeAttributes() -> QueryOverTimeAttributes object

return type : QueryOverTimeAttributes object
    GetQueryOverTimeAttributes returns a QueryOverTimeAttributes object.


**Description:**

    The GetQueryOverTimeAttributes function returns a QueryOverTimeAttributes
    object containing the settings that VisIt currently uses for query over
    time. You can use the returned object to change those settings by first
    setting object properties and then by passing the modified object to the
    SetQueryOverTimeAttributes function.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4)
  OpenDatabase("/usr/gapps/visit/data/allinone00.pdb")
  AddPlot("Pseudocolor", "mesh/mixvar")
  DrawPlots()
  qot = GetQueryOverTimeAttributes()
  print(qot)
  # Make queries over time go to window 4.
  qot.createWindow,q.windowId = 0, 4
  SetQueryOverTimeAttributes(qot)
  QueryOverTime("Min")
  # Make queries over time only use half of the number of time states.
  endTime = GetDatabaseNStates() / 2
  QueryOverTime("Min", end_time=endTime)
  ResetView()


GetQueryParameters
------------------

**Synopsis:**

::

  GetQueryParameters(name) -> dictionary

return type : dictionary
    A python dictionary.


**Description:**

    The GetQueryParameters function returns a Python dictionary containing
    the default parameters for the named query, or None if the query does
    not accept additional parameters.  The returned dictionary (if any) can
    then be modified if necessary and passed back as an argument to the
    Query function.


**Example:**

::

  #% visit -cli
  minMaxInput = GetQueryParameters("MinMax")
  minMaxInput["use_actual_data"] = 1
  Query("MinMax", minMaxInput)
  xrayInput = GetQueryParameters("XRay Image")
  xrayInput["origin"]=(0.5, 2.5, 0.)
  xrayInput["image_size"]=(300,300)
  xrayInput["vars"]=("p", "d")
  Query("XRay Image", xrayInput)


GetRenderingAttributes
----------------------

**Synopsis:**

::

  GetRenderingAttributes() -> RenderingAttributes object

return type : RenderingAttributes object
    Returns a RenderingAttributes object.


**Description:**

    The GetRenderingAttributes function returns a RenderingAttributes object
    that contains the rendering settings that VisIt currently uses. The
    RenderingAttributes object contains information related to rendering such
    as whether or not specular highlights or shadows are enabled. The
    RenderingAttributes object also contains information scalable rendering
    such as whether or not it is currently in use and the scalable rendering
    threshold.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Surface", "hgslice")
  DrawPlots()
  v = GetView3D()
  v.viewNormal = (-0.215934, -0.454611, 0.864119)
  v.viewUp = (0.973938, -0.163188, 0.157523)
  v.imageZoom = 1.64765
  SetView3D(v)
  light = GetLight(0)
  light.direction = (0,1,-1)
  SetLight(0, light)
  r = GetRenderingAttributes()
  r.scalableActivationMode = r.Always
  r.doShadowing = 1
  SetRenderingAttributes(r)


GetSaveWindowAttributes
-----------------------

**Synopsis:**

::

  GetSaveWindowAttributes() -> SaveWindowAttributes object

return type : SaveWindowAttributes object
    This function returns a VisIt SaveWindowAttributes object that contains
    the attributes used in saving windows.


**Description:**

    The GetSaveWindowAttributes function returns a SaveWindowAttributes object
    that is a structure containing several fields which determine how windows
    are saved to files. The object that us returned can be modified and used to
    set the save window attributes.


**Example:**

::

  #% visit -cli
  s = GetSaveWindowAttributes()
  print(s)
  s.width = 600
  s.height = 600
  s.format = s.RGB
  print(s)


GetSelection
------------

**Synopsis:**

::

  GetSelection(name) -> SelectionProperties object


name : string
    The name of the selection whose properties we want to retrieve.

return type : SelectionProperties object
    The GetSelection function returns a SelectionProperties object.


**Description:**

    Named selections have properties that describe how the selection is defined.
    This function lets you query those selection properties.


**Example:**

::

  CreateNamedSelection('selection1')
  s = GetSelection('selection1')
  s.selectionType = s.CumulativeQuerySelection
  s.histogramType = s.HistogramMatches
  s.combineRule = s.CombineOr
  s.variables = ('temperature',)
  s.variableMins = (2.9,)
  s.variableMaxs = (3.1,)
  UpdateNamedSelection('selection1', s)


GetSelectionList
----------------

**Synopsis:**

::

  GetSelectionList() -> SelectionList object


return type : SelectionList object
    The GetSelectionList function returns a SelectionList object.


**Description:**

    VisIt maintains a list of named selections, which are sets of cells that are
    used to restrict the cells processed by other plots. This function returns a
    list of the selections that VisIt knows about, including their properties.


**Example:**

::

  s = GetSelectionList()


GetSelectionSummary
-------------------

**Synopsis:**

::

  GetSelectionSummary(name) -> SelectionSummary object


name : string
    The name of the selection whose summary we want to retrieve.

return type : SelectionSummary object
    The GetSelectionSummary function returns a SelectionSummary object.


**Description:**

    Named selections have both properties, which describe how the selection is
    defined, and a summary that desribes the data that was processed while creating
    the selection. The selection summary object contains some statistics about
    the selection such as how many cells it contains and histograms of the various
    variables that were used in creating the selection.


**Example:**

::

  print(GetSelectionSummary('selection1'))


GetTimeSliders
--------------

**Synopsis:**

::

  GetTimeSliders() -> tuple of strings

return type : tuple of strings
    GetTimeSliders returns a tuple of strings.


**Description:**

    The GetTimeSliders function returns a tuple of strings containing the
    names of each of the available time sliders. The list of time sliders
    contains the names of any open time-varying database, all database
    correlations, and the keyframing time slider if VisIt is in keyframing mode.


**Example:**

::

  #% visit -cli
  path = "/usr/gapps/visit/data/"
  dbs = (path + "/dbA00.pdb", path + "dbB00.pdb", path + "dbC00.pdb")
  for db in dbs:
      OpenDatabase(db)
      AddPlot("FilledBoundary", "material(mesh)")
      DrawPlots()
  CreateDatabaseCorrelation("common", dbs, 1)
  print("The list of time sliders is: ", GetTimeSliders())


GetUltraScript
--------------

**Synopsis:**

::

  GetUltraScript() -> string


return type : string
    The GetUltraScript function returns a filename.


**Description:**

    Return the name of the file in use by the LoadUltra function. Normal users do
    not need to use this function.


GetView2D
---------

**Synopsis:**

::

  GetView2D() -> View2DAttributes object

return type : View2DAttributes object
    Object that represents the 2D view information.


**Description:**

    The GetView functions return ViewAttributes objects which describe the
    current camera location. The GetView2D function should be called if the
    active visualization window contains 2D plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Change the view interactively using the mouse.
  v0 = GetView3D()
  # Change the view again using the mouse
  v1 = GetView3D()
  print(v0)
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v1 + t * v0
      SetView3D(v2) # Animate the view back to the first view.


GetView3D
---------

**Synopsis:**

::

  GetView3D() -> View3DAttributes object

return type : View3DAttributes object
    Object that represents the 3D view information.


**Description:**

    The GetView functions return ViewAttributes objects which describe the
    current camera location. The GetView3D function should be called to get
    the view if the active visualization window contains 3D plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Change the view interactively using the mouse.
  v0 = GetView3D()
  # Change the view again using the mouse
  v1 = GetView3D()
  print(v0)
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v1 + t * v0
      SetView3D(v2) # Animate the view back to the first view.


GetViewAxisArray
----------------

**Synopsis:**

::

  GetViewAxisArray() -> ViewAxisArrayAttributes object

return type : ViewAxisArrayAttributes object
    Object that represents the AxisArray view information.


**Description:**

    The GetView functions return ViewAttributes objects which describe the
    current camera location. The GetViewAxisArray function should be called
    if the active visualization window contains axis-array plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Change the view interactively using the mouse.
  v0 = GetView3D()
  # Change the view again using the mouse
  v1 = GetView3D()
  print(v0)
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v1 + t * v0
      SetView3D(v2) # Animate the view back to the first view.


GetViewCurve
------------

**Synopsis:**

::

  GetViewCurve() -> ViewCurveAttributes object

return type : ViewCurveAttributes object
    Object that represents the curve view information.


**Description:**

    The GetView functions return ViewAttributes objects which describe the
    current camera location. The GetViewCurve function should be called if
    the active visualization window contains 1D curve plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Change the view interactively using the mouse.
  v0 = GetView3D()
  # Change the view again using the mouse
  v1 = GetView3D()
  print(v0)
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v1 + t * v0
      SetView3D(v2) # Animate the view back to the first view.


GetWindowInformation
--------------------

**Synopsis:**

::

  GetWindowInformation() -> WindowInformation object

return type : WindowInformation object
    The GetWindowInformation object returns a WindowInformation object.


**Description:**

    The GetWindowInformation object returns a WindowInformation object that
    contains information about the active visualization window. The
    WindowInformation object contains the name of the active source, the active
    time slider index, the list of available time sliders and their current
    states, as well as certain window flags that determine whether a window's
    view is locked, etc. Use the WindowInformation object if you need to query
    any of these types of information in your script to influence how it
    behaves.


**Example:**

::

  path = "/usr/gapps/visit/data/"
  dbs = (path + "dbA00.pdb", path + "dbB00.pdb", path + "dbC00.pdb")
  for db in dbs:
      OpenDatabase(db)
      AddPlot("FilledBoundary", "material(mesh)")
      DrawPlots()
  CreateDatabaseCorrelation("common", dbs, 1)
  # Get the list of available time sliders.
  tsList = GetWindowInformation().timeSliders
  # Iterate through "time" on each time slider.
  for ts in tsList:
      SetActiveTimeSlider(ts)
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
  # Print the window information to examine the other attributes
  # that are available.
  GetWindowInformation()


HideActivePlots
---------------

**Synopsis:**

::

  HideActivePlots() -> integer

return type : CLI_return_t
    The HideActivePlots function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    The HideActivePlots function tells the viewer to hide the active plots in
    the active visualization window.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddPlot("Mesh", "mesh1")
  DrawPlots()
  SetActivePlots(0)
  HideActivePlots()
  AddPlot("FilledBoundary", "mat1")
  DrawPlots()


HideToolbars
------------

**Synopsis:**

::

  HideToolbars() -> integer
  HideToolbars(allWindows) -> integer


allWindows : integer
    An optional integer value that tells VisIt to hide the toolbars for
    all windows when it is non-zero.

return type : CLI_return_t
    The HideToolbars function returns 1 on success and 0 on failure.


**Description:**

    The HideToolbars function tells VisIt to hide the toolbars for the active
    visualization window or for all visualization windows when the optional
    allWindows argument is provided and is set to a non-zero value.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4)
  HideToolbars()
  ShowToolbars()
  # Hide the toolbars for all windows.
  HideToolbars(1)


IconifyAllWindows
-----------------

**Synopsis:**

::

  IconifyAllWindows()


**Description:**

    The IconifyAllWindows function minimizes all of the hidden visualization
    windows to get them out of the way.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4) # Have 4 windows
  IconifyAllWindows()
  DeIconifyAllWindows()


InitializeNamedSelectionVariables
---------------------------------

**Synopsis:**

::

  InitializeNamedSelectionVariables(name) -> integer


name : string
    The name of the named selection to initialize.

return type : CLI_return_t
    The InitializeNamedSelectionVariables function returns 1 on success and 0 on failure.


**Description:**

    Complex thresholds are often defined using the Parallel Coordinates plot
    or the Threshold operator. This function can copy variable ranges from
    compatible plots and operators into the specified named selection's
    properties. This can be useful when setting up Cumulative
    Query selections.


**Example:**

::

  InitializeNamedSelectionVariables('selection1')


InvertBackgroundColor
---------------------

**Synopsis:**

::

  InvertBackgroundColor()


**Description:**

    The InvertBackgroundColor function swaps the background and foreground
    colors in the active visualization window. This function is a cheap
    alternative to setting the foreground and background colors though the
    AnnotationAttributes in that it is a simple no-argument function call. It
    is not adequate to set new colors for the background and foreground, but in
    the event where the two colors can be exchanged favorably, it is a good
    function to use. An example of when this function is used is after the
    creation of a Volume plot.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Volume", "u")
  DrawPlots()
  InvertBackgroundColor()


Launch
------

**Synopsis:**

::

  Launch() -> integer
  Launch(program) -> integer


program : string
    The complete path as a string to the top level 'visit' script.

return type : CLI_return_t
    The Launch functions return 1 for success and 0 for failure


**Description:**

    The Launch function is used to launch VisIt's viewer when the VisIt module
    is imported into a stand-alone Python interpreter. The Launch function has
    no effect when a viewer already exists. The difference between Launch and
    LaunchNowin is that LaunchNowin prevents the viewer from ever creating
    onscreen visualization windows. The LaunchNowin function is primarily used
    in Python scripts that want to generate visualizations using VisIt without
    the use of a display such as when generating movies.


**Example:**

::

  import visit
  import visit
  visit.AddArgument("-nowin")
  visit.Launch()


LaunchNowin
-----------

**Synopsis:**

::

  LaunchNowin() -> integer
  LaunchNowin(program) -> integer


program : string
    The complete path as a string to the top level 'visit' script.

return type : CLI_return_t
    The LaunchNowin functions return 1 for success and 0 for failure


**Description:**

    The Launch function is used to launch VisIt's viewer when the VisIt module
    is imported into a stand-alone Python interpreter. The Launch function has
    no effect when a viewer already exists. The difference between Launch and
    LaunchNowin is that LaunchNowin prevents the viewer from ever creating
    onscreen visualization windows. The LaunchNowin function is primarily used
    in Python scripts that want to generate visualizations using VisIt without
    the use of a display such as when generating movies.


**Example:**

::

  import visit
  visit.AddArgument("-geometry")
  visit.AddArgument("1024x1024")
  visit.LaunchNowin()


Lineout
-------

**Synopsis:**

::

  Lineout(start, end) -> integer
  Lineout(start, end, variables) -> integer
  Lineout(start, end, samples) -> integer
  Lineout(start, end, variables, samples) -> integer
  Lineout(keywordarg1=arg1, keywrdarg2=arg2,...,keywordargn=argn ) -> integer


start : tuple of doubles
    A 2 or 3 item tuple containing the coordinates of the starting point.
    keyword arg - start_point

end : tuple of doubles
    A 2 or 3 item tuple containing the coordinates of the end point.
    keyword arg - end_point

variables : tuple of strings
    A tuple of strings containing the names of the variables for which
    lineouts should be created.
    keyword arg - vars

samples : integer
    An integer value containing the number of sample points along the lineout.
    keyword arg - num_samples
    keyword arg - use_sampling

return type : CLI_return_t
    The Lineout function returns 1 on success and 0 on failure.


**Description:**

    The Lineout function extracts data along a given line segment and creates
    curves from it in a new visualization window. The start argument is a tuple
    of numbers that make up the coordinate of the lineout's starting location.
    The end argument is a tuple of numbers that make up the coordinate of the
    lineout's ending location. The optional variables argument is a tuple of
    strings that contain the variables that should be sampled to create
    lineouts. The optional samples argument is used to determine the number of
    sample points that should be taken along the specified line. If the samples
    argument is not provided then VisIt will sample the mesh where it
    intersects the specified line instead of using the number of samples to
    compute a list of points to sample.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "ascii")
  DrawPlots()
  Lineout((0.2,0.2), (0.8,1.2))
  Lineout((0.2,1.2), (0.8,0.2), ("default", "d", "u"))
  Lineout((0.6, 0.1), (0.6, 1.2), 100)
  Lineout(start_point=(0.6, 0.1), end_point=(0.6, 1.2), use_sampling=1, num_samples=100)


ListDomains
-----------

**Synopsis:**

::

  ListDomains()


**Description:**

    ListDomains  prints a list of the domains for the active plots, which
    indicates which domains are on and off. The list functions are used
    mostly to print the results of restricting the SIL.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  TurnMaterialsOff("4") # Turn off material 4
  ListMaterials() # List the materials in the SIL restriction


ListMaterials
-------------

**Synopsis:**

::

  ListMaterials()


**Description:**

    ListMaterials prints a list of the materials for the active plots, which
    indicates which materials are on and off. The list functions are used
    mostly to print the results of restricting the SIL.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  TurnMaterialsOff("4") # Turn off material 4
  ListMaterials() # List the materials in the SIL restriction


ListPlots
---------

**Synopsis:**

::

  ListPlots() -> string
  ListPlots(stringOnly) -> string

return type : string
    The ListPlots function returns a string containing a representation of the.
    plot list.


**Description:**

    Sometimes it is difficult to remember the order of the plots in the active
    visualization window's plot list. The ListPlots function prints the
    contents of the plot list to the output console and returns that string as well.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "u")
  AddPlot("Contour", "d")
  DrawPlots()
  ListPlots()


LoadAttribute
-------------

**Synopsis:**

::

  LoadAttribute(filename, object)


filename : string
    The name of the XML file to load the attribute from or save the attribute to.

object
    The object to load or save.


**Description:**

    The LoadAttribute and SaveAttribute methods save a single
    attribute, such as a current plot or operator python object,
    to a standalone XML file.  Note that LoadAttribute requires
    that the target attribute already be created by other means;
    it fills, but does not create, the attribute.


**Example:**

::

  #% visit -cli
  a = MeshPlotAttributes()
  SaveAttribute('mesh.xml', a)
  b = MeshPlotAttributes()
  LoadAttribute('mesh.xml', b)


LoadNamedSelection
------------------

**Synopsis:**

::

  LoadNamedSelection(name) -> integer
  LoadNamedSelection(name, engineName) -> integer
  LoadNamedSelection(name, engineName, simName) -> integer


name : string
    The name of a named selection.

engineName : string
    (optional) The name of the engine where the selection was saved.

simName : string
    (optional) The name of the simulation that saved the selection.

return type : CLI_return_t
    The LoadNamedSelection function returns 1 for success and 0 for failure.


**Description:**

    Named Selections allow you to select a group of elements (or particles).
    One typically creates a named selection from a group of elements and then
    later applies the named selection to another plot (thus reducing the
    set of elements displayed to the ones from when the named selection was
    created).  Named selections only last for the current session.  However,
    if you find a named selection that is particularly interesting, you can
    save it to a file for use in later sessions.  You would use
    LoadNamedSelection to do the loading.


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/wave*.silo database"
  OpenDatabase(db)
  AddPlot("Pseudocolor", "pressure")
  LoadNamedSelection("selection_from_previous_session")
  ApplyNamedSelection("selection_from_previous_session")


LoadUltra
---------

**Synopsis:**

::

  LoadUltra()


**Description:**

    LoadUltra launches the Ultra command parser, allowing you to enter Ultra
    commands and have VisIt process them.  A new command prompt is presented,
    and only Ultra commands will be allowed until 'end' or 'quit' is entered,
    at which time, you will be returned to VisIt's cli prompt.  For information
    on currently supported commands, type 'help' at the Ultra prompt
    Please note that filenames/paths must be surrounded by quotes, unlike with
    Ultra.


**Example:**

::

  #% visit -cli
  #>>> LoadUltra()
  #U-> rd "../../data/distribution.ultra"
  #U-> select 1
  #U-> end
  #>>>


LocalNameSpace
--------------

**Synopsis:**

::

  LocalNameSpace()



**Description:**

    The LocalNameSpace function tells the VisIt module to add plugin functions
    to the global namespace when the VisIt module is imported into a
    stand-alone Python interpreter. This is the default behavior when using
    VisIt's cli program.


**Example:**

::

  import visit
  visit.LocalNameSpace()
  visit.Launch()


LongFileName
------------

**Synopsis:**

::

  LongFileName(filename) -> string


filename : string
    A string object containing the short filename to expand.

return type : string
    The LongFileName function returns a string. This function returns the
    input argument unless you are on the Windows platform.


**Description:**

    On Windows, filenames can have two different sizes: traditional 8.3
    format, and long format. The long format, which lets you name files
    whatever you want, is implemented using the traditional 8.3 format under
    the covers. Sometimes filenames are given to VisIt in the traditional 8.3
    format and must be expanded to long format before it is possible to open
    them. If you ever find that you need to do this conversion, such as when
    you process command line arguments, then you can use the LongFileName
    function to return the longer filename.


MoveAndResizeWindow
-------------------

**Synopsis:**

::

  MoveAndResizeWindow(win, x, y, w, h) -> integer


win : integer
    The integer id of the window to be moved [1..16].

x : integer
    The new integer x location for the window being moved.

y : integer
    The new integer y location for the window being moved.

w : integer
    The new integer width for the window being moved.

h : integer
    The new integer height for the window being moved.

return type : CLI_return_t
    MoveAndResizeWindow returns 1 on success and 0 on failure.


**Description:**

    MoveAndResizeWindow moves and resizes a visualization window.


**Example:**

::

  #% visit -cli
  MoveAndResizeWindow(1, 100, 100, 300, 600)


MovePlotDatabaseKeyframe
------------------------

**Synopsis:**

::

  MovePlotDatabaseKeyframe(index, oldFrame, newFrame)


index : integer
    An integer representing the index of the plot in the plot list.

oldFrame : integer
    An integer that is thhe old animation frame where the keyframe is located.

newFrame : integer
    An integer that is the new animation frame where the keyframe will be moved.


**Description:**

    MovePlotDatabaseKeyframe moves a database keyframe for a specified plot to
    a new animation frame, which changes the list of database time states that
    are used for each animation frame when VisIt is in keyframing mode.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  k = GetKeyframeAttributes()
  nFrames = 20
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1, nFrames, 1
  AddPlot("Pseudocolor", "pressure")
  SetPlotFrameRange(0, 0, nFrames-1)
  SetPlotDatabaseKeyframe(0, 0, 70)
  SetPlotDatabaseKeyframe(0, nFrames/2, 35)
  SetPlotDatabaseKeyframe(0, nFrames-1, 0)
  DrawPlots()
  for state in list(range(TimeSliderGetNStates())) + [0]:
      SetTimeSliderState(state)
  MovePlotDatabaseKeyframe(0, nFrames/2, nFrames/4)
  for state in list(range(TimeSliderGetNStates())) + [0]:
      SetTimeSliderState(state)


MovePlotKeyframe
----------------

**Synopsis:**

::

  MovePlotKeyframe(index, oldFrame, newFrame)


index : integer
    An integer representing the index of the plof in the plot list.

oldFrame : integer
    An integer that is the old animation frame where the keyframe is located.

newFrame : integer
    An integer that is the new animation frame where the keyframe will be moved.


**Description:**

    MovePlotKeyframe moves a keyframe for a specified plot to a new animation
    frame, which changes the plot attributes that are used for each animation
    frame when VisIt is in keyframing mode.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Contour", "hgslice")
  DrawPlots()
  k = GetKeyframeAttributes()
  nFrames = 20
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1, nFrames, 1
  SetKeyframeAttributes(k)
  SetPlotFrameRange(0, 0, nFrames-1)
  c = ContourAttributes()
  c.contourNLevels = 5
  SetPlotOptions(c)
  SetTimeSliderState(nFrames/2)
  c.contourNLevels = 10
  SetPlotOptions(c)
  c.contourLevels = 25
  SetTimeSliderState(nFrames-1)
  SetPlotOptions(c)
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()
  temp = nFrames-2
  MovePlotKeyframe(0, nFrames/2, temp)
  MovePlotKeyframe(0, nFrames-1, nFrames/2)
  MovePlotKeyframe(0, temp, nFrames-1)
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()


MovePlotOrderTowardFirst
------------------------

**Synopsis:**

::

  MovePlotOrderTowardFirst(index) -> integer


index : integer
    The integer index of the plot that will be moved within the plot list.

return type : CLI_return_t
    The MovePlotOrderTowardFirst function returns 1 on success and 0 on failure.


**Description:**

    This function shifts the specified plot one slot towards the start of the plot list.


**Example:**

::

  MovePlotOrderTowardFirst(2)


MovePlotOrderTowardLast
-----------------------

**Synopsis:**

::

  MovePlotOrderTowardLast(index) -> integer


index : integer
    The integer index of the plot that will be moved within the plot list.

return type : CLI_return_t
    The MovePlotOrderTowardLast function returns 1 on success and 0 on failure.


**Description:**

    This function shifts the specified plot one slot towards the end of the plot list.


**Example:**

::

  MovePlotOrderTowardLast(0)


MoveViewKeyframe
----------------

**Synopsis:**

::

  MoveViewKeyframe(oldFrame, newFrame) -> integer


oldFrame : integer
    An integer that is the old animation frame where the keyframe is located.

newFrame : integer
    An integer that is the new animation frame where the keyframe will be moved.

return type : CLI_return_t
    MoveViewKeyframe returns 1 on success and 0 on failure.


**Description:**

    MoveViewKeyframe moves a view keyframe to a new animation frame, which
    changes the view that is used for each animation frame when VisIt is in
    keyframing mode.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Contour", "hardyglobal")
  DrawPlots()
  k = GetKeyframeAttributes()
  nFrames = 20
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1, nFrames, 1
  SetKeyframeAttributes(k)
  SetViewKeyframe()
  SetTimeSliderState(nFrames/2)
  v = GetView3d()
  v.viewNormal = (-0.616518, 0.676972, 0.402014)
  v.viewUp = (0.49808, 0.730785, -0.466764)
  SetViewKeyframe()
  SetTimeSliderState(0)
  # Move the view keyframe to the last animation frame.
  MoveViewKeyframe(nFrames/2, nFrames-1)


MoveWindow
----------

**Synopsis:**

::

  MoveWindow(win, x, y) -> integer


win : integer
    The integer id of the window to be moved [1..16].

x : integer
    The new integer x location for the window being moved.

y : integer
    The new integer y location for the window being moved.

return type : CLI_return_t
    MoveWindow returns 1 on success and 0 on failure.


**Description:**

    MoveWindow moves a visualization window.


**Example:**

::

  #% visit -cli
  MoveWindow(1, 100, 100)


NodePick
--------

**Synopsis:**

::

  NodePick(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


coord : tuple
    A tuple of doubles containing the spatial coordinate (x, y, z).

x : integer
    An integer containing the screen X location (in pixels) offset
    from the left side of the visualization window.

y : integer
    An integer containing the screen Y location (in pixels) offset
    from the bottom of the visualization window.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes.

return type : dictionary
    NodePick returns a python dictionary of the pick results,
    unless do_time is specified, then a time curve is created
    in a new window.


**Description:**

    The NodePick function prints pick information for the node closest to the
    specified point. The point can be specified as a 2D or 3D point in world
    space or it can be specified as a pixel location in screen space. If the
    point is specified as a pixel location then VisIt finds the node closest to
    a ray that is projected into the mesh. Once the nodal pick has been
    calculated, you can use the GetPickOutput function to retrieve the printed
    pick output as a string which can be used for other purposes.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hgslice")
  DrawPlots()
  # Perform node pick in screen space
  pick_out = NodePick(x=200,y=200)
  # Perform node pick in world space.
  pick_out = NodePick(coord=(-5.0, 5.0, 0))


NumColorTableNames
------------------

**Synopsis:**

::

  NumColorTableNames() -> integer

return type : CLI_return_t
    The NumColorTableNames function return an integer.


**Description:**

    The NumColorTableNames function returns the number of color tables that
    have been defined.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  p = PseudocolorAttributes()
  p.colorTableName = "default"
  SetPlotOptions(p)
  DrawPlots()
  print("There are %d color tables." % NumColorTableNames())
  for ct in ColorTableNames():
      SetActiveContinuousColorTable(ct)
      SaveWindow()


NumOperatorPlugins
------------------

**Synopsis:**

::

  NumOperatorPlugins() -> integer

return type : CLI_return_t
    The NumOperatorPlugins function returns an integer.


**Description:**

    The NumOperatorPlugins function returns the number of available operator
    plugins.


**Example:**

::

  #% visit -cli
  print("The number of operator plugins is: ", NumOperatorPlugins())
  print("The names of the plugins are: ", OperatorPlugins())


NumPlotPlugins
--------------

**Synopsis:**

::

  NumPlotPlugins() -> integer

return type : CLI_return_t
    The NumPlotPlugins function returns an integer.


**Description:**

    The NumPlotPlugins function returns the number of available plot plugins.


**Example:**

::

  #% visit -cli
  print("The number of plot plugins is: ", NumPlotPlugins())
  print("The names of the plugins are: ", PlotPlugins())


OpenComputeEngine
-----------------

**Synopsis:**

::

  OpenComputeEngine() -> integer
  OpenComputeEngine(hostName) -> integer
  OpenComputeEngine(hostName, simulation) -> integer
  OpenComputeEngine(hostName, args) -> integer
  OpenComputeEngine(MachineProfile) -> integer


hostName : string
    The name of the computer on which to start the engine.

args : tuple
    Optional tuple of command line arguments for the engine.
    Alternative arguments - MachineProfile object to load with
    OpenComputeEngine call

return type : CLI_return_t
    The OpenComputeEngine function returns an integer value of 1 for success
    and 0 for failure.


**Description:**

    The OpenComputeEngine function is used to explicitly open a compute engine
    with certain properties. When a compute engine is opened implicitly, the
    viewer relies on sets of attributes called host profiles. Host profiles
    determine how compute engines are launched. This allows compute engines to
    be easily launched in parallel. Since the VisIt Python Interface does not
    expose VisIt's host profiles, it provides the OpenComputeEngine function to
    allow users to launch compute engines. The OpenComputeEngine function must
    be called before opening a database in order to prevent any latent host
    profiles from taking precedence.


**Example:**

::

  #% visit -cli
  # Launch parallel compute engine remotely.
  args = ("-np", "16", "-nn", "4")
  OpenComputeEngine("thunder", args)
  OpenDatabase("thunder:/usr/gapps/visit/data/multi_ucd3d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()


OpenDatabase
------------

**Synopsis:**

::

  OpenDatabase(databaseName) -> integer
  OpenDatabase(databaseName, timeIndex) -> integer
  OpenDatabase(databaseName, timeIndex, dbPluginName) -> integer


databaseName : string
    The name of the database to open.

timeIndex : integer
    This is an optional integer argument indicating the time index at which
    to open the database. If it is not specified, a time index of zero is
    assumed.

dbPluginIndex : string
    An optional string containing the name of the plugin to use. Note that
    this string must also include the plugin's version number (with few
    exceptions, almost all plugins' version numbers are 1.0). Note also
    that you must capitalize the spelling identically to what the plugin's
    GetName() method returns. For example, "XYZ_1.0" is the string you would
    use for the XYZ plugin.

return type : CLI_return_t
    The OpenDatabase function returns an integer value of 1 for success and 0
    for failure.


**Description:**

    The OpenDatabase function is one of the most important functions in the
    VisIt Python Interface because it opens a database so it can be plotted.
    The databaseName argument is a string containing the full name of the
    database to be opened. The database name is of the form:
    computer:/path/filename. The computer part of the filename can be omitted
    if the database to be opened resides on the local computer.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  OpenDatabase("mcr:/usr/gapps/visit/data/multi_ucd3d.silo")
  OpenDatabase("file.visit")
  OpenDatabase("file.visit", 4)
  OpenDatabase("mcr:/usr/gapps/visit/data/multi_ucd3d.silo",0,"Silo_1.0")


OpenMDServer
------------

**Synopsis:**

::

  OpenMDServer() -> integer
  OpenMDServer(host) -> integer
  OpenMDServer(host, args) -> integer
  OpenMDServer(MachineProfile) -> integer


host : string
    The optional host argument determines the host on which the metadata
    server is to be launched. If this argument is not provided, "localhost"
    is assumed.

args : tuple
    A tuple of strings containing command line flags for the metadata server.
    
    =============  ========================================================
    **Argument**   Description
    =============  ========================================================
    -debug #       The -debug argument allows you to specify a debug level.
    -dir visitdir  The -dir argument allows you to specify where VisIt is.
    =============  ========================================================

MachineProfile : MachineProfile object
    MachineProfile object to load with OpenMDServer call

return type : CLI_return_t
    The OpenMDServer function returns 1 on success and 0 on failure.


**Description:**

    The OpenMDServer explicitly launches a metadata server on a specified
    host. This allows you to provide command line options that influence how
    the metadata server will run.
    range [1,5] that VisIt uses to write debug logs to disk.
    located on a remote computer. This allows you to successfully
    connect to a remote computer in the absence of host profiles.
    It also allows you to debug VisIt in distributed mode.
    -fallback_format <format>
    The -fallback_format argument allows you to specify the
    database plugin that will be used to open files if all
    other guessing failed. This is useful when the files
    that you want to open do not have file extensions.
    -assume_format <format>
    The -assume_format argument allows you to specify the
    database plugin that will be used FIRST when attempting
    to open files. This is useful when the files that you
    want to open have a file extension which may match
    multiple file format readers.


**Example:**

::

  
  #% visit -cli -assume_format PDB
  #args = ("-dir", "/my/private/visit/version/", "-assume_format", "PDB", "-debug", "4")
  # Open a metadata server before the call to OpenDatabase so we
  # can launch it how we want.
  OpenMDServer("thunder", args)
  OpenDatabase("thunder:/usr/gapps/visit/data/allinone00.pdb")
  # Open a metadata server on localhost too.
  OpenMDServer()


OperatorPlugins
---------------

**Synopsis:**

::

  OperatorPlugins() -> tuple of strings

return type : tuple of strings
    The OperatorPlugins function returns a tuple of strings.


**Description:**

    The OperatorPlugins function returns a tuple of strings that contain the
    names of the loaded operator plugins. This can be useful for the creation
    of scripts that alter their behavior based on the available operator
    plugins.


**Example:**

::

  #% visit -cli
  for plugin in OperatorPlugins():
      print("The %s operator plugin is loaded." % plugin)


OverlayDatabase
---------------

**Synopsis:**

::

  OverlayDatabase(databaseName) -> integer
  OverlayDatabase(databaseName, state) -> integer


databaseName : string
    The name of the new plot database.

state
    The time state at which to open the database.

return type : CLI_return_t
    The OverlayDatabase function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    VisIt has the concept of overlaying plots which, in the nutshell, means
    that the entire plot list is copied and a new set of plots with exactly the
    same attributes but a different database is appended to the plot list of
    the active window. The OverlayDatabase function allows the VisIt Python
    Interface to overlay plots. OverlayDatabase takes a single string argument
    which contains the name of the database. After calling the OverlayDatabase
    function, the plot list is larger and contains plots of the specified
    overlay database.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  OverlayDatabase("riptide:/usr/gapps/visit/data/curv3d.silo")


PickByGlobalNode
----------------

**Synopsis:**

::

  PickByGlobalNode(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


element : integer
    An integer with the global node id.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes.

return type : dictionary
    PickByGlobalNode returns a python dictionary of pick results.


**Description:**

    The PickByGlobalNode function tells VisIt to perform pick using a specific
    global node index for the entire problem. Some meshes are broken up into
    smaller "domains" and then these smaller domains can employ a global
    indexing scheme to make it appear as though the mesh was still one large
    mesh. Not all meshes that have been decomposed into domains provide
    sufficient information to allow global node indexing. You can use the
    GetPickOutput function to retrieve a string containing the pick information
    once you've called PickByGlobalNode.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/global_node.silo")
  AddPlot("Pseudocolor", "dist")
  DrawPlots()
  # Pick on global node 236827
  pick_out = PickByGlobalNode(element=246827)
  # examine output
  print('value of dist at global node 246827: %g' % pick_out['dist'])
  print('local domain/node: %d/%d' % (pick_out['domain_id'], pick_out['node_id']))
  # get last pick output as string
  print('Last pick = ', GetPickOutput())


PickByGlobalZone
----------------

**Synopsis:**

::

  PickByGlobalZone(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


element : integer
    An integer with the global zone id.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes.

return type : dictionary
    PickByGlobalZone returns a python dictionary of pick results.


**Description:**

    The PickByGlobalZone function tells VisIt to perform pick using a specific
    global cell index for the entire problem. Some meshes are broken up into
    smaller "domains" and then these smaller domains can employ a global
    indexing scheme to make it appear as though the mesh was still one large
    mesh. Not all meshes that have been decomposed into domains provide
    sufficient information to allow global cell indexing. You can use the
    GetPickOutput function to retrieve a string containing the pick information
    once you've called PickByGlobalZone.


**Example:**

::

  OpenDatabase("/usr/gapps/visit/data/global_node.silo")
  AddPlot("Pseudocolor", "p")
  DrawPlots()
  # Pick on global zone 237394
  pick_out = PickByGlobalZone(element=237394)
  # examine output
  print('value of p at global zone 237394: %g' % pick_out['p'])
  print('local domain/zone: %d/%d' % (pick_out['domain_id'], pick_out['zone_id']))
  # get last pick output as string
  print('Last pick = ', GetPickOutput())


PickByNode
----------

**Synopsis:**

::

  PickByNode(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


domain : integer
    An integer with the domain id.

element : integer
    An integer with the node id.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes. Currently, this is only available when performing
    a pick range.

return type : dictionary
    PickByNode returns a python dictionary of the pick results,
    unless do_time is specified, then a time curve is created in a
    new window. If the picked variable is zone centered, the variable
    values are grouped according to incident zone ids.


**Description:**

    The PickByNode function tells VisIt to perform pick using a specific node
    index in a given domain. Other pick by node variants first determine the
    node that is closest to some user-specified 3D point but the PickByNode
    functions cuts out this step and allows you to directly pick on the node of
    your choice. You can use the GetPickOutput function to retrieve a string
    containing the pick information once you've called PickByNode.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/multi_curv2d.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Pick on node 200 in the first domain.
  pick_out = PickByNode(element=200, domain=1)
  # examine output
  print('value of u at node 200: %g' % pick_out['u'])
  # Pick on node 100 in domain 5 and return information for two additional
  # variables.
  pick_out = PickByNode(domain=5, element=100, vars=("u", "v", "d"))
  # examine output
  print('incident zones for node 100: ', pick_out['incident_zones'])
  print('value of d at incident zone %d: %g' % (pick_out['incident_zones'][0], pick_out['d'][str(pick_out['incident_zones'][0])]))
  # print results formatted as string
  print("Last pick = ", GetPickOutput())


PickByNodeLabel
---------------

**Synopsis:**

::

  PickByNodeLabel(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


element_label : string
    An string with the label of the node to pick.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes.

return type : dictionary
    PickByNodeLabel returns a python dictionary of the pick results,
    unless do_time is specified, then a time curve is created in a
    new window. If the picked variable is node centered, the variable
    values are grouped according to incident node ids.


**Description:**

    The PickByNodeLabel function tells VisIt to perform pick using a specific cell
    label. Other pick by zone variants first determine the
    cell that contains some user-specified 3D point but the PickByZone
    functions cuts out this step and allows you to directly pick on the cell of
    your choice. You can use the GetPickOutput function to retrieve a string
    containing the pick information once you've called PickByZone.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/multi_curv2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Pick on node labeled "node 4".
  pick_out = PickByNodeLabel(element_label="node 4")
  # Pick on cell labeled "node 4" using a python dictionary.
  opts = {}
  opts["element_label"] ="node 4"
  pick_out = PickByNodeLabel(opts)
  # examine output
  print('value of d at "node 4": %g' % pick_out['d'])
  # Pick on node labeled "node 12" return information for two additional
  # variables.
  pick_out = PickByNodeLabel(element_label="node 12", vars=("d", "u", "v"))
  # examine output
  print('incident nodes for "node 12": ', pick_out['incident_nodes'])
  print('values of u at incident node %d: %g' % (pick_out['incident_nodes'][0], pick_out['u'][str(pick_out['incident_zones'][0])]))
  # print results formatted as string
  print("Last pick = ", GetPickOutput())


PickByZone
----------

**Synopsis:**

::

  PickByZone(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


domain : integer
    An integer with the domain id.

element : integer
    An integer with the zone id.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes. Currently, this is only available when performing
    a pick range.

return type : dictionary
    PickByZone returns a python dictionary of the pick results,
    unless do_time is specified, then a time curve is created in a
    new window. If the picked variable is node centered, the variable
    values are grouped according to incident node ids.


**Description:**

    The PickByZone function tells VisIt to perform pick using a specific cell
    index in a given domain. Other pick by zone variants first determine the
    cell that contains some user-specified 3D point but the PickByZone
    functions cuts out this step and allows you to directly pick on the cell of
    your choice. You can use the GetPickOutput function to retrieve a string
    containing the pick information once you've called PickByZone.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/multi_curv2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Pick on cell 200 in the second domain.
  pick_out = PickByZone(element=200, domain=2)
  # examine output
  print('value of d at zone 200: %g' % pick_out['d'])
  # Pick on cell 100 in domain 5 and return information for two additional
  # variables.
  pick_out = PickByZone(element=100, domain=5, vars=("d", "u", "v"))
  # examine output
  print('incident nodes for zone 100: ', pick_out['incident_nodes'])
  print('values of u at incident zone %d: %g' % (pick_out['incident_nodes'][0], pick_out['u'][str(pick_out['incident_zones'][0])]))
  # print results formatted as string
  print("Last pick = ", GetPickOutput())


PickByZoneLabel
---------------

**Synopsis:**

::

  PickByZoneLabel(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


element_label : string
    An string with the label of the zone to pick.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes.

return type : dictionary
    PickByZoneLabel returns a python dictionary of the pick results,
    unless do_time is specified, then a time curve is created in a
    new window. If the picked variable is node centered, the variable
    values are grouped according to incident node ids.


**Description:**

    The PickByZoneLabel function tells VisIt to perform pick using a specific cell
    label. Other pick by zone variants first determine the
    cell that contains some user-specified 3D point but the PickByZone
    functions cuts out this step and allows you to directly pick on the cell of
    your choice. You can use the GetPickOutput function to retrieve a string
    containing the pick information once you've called PickByZone.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/multi_curv2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Pick on cell labeled "brick 4".
  pick_out = PickByZoneLabel(element_label="brick 4")
  # Pick on cell labeled "brick 4" using a python dictionary.
  opts = {}
  opts["element_label"] ="brick 4"
  pick_out = PickByZoneLabel(opts)
  # examine output
  print('value of d at "brick 4": %g' % pick_out['d'])
  # Pick on cell labeled "shell 12" return information for two additional
  # variables.
  pick_out = PickByZoneLabel(element_label="shell 12", vars=("d", "u", "v"))
  # examine output
  print('incident nodes for "shell 12": ', pick_out['incident_nodes'])
  print('values of u at incident zone %d: %g' % (pick_out['incident_nodes'][0], pick_out['u'][str(pick_out['incident_zones'][0])]))
  # print results formatted as string
  print("Last pick = ", GetPickOutput())


PlotPlugins
-----------

**Synopsis:**

::

  PlotPlugins() -> tuple of strings

return type : tuple of strings
    The PlotPlugins function returns a tuple of strings.


**Description:**

    The PlotPlugins function returns a tuple of strings that contain the names
    of the loaded plot plugins. This can be useful for the creation of scripts
    that alter their behavior based on the available plot plugins.


**Example:**

::

  #% visit -cli
  for plugin in PluginPlugins():
      print("The %s plot plugin is loaded." % plugin)


PointPick
---------

**Synopsis:**

::

  PointPick(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


coord : tuple
    A tuple of doubles containing the spatial coordinate (x, y, z).

x : integer
    An integer containing the screen X location (in pixels) offset from the
    left side of the visualization window.

y : integer
    An integer containing the screen Y location (in pixels) offset from the
    bottom of the visualization window.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes.

return type : dictionary
    PointPick returns a python dictionary of the pick results,
    unless do_time is specified, then a time curve is created in a new window.


**Description:**

    The PointPick function prints pick information for the node closest to the
    specified point. The point can be specified as a 2D or 3D point in world
    space or it can be specified as a pixel location in screen space. If the
    point is specified as a pixel location then VisIt finds the node closest to
    a ray that is projected into the mesh. Once the nodal pick has been
    calculated, you can use the GetPickOutput function to retrieve the printed
    pick output as a string which can be used for other purposes.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hgslice")
  DrawPlots()
  # Perform node pick in screen space
  pick_out = PointPick(x=200,y=200)
  # Perform node pick in world space.
  pick_out = PointPick(coord=(-5.0, 5.0, 0))


PrintWindow
-----------

**Synopsis:**

::

  PrintWindow() -> integer

return type : CLI_return_t
    The PrintWindow function returns an integer value of 1 for success and 0
    for failure.


**Description:**

    The PrintWindow function tells the viewer to print the image in the active
    visualization window using the current printer settings.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "d")
  AddPlot("Contour", "u")
  DrawPlots()
  PrintWindow()


PromoteOperator
---------------

**Synopsis:**

::

  PromoteOperator(opIndex) -> integer
  PromoteOperator(opIndex, applyToAllPlots) -> integer


opIndex : integer
    A zero-based integer corresponding to the operator that should be promoted.

applyToAllPlots : integer
    An integer flag that causes all plots in the plot list to be affected when
    it is non-zero.

return type : CLI_return_t
    PromoteOperator returns 1 on success and 0 on failure.


**Description:**

    The PromoteOperator function moves an operator closer to the end of the
    visualization pipeline. This allows you to change the order of operators
    that have been applied to a plot without having to remove them from the
    plot. For example, consider moving a Slice to after a Reflect operator when
    it had been the other way around. Changing the order of operators can
    result in vastly different results for a plot. The opposite function is
    DemoteOperator.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hardyglobal")
  AddOperator("Slice")
  s = SliceAttributes()
  s.project2d = 0
  s.originPoint = (0,5,0)
  s.originType=s.Point
  s.normal = (0,1,0)
  s.upAxis = (-1,0,0)
  SetOperatorOptions(s)
  AddOperator("Reflect")
  DrawPlots()
  # Now slice after reflect. We'll only get 1 slice plane instead of 2.
  PromoteOperator(0)
  DrawPlots()


PythonQuery
-----------

**Synopsis:**

::

  PythonQuery(source='python filter source ...') -> integer
  PythonQuery(file='path/to/python_filter_script.py') -> integer


source : string
    A string containing the source code for a Python Query Filter .

file : string
    A string containing the path to a Python Query Filter script file.
    Note - Use only one of the 'source' or 'file' arguments. If both are
    used the 'source' argument overrides 'file'.

return type : CLI_return_t
    The PythonQuery function returns 1 on success and 0 on failure.


**Description:**

    Used to execute a Python Filter Query.


Queries
-------

**Synopsis:**

::

  Queries() -> tuple of strings

return type : tuple of strings
    The Queries function returns a tuple of strings.


**Description:**

    The Queries function returns a tuple of strings that contain the names of
    all of VisIt's supported queries.


**Example:**

::

  #% visit -cli
  print("supported queries: ", Queries())


QueriesOverTime
---------------

**Synopsis:**

::

  QueriesOverTime() -> tuple of strings

return type : tuple of strings
    Returns a tuple of strings.


**Description:**

    The QueriesOverTime function returns a tuple of strings that contains the
    names of all of the VisIt queries that can be executed over time.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/allineone00.pdb")
  AddPlot("Pseudocolor", "mesh/mixvar")
  DrawPlots()
  # Execute each of the queries over time on the plots.
  for q in QueriesOverTime():
      QueryOverTime(q)
  # You can control timestates used in the query via start_time,
  # end_time, and stride as follows:
  QueryOverTime("Volume", start_time=5, end_time=250, stride=5)
  # (Defaults used if not specified are 0, nStates, 1)


Query
-----

**Synopsis:**

::

  Query(name) -> string
  Query(name, dict) -> string
  Query(name, namedarg1=arg1, namedarg2=arg2, ...) -> string
  Query(name) -> double, tuple of double
  Query(name, dict) -> double, tuple of double
  Query(name, namedarg1=arg1, namedarg2=arg2, ...) -> double, tuple of double
  Query(name) -> dictionary
  Query(name, dict) -> dictionary
  Query(name, namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


name : string
    The name of the query to execute.

dict : dictionary
    An optional dictionary containing additional query arguments.
    namedarg1, namedarg2,...
    An optional list of named arguments supplying additional query parameters.

return type : see SetQueryOutputToXXX() functions
    The Query function returns either a String (default), Value(s), or Object.
    The return type can be customized via calls to SetQueryOutputToXXX(), where
    'XXX' is 'String', 'Value', or 'Object'. For more information on these
    return types, see 'GetQueryOutput'.


**Description:**

    The Query function is used to execute any of VisIt's predefined queries.
    The list of queries can be found in theVisIt User's Manual in the
    Quantitative Analysis chapter. You can get also get a list of queries
    using 'Queries' function.
    Since queries can take a wide array of arguments, the Query function takes
    either a python dictorary or a list of named arguments specific to the
    given query.  To obtain the possible options for a given query, use the
    GetQueryParameters(name) function.  If the query accepts additional
    arguments beyond its name, this function will return a python dictionary
    containing the needed variables and their default values.  This can be
    modified and passed back to the Query method, or named arguments can be
    used instead.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  Query("Volume")
  Query("MinMax")
  Query("MinMax", use_actual_data=1)
  hohlraumArgs = GetQueryParameters("Hohlraum Flux")
  hohlraumArgs["ray_center"]=(0.5,0.5,0)
  hohlraumArgs["vars"]=("a1", "e1")
  Query("Hohlraum Flux", hohlraumArgs)


QueryOverTime
-------------

**Synopsis:**

::

  QueryOverTime(name) -> integer
  QueryOverTime(name, dict) -> integer
  QueryOverTime(name, namedarg1=val1, namedarg2=val2, ...) -> integer


name : string
    The name of the query to execute.

dict : dictionary
    An optional dictionary containing additional query arguments.
    namedarg1, namedarg2, ...
    An optional list of named arguments supplying additional query parameters.

return type : CLI_return_t
    The QueryOverTime function returns 1 on success and 0 on failure.


**Description:**

    The QueryOverTime function is used to execute any of VisIt's predefined
    queries.
    The list of queries can be found in the VisIt User's Manual in the
    Quantitative Analysis chapter. You can get also get a list of queries that
    can be executed over time using 'QueriesOverTime' function.
    Since queries can take a wide array of arguments, the Query function takes
    either a python dictionary or a list of named arguments specific to the
    given query.  To obtain the possible options for a given query, use the
    GetQueryParameters(name) function.  If the query accepts additional
    arguments beyond its name, this function will return a python dictionary
    containing the needed variables and their default values.  This can be
    modified and passed back to the QueryOverTime method, or named arguments
    can be used instead.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  for q in QueriesOverTime():
      QueryOverTime(q)



ReOpenDatabase
--------------

**Synopsis:**

::

  ReOpenDatabase(databaseName) -> integer


databaseName : string
    The name of the database to open.

return type : CLI_return_t
    The ReOpenDatabase function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    The ReOpenDatabase function reopens a database that has been opened
    previously with the OpenDatabase function. The ReOpenDatabase function is
    primarily used for regenerating plots whose database has been rewritten on
    disk. ReOpenDatabase allows VisIt to access new variables and new time
    states that have been added since the database was opened using the
    OpenDatabase function. Note that ReOpenDatabase is expensive since it
    causes all plots that use the specified database to be regenerated. If you
    want to ensure that a time-varying database has all of its time states as
    they are being created by a simulation, try the CheckForNewStates function
    instead.
    The databaseName argument is a string containing the full name of the
    database to be opened. The database name is of the form:
    host:/path/filename. The host part of the filename can be omitted if the
    database to be reopened resides on the local computer.


**Example:**

::

  #% visit -cli
  OpenDatabase("edge:/usr/gapps/visit/data/wave*.silo database")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  last = TimeSliderGetNStates()
  for state in range(last):
      SetTimeSliderState(state)
      SaveWindow()
  ReOpenDatabase("edge:/usr/gapps/visit/data/wave*.silo database")
  for state in range(last, TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()


ReadHostProfilesFromDirectory
-----------------------------

**Synopsis:**

::

  ReadHostProfilesFromDirectory(directory, clear) -> integer


directory : string
    The name of the directory that contains the host profile XML files.

clear : integer
    An integer flag indicating whether the host profile list should cleared first.

return type : CLI_return_t
    The ReadHostProfilesFromDirectory function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    The ReadHostProfilesFromDirectory provides a way to tell VisIt to load host
    profiles from the XML files in a specified directory. This is needed because
    the machine profile for host profiles contains client/server options that
    sometimes cannot be specified via the VisIt command line.


**Example:**

::

  ReadHostProfilesFromDirectory("/usr/gapps/visit/2.8.2/linux-x86_64/resources/hosts/llnl", 1)


RecenterView
------------

**Synopsis:**

::

  RecenterView() -> integer

return type : CLI_return_t
    The RecenterView function returns 1 on success and 0 on failure.


**Description:**

    After adding plots to a visualization window or applying operators to
    those plots, it is sometimes necessary to recenter the view. When the view
    is recentered, the orientation does not change but the view is shifted to
    make better use of the screen.


**Example:**

::

  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  OpenDatabase("/usr/gapps/visit/data/curv3d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  RecenterView()


RedoView
--------

**Synopsis:**

::

  RedoView() -> integer

return type : CLI_return_t
    The RedoView function returns 1 on success and 0 on failure.


**Description:**

    When the view changes in the visualization window, it puts the old view on
    a stack of views. VisIt provides the UndoView function that lets you undo
    view changes. The RedoView function re-applies any views that have been
    undone by the UndoView function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Subset", "mat1")
  DrawPlots()
  v = GetView2D()
  v.windowCoords = (-2.3,2.4,0.2,4.9)
  SetView2D(v)
  UndoView()
  RedoView()


RedrawWindow
------------

**Synopsis:**

::

  RedrawWindow() -> integer

return type : CLI_return_t
    The RedrawWindow function returns 1 on success and 0 on failure.


**Description:**

    The RedrawWindow function allows a visualization window to redraw itself
    and then forces the window to redraw. This function does the opposite of
    the DisableRedraw function and is used to recover from it.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Contour", "u")
  AddPlot("Pseudocolor", "w")
  DrawPlots()
  DisableRedraw()
  AddOperator("Slice")
  # Set the slice operator attributes
  # Redraw now that the operator attributes are set. This will
  # prevent 1 redraw.
  RedrawWindow()


RegisterCallback
----------------

**Synopsis:**

::

  RegisterCallback(callbackname, callback) --> integer


callbackname : string
    A string object designating the callback that we're installing. Allowable
    values are returned by the GetCallbackNames() function.

callback : python function
    A Python function, typically with one argument by which VisIt passes the
    object that caused the callback to be called.

return type : CLI_return_t
    RegisterCallback returns 1 on success.


**Description:**

    The RegisterCallback function is used to associate a user-defined callback
    function with the updating of a state object or execution of a particular
    rpc


**Example:**

::

  import visit
  def print_sliceatts(atts):
      print("SLICEATTS=", atts)
      
  visit.RegisterCallback("SliceAttributes", print_sliceatts)


RegisterMacro
-------------

**Synopsis:**

::

  RegisterMacro(name, callable)


name : string
    The name of the macro.

callable : python function
    A Python function that will be associated with the macro name.


**Description:**

    The RegisterMacro function lets you associate a Python function with a name
    so when VisIt's gui calls down into Python to execute a macro, it ends up
    executing the registered Python function. Macros let users define complex
    new behaviors using Python functions yet still call them simply by clicking
    a button within VisIt's gui. When a new macro function is registered, a
    message is sent to the gui that adds the known macros as buttons in the
    Macros window.


**Example:**

::

  def SetupMyPlots():
      OpenDatabase('noise.silo')
      AddPlot('Pseudocolor', 'hardyglobal')
      DrawPlots()
      
  RegisterMacro('Setup My Plots', SetupMyPlots)


RemoveAllOperators
------------------

**Synopsis:**

::

  RemoveAllOperators() -> integer
  RemoveAllOperators(all) -> integer


all : integer
    An optional integer argument that tells the function to ignore the
    active plots and use all plots in the plot list if the value of
    the argument is non-zero.

return type : CLI_return_t
    All functions return an integer value of 1 for success and 0 for failure.


**Description:**

    The RemoveAllOperators function removes all operators from the active
    plots in the active visualization window. If the all argument is
    provided and contains a non-zero value, all plots in the active
    visualization window are affected. If the value is zero or if the argument
    is not provided, only the active plots are affected.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddOperator("Threshold")
  AddOperator("Slice")
  AddOperator("SphereSlice")
  DrawPlots()
  RemoveLastOperator() # Remove SphereSlice
  RemoveOperator(0) # Remove Threshold
  RemoveAllOperators() # Remove the rest of the operators


RemoveLastOperator
------------------

**Synopsis:**

::

  RemoveLastOperator() -> integer
  RemoveLastOperator(all) -> integer


all : integer
    An optional integer argument that tells the function to ignore the active
    plots and use all plots in the plot list if the value of the argument is
    non-zero.

return type : CLI_return_t
    All functions return an integer value of 1 for success and 0 for failure.


**Description:**

    The RemoveLastOperator function removes the operator that was last applied
    to the active plots. If the all argument is provided and contains a
    non-zero value, all plots in the active visualization window are affected.
    If the value is zero or if the argument is not provided, only the active
    plots are affected.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddOperator("Threshold")
  AddOperator("Slice")
  AddOperator("SphereSlice")
  DrawPlots()
  RemoveLastOperator() # Remove SphereSlice
  RemoveOperator(0) # Remove Threshold
  RemoveAllOperators() # Remove the rest of the operators


RemoveMachineProfile
--------------------

**Synopsis:**

::

  RemoveMachineProfile(hostname) -> integer


hostname : string
    

**Description:**

    Removes machine profile with hostname from HostProfileList


RemoveOperator
--------------

**Synopsis:**

::

  RemoveOperator(index) -> integer
  RemoveOperator(index, all) -> integer


all : integer
    An optional integer argument that tells the function to ignore the
    active plots and use all plots in the plot list if the value of the
    argument is non-zero.

index : integer
    The zero-based integer index into a plot's operator list that specifies
    which operator is to be deleted.

return type : CLI_return_t
    All functions return an integer value of 1 for success and 0 for failure.


**Description:**

    The RemoveOperator functions allow operators to be removed from plots.
    If the all argument is provided and contains a non-zero value, all
    plots in the active visualization window are affected. If the value
    is zero or if the argument is not provided, only the active plots
    are affected.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddOperator("Threshold")
  AddOperator("Slice")
  AddOperator("SphereSlice")
  DrawPlots()
  RemoveLastOperator() # Remove SphereSlice
  RemoveOperator(0) # Remove Threshold
  RemoveAllOperators() # Remove the rest of the operators


RemovePicks
-----------

**Synopsis:**

::

  RemovePicks()


**Description:**

    The RemovePicks function removes a list of pick points from the active
    visualization window. Pick points are the letters that are added to the
    visualization window where the mouse is clicked when the visualization
    window is in pick mode.


**Example:**

::

  #% visit -cli
  # Put the visualization window into pick mode using the popup
  # menu and add some pick points (let's say A -> G).
  # Clear the pick points.
  RemovePicks('A, B, D')


RenamePickLabel
---------------

**Synopsis:**

::

  RenamePickLabel(oldLabel, newLabel) -> integer


oldLabel : string
    A string that is the old pick label to replace. (e.g. 'A', 'B').

newLabel : string
    A string that is the new label to display in place of the old label.

return type : CLI_return_t
    The RenamePickLabel function returns 1 on success and 0 on failure.


**Description:**

    The RenamePickLabel function can be used to replace an automatically generated
    pick label such as 'A' with a user-defined string.


**Example:**

::

  RenamePickLabel('A', 'Point of interest')


ReplaceDatabase
---------------

**Synopsis:**

::

  ReplaceDatabase(databaseName) -> integer
  ReplaceDatabase(databaseName, timeState) -> integer


databaseName : string
    The name of the new database.

timeState : integer
    A zero-based integer containing the time state that should be made active
    once the database has been replaced.

return type : CLI_return_t
    The ReplaceDatabase function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    The ReplaceDatabase function replaces the database in the current plots
    with a new database. This is one way of switching timesteps if no
    ".visit" file was ever created. If two databases have the same variable
    name then replace is usually a success. In the case where the new database
    does not have the desired variable, the plot with the variable not
    contained in the new database does not get regenerated with the new
    database.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  ReplaceDatabase("/usr/gapps/visit/data/curv3d.silo")
  SaveWindow()
  # Replace with a time-varying database and change the time
  # state to 17.
  ReplaceDatabase("/usr/gapps/visit/data/wave.visit", 17)


ResetLineoutColor
-----------------

**Synopsis:**

::

  ResetLineoutColor() -> integer

return type : CLI_return_t
    ResetLineoutColor returns 1 on success and 0 on failure.


**Description:**

    Lineouts on VisIt cause reference lines to be drawn over the plot where
    the lineout was being extracted. Each reference line uses a different color
    in a discrete color table. Once the colors in the discrete color table are
    used up, the reference lines start using the color from the start of the
    discrete color table and so on. ResetLineoutColor forces reference lines to
    start using the color at the start of the discrete color table again thus
    resetting the lineout color.


ResetOperatorOptions
--------------------

**Synopsis:**

::

  ResetOperatorOptions(operatorType) -> integer
  ResetOperatorOptions(operatorType, all) -> integer


operatorType : string
    The name of a valid operator type.

all : integer
    An optional integer argument that tells the function to reset the operator
    options for all plots regardless of whether or not they are active.

return type : CLI_return_t
    The ResetOperatorOptions function returns an integer value of 1 for
    success and 0 for failure.


**Description:**

    The ResetOperatorOptions function resets the operator attributes of the
    specified operator type for the active plots back to the default values.
    The operatorType argument is a string containing the name of the type of
    operator whose attributes are to be reset. The all argument is an optional
    flag that tells the function to reset the operator attributes for the
    indicated operator in all plots regardless of whether the plots are
    active. When non-zero values are passed for the all argument, all plots
    are reset. When the all argument is zero or not provided, only the
    operators on active plots are modified.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  AddOperator("Slice")
  a = SliceAttributes()
  a.normal,a.upAxis = (0,0,1),(0,1,0)
  SetOperatorOptions(a)
  ResetOperatorOptions("Slice")


ResetPickLetter
---------------

**Synopsis:**

::

  ResetPickLetter() -> integer

return type : CLI_return_t
    ResetPickLetter returns 1 on success and 0 on failure.


**Description:**

    The ResetPickLetter function resets the pick marker back to "A" so that
    the next pick will use "A" as the pick letter and then "B" and so on.


ResetPlotOptions
----------------

**Synopsis:**

::

  ResetPlotOptions(plotType) -> integer


plotType : string
    The name of the plot type.

return type : CLI_return_t
    The ResetPlotOptions function returns an integer value of 1 for success
    and 0 for failure.


**Description:**

    The ResetPlotOptions function resets the plot attributes of the specified
    plot type for the active plots back to the default values. The plotType
    argument is a string containing the name of the type of plot whose
    attributes are to be reset.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  p = PseudocolorAttributes()
  p.colorTableName = "calewhite"
  p.minFlag,p.maxFlag = 1,1
  p.min,p.max = -5.0, 8.0
  SetPlotOptions(p)
  ResetPlotOptions("Pseudocolor")


ResetView
---------

**Synopsis:**

::

  ResetView() -> integer

return type : CLI_return_t
    The ResetView function returns 1 on success and 0 on failure.


**Description:**

    The ResetView function resets the camera to the initial view.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv3d.silo")
  AddPlot("Mesh", "curvmesh3d")
  v = ViewAttributes()
  v.camera = (-0.45396, 0.401908, 0.79523)
  v.focus = (0, 2.5, 15)
  v.viewUp = (0.109387, 0.910879, -0.397913)
  v.viewAngle = 30
  v.setScale = 1
  v.parallelScale = 16.0078
  v.nearPlane = -32.0156
  v.farPlane = 32.0156
  v.perspective = 1
  SetView3D(v) # Set the 3D view
  DrawPlots()
  ResetView()


ResizeWindow
------------

**Synopsis:**

::

  ResizeWindow(win, w, h) -> integer


win : integer
    The integer id of the window to be moved [1..16].

w : integer
    The new integer width for the window.

h : integer
    The new integer height for the window.

return type : CLI_return_t
    ResizeWindow returns 1 on success and 0 on failure.


**Description:**

    ResizeWindow resizes a visualization window.


**Example:**

::

  #% visit -cli
  ResizeWindow(1, 300, 600)


RestoreSession
--------------

**Synopsis:**

::

  RestoreSession(filename, visitDir) -> integer


filename : string
    The name of the session file to restore.

visitDir : integer
    An integer flag that indicates whether the filename to be restored is
    located in the user's VisIt directory. If the flag is set to 1 then the
    session file is assumed to be located in the user's VisIt directory
    otherwise the filename must contain an absolute path.

return type : CLI_return_t
    RestoreSession returns 1 on success and 0 on failure.


**Description:**

    The RestoreSession function is important for setting up complex
    visualizations because you can design a VisIt session file, which is an XML
    file that describes exactly how plots are set up, using the VisIt GUI and
    then use that same session file in the CLI to generate movies in batch. The
    RestoreSession function takes 2 arguments. The first argument specifies the
    filename that contains the VisIt session to be restored. The second
    argument determines whether the session file is assumed to be in the user's
    VisIt directory. If the visitDir argument is set to 0 then the filename
    argument must contain the absolute path to the session file.


**Example:**

::

  #% visit -cli
  # Restore my session file for a time-varying database from
  # my .visit directory.
  RestoreSessionFile("visit.session", 1)
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()


RestoreSessionWithDifferentSources
----------------------------------

**Synopsis:**

::

  RestoreSessionWithDifferentSources(filename, visitDir, mapping) -> integer


filename : string
    The name of the session file to restore.

visitDir : integer
    An integer flag that indicates whether the filename to be restored is
    located in the user's VisIt directory. If the flag is set to 1 then the
    session file is assumed to be located in the user's VisIt directory
    otherwise the filename must contain an absolute path.

mapping : tuple
    A tuple of strings representing the maping from sources as specified
    in the original session file to new sources. Sources in the original
    session file are numbered starting from 0. So, this tuple of strings
    simply contains the new names for each of the sources, in order.

return type : CLI_return_t
    RestoreSession returns 1 on success and 0 on failure.


**Description:**

    The RestoreSession function is important for setting up complex
    visualizations because you can design a VisIt session file, which is an XML
    file that describes exactly how plots are set up, using the VisIt GUI and
    then use that same session file in the CLI to generate movies in batch. The
    RestoreSession function takes 2 arguments. The first argument specifies the
    filename that contains the VisIt session to be restored. The second
    argument determines whether the session file is assumed to be in the user's
    VisIt directory. If the visitDir argument is set to 0 then the filename
    argument must contain the absolute path to the session file.


**Example:**

::

  #% visit -cli
  # Restore my session file for a time-varying database from
  # my .visit directory.
  RestoreSessionFile("visit.session", 1)
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()


SaveAttribute
-------------

**Synopsis:**

::

  SaveAttribute(filename, object)


filename : string
    The name of the XML file to load the attribute from or save the attribute to.

object
    The object to load or save.


**Description:**

    The LoadAttribute and SaveAttribute methods save a single
    attribute, such as a current plot or operator python object,
    to a standalone XML file.  Note that LoadAttribute requires
    that the target attribute already be created by other means;
    it fills, but does not create, the attribute.


**Example:**

::

  #% visit -cli
  a = MeshPlotAttributes()
  SaveAttribute('mesh.xml', a)
  b = MeshPlotAttributes()
  LoadAttribute('mesh.xml', b)


SaveNamedSelection
------------------

**Synopsis:**

::

  SaveNamedSelection(name) -> integer


name : string
    The name of a named selection.

return type : CLI_return_t
    The SaveNamedSelection function returns 1 for success and 0 for failure.


**Description:**

    Named Selections allow you to select a group of elements (or particles).
    One typically creates a named selection from a group of elements and then
    later applies the named selection to another plot (thus reducing the
    set of elements displayed to the ones from when the named selection was
    created).  Named selections only last for the current session.  If you
    create a named selection that you want to use over and over, you can save
    it to a file with the SaveNamedSelection function.


**Example:**

::

  #% visit -cli
  db = "/usr/gapps/visit/data/wave*.silo database"
  OpenDatabase(db)
  AddPlot("Pseudocolor", "pressure")
  AddOperator("Clip")
  c = ClipAttributes()
  c.plane1Origin = (0,0.6,0)
  c.plane1Normal = (0,-1,0)
  SetOperatorOption(c)
  DrawPlots()
  CreateNamedSelection("els_above_at_time_0")
  SaveNamedSelection("els_above_at_time_0")


SaveSession
-----------

**Synopsis:**

::

  SaveSession(filename) -> integer


filename : string
    The filename argument is the filename that is used to save the session
    file. The filename is relative to the current working directory.

return type : CLI_return_t
    The SaveSession function returns 1 on success and 0 on failure.


**Description:**

    The SaveSession function tells VisIt to save an XML session file that
    describes everything about the current visualization. Session files are
    very useful for creating movies and also as shortcuts for setting up
    complex visualizations.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  # Set up a keyframe animation of view and save a session file of it.
  k = GetKeyframeAttributes()
  k.enabled,k.nFrames,k.nFramesWasUserSet = 1,20,1
  SetKeyframeAttributes(k)
  AddPlot("Surface", "hgslice")
  DrawPlots()
  v = GetView3D()
  v.viewNormal = (0.40823, -0.826468, 0.387684)
  v.viewUp, v.imageZoom = (-0.261942, 0.300775, 0.917017), 1.60684
  SetView3D(v)
  SetViewKeyframe()
  SetTimeSliderState(TimeSliderGetNStates() - 1)
  v.viewNormal = (-0.291901, -0.435608, 0.851492)
  v.viewUp = (0.516969, 0.677156, 0.523644)
  SetView3D(v)
  SetViewKeyframe()
  ToggleCameraViewMode()
  SaveSession("~/.visit/keyframe.session")


SaveWindow
----------

**Synopsis:**

::

  SaveWindow() -> string

return type : string
    The SaveWindow function returns a string containing the name of the file
    that was saved.


**Description:**

    The SaveWindow function saves the contents of the active visualization
    window. The format of the saved window is dictated by the
    SaveWindowAttributes which can be set using the SetSaveWindowAttributes
    function. The contents of the active visualization window can be saved as
    TIFF, JPEG, RGB, PPM, PNG images or they can be saved as curve, Alias
    Wavefront Obj, or VTK geometry files.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv3d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set the save window attributes.
  s = SaveWindowAttributes()
  s.fileName = "test"
  s.format = s.JPEG
  s.progressive = 1
  s.fileName = "test"
  SetSaveWindowAttributes(s)
  name = SaveWindow()
  print("name = %s" % name)


SendSimulationCommand
---------------------

**Synopsis:**

::

  SendSimulationCommand(host, simulation, command)
  SendSimulationCommand(host, simulation, command, argument)


host : string
    The name of the computer where the simulation is running.

simulation : string
    The name of the simulation being processed at the specified host.

command : string
    A string that is the command to send to the simulation.

argument
    An argument to the command.


**Description:**

    The SendSimulationCommand method tells the viewer to send a command to a
    simulation that is running on the specified host. The host argument is a
    string that contains the name of the computer where the simulation is
    running. The simulation argument is a string that contains the name of the
    simulation to send the command to.


SetActiveContinuousColorTable
-----------------------------

**Synopsis:**

::

  SetActiveContinuousColorTable(name) -> integer


name : string
    The name of the color table to use for the active color table. The name
    must be present in the tuple returned by the ColorTableNames function.

return type : CLI_return_t
    Both functions return 1 on success and 0 on failure.


**Description:**

    VisIt supports two flavors of color tables: continuous and discrete. Both
    types of color tables have the same underlying representation but each type
    of color table is used a slightly different way. Continuous color tables
    are made of a small number of color control points and the gaps in the
    color table between two color control points are filled by interpolating
    the colors of the color control points. Discrete color tables do not use
    any kind of interpolation and like continuous color tables, they are made
    up of control points. The color control points in a discrete color table
    repeat infinitely such that if we have 4 color control points: A, B, C, D
    then the pattern of repetition is: ABCDABCDABCD... Discrete color tables
    are mainly used for plots that have a discrete set of items to display
    (e.g. Subset plot). Continuous color tables are used in plots that display
    a continuous range of values (e.g. Pseudocolor).


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Contour", "hgslice")
  DrawPlots()
  SetActiveDiscreteColorTable("levels")


SetActiveDiscreteColorTable
---------------------------

**Synopsis:**

::

  SetActiveDiscreteColorTable(name) -> integer


name : string
    The name of the color table to use for the active color table. The name
    must be present in the tuple returned by the ColorTableNames function.

return type : CLI_return_t
    Both functions return 1 on success and 0 on failure.


**Description:**

    VisIt supports two flavors of color tables: continuous and discrete. Both
    types of color tables have the same underlying representation but each type
    of color table is used a slightly different way. Continuous color tables
    are made of a small number of color control points and the gaps in the
    color table between two color control points are filled by interpolating
    the colors of the color control points. Discrete color tables do not use
    any kind of interpolation and like continuous color tables, they are made
    up of control points. The color control points in a discrete color table
    repeat infinitely such that if we have 4 color control points: A, B, C, D
    then the pattern of repetition is: ABCDABCDABCD... Discrete color tables
    are mainly used for plots that have a discrete set of items to display
    (e.g. Subset plot). Continuous color tables are used in plots that display
    a continuous range of values (e.g. Pseudocolor).


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Contour", "hgslice")
  DrawPlots()
  SetActiveDiscreteColorTable("levels")


SetActivePlots
--------------

**Synopsis:**

::

  SetActivePlots(plots) -> integer


plots : tuple of integers
    A tuple of integer plot indices starting at zero. A single integer is
    also accepted

return type : CLI_return_t
    The SetActivePlots function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    Any time VisIt sets the attributes for a plot, it only sets the attributes
    for plots which are active. The SetActivePlots function must be called to
    set the active plots. The function takes one argument which is a tuple of
    integer plot indices that start at zero. If only one plot is being
    selected, the plots argument can be an integer instead of a tuple.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Subset", "mat1")
  AddPlot("Mesh", "mesh1")
  AddPlot("Contour", "u")
  DrawPlots()
  SetActivePlots((0,1,2)) # Make all plots active
  SetActivePlots(0) # Make only the Subset plot active


SetActiveTimeSlider
-------------------

**Synopsis:**

::

  SetActiveTimeSlider(tsName) -> integer


tsName : string
    The name of the time slider that should be made active.

return type : CLI_return_t
    SetActiveTimeSlider returns 1 on success and 0 on failure.


**Description:**

    Sets the active time slider, which is the time slider that is used to
    change time states.


**Example:**

::

  #% visit -cli
  path = "/usr/gapps/visit/data/"
  dbs = (path + "dbA00.pdb", path + "dbB00.pdb", path + "dbC00.pdb")
  for db in dbs:
      OpenDatabase(db)
      AddPlot("FilledBoundary", "material(mesh)")
      DrawPlots()
  CreateDatabaseCorrelation("common", dbs, 1)
  tsNames = GetWindowInformation().timeSliders
  for ts in tsNames:
      SetActiveTimeSlider(ts)
  for state in list(range(TimeSliderGetNStates())) + [0]:
      SetTimeSliderState(state)


SetActiveWindow
---------------

**Synopsis:**

::

  SetActiveWindow(windowIndex) -> integer
  SetActiveWindow(windowIndex, raiseWindow) -> integer


windowIndex : integer
    An integer window index starting at 1.

raiseWindow : integer
    This is an optional integer argument that raises and activates the window if
    set to 1. If omitted, the default behavior is to raise and activate the window.

return type : CLI_return_t
    The SetActiveWindow function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    Most of the functions in the VisIt Python Interface operate on the
    contents of the active window. If there is more than one window, it is very
    important to be able to set the active window. To set the active window,
    use the SetActiveWindow function. The SetActiveWindow function takes a
    single integer argument which is the index of the new active window. The
    new window index must be an integer greater than zero and less than or
    equal to the number of open windows.


**Example:**

::

  #% visit -cli
  SetWindowLayout(2)
  SetActiveWindow(2)
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Mesh", "mesh1")
  DrawPlots()


SetAnimationTimeout
-------------------

**Synopsis:**

::

  SetAnimationTimeout(milliseconds) -> integer

return type : CLI_return_t
    The SetAnimationTimeout function returns 1 for success and 0 for failure.


**Description:**

    The SetAnimationTimeout function sets the animation timeout which is a
    value that governs how fast animations play. The timeout is specified in
    milliseconds and has a default value of 1 millisecond. Larger timeout
    values decrease the speed at which animations play.


**Example:**

::

  #%visit -cli
  # Play a new frame every 5 seconds.
  SetAnimationTimeout(5000)
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  # Click the play button in the toolbar


SetAnnotationAttributes
-----------------------

**Synopsis:**

::

  SetAnnotationAttributes(atts) -> integer


atts : AnnotationAttributes object
    An AnnotationAttributes object containing the annotation settings.

return type : CLI_return_t
    Both functions return 1 on success and 0 on failure.


**Description:**

    The annotation settings control what bits of text are drawn in the
    visualization window. Among the annotations are the plot legends, database
    information, user information, plot axes, triad, and the background style
    and colors. Setting the annotation attributes is important for producing
    quality visualizations. The annotation settings are stored in
    AnnotationAttributes objects. To set the annotation attributes, first
    create an AnnotationAttributes object using the AnnotationAttributes
    function and then pass the object to the SetAnnotationAttributes function.
    To set the default annotation attributes, also pass the object to the
    SetDefaultAnnotationAttributes function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  a = AnnotationAttributes()
  a.gradientBackgroundStyle = a.GRADIENTSTYLE_RADIAL
  a.gradientColor1 = (0,255,255)
  a.gradientColor2 = (0,0,0)
  a.backgroundMode = a.BACKGROUNDMODE_GRADIENT
  SetAnnotationAttributes(a)


SetBackendType
--------------

**Synopsis:**

::

  SetBackendType(name) -> integer


name : string
    VTK, VTKM.

return type : CLI_return_t
    Both functions return 1 on success and 0 on failure.


**Description:**

    The compute back end determines the compute library that is used for
    processing plots in VisIt. The default is VTK, which supports all VisIt
    operations. VTKm can be used too but it only supports a fraction
    of VisIt's functionality. Filters that support VTKm will use those
    libraries when their compute back end is selected using this function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  SetBackendType("VTKm")
  AddPlot("Contour", "radial")
  DrawPlots()


SetCenterOfRotation
-------------------

**Synopsis:**

::

  SetCenterOfRotation(x,y,z) -> integer


x : double
    A double that is the x component of the center of rotation.

y : double
    A double that is the y component of the center of rotation.

z : double
    A double that is the z component of the center of rotation.

return type : CLI_return_t
    The SetCenterOfRotation function returns 1 on success and 0 on failure.


**Description:**

    The SetCenterOfRotation function sets the center of rotation for plots in
    a 3D visualization window. The center of rotation, is the point about which
    plots are rotated when you interactively spin the plots using the mouse. It
    is useful to set the center of rotation if you've zoomed in on any 3D plots
    so in the event that you rotate the plots, the point of interest remains
    fixed on the screen.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddPlot("Mesh", "mesh1")
  DrawPlots()
  v = GetView3D()
  v.viewNormal = (-0.409139, 0.631025, 0.6591)
  v.viewUp = (0.320232, 0.775678, -0.543851)
  v.imageZoom = 4.8006
  SetCenterOfRotation(-4.755280, 6.545080, 5.877850)
  # Rotate the plots interactively.


SetColorTexturingEnabled
------------------------

**Synopsis:**

::

  SetColorTexturingEnabled(enabled) -> integer


enabled : integer
    A integer value. Non-zero values enable color texturing and zero disables it.

return type : CLI_return_t
    The SetColorTexturingEnabled function returns 1 on success and 0 on failure.


**Description:**

    Node-centered variables are drawn on plots such as the Pseudocolor plot such
    that the nodal value looks interpolated throughout the zone. This can be done
    by interpolating colors, which can produce some colors that do not appear in
    a color table. Alternatively, the nodal values can be mapped to a texture
    coordinate in a 1D texture and those values can be interpolated, with colors
    being selected after interpolating the texture coordinate. This method always
    uses colors that are defined in the color table.


**Example:**

::

  SetColorTexturingEnabled(1)


SetCreateMeshQualityExpressions
-------------------------------

**Synopsis:**

::

  SetCreateMeshQualityExpressions(val) -> integer


val : integer
    Either a zero (false) or non-zero (true) integer value to indicate ifMesh Quality expressions should be automatically created when a database is opened.

return type : CLI_return_t
    The SetCreateMeshQualityExpressions function returns 1 on success and 0 on failure.


**Description:**

    The SetCreateMeshQualityExpressions function sets a boolean in the
    global attributes indicating whether or not Mesh Quality expressions
    should be automatically created. The default behavior is for the
    expressions to be created, which may slow down VisIt's performance
    if there is an extraordinary large number of meshes.  Turning this
    feature off tells VisIt to skip automatic creation of the Mesh Quality
    expressions.


**Example:**

::

  #% visit -cli
  SetCreateMeshQualityExpressions(1) # turn this feature on
  SetCreateMeshQualityExpressions(0) # turn this feature off


SetCreateTimeDerivativeExpressions
----------------------------------

**Synopsis:**

::

  SetCreateTimeDerivativeExpressions(val) -> integer


val : integer
    Either a zero (false) or non-zero (true) integer value to indicate if
    Time Derivative expressions should be automatically created when a
    database is opened.

return type : CLI_return_t
    The SetCreateTimeDerivativeExpressions function returns 1 on success
    and 0 on failure.


**Description:**

    The SetCreateTimeDerivativeExpressions function sets a boolean in the
    global attributes indicating whether or not Time Derivative expressions
    should be automatically created. The default behavior is for the
    expressions to be created, which may slow down VisIt's performance
    if there is an extraordinary large number of variables.  Turning this
    feature off tells VisIt to skip automatic creation of the Time Derivative
    expressions.


**Example:**

::

  #% visit -cli
  SetCreateTimeDerivativeExpressions(1) # turn this feature on
  SetCreateTimeDerivativeExpressions(0) # turn this feature off


SetCreateVectorMagnitudeExpressions
-----------------------------------

**Synopsis:**

::

  SetCreateVectorMagnitudeExpressions(val) -> integer


val : integer
    Either a zero (false) or non-zero (true) integer value to indicate if
    Vector magnitude expressions should be automatically created when a database is opened.

return type : CLI_return_t
    The SetCreateVectorMagnitudeExpressions function returns 1 on success
    and 0 on failure.


**Description:**

    The SetCreateVectorMagnitudeExpressions function sets a boolean in the
    global attributes indicating whether or not vector magnitude expressions
    should be automatically created. The default behavior is for the
    expressions to be created, which may slow down VisIt's performance
    if there is an extraordinary large number of vector variables. Turning this
    feature off tells VisIt to skip automatic creation of the vector
    magnitude expressions.


**Example:**

::

  #% visit -cli
  SetCreateVectorMagnitudeExpressions(1) # turn this feature on
  SetCreateVectorMagnitudeExpressions(0) # turn this feature off


SetDatabaseCorrelationOptions
-----------------------------

**Synopsis:**

::

  SetDatabaseCorrelationOptions(method, whenToCreate) -> integer


method : integer
    An integer that tells VisIt what default method to use when automatically
    creating a database correlation. The value must be in the range [0,3].
    
    ==========  =========================
    **method**  Description
    ==========  =========================
    0           IndexForIndexCorrelation
    1           StretchedIndexCorrelation
    2           TimeCorrelation
    3           CycleCorrelation
    ==========  =========================

whenToCreate : integer
    An integer that tells VisIt when to automatically create database correlations.
    
    ================  ===================================
    **whenToCreate**  Description
    ================  ===================================
    0                 Always create database correlation
    1                 Never create database correlation
    2                 Create database correlation only if
                      the new time-varying database has
    ================  ===================================

return type : CLI_return_t
    SetDatabaseCorrelationOptions returns 1 on success and 0 on failure.


**Description:**

    VisIt provides functions to explicitly create and alter database
    correlations but there are also a number of occasions where VisIt can
    automatically create a database correlation. The
    SetDatabaseCorrelationOptions function allows you to tell VisIt the default
    correlation method to use when automatically creating a new database
    correlation and it also allows you to tell VisIt when database correlations
    can be automatically created.
    the same length as another time-varying database already being used in a
    plot.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/dbA00.pdb")
  AddPlot("FilledBoundary", "material(mesh)")
  DrawPlots()
  # Always create a stretched index correlation.
  SetDatabaseCorrelationOptions(1, 0)
  OpenDatabase("/usr/gapps/visit/data/dbB00.pdb")
  AddPlot("FilledBoundary", "material(mesh)")
  # The AddPlot caused a database correlation to be created.
  DrawPlots()
  wi = GetWindowInformation()
  print("Active time slider: " % wi.timeSliders[wi.activeTimeSlider])
  # This will set time for both databases since the database correlation is
  # the active time slider.
  SetTimeSliderState(5)


SetDebugLevel
-------------

**Synopsis:**

::

  SetDebugLevel(level)


level : string
    A string '1', '2', '3', '4', '5' with an optional 'b' suffix to indicate
    whether the output should be buffered. A value of '1' is a low debug
    level, which should be used to produce little output while a value of 5
    should produce a lot of debug output.


**Description:**

    The GetDebugLevel and SetDebugLevel functions are used when debugging
    VisIt Python scripts. The SetDebugLevel function sets the debug level for
    VisIt's viewer thus it must be called before a Launch method. The debug
    level determines how much detail is written to VisIt's execution logs when
    it executes.


**Example:**

::

  #% visit -cli -debug 2
  print("VisIt's debug level is: %d" % GetDebugLevel())


SetDefaultAnnotationAttributes
------------------------------

**Synopsis:**

::

  SetDefaultAnnotationAttributes(atts) -> integer


atts : AnnotationAttributes object
    An AnnotationAttributes object containing the annotation settings.

return type : CLI_return_t
    Both functions return 1 on success and 0 on failure.


**Description:**

    The annotation settings control what bits of text are drawn in the
    visualization window. Among the annotations are the plot legends, database
    information, user information, plot axes, triad, and the background style
    and colors. Setting the annotation attributes is important for producing
    quality visualizations. The annotation settings are stored in
    AnnotationAttributes objects. To set the annotation attributes, first
    create an AnnotationAttributes object using the AnnotationAttributes
    function and then pass the object to the SetAnnotationAttributes function.
    To set the default annotation attributes, also pass the object to the
    SetDefaultAnnotationAttributes function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  a = AnnotationAttributes()
  a.gradientBackgroundStyle = a.GRADIENTSTYLE_RADIAL
  a.gradientColor1 = (0,255,255)
  a.gradientColor2 = (0,0,0)
  a.backgroundMode = a.BACKGROUNDMODE_GRADIENT
  SetAnnotationAttributes(a)


SetDefaultFileOpenOptions
-------------------------

**Synopsis:**

::

  SetDefaultFileOpenOptions(pluginName, options) -> integer


pluginName : string
    The name of a plugin.

options : dictionary
    A dictionary containing the new default options for that plugin.

return type : CLI_return_t
    The SetDefaultFileOpenOptions function returns 1 on success and 0 on
    failure.


**Description:**

    SetDefaultFileOpenOptions sets the current options used to open new
    files when a specific plugin is triggered.


**Example:**

::

  #% visit -cli
  OpenMDServer()
  opts = GetDefaultFileOpenOptions("VASP")
  opts["Allow multiple timesteps"] = 1
  SetDefaultFileOpenOptions("VASP", opts)
  OpenDatabase("CHGCAR")


SetDefaultInteractorAttributes
------------------------------

**Synopsis:**

::

  SetDefaultInteractorAttributes(atts) -> integer


atts : InteractorAttributes object
    An InteractorAttributes object that contains the new interactor attributes
    that you want to use.

return type : CLI_return_t
    SetInteractorAttributes returns 1 on success and 0 on failure.


**Description:**

    The SetInteractorAttributes function is used to set certain interactor
    properties. Interactors, can be thought of as how mouse clicks and
    movements are translated into actions in the vis window. To set the
    interactor attributes, first get the interactor attributes using the
    GetInteractorAttributes function. Once you've set the
    object's properties, call the SetInteractorAttributes function to make
    VisIt use the new interactor attributes.
    The SetDefaultInteractorAttributes function sets the default interactor
    attributes, which are used for new visualization windows. The default
    interactor attributes can also be saved to the VisIt configuration file to
    ensure that future VisIt sessions have the right default interactor
    attributes.


**Example:**

::

  #% visit -cli
  ia = GetInteractorAttributes()
  print(ia)
  ia.showGuidelines = 0
  SetInteractorAttributes(ia)


SetDefaultMaterialAttributes
----------------------------

**Synopsis:**

::

  SetDefaultMaterialAttributes(atts) -> integer


atts : MaterialAttributes object
    A MaterialAttributes object containing the new settings.

return type : CLI_return_t
    Both functions return 1 on success and 0 on failure.


**Description:**

    The SetMaterialAttributes function takes a MaterialAttributes object and
    makes VisIt use the material settings that it contains. You use the
    SetMaterialAttributes function when you want to change how VisIt performs
    material interface reconstruction. The SetDefaultMaterialAttributes
    function sets the default material attributes, which are saved to the
    config file and are also used by new visualization windows.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/allinone00.pdb")
  AddPlot("Pseudocolor", "mesh/mixvar")
  p = PseudocolorAttributes()
  p.min,p.minFlag = 4.0, 1
  p.max,p.maxFlag = 13.0, 1
  SetPlotOptions(p)
  DrawPlots()
  # Tell VisIt to always do material interface reconstruction.
  m = GetMaterialAttributes()
  m.forceMIR = 1
  SetMaterialAttributes(m)
  ClearWindow()
  # Redraw the plot forcing VisIt to use the mixed variable information.
  DrawPlots()


SetDefaultMeshManagementAttributes
----------------------------------

**Synopsis:**

::

  SetMeshManagementAttributes() -> MeshmanagementAttributes object

return type : MeshmanagementAttributes object
    Returns a MeshmanagementAttributes object.


**Description:**

    The GetMeshmanagementAttributes function returns a MeshmanagementAttributes object
    that contains VisIt's current mesh discretization settings.
    You can set properties on the MeshManagementAttributes object and then pass it to
    SetMeshManagementAttributes to make VisIt use the new material attributes that
    you've specified:


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/csg.silo")
  AddPlot("Mesh", "csgmesh")
  DrawPlots()
  # Tell VisIt to always do material interface reconstruction.
  mma = GetMeshManagementAttributes()
  mma.discretizationTolernace = (0.01, 0.025)
  SetMeshManagementAttributes(mma)
  ClearWindow()
  # Redraw the plot forcing VisIt to use the mixed variable information.
  DrawPlots()


SetDefaultOperatorOptions
-------------------------

**Synopsis:**

::

  SetDefaultOperatorOptions(atts) -> integer


atts : operator attributes object
    Any type of operator attributes object.

return type : CLI_return_t
    All functions return an integer value of 1 for success and 0 for failure.


**Description:**

    Each operator in VisIt has a group of attributes that controls the
    operator. To set the attributes for an operator, first create an operator
    attributes object. This is done by calling a function which is the name of
    the operator plus the word "Attributes". For example, a Slice operator's
    operator attributes object is created and returned by the SliceAttributes
    function. Assign the new operator attributes object into a variable and set
    its fields. After setting the desired fields in the operator attributes
    object, pass the object to the SetOperatorOptions function. The
    SetOperatorOptions function determines the type of operator to which the
    operator attributes object applies and sets the attributes for that
    operator type. To set the default plot attributes, use the
    SetDefaultOperatorOptions function. Setting the default attributes ensures
    that all future instances of a certain operator are initialized with the
    new default values. Note that there is no SetOperatorOptions(atts, all)
    variant of this call. To set operator options for all plots that have a
    an instance of the associated operator, you must first make all plots
    active with SetActivePlots() and then use the SetOperatorOptions(atts)
    variant.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddPlot("Mesh", "mesh1")
  AddOperator("Slice", 1) # Add the operator to both plots
  a = SliceAttributes()
  a.normal, a.upAxis = (0,0,1), (0,1,0)
  # Only set the attributes for the active plot.
  SetOperatorOptions(a)
  DrawPlots()


SetDefaultPickAttributes
------------------------

**Synopsis:**

::

  SetDefaultPickAttributes(atts) -> integer


atts : PickAttributes object
    A PickAttributes object containing the new pick settings.

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The SetPickAttributes function changes the pick attributes that are used
    when VisIt picks on plots. The pick attributes allow you to format your
    pick output in various ways and also allows you to select auxiliary pick
    variables.


**Example:**

::

  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hgslice")
  DrawPlots()
  ZonePick(coord=(-5,5,0))
  p = GetPickAttributes()
  p.showTimeStep = 0
  p.showMeshName = 0
  p.showZoneId = 0
  SetPickAttributes(p)
  ZonePick(coord=(0,5,0))


SetDefaultPlotOptions
---------------------

**Synopsis:**

::

  SetDefaultPlotOptions(atts) -> integer


atts : plot attributes object
    Any type of plot attributes object.

return type : CLI_return_t
    All functions return an integer value of 1 for success and 0 for failure.


**Description:**

    Each plot in VisIt has a group of attributes that controls the appearance
    of the plot. To set the attributes for a plot, first create a plot
    attributes object. This is done by calling a function which is the name of
    the plot plus the word "Attributes". For example, a Pseudocolor plot's
    plotattributes object is created and returned by the PseudocolorAttributes
    function. Assign the new plot attributes object into a variable and set its
    fields. After setting the desired fields in the plot attributes object,
    pass the object to the SetPlotOptions function. The SetPlotOptions function
    determines the type of plot to which the plot attributes object applies and
    sets the attributes for that plot type. To set the default plot attributes,
    use the SetDefaultPlotOptions function. Setting the default attributes
    ensures that all future instances of a certain plot are initialized with
    the new default values.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  p = PseudocolorAttributes()
  p.colorTableName = "calewhite"
  p.minFlag,p.maxFlag = 1,1
  p.min,p.max = -5.0, 8.0
  SetPlotOptions(p)
  DrawPlots()


SetGlobalLineoutAttributes
--------------------------

**Synopsis:**

::

  SetGlobalLineoutAttributes(atts) -> integer


atts : GlobalLineoutAttributes object
    A GlobalLineoutAttributes object that contains the new settings.

return type : CLI_return_t
    The SetGlobalLineoutAttributes function returns 1 on success and 0 on
    failure.


**Description:**

    The SetGlobalLineoutAttributes function allows you to set global lineout
    options that are used in the creation of all lineouts. You can, for
    example, specify the destination window and the number of sample points for
    lineouts.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4)
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hgslice")
  DrawPlots()
  gla = GetGlobalLineoutAttributes()
  gla.createWindow = 0
  gla.windowId = 4
  gla.samplingOn = 1
  gla.numSamples = 150
  SetGlobalLineoutAttributes(gla)
  Lineout((-5,-8), (-3.5, 8))


SetInteractorAttributes
-----------------------

**Synopsis:**

::

  SetInteractorAttributes(atts) -> integer


atts : InteractorAttributes object
    An InteractorAttributes object that contains the new interactor attributes
    that you want to use.

return type : CLI_return_t
    SetInteractorAttributes returns 1 on success and 0 on failure.


**Description:**

    The SetInteractorAttributes function is used to set certain interactor
    properties. Interactors, can be thought of as how mouse clicks and
    movements are translated into actions in the vis window. To set the
    interactor attributes, first get the interactor attributes using the
    GetInteractorAttributes function. Once you've set the
    object's properties, call the SetInteractorAttributes function to make
    VisIt use the new interactor attributes.
    The SetDefaultInteractorAttributes function sets the default interactor
    attributes, which are used for new visualization windows. The default
    interactor attributes can also be saved to the VisIt configuration file to
    ensure that future VisIt sessions have the right default interactor
    attributes.


**Example:**

::

  #% visit -cli
  ia = GetInteractorAttributes()
  print(ia)
  ia.showGuidelines = 0
  SetInteractorAttributes(ia)


SetKeyframeAttributes
---------------------

**Synopsis:**

::

  SetKeyframeAttributes(kfAtts) -> integer


kfAtts : KeyframeAttributes object
    A KeyframeAttributes object that contains the new keyframing attributes
    to use.

return type : CLI_return_t
    SetKeyframeAttributes returns 1 on success and 0 on failure.


**Description:**

    Use the SetKeyframeAttributes function when you want to change VisIt's
    keyframing settings. You must pass a KeyframeAttributes object, which you
    can create using the GetKeyframeAttributes function. The
    KeyframeAttributes object must contain the keyframing settings that you
    want VisIt to use. For example, you would use the SetKeyframeAttributes
    function if you wanted to turn on keyframing mode and set the number of
    animation frames.


**Example:**

::

  #% visit -cli
  k = GetKeyframeAttributes()
  print(k)
  k.enabled,k.nFrames,k.nFramesWasUserSet = 1, 100, 1
  SetKeyframeAttributes(k)


SetLight
--------

**Synopsis:**

::

  SetLight(index, light) -> integer


index : integer
    A zero-based integer index into the light list. Index can be in the range [0,7].

light : LightAttributes object
    A LightAttributes object containing the properties to use for the specified light.

return type : CLI_return_t
    SetLight returns 1 on success and 0 on failure.


**Description:**

    The SetLight function sets the attributes for a specific light.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "w")
  p = PseudocolorAttributes()
  p.colorTableName = "xray"
  SetPlotOptions(p)
  DrawPlots()
  InvertBackgroundColor()
  light = GetLight(0)
  print(light)
  light.enabledFlag = 1
  light.direction = (0,-1,0)
  light.color = (255,0,0,255)
  SetLight(0, light)
  light.color,light.direction = (0,255,0,255), (-1,0,0)
  SetLight(1, light)


SetMachineProfile
-----------------

**Synopsis:**

::

  SetMachineProfile(MachineProfile) -> integer


MachineProfile : MachineProfile object
    A MachineProfile object containing the new settings.
    

**Description:**

    Sets the input machine profile in the HostProfileList, replaces if one already exists
    Otherwise adds to the list


SetMaterialAttributes
---------------------

**Synopsis:**

::

  SetMaterialAttributes(atts) -> integer


atts : MaterialAttributes object
    A MaterialAttributes object containing the new settings.

return type : CLI_return_t
    Both functions return 1 on success and 0 on failure.


**Description:**

    The SetMaterialAttributes function takes a MaterialAttributes object and
    makes VisIt use the material settings that it contains. You use the
    SetMaterialAttributes function when you want to change how VisIt performs
    material interface reconstruction. The SetDefaultMaterialAttributes
    function sets the default material attributes, which are saved to the
    config file and are also used by new visualization windows.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/allinone00.pdb")
  AddPlot("Pseudocolor", "mesh/mixvar")
  p = PseudocolorAttributes()
  p.min,p.minFlag = 4.0, 1
  p.max,p.maxFlag = 13.0, 1
  SetPlotOptions(p)
  DrawPlots()
  # Tell VisIt to always do material interface reconstruction.
  m = GetMaterialAttributes()
  m.forceMIR = 1
  SetMaterialAttributes(m)
  ClearWindow()
  # Redraw the plot forcing VisIt to use the mixed variable information.
  DrawPlots()


SetMeshManagementAttributes
---------------------------

**Synopsis:**

::

  GetMeshManagementAttributes() -> MeshmanagementAttributes object

return type : MeshmanagementAttributes object
    Returns a MeshmanagementAttributes object.


**Description:**

    The GetMeshmanagementAttributes function returns a MeshmanagementAttributes object
    that contains VisIt's current mesh discretization settings.
    You can set properties on the MeshManagementAttributes object and then pass it to
    SetMeshManagementAttributes to make VisIt use the new material attributes that
    you've specified:


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/csg.silo")
  AddPlot("Mesh", "csgmesh")
  DrawPlots()
  # Tell VisIt to always do material interface reconstruction.
  mma = GetMeshManagementAttributes()
  mma.discretizationTolernace = (0.01, 0.025)
  SetMeshManagementAttributes(mma)
  ClearWindow()
  # Redraw the plot forcing VisIt to use the mixed variable information.
  DrawPlots()


SetNamedSelectionAutoApply
--------------------------

**Synopsis:**

::

  SetNamedSelectionAutoApply(flag) -> integer


flag : integer
    An integer flag. Non-zero values turn on selection auto apply mode.

return type : CLI_return_t
    The SetNamedSelectionAutoApply function returns 1 on success and 0 on failure.


**Description:**

    Named selections are often associated with plots for their data source. When
    those plots update, their named selections can be updated, which in turn will
    update any plots that use the named selection. When this mode is enabled, changes
    to a named selection's originating plot will cause the selection to be updated
    automatically.


**Example:**

::

  SetNamedSelectionAutoApply(1)


SetOperatorOptions
------------------

**Synopsis:**

::

  SetOperatorOptions(atts) -> integer
  SetOperatorOptions(atts, operatorIndex) -> integer
  SetOperatorOptions(atts, operatorIndex, all) -> integer


atts : operator attributes object
    Any type of operator attributes object.

operatorIndex : integer
    An optional zero-based integer that serves as an index into the active
    plot's operator list. Use this argument if you want to set the operator
    attributes for a plot that has multiple instances of the same type of
    operator. For example, if the active plot had a Transform operator
    followed by a Slice operator followed by another Transform operator and
    you wanted to adjust the attributes of the second Transform operator,
    you would pass an operatorIndex value of 2.

all : integer
    An optional integer argument that tells the function to apply theoperator
    attributes to all plots containing the specified operator if the value of
    the argument is non-zero.

return type : CLI_return_t
    All functions return an integer value of 1 for success and 0 for failure.


**Description:**

    Each operator in VisIt has a group of attributes that controls the
    operator. To set the attributes for an operator, first create an operator
    attributes object. This is done by calling a function which is the name of
    the operator plus the word "Attributes". For example, a Slice operator's
    operator attributes object is created and returned by the SliceAttributes
    function. Assign the new operator attributes object into a variable and set
    its fields. After setting the desired fields in the operator attributes
    object, pass the object to the SetOperatorOptions function. The
    SetOperatorOptions function determines the type of operator to which the
    operator attributes object applies and sets the attributes for that
    operator type. To set the default plot attributes, use the
    SetDefaultOperatorOptions function. Setting the default attributes ensures
    that all future instances of a certain operator are initialized with the
    new default values. Note that there is no SetOperatorOptions(atts, all)
    variant of this call. To set operator options for all plots that have a
    an instance of the associated operator, you must first make all plots
    active with SetActivePlots() and then use the SetOperatorOptions(atts)
    variant.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  AddPlot("Mesh", "mesh1")
  AddOperator("Slice", 1) # Add the operator to both plots
  a = SliceAttributes()
  a.normal, a.upAxis = (0,0,1), (0,1,0)
  # Only set the attributes for the active plot.
  SetOperatorOptions(a)
  DrawPlots()


SetPickAttributes
-----------------

**Synopsis:**

::

  SetPickAttributes(atts) -> integer


atts : PickAttributes object
    A PickAttributes object containing the new pick settings.

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The SetPickAttributes function changes the pick attributes that are used
    when VisIt picks on plots. The pick attributes allow you to format your
    pick output in various ways and also allows you to select auxiliary pick
    variables.


**Example:**

::

  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hgslice")
  DrawPlots()
  ZonePick(coord=(-5,5,0))
  p = GetPickAttributes()
  p.showTimeStep = 0
  p.showMeshName = 0
  p.showZoneId = 0
  SetPickAttributes(p)
  ZonePick(coord=(0,5,0))


SetPipelineCachingMode
----------------------

**Synopsis:**

::

  SetPipelineCachingMode(mode) -> integer

return type : CLI_return_t
    The SetPipelineCachingMode function returns 1 for success and 0 for
    failure.


**Description:**

    The SetPipelineCachingMode function turns pipeline caching on or off in
    the viewer. When pipeline caching is enabled, animation timesteps are
    cached for fast playback. This can be a disadvantage for large databases or
    for plots with many timesteps because it increases memory consumption. In
    those cases, it is often useful to disable pipeline caching so the viewer
    does not use as much memory. When the viewer does not cache pipelines, each
    plot for a timestep must be recalculated each time the timestep is visited.


**Example:**

::

  #% visit -cli
  SetPipelineCachingMode(0) # Disable caching
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  AddPlot("Mesh", "quadmesh")
  DrawPlots()
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)


SetPlotDatabaseState
--------------------

**Synopsis:**

::

  SetPlotDatabaseState(index, frame, state)


index : integer
    A zero-based integer index that is the plot's location in the plot list.

frame : integer
    A zero-based integer index representing the animation frame for which we're
    going to add a database keyframe.

state : integer
    A zero-based integer index representing the database time state that we're
    going to use at the specified animation frame.


**Description:**

    The SetPlotDatabaseState function is used when VisIt is in keyframing mode
    to add a database keyframe for a specific plot. VisIt uses database
    keyframes to determine which database state is to be used for a given
    animation frame. Database keyframes can be used to stop "database time"
    while "animation time" continues forward and they can also be used to
    make "database time" go in reverse, etc.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  k = GetKeyframeAttributes()
  nFrames = 20
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1, nFrames, 1
  SetKeyframeAttributes(k)
  AddPlot("Pseudocolor", "pressure")
  AddPlot("Mesh", "quadmesh")
  DrawPlots()
  # Make "database time" for the Pseudocolor plot go in reverse
  SetPlotDatabaseState(0, 0, 70)
  SetPlotDatabaseState(0, nFrames-1, 0)
  # Animate through the animation frames since the "Keyframe animation"
  # time slider is active.
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)


SetPlotDescription
------------------

**Synopsis:**

::

  SetPlotDescription(index, description) -> integer


index : integer
    The integer index of the plot within the plot list.

description : list
    A new description srting that will be shown in the plot list so the plot
    can be identified readily.

return type : CLI_return_t
    The SetPlotDescription function returns 1 on success and 0 on failure.


**Description:**

    Managing many related plots can be a complex task. This function lets users
    provide meaningful descriptions for each plot so they can more easily be
    identified in the plot list.


**Example:**

::

  SetPlotDescription(0, 'Mesh for reflected pressure plot')


SetPlotFollowsTime
------------------

**Synopsis:**

::

  SetPlotFollowsTime(val) -> integer


val : integer
    An optional integer flag indicating whether the plot should follow the
    time slider. The default behavior is for the plot to follow the time slider.

return type : CLI_return_t
    The function returns 1 on success and 0 on failure.


**Description:**

    SetPlotFollowsTime can let you set whether the active plot follows the time slider.


**Example:**

::

  SetPlotFollowsTime()


SetPlotFrameRange
-----------------

**Synopsis:**

::

  SetPlotFrameRange(index, start, end)


index : integer
    A zero-based integer representing an index into the plot list.

start : integer
    A zero-based integer representing the animation frame where the plot
    first appears in the visualization.

end : integer
    A zero-based integer representing the animation frame where the plot
    disappears from the visualization.


**Description:**

    The SetPlotFrameRange function sets the start and end frames for a plot
    when VisIt is in keyframing mode. Outside of this frame range, the plot
    does not appear in the visualization. By default, plots are valid over the
    entire range of animation frames when they are first created. Frame ranges
    allow you to construct complex animations where plots appear and disappear
    dynamically.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  k = GetKeyframeAttributes()
  nFrames = 20
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1, nFrames, 1
  SetKeyframeAttributes(k)
  AddPlot("Pseudocolor", "pressure")
  AddPlot("Mesh", "quadmesh")
  DrawPlots()
  # Make the Pseudocolor plot take up the first half of the animation frames
  # before it disappears.
  SetPlotFrameRange(0, 0, nFrames/2-1)
  # Make the Mesh plot take up the second half of the animation frames.
  SetPlotFrameRange(1, nFrames/2, nFrames-1)
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()


SetPlotOptions
--------------

**Synopsis:**

::

  SetPlotOptions(atts) -> integer


atts : plot attributes object
    Any type of plot attributes object.

return type : CLI_return_t
    All functions return an integer value of 1 for success and 0 for failure.


**Description:**

    Each plot in VisIt has a group of attributes that controls the appearance
    of the plot. To set the attributes for a plot, first create a plot
    attributes object. This is done by calling a function which is the name of
    the plot plus the word "Attributes". For example, a Pseudocolor plot's
    plotattributes object is created and returned by the PseudocolorAttributes
    function. Assign the new plot attributes object into a variable and set its
    fields. After setting the desired fields in the plot attributes object,
    pass the object to the SetPlotOptions function. The SetPlotOptions function
    determines the type of plot to which the plot attributes object applies and
    sets the attributes for that plot type. To set the default plot attributes,
    use the SetDefaultPlotOptions function. Setting the default attributes
    ensures that all future instances of a certain plot are initialized with
    the new default values.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  p = PseudocolorAttributes()
  p.colorTableName = "calewhite"
  p.minFlag,p.maxFlag = 1,1
  p.min,p.max = -5.0, 8.0
  SetPlotOptions(p)
  DrawPlots()


SetPlotOrderToFirst
-------------------

**Synopsis:**

::

  SetPlotOrderToFirst(index) -> integer


index : integer
    The integer index of the plot within the plot list.

return type : CLI_return_t
    The SetPlotOrderToFirst function returns 1 on success and 0 on failure.


**Description:**

    Move the i'th plot in the plot list to the start of the plot list.


**Example:**

::

  AddPlot('Mesh', 'mesh')
  AddPlot('Pseudocolor', 'pressure')
  # Make the Pseudocolor plot first in the plot list
  SetPlotOrderToFirst(1)


SetPlotOrderToLast
------------------

**Synopsis:**

::

  SetPlotOrderToLast(index) -> integer


index : integer
    The integer index of the plot within the plot list.

return type : CLI_return_t
    The SetPlotOrderToLast function returns 1 on success and 0 on failure.


**Description:**

    Move the i'th plot in the plot list to the end of the plot list.


**Example:**

::

  AddPlot('Mesh', 'mesh')
  AddPlot('Pseudocolor', 'pressure')
  # Make the Mesh plot last in the plot list
  SetPlotOrderToLast(0)


SetPlotSILRestriction
---------------------

**Synopsis:**

::

  SetPlotSILRestriction(silr) -> integer
  SetPlotSILRestriction(silr, all) -> integer


silr : SIL restriction object
    A SIL restriction object.

all
    An optional argument that tells the function if the SIL restriction
    should be applied to all plots in the plot list.

return type : CLI_return_t
    The SetPlotSILRestriction function returns an integer value of 1 for
    success and 0 for failure.


**Description:**

    VisIt allows the user to select subsets of databases. The description of
    the subset is called a Subset Inclusion Lattice Restriction, or SIL
    restriction. The SIL restriction allows databases to be subselected in
    several different ways. The VisIt Python Interface provides the
    SetPlotSILRestriction function to allow Python scripts to turn off portions
    of the plotted database. The SetPlotSILRestriction function accepts a
    SILRestriction object that contains the SIL restriction for the active
    plots. The optional all argument is an integer that tells the function to
    apply the SIL restriction to all plots when the value of the argument is
    non-zero. If the all argument is not supplied, then the SIL restriction is
    only applied to the active plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/multi_curv2d.silo")
  AddPlot("Subset", "mat1")
  silr = SILRestriction()
  silr.TurnOffSet(silr.SetsInCategory('mat1')[1])
  SetPlotSILRestriction(silr)
  DrawPlots()


SetPrecisionType
----------------

**Synopsis:**

::

  SetPrecisionType(typeAsInt)
  SetPrecisionType(typeAsString)


typeAsInt : double
    Precision type specified as an integer. 0 = float 1 = native 2 = double

typeAsString : string
    Precision type specified as a string. Options are 'float', 'native',
    and 'double'.


**Description:**

    The SetPrecisionType function sets the floating point pecision
    used by VisIt's pipeline.  The function accepts a single argument
    either an integer or string representing the precision desired.
    0 = "float", 1 = "native", 2 = "double"


**Example:**

::

  SetPrecisionType("double")
  SetPrecisionType(2)


SetPreferredFileFormats
-----------------------

**Synopsis:**

::

  SetPreferredFileFormats(pluginIDs) -> integer


pluginIDs : tuple
    A tuple of plugin IDs to be attempted first when opening files.

return type : CLI_return_t
    The SetPreferredFileFormats method does not return a value.


**Description:**

    The SetPreferredFileFormats method is a way to set the list of
    file format reader plugins which are tried before any others.
    These IDs must be full IDs, not just names, and are tried in order.


**Example:**

::

  SetPreferredFileFormats('Silo_1.0')
  SetPreferredFileFormats(('Silo_1.0','PDB_1.0'))


SetPrinterAttributes
--------------------

**Synopsis:**

::

  SetPrinterAttributes(atts)


atts : PrinterAttributes object
    A PrinterAttributes object.


**Description:**

    The SetPrinterAttributes function sets the printer attributes. VisIt uses
    the printer attributes to determine how the active visualization window
    should be printed. The function accepts a single argument which is a
    PrinterAttributes object containing the printer attributes to use for
    future printing. VisIt allows images to be printed to a network printer or
    to a PostScript file that can be printed later by other applications.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Surface", "v")
  DrawPlots()
  # Make it print to a file.
  p = PrinterAttributes()
  p.outputToFile = 1
  p.outputToFileName = "printfile"
  SetPrinterAttributes(p)
  PrintWindow()


SetQueryFloatFormat
-------------------

**Synopsis:**

::

  SetQueryFloatFormat(format_string)


format_string : string
    A string object that provides a printf style floating point format.


**Description:**

    The SetQueryFloatFormat method sets a printf style format string that
    isused by VisIt's querys to produce textual output.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set floating point format string.
  SetQueryFloatFormat("%.1f")
  Query("MinMax")
  # Set format back to default "%g".
  SetQueryFloatFormat("%g")
  Query("MinMax")


SetQueryOutputToObject
----------------------

**Synopsis:**

::

  SetQueryOutputToObject()


**Description:**

    SetQueryOutputToObject changes the return type of future Queries to the
    'object' or Python dictionary form. This is the same object that would be
    returned by calling 'GetQueryOutputObject()' after a Query call. All other
    output modes are still available after the Query call
    (eg GetQueryOutputValue(),GetQueryOutputObject(), GetQueryOutputString()).


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set query output type.
  SetQueryOutputToObject()
  query_output = Query("MinMax")
  print(query_output)


SetQueryOutputToString
----------------------

**Synopsis:**

::

  SetQueryOutputToString()


**Description:**

    SetQueryOutputToString changes the return type of future Queries to the
    'string' form. This is the same as what would be returned by calling
    'GetQueryOutputString' after a Query call. All other output modes are
    still available after the Query call (eg GetQueryOutputValue(),
    GetQueryOutputObject(), GetQueryOutputString()).


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set query output type.
  SetQueryOutputToString()
  query_output = Query("MinMax")
  print(query_output)
  '''
  d -- Min = 0.0235702 (zone 434 at coord <0.483333, 0.483333>)
  d -- Max = 0.948976 (zone 1170 at coord <0.0166667, 1.31667>)
  '''


SetQueryOutputToValue
---------------------

**Synopsis:**

::

  SetQueryOutputToValue()


**Description:**

    SetQueryOutputToValue changes the return type of future Queries to the
    'value' form. This is the same as what would be returned by calling
    'GetQueryOutputValue()' after a Query call. All other output modes are
    still available after the Query call (eg GetQueryOutputValue(),
    GetQueryOutputObject(), GetQueryOutputString()).


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set query output type.
  SetQueryOutputToValue()
  query_output = Query("MinMax")
  print(query_output)
  (0.02357020415365696, 0.9489759802818298)


SetQueryOverTimeAttributes
--------------------------

**Synopsis:**

::

  SetQueryOverTimeAttributes(atts) -> integer


atts : QueryOverTimeAttributes object
    A QueryOverTimeAttributes object containing the new settings to use for
    queries over time.

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The SetQueryOverTimeAttributes function changes the settings that VisIt
    uses for query over time. The SetDefaultQueryOverTimeAttributes function
    changes the settings that new visualization windows inherit for doing query
    over time. Finally, the ResetQueryOverTimeAttributes function forces VisIt
    to use the stored default query over time attributes instead of the
    previous settings.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4)
  OpenDatabase("/usr/gapps/visit/data/allinone00.pdb")
  AddPlot("Pseudocolor", "mesh/mixvar")
  DrawPlots()
  qot = GetQueryOverTimeAttributes()
  # Make queries over time go to window 4.
  qot.createWindow,q.windowId = 0, 4
  SetQueryOverTimeAttributes(qot)
  QueryOverTime("Min")
  # Make queries over time only use half of the number of time states.
  qot.endTimeFlag,qot.endTime = 1, GetDatabaseNStates() / 2
  SetQueryOverTimeAttributes(qot)
  QueryOverTime("Min")
  ResetView()


SetRemoveDuplicateNodes
-----------------------

**Synopsis:**

::

  SetRemoveDuplicateNodes(val) -> integer


val : integer
    Either a zero (false) or non-zero (true) integer value to indicate if
    duplicate nodes in fully disconnected unstructured grids should be
    automatically removed by visit.

return type : CLI_return_t
    The SetRemoveDuplicateNodes function returns 1 on success and 0 on failure.


**Description:**

    The SetRemoveDuplicateNodes function sets a boolean in the
    global attributes indicating whether or not duplicate nodes in
    fully disconnected unstructured grids should be automatically removed.
    The default behavior is for the original grid to be left as read, which
    may slow down VisIt's performance for extraordinary large meshes.
    Turning this feature off tells VisIt to remove the duplicate nodes after
    the mesh is read, but before further processing in VisIt.


**Example:**

::

  #% visit -cli
  SetRemoveDuplicateNodes(1) # turn this feature on
  SetRemoveDuplicateNodes(0) # turn this feature off


SetRenderingAttributes
----------------------

**Synopsis:**

::

  SetRenderingAttributes(atts) -> integer


atts : RenderingAttributes object
    A RenderingAttributes object that contains the rendering attributes that
    we want to make VisIt use.

return type : CLI_return_t
    The SetRenderingAttributes function returns 1 on success and 0 on failure.


**Description:**

    The SetRenderingAttributes makes VisIt use the rendering attributes stored
    in the specified RenderingAttributes object. The RenderingAttributes object
    stores rendering attributes such as: scalable rendering options, shadows,
    specular highlights, display lists, etc.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Surface", "hgslice")
  DrawPlots()
  v = GetView2D()
  v.viewNormal = (-0.215934, -0.454611, 0.864119)
  v.viewUp = (0.973938, -0.163188, 0.157523)
  v.imageZoom = 1.64765
  SetView3D(v)
  light = GetLight(0)
  light.direction = (0,1,-1)
  SetLight(0, light)
  r = GetRenderingAttributes()
  print(r)
  r.scalableActivationMode = r.Always
  r.doShadowing = 1
  SetRenderingAttributes(r)


SetSaveWindowAttributes
-----------------------

**Synopsis:**

::

  SetSaveWindowAttributes(atts)


atts : SaveWindowAttributes object
    A SaveWindowAttributes object.


**Description:**

    The SetSaveWindowAttributes function sets the format and filename that are
    used to save windows when the SaveWindow function is called. The contents
    of the active visualization window can be saved as TIFF, JPEG, RGB, PPM,
    PNG images or they can be saved as curve, Alias Wavefront Obj, or VTK
    geometry files. To set the SaveWindowAttributes, create a
    SaveWindowAttributes object using the SaveWindowAttributes function and
    assign it into a variable. Set the fields in the object and pass it to the
    SetSaveWindowAttributes function.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv3d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Set the save window attributes
  s = SaveWindowAttributes()
  s.fileName = "test"
  s.format = s.JPEG
  s.progressive = 1
  s.fileName = "test"
  SetSaveWindowAttributes(s)
  # Save the window
  SaveWindow()


SetTimeSliderState
------------------

**Synopsis:**

::

  SetTimeSliderState(state) -> integer


state : integer
    A zero-based integer containing the time state that we want to make active.

return type : CLI_return_t
    The SetTimeSliderState function returns 1 on success and 0 on failure.


**Description:**

    The SetTimeSliderState function sets the time state for the active time
    slider. This is the function to use if you want to animate through time or
    change the current keyframe frame.


**Example:**

::

  #% visit -cli
  path = "/usr/gapps/visit/data/"
  dbs = (path + "dbA00.pdb", path + "dbB00.pdb", path + "dbC00.pdb")
  for db in dbs:
      OpenDatabase(db)
      AddPlot("FilledBoundary", "material(mesh)")
      DrawPlots()
  CreateDatabaseCorrelation("common", dbs, 1)
  tsNames = GetWindowInformation().timeSliders
  for ts in tsNames:
      SetActiveTimeSlider(ts)
  for state in list(range(TimeSliderGetNStates())) + [0]:
      SetTimeSliderState(state)


SetTreatAllDBsAsTimeVarying
---------------------------

**Synopsis:**

::

  SetTreatAllDBsAsTimeVarying(val) -> integer


val : integer
    Either a zero (false) or non-zero (true) integer value to indicate if
    all databases should be treated as time varying (true) or not (false).

return type : CLI_return_t
    The SetTreatAllDBsAsTimeVarying function returns 1 on success and 0 on failure.


**Description:**

    The SetTreatAllDBsAsTimeVarying function sets a boolean in the global attributes
    indicating if all databases should be treated as time varying or not. Ordinarily,
    VisIt tries to minimize file I/O and database interaction by avoiding re-reading
    metadata that is 'time-invariant' and, therefore, assumed to be the same in a
    database from one time step to the next. However, sometimes, portions of the metadata,
    such as the list of variable names and/or number of domains, does in fact vary. In this
    case, VisIt can actually fail to acknowledge the existence of new variables in the file.
    Turning this feature on forces VisIt to re-read metadata each time the time-state is
    changed.


**Example:**

::

  #% visit -cli
  SetTreatAllDBsAsTimeVarying(1) # turn this feature on
  SetTreatAllDBsAsTimeVarying(0) # turn this feature off


SetTryHarderCyclesTimes
-----------------------

**Synopsis:**

::

  SetTryHarderCyclesTimes(val) -> integer


val : integer
    Either a zero (false) or non-zero (true) integer value to indicate if
    VisIt read cycle/time information for all timestates when opening a
    database.

return type : CLI_return_t
    The SetTryHarderCyclesTimes function returns 1 on success and 0 on failure.


**Description:**

    For certain classes of databases, obtaining cycle/time information for all time
    states in the database is an expensive operation, requiring each file to be opened
    and queried. The cost of the operation gets worse the more time states there are
    in the database. Ordinarily, VisIt does not bother to query each time state for
    precise cycle/time information. In fact, often VisIt can guess this information
    from the filename(s) comprising the databse. However, turning this feature on
    will force VisIt to obtain accurate cycle/time information for all time states
    by opening and querying all file(s) in the database.


**Example:**

::

  #% visit -cli
  SetTryHarderCyclesTimes(1) # Turn this feature on
  SetTryHarderCyclesTimes(0) # Turn this feature off


SetUltraScript
--------------

**Synopsis:**

::

  SetUltraScript(filename) -> integer


filename : string
    The name of the file to be used as the ultra script when LoadUltra is called.

return type : CLI_return_t
    The SetUltraScript function returns 1.


**Description:**

    Set the path to the script to be used by the LoadUltra command. Normal users do
    not need to use this function.


SetView2D
---------

**Synopsis:**

::

  SetView2D(View2DAttributes) -> integer


view : ViewAttributes object
    A ViewAttributes object containing the view.

return type : CLI_return_t
    All functions returns 1 on success and 0 on failure.


**Description:**

    The view is a crucial part of a visualization since it determines
    which parts of the database are examined. The VisIt Python Interface
    provides four functions for setting the view: SetView2D, SetView3D,
    SetViewCurve, and SetViewAxisArray. If the visualization window
    contains 2D plots, use the SetView2D function. To set the view, first
    create the appropriate ViewAttributes object and set the object's
    fields to set a new view. After setting the fields, pass the object
    to the matching SetView function. A common use of the SetView
    functions is to animate the view to produce simple animations where
    the camera appears to fly around the plots in the visualization
    window.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "v")
  DrawPlots()
  va = GetView3D()
  va.RotateAxis(1,30.0) # rotate around the y axis 30 degrees.
  SetView3D(va)
  v0 = GetView3D()
  v1 = GetView3D()
  v1.camera,v1.viewUp = (1,1,1),(-1,1,-1)
  v1.parallelScale = 10.
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v0 + t * v1
      SetView3D(v2) # Animate the view.


SetView3D
---------

**Synopsis:**

::

  SetView3D(View3DAttributes) -> integer


view : ViewAttributes object
    A ViewAttributes object containing the view.

return type : CLI_return_t
    All functions returns 1 on success and 0 on failure.


**Description:**

    The view is a crucial part of a visualization since it determines
    which parts of the database are examined. The VisIt Python Interface
    provides four functions for setting the view: SetView2D, SetView3D,
    SetViewCurve, and SetViewAxisArray. Use the SetView3D function when
    the visualization window contains 3D plots. To set the view, first
    create the appropriate ViewAttributes object and set the object's
    fields to set a new view. After setting the fields, pass the object
    to the matching SetView function. A common use of the SetView
    functions is to animate the view to produce simple animations where
    the camera appears to fly around the plots in the visualization
    window. A View3D object also supports the RotateAxis(int axis, double deg)
    method which mimics the 'rotx', 'roty' and 'rotz' view commands in
    the GUI.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "v")
  DrawPlots()
  va = GetView3D()
  va.RotateAxis(1,30.0) # rotate around the y axis 30 degrees.
  SetView3D(va)
  v0 = GetView3D()
  v1 = GetView3D()
  v1.camera,v1.viewUp = (1,1,1),(-1,1,-1)
  v1.parallelScale = 10.
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v0 + t * v1
      SetView3D(v2) # Animate the view.


SetViewAxisArray
----------------

**Synopsis:**

::

  SetViewAxisArray(ViewAxisArrayAttributes) -> integer


view : ViewAttributes object
    A ViewAttributes object containing the view.

return type : CLI_return_t
    All functions returns 1 on success and 0 on failure.


**Description:**

    The view is a crucial part of a visualization since it determines
    which parts of the database are examined. The VisIt Python Interface
    provides four functions for setting the view: SetView2D, SetView3D,
    SetViewCurve, and SetViewAxisArray. To set the view, first create
    the appropriate ViewAttributes object and set the object's fields
    to set a new view. After setting the fields, pass the object to the
    matching SetView function. A common use of the SetView functions is
    to animate the view to produce simple animations where the camera
    appears to fly around the plots in the visualization window.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "v")
  DrawPlots()
  va = GetView3D()
  va.RotateAxis(1,30.0) # rotate around the y axis 30 degrees.
  SetView3D(va)
  v0 = GetView3D()
  v1 = GetView3D()
  v1.camera,v1.viewUp = (1,1,1),(-1,1,-1)
  v1.parallelScale = 10.
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v0 + t * v1
      SetView3D(v2) # Animate the view.


SetViewCurve
------------

**Synopsis:**

::

  SetViewCurve(ViewCurveAttributes) -> integer


view : ViewAttributes object
    A ViewAttributes object containing the view.

return type : CLI_return_t
    All functions returns 1 on success and 0 on failure.


**Description:**

    The view is a crucial part of a visualization since it determines
    which parts of the database are examined. The VisIt Python Interface
    provides four functions for setting the view: SetView2D, SetView3D,
    SetViewCurve, and SetViewAxisArray. To set the view, first create
    the appropriate ViewAttributes object and set the object's fields
    to set a new view. After setting the fields, pass the object to the
    matching SetView function. A common use of the SetView functions is
    to animate the view to produce simple animations where the camera
    appears to fly around the plots in the visualization window.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "v")
  DrawPlots()
  va = GetView3D()
  va.RotateAxis(1,30.0) # rotate around the y axis 30 degrees.
  SetView3D(va)
  v0 = GetView3D()
  v1 = GetView3D()
  v1.camera,v1.viewUp = (1,1,1),(-1,1,-1)
  v1.parallelScale = 10.
  for i in range(0,20):
      t = float(i) / 19.
      v2 = (1. - t) * v0 + t * v1
      SetView3D(v2) # Animate the view.


SetViewExtentsType
------------------

**Synopsis:**

::

  SetViewExtentsType(type) -> integer


type : integer
    An integer or a string. Options are 0, 1 and 'original', 'actual', respectively.

return type : CLI_return_t
    SetViewExtentsType returns 1 on success and 0 on failure.


**Description:**

    VisIt can use a plot's spatial extents in two ways when computing the
    view. The first way of using the extents is to use the "original"
    extents, which are the spatial extents before any modifications, such as
    subset selection, have been made to the plot. This ensures that the view
    will remain relatively constant for a plot. Alternatively, you can use the
    "actual" extents, which are the spatial extents of the pieces of the plot
    that remain after operations such as subset selection.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  SetViewExtentsType("actual")
  AddPlot("FilledBoundary", "mat1")
  DrawPlots()
  v = GetView3D()
  v.viewNormal = (-0.618945, 0.450655, 0.643286)
  v.viewUp = (0.276106, 0.891586, -0.358943)
  SetView3D(v)
  mats = GetMaterials()
  nmats = len(mats)
  # Turn off all but the last material in sequence and watch
  # the view update each time.
  for i in range(nmats-1):
      index = nmats-1-i
      TurnMaterialsOff(mats[index])
      SaveWindow()
      SetViewExtentsType("original")


SetViewKeyframe
---------------

**Synopsis:**

::

  SetViewKeyframe() -> integer

return type : CLI_return_t
    The SetViewKeyframe function returns 1 on success and 0 on failure.


**Description:**

    The SetViewKeyframe function adds a view keyframe when VisIt is in
    keyframing mode. View keyframes are used to set the view at crucial points
    during an animation. Frames that lie between view keyframes have an
    interpolated view that is based on the view keyframes. You can use the
    SetViewKeyframe function to create complex camera animations that allow you
    to fly around (or through) your visualization.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Contour", "hardyglobal")
  DrawPlots()
  k = GetKeyframeAttributes()
  nFrames = 20
  k.enabled, k.nFrames, k.nFramesWasUserSet = 1, nFrames, 1
  SetKeyframeAttributes(k)
  SetPlotFrameRange(0, 0, nFrames-1)
  SetViewKeyframe()
  SetTimeSliderState(10)
  v = GetView3D()
  v.viewNormal = (-0.721721, 0.40829, 0.558944)
  v.viewUp = (0.294696, 0.911913, -0.285604)
  SetView3D(v)
  SetViewKeyframe()
  SetTimeSliderState(nFrames-1)
  v.viewNormal = (-0.74872, 0.423588, -0.509894)
  v.viewUp = (0.369095, 0.905328, 0.210117)
  SetView3D()
  SetViewKeyframe()
  ToggleCameraViewMode()
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()


SetWindowArea
-------------

**Synopsis:**

::

  SetWindowArea(x, y, width, height) -> integer


x : integer
    An integer that is the left X coordinate in screen pixels.

y : integer
    An integer that is the top Y coordinate in screen pixels.

width : integer
    An integer that is the width of the window area in pixels.

height : integer
    An integer that is the height of the window area in pixels.

return type : CLI_return_t
    The SetWindowArea function returns 1 on success and 0 on failure.


**Description:**

    The SetWindowArea method sets the area of the screen that can be used by
    VisIt's visualization windows. This is useful for making sure windows are a
    certain size when running a Python script.


**Example:**

::

  import visit
  visit.Launch()
  visit.SetWindowArea(0, 0, 600, 600)
  visit.SetWindowLayout(4)


SetWindowLayout
---------------

**Synopsis:**

::

  SetWindowLayout(layout) -> integer


layout : integer
    An integer that specifies the window layout. (1,2,4,8,9,16 are valid)

return type : CLI_return_t
    The SetWindowLayout function returns an integer value of 1 for success and
    0 for failure.


**Description:**

    VisIt's visualization windows can be arranged in various tiled patterns
    that allow VisIt to make good use of the screen while displaying several
    visualization windows. The window layout determines how windows are shown
    on the screen. The SetWindowLayout function sets the window layout. The
    layout argument is an integer value equal to 1,2,4,8,9, or 16.


**Example:**

::

  #% visit -cli
  SetWindowLayout(2) # switch to 1x2 layout
  SetWindowLayout(4) # switch to 2x2 layout
  SetWindowLayout(8) # switch to 2x4 layout


SetWindowMode
-------------

**Synopsis:**

::

  SetWindowMode(mode) -> integer


mode : string
    A string containing the new mode. Options are 'navigate', 'zoom',
    'lineout', 'pick', 'zone pick', 'node pick', 'spreadsheet pick'.

return type : CLI_return_t
    The SetWindowMode function returns 1 on success and 0 on failure.


**Description:**

    VisIt's visualization windows have various window modes that alter their
    behavior. Most of the time a visualization window is in "navigate" mode
    which changes the view when the mouse is moved in the window. The "zoom"
    mode allows a zoom rectangle to be drawn in the window for changing the
    view. The "pick" mode retrieves information about the plots when the
    mouse is clicked in the window. The "lineout" mode allows the user to
    draw lines which produce curve plots.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  SetWindowMode("zoom")
  # Draw a rectangle in the visualization window to zoom the plots


ShowAllWindows
--------------

**Synopsis:**

::

  ShowAllWindows() -> integer

return type : CLI_return_t
    The ShowAllWindows function returns 1 on success and 0 on failure.


**Description:**

    The ShowAllWindows function tells VisIt's viewer to show all of its
    visualization windows. The command line interface calls ShowAllWindows
    before giving control to any user-supplied script to ensure that the
    visualization windows appear as expected. Call the ShowAllWindows function
    when using the VisIt module inside another Python interpreter so the
    visualization windows are made visible.


**Example:**

::

  #% python
  import visit
  visit.Launch()
  visit.ShowAllWindows()


ShowToolbars
------------

**Synopsis:**

::

  ShowToolbars() -> integer
  ShowToolbars(allWindows) -> integer


allWindows : integer
    An integer value that tells VisIt to show the toolbars for all windows
    when it is non-zero.

return type : CLI_return_t
    The ShowToolbars function returns 1 on success and 0 on failure.


**Description:**

    The ShowToolbars function tells VisIt to show the toolbars for the active
    visualization window or for all visualization windows when the optional
    allWindows argument is provided and is set to a non-zero value.


**Example:**

::

  #% visit -cli
  SetWindowLayout(4)
  HideToolbars(1)
  ShowToolbars()
  # Show the toolbars for all windows.
  ShowToolbars(1)


Source
------

**Synopsis:**

::

  Source(filename)


**Description:**

    The Source function reads in the contents of a text file and interprets it
    with the Python interpreter. This is a simple mechanism that allows simple
    scripts to be included in larger scripts. The Source function takes a
    single string argument that contains the name of the script to execute.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # include another script that does some animation.
  Source("Animate.py")


SuppressMessages
----------------

**Synopsis:**

::

  SuppressMessages(level) -> integer


level : integer
    An integer value of 1,2,3 or 4

return type : CLI_return_t
    The SuppressMessages function returns the previous suppression level on
    success and 0 on failure.


**Description:**

    The SuppressMessage function sets the supression level for status messages
    generated by VisIt.  A value of 1 suppresses all types of messages. A value
    of 2 suppresses Warnings and Messages but does NOT suppress Errors.
    A value of 3 suppresses Messages but does not suppress Warnings or Errors.
    A value of 4 does not suppress any messages. The default setting is 4.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Turn off Warning and Message messages.
  SuppressMessages(2)
  SaveWindow()


SuppressQueryOutputOff
----------------------

**Synopsis:**

::

  SuppressQueryOutputOff() -> integer

return type : CLI_return_t
    The SuppressQueryOutput function returns 1 on success and 0 on failure.


**Description:**

    The SuppressQueryOutput function tells VisIt to turn on/off the automatic
    printing of query output.  Query output will still be available via
    GetQueryOutputString and GetQueryOutputValue.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Turn off automatic printing of Query output.
  SuppressQueryOutputOn()
  Query("MinMax")
  print("The min is: %g and the max is: %g" % GetQueryOutputValue())
  # Turn on automatic printing of Query output.
  SuppressQueryOutputOff()
  Query("MinMax")


SuppressQueryOutputOn
---------------------

**Synopsis:**

::

  SuppressQueryOutputOn() -> integer

return type : CLI_return_t
    The SuppressQueryOutput function returns 1 on success and 0 on failure.


**Description:**

    The SuppressQueryOutput function tells VisIt to turn on/off the automatic
    printing of query output.  Query output will still be available via
    GetQueryOutputString and GetQueryOutputValue.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
  AddPlot("Pseudocolor", "d")
  DrawPlots()
  # Turn off automatic printing of Query output.
  SuppressQueryOutputOn()
  Query("MinMax")
  print("The min is: %g and the max is: %g" % GetQueryOutputValue())
  # Turn on automatic printing of Query output.
  SuppressQueryOutputOff()
  Query("MinMax")


TimeSliderGetNStates
--------------------

**Synopsis:**

::

  TimeSliderGetNStates() -> integer

return type : CLI_return_t
    Returns an integer containing the number of time states for the current
    time slider.


**Description:**

    The TimeSliderGetNStates function returns the number of time states for
    the active time slider. Remember that the length of the time slider does
    not have to be equal to the number of time states in a time-varying
    database because of database correlations and keyframing. If you want to
    iterate through time, use this function to determine the number of
    iterations that are required to reach the end of the active time slider.


**Example:**

::

  OpenDatabase("/usr/gapps/visit/data/wave.visit")
  AddPlot("Pseudocolor", "pressure")
  DrawPlots()
  for state in range(TimeSliderGetNStates()):
      SetTimeSliderState(state)
      SaveWindow()


TimeSliderNextState
-------------------

**Synopsis:**

::

  TimeSliderNextState() -> integer

return type : CLI_return_t
    The TimeSliderNextState function returns 1 on success and 0 on failure.


**Description:**

    The TimeSliderNextState function advances the active time slider to the
    next time slider state.


**Example:**

::

  # Assume that files are being written to the disk.
  #% visit -cli
  OpenDatabase("dynamic*.silo database")
  AddPlot("Pseudocolor", "var")
  AddPlot("Mesh", "mesh")
  DrawPlots()
  SetTimeSliderState(TimeSliderGetNStates() - 1)
  while 1:
      SaveWindow()
      TimeSliderPreviousState()


TimeSliderPreviousState
-----------------------

**Synopsis:**

::

  TimeSliderPreviousState() -> integer

return type : CLI_return_t
    The TimeSliderPreviousState function returns 1 on success and 0 on failure.


**Description:**

    The TimeSliderPreviousState function moves the active time slider to the
    previous time slider state.


**Example:**

::

  # Assume that files are being written to the disk.
  #% visit -cli
  OpenDatabase("dynamic*.silo database")
  AddPlot("Pseudocolor", "var")
  AddPlot("Mesh", "mesh")
  DrawPlots()
  while 1:
      TimeSliderNextState()
      SaveWindow()


TimeSliderSetState
------------------

**Synopsis:**

::

  TimeSliderSetState(state) -> integer


state : integer
    A zero-based integer containing the time state that we want to make active.

return type : CLI_return_t
    The TimeSliderSetState function returns 1 on success and 0 on failure.


**Description:**

    The TimeSliderSetState function sets the time state for the active time
    slider. This is the function to use if you want to animate through time or
    change the current keyframe frame.


**Example:**

::

  #% visit -cli
  path = "/usr/gapps/visit/data/"
  dbs = (path + "dbA00.pdb", path + "dbB00.pdb", path + "dbC00.pdb")
  for db in dbs:
      OpenDatabase(db)
      AddPlot("FilledBoundary", "material(mesh)")
      DrawPlots()
  CreateDatabaseCorrelation("common", dbs, 1)
  tsNames = GetWindowInformation().timeSliders
  for ts in tsNames:
      SetActiveTimeSlider(ts)
  for state in list(range(TimeSliderGetNStates())) + [0]:
      TimeSliderSetState(state)


ToggleBoundingBoxMode
---------------------

**Synopsis:**

::

  ToggleBoundingBoxMode() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleBoundingBoxMode function toggles bounding box mode on and off.
    When the visualization window is in bounding box mode, any plots it
    contains are hidden while the view is being changed so the window redraws
    faster.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


ToggleCameraViewMode
--------------------

**Synopsis:**

::

  ToggleCameraViewMode() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleCameraViewMode function toggles camera view mode on and off.
    When the visualization window is in camera view mode, the view is updated
    using any view keyframes that have been defined when VisIt is in keyframing
    mode.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


ToggleFullFrameMode
-------------------

**Synopsis:**

::

  ToggleFullFrameMode() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleFullFrameMode function toggles fullframe mode on and off. When
    the visualization window is in fullframe mode, the viewport is stretched
    non-uniformly so that it covers most of the visualization window. While not
    maintaining a 1:1 aspect ratio, it does make better use of the
    visualization window.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


ToggleLockTime
--------------

**Synopsis:**

::

  ToggleLockTime() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleLockTime function turns time locking on and off in a
    visualization window. When time locking is on in a visualization window,
    VisIt creates a database correlation that works for the databases in all
    visualization windows that are time-locked. When you change the time state
    using the time slider for the the afore-mentioned database correlation, it
    has the effect of updating time in all time-locked visualization windows.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


ToggleLockTools
---------------

**Synopsis:**

::

  ToggleBoundingBoxMode() -> integer
  ToggleCameraViewMode() -> integer
  ToggleFullFrameMode() -> integer
  ToggleLockTime() -> integer
  ToggleLockViewMode() -> integer
  ToggleMaintainViewMode() -> integer
  ToggleSpinMode() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleBoundingBoxMode function toggles bounding box mode on and off.
    When the visualization window is in bounding box mode, any plots it
    contains are hidden while the view is being changed so the window redraws
    faster.
    The ToggleCameraViewMode function toggles camera view mode on and off.
    When the visualization window is in camera view mode, the view is updated
    using any view keyframes that have been defined when VisIt is in keyframing
    mode.
    The ToggleFullFrameMode function toggles fullframe mode on and off. When
    the visualization window is in fullframe mode, the viewport is stretched
    non-uniformly so that it covers most of the visualization window. While not
    maintaining a 1:1 aspect ratio, it does make better use of the
    visualization window.
    The ToggleLockTime function turns time locking on and off in a
    visualization window. When time locking is on in a visualization window,
    VisIt creates a database correlation that works for the databases in all
    visualization windows that are time-locked. When you change the time state
    using the time slider for the the afore-mentioned database correlation, it
    has the effect of updating time in all time-locked visualization windows.
    The ToggleLockViewMode function turns lock view mode on and off. When
    windows are in lock view mode, each view change is broadcast to other
    windows that are also in lock view mode. This allows windows containing
    similar plots to be compared easily.
    The ToggleMaintainViewMode function forces the view, that was in effect
    when the mode was toggled to be used for all subsequent time states.
    The ToggleSpinMode function turns spin mode on and off. When the
    visualization window is in spin mode, it continues to spin along the axis
    of rotation when the view is changed interactively.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


ToggleLockViewMode
------------------

**Synopsis:**

::

  ToggleLockViewMode() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleLockViewMode function turns lock view mode on and off. When
    windows are in lock view mode, each view change is broadcast to other
    windows that are also in lock view mode. This allows windows containing
    similar plots to be compared easily.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


ToggleMaintainViewMode
----------------------

**Synopsis:**

::

  ToggleMaintainViewMode() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleMaintainViewMode functions forces the view that was in effect
    when the mode was toggled to be used for all subsequent time states.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


ToggleSpinMode
--------------

**Synopsis:**

::

  ToggleSpinMode() -> integer

return type : CLI_return_t
    All functions return 1 on success and 0 on failure.


**Description:**

    The visualization window has various modes that affect its behavior and
    the VisIt Python Interface provides a few functions to toggle some of those
    modes.
    The ToggleSpinMode function turns spin mode on and off. When the
    visualization window is in spin mode, it continues to spin along the axis
    of rotation when the view is changed interactively.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  # Turn on spin mode.
  ToggleSpinMode()
  # Rotate the plot interactively using the mouse and watch it keep spinning
  # after the mouse release.
  # Turn off spin mode.
  ToggleSpinMode()


TurnDomainsOff
--------------

**Synopsis:**

::

  TurnDomainsOff() -> integer
  TurnDomainsOff(set_name) -> integer
  TurnDomainsOff(tuple_set_name) -> integer


set_name : string
    The name of the set to modify.

tuple_set_name : tuple of strings
    A tuple of strings for the sets to modify.

return type : CLI_return_t
    The Turn functions return an integer with a value of 1 for success or 0
    for failure.


**Description:**

    The Turn functions are provided to simplify the removal of material or
    domain subsets. Instead of creating a SILRestriction object, you can use
    the Turn functions to turn materials or domains on or off. The
    TurnDomainsOff function turns domains off. All of the Turn functions have
    three possible argument lists. When you do not provide any arguments, the
    function applies to all subsets in the SIL so if you called the
    TurnDomainsOff function with no arguments, all domains would be turned
    off. All functions can also take a string argument, which is the name of the set
    to modify. For example, you could turn off domain 0 by calling the
    TurnDomainsOff with a single argument of "domain0" (or the appropriate
    set name). All of the Turn functions can also be used to modify more than
    one set if you provide a tuple of set names. After you use the Turn
    functions to change the SIL restriction, you might want to call the
    ListMaterials or ListDomains functions to make sure that the SIL
    restriction was actually modified.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  TurnMaterialsOff("4") # Turn off material 4
  TurnMaterialsOff(("1", "2")) # Turn off materials 1 and 2
  TurnMaterialsOn() # Turn on all materials


TurnDomainsOn
-------------

**Synopsis:**

::

  TurnDomainsOn() -> integer
  TurnDomainsOn(set_name) -> integer
  TurnDomainsOn(tuple_set_name) -> integer


set_name : string
    The name of the set to modify.

tuple_set_name : tuple of strings
    A tuple of strings for the sets to modify.

return type : CLI_return_t
    The Turn functions return an integer with a value of 1 for success or 0
    for failure.


**Description:**

    The Turn functions are provided to simplify the removal of material or
    domain subsets. Instead of creating a SILRestriction object, you can use
    the Turn functions to turn materials or domains on or off. The
    TurnDomainsOn function turns domains on. All of the Turn functions have
    three possible argument lists. When you do not provide any arguments, the
    function applies to all subsets in the SIL so if you called the
    TurnDomainsOn function with no arguments, all domains would be turned
    on. All functions can also take a string argument, which is the name of
    the set to modify. For example, you could turn on domain 0 by calling the
    TurnDomainsOn with a single argument of "domain0" (or the appropriate
    set name). All of the Turn functions can also be used to modify more than
    one set if you provide a tuple of set names. After you use the Turn
    functions to change the SIL restriction, you might want to call the
    ListMaterials or ListDomains functions to make sure that the SIL
    restriction was actually modified.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  TurnMaterialsOff("4") # Turn off material 4
  TurnMaterialsOff(("1", "2")) # Turn off materials 1 and 2
  TurnMaterialsOn() # Turn on all materials


TurnMaterialsOff
----------------

**Synopsis:**

::

  TurnMaterialsOff() -> integer
  TurnMaterialsOff(set_name) -> integer
  TurnMaterialsOff(tuple_set_name) -> integer


set_name : string
    The name of the set to modify.

tuple_set_name : tuple of strings
    A tuple of strings for the sets to modify.

return type : CLI_return_t
    The Turn functions return an integer with a value of 1 for success or 0
    for failure.


**Description:**

    The Turn functions are provided to simplify the removal of material or
    domain subsets. Instead of creating a SILRestriction object, you can use
    the Turn functions to turn materials or domains on or off. The
    TurnMaterialsOff function turns materials off. All of the Turn functions have
    three possible argument lists. When you do not provide any arguments, the
    function applies to all subsets in the SIL so if you called the
    TurnMaterialsOff function with no arguments, all materials would be turned
    off. All functions can also take a string argument, which is the name of
    the set to modify. For example, you could turn off material 0 by calling
    TurnMaterialsOff with a single argument of "material0" (or the appropriate
    set name). All of the Turn functions can also be used to modify more than
    one set if you provide a tuple of set names. After you use the Turn
    functions to change the SIL restriction, you might want to call the
    ListMaterials or ListDomains functions to make sure that the SIL
    restriction was actually modified.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  TurnMaterialsOff("4") # Turn off material 4
  TurnMaterialsOff(("1", "2")) # Turn off materials 1 and 2
  TurnMaterialsOn() # Turn on all materials


TurnMaterialsOn
---------------

**Synopsis:**

::

  TurnMaterialsOn() -> integer
  TurnMaterialsOn(string) -> integer
  TurnMaterialsOn(tuple of strings) -> integer


set_name : string
    The name of the set to modify.

tuple_set_name : tuple of strings
    A tuple of strings for the sets to modify.

return type : CLI_return_t
    The Turn functions return an integer with a value of 1 for success or 0
    for failure.


**Description:**

    The Turn functions are provided to simplify the removal of material or
    domain subsets. Instead of creating a SILRestriction object, you can use
    the Turn functions to turn materials or domains on or off. The
    TurnMaterialsOn function turns materials on. All of the Turn functions have
    three possible argument lists. When you do not provide any arguments, the
    function applies to all subsets in the SIL so if you called the
    TurnMaterialsOn function with no arguments, all materials would be turned
    off. All functions can also take a string argument, which is the name of
    the set to modify. For example, you could turn on material 0 by calling the
    TurnMaterialsOn with a single argument of "material0" (or the appropriate
    set name). All of the Turn functions can also be used to modify more than
    one set if you provide a tuple of set names. After you use the Turn
    functions to change the SIL restriction, you might want to call the
    ListMaterials or ListDomains functions to make sure that the SIL
    restriction was actually modified.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/globe.silo")
  AddPlot("Pseudocolor", "u")
  DrawPlots()
  TurnMaterialsOff("4") # Turn off material 4
  TurnMaterialsOff(("1", "2")) # Turn off materials 1 and 2
  TurnMaterialsOn() # Turn on all materials


UndoView
--------

**Synopsis:**

::

  UndoView()


**Description:**

    When the view changes in the visualization window, it puts the old view on
    a stack of views. The UndoView function restores the view on top of the
    stack and removes it. This allows the user to undo up to ten view changes.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/curv2d.silo")
  AddPlot("Subset", "mat1")
  DrawPlots()
  v = GetView2D()
  v.windowCoords = (-2.3,2.4,0.2,4.9)
  SetView2D(v)
  UndoView()


UpdateNamedSelection
--------------------

**Synopsis:**

::

  UpdateNamedSelection(name) -> integer
  UpdateNamedSelection(name, properties) -> integer


name : string
    The name of the selection to update.

properties : SelectionProperties object
    An optional SelectionProperties object that contains the selection
    properties to use when reevaluating the selection.

return type : CLI_return_t
    The UpdateNamedSelection function returns 1 on success and 0 on failure.


**Description:**

    This function causes VisIt to reevaluate a named selection using new selection
    properties. If no selection properties are provided then the selection will
    be reevaluated using data for the plot that was associated with the selection
    when it was created. This is useful if you want to change a plot in several
    ways before causing its associated named selection to update using the changes.


**Example:**

::

  s = GetSelection('selection1')
  s.selectionType = s.CumulativeQuerySelection
  s.histogramType = s.HistogramMatches
  s.combineRule = s.CombineOr
  s.variables = ('temperature',)
  s.variableMins = (2.9,)
  s.variableMaxs = (3.1,)
  UpdateNamedSelection('selection1', s)


Version
-------

**Synopsis:**

::

  Version() -> string

return type : string
    The Version function return a string that represents VisIt's version.


**Description:**

    The Version function returns a string that represents VisIt's version. The
    version string can be used in Python scripts to make sure that the VisIt
    module is a certain version before processing the rest of the Python script.


**Example:**

::

  #% visit -cli
  print("We are running VisIt version %s" % Version())


WriteConfigFile
---------------

**Synopsis:**

::

  WriteConfigFile()


**Description:**

    The viewer maintains internal settings which determine the default values
    for objects like plots and operators. The viewer can save out the default
    values so they can be used in future VisIt sessions. The WriteConfig
    function tells the viewer to write out the settings to the VisIt
    configuration file.


**Example:**

::

  #% visit -cli
  p = PseudocolorAttributes()
  p.minFlag, p.min = 1, 5.0
  p.maxFlag, p.max = 1, 20.0
  SetDefaultPlotOptions(p)
  # Save the new default Pseudocolor settings to the config file.
  WriteConfig()


WriteScript
-----------


**Example:**

::

  f = open('script.py', 'wt')
  WriteScript(f)
  f.close()



ZonePick
--------

**Synopsis:**

::

  ZonePick(namedarg1=arg1, namedarg2=arg2, ...) -> dictionary


coord : tuple
    A tuple of doubles containing the spatial coordinate (x, y, z).

x : integer
    An integer containing the screen X location (in pixels) offset
    from the left side of the visualization window.

y : integer
    An integer containing the screen Y location (in pixels) offset
    from the bottom of the visualization window.

vars (optional) : tuple
    A tuple of strings with the variable names for which to return
    results. Default is the currently plotted variable.

do_time (optional) : integer
    An integer indicating whether to do a time pick. 1 -> do a time pick,
    0 (default) -> do not do a time pick.

start_time (optional) : integer
    An integer with the starting frame index. Default is 0.

end_time (optional) : integer
    An integer with the ending frame index. Default is num_timestates-1.

stride (optional) : integer
    An integer with the stride for advancing in time. Default is 1.

preserve_coord (optional) : integer
    An integer indicating whether to pick an element or a coordinate.
    0 -> used picked element (default), 1-> used picked coordinate.

curve_plot_type (optional) : integer
    An integer indicating whether the output should be on a single
    axis or with multiple axes. 0 -> single Y axis (default),
    1 -> multiple Y Axes.

return type : dictionary
    ZonePick returns a python dictionary of the pick results, unless
    do_time is specified, then a time curve is created in a new window.
    If the picked variable is node centered, the variable values are
    grouped according to incident node ids.


**Description:**

    The ZonePick function prints pick information for the cell (a.k.a zone) that
    contains the specified point. The point can be specified as a 2D or 3D
    point in world space or it can be specified as a pixel location in screen
    space. If the point is specified as a pixel location then VisIt finds the
    zone that contains the intersection of a cell and a ray that is projected
    into the mesh. Once the zonal pick has been calculated, you can use the
    GetPickOutput function to retrieve the printed pick output as a string
    which can be used for other purposes.


**Example:**

::

  #% visit -cli
  OpenDatabase("/usr/gapps/visit/data/noise.silo")
  AddPlot("Pseudocolor", "hgslice")
  DrawPlots()
  # Perform zone pick in screen space
  pick_out = ZonePick(x=200,y=200)
  # Perform zone pick in world space.
  pick_out = ZonePick(coord = (-5.0, 5.0, 0))

.. end of the file
