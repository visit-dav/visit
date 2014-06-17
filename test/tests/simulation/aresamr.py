# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  aresamr.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D aresamr mesh
#
#  Programmer: Kathleen Biagas 
#  Date:       June 17, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("aresamr", "aresamr.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("aresamr00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("aresamr01", sim.metadata())

    AddPlot("Pseudocolor", "zonal_scalar")
    DrawPlots()
    Test("aresamr02")

    silr = SILRestriction()
    silr.TurnOnAll()
    silr.TurnOffSet(4)
    SetPlotSILRestriction(silr, 1)
    Test("aresamr03")
    silr.TurnOnAll()
    silr.TurnOffSet(3)
    SetPlotSILRestriction(silr, 1)
    Test("aresamr04")

    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
