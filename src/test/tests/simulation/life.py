# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  life.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D rectilinear mesh
#
#  Programmer: Kathleen Biagas 
#  Date:       June 17, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("life", "life.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("life00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("life01", sim.metadata())

    AddPlot("Pseudocolor", "life")
    DrawPlots()
    Test("life02")

    # Run the simulation for awhile to get a different image
    for i in range(100):
        sim.consolecommand("step")
    Test("life03")

    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
