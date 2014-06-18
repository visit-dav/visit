# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  globalids.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 3D unstructured mesh.
#              global node and cell ids
#
#  Programmer: Brad Whitlock
#  Date:       Tue Jun 17 16:32:51 PDT 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("globalids", "globalids.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("globalids00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("globalids01", sim.metadata())

    DefineScalarExpression("gnid", "global_nodeid(mesh)")
    DefineScalarExpression("gzid", "global_zoneid(mesh)")
    DefineScalarExpression("nid",  "nodeid(mesh)")

    AddPlot("Pseudocolor", "nid")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (-0.707418, 0.404282, 0.579755)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.294715, 0.914272, -0.27794)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.imagePan = (0, 0.0589478)
    v.imageZoom = 1.0963
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)
    Test("globalids02")

    ChangeActivePlotsVar("gzid")
    DrawPlots()
    Test("globalids03")

    ChangeActivePlotsVar("gnid")
    DrawPlots()
    Test("globalids04")
    DeleteAllPlots()

    AddPlot("Subset", "Domains")
    DrawPlots()
    Test("globalids05")

    # Make sure that the plot is hollow
    s = SubsetAttributes(1)
    s_clear = SubsetAttributes(1)
    s_clear.opacity = 0.25
    s_clear.colorType = s_clear.ColorBySingleColor
    s_clear.singleColor = (200,200,200,255)
    SetPlotOptions(s_clear)
    DrawPlots()
    Test("globalids06")

    # Advance some steps and make sure that the plot
    # stays transparent. We're changing the size of the
    # domains at each time step and thus the global ids.
    times = "Times:\n"
    Query("Time")
    times = times + str(GetQueryOutputValue()) + "\n"
    idx = 7
    for i in xrange(3):
        # Advance some steps. This should make the plots update.
        nsteps = 5
        for j in xrange(nsteps):
            sim.consolecommand("step")
            DrawPlots()
        # Query the time
        Query("Time")
        times = times + str(GetQueryOutputValue()) + "\n"
        SetPlotOptions(s)
        Test("globalids%02d" % idx)
        idx = idx + 1
        SetPlotOptions(s_clear)
        Test("globalids%02d" % idx)
        idx = idx + 1

    TestText("globalids%02d" % idx, times)
    idx = idx + 1
    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
