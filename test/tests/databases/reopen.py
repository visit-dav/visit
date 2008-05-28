# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  reopen.py 
#
#  Tests:      Reopening a database where the SIL is invariant.
#
#  Programmer: Hank Childs
#  Date:       March 1, 2004
#
#  Modifications:
#    Brad Whitlock, Fri Mar 19 15:09:09 PST 2004
#    I changed one of the OpenDatabase calls to ReOpenDatabase. I also
#    added more extensive reopen tests.
#
#    Brad Whitlock, Fri Apr 9 16:28:01 PST 2004
#    I added a test that really makes sure the engine reexecuted during
#    a reopen.
#
#    Jeremy Meredith, Wed Apr 28 11:42:31 PDT 2004
#    I made the "Don't need to copy" message go to stderr.
#
#    Brad Whitlock, Wed Feb 2 16:27:54 PST 2005
#    Added a test for making sure that expressions are right after reopening
#    a database when there are multiple windows. I also added a test to 
#    make sure that time sliders are shortened when we reopen a database
#    that has had some of its time states removed.
#
#    Hank Childs, Wed Feb 16 07:34:07 PST 2005
#    Rename variables that have unsupported characters.
#
#    Brad Whitlock, Tue Feb 22 13:56:56 PST 2005
#    Added a test case for reopening a file that has been deleted.
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
# ----------------------------------------------------------------------------

import os, string, sys

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

#
# Test the expression list.
#
def TestExpressions(name):
    expr = Expressions()
    testString = ""
    for e in expr:
        s = e[0]
        while len(s) < 30:
            s = s + " "
        testString = testString + s + e[1] + "\n"
    TestText(name, testString)

#
# Create test cases for the time varying metadata files.
#
def TestTimeVarying(title, testIndex):
    # Set the title with the time state.
    timeSliders = GetTimeSliders()
    tsState = timeSliders[GetActiveTimeSlider()]
    title.text = "TimeVarying state=%d" % tsState
    # Test what the first time step looks like.
    Test("reopen_%02d" % testIndex)
    TestLength("reopen_%02d" % (testIndex + 1))
    return testIndex + 2

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
# Removes all files ending in .silo or .visit from the current directory
# to ensure that there are no such files left over from a failed test.
#
def RemoveAllSiloAndVisItFiles():
    currentFileList = os.listdir(".")
    for file in currentFileList:
        if file[-5:] == ".silo" or file[-6:] == ".visit":
            try:
                os.unlink(file)
            except:
                # Ignore any exceptions
                pass

#
# Function to create a .visit file or virtual database using time states
# from wave.
#
def CreateMTFile(prefix, makeVisItFile, percent):
    # Create a list of files that will be used to create the database
    # for the test.
    nStates = 71
    t = float(percent) / 100.
    maxState = int(float(nStates) * t)
    files = []
    prefixfiles = []
    for i in range(maxState):
        w = "wave%04d.silo" % (i * 10)
        files = files + [w]
        prefixfiles = prefixfiles + [prefix+w]

    if makeVisItFile == 0:
        # Virtual database
        for file in files:
            try:
                # Copy a file from the data directory to the current directory.
                os.link("../data/%s" % file, "%s%s" % (prefix, file))
            except OSError:
                print >>sys.stderr, "Don't need to copy %s" % file

        # Wait for all files in the list to be present so there is no
        # possibility of continuing until all files are ready.
        WaitForFilesToBePresent(prefixfiles)

        db = prefix + "wave*.silo database"
    else:
        # .visit file.
        db = "reopen_wave.visit"
        f = open(db, "wt")
        for file in files:
            f.write("../data/%s\n" % file)
        f.close()
    return (db, prefixfiles)

# Function to remove the .visit file or the virtual database.
def DestroyMTFile(makeVisItFile, db):
    RemoveAllSiloAndVisItFiles()

# Function to create a new SAMRAI .visit file.
def CreateTimeVaryingMTFile(percent):
    # Read in the entire dumps.visit file for the SAMRAI data so that
    # we can get the list of files.
    f = open("../data/samrai_test_data/sil_changes/dumps.visit", "rt")
    lines = f.readlines()
    f.close()

    # Create a new .visit file
    nStates = len(lines)
    t = float(percent) / 100.
    maxState = int(float(nStates) * t)
    db = "reopen_samrai.visit"
    f = open(db, "wt")
    for i in range(maxState):
        f.write("../data/samrai_test_data/sil_changes/%s" % lines[i])
    f.close()
    return db

# Function to remove the SAMRAI .visit file.
def DestroyTimeVaryingMTFile(db):
    RemoveAllSiloAndVisItFiles()

def SetTheView():
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

###############################################################################

