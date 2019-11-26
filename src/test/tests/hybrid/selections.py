# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  selections.py
#
#  Tests:      named selections
#
#  Programmer: Brad Whitlock
#  Date:       Thu Jun  2 14:23:25 PDT 2011
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def TestHistogram(name, data):
    file = "tmp.curve"
    f = open(file, "wt")
    f.write("# histogram\n")
    for x in range(len(data)):
        f.write("%g %g\n" % (x, data[x]))
    f.close()

    AddWindow()
    DeleteAllPlots()
    OpenDatabase(file)
    AddPlot("Curve", "histogram")
    c = CurveAttributes()
    c.showLines = 1
    c.lineWidth = 1
    c.showPoints = 1
    c.symbol = c.Point  # Point, TriangleUp, TriangleDown, Square, Circle, Plus, X
    c.pointSize = 5
    c.curveColorSource = c.Custom  # Cycle, Custom
    c.curveColor = (0, 0, 255, 255)
    c.showLabels = 0
    SetPlotOptions(c)
    DrawPlots()
    a = GetAnnotationAttributes()
    a.axes2D.visible = 1
    a.axes2D.xAxis.title.visible = 0
    a.axes2D.xAxis.label.visible = 0
    a.axes2D.xAxis.tickMarks.visible = 0
    a.axes2D.yAxis.title.visible = 0
    a.axes2D.yAxis.label.visible = 0
    a.axes2D.yAxis.tickMarks.visible = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    SetAnnotationAttributes(a)
    Test(name)
    DeleteAllPlots()
    CloseDatabase(file)
    DeleteWindow()
    os.unlink(file)

def GetVariableHistogram(selName, varName=None):
    s = GetSelectionSummary(selName)
    for j in range(s.GetNumVariables()):
        if s.GetVariables(j).name == varName:
            #print "returning histogram for variable ",varName
            return s.GetVariables(j).histogram
        #print "returning histogramValues for whole selection"
        return s.histogramValues
    return []

#################################################

def SetupPlots(var1, var2):
    AddPlot("Pseudocolor", var1)
    DrawPlots()
    v = View3DAttributes()
    v.viewNormal = (-0.4992, 0.804573, 0.321654)
    v.focus = (5, 0.753448, 2.5)
    v.viewUp = (0.71837, 0.591864, -0.365571)
    v.viewAngle = 30
    v.parallelScale = 5.6398
    v.nearPlane = -11.2796
    v.farPlane = 11.2796
    v.imagePan = (0.0166415, 0.0258621)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    SetView3D(v)

    AddPlot("Pseudocolor", var2)
    t = TransformAttributes()
    t.doTranslate = 1
    t.translateZ = 6
    AddOperator("Transform")
    SetOperatorOptions(t)
    DrawPlots()

#
# Test basic selection creation and usage.
#
def test0():
    # Creating and using selections
    TestSection("Selections")
    db = data_path("silo_hdf5_test_data/noise.silo")
    OpenDatabase(db)
    AddPlot("Pseudocolor", "hardyglobal")
    AddPlot("Pseudocolor", "grad_magnitude")
    t = TransformAttributes()
    t.doTranslate = 1
    t.translateX = 21
    AddOperator("Transform")
    SetOperatorOptions(t)

    SetActivePlots(0)
    AddOperator("ThreeSlice")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (-0.580761, 0.522378, 0.62437)
    v.focus = (10.5, 0, 0)
    v.viewUp = (0.340344, 0.852526, -0.396692)
    v.viewAngle = 30
    v.parallelScale = 24.9048
    v.nearPlane = -49.8096
    v.farPlane = 49.8096
    v.imagePan = (0.0272141, 0.0381474)
    v.imageZoom = 1.15431
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (10.5, 0, 0)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    SetView3D(v)
    Test("selections_0_00")

    # Create a selection and apply it to the 2nd plot
    selName = "threeslice_sel"
    CreateNamedSelection(selName)
    SetActivePlots(1)
    ApplyNamedSelection(selName)
    Test("selections_0_01")

    # Update plot 1 and update its selection.
    SetActivePlots(0)
    t3 = ThreeSliceAttributes()
    t3.x = 5
    t3.y = 5
    t3.z = 5
    SetOperatorOptions(t3)
    UpdateNamedSelection(selName)
    Test("selections_0_02")

    # Turn on selection auto updating.
    # Change the setup so plot 2 from window 1 goes to window 2.
    AddWindow()
    TurnOffAllAnnotations()
    AddPlot("Pseudocolor", "grad_magnitude")
    DrawPlots()
    ApplyNamedSelection(selName)
    Test("selections_0_03")

    SetActiveWindow(1)
    SetNamedSelectionAutoApply(1)
    t3 = ThreeSliceAttributes()
    t3.x = -5
    t3.y = -5
    t3.z = -5
    SetOperatorOptions(t3)
    SetActiveWindow(2)
    Test("selections_0_04")
    SetNamedSelectionAutoApply(0)

    DeleteWindow()
    DeleteNamedSelection(selName)
    DeleteAllPlots()
    #CloseDatabase(db)  #Uncomment this if you want to debug an engine crash

