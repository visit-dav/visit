# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  activesource.py
#
#  Tests:      mesh      - 2D, curvilinear, single domain
#              plots     - FilledBoundary
#              databases - PDB
#
#  Purpose:    This test case tests that the viewer uses the right active
#              source for the active window.
#
#  Bugs:       '4527
#
#  Programmer: Brad Whitlock
#  Date:       Thu Mar 18 13:45:29 PST 2004
#
#  Modifications:
#
# ----------------------------------------------------------------------------

import string

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

# Create a plot of dbA00.pdb.
OpenDatabase("../data/dbA00.pdb")
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("activesource00")

# Create another window and make sure that it has dbA00.pdb as its
# active source.
CloneWindow()
SetActiveWindow(2)
DeleteAllPlots()
TestWindowInformation("activesource01")

# Open dbB00.pdb and make sure that it becomes the active source.
OpenDatabase("../data/dbB00.pdb")
TestWindowInformation("activesource02")

# Create a plot from it.
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("activesource03")
SetTimeSliderState(TimeSliderGetNStates() / 2)
Test("activesource04")
TestWindowInformation("activesource05")

# Create a 3rd window by cloning window 2. We're making sure that the 
# time slider got copied to the new window and that it has the right
# time state.
CloneWindow()
SetActiveWindow(3)
DrawPlots()
Test("activesource06")
TestWindowInformation("activesource07")

# Switch back to window 1 and make sure that the window information has
# dbA00.pdb as the active source.
SetActiveWindow(1)
TestWindowInformation("activesource08")

Exit()
