# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  lineout.py #
#  Tests:      plots     - Curve
#              operators - Lineout
#
#  Defect ID:  none
#
#  Programmer: Brad Whitlock
#  Date:       Fri Jan 3 14:22:41 PST 2003
#
#  Modifications:
#    Kathleen Bonnell, Mon Mar 17 09:54:14 PST 2003
#    Added TestMultiVarLineout2D.
#
#    Kathleen Bonnell, Tue Dec 23 09:29:29 PST 2003 
#    Added TestSAMRAI.
#
#    Kathleen Bonnell, Thu Jul 29 11:59:35 PDT 2004 
#    Added tests for no-sampling version, renamed old Curve* tests to
#    indicate they were generated with-sampling. 
#
#    Kathleen Bonnell, Thu Aug  5 10:44:22 PDT 2004 
#    Added calls to ResetPickLetter() and ResetLineoutColor() at the end of
#    each test, so that failure on any one test won't necessarily affect the
#    tests that follow. 
#
#    Kathleen Bonnell, Wed Nov 24 11:38:55 PST 2004 
#    Modified the way that sampling gets turned on due to changes in Lineout
#    Attributes and GlobalLineoutAttributes.  Use global version to turn
#    sampling on and off. 
#
#    Kathleen Bonnell, Fri Feb  4 11:17:56 PST 2005 
#    Added TestDynamic, to test new global atts: curveOption and colorOption.
#
#    Hank Childs, Wed Feb 16 07:34:07 PST 2005
#    Rename variables that have unsupported characters.
#
#    Kathleen Bonnell, Wed Mar 23 17:58:20 PST 2005 
#    Added TestDynamic2. 
#
#    Kathleen Bonnell, hu May 19 11:26:39 PDT 2005 
#    Added TestTecPlot. 
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006
#    Added tests for GetOutputArray to Lineout2D.
#
#    Kathleen Bonnell, Wed Jun 28 15:57:58 PDT 2006
#    Added tests to TestDynamicLineout, testing having curves from  different
#    time-varying databases (same originating window and different originating
#    window) in same curve window, and update the curves via the originating
#    plots time-slider (bug '7002).
#
# ----------------------------------------------------------------------------

def InitAnnotation():
    # Turn off all annotation
    a = AnnotationAttributes()
    a.axesFlag2D = 1
    a.xAxisLabels2D = 0
    a.yAxisLabels2D = 0
    a.xAxisTitle2D = 0
    a.yAxisTitle2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def TestLineout2D(time, suffix):
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    # Set the colortable to one that has white at the bottom values.
    SetActiveContinuousColorTable("calewhite")
    pc = PseudocolorAttributes()
    pc.colorTableName = "Default"
    SetPlotOptions(pc)

    # Create the variable list.
    vars = ("default")

    # Do some lineouts.
    p0 = (-4.01261, 1.91818)
    p1 = (-0.693968, 4.448759)
    p2 = (4.144392, 1.713066)
    nsteps = 15
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        p3x = t * p2[0] + (1. - t) * p1[0]
        p3y = t * p2[1] + (1. - t) * p1[1]
        SetActiveWindow(1)
        Lineout(p0, (p3x, p3y), vars)

    if (time == 1):
        SetActiveWindow(1)
        Test("Lineout2d")

    if (time == 2):
        SetActiveWindow(1)
        oa = GetOutputArray(4, 2)
        s = ''.join(['%f, '% x for x in oa])
        s = '(' + s + ')'
        TestText("Lineout2d_output_04", s)
        oa = GetOutputArray(8, 2)
        s = ''.join(['%f, '% x for x in oa])
        s = '(' + s + ')'
        TestText("Lineout2d_output_08", s)

    SetActiveWindow(2)

    InitAnnotation()
    Test("CurvesFrom2d" + suffix)

    if (time == 2):
        oa = GetOutputArray(2)
        s = ''.join(['%f, '% x for x in oa])
        s = '(' + s + ')'
        TestText("Lineout2d_output_02", s)
        oa = GetOutputArray()
        s = ''.join(['%f, '% x for x in oa])
        s = '(' + s + ')'
        TestText("Lineout2d_output_15", s)

    # Delete the second window.
    DeleteWindow()
    DeleteAllPlots()
    ResetPickLetter()
    ResetLineoutColor()

def TestLineout3D(time, suffix):
    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()

    # Set the view
    v = View3DAttributes()
    v.viewNormal = (-0.65577, 0.350079, 0.668888)
    v.focus = (0, 0, 0)
    v.viewUp = (0.218553, 0.936082, -0.275655)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.perspective = 1
    SetView3D(v)

    # Do some lineouts
    vars = ("default")
    p0 = (-10., -10., -10.)
    P  = ((-10., -10., 10.), (-10., 10., -10.), (-10., 10., 10.),\
(10., -10., -10.), (10., -10., 10.), (10., 10., -10.), (10., 10., 10.))
    for p in P:
        SetActiveWindow(1)
        Lineout(p0, p, vars)

    if (time == 1):
        SetActiveWindow(1)
        pc = PseudocolorAttributes()
        pc.colorTableName = "xray"
        pc.opacity = 0.5
        SetPlotOptions(pc)
        Test("Lineout3d")

    SetActiveWindow(2)
    InitAnnotation()
    Test("CurvesFrom3d" + suffix)

    # Delete the second window.
    DeleteWindow()
    DeleteAllPlots()
    ResetPickLetter()
    ResetLineoutColor()

