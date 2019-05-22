# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  queriesOverTime.py #
#  Tests:      queries     - Database
#
#  Defect ID:  none
#
#  Programmer: Kathleen Bonnell 
#  Date:       March 31, 2004 
#
#  Modifications:
#
#    Hank Childs, Tue Apr 13 13:00:15 PDT 2004
#    Rename surface area query.
#
#    Kathleen Bonnell, Tue Apr 20 09:42:30 PDT 2004 
#    Added TestFilledBoundary.
#
#    Kathleen Bonnell, Tue Apr 27 12:10:44 PDT 2004 
#    Added TestExpressions, TestOperators.
#
#    Kathleen Bonnell, Thu Jun 24 09:49:35 PDT 2004 
#    Added TestTransientVariable.
#
#    Kathleen Bonnell, Wed Jul 21 16:51:31 PDT 2004 
#    Added TestSpecifyTimeQueryWindow.
#
#    Kathleen Bonnell, Wed Sep  8 10:53:58 PDT 2004 
#    Renamed 'WorldPick' as 'Pick'.
#
#    Kathleen Bonnell, Mon Dec 20 15:54:04 PST 2004 
#    Changed 'Variable by Node' to 'PickByNode'.
#
#    Kathleen Bonnell, Thu Jan  6 11:06:29 PST 2005 
#    Added TestTimeVaryingSIL.
#
#    Kathleen Bonnell, Wed Mar 16 11:13:40 PST 2005 
#    Added TestQueryAfterQueryOverTime.
#
#    Kathleen Bonnell, Wed Jul  6 16:21:34 PDT 2005 
#    Added TestMili.
#
#    Kathleen Bonnell, Thu Nov 10 08:21:54 PST 2005 
#    Added TrajectoryByZone to TestMili.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Fri Feb  5 09:27:37 PST 2010
#    Turn off color cycling to avoid possible propagation of error from
#    one failed test to several.
#
#    Kathleen Bonnell, Thu Mar  3 11:47:09 PST 2011
#    Added MultiVarTimePick tests.
#
#    Kathleen Biagas, Thu Jul 14 10:44:55 PDT 2011
#    Use named arguments. 
#
#    Alister Maguire, Tue Oct 17 16:54:48 PDT 2017
#    Added TestPickRangeTimeQuery
#
#    Alister Maguire, Wed May  9 10:13:26 PDT 2018
#    Added TestReturnValue. 
#
#    Alister Maguire, Wed May 30 14:16:28 PDT 2018
#    Added tests for performing pick ranges over time with and 
#    without plotting and returning the curves. 
#
#    Alister Maguire, Wed May 22 08:49:30 PDT 2019
#    Updated mili tests to reflect new plugin changes. 
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin(("PDB", "Mili", "SAMRAI"))

def InitAnnotation():
    # Turn off most annotations
    a = AnnotationAttributes()
    a.axes2D.visible = 1
    a.axes2D.xAxis.label.visible = 1
    a.axes2D.yAxis.label.visible = 1
    a.axes2D.xAxis.title.visible = 1
    a.axes2D.yAxis.title.visible = 1
    a.axes3D.triadFlag = 0
    a.axes3D.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = a.Solid
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def SetCurvePlotDefaults():
    # Disable Color Cycling, default to a blue curve.
    catts = CurveAttributes()
    catts.lineWidth = 0
    catts.color = (0, 0, 255, 255)
    catts.showLabels = 1
    catts.designator = ""
    catts.showPoints = 0
    catts.showLegend = 1
    catts.cycleColors = 0
    catts.renderMode = catts.RenderAsLines
    SetDefaultPlotOptions(catts)



