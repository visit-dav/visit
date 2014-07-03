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

def step(sim):
    sim.consolecommand("step")
    # Read from stderr to look for the echoed command. Sync.
    keepGoing = True
    while keepGoing:
        buf = sim.p.stderr.readline()
        print buf
        if "Command 'step'" in buf:
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

    for i in range(10):
        step(sim)
        step(sim)
        sim.consolecommand("update")
        Test("csg%02d" % (i+3))

    DeleteAllPlots()   


# Close down the simulation.
if started:        
    sim.endsim()

Exit()
