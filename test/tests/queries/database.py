# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  database.py #
#  Tests:      queries     - Database
#
#  Defect ID:  '6356
#
#  Programmer: Kathleen Bonnell 
#  Date:       July 11, 2003 
#
#  Modifications:
#    Kathleen Bonnell, Thu Aug 28 14:34:57 PDT 2003
#    Remove compound var name from FilledBoundary plots.
#
#    Kathleen Bonnell, Wed Oct 29 16:06:23 PST 2003 
#    Add PlotMinMax query. 
#
#    Kathleen Bonnell, Wed Dec  3 11:05:51 PST 2003 
#    Add SpatialExtents query. 
#
#    Kathleen Bonnell, Wed Dec  3 13:20:04 PST 2003 
#    Test SpatialExtents query with 'actual' and 'original' as args. 
#
#    Kathleen Bonnell, Mon Dec 22 16:46:27 PST 2003 
#    Added test for SAMRAI data.
#
#    Kathleen Bonnell, Fri Feb  6 10:56:25 PST 2004 
#    Test MinMax query with 'actual' and 'original' as args. 
#
#    Kathleen Bonnell, Fri Feb 20 17:05:04 PST 2004 
#    Added NumNodes and NumZones tests.
#
#    Kathleen Bonnell, Fri Mar  5 15:41:54 PST 2004 
#    Added QueryCurves tests, testing "Area Between Curves" queries. 
#
#    Jeremy Meredith, Mon Apr  5 14:19:47 PDT 2004
#    The Curv2D query was not getting boundary data.  I added code to remove
#    a couple materials so that it reflected the actual usage of this query.
#
#    Hank Childs, Tue Apr 13 12:58:04 PDT 2004
#    Rename surface area query.
#
#    Kathleen Bonnell, Tue Apr 20 09:42:30 PDT 2004 
#    Added QueryMultiWindow. 
#
#    Kathleen Bonnell, Wed May 19 16:31:04 PDT 2004
#    Moved QueryMultiWindow to its own .py file. 
#
#    Kathleen Bonnell, Thu Aug 12 13:32:35 PDT 2004 
#    Added QueryHistogram. 
#
#    Kathleen Bonnell, Wed Dec 29 07:57:35 PST 2004 
#    Added QueryGlobalId. 
#
#    Hank Childs, Wed Feb 16 07:34:07 PST 2005
#    Rename variables that have unsupported characters.
#
#    Hank Childs, Fri Jul  1 08:53:41 PDT 2005
#    Test getting a vector from the spatial extents query. ['6356]
#
#    Kathleen Bonnell, Wed Aug 10 15:07:50 PDT 2005 
#    Test Weighted Variable Sum Query with a postive-values variable and
#    a negative-values variable.  ('6453).
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Kathleen Bonnell, Wed Feb  8 11:08:01 PST 2006 
#    Added QueryZR_RZ. 
#
# ----------------------------------------------------------------------------

