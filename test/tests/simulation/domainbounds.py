# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  domainbounds.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 3D rectilinear mesh
#
#  Programmer: Kathleen Biagas 
#  Date:       June 17, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------

# Create our simulation object.
sim = TestSimulation("domainbounds", "domainbounds.sim2")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("domainbounds00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("domainbounds01", sim.metadata())

    AddPlot("Subset", "Domains")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (0.672727, 0.569817, 0.471961)
    v.viewUp = (-0.252634, 0.776445, -0.57733)
    SetView3D(v)
    Test("domainbounds02")

    DeleteAllPlots()
    AddPlot("Pseudocolor", "zonal")
    DrawPlots()
    Test("domainbounds03")

    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
