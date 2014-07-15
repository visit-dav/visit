# ----------------------------------------------------------------------------
#  MODES: serial, parallel, scalable
#  CLASSES: nightly
#
#  Test Case:  curve.py 
#
#  Tests:      Tests the curve plot
#
#  Programmer: Kathleen Bonnell 
#  Date:       August 3, 2006 
#
#  Modifications:
#    Brad Whitlock, Tue Nov 21 10:55:24 PDT 2006
#    Made it use enums for line style.
#
#    Mark C. Miller, Thu Jun 21 11:05:58 PDT 2007
#    Added tests for overlaying curves on 2D plots
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Bonnell, Fri Aug 13 16:10:56 MST 2010 
#    Added TestPointsAndSymbols to test various points/symbol settings.
#    Added TestTimeCue to test time-cue functionality
#
#    Kathleen Biagas, Thu Sep 12 09:08:00 PDT 2013
#    Added TestPolar to test new polar conversion capability.
#
#    Kathleen Biagas, Tue Jul 15 14:16:33 MST 2014
#    Added TestScale, which demonstrates the need to scale curves with values
#    greater than 1e37 or less than 1e-37.
#
# ----------------------------------------------------------------------------

def Test1():
    OpenDatabase(data_path("curve_test_data/distribution.ultra"))

    DefineCurveExpression("logED", "log10(<Exponential Distribution>)")
    DefineCurveExpression("logLap", "log10(<Laplace Distribution>)")
    AddPlot("Curve", "Exponential Distribution")
    DrawPlots()
    Test("curve_1_01")

    ChangeActivePlotsVar("logED")
    c = CurveAttributes()
    c.cycleColors = 0
    SetPlotOptions(c)
    ResetView()
    Test("curve_1_02")

    ChangeActivePlotsVar("Laplace Distribution")
    c = CurveAttributes()
    c.cycleColors = 0
    c.color = (0, 255, 0, 255)
    c.lineWidth = 2
    SetPlotOptions(c)
    ResetView()
    Test("curve_1_03")

    ChangeActivePlotsVar("logLap")
    c = CurveAttributes()
    c.cycleColors = 0
    c.color = (0, 255, 255, 255)
    c.lineStyle = c.DOT  # was 2
    SetPlotOptions(c)
    ResetView()
    Test("curve_1_04")

    v = GetViewCurve()
    v.domainCoords = (-19.3832, -19.3626)
    v.rangeCoords = (-8.71826, -8.71234)
    SetViewCurve(v)
    Test("curve_1_05")

    ChangeActivePlotsVar("Log Normal Distribution")
    ResetView()
    Test("curve_1_06")

    c.lineStyle = c.DOTDASH # was 3
    c.color = (120, 10, 150, 255)
    SetPlotOptions(c)

    v.domainCoords = (1.13181, 1.14079)
    v.rangeCoords = (0.346122, 0.350033)
    SetViewCurve(v)
    Test("curve_1_07")

    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/distribution.ultra"))


def TestOverlayCurves():
    TestSection("Overlay curves on 2D plots")

    # put up some 2D plots
    OpenDatabase(silo_data_path("ucd2d.silo"))

    AddPlot("Pseudocolor","d")    # id=0
    AddPlot("Mesh","ucdmesh2d")   # id=1
    SetActivePlots((1))
    AddOperator("Transform")
    ta=TransformAttributes()
    ta.doTranslate=1
    ta.translateY=2
    SetOperatorOptions(ta)
    DrawPlots()

    OpenDatabase(data_path("curve_test_data/ol_curveA.curve"))

    AddPlot("Curve","ol_curveA")  # id=2
    DrawPlots()
    Test("curve_2_01")

    OpenDatabase(data_path("curve_test_data/ol_curveB.curve"))

    AddPlot("Curve","ol_curveB")  # id=3
    DrawPlots()
    Test("curve_2_02")

    OpenDatabase(data_path("curve_test_data/ol_curveC.curve"))

    AddPlot("Curve","ol_curveC")  # id=4
    DrawPlots()
    Test("curve_2_03")

    # testing hiding/unhiding the 2D plots
    SetActivePlots(())
    SetActivePlots((0,1))
    HideActivePlots()
    #Test("curve_2_04")
    HideActivePlots()
    Test("curve_2_05")
    SetActivePlots((1))
    HideActivePlots()
    Test("curve_2_06")
    HideActivePlots()

    # test hiding some curves
    SetActivePlots((2,3,4))
    HideActivePlots()
    Test("curve_2_07")
    HideActivePlots()
    SetActivePlots((2))
    HideActivePlots()
    Test("curve_2_08")
    HideActivePlots()
    SetActivePlots((3))
    HideActivePlots()
    Test("curve_2_09")
    HideActivePlots()
    SetActivePlots((4))
    HideActivePlots()
    Test("curve_2_10")
    HideActivePlots()

    DeleteAllPlots()
    CloseDatabase(silo_data_path("ucd2d.silo"))

    CloseDatabase(data_path("curve_test_data/ol_curveA.curve"))

    CloseDatabase(data_path("curve_test_data/ol_curveB.curve"))

    CloseDatabase(data_path("curve_test_data/ol_curveC.curve"))


