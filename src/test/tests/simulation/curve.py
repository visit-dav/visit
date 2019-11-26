# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  curve.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - curve
#
#  Programmer: Kathleen Biagas 
#  Date:       Jun 17, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("curve", "curve.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("curve00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("curve01", sim.metadata())

    AddPlot("Curve", "sine")
    curveAtts = CurveAttributes(1)
    curveAtts.showLabels = 0
    curveAtts.lineWidth = 2
    SetPlotOptions(curveAtts)
    DrawPlots()
    Test("curve02")
  
    # Read the 'VisIt connected' message
    buf = sim.p.stderr.readline()
    for i in range(25):
        sim.consolecommand("step")
        # Read from stderr to look for the echoed command.
        buf = sim.p.stderr.readline()
        print(buf)
        sim.consolecommand("update")

    Test("curve03")
    
    for i in range(25):
        sim.consolecommand("step")
        buf = sim.p.stderr.readline()
        print(buf)
        sim.consolecommand("update")
    Test("curve04")

    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