#
# Test Cumulative Query selections.
#
def test123():
    TestSection("Cumulative Query")

    SetupPlots("m3", "pressure")
    Test("selections_1_00")

    # Create a named selection from the first plot
    SetActivePlots(0)
    selName = "matvf_selection"
    CreateNamedSelection(selName)
    s = GetSelection(selName)
    TestText("selections_1_01", str(s))

    # Apply the new selection to the plot on the right.
    SetActivePlots(1)
    ApplyNamedSelection(selName)
    Test("selections_1_02")

    # At this point, the selection will contain all cells. Let's focus
    # on a subset of cells that have m3 in [0,0.5] in all times. This will
    # be a disc of material in the center of the wave.
    s.selectionType = s.CumulativeQuerySelection
    s.variables = ("m3",)
    s.variableMins = (0.,)
    s.variableMaxs = (0.5,)
    s.combineRule = s.CombineAnd
    UpdateNamedSelection(selName, s)

    # Save off a picture of the m3 variable's histogram.
    TestHistogram("selections_1_03", GetVariableHistogram(selName, "m3"))

    # Save off a picture of the selection's #cells vs time histogram 
    TestHistogram("selections_1_04", GetVariableHistogram(selName))
    Test("selections_1_05")
    SetTimeSliderState(35)
    Test("selections_1_06")
    SetTimeSliderState(70)
    Test("selections_1_07")
    SetTimeSliderState(0)

    # Let's change to a set of cells where the dirt material is high in all
    # times. This will be a set of cells that lie on the left,top,right edges
    # of the dataset since those areas never get overwhelmed by material 2.
    s.variables = ("m3",)
    s.variableMins = (0.5,)
    s.variableMaxs = (1.,)
    s.combineRule = s.CombineAnd
    UpdateNamedSelection(selName, s)

    # Save off a picture of the selection's #cells vs time histogram 
    TestHistogram("selections_1_08", GetVariableHistogram(selName))
    Test("selections_1_09")
    SetTimeSliderState(35)
    Test("selections_1_10")
    SetTimeSliderState(70)
    Test("selections_1_11")
    SetTimeSliderState(0)

    # Let's change the selection so it has to contain m2 in [0.5,1.] in
    # any time. This will be a large swath out of the middle of the wave,
    # excluding the edges and the disc in the middle.
    SetActivePlots(0)
    ChangeActivePlotsVar("m2")
    SetActivePlots(1)
    s.combineRule = s.CombineOr
    s.variables = ("m2",)
    s.variableMins = (0.5,)
    s.variableMaxs = (1.,)
    UpdateNamedSelection(selName, s)

    # Save off a picture of the m2 variable's histogram.
    TestHistogram("selections_1_12", GetVariableHistogram(selName, "m2"))
    # Save off a picture of the selection's #cells vs time histogram 
    TestHistogram("selections_1_13", GetVariableHistogram(selName))
    Test("selections_1_14")
    SetTimeSliderState(35)
    Test("selections_1_15")
    SetTimeSliderState(70)
    Test("selections_1_16")
    SetTimeSliderState(0)

#
# Test 2
#
    # Test setting the min/max bounds of the histogram in different
    # histogram modes
    TestSection("Cumulative Query - Histogram Matches")
    s.histogramType = s.HistogramMatches
    s.histogramNumBins = 5
    s.histogramStartBin = 0
    s.histogramEndBin = 4
    UpdateNamedSelection(selName, s)
    
    # Plot the m2 variable's sum over time, which is kind of like a frequency
    # since cells that contain the material in more time steps have a higher
    # frequency.
    DefineScalarExpression("m2_frequency", "sum_over_time(m2)")
    ChangeActivePlotsVar("m2_frequency")
    SetTimeSliderState(70)
    Test("selections_2_00")
    # Save off a picture of the selection's frequency vs matches histogram 
    TestHistogram("selections_2_01", GetVariableHistogram(selName))
    TestText("selections_2_02", str(GetSelectionSummary(selName)))
    
    # Demonstrate that limiting the histogram end bin removes high frequency
    # cells.
    end_test = list(zip((4,2,0), (3,4,5)))
    for maxBin,testNum in end_test:
        s.histogramEndBin = maxBin
        UpdateNamedSelection(selName, s)
        Test("selections_2_%02d" % testNum)
    
    # Show that we can also limit using the start bin.
    s.histogramStartBin,s.histogramEndBin = 2,3
    UpdateNamedSelection(selName, s)
    Test("selections_2_06")

