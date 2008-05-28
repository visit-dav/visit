# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  timelock.py
#
#  Tests:      mesh      - 2D, 3D, curvilinear, single domain
#              plots     - FilledBoundary, Pseudocolor
#              databases - PDB, Silo
#
#  Purpose:    This test case tests out locking windows in time with multiple
#              databases in multiple windows. It makes sure that we get the
#              right database correlations and the right time sliders.
#
#  Programmer: Brad Whitlock
#  Date:       Thu Mar 17 09:58:35 PDT 2005
#
#  Modifications:
#    Brad Whitlock, Wed Mar 23 09:23:53 PDT 2005
#    I made it truncate the window information so the scalable rendering flag
#    is not included. This way, I don't need mode-specific baselines. I also
#    added 2 new test cases to check the window information because one of
#    the baseline images in test case 2 was incorrect. Finally, I added code
#    in the CleanSlate function to make sure that time locking is off. That
#    was what caused the incorrect test case image.
#
# ----------------------------------------------------------------------------

import os

#
# Look at the first few lines of the string representation of the
# WindowInformation to see the list of time sliders, etc.
#
def TestWindowInformation(testname):
    # Get the window information and convert it to a string.
    s = str(GetWindowInformation())
    # Only use the first 17 or so lines from the string.
    lines = string.split(s, "\n")
    s = ""
    for i in range(17):
        if(i < len(lines)):
            s = s + lines[i]
            s = s + "\n"
    # Get the window information and convert it to a string.
    TestText(testname, s)


#
# Tests that the database correlations look a certain way.
#
def TestCorrelations(testname):
    names = GetDatabaseCorrelationNames()
    s = ""
    for name in names:
        c = GetDatabaseCorrelation(name)
        s = s + str(c) + "\n"
    TestText(testname, s)

def CleanSlate():
    # Delete all but the first window.
    windows = list(GetGlobalAttributes().windows)
    windows.sort()
    for win in windows[1:]:
        SetActiveWindow(win)
        DeleteWindow()

    # Delete all of the plots.
    DeleteAllPlots()

    # Delete all of the database correlations:
    sources = GetGlobalAttributes().sources
    cL = GetDatabaseCorrelationNames()
    for name in cL:
        if name not in sources:
            DeleteDatabaseCorrelation(name)
 
    # Close all of the sources.
    for src in sources:
        CloseDatabase(src)

    # Make sure clone window on first reference is off.
    SetCloneWindowOnFirstRef(0)

    # Make sure that window 1 is not locked in time!
    if GetWindowInformation().lockTime == 1:
        ToggleLockTime()

#
# Returns whether all files in the list are in the current directory.
#
def FilesPresent(files):
    currentFileList = os.listdir(".")
    count = 0
    retval = 0
    if type(files) == type(()) or type(files) == type([]):
        for file in files:
            if file in currentFileList:
                count = count + 1
        retval = count == len(files)
    else:
        # We got here because the files argument was 
        # a single value instead of a tuple or list.
        if files in currentFileList:
            retval = 1

    return retval

#
# Waits for all files in the list to be present in the current directory.
#
def WaitForFilesToBePresent(files):
    while(FilesPresent(files) == 0): sleep(1)

#
# Remove all .visit files from the current directory.
#
def RemoveAllVisItFiles():
    currentFileList = os.listdir(".")
    for file in currentFileList:
        if file[-5:] == ".silo" or file[-6:] == ".visit":
            try:
                os.unlink(file)
            except:
                # Ignore any exceptions
                pass

#
# Set a better view for wave.
#
def SetWaveDatabaseView():
    v0 = View3DAttributes()
    v0.viewNormal = (-0.735926, 0.562657, 0.376604)
    v0.focus = (5, 0.753448, 2.5)
    v0.viewUp = (0.454745, 0.822858, -0.340752)
    v0.viewAngle = 30
    v0.parallelScale = 5.6398
    v0.nearPlane = -11.2796
    v0.farPlane = 11.2796
    v0.imagePan = (0.0589778, 0.0898255)
    v0.imageZoom = 1.32552
    v0.perspective = 1
    v0.eyeAngle = 2
    SetView3D(v0)

