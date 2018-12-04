# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  interleave.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#
#  Programmer: Kathleen Biagas
#  Date:       June 17, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("interleave", "interleave.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("interleave00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("interleave01", sim.metadata())

    meshes2d = ("curv2d_static_float", "curv2d_static_double", 
                "curv2d_dynamic_float", "curv2d_dynamic_double",

                "point2d_static_float", "point2d_static_double", 
                "point2d_dynamic_float", "point2d_dynamic_double",

                "ucd2d_static_float", "ucd2d_static_double", 
                "ucd2d_dynamic_float", "ucd2d_dynamic_double")

    meshes3d = ("curv3d_static_float", "curv3d_static_double", 
                "curv3d_dynamic_float", "curv3d_dynamic_double",

                "point3d_static_float", "point3d_static_double", 
                "point3d_dynamic_float", "point3d_dynamic_double",

                "ucd3d_static_float", "ucd3d_static_double", 
                "ucd3d_dynamic_float", "ucd3d_dynamic_double")

    i = 2
    meshAtts = MeshAttributes()
    meshAtts.pointSizePixels = 10 
    meshAtts.lineWidth = 2 
    SetDefaultPlotOptions(meshAtts)

    v2 = GetView2D()
    v2.windowCoords = (-0.104083, 4.10408, -0.369428, 3.36943)
    v2.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    AddPlot("Mesh", "%s"%meshes2d[0])
    DrawPlots()
    SetView2D(v2)
    Test("interleave%02d" % i)
    i = i + 1

    for m in meshes2d[1:]:
        ChangeActivePlotsVar(m)
        DrawPlots()
        Test("interleave%02d" % i)
        i = i + 1

    DeleteAllPlots()

    AddPlot("Mesh", "%s"%meshes3d[0])
    DrawPlots()
    ResetView()
    Test("interleave%02d" % i)
    i = i + 1

    for m in meshes3d[1:]:
        ChangeActivePlotsVar(m)
        DrawPlots()
        Test("interleave%02d" % i)
        i = i + 1

    DeleteAllPlots()
        
# Close down the simulation.
if started:        
    sim.endsim()

Exit()