def TestMultiVarLineout2D(time, suffix):
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    # Do some lineouts
    vars = ("p", "u", "v")
    Y = (2, 3, 4)
    x1 = -4.5
    x2 = 4.5
    for y in Y:
        SetActiveWindow(1)
        Lineout((x1, y), (x2, y), vars)

    if (time == 1):
        SetActiveWindow(1)
        Test("MultiVarLineout2d")

    SetActiveWindow(2)
    InitAnnotation()
    Test("MultiVarCurvesFrom2d" + suffix)
    DeleteWindow()
    DeleteAllPlots()

    ResetPickLetter()
    ResetLineoutColor()

def TestSAMRAI(time, suffix):
    OpenDatabase("../data/samrai_test_data/sil_changes/dumps.visit")
    AddPlot("Pseudocolor", "Primitive Var _number_0")
    DrawPlots()

    # Set the colortable to one that has white at the bottom values.
    SetActiveContinuousColorTable("rainbow")
    pc = PseudocolorAttributes()
    pc.colorTableName = "Default"
    SetPlotOptions(pc)

    AddOperator("Slice", 1)
    slice = SliceAttributes()
    slice.originType = slice.Percent
    slice.originPercent = 18
    slice.axisType = slice.ZAxis
    slice.project2d = 1
    SetOperatorOptions(slice, 0, 1)

    ResetView()

    SetTimeSliderState(1)

    #Do some lineouts
    p0 = (3, 3)
    p1 = (0, 20)
    p2 = (30, 0)
    nsteps = 15
    for i in range(nsteps):
        t = float(i) / float(nsteps - 1)
        p3x = t * p2[0] + (1. - t) * p1[0]
        p3y = t * p2[1] + (1. - t) * p1[1]
        SetActiveWindow(1)
        Lineout(p0, (p3x, p3y))

    if (time == 1):
        SetActiveWindow(1)
        Test("LineoutSAMRAI")

    SetActiveWindow(2)
    InitAnnotation()
    Test("CurvesFromSAMRAI" + suffix)
    DeleteWindow()
    DeleteAllPlots()

    ResetPickLetter()
    ResetLineoutColor()

def TestSpecifyLineoutWindow(time, suffix):
    #window 1
    OpenDatabase("../data/dbA00.pdb")
    AddPlot("Pseudocolor", "mesh/ireg") 
    DrawPlots()
    ResetView()

    Lineout((0, 2.5), (5, 2.5)) 

    if (time == 1):
        SetActiveWindow(1)
        InitAnnotation()
        Test("LineoutSpecifyWindow_01")

    SetActiveWindow(2)
    InitAnnotation()
    Test("CurvesFromSpecifyWindow_01" + suffix)

    SetActiveWindow(1)
    CloneWindow()
    #window 3
    SetTimeSliderState(4)
    DrawPlots()

    
    gla = GetGlobalLineoutAttributes()
    gla.createWindow = 0
    gla.windowId = 4
    SetGlobalLineoutAttributes(gla)

    Lineout((0, 2.5), (5, 2.5)) 

    if (time == 1):
        SetActiveWindow(3)
        InitAnnotation()
        Test("LineoutSpecifyWindow_02")

    SetActiveWindow(4)
    InitAnnotation()
    Test("CurvesFromSpecifyWindow_02" + suffix)

    DeleteWindow()
    SetActiveWindow(3)
    DeleteWindow()
    SetActiveWindow(2)
    DeleteWindow()
    DeleteAllPlots()
    gla.createWindow = 1
    gla.windowId = 2
    SetGlobalLineoutAttributes(gla)

    ResetPickLetter()
    ResetLineoutColor()