def QueryRect2d():
    OpenDatabase("../data/rect2d.silo")
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    # Do some database queries.
    Query("Compactness")
    s = GetQueryOutputString()
    s = s + "\n"
    Query("Eulerian")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Revolved volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("2D area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"

    AddOperator("Transform")
    transform = TransformAttributes()
    transform.doScale = 1
    transform.scaleX = 10
    SetOperatorOptions(transform)

    Query("SpatialExtents", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    Query("SpatialExtents", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
  
    TestText("QueryRect2d", s)
    DeleteAllPlots()

def QueryMultiUcd3d():
    OpenDatabase("../data/multi_ucd3d.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()

    # Do some database queries.
    Query("Eulerian")
    s = GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    v = GetQueryOutputValue()
    str = "Exts from vector are [%f-%f, %f-%f, %f-%f]\n" %(v[0], v[1], v[2], v[3], v[4], v[5])
    s = s + str
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    silr= SILRestriction();
    silr.TurnOffAll()
    silr.TurnOnSet(1)
    silr.TurnOnSet(3)
    silr.TurnOnSet(5)
    silr.TurnOnSet(10)
    SetPlotSILRestriction(silr)
    DrawPlots()
 
    s = s + "\nAFTER APPLYING SIL RESTRICTION:\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
 

    TestText("QueryMultiUcd3d", s)
    DeleteAllPlots()

def QueryContours():
    OpenDatabase("../data/noise.silo")
    AddPlot("Contour", "hardyglobal")
    c = ContourAttributes()
    c.contourNLevels = 10
    SetPlotOptions(c)
    DrawPlots()

    s = "For 10 contours of noise.silo (hardyglobal)\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n\nMinMax Actual:"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\nMinMax Original:"
    Query("MinMax", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    c.contourNLevels = 4
    SetPlotOptions(c)
    s  = s + "\nFor 4 contours of noise.silo (hardyglobal)\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n\nMinMax Actual:"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\nMinMax Original:"
    Query("MinMax", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
 
    DeleteAllPlots()  

    OpenDatabase("../data/rect3d.silo")
    AddPlot("Contour", "d")
    c.contourNLevels = 10
    SetPlotOptions(c)
    DrawPlots()

    s =  s + "\nFor 10 contours of rect3d.silo (d)\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    c.contourNLevels = 4
    SetPlotOptions(c)
    s  = s + "\nFor 4 contours of rect3d.silo (d)\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    TestText("QueryContours", s)

    DeleteAllPlots()  


def QueryCurv2d():
    # Testing database queries on curv2d.
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()

    # Do some database queries.
    TurnMaterialsOff("2")
    TurnMaterialsOff("3")
    Query("Compactness")
    TurnMaterialsOn()
    s = GetQueryOutputString()
    s = s + "\n"
    Query("Eulerian")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual") 
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Revolved volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("2D area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    DeleteAllPlots()
    AddPlot("Vector", "vel")
    DrawPlots()
    Query("MinMax", "actual")
    s = s + "\nMinMax Actual:" + GetQueryOutputString()
    s = s + "\nMinMax Original:"
    Query("MinMax", "original")
    s = s + GetQueryOutputString()

    TestText("QueryCurv2d", s)
    DeleteAllPlots()

def QueryCurv3d():
    OpenDatabase("../data/curv3d.silo")
    AddPlot("Pseudocolor", "p")
    DrawPlots()

    # Do some database queries.
    Query("Eulerian")
    s = GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    TestText("QueryCurv3d", s)
    DeleteAllPlots()

def QueryGlobe():
    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()

    # Do some database queries.
    Query("Eulerian")
    s = GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", 0)
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", 1)
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    AddOperator("Clip")
    clip = ClipAttributes()
    clip.plane1Origin = (0.2, 0.3, 0.4)
    clip.plane1Normal = (1, 1, 1)
    SetOperatorOptions(clip)

    s = s + "\nAFTER CLIPPING:\n" 
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", 0)
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", 1)
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"


    DeleteAllPlots()
    AddPlot("Vector", "vel")
    DrawPlots()
    Query("MinMax", "actual")
    s = s + "\nMinMax Actual:" + GetQueryOutputString()
    s = s + "\nMinMax Original:"
    Query("MinMax", "original")
    s = s + GetQueryOutputString()

    ChangeActivePlotsVar("disp")
    DrawPlots()
    Query("MinMax", "actual")
    s = s + "MinMAx Actual:" + GetQueryOutputString()

    DeleteAllPlots()
    AddPlot("Pseudocolor", "t")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + "\n" + GetQueryOutputString()
    DefineScalarExpression("neg_t", "-t")
    ChangeActivePlotsVar("neg_t")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + "\n" + GetQueryOutputString()
    s = s + "\n"
   
    TestText("QueryGlobe", s)
    DeleteAllPlots()

def QueryNoise():
    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()

    # Do some database queries.
    s = "For noise.silo(haryglobal)\n"
    Query("Eulerian")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    ChangeActivePlotsVar("hgslice")
    DrawPlots()
    # Do some database queries.
    s = s + "\nFor noise.silo(hgslice)\n"
    Query("Compactness")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Eulerian")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Revolved volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    DeleteAllPlots()
    AddPlot("Vector", "airVfGradient")
    DrawPlots()
    Query("MinMax", "actual")
    s = s + "\nMinMax Actual:" + GetQueryOutputString()
    s = s + "\nMinMax Original:" 
    Query("MinMax", "original")
    s = s + GetQueryOutputString()

    ChangeActivePlotsVar("grad")
    DrawPlots()
    Query("MinMax", "actual")
    s = s + "MinMax Actual:" + GetQueryOutputString()
    s = s + "\nMinMax Original:"
    Query("MinMax", "original")
    s = s + GetQueryOutputString()

    TestText("QueryNoise", s)
    DeleteAllPlots()

def QuerySid97():
    OpenDatabase("../data/sid97.silo")
    AddPlot("FilledBoundary", "mat1")
    DrawPlots()

    # Do some database queries.
    Query("3D surface area")
    s = GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    TestText("QuerySid97", s)
    DeleteAllPlots()

def QueryBigSil():
    OpenDatabase("../data/bigsil.silo")
    AddPlot("Pseudocolor", "dist")
    DrawPlots()

    # Do some database queries.
    Query("Eulerian")
    s = GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("3D surface area")
    s = s+GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    TestText("QueryBigSil", s)
    DeleteAllPlots()

def QueryMinMaxCurve():
    OpenDatabase("../data/curve.visit")
    AddPlot("Curve", "parabolic");
    DrawPlots()
    Query("MinMax", "actual") 
    s = "\n"
    frames = (0, 27, 56, 73, 91);
    for frame in frames:
        SetTimeSliderState(frame)
        s = s + "For entire curve at frame %d:" %frame 
        Query("MinMax", "actual") 
        s = s + GetQueryOutputString() 
  
    AddOperator("Clip") 
    clip = ClipAttributes()
    clip.planeInverse = 1
    clip.plane1Origin = (0.5, 0, 0)
    SetOperatorOptions(clip) 
    for frame in frames:
        SetTimeSliderState(frame)
        s = s + "For clipped curve at frame %d:" %frame 
        Query("MinMax", "actual") 
        s = s + GetQueryOutputString()

    TestText("QueryMinMaxCurve", s)
    DeleteAllPlots()

def QuerySAMRAI():
    OpenDatabase("../data/samrai_test_data/sil_changes/dumps.visit")
    AddPlot("Pseudocolor", "Primitive Var _number_0")
    DrawPlots()

    # Do some database queries.
    Query("Eulerian")
    s = GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("3D surface area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("Volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"

    TestText("QuerySAMRAI", s)
    DeleteAllPlots()

def QueryCurves():
    # in responses to VisIt00004449, Area between 2 curves query crashes
    # viewer if curves not from same database.
    OpenDatabase("../data/rect2d.silo")
    AddPlot("Pseudocolor", "u")
    DrawPlots()

    # do some lineouts so we can test curves from same database.
    Lineout((0.4, 1.04, 0), (0.4, 0.1, 0))
    SetActiveWindow(1)
    Lineout((0.02, 0.49, 0), (0.77, 0.49, 0))

    SetActiveWindow(2)
    SetActivePlots((0, 1))
    Query("Area Between Curves")
    s = GetQueryOutputString() + "\n"
    Query("L2Norm Between Curves")
    s = s + GetQueryOutputString() + "\n\n"
  
    # now add a curve from a different database, and test against 
    # previous curves.
    OpenDatabase("../data/c000.curve")
    AddPlot("Curve", "going_up")
    DrawPlots()

    SetActivePlots((0, 2))
    Query("Area Between Curves")
    s = s + GetQueryOutputString() + "\n"
    Query("L2Norm Between Curves")
    s = s + GetQueryOutputString() + "\n\n"

    SetActivePlots((1, 2))
    Query("Area Between Curves")
    s = s + GetQueryOutputString() + "\n"
    Query("L2Norm Between Curves")
    s = s + GetQueryOutputString() + "\n\n"

    TestText("QueryCurves", s)
    DeleteWindow()
    DeleteAllPlots()

def QueryHistogram():
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Histogram", "d")
    DrawPlots()
    
    Query("Integrate")
    s = "Histogram(d):  " + GetQueryOutputString() + "\n"

    ChangeActivePlotsVar("p")
    Query("Integrate")
    s = s + "Histogram(p):  " + GetQueryOutputString() + "\n"

    ChangeActivePlotsVar("u")
    Query("Integrate")
    s = s + "Histogram(u):  " + GetQueryOutputString() + "\n"

    ChangeActivePlotsVar("v")
    Query("Integrate")
    s = s + "Histogram(v):  " + GetQueryOutputString() + "\n\n"

    TestText("QueryHistogram", s)
    DeleteAllPlots()   

def QueryGlobalId():
    OpenDatabase("../data/global_node.silo")
    AddPlot("Pseudocolor", "dist")
    DrawPlots()

    Query("Global Zone Center", 15);
    s = GetQueryOutputString() + "\n"

    Query("Zone Center", 0, 2)
    s = s + GetQueryOutputString() + "\n"

    Query("Global Node Coords", 16)
    s = s + GetQueryOutputString() + "\n"

    Query("Node Coords", 1, 2)
    s = s + GetQueryOutputString() + "\n"

    TestText("QueryGlobalId", s)
    DeleteAllPlots()   

def QueryZR_RZ():
    s = "RZ UGRID:\n    "
    OpenDatabase("../data/vtk_cylindrical_test_data/ugrid_RZ.vtk")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    s = s + "ZR UGRID:\n    "
    ReplaceDatabase("../data/vtk_cylindrical_test_data/ugrid_ZR.vtk")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    s = s + "XY UGRID:\n    "
    ReplaceDatabase("../data/vtk_cylindrical_test_data/ugrid_XY.vtk")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    AddOperator("Transform")
    translate = TransformAttributes()
    translate.doTranslate = 1
    translate.translateX = 5
    translate.translateY = 17
    SetOperatorOptions(translate)
    DrawPlots()

    s = s + "XY UGRID TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    ReplaceDatabase("../data/vtk_cylindrical_test_data/ugrid_ZR.vtk")
    DrawPlots()

    s = s + "ZR UGRID TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    ReplaceDatabase("../data/vtk_cylindrical_test_data/ugrid_RZ.vtk")
    DrawPlots()

    s = s + "RZ UGRID TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    DeleteAllPlots()

    s = s + "RZ RGRID:\n    "
    OpenDatabase("../data/vtk_cylindrical_test_data/rect_RZ.vtk")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    s = s + "ZR RGRID:\n    "
    ReplaceDatabase("../data/vtk_cylindrical_test_data/rect_ZR.vtk")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    s = s + "XY RGRID:\n    "
    ReplaceDatabase("../data/vtk_cylindrical_test_data/rect_XY.vtk")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    AddOperator("Transform")
    translate = TransformAttributes()
    translate.doTranslate = 1
    translate.translateX = 5
    translate.translateY = 17
    SetOperatorOptions(translate)
    DrawPlots()

    s = s + "XY RGRID TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    ReplaceDatabase("../data/vtk_cylindrical_test_data/rect_ZR.vtk")
    DrawPlots()
    s = s + "ZR RGRID TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    ReplaceDatabase("../data/vtk_cylindrical_test_data/rect_RZ.vtk")
    DrawPlots()

    s = s + "RZ RGRID TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    DeleteAllPlots()

    s = s + "RZ POLY:\n    "
    OpenDatabase("../data/vtk_cylindrical_test_data/poly_RZ.vtk")
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    s = s + "ZR POLY:\n    "
    ReplaceDatabase("../data/vtk_cylindrical_test_data/poly_ZR.vtk")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    s = s + "XY POLY:\n    "
    ReplaceDatabase("../data/vtk_cylindrical_test_data/poly_XY.vtk")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    AddOperator("Transform")
    translate = TransformAttributes()
    translate.doTranslate = 1
    translate.translateX = 5
    translate.translateY = 17
    SetOperatorOptions(translate)
    DrawPlots()
    s = s + "XY POLY TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    ReplaceDatabase("../data/vtk_cylindrical_test_data/poly_ZR.vtk")
    DrawPlots()
    s = s + "ZR POLY TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    ReplaceDatabase("../data/vtk_cylindrical_test_data/poly_RZ.vtk")
    DrawPlots()

    s = s + "RZ POLY TRANSLATED:\n    "
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved volume")
    s = s + GetQueryOutputString() + "\n    "
    Query("Revolved surface area")
    s = s + GetQueryOutputString() + "\n\n"

    

    TestText("QueryZR_RZ", s)


def QueryMain():
    QueryBigSil()
    QueryContours()
    QueryCurv2d()
    QueryCurv3d()
    QueryGlobe()
    QueryMultiUcd3d()
    QueryNoise()
    QueryRect2d()
    QuerySid97()
    QueryMinMaxCurve()
    QuerySAMRAI()
    QueryCurves()
    QueryHistogram()
    QueryGlobalId()
    QueryZR_RZ()

# Call the main function
QueryMain()
Exit()

