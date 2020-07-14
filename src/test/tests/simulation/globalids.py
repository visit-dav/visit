# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  globalids.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 3D unstructured mesh.
#              global node and cell ids
#              unstructured ghost cell generation from global ids
#
#  Programmer: Brad Whitlock
#  Date:       Tue Jun 17 16:32:51 PDT 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def step(sim):
    sim.consolecommand("step")
    # Read from stderr to look for the echoed command. Sync.
    keepGoing = True
    while keepGoing:
        buf = sim.p.stderr.readline()
        print(buf)
        if "Command step" in buf:
            keepGoing = False

def set_the_view():
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

def set_the_view2():
    v = GetView3D()
    v.viewNormal = (-0.542717, -0.70433, 0.457578)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.252732, 0.3826, 0.888675)
    v.viewAngle = 30
    v.parallelScale = 0.722842
    v.nearPlane = -1.44568
    v.farPlane = 1.44568
    v.imagePan = (-0.00135472, 0.013532)
    v.imageZoom = 1.12868
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

def add_time(times):
    Query("Time")
    t2 = times + str(GetQueryOutputValue()) + "\n"
    return t2

def start_time():
    return add_time("Times:\n")

def test0(sim):
    TestSection("Topologically 3D meshes in 3D")
    DefineScalarExpression("gnid", "global_nodeid(mesh)")
    DefineScalarExpression("gzid", "global_zoneid(mesh)")
    DefineScalarExpression("nid",  "nodeid(mesh)")

    AddPlot("Pseudocolor", "nid")
    DrawPlots()
    set_the_view()
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
    times = start_time()
    idx = 7
    for i in range(3):
        # Advance some steps. This should make the plots update.
        nsteps = 5
        for j in range(nsteps):
            step(sim)
            DrawPlots()
        times = add_time(times)
        SetPlotOptions(s)
        Test("globalids%02d" % idx)
        idx = idx + 1
        SetPlotOptions(s_clear)
        Test("globalids%02d" % idx)
        idx = idx + 1

    TestText("globalids%02d" % idx, times)
    idx = idx + 1
    DeleteAllPlots()

def hideplot(id):
    pl = GetPlotList()
    if pl.GetPlots(id).hiddenFlag == 0:
        SetActivePlots(id)
        HideActivePlots()

def showplot(id):
    pl = GetPlotList()
    if pl.GetPlots(id).hiddenFlag == 1:
        SetActivePlots(id)
        HideActivePlots()

def test1(sim):
    TestSection("Topologically 2D meshes in 3D")
    DefineScalarExpression("gnid2d", "global_nodeid(surface)")
    DefineScalarExpression("gzid2d", "global_zoneid(surface)")
    DefineScalarExpression("nid2d",  "nodeid(surface)")

    AddPlot("FilledBoundary", "surfacemat")
    fb = FilledBoundaryAttributes(1)
    fb.colorType = fb.ColorBySingleColor
    fb.singleColor = (0,0,0,255)
    fb.wireframe = 1
    fb.lineWidth = 3
    SetPlotOptions(fb)
    AddPlot("Subset", "Domains(surface)")
    AddPlot("Pseudocolor", "nid2d")
    DrawPlots()
    set_the_view2()

    idx = 0
    times = start_time()
    ntests = 4
    for i in range(3):
        ids = [idx+j for j in range(ntests)]
        # Show the Subset plot
        showplot(1)
        hideplot(2)
        Test("globalids_1_%02d" % ids[0])

        # Show the Pseudocolor plot
        hideplot(1)
        showplot(2)
        ChangeActivePlotsVar("nid2d")
        Test("globalids_1_%02d" % ids[1])

        ChangeActivePlotsVar("gnid2d")
        Test("globalids_1_%02d" % ids[2])

        ChangeActivePlotsVar("gzid2d")
        Test("globalids_1_%02d" % ids[3])
        SetActivePlots(0)
        times = add_time(times)

        # Take a step.
        showplot(1)
        step(sim)
        idx = idx + ntests

    TestText("globalids_1_%02d" % idx, times)
    DeleteAllPlots()

def main():
    # Create our simulation object.
    sim = TestSimulation("globalids", "globalids.sim2")
    sim.addargument("-echo")

    # Test that we can start and connect to the simulation.
    started, connected = TestSimStartAndConnect("globalids00", sim)

    # Perform our tests.
    if connected:
        # Make sure the metadata is right.
        TestSimMetaData("globalids01", sim.metadata())
        test0(sim)
        test1(sim)

    # Close down the simulation.
    if started:        
        sim.endsim()

main()
Exit()
