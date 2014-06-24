# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ucdcurve.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - ucdcurve
#
#  Programmer: Kathleen Biagas 
#  Date:       Jun 24, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("ucdcurve", "ucdcurve.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("ucdcurve00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("ucdcurve01", sim.metadata())

    AddPlot("Pseudocolor", "nodal")
    pcAtts = PseudocolorAttributes()
    pcAtts.lineWidth = 4
    SetPlotOptions(pcAtts)
    DrawPlots()
    Test("ucdcurve02")

    times = "Times:\n"
    Query("Time")
    times = times + str(GetQueryOutputValue()) + "\n"

    # Read the 'VisIt connected' message
    buf = sim.p.stderr.readline() 
    for i in range(4):
        sim.consolecommand("step")
        # Read from stderr to look for the echoed command.
        buf = sim.p.stderr.readline()
    Test("ucdcurve03")

    Query("Time")
    times = times + str(GetQueryOutputValue()) + "\n"

    ChangeActivePlotsVar("zonal")

    for i in range(10):
        sim.consolecommand("step")
        buf = sim.p.stderr.readline()
    Test("ucdcurve04")

    Query("Time")
    times = times + str(GetQueryOutputValue()) + "\n"

    TestText("ucdcurve05", times) 
    
    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