#
# Test 3
#

    TestSection("Cumulative Query - Histogram ID")
    s.histogramType = s.HistogramID
    s.histogramNumBins = 5
    s.histogramStartBin = 0
    s.histogramEndBin = 4
    UpdateNamedSelection(selName, s)
    
    # Plot the zoneid so we can see that bands of related zones go away
    # at the same time.
    DefineScalarExpression("zid", "zoneid(quadmesh)")
    ChangeActivePlotsVar("zid")
    Test("selections_3_00")
    # Save off a picture of the selection's frequency vs ID histogram 
    TestHistogram("selections_3_01", GetVariableHistogram(selName))
    TestText("selections_3_02", str(GetSelectionSummary(selName)))

    # Demonstrate that limiting the histogram end bin removes related cells.
    end_test = list(zip((4,2,0), (3,4,5)))
    for maxBin,testNum in end_test:
        s.histogramEndBin = maxBin
        UpdateNamedSelection(selName, s)
        Test("selections_3_%02d" % testNum)
    
    # Show that we can also limit using the start bin.
    s.histogramStartBin,s.histogramEndBin = 2,3
    UpdateNamedSelection(selName, s)
    Test("selections_3_06")
    DeleteNamedSelection(selName)
    DeleteAllPlots()
    
#
# Test Cumulative query selections with variable histogramming.
#
def test4():
    # We create a CQ selection based on 2 variables so we can sort the cells
    # by the dist variable. This will let us do a radial dissolve from the
    # center of the remaining cells.
    TestSection("Cumulative Query - Histogram Variable")
    DefineVectorExpression("xyz", "coord(quadmesh)")
    DefineScalarExpression("dist", "sqrt((xyz[2] - 2.3)*(xyz[2] - 2.3) + (xyz[0] - 4.8)*(xyz[0] - 4.8))")
    SetupPlots("m2", "dist")

    SetActivePlots(0)
    selName = "xyz_selection"
    CreateNamedSelection(selName)
    s = GetSelection(selName)
    s.histogramType = s.HistogramVariable
    s.selectionType = s.CumulativeQuerySelection
    s.variables = ("m2", "dist")
    s.variableMins = (0.5, 0.)
    s.variableMaxs = (1., 6.)
    s.histogramVariable = s.variables[1]
    s.histogramNumBins = 5
    s.histogramStartBin = 0
    s.histogramEndBin = 4
    UpdateNamedSelection(selName, s)

    SetActivePlots(1)
    ApplyNamedSelection(selName)
    SetTimeSliderState(70)

    # Plot the dist so we can see that concentric bands of zones go away
    # at the same time.
    ChangeActivePlotsVar("dist")
    Test("selections_4_00")
    # Save off a picture of the dist variable's histogram.
    TestHistogram("selections_4_01", GetVariableHistogram(selName, "dist"))
    # Save off a picture of the selection's frequency vs variable histogram 
    TestHistogram("selections_4_02", GetVariableHistogram(selName))

# This is not working in parallel. The wrong min/max get returned for the dist var.
#    TestText("selections_4_03", str(GetSelectionSummary(selName)))

    # Demonstrate that limiting the histogram start bin removes cells with
    # low dist values.
    end_test = list(zip((0,2,4), (4,5,6)))
    for minBin,testNum in end_test:
        s.histogramStartBin = minBin
        UpdateNamedSelection(selName, s)
        Test("selections_4_%02d" % testNum)
    
    # Show that we can also limit using the start bin.
    s.histogramStartBin,s.histogramEndBin = 2,3
    UpdateNamedSelection(selName, s)
    Test("selections_4_07")

    DeleteAllPlots()
    DeleteNamedSelection(selName)

#
# Test selection creation directly on files (not plots)
#
def test5():
    TestSection("Cumulative Query based on database")
    DefineScalarExpression("mat1", "matvf(<material(mesh)>, 1)")

    db = data_path("pdb_test_data/dbA00.pdb")
    OpenDatabase(db, 0, "PDB")

    selName = "dbA_sel"
    s = SelectionProperties()
    s.name = selName
    s.source = db
    s.selectionType = s.CumulativeQuerySelection
    s.histogramType = s.HistogramMatches
    s.combineRule = s.CombineOr
    s.variables = ("mesh/ireg",)
    s.variableMins = (2.9,)
    s.variableMaxs = (3.1,)
    s.histogramNumBins = 5
    s.histogramStartBin = 0
    s.histogramEndBin = 4

    # Create a selection without a plot
    CreateNamedSelection(selName, s)

    # Create a plot
    AddPlot("Pseudocolor", "mesh/nummm")
    DrawPlots()
    Test("selections_5_00")
    
    # Apply the selection
    ApplyNamedSelection(selName)
    Test("selections_5_01")
    
    # Demonstrate that limiting the histogram end bin removes high frequency
    # cells.
    end_test = list(zip((3,1,0), (2,3,4)))
    for maxBin,testNum in end_test:
        s.histogramEndBin = maxBin
        UpdateNamedSelection(selName, s)
        Test("selections_5_%02d" % testNum)

    DeleteAllPlots()
    CloseDatabase(db)

#
# Test Cumulative query selections on multidomain datasets.
#    
def test6():
    # write me
    return

def main():
    test0()

    OpenDatabase(data_path("silo_hdf5_test_data/wave.visit"))
    DefineScalarExpression("m3", "matvf(Material, 3)")
    DefineScalarExpression("m2", "matvf(Material, 2)")

    test123()
    test4()

    CloseDatabase(data_path("silo_hdf5_test_data/wave.visit"))

    test5()
    test6()

main()
Exit()
