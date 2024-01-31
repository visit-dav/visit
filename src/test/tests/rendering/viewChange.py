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
#    Kathleen Bonnell, Tue Mar  3 13:20:57 PST 2009
#    Added testing for log-scaling of curves and 2d plots.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Wed Oct 21 11:19:52 PDT 2020
#    Added TestLargeValueLineoutWithLogScaling
#
#    Alister Maguire, Mon May 24 12:50:20 PDT 2021
#    Added TestViewChangeFullFrameWithLabels.
#
# ----------------------------------------------------------------------------

def InitAnnotation():
    # Turn off most annotation
    a = GetAnnotationAttributes()
    a.axes2D.visible = 1
    a.axes2D.xAxis.label.visible = 0
    a.axes2D.yAxis.label.visible = 0
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
    OpenDatabase(silo_data_path("wave0000.silo"))

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
    OpenDatabase(silo_data_path("curv2d.silo"))

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
    p.SetOpacityType(p.Constant)
    p.opacity = 0.5;
    SetPlotOptions(p);
    Test("ViewChangeFullFrame_02")


    p.SetOpacityType(p.FullyOpaque)
    SetPlotOptions(p)
    # Turn full frame mode OFF
    ToggleFullFrameMode()
    AddOperator("Clip")
    c = ClipAttributes()
    c.plane1Origin = (0, 3, 0)
    c.plane1Normal = (0, -1, 0)
    SetOperatorOptions(c)
    DrawPlots()
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

    OpenDatabase(silo_data_path("globe.silo"))

    AddPlot("Histogram", "u")
    DrawPlots()
    ResetView()
    Test("ViewChangeFullFrame_06")
    h = HistogramAttributes()
    h.numBins = 5
    SetPlotOptions(h)
    Test("ViewChangeFullFrame_07")

    # cleanup for next test
    v = GetView2D()
    v.fullFrameActivationMode = v.Auto
    SetView2D(v)

    DeleteAllPlots()

def TestViewChangeFullFrameWithGlyphs():
    TestSection("Testing view changes with fullframe and glyphed plots")
    OpenDatabase(silo_data_path("fullframe.silo"))

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

def TestViewChangeLogScaling2D():
    TestSection("Testing view changes with log scaling of 2D plots")

    OpenDatabase(silo_data_path("curv2d.silo"))

    AddPlot("Pseudocolor", "u")
    DrawPlots()
    ResetView()

    #7944  ensure that changing curve view scaling does not affect 2D.
    v = GetViewCurve()
    v.domainScale = v.LOG
    v.rangeScale = v.LOG
    SetViewCurve(v)

    Test("ViewChangeLogScale2D_00")

    #8563  using view window coords to put a plot into a range suitable 
    #      for log-scaling

    v = GetView2D()
    v.fullFrameActivationMode = v.Off
    wc = v.windowCoords
    v.windowCoords = (1e-13, 0.25, wc[2], wc[3])
    SetView2D(v)

    Test("ViewChangeLogScale2D_01")
    v.xScale = v.LOG 
    SetView2D(v)
    Test("ViewChangeLogScale2D_02")

    #clean up for next test
    v = GetView2D()
    v.xScale = v.LINEAR
    v.yScale = v.LINEAR
    SetView2D(v)
    ResetView()

    v = GetViewCurve()
    v.domainScale = v.LINEAR
    v.rangeScale = v.LINEAR
    SetViewCurve(v)
    DeleteAllPlots()

def TestViewChangeLogScalingCurves():
    TestSection("Testing view changes with log scaling of curves")
    # '8880:  Add curve plot.  Set scaling to log-log.  Add another curve plot.
    OpenDatabase(data_path("curve_test_data/distribution.ultra"))

    AddPlot("Curve", "Laplace Distribution")
    curveAtts = CurveAttributes()
    curveAtts.curveColorSource = curveAtts.Custom
    curveAtts.curveColor = (255, 0, 0, 255)
    SetPlotOptions(curveAtts)

    # For log scaling to work with these curves, we need to transform
    # first.

    trans = TransformAttributes()
    trans.doTranslate = 1
    trans.translateX = 30
    trans.translateY = 10
    SetDefaultOperatorOptions(trans)
    AddOperator("Transform")
    DrawPlots()
    ResetView()
    Test("ViewChangeLogScaleCurves_00")
   
    v = GetViewCurve()
    v.domainScale = v.LOG 
    v.rangeScale  = v.LOG 
    SetViewCurve(v)

    Test("ViewChangeLogScaleCurves_01")

    AddPlot("Curve", "Log Normal Distribution")
    curveAtts.curveColor = (0, 255, 0, 255)
    SetPlotOptions(curveAtts)
    AddOperator("Transform")
    DrawPlots()
    Test("ViewChangeLogScaleCurves_02")

    AddPlot("Curve", "Exponential Distribution")
    curveAtts.curveColor = (0, 0, 255, 255)
    SetPlotOptions(curveAtts)
    AddOperator("Transform")
    DrawPlots()
    Test("ViewChangeLogScaleCurves_03")

    v = GetViewCurve()
    v.domainScale = v.LINEAR 
    v.rangeScale  = v.LINEAR 
    SetViewCurve(v)

    DeleteAllPlots()

    #8660  add log scaling to a curve put in an appropriate range via the
    #      box operator
    OpenDatabase(data_path("curve_test_data/c063.curve"))

    AddPlot("Curve", "flat")
    curveAtts.curveColor = (0, 255, 255, 255)
    SetPlotOptions(curveAtts)
    AddPlot("Curve", "going_down")
    curveAtts.curveColor = (255, 0, 255, 255)
    SetPlotOptions(curveAtts)
    AddPlot("Curve", "going_up")
    curveAtts.curveColor = (255, 255, 0, 255)
    SetPlotOptions(curveAtts)
    AddPlot("Curve", "parabolic")
    curveAtts.curveColor = (255, 135, 0, 255)
    SetPlotOptions(curveAtts)

    DrawPlots()

    SetActivePlots((0, 1, 2, 3))
    AddOperator("Box")
    b = BoxAttributes()
    b.minx = 0.4
    b.maxx = 0.8
    b.miny = 0.000001 
    b.maxy = 10000 
    SetOperatorOptions(b) 
    DrawPlots()

    SetViewExtentsType("actual")

    Test("ViewChangeLogScaleCurves_04")

    v = GetViewCurve()
    v.domainScale = v.LOG 
    v.rangeScale  = v.LOG 
    SetViewCurve(v)
   
    Test("ViewChangeLogScaleCurves_05")

    #clean up for next test
    v = GetViewCurve()
    v.domainScale = v.LINEAR 
    v.rangeScale  = v.LINEAR 
    SetViewCurve(v)
    ResetView()
    DeleteAllPlots()

