# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  clonefirstref.py
#
#  Tests:      mesh      - 2D, curvilinear, single domain
#              plots     - FilledBoundary
#              databases - PDB
#
#  Purpose:    This test case tests the viewer's ability to consider a window
#              that had been referenced as unreferenced if its plot list has
#              no active source.
#
#  Programmer: Brad Whitlock
#  Date:       Fri Feb 18 14:01:48 PST 2005
#
#  Modifications:
#
# ----------------------------------------------------------------------------

#
# Look at the first few lines of the string representation of the
# WindowInformation to see the list of time sliders, etc.
#
def TestWindowInformation(testname):
    # Get the window information and convert it to a string.
    s = str(GetWindowInformation())
    # Only use the first 5 or so lines from the string.
    lines = string.split(s, "\n")
    s = ""
    for i in range(5):
        if(i < len(lines)):
            s = s + lines[i]
            s = s + "\n"
    TestText(testname, s)

TurnOnAllAnnotations()
SetCloneWindowOnFirstRef(1)
SetWindowLayout(2)
SetActiveWindow(2)
OpenDatabase("../data/dbB00.pdb")
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("clonefirstref_00")
TestWindowInformation("clonefirstref_01")

# Make sure that the database and time sliders, etc got copied to window 1,
# even though it had been "referenced" already. Without the fix, VisIt used
# to complain about no database being open.
SetActiveWindow(1)
Test("clonefirstref_02")
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("clonefirstref_03")
TestWindowInformation("clonefirstref_04")

# Make sure that we can move through time.
SetTimeSliderState(5)
Test("clonefirstref_05")
TestWindowInformation("clonefirstref_06")
Exit()