def TestAllTimeQueries():
    OpenDatabase(silo_data_path("wave.visit"))

    AddPlot("Pseudocolor", "pressure")
    DrawPlots()

    # Do some database queries.
    QueryOverTime("3D surface area")
    SetActiveWindow(2) 
    InitAnnotation()
    Test("AllTimeQueries_01")
    DeleteAllPlots()
   
    SetActiveWindow(1) 
    QueryOverTime("Volume")
    SetActiveWindow(2);
    Test("AllTimeQueries_02")
    DeleteAllPlots()

    SetActiveWindow(1) 
    QueryOverTime("Min")
    SetActiveWindow(2);
    Test("AllTimeQueries_03")
    DeleteAllPlots()

    SetActiveWindow(1) 
    QueryOverTime("Max")
    SetActiveWindow(2);
    Test("AllTimeQueries_04")
    DeleteAllPlots()

    SetActiveWindow(1) 
    QueryOverTime("Variable Sum")
    SetActiveWindow(2);
    Test("AllTimeQueries_05")
    DeleteAllPlots()

    SetActiveWindow(1) 
    QueryOverTime("Weighted Variable Sum")
    SetActiveWindow(2);
    Test("AllTimeQueries_06")
    DeleteAllPlots()

    SetActiveWindow(1) 
    pa = GetPickAttributes()
    pa.doTimeCurve = 1
    pa.timePreserveCoord = 0
    SetPickAttributes(pa)
    PickByNode(15947)
    # reset some defaults
    pa.doTimeCurve = 0
    pa.timePreserveCoord = 1
    SetPickAttributes(pa)
    SetActiveWindow(2);
    Test("AllTimeQueries_07")

    # delete window 2
    DeleteWindow()
    # remove plots from window 1
    DeleteAllPlots()

def TestFilledBoundary():
    #  bug '4708
    OpenDatabase(silo_data_path("wave.visit"))

    AddPlot("FilledBoundary", "Material")
    DrawPlots()
    TurnMaterialsOff(("1 barrier", "2 water"))
    
    SetActiveWindow(1)
    QueryOverTime("3D surface area")
    SetActiveWindow(2)
    InitAnnotation()

    Test("FBTimeQuery_01")
    DeleteAllPlots()

    SetActiveWindow(1)
    DeleteAllPlots()
    AddPlot("Pseudocolor", "pressure") 
    DrawPlots()
    TurnMaterialsOff(("1 barrier", "2 water"))
    QueryOverTime("3D surface area")
    SetActiveWindow(2)
    Test("FBTimeQuery_02")

    # delete window 2
    DeleteWindow()
    # remove plots from window 1
    TurnMaterialsOn()
    DeleteAllPlots()

def TestOperators():
    #  bug '4818
    OpenDatabase(silo_data_path("wave*.silo database"))

    AddPlot("Pseudocolor", "pressure")
    AddOperator("Isovolume")
    iso = IsovolumeAttributes()
    iso.lbound = 0.1
    iso.ubound = 1.0
    SetOperatorOptions(iso)
    DrawPlots()
   
    SetActiveWindow(1)
    QueryOverTime("Volume", stride=10) 
    SetActiveWindow(2)
    InitAnnotation()
    Test("TimeQuery_ops_01")

    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

    AddPlot("Pseudocolor", "mesh_quality/jacobian")
    AddOperator("Slice")
    slice = SliceAttributes()
    slice.axisType = slice.Arbitrary
    slice.normal = (-0.689, -0.0416, 0.7233)
    slice.originType = slice.Point
    slice.originPoint = (2.0011, -0.4084, -1.1279)
    slice.upAxis = (-0.08584, 0.996007, -0.0245)
    slice.project2d = 1
    SetOperatorOptions(slice)

    DrawPlots()
    QueryOverTime("2D area", stride=10)
    SetActiveWindow(2)
    InitAnnotation()
    Test("TimeQuery_ops_02")

    # prepare for next test-set
    # delete plots from window 2 & l
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestExpressions():
    #bug '4784
    OpenDatabase(data_path("pdb_test_data/dbA00.pdb"))

    AddPlot("Pseudocolor", "mesh/ireg")
    pa = PseudocolorAttributes()
    pa.minFlag = 1
    pa.maxFlag = 1
    pa.min = 1
    pa.max = 4
    SetPlotOptions(pa)
    DrawPlots()

    pt = (4., 3., 0.)
    pick = GetPickAttributes()
    pick.doTimeCurve = 1
    SetPickAttributes(pick)
    Pick(pt)

    SetActiveWindow(2)
    InitAnnotation()
    Test("TimeQuery_expr_01")

    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

    # test a scalar expression
    OpenDatabase(silo_data_path("wave*.silo database"))

    DefineScalarExpression("p2", "pressure*pressure")
    AddPlot("Pseudocolor", "p2")
    DrawPlots()

    QueryOverTime("Variable Sum", stride=10)
    SetActiveWindow(2)
    Test("TimeQuery_expr_02")
    
    # prepare for next test-set
    # delete plots from window 2 & l
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

    OpenDatabase(data_path("pdb_test_data/dbA00.pdb"))

    DefineScalarExpression("m", "matvf(material, 1)")
    AddPlot("Pseudocolor", "m")
    DrawPlots()

    QueryOverTime("Variable Sum")
    SetActiveWindow(2)
    Test("TimeQuery_expr_03")
    
    # prepare for next test-set
    # delete plots from window 2 & l
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()



