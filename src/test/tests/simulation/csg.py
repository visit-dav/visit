# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  csg.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              csg   - 3D csg mesh .
#
#  Programmer: Kathleen Biagas 
#  Date:       June 27, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("csg", "csg.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("csg00", sim)

def simcommand(sim, com):
    sim.consolecommand(com)
    # Read from stderr to look for the echoed command. Sync.
    keepGoing = True
    while keepGoing:
        buf = sim.p.stderr.readline()
        print(buf)
        if "Command '%s'"%com in buf:
            keepGoing = False

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("csg01", sim.metadata())

    AddPlot("Subset", "Regions")
    s = SubsetAttributes()
    s.smoothingLevel = 2
    SetPlotOptions(s)
    DrawPlots()
    Test("csg02")

    times = "Times:\n"
    SetQueryOutputToValue();
    times = times + str(Query("Time")) + "\n"
    for i in range(10):
        simcommand(sim, 'step')
        simcommand(sim, 'step')
        simcommand(sim, 'update')
        Test("csg%02d" % (i+3))
        times = times + str(Query("Time")) + "\n"
        
    TestText("csg13", times)
    DeleteAllPlots()   
    SetQueryOutputToString()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
