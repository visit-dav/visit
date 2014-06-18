# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  updateplots.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#
#  Programmer: Kathleen Biagas 
#  Date:       June 18, 2014 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("updateplots", "updateplots.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("updateplots00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("updateplots01", sim.metadata())

    # 2d mesh and updateplotss
    #AddPlot("Mesh", "mesh2d")
    AddPlot("Pseudocolor", "zonal")
    AddPlot("Vector", "zvec")
    VectorAtts = VectorAttributes()
    VectorAtts.scale = 0.5
    VectorAtts.colorByMag = 0
    VectorAtts.vectorColor = (255, 255, 255, 255)
    SetPlotOptions(VectorAtts)
    DrawPlots()
    Test("updateplots02")

    i = 3
    times = "Times:\n"
    Query("Time")
    times = times + str(GetQueryOutputValue()) + "\n"
    for outer in xrange(5):
        for inner in xrange(5):
            sim.consolecommand("step")
        Query("Time")
        times = times + str(GetQueryOutputValue()) + "\n"
        Test("updateplots%02d"%i)
        i = i+1

    TestText("updateplots%02d"%i, times)
             
# Close down the simulation.
if started:        
    sim.endsim()

Exit()
