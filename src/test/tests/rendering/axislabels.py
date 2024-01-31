# ----------------------------------------------------------------------------
#  MODES: serial, parallel, scalable
#  CLASSES: nightly
#
#  Test Case:  axislabels.py 
#
#  Tests:      Tests the axis labels
#
#  Programmer: Eric Brugger
#  Date:       Fri Dec 17 16:19:49 PST 2021
#
#  Modifications
#
# ----------------------------------------------------------------------------

def TestLog2D():
    OpenDatabase(data_path("curve_test_data/distribution.ultra"))

    swa = SaveWindowAttributes()
    swa.width = 600
    swa.height = 600
    swa.screenCapture = 0

    a = GetAnnotationAttributes()
    a.axes2D.visible = 1
    SetAnnotationAttributes(a)

    v = GetViewCurve()
    v.domainScale = v.LINEAR
    v.rangeScale = v.LOG
    SetViewCurve(v)

    AddPlot("Curve", "Exponential Distribution")
    c = CurveAttributes()
    c.curveColorSource = c.Custom
    c.curveColor = (255, 0, 0, 255)
    SetPlotOptions(c)
    DrawPlots()
    Test("AxisLabelsLog2D_00", swa)

    v.domainCoords = (1.601123657864678, 18.12971869918699)
    v.rangeCoords = (-7.130672752885499, 0.04760456234965804)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_01", swa)

    v.domainCoords = (0.638075437299828, 17.16667047862213)
    v.rangeCoords = (-10.48436074884018, -3.306083433605024)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_02", swa)

    v.domainCoords = (-8.812882729839023, 26.61762864576098)
    v.rangeCoords = (-14.58885955523007, 0.7984153727848649)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_03", swa)

    v.domainCoords = (-37.04643791101395, 54.85118382693591)
    v.rangeCoords = (-26.85053625423488, 13.06009207178967)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_04", swa)

    v.domainCoords = (-72.49874837389116, 90.30349428981312)
    v.rangeCoords = (-42.2472784051628, 28.4568342227176)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_05", swa)

    v.domainCoords = (-135.3046789498164, 153.1094248657384)
    v.rangeCoords = (-69.52354632680284, 55.73310214435764)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_06", swa)

    v.domainCoords = (2.563824255463072, 8.936313157343124)
    v.rangeCoords = (-4.113141545611167, -1.345604897058017)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_07", swa)

    v.domainCoords = (2.581093873083343, 8.953582774963396)
    v.rangeCoords = (-3.296916569328953, -0.5293799207758031)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_08", swa)

    v.domainCoords = (0.4742005234102636, 6.846689425290313)
    v.rangeCoords = (-3.235890402877946, -0.4683537543247966)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_09", swa)

    v.domainCoords = (0.6728011260433816, 7.045290027923435)
    v.rangeCoords = (-2.461620916030798, 0.3059157325223497)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_10", swa)

    v.domainCoords = (-3.883839369631302, 7.405413441872227)
    v.rangeCoords = (-2.289688582041886, 2.613171410605579)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_11", swa)

    v.domainCoords = (-5.087720394381069, 8.572275507538203)
    v.rangeCoords = (-2.632794183897802, 3.299666407205631)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_12", swa)

    v.domainCoords = (-4.680511844459359, 8.979484057459912)
    v.rangeCoords = (-1.569922247785004, 4.362538343318429)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_13", swa)

    # The curve doesn't intersect the window.
    v.domainCoords = (-1.565282256005422, 0.4651890936507914)
    v.rangeCoords = (1.046155226469858, 1.927977691841728)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_14", swa)

    # The curve doesn't intersect the window.
    v.domainCoords = (0.7831595313488587, 2.813630881005071)
    v.rangeCoords = (0.07581375463087496, 0.9576362200027442)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_15", swa)

    v.domainCoords = (0.1079984801721102, 1.494837732720581)
    v.rangeCoords = (-0.7549194057961169, -0.1526227967039314)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_16", swa)

    v.domainCoords = (7.150150811017356, 9.18062216067357)
    v.rangeCoords = (-3.893382901287159, -3.01156043591529)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_17", swa)

    v.domainCoords = (8.915537392097869, 10.94600874175408)
    v.rangeCoords = (-4.898933596142739, -4.017111130770868)
    SetViewCurve(v)
    Test("AxisLabelsLog2D_18", swa)

    DeleteAllPlots()
    CloseDatabase(data_path("curve_test_data/distribution.ultra"))

def Main():
    TestLog2D()

Main()
Exit()
