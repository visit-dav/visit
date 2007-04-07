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
# ----------------------------------------------------------------------------

def InitAnnotations(): 
    a = AnnotationAttributes()
    a.axesFlag2D = 1
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

def Main():
    InitAnnotations()
    Test1()

Main()
Exit()
