# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  shapefile.py
#
#  Tests:      mesh      - 2D polydata, single domain
#              plots     - Pseudocolor, Mesh, Subset
#
#  Programmer: Brad Whitlock
#  Date:       Mon Mar 28 11:06:05 PDT 2005
#
#  Modifications:
#
# ----------------------------------------------------------------------------

#
# Test a file that contains points.
#
def test0():
    TestSection("Test reading a shapefile that contains points")

    db = "../data/shapefile_test_data/m195mbsp/m195mbsp.shp"
    OpenDatabase(db)
    AddPlot("Mesh", "point")
    m = MeshAttributes()
    m.pointType = m.Box
    m.pointSize = 2000
    SetPlotOptions(m)
    DrawPlots()
    Test("shapefile_0_00")

    DeleteActivePlots()
    AddPlot("Pseudocolor", "DEPTH_M")
    p = PseudocolorAttributes()
    p.pointType = p.Point # hack -- remove this someday
    p.pointSizePixels = 1
    SetPlotOptions(p)
    DrawPlots()
    Test("shapefile_0_01")

    ChangeActivePlotsVar("LONGITUDE8")
    Test("shapefile_0_02")
    ChangeActivePlotsVar("LATITUDE83")
    Test("shapefile_0_03")
    DeleteActivePlots()
    CloseDatabase(db)

#
# Test a file that contains polylines.
#
def test1():
    TestSection("Test reading a shapefile that contains polylines")
    db = "../data/shapefile_test_data/morrison/499071-HP.shp"
    OpenDatabase(db)
    AddPlot("Mesh", "polyline")
    DrawPlots()
    Test("shapefile_1_00")

    DeleteActivePlots()
    AddPlot("Pseudocolor", "TPMAJOR_2")
    DrawPlots()
    Test("shapefile_1_01")

    ChangeActivePlotsVar("RWE_INDEX")
    Test("shapefile_1_02")

    DeleteActivePlots()
    CloseDatabase(db)

#
# Test a file that has polygons.
#
def test2():
    TestSection("Test reading shapefiles that contain polygons")
    db = "../data/shapefile_test_data/west/states.shp"
    OpenDatabase(db)
    AddPlot("Mesh", "polygon")
    DrawPlots()
    ResetView()
    Test("shapefile_2_00")

    DeleteActivePlots()
    AddPlot("Pseudocolor", "AVG_SALE87")
    DrawPlots()
    Test("shapefile_2_01")

    ChangeActivePlotsVar("MEDIANRENT")
    Test("shapefile_2_02")
    DeleteActivePlots()
    CloseDatabase(db)
    
    # Look at another file.
    db = "../data/shapefile_test_data/prism0p020/prism0p020.shp"
    OpenDatabase(db)
    AddPlot("Mesh", "polygon")
    DrawPlots()
    Test("shapefile_2_03")

    DeleteActivePlots()
    AddPlot("Pseudocolor", "RANGE")
    DrawPlots()
    Test("shapefile_2_04")
    
    # Zoom in on some interesting areas
    v = View2DAttributes()
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.windowCoords = (-124.76, -118.94, 43.9999, 49.3856)
    SetView2D(v)
    Test("shapefile_2_05")

    v.windowCoords = (-78.596, -74.3052, 36.34, 40.2596)
    SetView2D(v)
    Test("shapefile_2_06")

    v.windowCoords = (-83.2497, -79.3216, 24.3312, 29.1338)
    SetView2D(v)
    Test("shapefile_2_07")
    ResetView()
    DeleteActivePlots()
    CloseDatabase(db)

#
# Test multiple plots showing different features.
# 
def test3():
    TestSection("Test multiple files for showing different features")
    dbs = ("../data/shapefile_test_data/alameda/tgr06001lkA.shp", \
           "../data/shapefile_test_data/alameda/tgr06001cty00.shp", \
           "../data/shapefile_test_data/alameda/tgr06001wat.shp", \
           "../data/shapefile_test_data/alameda/tgr06001lkH.shp")
    # Put the roads on.
    OpenDatabase(dbs[0])
    AddPlot("Mesh", "polyline")
    g = 180
    m = MeshAttributes()
    m.foregroundFlag = 0
    m.meshColor = (g,g,g,255)
    m.legendFlag = 0
    SetPlotOptions(m)
    ResetView()
    DrawPlots()    

    # Put the county outline on.
    OpenDatabase(dbs[1])
    AddPlot("Mesh", "polygon")
    m = MeshAttributes()
    m.foregroundFlag = 0
    m.meshColor = (255,0,0,255)
    m.lineWidth = 1
    m.legendFlag = 0
    SetPlotOptions(m)
    DrawPlots()

    # Put the bodies of water on.
    OpenDatabase(dbs[2])
    AddPlot("Mesh", "polygon")
    m = MeshAttributes()
    m.foregroundFlag = 0
    m.meshColor = (0,0,255,255)
    m.lineWidth = 1
    m.legendFlag = 0
    SetPlotOptions(m)
    DrawPlots()

    # Put the streams on.
    OpenDatabase(dbs[3])
    AddPlot("Mesh", "polyline")
    m = MeshAttributes()
    m.foregroundFlag = 0
    m.meshColor = (150,150,255,255)
    m.legendFlag = 0
    SetPlotOptions(m)
    DrawPlots()

    t = CreateAnnotationObject("Text2D")
    t.text = "Alameda County"
    t.position = (0.39, 0.95)
    t.width = 0.4
    Test("shapefile_3_00")

    # Zoom in some
    v = View2DAttributes()
    v.windowCoords = (-121.845, -121.681, 37.608, 37.775)
    SetView2D(v)
    t.text = "Livermore"
    Test("shapefile_3_01")

    # Zoom in even more
    v.windowCoords = (-121.763, -121.718, 37.6533, 37.6983)
    SetView2D(v)
    llnl = CreateAnnotationObject("Text2D")
    llnl.width = 0.25
    llnl.position = (0.65, 0.5)
    llnl.text = "LLNL!"
    Test("shapefile_3_02")

    # Clean up
    t.Delete()
    llnl.Delete()
    DeleteAllPlots()
    for db in dbs:
        CloseDatabase(db)

#
# Test a .visit file of shapefiles
#
def test4():
    TestSection("Test a .visit file of shapefiles")
    db = "../data/shapefile_test_data/bayarearoads/roads.visit"
    OpenDatabase(db)
    AddPlot("Subset", "blocks")
    DrawPlots()
    t = CreateAnnotationObject("Text2D")
    t.text = "Bay area roads"
    t.position = (0.39, 0.7)
    t.width = 0.4
    ResetView()
    Test("shapefile_4_00")

    # Zoom in a little
    v = View2DAttributes()
    v.windowCoords = (-122.614, -121.92, 37.3495, 37.9982)
    SetView2D(v)
    t.position = (0.39, 0.9)
    Test("shapefile_4_01")
    
    # Zoom in more
    v.windowCoords = (-122.522, -122.362, 37.7017, 37.8424)
    SetView2D(v)
    Test("shapefile_4_02")

    # Zoom in more still
    v.windowCoords = (-122.488, -122.445, 37.7816, 37.825)
    SetView2D(v)
    t.position = (0.39, 0.95)
    gg = CreateAnnotationObject("Text2D")
    gg.text = "Golden Gate Bridge"
    gg.position = (0.22, 0.76)
    gg.width = 0.4
    Test("shapefile_4_03")

    # Delete the text annotations
    gg.Delete()
    t.Delete()

    DeleteActivePlots()
    CloseDatabase(db)

#
# Run the tests.
#
test0()
test1()
test2()
test3()
test4()

Exit()
