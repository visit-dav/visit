# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  var.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#
#  Programmer: Kathleen Biagas 
#  Date:       June 6, 2014 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("var", "var.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("var00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("var01", sim.metadata())

    # 2d mesh and vars
    AddPlot("Mesh", "mesh2d")
    AddPlot("Pseudocolor", "zonal_scalar")
    AddPlot("Vector", "zonal_vector")
    VectorAtts = VectorAttributes()
    VectorAtts.scale = 0.5
    VectorAtts.colorByMag = 0
    VectorAtts.vectorColor = (255, 255, 255, 255)
    SetPlotOptions(VectorAtts)
    AddPlot("Label", "zonal_label")
    LabelAtts = LabelAttributes()
    LabelAtts.textHeight1 = 0.04
    SetPlotOptions(LabelAtts)
    DrawPlots()
    Test("var02")

    DeleteAllPlots()

    # 2d mesh and vars
    AddPlot("Mesh", "mesh3d")
    AddPlot("Pseudocolor", "nodal_scalar")
    AddPlot("Vector", "nodal_vector")
    VectorAtts = VectorAttributes()
    VectorAtts.scale = 0.5
    SetPlotOptions(VectorAtts)

    DrawPlots()
    Test("var03")

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