#
# Set the active window and also set the window's background color so it's
# easy to tell which window we're looking at.
#
def GotoWindow(win):
    SetActiveWindow(win)

    a = GetAnnotationAttributes()
    if win == 1:
        a.backgroundColor = (255,200,200,255)
    elif win == 2:
        a.backgroundColor = (200,255,200,255)
    else:
        a.backgroundColor = (200,200,255,255)
    SetAnnotationAttributes(a)

#
# Test that we get an active time slider when a correlation is modified
# as a result of locking the window in time.
#
def test1(testindex):
    TestSection("Make sure we get a time slider when locking a window "
                "causes the most suitable correlation to be altered.")
    SetWindowLayout(4)

    # Turn on "CloneWindowOnFirstRef"
    SetCloneWindowOnFirstRef(1)

    # Copy wave.visit to this directory a few times.
    f = open("../data/wave.visit", "rt")
    lines = f.readlines()
    f.close()
    f0 = open("wave.visit","wt")
    f1 = open("wave1.visit","wt")
    f2 = open("wave2.visit","wt")
    for line in lines:
        f0.write("../data/" + line)
        f1.write("../data/" + line)
        f2.write("../data/" + line)
    f0.close()
    f1.close()
    f2.close()

    GotoWindow(1)
    OpenDatabase("wave.visit")
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    SetWaveDatabaseView()
    Test("timelock_%02d" % testindex)
    TestWindowInformation("timelock_%02d" % (testindex+1))

    # Go to the next window. The plot should be copied. Replace the database
    # before we draw the plots so we'll be using a different database.
    GotoWindow(2)
    ReplaceDatabase("wave1.visit")
    DrawPlots()
    Test("timelock_%02d" % (testindex+2))
    TestWindowInformation("timelock_%02d" % (testindex+3))

    # Go to the next window. The plot should be copied. Replace the database
    # before we draw the plots so we'll be using a different database.
    GotoWindow(3)
    ReplaceDatabase("wave2.visit")
    DrawPlots()
    Test("timelock_%02d" % (testindex+4))
    TestWindowInformation("timelock_%02d" % (testindex+5))

    # Lock window 1 and 2. This should result in a database correlation.
    GotoWindow(1)
    ToggleLockTime()
    GotoWindow(2)
    ToggleLockTime()
    TestCorrelations("timelock_%02d" % (testindex+6))

    # Lock window 3 in time now also. This should result in the new database
    # correlation being modified to accomodate window 3's database.
    GotoWindow(3)
    ToggleLockTime()
    TestCorrelations("timelock_%02d" % (testindex+7))

    # Change time states and make sure all windows look the same.
    SetTimeSliderState(36)
    Test("timelock_%02d" % (testindex+8))
    TestWindowInformation("timelock_%02d" % (testindex+9))
    GotoWindow(2)
    Test("timelock_%02d" % (testindex+10))
    TestWindowInformation("timelock_%02d" % (testindex+11))
    GotoWindow(1)
    Test("timelock_%02d" % (testindex+12))
    TestWindowInformation("timelock_%02d" % (testindex+13))

    # Get ready for the next test.
    CleanSlate()
    RemoveAllVisItFiles()

    return testindex + 14