def TestTransientVariable():
    #bug '4906

    # Do what is necessary to get access to the transient variable,
    # because QueryOverTime requires an active drawn plot.
    db = silo_data_path("wave_tv*.silo database") 
    OpenDatabase(db)
    SetTimeSliderState(17)
    ReOpenDatabase(db)
    AddPlot("Pseudocolor", "transient")
    DrawPlots()

    qt = GetQueryOverTimeAttributes()
    qt.timeType = qt.Timestep
    SetQueryOverTimeAttributes(qt)

    QueryOverTime("Variable Sum")

    SetActiveWindow(2)
    InitAnnotation()
    Test("TimeQuery_trans_01")

    DeleteAllPlots()
    SetActiveWindow(1)

    pick = GetPickAttributes()
    pick.doTimeCurve = 1
    pick.timePreserveCoord = 0
    SetPickAttributes(pick)
    PickByNode(327)

    pick.doTimeCurve = 0
    pick.timePreserveCoord = 1
    SetPickAttributes(pick)

    SetActiveWindow(2)
    InitAnnotation()
    Test("TimeQuery_trans_02")

    # Prepare for next test
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestSpecifyTimeQueryWindow():
    # bug '5163
    OpenDatabase(silo_data_path("wave.visit"))

    AddPlot("Pseudocolor", "pressure") 
    DrawPlots()

    qt = GetQueryOverTimeAttributes()
    qt.timeType = qt.Timestep
    SetQueryOverTimeAttributes(qt)

    QueryOverTime("3D surface area")

    SetActiveWindow(2)
    InitAnnotation()
    Test("SpecifyTimeQueryWindow_01")
    DeleteAllPlots()

    SetActiveWindow(1)
    TurnMaterialsOff(("1 barrier"))
    DrawPlots()

    qot = GetQueryOverTimeAttributes()
    qot.createWindow = 0
    qot.windowId = 3
    SetQueryOverTimeAttributes(qot)

    QueryOverTime("3D surface area")

    SetActiveWindow(3)
    InitAnnotation()
    Test("SpecifyTimeQueryWindow_02")
    DeleteAllPlots()

    SetActiveWindow(1)
    TurnMaterialsOff(("2 water"))
    DrawPlots()

    qot.windowId = 2
    SetQueryOverTimeAttributes(qot)

    QueryOverTime("3D surface area")
    SetActiveWindow(2)
    InitAnnotation()
    Test("SpecifyTimeQueryWindow_03")

    # Prepare for next test
    DeleteAllPlots()
    DeleteWindow()
    SetActiveWindow(3)
    DeleteWindow()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestTimeVaryingSIL():
    #bug '5473
    OpenDatabase(data_path("samrai_test_data/sil_changes/dumps.visit"))


    cfileName = "./temp.curve"
    curveFile = open(cfileName, "wt")
    curveFile.write("#3D surface area\n")
    nframes = TimeSliderGetNStates()
    for i in range(nframes):
        Query("3D surface area")
        val = GetQueryOutputValue()
        curveFile.write("%g %g\n" % (i, val))
        TimeSliderNextState()

    curveFile.close()

    AddWindow()
    SetActiveWindow(2)
    DeleteAllPlots()
    OpenDatabase(cfileName)
    AddPlot("Curve", "3D surface area")
    DrawPlots()

    SetActiveWindow(1)

    # Go ahead and use default plot for now.
    qt = GetQueryOverTimeAttributes()
    qt.timeType = qt.Timestep
    qt.createWindow = 0
    qt.windowId = 2
    SetQueryOverTimeAttributes(qt)

    QueryOverTime("3D surface area")

    SetActiveWindow(2)
    InitAnnotation()
    cv = GetViewCurve();
    cv.domainCoords = (-0.534115, 10.5341)
    cv.rangeCoords = (4029.87, 5856.13)
    SetViewCurve(cv)

    SetActivePlots((0, 1))
    c = CurveAttributes()
    c.showPoints = 1
    SetPlotOptions(c)

    Query("Area Between Curves")
    s = GetQueryOutputString()
    text = CreateAnnotationObject("Text2D")
    text.text =  s
    text.height = 0.02
    text.position = (0.55, 0.4)
    
    Test("TimeQuery_sil_01")

    text.Delete()
    os.unlink(cfileName)

    # Prepare for next test
    DeleteAllPlots()
    SetActiveWindow(2)
    DeleteWindow()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestQueryAfterQueryOverTime():
    # bug '5823
    OpenDatabase(silo_data_path("wave_tv.visit"))

    SetTimeSliderState(17)
    ReOpenDatabase(silo_data_path("wave_tv.visit"))

    AddPlot("Pseudocolor", "transient")
    DrawPlots()

    QueryOverTime("Volume")
    Query("Volume")
    s = GetQueryOutputString()
    QueryOverTime("Max")
    Query("Max")
    s = s + GetQueryOutputString()

    SetActiveWindow(2)
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

    # bug '6042
    OpenDatabase(silo_data_path("wave.visit"))

    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    TurnMaterialsOn()
    QueryOverTime("3D surface area", stride=10)

    SetActiveWindow(2)
    DeleteAllPlots()
    SetActiveWindow(1)

    Query("3D surface area")
    s = s + GetQueryOutputString()
    TestText("QueryAfterQueryOverTime", s)

    SetActiveWindow(2)
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestMili():
    # bug '6430
    OpenDatabase(data_path("mili_test_data/single_proc/m_plot.mili"))

    AddPlot("Pseudocolor", "Primal/node/nodvel/vz")
    DrawPlots()

    ResetQueryOverTimeAttributes()

    QueryOverTime("Volume")
    SetActiveWindow(2) 
    ResetView()
    InitAnnotation()
    Test("TimeQuery_mili_01")
    DeleteAllPlots()
    SetActiveWindow(1)
    QueryOverTime("Max")
    SetActiveWindow(2) 
    InitAnnotation()
    Test("TimeQuery_mili_02")
    DeleteAllPlots()
    SetActiveWindow(1)

    p = GetPickAttributes()
    p.doTimeCurve = 1
    p.timePreserveCoord = 0
    SetPickAttributes(p)
    NodePick(122, 161)

    p.doTimeCurve = 0
    SetPickAttributes(p)

    SetActiveWindow(2) 
    InitAnnotation()
    Test("TimeQuery_mili_03")
    DeleteAllPlots()

    SetActiveWindow(1)
    qvars = ("Primal/shell/edv1", "Primal/shell/edv2")
    QueryOverTime("TrajectoryByZone", element=242, vars=qvars)
    SetActiveWindow(2) 
    ResetView()
    InitAnnotation()
    Test("TimeQuery_mili_04")
    DeleteAllPlots()

    SetActiveWindow(1)
    DeleteAllPlots()

