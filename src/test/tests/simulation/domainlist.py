# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  domainlist.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D,3D unstructured mesh.
#
#              Tests that VisIt's load balancer can accept different domain
#              lists from simulations and distribute work accordingly. If it
#              was to mess up then we would not get all domains for at least
#              one of the meshes.
#
#              This test must run in parallel.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Jun 26 17:19:08 PDT 2014
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
    v.viewNormal = (0.48257, -0.684101, 0.54693)
    v.focus = (0.95, 1.5, 0.5)
    v.viewUp = (-0.352936, 0.419625, 0.836272)
    v.viewAngle = 30
    v.parallelScale = 1.84459
    v.nearPlane = -3.68917
    v.farPlane = 3.68917
    v.imagePan = (0.0637817, 0.0407714)
    v.imageZoom = 1.04766
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.95, 1.5, 0.5)
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
    AddPlot("Subset", "Domains(mesh)")
    AddPlot("Subset", "Domains(surface)")
    SetActivePlots((0,1))
    AddOperator("Reflect")
    r = ReflectAttributes()
    r.octant = r.PXPYPZ  # PXPYPZ, NXPYPZ, PXNYPZ, NXNYPZ, PXPYNZ, NXPYNZ, PXNYNZ, NXNYNZ
    r.useXBoundary = 1
    r.specifiedX = 0
    r.useYBoundary = 0
    r.specifiedY = 1.5
    r.useZBoundary = 1
    r.specifiedZ = 0
    r.reflections = (1, 0, 1, 0, 0, 0, 0, 0)
    SetOperatorOptions(r)

    SetActivePlots(1)
    AddOperator("Transform", 0)
    trans = TransformAttributes()
    trans.doTranslate = 1
    trans.translateX = 1
    SetOperatorOptions(trans)
    DrawPlots()
    set_the_view()

    # Advance some steps and make sure that we get all domains for each mesh
    times = start_time()
    idx = 2
    for i in range(10):
        Test("domainlist%02d" % idx)
        times = add_time(times)
        step(sim)
        idx = idx + 1

    # Get the number of processors.
    e,s = GetEngineList(1)[0]
    text = "Engine Properties:\n" + str(GetEngineProperties(e,s)) + "\n" + times
    TestText("domainlist%02d" % idx, text)
    idx = idx + 1
    DeleteAllPlots()



def main():
    # Create our simulation object.
    sim = TestParallelSimulation("globalids_par", "globalids_par.sim2", 4)
    sim.addargument("-echo")

    # Test that we can start and connect to the simulation.
    started, connected = TestSimStartAndConnect("domainlist00", sim)

    # Perform our tests.
    if connected:
        # Make sure the metadata is right.
        TestSimMetaData("domainlist01", sim.metadata())
        test0(sim)

    # Close down the simulation.
    if started:        
        sim.endsim()

main()
Exit()
