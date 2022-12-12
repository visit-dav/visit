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

    # print(x) will produce something like...
    #     ('Boundary', 'Contour', 'Curve', 'FilledBoundary', 'Histogram',
    #     'Label', 'Mesh', 'Molecule', 'MultiCurve', 'ParallelCoordinates',
    #     'Pseudocolor', 'Scatter', 'Spreadsheet', 'Subset', 'Tensor',
    #     'Truecolor', 'Vector', 'Volume')
    print(x)

    # Create plots with AddPlot(<plot-plugin-name>,<database-variable-name>)
    AddPlot("Pseudocolor", "pressure")
    AddPlot("Mesh", "quadmesh")

    # Draw the plots
    DrawPlots()
    # creating a plot }
    TestValueEQ('creating a plot error message',GetLastError(),'')
    TestPOA('creating a plot exceptions')
  except Exception as inst:
    TestFOA('creating a plot exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

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
  except Exception as inst:
    TestFOA('plotting materials exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def setting_plot_attributes():
 
  OpenDatabase(silo_data_path("wave.visit"))
  try:
    # setting plot attributes {
    # Creating a Pseudocolor plot and setting min/max values.
    AddPlot("Pseudocolor", "pressure")
    p = PseudocolorAttributes()

    # print p to find the names of members you want to change
    #
    # print(p) will produce output somewhat like...
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
    print(p)

    # Set the min/max values
    p.min, p.minFlag = 0.0, 1
    p.max, p.maxFlag = 10.0, 1
    SetPlotOptions(p)
    # setting plot attributes }
    TestValueEQ('setting plot attributes message',GetLastError(),'')
    TestPOA('setting plot attributes exceptions')
  except Exception as inst:
    TestFOA('setting plot attributes exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

def working_with_multiple_plots():
 
  OpenDatabase(silo_data_path("tire.silo"))
  try:
    # working with multiple plots {

    # Create more than 1 plot of the same type
    AddPlot("Pseudocolor", "pressure")
    AddPlot("Pseudocolor", "temperature")

    # List the plots. The second plot should be active.
    ListPlots()

    # The output from ListPlots() will look something like...
    #     Plot[0]|id=5;type="Pseudocolor";database="localhost:/Users/miller86/visit/visit/data/silo_hdf5_test_data/tire.silo";
    #         var=pressure;active=0;hidden=0;framerange=(0, 0);keyframes={0};database keyframes={0};operators={};
    #         activeOperator=-1
    #     Plot[1]|id=6;type="Pseudocolor";database="localhost:/Users/miller86/visit/visit/data/silo_hdf5_test_data/tire.silo";
    #         var=temperature;active=1;hidden=0;framerange=(0, 0);keyframes={0};database keyframes={0};operators={};
    #         activeOperator=-1
    # Note that active=1 for Plot[1] meaning plot #1 is the active plot

    # Draw the plots
    DrawPlots()

    # Hide the first plot
    SetActivePlots(0) # makes plot 0 the active plot
    HideActivePlots()

    # Set both plots' color table to "hot"
    p = PseudocolorAttributes()
    p.colorTableName = "hot"
    SetActivePlots((0,1)) # makes both plots active
    SetPlotOptions(p)

    # Show the first plot again.
    SetActivePlots(0)
    HideActivePlots()

    # Delete the second plot
    SetActivePlots(1)
    DeleteActivePlots()
    ListPlots()

    # working with multiple plots }
    TestValueEQ('working with multiple plots error message',GetLastError(),'')
    TestPOA('working with multiple plots exceptions')
  except Exception as inst:
    TestFOA('working with multiple plots exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()
  CloseDatabase(silo_data_path("tire.silo"))

def plots_in_the_error_state():
 
  try:
    # plots in the error state {

    # Open the database at state 20 and add plots for "pressure" and "transient".
    # "transient" variable exists only in states 18...51.
    OpenDatabase(silo_data_path("wave.visit"),20)
    AddPlot("Pseudocolor","pressure")
    AddPlot("Pseudocolor","transient")
    DrawPlots()

    # Start saving images from every 10th state starting at state 20 
    # but take care to clean up when we get an error.
    for state in range(20,TimeSliderGetNStates(),10):

        TimeSliderSetState(state)

        if DrawPlots() == 0:

            # Find plot(s) in error state and remove them
            pl = GetPlotList()
            for i in range(pl.GetNumPlots()):
                if pl.GetPlots(i).stateType == pl.GetPlots(i).Error:
                    SetActivePlots((i,))
                    DeleteActivePlots()

            # Clear the last error message
            GetLastError(1)

        SaveWindow()

    # plots in the error state }
    TestValueEQ('plots in the error state error message',GetLastError(),'')
    TestPOA('plots in the error state exceptions')
  except Exception as inst:
    TestFOA('plots in the error state exception "%s"'%str(inst), LINE())
    pass
  vqr_cleanup()

creating_a_plot()
plotting_materials()
setting_plot_attributes()
working_with_multiple_plots()
plots_in_the_error_state()

Exit()