def TestDynamicLineout(time, suffix):
    if (time == 1):
        return
    #window 1
    OpenDatabase("../data/wave.visit")
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    ResetView()

    Lineout((0, 0.5, 2.5), (10, 0.5, 2.5)) 

    gla = GetGlobalLineoutAttributes()
    gla.Dynamic = 1
    gla.curveOption = gla.UpdateCurve
    SetGlobalLineoutAttributes(gla)

    SetActiveWindow(1)
    t = 0
    for i in range (10):
        t += 5
        SetTimeSliderState(t)

    SetActiveWindow(2)
    InitAnnotation()
    ResetView()
    Test("CurvesFromDynamic_01")

    # go back to the beginning time state
    # and have new curves created for each new time 
    SetActiveWindow(1)
    t = 0
    SetTimeSliderState(t)

    gla.curveOption = gla.CreateCurve
    SetGlobalLineoutAttributes(gla)
    for i in range (7):
        t += 5
        SetTimeSliderState(t)

    # now have each new curve have its own color.
    gla.colorOption = gla.CreateColor
    SetGlobalLineoutAttributes(gla)
    for i in range (7):
        t += 5
        SetTimeSliderState(t)

    SetActiveWindow(2)
    InitAnnotation()
    ResetView()
    Test("CurvesFromDynamic_02")

    ResetPickLetter()
    ResetLineoutColor()
    # delete window 2
    DeleteWindow()
    # clear all plots from window 1
    DeleteAllPlots()

    dbs = ("../data/dbA00.pdb", "../data/dbB00.pdb", "../data/dbC00.pdb")
    OpenDatabase(dbs[0])
    AddPlot("Pseudocolor", "mesh/ireg")
    OpenDatabase(dbs[1])
    AddPlot("Pseudocolor", "mesh/ireg")
    DrawPlots()

    AddWindow()
    SetActiveWindow(2)
    DeleteAllPlots()
    OpenDatabase(dbs[2])
    AddPlot("Pseudocolor", "mesh/ireg")
    DrawPlots()

    gla.Dynamic = 1
    gla.curveOption = gla.UpdateCurve
    SetGlobalLineoutAttributes(gla)

    #Lineout for dbC00.pdb in window 2
    Lineout((5.0, 7.5, 0.), (10, 7.5, 0.))
   
    SetActiveWindow(1)
    SetActivePlots(1)

    #Lineout for dbB00.pdb in window 1
    Lineout((0, 8, 0), (5, 8, 0))

    SetActivePlots(0)
    #Lineout for dbA00.pdb in window 1
    Lineout((0, 3, 0), (5, 3, 0))


    SetActiveWindow(3)
    InitAnnotation()
    Test("CurvesFromDynamic_03")

    SetActiveWindow(1)
    SetActiveTimeSlider(dbs[1])
    SetTimeSliderState(15)
   
    SetActiveWindow(3)
    Test("CurvesFromDynamic_04")

    SetActiveWindow(1)
    SetActiveTimeSlider(dbs[0])
    SetTimeSliderState(3)


    SetActiveWindow(3)
    Test("CurvesFromDynamic_05")

    SetActiveWindow(2)
    SetTimeSliderState(29)
    
    SetActiveWindow(3) 
    Test("CurvesFromDynamic_06")

    ResetLineoutColor()
    ResetPickLetter()
    # delete window 3
    DeleteWindow()
    # delete window 2
    SetActiveWindow(2)
    DeleteWindow()
    # clear all plots from window 1
    DeleteAllPlots()

def TestDynamic2():
    # VisIt00006006 -- ensure  that 'ClearRefLines' will 'disconnect' the lineout 
    # from its originating plot, and won't update when orig plot changes time.
    OpenDatabase("../data/wave.visit")
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    ResetView()
    SetTimeSliderState(0) 
    Lineout((0, 0.5, 2.5), (10, 0.5, 2.5)) 

    SetActiveWindow(2)
    InitAnnotation()
    Test("Dynamic2_01")

    SetActiveWindow(1)
    gla = GetGlobalLineoutAttributes()
    gla.Dynamic = 1
    gla.curveOption = gla.UpdateCurve
    SetGlobalLineoutAttributes(gla)

    SetTimeSliderState(27)

    SetActiveWindow(2)
    Test("Dynamic2_02")

    SetActiveWindow(1)
    gla.Dynamic = 0
    SetGlobalLineoutAttributes(gla)
  
    SetTimeSliderState(52)

    SetActiveWindow(2)
    Test("Dynamic2_03")

    ResetPickLetter()
    ResetLineoutColor()
    DeleteWindow()
    DeleteAllPlots()


def TestTecPlot():
    # VisIt00006243 -- curve generated from Lineout looks reversed in X
    OpenDatabase("../data/tecplot_test_data/T3L3CLS17u.plt")
    AddPlot("Mesh", "mesh")
    AddPlot("Pseudocolor", "k")
    DrawPlots()
    ResetView()

    v = GetView2D()
    v.windowCoords = (0.340063, 0.340868, 0.00512584, 0.00572613 )
    SetView2D(v)

    Lineout((0.340505, 0.00565604, 0), (0.340291, 0.00514717, 0))

    InitAnnotation()
    Test("LineoutTecPlot_01")

    SetActiveWindow(2)
    InitAnnotation()
    Test("CurvesFromTecPlot_01")

    ResetPickLetter()
    ResetLineoutColor()
    # delete window 2
    DeleteWindow()
    # remove plots from window 1
    DeleteAllPlots()

def DoTests(t,s):
    TestLineout2D(t,s)
    TestLineout3D(t,s)
    TestMultiVarLineout2D(t,s)
    TestSAMRAI(t,s)
    TestSpecifyLineoutWindow(t,s)
    TestDynamicLineout(t,s)

def LineoutMain():
    InitAnnotation()
    la = GetGlobalLineoutAttributes()
    la.samplingOn = 1
    SetGlobalLineoutAttributes(la)

    DoTests(1, "_withSampling")

    la.samplingOn = 0
    SetGlobalLineoutAttributes(la)

    DoTests(2, "_noSampling")

    TestDynamic2()
    TestTecPlot()


# Call the main function
LineoutMain()
Exit()