def MultiVarTimePick():
    OpenDatabase(silo_data_path("wave.visit"))

    AddPlot("Pseudocolor", "pressure")
    DrawPlots()

    pa = GetPickAttributes()
    pa.doTimeCurve = 1
    pa.timeCurveType = pa.Single_Y_Axis
    SetPickAttributes(pa)

    vars =("pressure", "v", "direction_magnitude")
    PickByNode(8837, vars)

    SetActiveWindow(2);
    InitAnnotation()
    Test("TimePick_MultiVar_01")
    DeleteAllPlots()

    SetActiveWindow(1)
    pa.timeCurveType = pa.Multiple_Y_Axes
    SetPickAttributes(pa)
    PickByNode(8837, vars)

    SetActiveWindow(2);
    Test("TimePick_MultiVar_02")
    DeleteAllPlots()

    # remove plots from window 1
    SetActiveWindow(1)
    DeleteAllPlots()

    OpenDatabase(data_path("mili_test_data/single_proc/m_plot.mili"))

    AddPlot("Pseudocolor", "Primal/shell/inteng")
    DrawPlots()

    pa.timePreserveCoord = 0
    pa.timeCurveType = pa.Single_Y_Axis
    SetPickAttributes(pa)

    vars = ("default", "Primal/shell/normal_magnitude")
    PickByZone(233, vars)

    SetActiveWindow(2);
    Test("TimePick_MultiVar_03")
    DeleteAllPlots()

    SetActiveWindow(1)
    pa.timeCurveType = pa.Multiple_Y_Axes
    SetPickAttributes(pa)
    PickByZone(233, vars)

    SetActiveWindow(2);
    Test("TimePick_MultiVar_04")
    DeleteAllPlots()

    SetActiveWindow(1)
    DeleteAllPlots()