#
# Make sure that Reopen does not reset the time slider state and also make
# sure that we are getting the metadata for a late time state.
#
def test1(testIndex):
    #
    # This will open at timestep 0.  The variable "transient" will not be
    # available then.
    #
    db = "../data/wave.visit"
    OpenDatabase(db)

    #
    # Now set the time to a timestep when "transient" is available.
    #
    SetTimeSliderState(20)
    TestWindowInformation("reopen_00")

    #
    # If we were to try and make a PC plot of transient right now, it wouldn't
    # work.  We need to do a re-open first.
    #
    ReOpenDatabase(db)

    #
    # Make a plot of transient. This had better not reset the time state to 0.
    #
    AddPlot("Pseudocolor","transient")
    DrawPlots()
    SetTheView()
    Test("reopen_01")

    #
    # Delete the plots and close the database
    #
    DeleteAllPlots()
    CloseDatabase(db)
    TestWindowInformation("reopen_02")

    return testIndex + 3

#
# Now test that reopening a file actually makes the time slider longer. First
# create a new .visit file that we can add onto later.
#
def test2(testIndex):
    VirtualDatabase = 0
    VisItFile = 1

    for method in (VirtualDatabase, VisItFile):
        # Add a section title.
        if(method == VirtualDatabase):
            TestSection("Reopening virtual database")
        else:
            TestSection("Reopening .visit file")

        db, files = CreateMTFile("", method, 30)
        OpenDatabase(db)
        AddPlot("Pseudocolor", "pressure")
        DrawPlots()

        # Set up the view.
        SetTheView()

        # Go to the last time state.
        SetTimeSliderState(TimeSliderGetNStates() - 1)
        Test("reopen_%02d" % testIndex)
        TestLength("reopen_%02d" % (testIndex + 1))

        # Create more time states in the file.
        db, files = CreateMTFile("", method, 60)
        ReOpenDatabase(db)
        # Go to the last time state.
        SetTimeSliderState(TimeSliderGetNStates() - 1)
        Test("reopen_%02d" % (testIndex + 2))
        TestLength("reopen_%02d" % (testIndex + 3))

        # Create more time states in the file.
        db, files = CreateMTFile("", method, 100)
        ReOpenDatabase(db)
        # Go to the last time state.
        SetTimeSliderState(TimeSliderGetNStates() - 1)
        Test("reopen_%02d" % (testIndex + 4))
        TestLength("reopen_%02d" % (testIndex + 5))

        # Get rid of the .visit file that we created.
        DestroyMTFile(method, db)
        DeleteAllPlots()
        CloseDatabase(db)

        # Get to the next testIndex
        testIndex = testIndex + 6

    return testIndex

#
# Now that we've tested time-invariant databases, try testing reopen with
# a time-varying database to see if we get the right plots.
#
def test3(testIndex):
    TestSection("Reopening .visit file of time-varying data")

    # Create a title text object
    title = CreateAnnotationObject("Text2D")
    title.position = (0.15, 0.93)
    title.width = 0.7

    for percent in (30,60,100):
        db = CreateTimeVaryingMTFile(percent)
        if(percent == 30):
            OpenDatabase(db)
            AddPlot("Pseudocolor", "Primitive Var _number_0")
            DrawPlots()

            # Set the view
            v0 = View3DAttributes()
            v0.viewNormal = (-0.598154, 0.519575, -0.610127)
            v0.focus = (15, 10, 10)
            v0.viewUp = (0.418052, 0.851849, 0.315574)
            v0.viewAngle = 30
            v0.parallelScale = 20.6155
            v0.nearPlane = -41.2311
            v0.farPlane = 41.2311
            v0.imagePan = (0.0200698, 0.0374771)
            v0.imageZoom = 1
            v0.perspective = 1
            v0.eyeAngle = 2
            SetView3D(v0)

            # Save a test
            testIndex = TestTimeVarying(title, testIndex)
        else:
            # Reopen the database to add the new time states.
            ReOpenDatabase(db)

        # Go to the last time state.
        SetTimeSliderState(TimeSliderGetNStates() - 1)

        # Save a test
        testIndex = TestTimeVarying(title, testIndex)

    # Clean up the time varying .visit file.
    DeleteAllPlots()
    CloseDatabase(db)
    DestroyTimeVaryingMTFile(db)

    return testIndex

#
# Test that reopening a file that has been overwritten works.
#
def test4(testIndex):
    TestSection("Reopening overwritten file to test engine")

    # Delete all of the annotation objects
    try:
        while(1):
            GetAnnotationObject(0).Delete()
    except VisItException:
        # Done deleting annotation objects.
        pass

    # Copy curv2d to the current directory.
    db = "test4.silo"
    os.link("../data/curv2d.silo", db)

    # Open up the file and create a plot.
    OpenDatabase(db)
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Test("reopen_%02d" % testIndex)

    # Delete the file
    os.unlink(db)
    os.link("../data/rect2d.silo", db)
    ReOpenDatabase(db)
    ResetView()
    Test("reopen_%02d" % (testIndex + 1))

    DeleteAllPlots()
    # Delete the file
    os.unlink(db)

    return testIndex + 2

