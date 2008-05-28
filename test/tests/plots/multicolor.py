# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  multicolor.py
#
#  Tests:      Tests setting colors using the multiColor field in some of
#              our plots.
#              Plots     - Boundary, Contour, FilledBoundary, Subset
#              Operators - Transform
#
#  Programmer: Brad Whitlock
#  Date:       Wed Apr 6 17:52:12 PST 2005
#
#  Modifications:
#
#    Mark C. Miller, Thu Jul 13 22:41:56 PDT 2006
#    Added test of user-specified material colors
#
# ----------------------------------------------------------------------------

def TestColorDefinitions(testname, colors):
    s = ""
    for c in colors:
        s = s + str(c) + "\n"
    TestText(testname, s)

def TestMultiColor(section, plotAtts, decreasingOpacity):
    # Get the current colors.
    m = plotAtts.GetMultiColor()

    # Test what the image currently looks like.
    Test("multicolor_%d_00" % section)

    # Change the colors all at once. We should have red->blue
    for i in range(len(m)):
        t = float(i) / float(len(m) - 1)
        c = int(t * 255.)
        m[i] = (255-c, 0, c, 255)
    plotAtts.SetMultiColor(m)
    SetPlotOptions(plotAtts)
    Test("multicolor_%d_01" % section)
    TestColorDefinitions("multicolor_%d_02" % section, plotAtts.GetMultiColor())

    # Change the colors another way. We should get green to blue
    for i in range(len(m)):
        t = float(i) / float(len(m) - 1)
        c = int(t * 255.)
        plotAtts.SetMultiColor(i, 0, 255-c, c)
    SetPlotOptions(plotAtts)
    Test("multicolor_%d_03" % section)
    TestColorDefinitions("multicolor_%d_04" % section, plotAtts.GetMultiColor())

    # Change the colors another way. We should get yellow to red but
    # the redder it gets, the more transparent it should also get.
    for i in range(len(m)):
        t = float(i) / float(len(m) - 1)
        c = int(t * 255.)
        if decreasingOpacity:
            plotAtts.SetMultiColor(i, (255, 255-c, 0, 255 - c))
        else:
            plotAtts.SetMultiColor(i, (255, 255-c, 0, c))
    SetPlotOptions(plotAtts)
    Test("multicolor_%d_05" % section)
    TestColorDefinitions("multicolor_%d_06" % section, plotAtts.GetMultiColor())

def test1():
    TestSection("Testing setting of multiColor in Boundary plot")
    # Set up the plot
    OpenDatabase("../data/rect2d.silo")
    AddPlot("Boundary", "mat1")
    b = BoundaryAttributes()
    b.lineWidth = 4
    DrawPlots()

    # Test the plot
    TestMultiColor(0, b, 0)

    # Delete the plots
    DeleteAllPlots()

def test2():
    TestSection("Testing setting of multiColor in Contour plot")
    # Set up the plot
    OpenDatabase("../data/noise.silo")
    AddPlot("Contour", "hardyglobal")
    c = ContourAttributes()
    c.contourNLevels = 20
    SetPlotOptions(c)
    DrawPlots()

    # Set the view.
    v = GetView3D()
    v.viewNormal = (-0.400348, -0.676472, 0.618148)
    v.focus = (0,0,0)
    v.viewUp = (-0.916338, 0.300483, -0.264639)
    v.parallelScale = 17.3205
    v.imagePan = (0, 0.0397866)
    v.imageZoom = 1.07998
    SetView3D(v)

    # Test the plot
    TestMultiColor(1, c, 0)

    # Delete the plots
    DeleteAllPlots()

def test3():
    TestSection("Testing setting of multiColor in FilledBoundary plot")
    # Set up the plots. First we want globe so we can see something inside
    # of the Subset plot to make sure that setting alpha works.
    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "w")
    p = PseudocolorAttributes()
    p.legendFlag = 0
    p.colorTableName = "xray"
    SetPlotOptions(p)
    OpenDatabase("../data/bigsil.silo")
    AddPlot("FilledBoundary", "mat")
    f = FilledBoundaryAttributes()
    f.legendFlag = 0
    SetPlotOptions(f)

    # Add an operator to globe to make it small.
    SetActivePlots(0)
    AddOperator("Transform", 0)
    t = TransformAttributes()
    t.doScale = 1
    t.scaleX, t.scaleY, t.scaleZ = 0.04, 0.04, 0.04
    t.doTranslate = 1
    t.translateX, t.translateY, t.translateZ = 0.5, 0.5, 0.5
    SetOperatorOptions(t)
    SetActivePlots(1)
    DrawPlots()

    # Set the view.
    v = GetView3D()
    v.viewNormal = (-0.385083, -0.737931, -0.554229)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (-0.922871, 0.310902, 0.227267)
    v.parallelScale = 0.866025
    v.imagePan = (-0.0165315, 0.0489375)
    v.imageZoom = 1.13247
    SetView3D(v)

    # Test the plot
    TestMultiColor(2, f, 1)

    # Delete the plots
    DeleteAllPlots()

def test4():
    TestSection("Testing setting of multiColor in Subset plot")
    # Set up the plots. First we want globe so we can see something inside
    # of the Subset plot to make sure that setting alpha works.
    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "w")
    p = PseudocolorAttributes()
    p.legendFlag = 0
    p.colorTableName = "xray"
    SetPlotOptions(p)
    OpenDatabase("../data/bigsil.silo")
    AddPlot("Subset", "domains")
    s = SubsetAttributes()
    s.legendFlag = 0
    SetPlotOptions(s)

    # Add an operator to globe to make it small.
    SetActivePlots(0)
    AddOperator("Transform", 0)
    t = TransformAttributes()
    t.doScale = 1
    t.scaleX, t.scaleY, t.scaleZ = 0.04, 0.04, 0.04
    t.doTranslate = 1
    t.translateX, t.translateY, t.translateZ = 0.5, 0.5, 0.5
    SetOperatorOptions(t)
    SetActivePlots(1)
    DrawPlots()

    # Set the view.
    v = GetView3D()
    v.viewNormal = (-0.385083, -0.737931, -0.554229)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (-0.922871, 0.310902, 0.227267)
    v.parallelScale = 0.866025
    v.imagePan = (-0.0165315, 0.0489375)
    v.imageZoom = 1.13247
    SetView3D(v)

    # Test the plot
    TestMultiColor(3, s, 1)

    # Delete the plots
    DeleteAllPlots()

def test5():
    TestSection("Testing user defined colors for FilledBoundary")

    ResetView()
    OpenDatabase("../data/globe_matcolors.silo")
    AddPlot("FilledBoundary","mat1")
    AddOperator("Slice")
    DrawPlots()

    Test("multicolor_matcolors")

    DeleteAllPlots()

def main():
    # Turn off 3D axes
    TurnOnAllAnnotations()
    a = GetAnnotationAttributes()
    a.axes3D.visible = 0
    a.userInfoFlag = 0
    SetAnnotationAttributes(a)

    test1()
    test2()
    test3()
    test4()
    test5()

# Run the tests
main()

Exit()

