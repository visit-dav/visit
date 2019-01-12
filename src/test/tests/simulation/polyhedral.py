# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  polyhedral.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              polyhedral   - 3D polyhedral.
#
#  Programmer: Kathleen Biagas 
#  Date:       June 26, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("polyhedral", "polyhedral.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("polyhedral00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("polyhedral01", sim.metadata())

    AddPlot("Mesh", "polyhedral")
    m = MeshAttributes(1)
    m.lineWidth = 2
    m.opaqueMode = m.Off
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0.04, 0.66, 0.74)
    v.viewUp = (-0.35, 0.71, -0.62)
    SetView3D(v)
    Test("polyhedral02")

    AddPlot("Pseudocolor", "nodal")
    DrawPlots()
    Test("polyhedral03")

    ChangeActivePlotsVar("zonal")
    SetActivePlots(0)
    # show the tesselation that occured on the polyhedral zone
    m.showInternal = 1
    SetPlotOptions(m)
    DrawPlots()
    Test("polyhedral04")
 
    SetActivePlots(1)
    DeleteActivePlots()

    Test("polyhedral05")

    DeleteAllPlots()


# Close down the simulation.
if started:        
    sim.endsim()

Exit()
