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
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Jeremy Meredith, Wed Jan 20 11:02:36 EST 2010
#    Fixed some file path issues and checked for "from" file before
#    assuming a symlink return error implied a different error message.
#
#    Kathleen Biagas, Fri May 26 08:31:00 MST 2017
#    Fixed reopen_04_01 for windows to use the same file as for non-windows.
#
# ----------------------------------------------------------------------------
from __future__ import print_function
import os
import string
import sys
import time
import shutil

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
            fileFrom = data_path("silo_hdf5_test_data/%s" % file)
            fileTo   = "%s%s" % (prefix, file)
            if not os.path.exists(fileFrom):
                print("Error: %s didn't exist" % fileFrom, file=sys.stderr)
            try:
                # Copy a file from the data directory to the current directory.
                if not sys.platform.startswith("win"):
                    os.link(fileFrom, fileTo)
                else:
                    shutil.copyfile(fileFrom, fileTo)
            except OSError:
                print("Don't need to copy %s" % file, file=sys.stderr)
        db = prefix + "wave*.silo database"
    else:
        # .visit file.
        db = "reopen_wave.visit"
        f = open(db, "wt")
        for file in files:
            f.write(data_path("silo_hdf5_test_data/%s\n" % file))
        f.close()
    return (db, prefixfiles)

# Function to remove the .visit file or the virtual database.
def DestroyMTFile(makeVisItFile, db):
    RemoveAllSiloAndVisItFiles()

# Function to create a new SAMRAI .visit file.
def CreateTimeVaryingMTFile(percent):
    # Read in the entire dumps.visit file for the SAMRAI data so that
    # we can get the list of files.
    f = open(data_path("samrai_test_data/sil_changes/dumps.visit"), "rt")
    lines = f.readlines()
    f.close()

    # Create a new .visit file
    nStates = len(lines)
    t = float(percent) / 100.
    maxState = int(float(nStates) * t)
    db = "reopen_samrai.visit"
    f = open(db, "wt")
    for i in range(maxState):
        f.write(data_path("samrai_test_data/sil_changes/%s") % lines[i])
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
def test1():
    #
    # This will open at timestep 0.  The variable "transient" will not be
    # available then.
    #
    db = silo_data_path("wave.visit") 
    OpenDatabase(db)

    #
    # Now set the time to a timestep when "transient" is available.
    #
    SetTimeSliderState(20)
    TestWindowInformation("reopen_1_00")

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
    Test("reopen_1_01")

    #
    # Delete the plots and close the database
    #
    DeleteAllPlots()
    CloseDatabase(db)
    TestWindowInformation("reopen_1_02")


#
# Now test that reopening a file actually makes the time slider longer. First
# create a new .visit file that we can add onto later.
#
def test2():
    VirtualDatabase = 0
    VisItFile = 1

    testIndex = 0
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
        Test("reopen_2_%02d" % testIndex)
        TestLength("reopen_2_%02d" % (testIndex + 1))

        # Create more time states in the file.
        db, files = CreateMTFile("", method, 60)
        ReOpenDatabase(db)
        # Go to the last time state.
        SetTimeSliderState(TimeSliderGetNStates() - 1)
        Test("reopen_2_%02d" % (testIndex + 2))
        TestLength("reopen_2_%02d" % (testIndex + 3))

        # Create more time states in the file.
        db, files = CreateMTFile("", method, 100)
        ReOpenDatabase(db)
        # Go to the last time state.
        SetTimeSliderState(TimeSliderGetNStates() - 1)
        Test("reopen_2_%02d" % (testIndex + 4))
        TestLength("reopen_2_%02d" % (testIndex + 5))

        # Get rid of the .visit file that we created.
        DestroyMTFile(method, db)
        DeleteAllPlots()
        CloseDatabase(db)

        # Get to the next testIndex
        testIndex = testIndex + 6


#
# Now that we've tested time-invariant databases, try testing reopen with
# a time-varying database to see if we get the right plots.
#
def test3():
    TestSection("Reopening .visit file of time-varying data")

    testIndex = 0
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
            Test("reopen_3_%02d" % testIndex)
            testIndex = testIndex + 1
            TestLength("reopen_3_%02d" % testIndex)
            testIndex = testIndex + 1

        else:
            # Reopen the database to add the new time states.
            ReOpenDatabase(db)

        # Go to the last time state.
        SetTimeSliderState(TimeSliderGetNStates() - 1)

        # Save a test
        Test("reopen_3_%02d" % testIndex)
        testIndex = testIndex + 1
        TestLength("reopen_3_%02d" % testIndex)
        testIndex = testIndex + 1

    # Clean up the time varying .visit file.
    DeleteAllPlots()
    CloseDatabase(db)
    DestroyTimeVaryingMTFile(db)

