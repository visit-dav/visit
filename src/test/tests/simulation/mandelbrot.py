# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mandelbrot.py
#
#  Tests:      libsim - connecting to simulation and retrieving data from it.
#              mesh   - 2D structured mesh.
#              scalars
#              AMR
#
#  Programmer: Brad Whitlock
#  Date:       Wed Dec 18 12:04:01 PST 2013
#
#  Modifications:
#
# ----------------------------------------------------------------------------
import time

# Create our simulation object.
sim = TestSimulation("mandelbrot", "mandelbrot.sim2")
sim.addargument("-echo")

# Test that we can start and connect to the simulation.
started, connected = TestSimStartAndConnect("mandelbrot00", sim)

# Perform our tests.
if connected:
    # Make sure the metadata is right.
    TestSimMetaData("mandelbrot01", sim.metadata())

    AddPlot("Pseudocolor", "mandelbrot")
    DrawPlots()
    v = GetView2D()
    v.viewportCoords = (0., 1., 0., 1.)
    SetView2D(v)
    Test("mandelbrot02")

    # Advance some steps and update the plots.

    # Read the "VisIt connected" message.
    buf = sim.p.stderr.readline()
    for i in range(12):
        sim.consolecommand("step")
        # Read from stderr to look for the echoed command.
        buf = sim.p.stderr.readline()
        print(buf)
        time.sleep(1)
    # Update the data for the new time step.
    sim.consolecommand("update")
    Test("mandelbrot03")

    # Make sure the metadata has updated.
    TestSimMetaData("mandelbrot04", sim.metadata())

    # Test transparency to see if nesting is used.
    pc = PseudocolorAttributes(1)
    pc.opacityType = pc.Constant
    pc.opacity = 0.5
    SetPlotOptions(pc)
    Test("mandelbrot05")

    DeleteActivePlots()
    AddPlot("Subset", "levels")
    DrawPlots()
    Test("mandelbrot06")
    
    ChangeActivePlotsVar("patches")
    Test("mandelbrot07")

    DeleteAllPlots()

# Close down the simulation.
if started:        
    sim.endsim()

Exit()
