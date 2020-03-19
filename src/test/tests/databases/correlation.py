# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  correlation.py 
#
#  Tests:      Tests database correlations and automatic correlation.
#
#  Programmer: Brad Whitlock
#  Date:       Mon Mar 22 08:28:25 PDT 2004
#
#  Modifications:
#    Brad Whitlock, Fri Apr 1 09:48:23 PDT 2005
#    I renamed some baselines so I could add correlation definition tests.
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

#
# Look at the first few lines of the string representation of the
# WindowInformation to see the list of time sliders, etc.
#
def TestWindowInformation(testname):
    TestText(testname, GetTruncatedWindowInformationString())

def TestLength(testname):
    tsLength = TimeSliderGetNStates()
    testString = "%s has %d states\n" % (GetActiveTimeSlider(), tsLength)
    testString = testString + GetTruncatedWindowInformationString()
    TestText(testname, testString)

def ComputeStates(nStates, stateIncrement):
    lastState = nStates - 1
    states = list(range(0, nStates, stateIncrement))
    if(states[-1] != lastState):
        states = states  + [lastState]
    return states

def GetTestName(sectionIndex, testIndex):
    return "correlation_%d_%02d" % (sectionIndex, testIndex)
    
def TestTimeSlider(sectionIndex, testIndex):
    Test(GetTestName(sectionIndex, testIndex))
    TestLength(GetTestName(sectionIndex, testIndex + 1))
    return testIndex + 2

#
# Tests that the database correlations look a certain way.
#
def TestCorrelation(name, sectionIndex, testIndex):
    names = GetDatabaseCorrelationNames()
    s = ""
    if name in names:
        c = GetDatabaseCorrelation(name)
        s = s + str(c) + "\n"
    TestText(GetTestName(sectionIndex, testIndex), s)
    return testIndex + 1

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

#
# The databases that we'll use for most tests.
#
dbs = (data_path("pdb_test_data/dbA00.pdb"),
       data_path("pdb_test_data/dbB00.pdb"),
       data_path("pdb_test_data/dbC00.pdb"))

#
# Open each database and create a plot.
#
for db in dbs:
    OpenDatabase(db)
    AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()

#
# Correlation Types
#
IndexForIndexCorrelation = 0
StretchedIndexCorrelation = 1
TimeCorrelation = 2
CycleCorrelation = 3
correlationTypes = (IndexForIndexCorrelation, StretchedIndexCorrelation,\
    TimeCorrelation, CycleCorrelation)
correlationNames = ("ABC_Index", "ABC_Stretch", "ABC_Time", "ABC_Cycle")
correlationTitles = ("Padded index correlation", "Stretched correlation",\
    "Time correlation", "Cycle correlation")

#
# Create correlations between the 3 databases
#
sectionIndex = 0
testIndex = 0
for i in range(len(correlationTypes)):
    TestSection(correlationTitles[i])
    CreateDatabaseCorrelation(correlationNames[i], dbs, correlationTypes[i])
    testIndex = TestCorrelation(correlationNames[i], sectionIndex, 0)
    states = ComputeStates(TimeSliderGetNStates(), 5)
    for state in states:
        SetTimeSliderState(state)
        testIndex = TestTimeSlider(sectionIndex, testIndex)
    sectionIndex = sectionIndex + 1

#
# Make sure that a new window has the same plots and correlations.
#
TestSection("Creating new window")
sectionIndex = 4
testIndex = 0
alteredCorrelation = correlationNames[0]
SetActiveTimeSlider(alteredCorrelation)
SetTimeSliderState(15)
testIndex = TestTimeSlider(sectionIndex, testIndex)
CloneWindow()
SetActiveWindow(2)
InvertBackgroundColor()
DrawPlots()
testIndex = TestTimeSlider(sectionIndex, testIndex)
SetActiveWindow(1)

#
# Now that multiple windows use the same correlation, alter the correlation
# so it only uses 2 databases and make sure that both windows change when
# we change the time slider.
#
TestSection("Altering correlation")
sectionIndex = 5
testIndex = 0
testIndex = TestCorrelation(alteredCorrelation, sectionIndex, testIndex)
AlterDatabaseCorrelation(alteredCorrelation, (dbs[0], dbs[1]), StretchedIndexCorrelation)
testIndex = TestCorrelation(alteredCorrelation, sectionIndex, testIndex)
testIndex = TestTimeSlider(sectionIndex, testIndex)
SetActiveWindow(2)
testIndex = TestTimeSlider(sectionIndex, testIndex)
SetActiveWindow(1)
SetTimeSliderState(19)
testIndex = TestTimeSlider(sectionIndex, testIndex)
SetActiveWindow(2)
SetTimeSliderState(19)
testIndex = TestTimeSlider(sectionIndex, testIndex)
SetActiveWindow(1)

#
# Test deleting correlations
#
TestSection("Deleting correlations")
sectionIndex = 6
testIndex = 0
for n in correlationNames:
    DeleteDatabaseCorrelation(n)
TestLengthAndCorrelationList(GetTestName(sectionIndex, testIndex))

#
# Test automatically correlating by setting the correlation options such
# that we automatically create a StretchedIndex correlation when adding
# plots, etc.
#
TestSection("Automatic correlations")
sectionIndex = 7
SetActiveWindow(1)
DeleteAllPlots()
TestLengthAndCorrelationList(GetTestName(sectionIndex, 0))
testIndex = 1
SetDatabaseCorrelationOptions(StretchedIndexCorrelation, 0)
OpenDatabase(dbs[0])
AddPlot("FilledBoundary", "material(mesh)")
OpenDatabase(dbs[1])
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
# At this point, the viewer should have automatically created a new correlation
# and it should be the active time slider.
testIndex = TestCorrelation(GetActiveTimeSlider(), sectionIndex, testIndex);
testIndex = TestTimeSlider(sectionIndex, testIndex)
SetTimeSliderState(TimeSliderGetNStates() / 2)
testIndex = TestTimeSlider(sectionIndex, testIndex)
SetTimeSliderState(TimeSliderGetNStates() - 1)
testIndex = TestTimeSlider(sectionIndex, testIndex)

# Now that we've verified the correlation, add another database to it by 
# Creating a plot from yet another database.
OpenDatabase(dbs[2])
AddPlot("FilledBoundary", "material(mesh)")
DrawPlots()
testIndex = TestCorrelation(GetActiveTimeSlider(), sectionIndex, testIndex);
SetTimeSliderState(int(TimeSliderGetNStates() * 0.8))
testIndex = TestTimeSlider(sectionIndex, testIndex)


Exit()