#
# Test that reopening a file that has been overwritten works.
#
def test4():
    TestSection("Reopening overwritten file to test engine")

    # Copy curv2d to the current directory.
    db = "test4.silo"
    if not sys.platform.startswith("win"):
        os.link(silo_data_path("curv2d.silo") , db)
    else: 
        shutil.copyfile(silo_data_path("curv2d.silo") , db)

    # Open up the file and create a plot.
    OpenDatabase(db)
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Test("reopen_4_00")

    # Delete the file
    try:
        os.unlink(db)
    except:
        # Ignore any exceptions
        pass

    if not sys.platform.startswith("win"):
        os.link(silo_data_path("rect2d.silo") , db)
    else: 
        shutil.copyfile(silo_data_path("rect2d.silo") , db)

    ReOpenDatabase(db)
    ResetView()
    Test("reopen_4_01")

    DeleteAllPlots()
    # Delete the file
    try:
        os.unlink(db)
    except:
        # Ignore any exceptions
        pass


#
# Test that expressions are not lost after reopening a file when there
# are multiple windows.
#
def test5():
    TestSection("Testing reopen/expressions with multiple windows")
    db = silo_data_path("curv3d.silo") 
    AddWindow()
    SetActiveWindow(1)
    OpenDatabase(db)
    TestExpressions("reopen_5_00")

    # See if the expressions are right after reopening.
    ReOpenDatabase(db)
    TestExpressions("reopen_5_01")

    # Delete the window that we added.
    SetActiveWindow(2)
    DeleteWindow()
    CloseDatabase(db)

#
# Test that time sliders are shortened when we reopen an MT database
# that has had time states removed.
#
def test6():
    TestSection("Testing reopen at an invalid time state")
    RemoveAllSiloAndVisItFiles()

    # Make sure that there is no open database.
    TestLength("reopen_6_00")

    # Create a short MT file.
    db, files = CreateMTFile("", 0, (10. / 71.) * 100.)

    # Create a plot and make sure its database has the right number
    # of time states.
    OpenDatabase(db)
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()
    ResetView()
    Test("reopen_6_01")
    TestLength("reopen_6_02")

    # Make a copy of the first window.
    CloneWindow()
    DrawPlots()
    Test("reopen_6_03")
    TestLength("reopen_6_04")
    SetActiveWindow(1)

    # Delete the last few time states
    nStates = len(files)
    for f in files[-5:]:
        try:
            os.unlink(f)
        except:
            print("Could not delete %s" % f, file=sys.stderr)

    # Change to a time state that we deleted. This should put the plot
    # in the error state and we should get an error message.
    SetTimeSliderState(nStates - 2)
    Test("reopen_6_05")
    TestText("reopen_6_06", GetLastError())

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
    Test("reopen_6_07")
    TestLength("reopen_6_08")

    # See if we're at the right time state in window 2 too.
    SetActiveWindow(2)
    DrawPlots()
    Test("reopen_6_09")
    TestLength("reopen_6_10")
    DeleteWindow()

    # Delete all of the plots in window 1 and close the database.
    DeleteAllPlots()
    CloseDatabase(db)

    # Delete the last few files.
    DestroyMTFile(0, db)


#
# Test reopening a database that has been removed from disk.
#
def test7():
    TestSection("Testing reopen on a deleted file")
    # Link a file from the data directory to the current directory.
    db = "reopen_globe.silo"
    if not sys.platform.startswith("win"):
        os.link(silo_data_path("globe.silo") , db)
    else:
        shutil.copyfile(silo_data_path("globe.silo") , db)

    OpenDatabase(db)
    AddPlot("Pseudocolor", "t")
    DrawPlots()
    Test("reopen_7_00")

    # Remove the file and make sure that we can't reopen it.
    RemoveAllSiloAndVisItFiles()
    if ReOpenDatabase(db):
        s = "VisIt was able to reopen " + db
    else:
        s = "VisIt was *NOT* able to reopen " + db + "!"
    TestText("reopen_7_01", s)

    # Do something that will make the plot be regenerated. Here we're changing
    # plot variables to force VisIt to recalculate the plot.
    ChangeActivePlotsVar("u")
    Test("reopen_7_02")
    DeleteAllPlots()
    CloseDatabase(db)


#
# Run the tests
#
try:
    # Remove all .silo and .visit files that could be left over from
    # previous failed runs.
    RemoveAllSiloAndVisItFiles()

    # Run the tests
    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()
except:
    # If we got any kind of exception, make sure that we get rid of
    # all of the .silo and .visit files that might be left.
    RemoveAllSiloAndVisItFiles()

    # Rethrow the exception so the test won't look like it passed if
    # it really didn't pass
    raise

Exit()
