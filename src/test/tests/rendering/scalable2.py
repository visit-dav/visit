# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scalable2.py
#
#  Tests:      mesh      - 2D curvilinear
#              plots     - Pseudocolor, Contour
#
#  Programmer: Brad Whitlock
#  Date:       Fri May 12 15:55:22 PST 2006
#
#  Modifications:
#    Mark C. Miller, Wed Jan 21 10:00:10 PST 2009
#    Removed silly comment regarding global annotation object 'a'
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------
import os, sys

#
# Test that single-timestep plots have their networks reset in the
# viewer when we lose the compute engine.
#
def TestNetworkReset():
    TestSection("Testing that plots of single-timestep are "\
                "requested in SR after engine loss")
    OpenDatabase(silo_data_path("curv2d.silo"))

    AddPlot("Mesh", "curvmesh2d")

    OpenDatabase(data_path("pdb_test_data/dbA00.pdb"))

    AddPlot("Contour", "mesh/nummm")
    DrawPlots()

    # Use scalable rendering always
    r = GetRenderingAttributes()
    r.scalableActivationMode = r.Always
    SetRenderingAttributes(r)

    Test("scalable2_0_00")

    # Turn off screen capture
    base = "TestNetworkReset"
    s = GetSaveWindowAttributes()
    s.screenCapture = 0
    s.width = 400
    s.height = 400
    s.format = s.PNG
    s.fileName = base
    s.family = 1
    SetSaveWindowAttributes(s)

    # Save out images, killing the compute engine halfway through
    usingSR = ""
    for i in range(TimeSliderGetNStates()):
        SetTimeSliderState(i)
        # TODO_WINDOWS THIS WONT WORK ON WINDOWS
        # Kill the compute engine partway through to simulate it getting
        # booted out of the batch queue.
        if(i == 5):
            pinfo = GetProcessAttributes("engine")
            for pid in pinfo.pids:
                os.system("kill -9 %d" % pid)

        DrawPlots()
        SaveWindow()
        if(GetWindowInformation().usingScalableRendering == 1):
            usingSR = usingSR + "Using scalable rendering on frame %d: true\n" % i
        else:
            usingSR = usingSR + "Using scalable rendering on frame %d: false\n" % i

    # Look at the file sizes to make sure that all are non-zero
    files = os.listdir(".")
    pngs = []
    for f in files:
        if(len(f) > len(base) and f[:len(base)] == base):
            pngs = pngs + [f]
    pngs.sort()

    # Iterate through the png files and look at their sizes. Remove them too.
    pngSize = ""
    for png in pngs:
        filesize = os.stat(png)[6]
        if filesize > 0:
            pngSize = pngSize + "File size for %s greater than zero.\n" % png
        else:
            pngSize = pngSize + "File size for %s is zero.\n" % png
        # remove the png file.
        os.unlink(png)

    TestText("scalable2_0_01", usingSR)
    TestText("scalable2_0_02", pngSize)

    DeleteAllPlots()

#
# Call all of the subtests.
#
def main():
    # Turn off all annotation
    TurnOffAllAnnotations()

    TestNetworkReset()

#
# Call main
#
main()
Exit()