def TestLargeValueLineoutWithLogScaling():
    # github bug #5066
    OpenDatabase(silo_data_path("rect2d.silo"))
    DefineScalarExpression("tlarge", "t*1e19")
    AddPlot("Pseudocolor", "tlarge")
    DrawPlots()
    Lineout((0.5, 1.5, 0), (0.5, 0, 0))
    SetActiveWindow(2)
    # Instead of getting *default* curve plot atts,
    # get *current* plot's atts using `1` arg. This
    # is to change *only* the attrs we want to change.
    curveAtts = CurveAttributes(1)
    curveAtts.lineWidth = 3
    SetPlotOptions(curveAtts)
    Test("largeValueLineout")

    # save the curve as VTK so Mesh plot and 2D view can be tested.
    oldSwa = SaveWindowAttributes() 
    swa = SaveWindowAttributes() 
    swa.fileName="lineoutRes"
    swa.family=0
    swa.format=swa.VTK
    # need to ensure this is set (not the default on Windows)
    swa.outputToCurrentDirectory=1
    SetSaveWindowAttributes(swa)
    if TestEnv.params["scalable"] == 0:
        SaveWindow()
    else:
        # Turn of SR mode for the saveWindow, then turn it back on
        ra = GetRenderingAttributes()
        srm = ra.scalableActivationMode
        ra.scalableActivationMode = ra.Never
        SetRenderingAttributes(ra)
        SaveWindow()
        ra = GetRenderingAttributes()
        ra.scalableActivationMode = srm
        SetRenderingAttributes(ra)
    # restore previous settings
    SetSaveWindowAttributes(oldSwa)

    v = GetViewCurve()
    v.rangeScale  = v.LOG 
    SetViewCurve(v)
    Test("largeValueLineout_logScaling")

    v = GetViewCurve()
    v.rangeScale  = v.LINEAR 
    SetViewCurve(v)

    DeleteAllPlots()
    OpenDatabase("lineoutRes.vtk")
    AddPlot("Mesh", "mesh")
    meshAtts = MeshAttributes()
    meshAtts.lineWidth=3
    SetPlotOptions(meshAtts)
    DrawPlots()
    Test("largeValueMeshCurve")

    v = GetView2D()
    v.yScale = v.LOG 
    SetView2D(v)
    Test("largeValueMeshCurve_logScaling")
    v.yScale = v.LINEAR 
    SetView2D(v)
    DeleteWindow()
    DeleteAllPlots()
 
def TestViewChangeFullFrameWithLabels():
    TestSection("Testing view changes with fullframe and label plots")
    OpenDatabase(silo_data_path("curv2d.silo"))

    AddPlot("Mesh", "curvmesh2d")
    AddPlot("Label", "curvmesh2d")

    View2DAtts = View2DAttributes()
    View2DAtts.windowCoords = (-4.69855, 4.88342, 0.225185, 4.93329)
    View2DAtts.fullFrameActivationMode = View2DAtts.On
    SetView2D(View2DAtts)

    DrawPlots()

    Test("ViewChangeFullFrameWithLabels_00")

    DeleteAllPlots()
    ResetView()
    CloseDatabase(silo_data_path("curv2d.silo"))



def ViewChangeMain():
    InitAnnotation()
    TestViewChangeSliceFlip()
    TestViewChangeFullFrame()
    TestViewChangeFullFrameWithGlyphs()
    TestViewChangeLogScaling2D()
    TestViewChangeLogScalingCurves()
    TestLargeValueLineoutWithLogScaling()
    TestViewChangeFullFrameWithLabels()

# Call the main function
ViewChangeMain()
Exit()
