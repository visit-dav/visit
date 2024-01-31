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
#   Kathleen Biagas, Fri Sep 10 09:37:11 PDT 2021
#   Added test for exporting vtk.
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("updateplots", "updateplots.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("updateplots00", sim)

def step(sim):
    sim.consolecommand("step")
    # Read from stderr to look for the echoed command. Sync.
    keepGoing = True
    while keepGoing:
        buf = sim.p.stderr.readline()
        print(buf)
        if "Command 'step'" in buf:
            keepGoing = False

def testExportVTK(sim):
    # default export FileFormat for VTK is Legacy ascii (.vtk extension),
    # Test an export that sets the FileFormat to XML Binary (.vtr extension)
    sim.consolecommand("exportVTK")
    # Read from stderr to look for the echoed command. Sync.
    keepGoing = True
    while keepGoing:
        buf = sim.p.stderr.readline()
        print(buf)
        if "Command 'exportVTK'" in buf:
            keepGoing = False
    TestValueEQ("updateplots_export0000.vtr exists", 
         os.path.isfile(os.path.join(TestEnv.params["run_dir"], "updateplots_export0000.vtr")),
         True)


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
    VectorAtts.colorByMagnitude = 0
    VectorAtts.vectorColor = (255, 255, 255, 255)
    SetPlotOptions(VectorAtts)
    DrawPlots()
    Test("updateplots02")

    i = 3
    times = "Times:\n"
    Query("Time")
    times = times + str(GetQueryOutputValue()) + "\n"
    for outer in range(6):
        for inner in range(3):
            step(sim)
        Query("Time")
        times = times + str(GetQueryOutputValue()) + "\n"
        Test("updateplots%02d"%i)
        i = i+1

    TestText("updateplots%02d"%i, times)

    # Uncomment this when #17008 is fixed (crash when Logging ExportDBRPC)
    #testExportVTK(sim)

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
