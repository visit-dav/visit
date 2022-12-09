# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scalar.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#
#  Programmer: Kathleen Biagas
#  Date:       June 18, 2014
#
#  Modifications:
#    Kathleen Biagas, Mon Nov 28, 2022
#    Replace obsolete Label text attributes with new versions.
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("scalar", "scalar.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("scalar00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("scalar01", sim.metadata())

    # 2d mesh and scalars
    AddPlot("Mesh", "mesh2d")
    AddPlot("Pseudocolor", "zonal")
    AddPlot("Label", "zonal")
    LabelAtts = LabelAttributes()
    LabelAtts.textFont1.scale = 7
    SetPlotOptions(LabelAtts)
    DrawPlots()
    Test("scalar02")

    DeleteAllPlots()

    # 2d mesh and scalars
    AddPlot("Mesh", "mesh3d")
    AddPlot("Pseudocolor", "nodal")
    AddPlot("Label", "nodal")
    LabelAtts = LabelAttributes()
    LabelAtts.textFont1.scale = 7
    LabelAtts.depthTestMode = LabelAtts.LABEL_DT_NEVER
    SetPlotOptions(LabelAtts)

    DrawPlots()
    Test("scalar03")

# Close down the simulation.
if started:
    sim.endsim()

Exit()
