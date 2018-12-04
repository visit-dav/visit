# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  multiblock.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D structured mesh.
#              scalars
#              domain boundaries
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 18 12:04:01 PST 2013
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("multiblock", "multiblock.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("multiblock00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("multiblock01", sim.metadata())

    AddPlot("Mesh", "mesh2d")
    AddPlot("Pseudocolor", "domainID")
    DrawPlots()
    Test("multiblock02")

    # Force nodal centering to test domain boundary information.
    pc = PseudocolorAttributes(1)
    pc.centering = pc.Nodal
    SetPlotOptions(pc)
    Test("multiblock03")

    DeleteActivePlots()
    
    AddPlot("Subset", "Domains")
    DrawPlots()
    Test("multiblock04")

    ChangeActivePlotsVar("Blocks")
    Test("multiblock05")

    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
