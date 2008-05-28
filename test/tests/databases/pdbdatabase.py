# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  pdbdatabase.py
#
#  Tests:      mesh      - 2D,3D curvilinear, single domain
#              plots     - Pseudocolor, Subset, Vector
#              operators - Clip
#
#  Programmer: Brad Whitlock
#  Date:       Thu Sep 25 09:31:28 PDT 2003
#
#  Modifications:
#    Brad Whitlock, Wed Mar 31 09:11:08 PDT 2004
#    I added code to clear the engine cache to reduce memory usage.
#
#    Brad Whitlock, Fri Apr 9 16:54:15 PST 2004
#    I added TestSection to divide up the tests a little.
#
#    Brad Whitlock, Thu Sep 2 12:08:59 PDT 2004
#    I replaced some deprecated calls with their new equivalents.
#
#    Brad Whitlock, Tue Dec 7 17:52:33 PST 2004
#    I added a test for mixvars in Flash files.
#
#    Mark C. Miller, Sat Feb  3 00:42:05 PST 2007
#    Added tests for array variables
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
TurnOffAllAnnotations(a)
a.databaseInfoFlag = 1
SetAnnotationAttributes(a)

##
## Set global attributes such that visit trys harder
## to get cycle/time information
##
#SetTryHarderCyclesTimes(1)

##
## This creates a name for a test.
##
def CreateTestName(testName, testIndex):
    name = "%s_%02d" % (testName, testIndex)
    return name

def sv3():
    v3 = View3DAttributes()
    v3.viewNormal = (0.516282, 0.582114, 0.628169)
    v3.focus = (0, 0, 0)
    v3.viewUp = (-0.488576, 0.80261, -0.342213)
    v3.viewAngle = 30
    v3.parallelScale = 43.589
    v3.nearPlane = -87.178
    v3.farPlane = 87.178
    v3.imagePan = (0, 0)
    v3.imageZoom = 1.41577
    v3.perspective = 1
    SetView3D(v3)

##
## This function performs the test using the specified database.
##
def TestWithDatabase(db, testName):
    TestSection("Testing with %s" % db)

    # Open the test database
    OpenDatabase(db)

    ##
    ## Do the 2D tests.
    ##

    # Add the plots.
    AddPlot("Subset", "material(mesh)")
    DrawPlots()

    # Do the first test in the series
    Test(CreateTestName(testName, 0))

    SetTimeSliderState(6)
    Test(CreateTestName(testName, 1))

    SetTimeSliderState(15)
    Test(CreateTestName(testName, 2))

    # Do a test on the last frame in the animation.
    SetTimeSliderState(22)
    Test(CreateTestName(testName, 3))

    AddPlot("Mesh", "mesh")
    DrawPlots()
    v = View2DAttributes()
    v.windowCoords = (-6.07862, -0.374491, 4.48986, 10.8545)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    SetView2D(v)
    Test(CreateTestName(testName, 4))

    # Try turning off material 2
    SetActivePlots((0,1))
    TurnMaterialsOff("2")
    Test(CreateTestName(testName, 5))
    TurnMaterialsOn()

    ResetView()
    DeleteAllPlots()
    AddPlot("Pseudocolor", "mesh/a")
    DrawPlots()
    Test(CreateTestName(testName, 6))

    # Define a expression. I'm testing this because of the strange
    # <mesh/var> syntax that my plugin has.
    DefineVectorExpression("testexp1", "3.0 * {<mesh/lt>, <mesh/a>/399.0}")
    AddPlot("Vector", "testexp1")
    DrawPlots();
    vec = VectorAttributes()
    vec.nVectors = 1200
    vec.colorByMag = 0
    SetPlotOptions(vec)
    v.windowCoords = (-9.51217, -0.289482, 0.983025, 10.6717)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    SetView2D(v)
    Test(CreateTestName(testName, 7))

    # Set the time back to frame 0
    SetTimeSliderState(0)
    ResetView()
    DeleteAllPlots()

    ##
    ## Do the 3D tests.
    ##

    AddPlot("Subset", "material2(revolved_mesh)")
    AddOperator("Clip")
    c = ClipAttributes()
    c.funcType = c.Plane
    c.plane1Status = 0
    c.plane2Status = 1
    c.plane3Status = 1
    SetOperatorOptions(c)
    DrawPlots()

    # Set the view
    sv3()
    Test(CreateTestName(testName, 8))

    SetTimeSliderState(6)
    sv3()
    Test(CreateTestName(testName, 9))

    SetTimeSliderState(15)
    sv3()
    Test(CreateTestName(testName, 10))

    # Do a test on the last frame in the animation.
    SetTimeSliderState(22)
    sv3()
    Test(CreateTestName(testName, 11))

    # Turn off some materials
    TurnMaterialsOff(("1", "3", "4"))
    sv3()
    Test(CreateTestName(testName, 12))
    TurnMaterialsOn()

    # Set the time back to frame 2
    SetTimeSliderState(2)
    ResetView()
    DeleteAllPlots()

    #
    # Test array variables
    #
    AddPlot("Pseudocolor","logical_mesh/marray_comps/comp_002")
    DrawPlots()
    Test(CreateTestName(testName, 13))

    DeleteAllPlots()
    ResetView()
    AddPlot("Pseudocolor","revolved_mesh/marray_comps/comp_002")
    DrawPlots()
    Test(CreateTestName(testName, 14))

    DeleteAllPlots()
    ResetView()
    AddPlot("Label","logical_mesh/marray")
    DrawPlots()
    Test(CreateTestName(testName, 15))

    # Set the time back to frame 0
    SetTimeSliderState(0)
    ResetView()
    DeleteAllPlots()
    CloseDatabase(db)
    ClearCache("localhost")

#
# Test mixvars.
#
def TestMixvars(db):
    TestSection("Testing mixvars in Flash files")
    DeleteAllPlots()
    OpenDatabase(db)
    AddPlot("Pseudocolor", "mesh/mixvar")
    DrawPlots()
    ResetView()
    v = View2DAttributes()
    v.windowCoords = (-9.51866, 3.29394, 13.9258, 26.4126)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off 
    SetView2D(v)
    Test("pdb_nomix")

    # Do the same plot but with forced MIR so the mixvar gets reconstructed.
    ClearWindow()
    m = GetMaterialAttributes()
    m.forceMIR = 1
    SetMaterialAttributes(m)
    DrawPlots()
    Test("pdb_mix")
    DeleteAllPlots()

#
# Run the test a few times with different versions of the database. We do this
# because we have the same database split up three different ways and all the
# ways a database can be split up must work.
#
#   multi{00,01,02}.pdb  - Contains multiple time states in each file but
#                          we group them all into "multi*.pdb database".
#
#   family??.pdb         - Contains a single time state in each file but
#                          we group them all into "family*.pdb database".
#
#   allinone00.pdb       - Contains all of the time states in one file.
#

databases = ("../data/multi*.pdb database", \
             "../data/family*.pdb database", \
             "../data/allinone00.pdb")
testNames = ("pdb_multi", "pdb_family", "pdb_allinone")
for i in range(len(databases)):
    TestWithDatabase(databases[i], testNames[i])

# Do the mixvar test.
TestMixvars(databases[2])

Exit()