#
# Test that expressions are not lost after reopening a file when there
# are multiple windows.
#
def test5(testIndex):
    TestSection("Testing reopen/expressions with multiple windows")
    db = "../data/curv3d.silo"
    AddWindow()
    SetActiveWindow(1)
    OpenDatabase(db)
    TestExpressions("reopen_%02d" % testIndex)

    # See if the expressions are right after reopening.
    ReOpenDatabase(db)
    TestExpressions("reopen_%02d" % (testIndex + 1))

    # Delete the window that we added.
    SetActiveWindow(2)
    DeleteWindow()
    CloseDatabase(db)

    return testIndex + 2

#
# Test that time sliders are shortened when we reopen an MT database
# that has had time states removed.
#
def test6(testIndex):
    TestSection("Testing reopen at an invalid time state")
    RemoveAllSiloAndVisItFiles()

    # Make sure that there is no open database.
    TestLength("reopen_%02d" % testIndex)

    # Create a short MT file.
    db, files = CreateMTFile("", 0, (10. / 71.) * 100.)

    # Create a plot and make sure its database has the right number
    # of time states.
    OpenDatabase(db)
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    ResetView()
    Test("reopen_%02d" % (testIndex + 1))
    TestLength("reopen_%02d" % (testIndex + 2))

    # Make a copy of the first window.
    CloneWindow()
    DrawPlots()
    Test("reopen_%02d" % (testIndex + 3))
    TestLength("reopen_%02d" % (testIndex + 4))
    SetActiveWindow(1)

    # Delete the last few time states
    nStates = len(files)
    for f in files[-5:]:
        try:
            os.unlink(f)
        except:
            print >>sys.stderr, "Could not delete %s" % f

    # Change to a time state that we deleted. This should put the plot
    # in the error state and we should get an error message.
    SetTimeSliderState(nStates - 2)
    Test("reopen_%02d" % (testIndex + 5))
    TestText("reopen_%02d" % (testIndex + 6), GetLastError())

    # Do the same thing in window 2 so we can check leter if reopen
    # causes the time slider, etc to be corrected.
    SetActiveWindow(2)
    SetTimeSliderState(nStates - 2)
    SetActiveWindow(1)

    # Reopen the database. This should pick up that the database has
    # fewer time states and should update the time slider so it is
    # in bounds. The compute engine also should not crash.
    ReOpenDatabase(db)
    DrawPlots()
    Test("reopen_%02d" % (testIndex + 7))
    TestLength("reopen_%02d" % (testIndex + 8))

    # See if we're at the right time state in window 2 too.
    SetActiveWindow(2)
    DrawPlots()
    Test("reopen_%02d" % (testIndex + 9))
    TestLength("reopen_%02d" % (testIndex + 10))
    DeleteWindow()

    # Delete all of the plots in window 1 and close the database.
    DeleteAllPlots()
    CloseDatabase(db)

    # Delete the last few files.
    DestroyMTFile(0, db)

    return testIndex + 11

#
# Test reopening a database that has been removed from disk.
#
def test7(testIndex):
    TestSection("Testing reopen on a deleted file")
    # Link a file from the data directory to the current directory.
    db = "reopen_globe.silo"
    os.link("../data/globe.silo", db)
    WaitForFilesToBePresent(db)

    OpenDatabase(db)
    AddPlot("Pseudocolor", "t")
    DrawPlots()
    Test("reopen_%02d" % testIndex)

    # Remove the file and make sure that we can't reopen it.
    RemoveAllSiloAndVisItFiles()
    if ReOpenDatabase(db):
        s = "VisIt was able to reopen " + db
    else:
        s = "VisIt was *NOT* able to reopen " + db + "!"
    TestText("reopen_%02d" % (testIndex+1), s)

    # Do something that will make the plot be regenerated. Here we're changing
    # plot variables to force VisIt to recalculate the plot.
    ChangeActivePlotsVar("u")
    Test("reopen_%02d" % (testIndex+2))
    DeleteAllPlots()
    CloseDatabase(db)

    return testIndex+3

#
# Run the tests
#
try:
    # Remove all .silo and .visit files that could be left over from
    # previous failed runs.
    RemoveAllSiloAndVisItFiles()

    # Turn off some annotations
    a = AnnotationAttributes()
    TurnOffAllAnnotations(a)
    a.databaseInfoFlag = 1
    a.legendInfoFlag = 1
    SetAnnotationAttributes(a)

    # Run the tests
    testIndex = test1(0)
    testIndex = test2(testIndex)
    testIndex = test3(testIndex)
    testIndex = test4(testIndex)
    testIndex = test5(testIndex)
    testIndex = test6(testIndex)
    testIndex = test7(testIndex)
except:
    # If we got any kind of exception, make sure that we get rid of
    # all of the .silo and .visit files that might be left.
    RemoveAllSiloAndVisItFiles()

    # Rethrow the exception so the test won't look like it passed if
    # it really didn't pass
    raise

Exit()
