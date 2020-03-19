# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  zerocopy.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Jul 22 16:07:12 PDT 2015
#
#  Modifications:
#
# ----------------------------------------------------------------------------
import string

def step(sim):
    sim.consolecommand("step")
    # Read from stderr to look for the echoed command. Sync.
    keepGoing = True
    while keepGoing:
        buf = sim.p.stderr.readline()
        print(buf)
        if "Command 'step'" in buf:
            keepGoing = False

def MemorySample(sim, label = ""):
    EL = GetEngineList(1)
    pa = GetProcessAttributes("engine", EL[0][0], EL[0][1])
    return (pa.memory[0], label)

def sv():
    v = GetView3D()
    v.viewNormal = (0.284499, -0.766043, 0.576401)
    v.focus = (0, 0.499486, 0)
    v.viewUp = (-0.137723, 0.562352, 0.815348)
    v.viewAngle = 30
    v.parallelScale = 1.49949
    v.nearPlane = -2.99897
    v.farPlane = 2.99897
    v.imagePan = (0.0351381, 0.0550703)
    v.imageZoom = 1.15316
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0.499486, 0)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

def meshtest(sim, mesh):
    m = [MemorySample(sim, "Initial")]
    TestSection("Mesh " + mesh)

    AddPlot("Pseudocolor", mesh + "/temperature")
    DrawPlots()
    sv()
    m = m + [MemorySample(sim, "Pseudocolor plot temperature")]
    Test("zerocopy_%s_00" % mesh)

    ChangeActivePlotsVar(mesh + "/vx")
    Test("zerocopy_%s_01" % mesh)
    m = m + [MemorySample(sim, "Change to vx")]

    ChangeActivePlotsVar(mesh + "/I")
    Test("zerocopy_%s_02" % mesh)
    m = m + [MemorySample(sim, "Change to I")]

    ChangeActivePlotsVar(mesh + "/J")
    Test("zerocopy_%s_03" % mesh)
    m = m + [MemorySample(sim, "Change to J")]

    ChangeActivePlotsVar(mesh + "/K")
    Test("zerocopy_%s_04" % mesh)
    m = m + [MemorySample(sim, "Change to K")]

    DeleteAllPlots()

    AddPlot("Vector", mesh + "/velocity")
    DrawPlots()
    Test("zerocopy_%s_05" % mesh)
    m = m + [MemorySample(sim, "Vector plot velocity")]

    DeleteAllPlots()
    AddPlot("Pseudocolor", mesh + "/temperature")
    DrawPlots()
    for i in range(20):
        step(sim)
        m = m + [MemorySample(sim, "Pseudocolor plot temperature iteration %d" % i)]
    DeleteAllPlots()
    return m

def SaveTestImage(name):
    # Save these images somewhat larger than a regular test case image
    # since the images contain a lot of text.
    swa = SaveWindowAttributes()
    swa.width = 800
    swa.height = 800
    swa.screenCapture = 0
    Test(name, swa)

def MeshNames():
    return ("soa_s", "soa_m", "soa_i", "aos_s", "aos_m", "aos_c")

def PlotMemoryCurve(filename, testname):
    OpenDatabase("simmem.curve")
    M = MeshNames()
    for mesh in M:
        AddPlot("Curve", mesh)
    DrawPlots()
    ResetView()
    SaveTestImage(testname)

# Do a simple, reliable test without quantifying since that would not be
# portable and would require rebaselining as VisIt changes.
def TestMemoryUsage(memory, testname):
    def AllMethodsUseLessMemoryThanCopying(memory):
        numSamples = len(memory["aos_c"])
        for i in range(numSamples):
            copyMem = memory["aos_c"][i][0]
            for k in list(memory.keys()):
                if k != "aos_c":
                    if memory[k][i][0] > copyMem:
                        return False
        return True
    if AllMethodsUseLessMemoryThanCopying(memory):
        TestText(testname, "OK: All methods use less memory than copying.")
    else:
        TestText(testname, "ERROR: Some methods used more memory than copying.")

def WriteData(filename, memory):
    print(memory)
    numSamples = len(memory["aos_c"])
    f = open(filename, "wt")
    M = MeshNames()
    f.write("event," + string.replace(str(M), "'", "")[1:-1] + "\n")
    for row in range(numSamples):
        s = memory["aos_c"][row][1] 
        for m in M:
            s = s + ", " + str(memory[m][row][0])
        f.write(s + "\n")
    f.close()

# Create our simulation object.
sim = TestSimulation("zerocopy", "zerocopy.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("zerocopy00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("zerocopy01", sim.metadata())

    memorySamples = {}
    f = open("simmem.curve", "wt")
    M = MeshNames()
    for mesh in M:
        m = meshtest(sim, mesh)
        f.write("# " + mesh + "\n")
        for i in range(len(m)):
            f.write("%g %g\n" % (i,m[i][0]))
        memorySamples[mesh] = m
    f.close()

    TestSection("Memory usage")
    TestMemoryUsage(memorySamples, "zerocopy02")
#    PlotMemoryCurve("simmem.curve", "zerocopy03")
#    WriteData("zerocopy.dat", memorySamples)

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
