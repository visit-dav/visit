# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  species.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#
#  Programmer: Kathleen Biagas
#  Date:       July 9, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Force MIR
#m = GetMaterialAttributes()
#m.forceMIR = 1
#SetMaterialAttributes(m)
    
# Create our simulation object.
sim = TestSimulation("species", "species.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("species00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("species01", sim.metadata())

    AddPlot("FilledBoundary", "Material")
    DrawPlots()
    Test("species02")

    DeleteActivePlots()
    AddPlot("Pseudocolor", "scalar")
    DrawPlots()
    Test("species03")

    DeleteAllPlots()
    TestSection("Force MIR")
    # Force MIR
    m = GetMaterialAttributes()
    m.forceMIR = 1
    SetMaterialAttributes(m)
 
    AddPlot("Pseudocolor", "scalar")
    DrawPlots()
    Test("species04")

    TestSection("Turn off materials")
    TurnMaterialsOff("Water")
    DrawPlots()
    Test("species05")

    TurnMaterialsOn("Water")
    TurnMaterialsOff("Membrane")
    DrawPlots()
    Test("species06")

    TurnMaterialsOn("Membrane")
    TurnMaterialsOff("Air")
    DrawPlots()
    Test("species07")

    TurnMaterialsOn("Air")

    TestSection("Turn off species")
    s = SILRestriction()
    for i in range(8):
        s.TurnOffSet(s.SetsInCategory("Species")[i])
        SetPlotSILRestriction(s)
        s.TurnOnSet(s.SetsInCategory("Species")[i])
        Test("species%02d"%(i+8))
    DeleteAllPlots()


# Close down the simulation.
if started:        
    sim.endsim()

Exit()
