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
# ----------------------------------------------------------------------------

def InitAnnotation():
    # Turn off most annotations
    a = AnnotationAttributes()
    a.axesFlag2D = 1
    a.xAxisLabels2D = 1
    a.yAxisLabels2D = 1
    a.xAxisTitle2D = 1
    a.yAxisTitle2D = 1
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = a.Solid
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)


def TestAllTimeQueries():
    OpenDatabase("../data/wave.visit")
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
    SetPickAttributes(pa)
    PickByNode(15947)
    pa.doTimeCurve = 0
    SetPickAttributes(pa)
    SetActiveWindow(2);
    Test("AllTimeQueries_07")

    # delete window 2
    DeleteWindow()
    # remove plots from window 1
    DeleteAllPlots()

def TestFilledBoundary():
    #  bug '4708
    OpenDatabase("../data/wave.visit")
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
    OpenDatabase("../data/wave*.silo database")
    AddPlot("Pseudocolor", "pressure")
    AddOperator("Isovolume")
    iso = IsovolumeAttributes()
    iso.lbound = 0.1
    iso.ubound = 1.0
    SetOperatorOptions(iso)
    DrawPlots()
   
    qot = GetQueryOverTimeAttributes()
    qot.stride = 10
    SetQueryOverTimeAttributes(qot)
 
    SetActiveWindow(1)
    QueryOverTime("Volume") 
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
    QueryOverTime("2D area")
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
    OpenDatabase("../data/dbA00.pdb")
    AddPlot("Pseudocolor", "mesh/ireg")
    pa = PseudocolorAttributes()
    pa.minFlag = 1
    pa.maxFlag = 1
    pa.min = 1
    pa.max = 4
    SetPlotOptions(pa)
    DrawPlots()
    qt = GetQueryOverTimeAttributes()
    qt.startTimeFlag = 0
    qt.endTimeFlag = 0
    qt.stride = 1
    SetQueryOverTimeAttributes(qt)
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
    OpenDatabase("../data/wave*.silo database")
    DefineScalarExpression("p2", "pressure*pressure")
    AddPlot("Pseudocolor", "p2")
    DrawPlots()
    qt.stride = 10
    SetQueryOverTimeAttributes(qt)

    QueryOverTime("Variable Sum")
    SetActiveWindow(2)
    Test("TimeQuery_expr_02")
    
    # prepare for next test-set
    # delete plots from window 2 & l
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

    OpenDatabase("../data/dbA00.pdb")
    DefineScalarExpression("m", "matvf(material, 1)")
    AddPlot("Pseudocolor", "m")
    DrawPlots()
    qt.stride = 1
    SetQueryOverTimeAttributes(qt)

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
    db = "../data/wave_tv*.silo database"
    OpenDatabase(db)
    SetTimeSliderState(17)
    ReOpenDatabase(db)
    AddPlot("Pseudocolor", "transient")
    DrawPlots()

    qt = GetQueryOverTimeAttributes()
    qt.timeType = qt.Timestep
    qt.startTimeFlag = 0
    qt.endTimeFlag = 0
    qt.stride = 1
    SetQueryOverTimeAttributes(qt)

    QueryOverTime("Variable Sum")

    SetActiveWindow(2)
    InitAnnotation()
    Test("TimeQuery_trans_01")

    DeleteAllPlots()
    SetActiveWindow(1)

    pick = GetPickAttributes()
    pick.doTimeCurve = 1
    SetPickAttributes(pick)
    PickByNode(327)

    SetActiveWindow(2)
    InitAnnotation()
    Test("TimeQuery_trans_02")

    # Prepare for next test
    DeleteAllPlots()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestSpecifyTimeQueryWindow():
    # bug '5163
    OpenDatabase("../data/wave.visit")
    AddPlot("Pseudocolor", "pressure") 
    DrawPlots()

    qt = GetQueryOverTimeAttributes()
    qt.timeType = qt.Timestep
    qt.startTimeFlag = 0
    qt.endTimeFlag = 0
    qt.stride = 1
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
    SetActiveWindow(3)
    DeleteWindow()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestTimeVaryingSIL():
    #bug '5473
    OpenDatabase("../data/samrai_test_data/sil_changes/dumps.visit")

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
    qt.startTimeFlag = 0
    qt.endTimeFlag = 0
    qt.stride = 1
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
    text.width = 0.35
    text.position = (0.55, 0.4)
    
    Test("TimeQuery_sil_01")

    text.Delete()
    os.unlink(cfileName)

    # Prepare for next test
    DeleteAllPlots()
    SetActiveWindow(3)
    DeleteWindow()
    SetActiveWindow(1)
    DeleteAllPlots()

def TestQueryAfterQueryOverTime():
    # bug '5823
    OpenDatabase("../data/wave_tv.visit")
    SetTimeSliderState(17)
    ReOpenDatabase("../data/wave_tv.visit")
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
    OpenDatabase("../data/wave.visit")
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    TurnMaterialsOn()
    qot = GetQueryOverTimeAttributes()
    qot.stride = 10
    SetQueryOverTimeAttributes(qot)
    QueryOverTime("3D surface area")

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
    OpenDatabase("../data/mili_test_data/m_plot.mili")
    AddPlot("Pseudocolor", "derived/velocity/z")
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
    SetPickAttributes(p)
    NodePick(122, 161)

    p.doTimeCurve = 0
    SetPickAttributes(p)

    SetActiveWindow(2) 
    InitAnnotation()
    Test("TimeQuery_mili_03")
    DeleteAllPlots()

    SetActiveWindow(1)
    vars = ("edv1", "edv2")
    QueryOverTime("TrajectoryByZone", 242, vars)
    SetActiveWindow(2) 
    ResetView()
    InitAnnotation()
    Test("TimeQuery_mili_04")
    DeleteAllPlots()

    SetActiveWindow(1)
    DeleteAllPlots()

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

# main
InitAnnotation()    
TimeQueryMain()    
Exit()


