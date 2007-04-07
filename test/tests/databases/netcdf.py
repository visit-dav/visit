# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  netcdf.py
#
#  Tests:      mesh      - 2D, 3D structured, single domain
#              plots     - Mesh, Pseudocolor, FilledBoundary
#              operators - Transform
#
#  Programmer: Brad Whitlock
#  Date:       Mon Aug 22 11:37:35 PDT 2005
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def test0(datapath):
    TestSection("ADAPT files")
    OpenDatabase(datapath + "narac/adapt_etex_metfld_from_obs_102312.nc")
    AddPlot("Pseudocolor", "u")
    AddPlot("Mesh", "main")
    SetActivePlots((0,1))
    AddOperator("Transform")
    t = TransformAttributes()
    t.scaleZ = 200
    t.doScale = 1
    SetOperatorOptions(t)
    DrawPlots()

    # Set the view
    v = View3DAttributes()
    v.viewNormal = (-0.70982, -0.515727, 0.479773)
    v.focus = (0, 0, 753888)
    v.viewUp = (0.359948, 0.319901, 0.876414)
    v.viewAngle = 30
    v.parallelScale = 2.38503e+06
    v.nearPlane = -4.77005e+06
    v.farPlane = 4.77005e+06
    v.imagePan = (0.000886525, 0.00976909)
    v.imageZoom = 0.952092
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 753888)
    SetView3D(v)
    Test("netcdf_0_00")

    # Set up another view.
    v1 = View3DAttributes()
    v1.viewNormal = (-0.434823, -0.389027, -0.812149)
    v1.focus = (0, 0, 753888)
    v1.viewUp = (-0.545125, -0.604143, 0.581249)
    v1.viewAngle = 30
    v1.parallelScale = 2.38503e+06
    v1.nearPlane = -4.77005e+06
    v1.farPlane = 4.77005e+06
    v1.imagePan = (0.00461106, -0.0368703)
    v1.imageZoom = 0.952092
    v1.perspective = 1
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (0, 0, 753888)
    SetView3D(v1)
    Test("netcdf_0_01")

    # Plot the terrain by stripping away most of the air. 
    SetActivePlots(1)
    DeleteActivePlots()
    AddOperator("IndexSelect")
    idxsel = IndexSelectAttributes()
    idxsel.dim = idxsel.ThreeD
    idxsel.zMax = 1
    SetOperatorOptions(idxsel)

    v2 = View3DAttributes()
    v2.viewNormal = (-0.394106, -0.0944361, 0.9142)
    v2.focus = (0, 0, 508267)
    v2.viewUp = (0.867549, 0.290123, 0.403965)
    v2.viewAngle = 30
    v2.parallelScale = 2.31912e+06
    v2.nearPlane = -4.63825e+06
    v2.farPlane = 4.63825e+06
    v2.imagePan = (0.012587, 0.0291994)
    v2.imageZoom = 1.08833
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0, 0, 508267)
    SetView3D(v2)
    Test("netcdf_0_02")

    # Plot the wind using a streamline plot.
    AddPlot("Streamline", "wind")
    s = StreamlineAttributes()
    s.stepLength = 1.
    s.maxTime = 1e+06
    s.lineStart  = (-1e+06, -1.25e+06, 500000)
    s.lineEnd  = (1e+06, -1.25e+06, 500000)
    s.pointDensity = 15
    s.sourceType = s.SpecifiedLine
    SetPlotOptions(s)
    AddOperator("Transform")
    SetOperatorOptions(t)
    DrawPlots()
    Test("netcdf_0_03")

    # Plot something 2D
    DeleteAllPlots()
    AddPlot("Pseudocolor", "ustar")
    p = PseudocolorAttributes()
    p.skewFactor = 0.01
    p.scaling = p.Skew
    DrawPlots()
    Test("netcdf_0_04")
    DeleteAllPlots()

def test1(datapath):
    TestSection("LODI particle files")
    # First put in the terrain, though it is not a LODI particle file.
    OpenDatabase(datapath + "narac/adapt_etex_metfld_from_obs_102312.nc")
    AddPlot("Pseudocolor", "u")
    AddOperator("Transform", 1)
    t = TransformAttributes()
    t.scaleZ = 200
    t.doScale = 1
    SetOperatorOptions(t)
    AddOperator("IndexSelect")
    idxsel = IndexSelectAttributes()
    idxsel.dim = idxsel.ThreeD
    idxsel.zMax = 1
    SetOperatorOptions(idxsel)
    DrawPlots()

    # Now add the LODI particle file.
    db = datapath + "narac/ppart.nc"
    OpenDatabase(db)
    AddPlot("FilledBoundary", "sourceid")
    AddOperator("Transform")
    SetOperatorOptions(t)
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.57737, -0.765477, 0.28406)
    v.focus = (0, 0, 265602)
    v.viewUp = (0.165375, 0.231057, 0.958782)
    v.viewAngle = 30
    v.parallelScale = 2.27828e+06
    v.nearPlane = -4.55655e+06
    v.farPlane = 4.55655e+06
    v.imagePan = (0, 0)
    v.imageZoom = 3.17577
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 265602)
    SetView3D(v)
    Test("netcdf_1_00")

    # Do a Label plot
    AddPlot("Label", "sourceid")
    AddOperator("Transform")
    SetOperatorOptions(t)
    L = LabelAttributes()
    L.restrictNumberOfLabels = 0
    L.textHeight1 = 0.04
    SetPlotOptions(L)
    DrawPlots()
    SetActivePlots(1)
    f = FilledBoundaryAttributes()
    f.pointSizePixels = 10
    SetPlotOptions(f)
    v2 = View3DAttributes()
    v2.viewNormal = (-0.57737, -0.765477, 0.28406)
    v2.focus = (0, 0, 265602)
    v2.viewUp = (0.165375, 0.231057, 0.958782)
    v2.viewAngle = 30
    v2.parallelScale = 2.27828e+06
    v2.nearPlane = -4.55655e+06
    v2.farPlane = 4.55655e+06
    v2.imagePan = (-0.0022605, 0.0057171)
    v2.imageZoom = 111.554
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0, 0, 265602)
    SetView3D(v2)
    Test("netcdf_1_01")

    # Delete the label plot and go forward in time
    SetActivePlots(2)
    DeleteActivePlots()
    SetView3D(v)
    SetActivePlots(1)
    f.pointSizePixels = 2
    SetPlotOptions(f)
    TimeSliderNextState()
    Test("netcdf_1_02")
    DeleteAllPlots()

