# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mesh.py
#
#  Tests:      mesh      - 3D structured, single domain
#                        - 3D point, single domain
#                        - 2D point, single domain
#                        - 3D unstructured, single domain
#              plots     - mesh, pseudocolor
#
#  Defect ID:  none
#
#  Programmer: Kathleen Bonnell 
#  Date:       September 03, 2003
#
#  Modifications:
#    Kathleen Bonnell, Thu Sep  4 11:10:11 PDT 2003
#    Change the way MeshPlot's opaque mode is set (now an enum, not a bool).
#
#    Kathleen Bonnell, Thu Sep  4 11:10:11 PDT 2003
#    Added two test for new MeshAttribute 'showInternal':  mesh_globe_04
#    and mesh_rect3d_01.
#
#    Brad Whitlock, Thu Apr 8 12:50:21 PDT 2004
#    Added tests to test the auto mesh opacity flag.
#
#    Jeremy Meredith, Tue May  4 12:41:49 PDT 2004
#    Added test for unglyphed (i.e. GL_POINT) point meshes.
#
#    Kathleen Bonnell, Tue Nov  2 16:20:55 PST 2004 
#    Removed opaque-mode portion of mesh_point_01, as opaque-mode no longer
#    applies to Mesh plots of Point meshes. 
#
# ----------------------------------------------------------------------------

# Turn off all annotation

def InitAnnotation():
    a = AnnotationAttributes()
    a.axesFlag2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def TestCurve():
    TestSection("Mesh plot of a 3D curvilinear mesh")
    OpenDatabase("../data/curv3d.silo")
    AddPlot("Mesh", "curvmesh3d")
    DrawPlots()    

    v = GetView3D()
    v.viewNormal = (0.37, 0.48, 0.79)
    v.focus = (0, 2.5, 15)
    v.viewUp = (-0.18, 0.87, -0.45)
    v.imagePan = (0.08, 0.08)
    v.imageZoom = 2.0
    SetView3D(v)

    Test("mesh_curve_01")

    #Change line style, line width and color
    # and add a PC Plot.
    m = MeshAttributes()
    m.lineStyle = 3
    m.backgroundFlag = 0
    m.opaqueColor = (0, 122, 200, 255)
    SetPlotOptions(m)
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Test("mesh_curve_02")

    SetActivePlots(0)
    m.lineStyle = 2
    SetPlotOptions(m)
    SetActivePlots(1)
    HideActivePlots()
    Test("mesh_curve_03")
    DeleteAllPlots()

def TestPointMesh():
    TestSection("Mesh plot of a point mesh")
    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "PointVar")
    p = PseudocolorAttributes()
    p.pointType = p.Box
    p.pointSize = 1.0
    SetPlotOptions(p)

    DrawPlots()
    Test("mesh_point_01")
   
    p.pointType = p.Icosahedron
    p.pointSize = 1.5
    SetPlotOptions(p)
    DrawPlots()
    Test("mesh_point_02")

    DeleteActivePlots()
 
    AddPlot("Mesh", "PointMesh")
    m = MeshAttributes()
    m.foregroundFlag = 0
    m.meshColor = (0, 122, 200, 255)
    m.pointSize = 1.5
    m.pointType = m.Axis
    SetPlotOptions(m)
    DrawPlots()
    Test("mesh_point_03")

    m.pointType = m.Point
    m.pointSizePixels = 1
    SetPlotOptions(m)
    Test("mesh_point_04")

    DeleteAllPlots()

    OpenDatabase("../data/noise2d.silo")
    AddPlot("Mesh", "PointMesh")
    ResetView()
    DrawPlots()
    m.pointType = m.Axis
    m.pointSizeVarEnabled = 1
    m.pointSizeVar = "PointVar"
    m.pointSize = 0.05
    SetPlotOptions(m)
    Test("mesh_point_05")

    m.pointType = m.Point
    m.pointSizePixels = 1
    m.pointSizeVarEnabled = 0
    SetPlotOptions(m)
    Test("mesh_point_06")
    DeleteAllPlots()