def TestPointsAndSymbols():
    TestSection("Points and Symbols")
    OpenDatabase(data_path("curve_test_data/ol_curveA.curve"))

    AddPlot("Curve","ol_curveA")  
    #points and lines
    curve = CurveAttributes()
    curve.showLabels = 0
    curve.showPoints = 1
    SetPlotOptions(curve)
    DrawPlots()
    ResetView()
    v = GetViewCurve()
    v.domainCoords = (-0.5, 5.5)
    v.rangeCoords = (-0.5, 3.5)
    SetViewCurve(v)
    Test("curve_3_01")

    #stride the points
    curve.pointStride = 3
    SetPlotOptions(curve)
    Test("curve_3_02")

    #Dynamic fill 
    curve.pointFillMode = curve.Dynamic
    SetPlotOptions(curve)  
    Test("curve_3_03")

    #change density
    curve.symbolDensity = 10
    SetPlotOptions(curve)  
    Test("curve_3_04")

    #Symbol 
    curve.symbol = curve.TriangleDown
    SetPlotOptions(curve)
    Test("curve_3_05")

    #Zoom 
    v.domainCoords = (2, 5)
    v.rangeCoords = (2, 4)
    SetViewCurve(v)  
    Test("curve_3_06")

    #Symbol 
    v.domainCoords = (-0.5, 5.5)
    v.rangeCoords = (-0.5, 3.5)
    SetViewCurve(v)  
    curve.symbol = curve.Plus
    SetPlotOptions(curve)
    Test("curve_3_07")

    #Static with symbols
    curve.pointFillMode = curve.Static
    curve.pointStride = 1
    curve.symbol = curve.Circle
    SetPlotOptions(curve)
    Test("curve_3_08")

    #Remove lines
    curve.showLines = 0 
    curve.symbol = curve.X
    SetPlotOptions(curve)
    Test("curve_3_09")

    #Remove lines
    curve.pointFillMode = curve.Dynamic 
    curve.symbolDensity = 30
    curve.symbol = curve.TriangleUp
    SetPlotOptions(curve)
    Test("curve_3_10")

    OpenDatabase(data_path("curve_test_data/ol_curveB.curve"))

    AddPlot("Curve", "ol_curveB")
    DrawPlots()
    c2 = CurveAttributes()
    c2.showLabels = 0
    c2.curveColorSource = c2.Custom
    c2.curveColor = (255, 0, 0, 255)
    SetPlotOptions(c2)
    AddOperator("Transform")
    ta2=TransformAttributes()
    ta2.doTranslate=1
    ta2.translateX=-0.35
    SetOperatorOptions(ta2)
    DrawPlots()
    DrawPlots()
    Test("curve_3_11")

    c2.showLines = 0
    c2.showPoints = 1
    c2.pointFillMode = c2.Dynamic
    c2.symbol = c2.Circle
    c2.symbolDensity = 30
    SetPlotOptions(c2)
    DrawPlots()
    Test("curve_3_12")


    OpenDatabase(data_path("curve_test_data/ol_curveC.curve"))

    AddPlot("Curve", "ol_curveC")
    DrawPlots()
    c3 = CurveAttributes()
    c3.showLabels = 0
    c3.showPoints = 1
    c3.curveColorSource = c2.Custom
    c3.curveColor = (0, 255, 0, 255)
    SetPlotOptions(c3)
    AddOperator("Transform")
    ta3=TransformAttributes()
    ta3.doTranslate=1
    ta3.translateX=0.25
    SetOperatorOptions(ta3)
    DrawPlots()
    Test("curve_3_13")


    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/ol_curveA.curve"))

    CloseDatabase(data_path("curve_test_data/ol_curveB.curve"))

    CloseDatabase(data_path("curve_test_data/ol_curveB.curve"))


