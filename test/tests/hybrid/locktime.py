# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  multiwindowcorrelation.py 
#
#  Tests:      Tests database correlations when there are multiple windows.
#
#  Programmer: Brad Whitlock
#  Date:       Mon Mar 22 15:59:24 PST 2004
#
#  Modifications:
#    Brad Whitlock, Fri Apr 1 15:01:13 PST 2005
#    Added a function to check the database correlation list.
#
# ----------------------------------------------------------------------------

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
# Tests the time slider length and the correlation list.
#
def TestLengthAndCorrelationList(testname):
    tsLength = TimeSliderGetNStates()
    s = "%s has %d states\n" % (GetActiveTimeSlider(), tsLength)
    s = s + GetTruncatedWindowInformationString() + "\n\n"

    names = GetDatabaseCorrelationNames()
    for name in names:
        c = GetDatabaseCorrelation(name)
        s = s + str(c) + "\n"
    TestText(testname, s)

TurnOnAllAnnotations()

# Create a plot of DB A.
OpenDatabase("../data/dbA00.pdb")
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
Test("locktime_00")
TestLength("locktime_01")

# Create a second window and create plot of DB B.
CloneWindow()
DeleteAllPlots()
OpenDatabase("../data/dbB00.pdb")
AddPlot("FilledBoundary", "material(mesh)")
InvertBackgroundColor()
DrawPlots()
Test("locktime_02")
TestLength("locktime_03")

#
# Set the database correlation options to control how we create automatic
# correlations. When we lock the windows together, VisIt will create a
# correlation for the databases in both windows because the windows do not
# have a common database. I'm setting the correlation options here because 
# I want a StretchedIndexCorrelation correlation to be created.
#
StretchedIndexCorrelation = 1
SetDatabaseCorrelationOptions(StretchedIndexCorrelation, 0)

#
# Lock time for window 2 and window 1. This will create a correlation between
# the databases in both windows.
#
ToggleLockTime()
SetActiveWindow(1)
ToggleLockTime()
TestLengthAndCorrelationList("locktime_04")

SetTimeSliderState(17)
Test("locktime_05")
TestLength("locktime_06")

SetActiveWindow(2)
Test("locktime_07")
TestLength("locktime_08")

SetActiveWindow(1)
SetTimeSliderState(7)
Test("locktime_09")
TestLength("locktime_10")

SetActiveWindow(2)
Test("locktime_11")
TestLength("locktime_12")

#
# Now let's delete the correlation that was created. It will be have the same
# name as the active time slider.
#
DeleteDatabaseCorrelation(GetActiveTimeSlider())
TestLengthAndCorrelationList("locktime_13")

# Now try moving the time slider state for DB B, since that's the active time
# slider for window 2, which is the active window right now.
SetTimeSliderState(15)
Test("locktime_14")
TestLength("locktime_15")

# Make sure that window 1 did not change.
SetActiveWindow(1)
Test("locktime_16")
TestLength("locktime_17")


Exit()
