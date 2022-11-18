# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Mark C. Miller, Fri Aug 26 14:31:48 PDT 2022
# ----------------------------------------------------------------------------

#
# Get some utils shared across many .py files used for quick recipes.
#
Source(tests_path('quickrecipes','vqr_utils.py'))

def creating_a_plot():
 
  OpenDatabase(silo_data_path("wave.visit"))
  try:
    # creating a plot {
    # Names of all available plot plugins as a python tuple
    x = PlotPlugins()
    print(x)
    # print(x) will produce something like...
    # ('Boundary', 'Contour', 'Curve', 'FilledBoundary', 'Histogram', 'Label', 'Mesh', 'Molecule', 'MultiCurve', 'ParallelCoordinates', 'Pseudocolor', 'Scatter', 'Spreadsheet', 'Subset', 'Tensor', 'Truecolor', 'Vector', 'Volume')
    # Create plots with AddPlot(<plot-plugin-name>,<database-variable-name>)
    AddPlot("Pseudocolor", "pressure")
    AddPlot("Mesh", "quadmesh")
    # Draw the plots
    DrawPlots()
    # creating a plot }
    TestValueEQ('creating a plot error message',GetLastError(),'')
    TestPOA('creating a plot exceptions')
  except:
    TestFOA('creating a plot exception', LINE())
    pass
  vqr_cleanup()
  CloseDatabase(silo_data_path("wave.visit"))

def plotting_materials():
 
  OpenDatabase(silo_data_path("globe.silo"))
  try:
    # plotting materials {
    # Plot material boundaries
    AddPlot("Boundary", "mat1")
    # Plot materials
    AddPlot("FilledBoundary", "mat1")
    # plotting materials }
    TestValueEQ('plotting materials error message',GetLastError(),'')
    TestPOA('plotting materials exceptions')
  except:
    TestFOA('plotting materials exception', LINE())
    pass
  vqr_cleanup()
  CloseDatabase(silo_data_path("wave.visit"))

def setting_plot_attributes():
 
  OpenDatabase(silo_data_path("wave.visit"))
  try:
    # setting plot attributes {
    # Creating a Pseudocolor plot and setting min/max values.
    AddPlot("Pseudocolor", "pressure")
    p = PseudocolorAttributes()
    # print the object to see names of members
    print(p)
    # this will produce output somewhat like...
    #     scaling = Linear  # Linear, Log, Skew
    #     skewFactor = 1
    #     limitsMode = OriginalData  # OriginalData, ActualData
    #     minFlag = 0
    #     min = 0
    #     useBelowMinColor = 0
    #     belowMinColor = (0, 0, 0, 255)
    #     maxFlag = 0
    #     max = 1
    #     .
    #     .
    #     .
    # Set the min/max values
    p.min, p.minFlag = 0.0, 1
    p.max, p.maxFlag = 10.0, 1
    SetPlotOptions(p)
    # setting plot attributes }
    TestValueEQ('setting plot attributes message',GetLastError(),'')
    TestPOA('setting plot attributes exceptions')
  except:
    TestFOA('setting plot attributes exception', LINE())
    pass
  vqr_cleanup()
  CloseDatabase(silo_data_path("wave.visit"))

creating_a_plot()
plotting_materials()
setting_plot_attributes()

Exit()