def TestTimeCue():
    TestSection("Time Cue")
    OpenDatabase(data_path("curve_test_data/ol_curveC.curve"))

    AddPlot("Curve", "ol_curveC")
    c = CurveAttributes()
    c.showLabels = 0
    c.showPoints = 1
    c.doBallTimeCue = 1
    c.timeCueBallSize = 1
    c.ballTimeCueColor = (0, 255, 0, 255)
    SetPlotOptions(c)
    DrawPlots()

    v = GetViewCurve()
    v.domainCoords = (-0.25, 10.25)
    v.rangeCoords = (-0.25, 15.25)
    SetViewCurve(v)

    SuppressQueryOutputOn()
    Query("NumNodes")
    n = int(GetQueryOutputValue())
    for i in range(n-1):
        c.timeForTimeCue = i
        SetPlotOptions(c)
        s = "curve_4_%02d" % i
        Test(s)

    c.doLineTimeCue = 1
    c.lineTimeCueColor = c.ballTimeCueColor
    SetPlotOptions(c)
    s = "curve_4_%02d" % (n-1)
    Test(s)
   
    c.lineTimeCueWidth = 3
    SetPlotOptions(c)
    s = "curve_4_%02d" % n
    Test(s)
    n = n+ 1
    
    c.doCropTimeCue = 1
    SetPlotOptions(c)
    s = "curve_4_%02d" % n
    Test(s)
  

    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/ol_curveC.curve"))

def TestPolar():
    OpenDatabase(data_path("curve_test_data/rose_theta_r_rads.ultra"))
    AddPlot("Curve", "rose")
    c = CurveAttributes()
    c.curveColorSource = c.Custom
    c.color = (255, 0, 0, 255)
    c.showLabels = 0
    c.doLineTimeCue = 0
    c.doCropTimeCue = 0
    SetPlotOptions(c)
    DrawPlots()
    ResetView()
    Test("polar_curve_01")
    c.polarToCartesian = 1
    c.polarCoordinateOrder = c.Theta_R
    c.angleUnits = c.Radians
    SetPlotOptions(c)
    ResetView()
    Test("polar_curve_02")
    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/rose_theta_r_rads.ultra"))

    OpenDatabase(data_path("curve_test_data/rose_r_theta_rads.ultra"))
    AddPlot("Curve", "rose")
    c = CurveAttributes()
    c.curveColorSource = c.Custom
    c.color = (0, 0, 255, 255)
    c.showLabels = 0
    c.doLineTimeCue = 0
    c.doCropTimeCue = 0
    SetPlotOptions(c)
    DrawPlots()
    ResetView()
    Test("polar_curve_03")
    c.polarToCartesian = 1
    c.polarCoordinateOrder = c.R_Theta
    c.angleUnits = c.Radians
    SetPlotOptions(c)
    ResetView()
    Test("polar_curve_04")
    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/rose_r_theta_rads.ultra"))

    OpenDatabase(data_path("curve_test_data/spiral_r_theta_deg.ultra"))
    AddPlot("Curve", "spiral")
    c.color = (0, 255, 0, 255)
    c.polarToCartesian = 0
    SetPlotOptions(c)
    DrawPlots()
    Test("polar_curve_05")
    c.polarToCartesian = 1
    c.polarCoordinateOrder = c.R_Theta
    c.angleUnits = c.Degrees
    SetPlotOptions(c)
    ResetView()
    Test("polar_curve_06")
    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/spiral_r_theta_deg.ultra"))

    OpenDatabase(data_path("curve_test_data/circle.ultra"))
    AddPlot("Curve", "circle")
    c.color = (255, 0, 255, 255)
    c.polarToCartesian = 0
    SetPlotOptions(c)
    DrawPlots()
    Test("polar_curve_07")
    c.polarToCartesian = 1
    c.polarCoordinateOrder = c.Theta_R
    c.angleUnits = c.Degrees
    SetPlotOptions(c)
    ResetView()
    Test("polar_curve_08")
    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/circle.ultra"))

def TestScale():
    OpenDatabase(data_path("curve_test_data/need_scale.ultra"))
    AddPlot("Curve", "toobig")
    DrawPlots()
    Test("curve_scale_00")

    t = TransformAttributes()
    t.doScale =1
    t.scaleX=1e-15
    t.scaleY=1e-15
    t.scaleZ=1e-15
    AddOperator("Transform")
    SetOperatorOptions(t)
    DrawPlots()
    ResetView()
  
    Test("curve_scale_01")

    RemoveLastOperator()
    ChangeActivePlotsVar("toosmall")
    DrawPlots()
    ResetView()

    Test("curve_scale_02")

    t.scaleX=1e+15
    t.scaleY=1e+15
    t.scaleZ=1e+15
    AddOperator("Transform")
    SetOperatorOptions(t)
    DrawPlots()
    ResetView()

    Test("curve_scale_03")

    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/need_scale.ultra"))

def Main():
    Test1()
    TestOverlayCurves()
    TestPointsAndSymbols()
    TestTimeCue()
    TestPolar()
    TestScale()

Main()
Exit()
