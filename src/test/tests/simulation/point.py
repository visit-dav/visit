# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  point.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   -  point mesh
#
#  Programmer: Kathleen Biagas 
#  Date:       June 25, 2014 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("point", "point.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("point00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("point01", sim.metadata())

    # 2d mesh and points
    AddPlot("Mesh", "point3d")
    meshAtts = MeshAttributes()
    meshAtts.pointSizePixels = 3 
    SetPlotOptions(meshAtts)
    DrawPlots()
    Test("point02")

    v = GetView3D()
    v.viewNormal = (0, 1, 0)
    v.viewUp     = (0, 0, -1)
    SetView3D(v)

    Test("point03")

    DeleteAllPlots()

    AddPlot("Pseudocolor", "pointvar")
    pcAtts = PseudocolorAttributes()
    pcAtts.pointSizePixels = 3 
    SetPlotOptions(pcAtts)
    DrawPlots()
    Test("point04")

    ResetView()
    Test("point05")

    AddOperator("Project")
    DrawPlots()
    Test("point06")

    projAtts = ProjectAttributes()
    projAtts.projectionType = projAtts.ZYCartesian
    SetOperatorOptions(projAtts)
    Test("point07")

    projAtts.projectionType = projAtts.XRCylindrical
    SetOperatorOptions(projAtts)
    Test("point08")

    projAtts.projectionType = projAtts.ZRCylindrical
    SetOperatorOptions(projAtts)
    Test("point09")

    DeleteAllPlots() 

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
