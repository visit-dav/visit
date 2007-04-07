# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  correlationsession.py
#
#  Tests:      mesh      - 2D curvilinear, single domain
#              plots     - Boundary, FilledBoundary
#
#  Notes:      This test case makes sure that VisIt sessions that made heavy
#              use of database correlations can have that information
#              correctly captured in a session file.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Mar 25 11:32:09 PDT 2004
#
#  Modifications:
#
# ----------------------------------------------------------------------------

import string

def GetTruncatedWindowInformationString():
    # Get the window information and convert it to a string.
    s = str(GetWindowInformation())
    # Only use the first 5 or so lines from the string.
    lines = string.split(s, "\n")
    s = ""
    for i in range(5):
        if(i < len(lines)):
            s = s + lines[i]
            s = s + "\n"
    return s

def TestLength(testname):
    tsLength = TimeSliderGetNStates()
    testString = "%s has %d states\n" % (GetActiveTimeSlider(), tsLength)
    testString = testString + GetTruncatedWindowInformationString()
    TestText(testname, testString)

#
# Restore the session file and make sure that it at the right time step
# and has the right time slider.
#
RestoreSession("tests/session/correlationsession.session", 0)
Test("correlationsession00")
TestLength("correlationsession01")

#
# Make sure we have the right active source by making a new plot. The active
# source should be dbA00.pdb
#
AddPlot("Boundary", "material(mesh)")
b = BoundaryAttributes()
b.colorType = b.ColorBySingleColor
b.singleColor = (255,255,0,255)
b.lineWidth = 2
SetPlotOptions(b)
DrawPlots()
Test("correlationsession02")

#
# Make sure that we can advance to the end of the time slider. Save some
# test images along the way though.
#
timeSliders = GetTimeSliders()
currentState = timeSliders[GetActiveTimeSlider()]
endState = TimeSliderGetNStates() - 1
nSteps = 4
start = currentState + int(float(endState - currentState) / float(nSteps))
testIndex = 3
for i in range(nSteps):
    t = float(i) / float(nSteps - 1)
    omt = 1. - t
    state = int(omt * float(start) + t * float(endState))
    SetTimeSliderState(state)
    Test("correlationsession%02d" % testIndex)
    TestLength("correlationsession%02d" % (testIndex + 1))
    testIndex = testIndex + 2

Exit()
