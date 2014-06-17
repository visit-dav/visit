# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  amr.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D amr mesh
#
#  Programmer: Kathleen Biagas 
#  Date:       June 17, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("amr", "amr.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("amr00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("amr01", sim.metadata())

    AddPlot("Mesh", "amr")
    AddPlot("Subset", "Levels")
    DrawPlots()
    Test("amr02")

    SetActivePlots(1)
    ChangeActivePlotsVar("Patches")
    Test("amr03")

    SetActivePlots(0)
    DeleteActivePlots()
    silr = SILRestriction()
    silr.TurnOffSet(8)
    SetPlotSILRestriction(silr, 0)
    DrawPlots()
    Test("amr04")

    silr.TurnOnAll()
    silr.TurnOffSet(6)
    silr.TurnOffSet(8)
    SetPlotSILRestriction(silr, 0)
    DrawPlots()
    Test("amr05")

    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
