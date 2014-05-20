# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ghostcells.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D structured mesh.
#              scalars defined on material sub regions.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 18 12:04:01 PST 2013
#
#
#  NOTES: #3, #17 do not look correct.
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("ghostcells", "ghostcells.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("ghostcells00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("ghostcells01", sim.metadata())

    meshes = ("rect_blank", "curv_blank", "ucd_blank",
              "multi_domain_index", "multi_domain", "multi_types")
    scalars= ("rect_var", "curv_var", "ucd_var", 
              "multi_var", "multi_var_index", "multi_types_var")

    i = 2
    TestSection("Ghosting with meshes")
    for m in meshes:
        AddPlot("Subset", m)
        s = SubsetAttributes(1)
        if "multi" in m:
            s.opacity = 0.4
            SetPlotOptions(s)
        DrawPlots()
        ResetView()
        Test("ghostcells%02d" % i)

        s.colorType = s.ColorBySingleColor
        s.singleColor = (0,0,255,255)
        SetPlotOptions(s)
        AddOperator("InverseGhostZone")
        s.opacity = 1.
        SetPlotOptions(s)
        DrawPlots()
        Test("ghostcells%02d" % (i+1))
        DeleteAllPlots()
        i = i + 2
        
    TestSection("Ghosting with scalars")
    for s in scalars:
        AddPlot("Pseudocolor", s)
        pc = PseudocolorAttributes(1)
        if "multi" in s:
            pc.opacityType = pc.Constant
            pc.opacity = 0.4
            SetPlotOptions(pc)
        DrawPlots()
        ResetView() 
        Test("ghostcells%02d" % i)

        AddOperator("InverseGhostZone")
        pc.opacity = 1.0
        SetPlotOptions(pc)
        DrawPlots()
        Test("ghostcells%02d" % (i+1))
        DeleteAllPlots()
        i = i + 2
        
# Close down the simulation.
if started:        
    sim.endsim()

Exit()
