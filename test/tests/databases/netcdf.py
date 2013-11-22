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
#    Brad Whitlock, Wed Oct 10 11:20:44 PDT 2007
#    Added tests for CCSM data.
#
#    Jeremy Meredith, Fri Aug  8 11:23:29 EDT 2008
#    Updated streamline settings to match new attribute fields.
#
#    Brad Whitlock, Mon 16:37:34 PST 2009
#    I removed the Streamline plot and I fixed some other things to get it
#    working again.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Brad Whitlock, Thu Sep  9 11:03:30 PDT 2010
#    I added tests for time-varying curves, FVCOM.
#
#    Kathleen Biagas, Fri Sep 21 10:12:15 MST 2012 
#    Removed pjoin so that tests can run on Windows.
#
# ----------------------------------------------------------------------------

def test0():
    TestSection("ADAPT files")
    OpenDatabase(data_path("netcdf_test_data/narac/adapt_etex_metfld_from_obs_102312.nc"))
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
    DrawPlots()
    Test("netcdf_0_02")

    # Plot something 2D
    DeleteAllPlots()
    AddPlot("Pseudocolor", "ustar")
    p = PseudocolorAttributes()
    p.skewFactor = 0.01
    p.scaling = p.Skew
    DrawPlots()
    Test("netcdf_0_03")
    DeleteAllPlots()

def test1():
    TestSection("LODI particle files")
    # First put in the terrain, though it is not a LODI particle file.
    OpenDatabase(data_path("netcdf_test_data/narac/adapt_etex_metfld_from_obs_102312.nc"))
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
    db = data_path("netcdf_test_data/narac/ppart.nc")
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

def test2():
    TestSection("LODI files")
    # This test is reserved for LODI once it is assured to be right.
    return

def test3():
    TestSection("Basic NETCDF reader")
    OpenDatabase(data_path("netcdf_test_data/narac/etex_fill_lambertcc__4000_001.elev"))
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
    t.scaleZ = 3
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
    v.viewNormal = (0, -0.5, 0.866025)
    v.focus = (0, 0, 1646)
    v.viewUp = (0, 0.866025, 0.5)
    v.viewAngle = 30
    v.parallelScale = 282140
    v.nearPlane = -564281
    v.farPlane = 564281
    v.imagePan = (0, 0)
    v.imageZoom = 1.43024
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 1646)
    SetView3D(v)
    InvertBackgroundColor()
    Test("netcdf_3_01")
    SetLight(0, oldLight)
    DeleteAllPlots()
    InvertBackgroundColor()

    # Try some curves
    OpenDatabase(data_path("netcdf_test_data/3252ATT-A1H.cdf"))
    AddPlot("Curve", "ATTN_55")
    DrawPlots()
    Test("netcdf_3_02")
    DeleteAllPlots()
    OpenDatabase(data_path("netcdf_test_data/AAtestCTD.nc"))
    AddPlot("Curve", "ST_70")
    DrawPlots()
    ResetView()
    Test("netcdf_3_03")
    DeleteAllPlots()

    # Try some files that I found on climate web sites.
    OpenDatabase(data_path("netcdf_test_data/pressure.cdf"))
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    ResetView()
    ToggleFullFrameMode() # Did this get set somehow by the curve plots?
    Test("netcdf_3_04")
    DeleteAllPlots()

    OpenDatabase(data_path("netcdf_test_data/aou.cdf"))
    AddPlot("Pseudocolor", "aou")
    AddOperator("Slice")
    s = SliceAttributes()
    s.axisType = s.ZAxis
    SetOperatorOptions(s)
    DrawPlots()
    ResetView()
    Test("netcdf_3_05")
    DeleteAllPlots()

    OpenDatabase(data_path("netcdf_test_data/NASA_vegetation_lai.cdf"))
    AddPlot("Pseudocolor", "lai")
    DrawPlots()
    v2 = View2DAttributes()
    v2.windowCoords = (-139.883, -44.1452, -18.7702, 74.4037)
    v2.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v2.fullFrameActivationMode = v2.Off  # On, Off, Auto
    v2.fullFrameAutoThreshold = 100
    SetView2D(v2)
    Test("netcdf_3_06")
    DeleteAllPlots()

    # Try a file that used to crash on Windows
    db = "netcdf_test_data/Case5_2D-Q.nc"
    OpenDatabase(data_path(db))
    AddPlot("Pseudocolor", "Q")
    DrawPlots()
    ResetView()
    Test("netcdf_3_07")
    SetTimeSliderState(29)
    Test("netcdf_3_08")
    DeleteAllPlots()
    CloseDatabase(data_path(db))

    TestSection("Basic NETCDF reader with zone-centered data")
    swa = SaveWindowAttributes()
    swa.width = 1000 
    swa.height = 1000
    swa.screenCapture = 0
    db = "netcdf_test_data/oase-mapdata.nc"
    OpenDatabase(data_path(db))
    AddPlot("Pseudocolor", "national_rivers_2D")
    DrawPlots()
    ResetView()
    Test("netcdf_3_09", swa)
    DeleteAllPlots()
    AddPlot("Pseudocolor", "as_zonal/national_rivers_2D")
    DrawPlots()
    ResetView()
    Test("netcdf_3_10", swa)
    DeleteAllPlots()
    CloseDatabase(data_path(db))

def test4():
    TestSection("CCSM reader")
    db = "netcdf_test_data/tas_mean_T63.nc"
    OpenDatabase(data_path(db))
    AddPlot("Pseudocolor", "tas")
    DrawPlots()
    ResetView()
    Test("netcdf_4_00")

    # Change to the last time state
    SetTimeSliderState(1187)
    Test("netcdf_4_01")

    # Change to the global representation of the data
    ChangeActivePlotsVar("global/tas")
    ResetView()
    Test("netcdf_4_02")
    DeleteAllPlots()
    CloseDatabase(data_path(db))

def test5():
    TestSection("Time-varying curves")
    db = "netcdf_test_data/timecurve.nc"
    OpenDatabase(data_path(db))
    AddPlot("Curve", "theta_1_1")
    c = CurveAttributes()
    c.showLabels = 0
    c.curveColor = (255,0,0,255)
    c.curveColorSource = c.Custom
    SetPlotOptions(c)
    cv = GetViewCurve()
    cv.domainCoords = (0, 9)
    cv.rangeCoords = (0.40657, 5)
    cv.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    cv.domainScale = cv.LINEAR  # LINEAR, LOG
    cv.rangeScale = cv.LINEAR  # LINEAR, LOG
    SetViewCurve(cv)
    DrawPlots()
    Test("netcdf_5_00")
    SetTimeSliderState(4)
    Test("netcdf_5_01")
    DeleteAllPlots()
    CloseDatabase(data_path(db))

def test6():
    TestSection("FVCOM reader ")
    db = "netcdf_test_data/chn_0001.nc"
    OpenDatabase(data_path(db))
    AddPlot("Pseudocolor", "Dens3{S,Theta,P}")
    DrawPlots()
    ResetView()
    Test("netcdf_6_00")
    SetTimeSliderState(47)
    Test("netcdf_6_01")
    DeleteAllPlots()
    CloseDatabase(data_path(db))

def main():
    test0()
    test1()
    test2()
    test3()
    test4()
    test5()
    test6()

main()
Exit()
