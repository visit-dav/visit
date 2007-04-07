# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  moveoperators.py
#
#  Tests:      plots     - Pseudocolor, Mesh, Subset
#              operators - Erase, Isosurface, Reflect, Slice, Transform
#
#  Defect ID:  '1837
#
#  Programmer: Brad Whitlock
#  Date:       Thu Apr 17 16:45:46 PST 2003
#
#  Modifications:
#    Eric Brugger, Thu May  8 12:57:56 PDT 2003
#    Remove a call to ToggleAutoCenterMode since it no longer exists.
#
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from subset plots.
#
#    Kathleen Bonnell, Wed Mar 17 07:33:40 PST 2004 
#    Set default Slice atts, as these have changed. 
#
#    Kathleen Bonnell, Wed May  5 08:13:22 PDT 2004 
#    Modified Slice atts to get same picture as defaults have changed. 
#
#    Brad Whitlock, Tue Jan 17 12:14:21 PDT 2006
#    Added runTest4.
#
# ----------------------------------------------------------------------------

def InitAnnotation():
    # Turn off all annotation except for the bounding box.
    a = AnnotationAttributes()
    a.axesFlag2D = 1
    a.xAxisLabels2D = 0
    a.yAxisLabels2D = 0
    a.xAxisTitle2D = 0
    a.yAxisTitle2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 1
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def InitDefaults():
    # Set the default reflect operator attributes.
    reflect = ReflectAttributes()
    reflect.SetReflections(1, 1, 0, 0, 0, 0, 0, 0)
    SetDefaultOperatorOptions(reflect)
    slice = SliceAttributes()
    slice.project2d = 0
    slice.SetAxisType(slice.XAxis)
    slice.SetFlip(1)
    SetDefaultOperatorOptions(slice)

def setTheFirstView():
    # Set the view
    v = View3DAttributes()
    v.viewNormal = (-0.695118, 0.351385, 0.627168)
    v.focus = (-10, 0, 0)
    v.viewUp = (0.22962, 0.935229, -0.269484)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -70
    v.farPlane = 70
    v.perspective = 1
    SetView3D(v)

#
# Test operator promotion, demotion, and removal.
#
def runTest1():
    OpenDatabase("../data/noise.silo")

    # Set up a plot with a few operators.
    AddPlot("Pseudocolor", "hardyglobal")
    AddOperator("Isosurface")
    AddOperator("Slice")
    AddOperator("Reflect")
    DrawPlots()
    setTheFirstView()

    # Take a picture of the initial setup.
    Test("moveoperator_0")

    # Move the reflect so that it is before the slice in the pipeline.
    # The pipeline will be: Isosurface, Reflect, Slice
    DemoteOperator(2)
    DrawPlots()
    Test("moveoperator_1")

    # Move the reflect operator back so that the pipeline matches the
    # initial configuration: Isosurface, Slice, Reflect
    PromoteOperator(1)
    DrawPlots()
    Test("moveoperator_2")

    # Remove the slice operator from the middle, resulting in:
    # Isosurface, Reflect
    RemoveOperator(1)
    DrawPlots()
    Test("moveoperator_3")

    # Remove the Isosurface operator, resulting in: Reflect
    RemoveOperator(0)
    DrawPlots()
    Test("moveoperator_4")

    # Remove the Reflect operator
    RemoveOperator(0)
    DrawPlots()
    Test("moveoperator_5")

    DeleteAllPlots()

#
# Test removing an operator from more than one plot at the same time.
#
def runTest2():
    all = 1
    # Set up a couple plots of globe
    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "u")
    AddPlot("Mesh", "mesh1")
    # Add a reflect operator to both plots.
    AddOperator("Reflect", all)
    DrawPlots()
    Test("moveoperator_6")

    # Remove the operator from both plots.
    RemoveOperator(0, all)
    DrawPlots()
    Test("moveoperator_7")
    DeleteAllPlots()

#
# Test setting attributes for multiple operators of the same type.
#
def runTest3():
    # Set up a couple plots of globe
    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "u")
    pc = PseudocolorAttributes()
    pc.opacity = 0.2
    SetPlotOptions(pc)
    AddPlot("Subset", "mat1")

    # The subset plot is the active plot, add a couple transform
    # operators to it.
    AddOperator("Transform")
    AddOperator("Transform")

    # Set the attributes for the *first* transform operator.
    # This results in a full size globe translated up in Y.
    t0 = TransformAttributes()
    t0.doTranslate = 1
    t0.translateY = 15
    SetOperatorOptions(t0, 0)
    DrawPlots()
    Test("moveoperator_8")

    # Set the attributes for the *second* transform operator.
    # The plot has been translated, now scale it. Since it has already
    # been translated, this will also translate it a little in Y.
    t1 = TransformAttributes()
    t1.doScale = 1
    t1.scaleX = 0.5
    t1.scaleY = 0.5
    t1.scaleZ = 0.5
    SetOperatorOptions(t1, 1)
    Test("moveoperator_9")

    # Demote the last operator to reverse the order of the transformations.
    DemoteOperator(1)
    # Make the pc plot opaque again
    SetActivePlots(0)
    pc.opacity = 1
    SetPlotOptions(pc)
    DrawPlots()
    Test("moveoperator_10")

    DeleteAllPlots()

#
# Test that removing an operator using the RemoveOperator(i) method causes
# the vis window to get redrawn.
#
def runTest4():
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Pseudocolor", "d")
    AddOperator("Isosurface")
    DrawPlots()
    Test("moveoperator_11")

    RemoveOperator(0)
    Test("moveoperator_12")
    DeleteAllPlots()

#
# Set up the environment and run all of the tests.
#
def runTests():
    InitAnnotation()
    InitDefaults()
    runTest1()
    runTest2()
    runTest3()
    runTest4()

# Run the tests.
runTests()
Exit()
