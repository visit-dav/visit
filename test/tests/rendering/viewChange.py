# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  viewChange.py
#
#  Tests:      mesh      - structured grid 
#              plots     - pc
#              operators - slice
#              selection - none
#
#  Defect ID:  VisIt00003350, VisIt000032263
#
#  Programmer: Kathleen Bonnell 
#  Date:       May 14, 2003 
#
#  Modifications:
#    Kathleen Bonnell, Tue Jul  8 21:04:45 PDT 2003 
#    Changed ViewChangeSliceFlip, to use Flip, then change so that
#    image remains the same .. necessary do to slice changes.
#
#    Kathleen Bonnell, Wed Jul 16 18:07:33 PDT 2003 
#    Added more full-frame tests: for Histogram plot, transparent plot,
#    and plots with ViewExtentsType set to 'actual'.
#
#    Mark C. Miller, Tue Mar 14 07:54:20 PST 2006
#    Changed how full-frame is turned on/off
#
#    Brad Whitlock, Wed Jul 26 14:31:57 PST 2006
#    Added testing for auto fullframe of some glyphed plots.
#
# ----------------------------------------------------------------------------

def InitAnnotation():
    # Turn off most annotation
    a = AnnotationAttributes()
    a.axes2D.visible = 1
    a.axes2D.xAxis.label.visible = 1
    a.axes2D.yAxis.label.visible = 1
    a.axes2D.xAxis.title.visible = 0
    a.axes2D.yAxis.title.visible = 0
    a.axes3D.visible = 0
    a.axes3D.triadFlag = 0
    a.axes3D.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)


def TestViewChangeSliceFlip():
    TestSection("Testing view changes with slice flip")
    OpenDatabase("../data/wave0000.silo")
    AddPlot("Pseudocolor", "pressure")
    AddOperator("Slice")
    slice = SliceAttributes()
    slice.project2d = 1
    slice.axisType = slice.ZAxis
    slice.flip = 1
    SetOperatorOptions(slice)
    DrawPlots()

    # We want to capture the image after the flip, to ensure
    # the frame and axes are still drawn correctly.
    slice.flip = 0
    SetOperatorOptions(slice)

    Test("ViewChangeSliceFlip")
    DeleteAllPlots()

def TestViewChangeFullFrame():
    TestSection("Testing view changes with fullframe")
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    # We want to capture the image after toggling full frame mode, 
    # to ensure frame and axes are still drawn correctly.

    # Turn full frame mode ON
    v = GetView2D()
    v.fullFrameActivationMode = v.On
    SetView2D(v)
    Test("ViewChangeFullFrame_01")

    # Does a transparent actor maintain the full frame mode? 
    p = PseudocolorAttributes()
    p.opacity = 0.5;
    SetPlotOptions(p);
    Test("ViewChangeFullFrame_02")


    p.opacity = 1.
    SetPlotOptions(p)
    # Turn full frame mode OFF
    ToggleFullFrameMode()
    AddOperator("Clip")
    c = ClipAttributes()
    c.plane1Origin = (0, 3, 0)
    c.plane1Normal = (0, -1, 0)
    SetOperatorOptions(c)
    SetViewExtentsType("actual")
    # Turn full frame mode ON
    ToggleFullFrameMode()
    Test("ViewChangeFullFrame_03")
    # Turn full frame mode OFF
    ToggleFullFrameMode()
    Test("ViewChangeFullFrame_04")
    # Turn full frame mode ON
    ToggleFullFrameMode()
    SetViewExtentsType("original")
    Test("ViewChangeFullFrame_05")

    DeleteAllPlots()

    OpenDatabase("../data/globe.silo")
    AddPlot("Histogram", "u")
    DrawPlots()
    ResetView()
    Test("ViewChangeFullFrame_06")
    h = HistogramAttributes()
    h.numBins = 5
    SetPlotOptions(h)
    Test("ViewChangeFullFrame_07")

    DeleteAllPlots()

def TestViewChangeFullFrameWithGlyphs():
    TestSection("Testing view changes with fullframe and glyphed plots")
    OpenDatabase("../data/fullframe.silo")
    AddPlot("Mesh", "fullframe")
    DrawPlots()
    ResetView()
    Test("ViewChangeAutoFF_00")

    AddPlot("Vector", "vec")
    v = VectorAttributes()
    v.nVectors = 800
    SetPlotOptions(v)
    DrawPlots()
    Test("ViewChangeAutoFF_01")

    DeleteAllPlots()

    # Make sure it kicks in when the vector plot is the only plot.
    AddPlot("Vector", "vec")
    v = VectorAttributes()
    v.nVectors = 800
    SetPlotOptions(v)
    DrawPlots()
    Test("ViewChangeAutoFF_02")

    DeleteAllPlots()

    # Test a Pseudocolor plot of point mesh data
    AddPlot("Pseudocolor", "rad")
    p = PseudocolorAttributes()
    p.pointType = p.Box
    p.pointSize = 5e+8
    SetPlotOptions(p)
    DrawPlots()
    Test("ViewChangeAutoFF_03")

    p.pointType = p.Axis
    SetPlotOptions(p)
    Test("ViewChangeAutoFF_04")

    DeleteAllPlots()

    # Test a Mesh plot of a point mesh
    AddPlot("Mesh", "pointmesh")
    m = MeshAttributes()
    m.pointType = m.Box
    m.pointSize = 5e+8
    SetPlotOptions(m)
    DrawPlots()
    Test("ViewChangeAutoFF_05")
    DeleteAllPlots()

def ViewChangeMain():
    InitAnnotation()
    TestViewChangeSliceFlip()
    TestViewChangeFullFrame()
    TestViewChangeFullFrameWithGlyphs()

# Call the main function
ViewChangeMain()
Exit()
