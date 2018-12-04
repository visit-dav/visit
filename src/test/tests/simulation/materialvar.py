# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  materialvar.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D structured mesh.
#              scalars defined on material sub regions.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 18 12:04:01 PST 2013
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("materialvar", "materialvar.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("materialvar00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("materialvar01", sim.metadata())

    AddPlot("Mesh", "mesh2d")
    AddPlot("FilledBoundary", "Material")
    DrawPlots()
    Test("materialvar02")
    DeleteActivePlots()

    scalars = ("scalar",
               "zonal_scalar_on_mat1",
               "zonal_scalar_on_mat2",
               "zonal_scalar_on_mat3",
               "zonal_scalar_on_mats12",
               "nodal_scalar_on_mat1",
               "nodal_scalar_on_mat2",
               "nodal_scalar_on_mat3",
               "nodal_scalar_on_mats23")
    i = 3
    for s in scalars:
        AddPlot("Pseudocolor", s)
        DrawPlots()
        Test("materialvar%02d" % i)
        DeleteActivePlots()
        i = i + 1
  
    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