def TestGlobe():
    TestSection("Mesh plot of a 3D unstructured mesh")
    OpenDatabase("../data/globe.silo")
    AddPlot("Mesh", "mesh1")
    DrawPlots()
    
    v = GetView3D()
    v.viewNormal = (1, 0, 0)
    v.focus = (0, 0, 0)
    v.viewUp = (0, 1, 0)
    v.imagePan = (0, 0)
    v.imageZoom = 1.0
    SetView3D(v)

    m = MeshAttributes()
    m.opaqueMode = m.Off
    SetPlotOptions(m)
    Test("mesh_globe_01")

    m.smoothingLevel = m.Fast
    SetPlotOptions(m)
    Test("mesh_globe_02")

    m.smoothingLevel = m.High
    SetPlotOptions(m)
    Test("mesh_globe_03")

    m.smoothingLevel = m.None
    m.opaqueMode = m.Auto
    m.showInternal = 1
    SetPlotOptions(m)
    ResetView()

    TurnMaterialsOff(("1", "2", "4"))

    AddOperator("Clip")
    clip = ClipAttributes()
    clip.plane1Status = 0
    clip.plane3Status = 1
    SetOperatorOptions(clip)

    AddOperator("Transform")
    t = TransformAttributes()
    t.doRotate = 1
    t.rotateAxis = (1, 0, 0)
    t.rotateAmount = 112
    SetOperatorOptions(t)

    SetViewExtentsType("actual") 
    Test("mesh_globe_04")

    DeleteAllPlots()

def TestRect3d():
    TestSection("Mesh plot of a 3D rectilinear mesh")
    OpenDatabase("../data/multi_rect3d.silo")
    AddPlot("Mesh", "mesh1")
    m = MeshAttributes()
    m.opaqueMode = m.Auto
    m.showInternal = 1
    SetPlotOptions(m)

    #Show a small area that contains the boundary
    #between domains 1 & 2 by using the box operator
    AddOperator("Box")
    box = BoxAttributes();
    box.minx = 0.3
    box.maxx = 0.4
    box.miny = 0.1
    box.maxy = 0.2
    box.minz = 0.1
    box.maxz = 0.2
    SetOperatorOptions(box)

    DrawPlots()
   
    SetViewExtentsType("actual") 
    v = GetView3D()
    v.viewNormal = (0.27, 0.27, 0.93)
    v.focus = (0.35, 0.15, 0.15)
    v.viewUp = (-0.06, 0.96, -0.26) 
    v.parallelScale = 0.139692
    v.nearPlane = -0.28
    v.farPlane = 0.28
    SetView3D(v)

    Test("mesh_rect3d_01")

    DeleteAllPlots()

def TestAutoOpaqueFlag():
    TestSection("Testing Mesh plot's opaque flag")

    # Set up a mesh plot with the auto opaque flag.
    OpenDatabase("../data/globe.silo")
    AddPlot("Mesh", "mesh1")
    m = MeshAttributes()
    m.opaqueMode = m.Auto
    SetPlotOptions(m)
    ResetView()
    TurnMaterialsOn()
    DrawPlots()
    Test("mesh_opaque_01")

    # Add a Pseudocolor plot and make sure that the mesh plot
    # is not drawn in opaque mode.
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    Test("mesh_opaque_02")
    
    # Hide the Pseudocolor plot and make sure that the mesh plot
    # goes opaque.
    HideActivePlots()
    Test("mesh_opaque_03")
    
    # Unhide the Pseudocolor plot and then delete it. Make sure that the
    # mesh plot goes opaque.
    HideActivePlots()
    Test("mesh_opaque_04")
    DeleteActivePlots()
    Test("mesh_opaque_05")
    DeleteAllPlots()

    # Restore a session file that has a keyfram animation where the mesh
    # plot exists over all frames but the Pseudocolor plot does not.
    RestoreSession("tests/plots/mesh_opaque.session", 0)
    Test("mesh_opaque_06")
    SetTimeSliderState(4)
    Test("mesh_opaque_07")
    SetTimeSliderState(9)
    Test("mesh_opaque_08")
    DeleteAllPlots()

def Main():
    InitAnnotation()
    TestCurve()
    TestPointMesh()
    TestGlobe()
    TestRect3d()
    TestAutoOpaqueFlag()

Main()
Exit()