#
# Test that time locking works for multiple windows. What we're really
# testing is that the database is copied to the newly referenced window
# even though we have "CloneWindowOnFirstRef" set to off. Defect '6053.
#
def test2(testindex):
    TestSection("Test that the time slider works when time locking multiple windows")

    a = GetAnnotationAttributes()
    b = GetAnnotationAttributes()
    b.axesFlag2D = 0
    b.xAxisLabels2D = 0
    b.yAxisLabels2D = 0
    b.xAxisTitle2D = 0
    b.yAxisTitle2D = 0
    b.xLabelScaling2D = 0
    b.yLabelScaling2D = 0
    b.databaseInfoFlag = 0
    b.legendInfoFlag = 0
    SetAnnotationAttributes(b)

    SetCloneWindowOnFirstRef(0)
    OpenDatabase("../data/dbA00.pdb")
    AddPlot("FilledBoundary", "material(mesh)")
    DrawPlots()
    Test("timelock_%02d" % testindex)

    SetWindowLayout(2)
    GotoWindow(2)
    SetAnnotationAttributes(b)
    TestWindowInformation("timelock_%02d" % (testindex+1))
    AddPlot("Pseudocolor", "mesh/nummm")
    DrawPlots()
    Test("timelock_%02d" % (testindex+2))

    # Turn on time locking in both windows.
    GotoWindow(1)
    ToggleLockTime()
    GotoWindow(2)
    ToggleLockTime()

    # See if both windows updated when we changed the time in window 2.
    SetTimeSliderState(5)
    Test("timelock_%02d" % (testindex+3))
    TestWindowInformation("timelock_%02d" % (testindex+4))
    GotoWindow(1)
    ResetView()
    Test("timelock_%02d" % (testindex+5))
    TestWindowInformation("timelock_%02d" % (testindex+6))

    # Get ready for the next test.
    CleanSlate()
    SetAnnotationAttributes(a)

    return testindex + 7

#
# Make sure that replacing into a time-locked window updates the database
# correlation.
#
def test3(testindex):
    TestSection("Make sure replacing into a time-locked window updates "
                "the database correlation.")
    SetWindowLayout(4)

    # Turn on "CloneWindowOnFirstRef"
    SetCloneWindowOnFirstRef(1)

    dbs = ("../data/wave.visit", "../data/wave_tv.visit")
    OpenDatabase(dbs[0])
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    SetWaveDatabaseView()
    Test("timelock_%02d" % testindex)

    GotoWindow(2)
    DeleteAllPlots()
    AddPlot("FilledBoundary", "Material")
    DrawPlots()
    Test("timelock_%02d" % (testindex+1))
    ToggleLockTime()
    TestWindowInformation("timelock_%02d" % (testindex+2))
    GotoWindow(1)
    ToggleLockTime()
    TestWindowInformation("timelock_%02d" % (testindex+3))

    # Go to window 3 and make sure that certain fields were copied.
    # Window 3 should already be locked in time and it should contain
    # a Pseudocolor plot.
    GotoWindow(3)
    TestWindowInformation("timelock_%02d" % (testindex+4))

    # Replace the database with wave_tv so we can make sure that VisIt can
    # create correlations when it needs to during a replace. Note that we
    # also replace with a later time state. This should cause the time states
    # for the other windows to be updated.
    ReplaceDatabase(dbs[1], 36)
    TestCorrelations("timelock_%02d" % (testindex+5))
    DrawPlots()
    Test("timelock_%02d" % (testindex+6))

    # Test that we change change to the transient variable.
    ChangeActivePlotsVar("transient")
    Test("timelock_%02d" % (testindex+7))

    # Make sure that the time state changed in windows 1,2.
    GotoWindow(2)
    Test("timelock_%02d" % (testindex+8))
    TestWindowInformation("timelock_%02d" % (testindex+9))
    GotoWindow(1)
    Test("timelock_%02d" % (testindex+10))
    TestWindowInformation("timelock_%02d" % (testindex+11))

    # Get ready for the next test.
    CleanSlate()

    return testindex + 12

#
# Run the tests
#
try:
    TurnOnAllAnnotations()
    testindex = 0
    testindex = test1(testindex)
    testindex = test2(testindex)
    testindex = test3(testindex)
except:
    RemoveAllVisItFiles()
    raise

Exit()
