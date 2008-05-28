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
#
# ----------------------------------------------------------------------------
import os, sys

#
# Test that single-timestep plots have their networks reset in the
# viewer when we lose the compute engine.
#
def TestNetworkReset():
    TestSection("Testing that plots of single-timestep are "\
                "requested in SR after engine loss")
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Mesh", "curvmesh2d")

    OpenDatabase("../data/dbA00.pdb")
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
    s.format = s.TIFF
    s.fileName = base
    s.family = 1
    SetSaveWindowAttributes(s)

    # Save out images, killing the compute engine halfway through
    usingSR = ""
    for i in range(TimeSliderGetNStates()):
        SetTimeSliderState(i)

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
    tiffs = []
    for f in files:
        if(len(f) > len(base) and f[:len(base)] == base):
            tiffs = tiffs + [f]
    tiffs.sort()

    # Iterate through the tiff files and look at their sizes. Remove them too.
    tiffSize = ""
    for tiff in tiffs:
        filesize = os.stat(tiff)[6]
        if filesize > 0:
            tiffSize = tiffSize + "File size for %s greater than zero.\n" % tiff
        else:
            tiffSize = tiffSize + "File size for %s is zero.\n" % tiff
        # remove the tiff file.
        os.unlink(tiff)

    TestText("scalable2_0_01", usingSR)
    TestText("scalable2_0_02", tiffSize)

    DeleteAllPlots()

#
# Call all of the subtests.
#
def main():
    # Turn off all annotation
    TurnOffAllAnnotations() # defines global object 'a'

    TestNetworkReset()

#
# Call main
#
main()
Exit()
