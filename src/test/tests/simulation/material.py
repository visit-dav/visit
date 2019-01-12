# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  material.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              material   - 2D structured/unstructured material.
#              materials
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 18 12:04:01 PST 2013
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Force MIR
m = GetMaterialAttributes()
m.forceMIR = 1
SetMaterialAttributes(m)
    
# Create our simulation object.
sim = TestSimulation("material", "material.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("material00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("material01", sim.metadata())

    AddPlot("Mesh", "mesh2d")
    m = MeshAttributes(1)
    m.lineWidth = 1
    SetPlotOptions(m)
    AddPlot("FilledBoundary", "Material")
    DrawPlots()
    Test("material02")

    DeleteActivePlots()
    AddPlot("Pseudocolor", "scalar")
    DrawPlots()
    Test("material03")
    DeleteAllPlots()

    # Make a new plot.
    AddPlot("Mesh", "ucdmesh")
    SetPlotOptions(m)
    AddPlot("FilledBoundary", "MaterialFromArrays")
    DrawPlots()
    Test("material04")

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
