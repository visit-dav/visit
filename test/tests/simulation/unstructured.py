# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  unstructured.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              unstructured   - 3D unstructured.
#
#  Programmer: Kathleen Biagas 
#  Date:       June 18, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("unstructured", "unstructured.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("unstructured00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("unstructured01", sim.metadata())

    AddPlot("Mesh", "unstructured3d")
    m = MeshAttributes(1)
    m.lineWidth = 1
    SetPlotOptions(m)
    AddPlot("Subset", "Domains")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (0.707107, 0, 0.707107)
    SetView3D(v)
    Test("unstructured02")


# Close down the simulation.
if started:        
    sim.endsim()

Exit()
