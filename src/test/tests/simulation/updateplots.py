# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  updateplots.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#
#  Programmer: Kathleen Biagas 
#  Date:       June 18, 2014 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("updateplots", "updateplots.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("updateplots00", sim)

def step(sim):
    sim.consolecommand("step")
    # Read from stderr to look for the echoed command. Sync.
    keepGoing = True
    while keepGoing:
        buf = sim.p.stderr.readline()
        print(buf)
        if "Command 'step'" in buf:
            keepGoing = False


# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("updateplots01", sim.metadata())

    # 2d mesh and updateplotss
    #AddPlot("Mesh", "mesh2d")
    AddPlot("Pseudocolor", "zonal")
    AddPlot("Vector", "zvec")
    VectorAtts = VectorAttributes()
    VectorAtts.scale = 0.5
    VectorAtts.colorByMag = 0
    VectorAtts.vectorColor = (255, 255, 255, 255)
    SetPlotOptions(VectorAtts)
    DrawPlots()
    Test("updateplots02")

    i = 3
    times = "Times:\n"
    Query("Time")
    times = times + str(GetQueryOutputValue()) + "\n"
    for outer in range(6):
        for inner in range(3):
            step(sim)
        Query("Time")
        times = times + str(GetQueryOutputValue()) + "\n"
        Test("updateplots%02d"%i)
        i = i+1

    TestText("updateplots%02d"%i, times)
             
# Close down the simulation.
if started:        
    sim.endsim()

Exit()