def test2(datapath):
    TestSection("LODI files")
    # This test is reserved for LODI once it is assured to be right.
    return

def test3(datapath):
    TestSection("Basic NETCDF reader")
    OpenDatabase(datapath + "narac/etex_fill_lambertcc__4000_001.elev")
    AddPlot("Pseudocolor", "elevations")
    DrawPlots()
    Test("netcdf_3_00")

    # Elevate the data
    ClearWindow()
    AddOperator("Elevate")
    e = ElevateAttributes()
    e.useXYLimits = 1
    SetOperatorOptions(e)
    AddOperator("Transform")
    t = TransformAttributes()
    t.doScale = 1
    t.scaleX = 100
    t.scaleY = 100
    SetOperatorOptions(t)
    th = ThresholdAttributes()
    th.listedVarNames = ("elevations")
    th.lowerBounds = (1.0)
    SetDefaultOperatorOptions(th)
    AddOperator("Threshold")
    DrawPlots()
    # Set up a light
    oldLight = GetLight(0)
    newLight = GetLight(0)
    newLight.enabledFlag = 1
    newLight.type = newLight.Camera  # Ambient, Object, Camera
    newLight.direction = (-0.626, -0.427, -0.653)
    newLight.color = (255, 255, 255, 255)
    newLight.brightness = 1
    SetLight(0, newLight)
    # Set up the view
    v = View3DAttributes()
    v.viewNormal = (0, -0.642787, 0.766045)
    v.focus = (20300, 19950, 199.5)
    v.viewUp = (0, 0.766045, 0.642787)
    v.viewAngle = 30
    v.parallelScale = 27967.9
    v.nearPlane = -55935.8
    v.farPlane = 55935.8
    v.imagePan = (0, 0)
    v.imageZoom = 1.40076
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (20300, 19950, 199.5)
    SetView3D(v)
    InvertBackgroundColor()
    Test("netcdf_3_01")
    SetLight(0, oldLight)
    DeleteAllPlots()
    InvertBackgroundColor()

    # Try some curves
    OpenDatabase(datapath + "3252ATT-A1H.cdf")
    AddPlot("Curve", "ATTN_55")
    DrawPlots()
    Test("netcdf_3_02")
    DeleteAllPlots()
    OpenDatabase(datapath + "AAtestCTD.nc")
    AddPlot("Curve", "ST_70")
    DrawPlots()
    ResetView()
    Test("netcdf_3_03")
    DeleteAllPlots()

    # Try some files that I found on climate web sites.
    OpenDatabase(datapath + "pressure.cdf")
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    ResetView()
    Test("netcdf_3_04")
    DeleteAllPlots()

    OpenDatabase(datapath + "aou.cdf")
    AddPlot("Pseudocolor", "aou")
    AddOperator("Slice")
    s = SliceAttributes()
    s.axisType = s.ZAxis
    SetOperatorOptions(s)
    DrawPlots()
    ResetView()
    Test("netcdf_3_05")
    DeleteAllPlots()

    # This one could be time varying if we made the "T" dimension be time.
    OpenDatabase(datapath + "NASA_vegetation_lai.cdf")
    AddPlot("Pseudocolor", "lai")
    AddOperator("Slice")
    SetOperatorOptions(s)
    AddOperator("Transform")
    t = TransformAttributes()
    t.doScale = 1
    t.scaleY = -1.
    SetOperatorOptions(t)
    DrawPlots()
    v2 = View2DAttributes()
    v2.windowCoords = (39.8281, 144.722, -111.97, -15.0293)
    v2.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v2.fullFrameActivationMode = v2.Off  # On, Off, Auto
    v2.fullFrameAutoThreshold = 100
    SetView2D(v2)
    Test("netcdf_3_06")
    DeleteAllPlots()

def main():
    a = GetAnnotationAttributes()
    a.databaseInfoFlag = 0
    a. userInfoFlag = 0
    a.xAxisLabels = 0
    a.yAxisLabels = 0
    a.zAxisLabels = 0
    SetAnnotationAttributes(a)

    datapath = "../data/netcdf_test_data/"
    test0(datapath)
    test1(datapath)
    test2(datapath)
    test3(datapath)

main()
Exit()