def TestPickRangeTimeQuery():

    OpenDatabase(silo_data_path("wave_tv.visit"))
    SetTimeSliderState(17)

    AddPlot("Pseudocolor", "v")
    DrawPlots()

    pickAtts = GetPickAttributes()
    pickAtts.doTimeCurve = 0
    pickAtts.variables = ("default", "v")
    pickAtts.timeCurveType =  pickAtts.Single_Y_Axis
    SetPickAttributes(pickAtts)

    #
    # Return the curves without plotting, and show
    # highlights. 
    #
    pickAtts.showPickHighlight = 1
    SetPickAttributes(pickAtts)
    options = {}
    options["pick_range"] = "100-105, 100, 1"
    options["do_time"] = 0
    options["return_curves"] = 1
    output_dict = PickByZone(options)
    s = str(output_dict)

    Test("TimePickRange_00")
    TestText("TimePickRangeDict_00",s)
    ClearPickPoints()

    #
    # Plot the curves, but don't return them. 
    #
    pickAtts.showPickHighlight = 0
    SetPickAttributes(pickAtts)
    options = {}
    options["pick_range"] = "100-105, 100, 1"
    options["do_time"] = 1
    options["return_curves"] = 0
    options["start_time"] = 10
    options["end_time"] = 14
    options["stride"] = 2
    output_dict = PickByNode(options)
    s = str(output_dict)
    SetActiveWindow(2)
    Test("TimePickRange_01")
    TestText("TimePickRangeDict_01",s)
    ClearPickPoints()
    SetActiveWindow(1)

    #
    # Plot the curves, and return them. 
    #
    pickAtts.showPickHighlight = 0
    SetPickAttributes(pickAtts)
    options = {}
    options["pick_range"] = "100-105"
    options["do_time"] = 1
    options["return_curves"] = 1
    options["start_time"] = 20
    options["end_time"] = 60
    options["stride"] = 2
    output_dict = PickByNode(options)
    s = str(output_dict)

    SetActiveWindow(2)
    Test("TimePickRange_02")
    TestText("TimePickRangeDict_02",s)
    SetActiveWindow(1)

    ClearPickPoints()
    DeleteAllPlots()
    ResetPickLetter()

    SetActiveWindow(1)
    ClearPickPoints()
    DeleteAllPlots()
    ResetPickLetter()

def TestReturnValue():
    #
    # There used to be a bug where the return value
    # from previous picks would propagate to the following
    # time query. Let's make sure this isn't re-introduced. 
    #
    OpenDatabase(silo_data_path("wave.visit"))
    AddPlot("Pseudocolor", "v")
    DrawPlots()

    pickAtts = GetPickAttributes()
    pickAtts.doTimeCurve = 0
    pickAtts.variables = ("default", "v")
    pickAtts.timeCurveType =  pickAtts.Single_Y_Axis
    SetPickAttributes(pickAtts)

    time1   = NodePick(coord=(3, .5, 3), do_time=1, start_time=0, end_time=70)
    no_time = NodePick(coord=(2, .2, 2), do_time=0)
    time2   = NodePick(coord=(3, .5, 3), do_time=1, start_time=0, end_time=70)

    AssertEqual("Pick Updated", type(time1), type(time2))

    ClearPickPoints() 
    DeleteAllPlots()
    ResetPickLetter()

   
def TimeQueryMain():
    TestAllTimeQueries()
    TestFilledBoundary()
    TestOperators()
    TestExpressions()
    TestTransientVariable()
    TestSpecifyTimeQueryWindow()
    TestTimeVaryingSIL()
    TestQueryAfterQueryOverTime()
    TestMili()
    MultiVarTimePick()
    TestPickRangeTimeQuery()
    TestReturnValue()

# main
InitAnnotation()
SetCurvePlotDefaults()
TimeQueryMain()
Exit()


