# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mesh.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D and 3D structured mesh.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 18 12:04:01 PST 2013
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("mesh", "mesh.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("mesh00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("mesh01", sim.metadata())

    AddPlot("Mesh", "mesh2d")
    m = MeshAttributes(1)
    m.lineWidth = 1
    SetPlotOptions(m)
    DrawPlots()
    Test("mesh02")

    ChangeActivePlotsVar("mesh3d")
    Test("mesh03")
    DeleteAllPlots()

    # Disconnect and reconnect.
    sim.disconnect()
    sim.connect()

    # Make a new plot.
    AddPlot("Mesh", "mesh2d")
    SetPlotOptions(m)
    DrawPlots()
    Test("mesh04")

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
