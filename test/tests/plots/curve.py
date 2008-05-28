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
# ----------------------------------------------------------------------------

def InitAnnotations(): 
    a = AnnotationAttributes()
    TurnOffAllAnnotations(a)
    a.axesFlag2D = 1
    SetAnnotationAttributes(a)


def Test1():
    OpenDatabase("../data/distribution.ultra")
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
    CloseDatabase("../data/distribution.ultra")

def TestOverlayCurves():
    TestSection("Overlay curves on 2D plots")

    # put up some 2D plots
    OpenDatabase("../data/ucd2d.silo")
    AddPlot("Pseudocolor","d")    # id=0
    AddPlot("Mesh","ucdmesh2d")   # id=1
    SetActivePlots((1))
    AddOperator("Transform")
    ta=TransformAttributes()
    ta.doTranslate=1
    ta.translateY=2
    SetOperatorOptions(ta)
    DrawPlots()

    OpenDatabase("../data/ol_curveA.curve")
    AddPlot("Curve","ol_curveA")  # id=2
    DrawPlots()
    Test("curve_2_01")

    OpenDatabase("../data/ol_curveB.curve")
    AddPlot("Curve","ol_curveB")  # id=3
    DrawPlots()
    Test("curve_2_02")

    OpenDatabase("../data/ol_curveC.curve")
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
    CloseDatabase("../data/ucd2d.silo")
    CloseDatabase("../data/ol_curveA.curve")
    CloseDatabase("../data/ol_curveB.curve")
    CloseDatabase("../data/ol_curveC.curve")

def Main():
    InitAnnotations()
    Test1()
    TestOverlayCurves()

Main()
Exit()
