# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case: pseudocolor.py 
#
#  Tests:      meshes    - 2D rectilinear, 3D curvilinear. 
#              plots     - pseudocolor 
#
#  Defect ID:  '1016, '987
#
#  Programmer: Kevin Griffin
#  Date:       March 19, 2015
#
#  Modifications:
#
#    Kevin Griffin, Thu Mar 19 12:00:23 PDT 2015
#    Add test for drawing pseudocolor plot using a log scaling
#    and very small min value.
#
#    Alister Maguire, Wed Jul 17 08:24:37 PDT 2019
#    Wrapped the first test in a function and added 
#    TestDomainBoundaries. 
#
#    Kathleen Biagas, Wed Nov  6 18:29:29 PST 2019
#    Add new tests: PointGlyphing MixedTopology Lines ObjectRenderingOptions
#
# ----------------------------------------------------------------------------

import itertools

def TestScale():
    OpenDatabase(silo_data_path("rect2d.silo"))
    
    AddPlot("Pseudocolor", "d")
    
    pc = PseudocolorAttributes()
    pc.minFlag = 1
    pc.min = 1e-5
    pc.maxFlag = 1
    pc.max = 1
    pc.centering = pc.Nodal
    pc.scaling = pc.Log
    SetPlotOptions(pc)
    DrawPlots()
    
    Test("pseudocolor_01")

    DeleteAllPlots()

def TestDomainBoundaries():
    #
    # First, let's remove some domains and make sure that the 
    # faces are rendered. 
    #
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))
    
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOnAll()
    for silSet in (41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,60,61,
                   62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,
                   82,83,84,85,86,87,88,89,90,91,92,93,94,95,97,98,99,100,101,
                   102,103,104,105,106,107,108,109,110,111,112,113,114,116,117,
                   118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,
                   133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,
                   148,149,150,151,152,153,154,236,237,238,239,240,241,242,243,
                   244):
        silr.TurnOffSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr ,1)
    
    Test("pseudocolor_domain_bounds_01")
    
    #
    # Next, we need to make sure that the processer boundaries are not
    # rendered when we enable transparency. 
    #
    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (0.883086, 0.11282, 0.455446)
    View3DAtts.focus = (0, 2.5, 10)
    View3DAtts.viewUp = (-0.0918142, 0.993447, -0.068068)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 11.4564
    View3DAtts.nearPlane = -22.9129
    View3DAtts.farPlane = 22.9129
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0, 2.5, 10)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)
    
    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOnAll()
    for silSet in (41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,60,61,
                   62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,
                   82,83,84,85,86,87,88,89,90,91,92,93,94,95,97,98,99,100,101,
                   102,103,104,105,106,107,108,109,110,111,112,113,114,116,117,
                   118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,
                   133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,
                   148,149,150,151,152,153,154):
        silr.TurnOffSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr ,1)
    
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.opacityType = PseudocolorAtts.Constant  
    PseudocolorAtts.opacity = 0.462745
    SetPlotOptions(PseudocolorAtts)
    
    Test("pseudocolor_domain_bounds_02")
    
    #
    # Now let's make sure that the ghost/face removal has been 
    # reversed back to its original order when we turn transparency
    # back off. 
    #
    PseudocolorAtts = PseudocolorAttributes()
    PseudocolorAtts.opacityType = PseudocolorAtts.FullyOpaque
    SetPlotOptions(PseudocolorAtts)
    
    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOnAll()
    for silSet in (41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,60,61,
                   62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,
                   82,83,84,85,86,87,88,89,90,91,92,93,94,95,97,98,99,100,101,
                   102,103,104,105,106,107,108,109,110,111,112,113,114,116,117,
                   118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,
                   133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,
                   148,149,150,151,152,153,154,236,237,238,239,240,241,242,243,
                   244):
        silr.TurnOffSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr ,1)
    
    Test("pseudocolor_domain_bounds_03")

    DeleteAllPlots()
    CloseDatabase(silo_data_path("multi_ucd3d.silo"))

def PointGlyphing():
    # github issue 3390
    TestSection("Point Glyphing: Topological dimension not set to 0")
    idx = itertools.count(1)
    # Reader not reporting topodim 0
    OpenDatabase(data_path("blueprint_v0.3.1_test_data/braid_2d_examples.blueprint_root_hdf5"))
    AddPlot("Pseudocolor", "points_mesh/braid")
    DrawPlots()
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    # glyph the points with Tets
    pc = PseudocolorAttributes()
    pc.pointType = pc.Tetrahedron
    pc.pointSize = 0.5
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))
    # transition back to point
    pc.pointType = pc.Point
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))
    # glyph the points with Sphere 
    pc.pointType = pc.Sphere
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    DeleteAllPlots()
    CloseDatabase(data_path("blueprint_v0.3.1_test_data/braid_2d_examples.blueprint_root_hdf5"))

    OpenDatabase(data_path("synergia_test_data/vis_particles_* database"), 0, "Vs")
    AddPlot("Pseudocolor", "particles_0")
    pc.pointType = pc.Tetrahedron
    pc.pointSize = 0.005
    SetPlotOptions(pc)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (-0.882869, -0.011028, 0.469489)
    v.focus = (-0.000169306, 0.000401067, -0.00167359)
    v.viewUp = (-0.118292, 0.972711, -0.199599)
    v.imageZoom = 1.77
    SetView3D(v); 
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    # PsersistentParticles sets the topological dimension incorrectly,
    # but the points should still be glyphable
    AddOperator("PersistentParticles")
    pp = PersistentParticlesAttributes()
    pp.connectParticles = 0
    SetOperatorOptions(pp)
    DrawPlots()
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    DeleteAllPlots()
    CloseDatabase(data_path("synergia_test_data/vis_particles_* database"))

    TestSection("Point Glyphing: Lighting")
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Pseudocolor", "PointVar")
    pc.pointType = pc.Icosahedron
    pc.pointSize = 3
    SetPlotOptions(pc)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (0.272848, 0.209847, 0.938892)
    v.focus = (-0.0242615, -0.170417, -0.0304632)
    v.viewUp = (0.134518, 0.958013, -0.253212)
    v.imageZoom=1.5
    SetView3D(v) 
    # standard lighting
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    # turn off lighting, points should still be glyphed
    pc.lightingFlag = 0
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))
    pc.lightingFlag = 1

    # github issue 1461, 1800
    TestSection("Point Glyphing: Changing Opacity")
    pc.opacityType = pc.Constant
    pc.opacity = 0.25
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    # change glyph type while transparent
    pc.pointType = pc.Box
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    pc.opacityType = pc.FullyOpaque

    DeleteAllPlots()
    CloseDatabase(silo_data_path("noise.silo"))

    TestSection("Unstructured grid, cell-centered data, poly-vertex cells")
    OpenDatabase(data_path("vtk_test_data/ugrid_polyvertex_points.vtk"))
    AddPlot("Pseudocolor", "foo_cells")
    DrawPlots()
    v2 = GetView2D()
    v2.windowCoords = (-0.945, 9.945, -0.945, 9.945)
    SetView2D(v2)
    pc.pointType = pc.Axis
    pc.pointSize = 0.6
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))
    pc.pointType = pc.Box
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))
    ChangeActivePlotsVar("u")
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    TestSection("scaling point glyphs by variable")
    pc.pointSizeVarEnabled = 1
    pc.pointSize = 0.1
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    pc.pointSizeVar = "foo_cells"
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))

    ChangeActivePlotsVar("foo_cells")
    Test("pseudocolor_point_glyphs_%02d" %next(idx))
    pc.pointSizeVar = "u"
    SetPlotOptions(pc)
    Test("pseudocolor_point_glyphs_%02d" %next(idx))
    
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/ugrid_polyvertex_points.vtk"))

    
def MixedTopology(): 
    TestSection("ugrid, vertex and poly vertex, line and polyline")
    OpenDatabase(data_path("vtk_test_data/ugrid_mixed_cells.vtk"))
    AddPlot("Pseudocolor","foo_cells")
    pc = PseudocolorAttributes()
    pc.colorTableName = "rainbow"
    pc.invertColorTable = 1
    pc.lineWidth = 2
    pc.pointSizePixels = 10 
    SetPlotOptions(pc)
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (-0.195485, 0.629643, 0.751888)
    v.focus = (1.5, 0.5, 0.5)
    v.viewUp = (0.158378, 0.776885, -0.609398)
    SetView3D(v)

    idx = itertools.count(1)
    Test("pseudocolor_mixed_cells_%02d"%next(idx))

    pc.pointType = pc.Tetrahedron
    pc.pointSize = 0.4
    SetPlotOptions(pc)
    Test("pseudocolor_mixed_cells_%02d"%next(idx))

    pc.lineType = pc.Tube
    pc.tubeRadiusBBox = 0.05
    SetPlotOptions(pc)
    Test("pseudocolor_mixed_cells_%02d"%next(idx))

    pc.tubeRadiusSizeType = pc.Absolute
    pc.tubeRadiusAbsolute = 0.125
    SetPlotOptions(pc)
    Test("pseudocolor_mixed_cells_%02d"%next(idx))

    pc.tubeRadiusSizeType = pc.FractionOfBBox
    pc.tailStyle = pc.Spheres 
    pc.headStyle = pc.Cones 
    SetPlotOptions(pc)
    Test("pseudocolor_mixed_cells_%02d"%next(idx))
   
    DeleteAllPlots() 
    CloseDatabase(data_path("vtk_test_data/ugrid_mixed_cells.vtk"))

def DoLines(ltype):
    pc = PseudocolorAttributes()
    idx =  itertools.count(1)
    if ltype == "tube":
        TestSection("Line Tubes")
        pc.lineType = pc.Tube
    else:
        TestSection("Line Ribbons")
        pc.lineType = pc.Ribbon

    pc.tubeRadiusSizeType = pc.FractionOfBBox
    pc.tubeRadiusBBox = 0.2
    SetPlotOptions(pc)
    Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

    if ltype == "tube":
        # test tube resolution
        pc.tubeResolution = 3
        SetPlotOptions(pc)
        Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

        pc.tubeResolution = 4
        SetPlotOptions(pc)
        Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

        pc.tubeResolution = 20 
        SetPlotOptions(pc)
        Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

    pc.tubeRadiusSizeType = pc.Absolute
    pc.tubeRadiusAbsolute = 0.6
    SetPlotOptions(pc)
    Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

    pc.tubeRadiusVarEnabled = 1
    pc.tubeRadiusVar = "var2"
    pc.tubeRadiusVarRatio = 2
    SetPlotOptions(pc)
    Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

    pc.tubeRadiusVar = "var1"
    SetPlotOptions(pc)
    Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

    ChangeActivePlotsVar("var1")
    Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

    pc.tubeRadiusVar = "var2"
    SetPlotOptions(pc)
    Test("pseudocolor_line_%s_%02d" %(ltype, next(idx)))

def DoEndpoints():
    TestSection("line endpoints")
    ChangeActivePlotsVar("var2")
    idx = itertools.count(1) 
    pc = PseudocolorAttributes()
    pc.lineType = pc.Tube
    pc.tubeRadiusSizeType = pc.Absolute
    pc.tubeRadiusAbsolute = 0.1
    pc.tailStyle = pc.Spheres
    pc.headStyle = pc.Cones
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    pc.endPointRadiusSizeType = pc.Absolute
    pc.endPointRadiusAbsolute = 0.5
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    pc.endPointResolution = 3
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    pc.endPointResolution = 20 
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    pc.endPointRadiusVarEnabled = 1
    pc.endPointRadiusVar = "var2"
    pc.endPointRadiusVarRatio = 2
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    pc.endPointRadiusVar = "var1"
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    ChangeActivePlotsVar("var2")
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    pc.endPointRadiusVar = "var2"
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))

    pc.endPointRatio = 1
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))
    
    pc.endPointRatio = 10
    SetPlotOptions(pc)
    Test("pseudocolor_line_endpoints_%02d"%next(idx))
    

def Lines():
    OpenDatabase(data_path("vtk_test_data/lines.vtk"))
    AddPlot("Pseudocolor", "var2")
    DrawPlots()
    ResetView()
    v = GetView3D()
    v.viewNormal = (0.508199, 0.390103, 0.767824)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.065159, 0.906394, -0.417379)
    v.imageZoom = 1.4641
    SetView3D(v)

    DoLines("tube")
    DoLines("ribbon")
    DoEndpoints()
   
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/lines.vtk"))


def ObjectRenderingOptions():
    TestSection("Object rendering options")
    idx = itertools.count(1)
    OpenDatabase(silo_data_path("rect3d.silo"))
    AddPlot("Pseudocolor", "p")
    DrawPlots()

    ResetView()
    v = GetView3D()
    v.viewNormal = (0.30955, 0.438716, 0.843627)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (0.00113002, 0.887033, -0.461704)
    SetView3D(v)

    Test("pseudocolor_rendering_options_%02d" %next(idx))
 
    pc = PseudocolorAttributes()
    pc.renderWireframe = 1
    pc.wireframeColor = (255, 0, 0)
    SetPlotOptions(pc) 
    Test("pseudocolor_rendering_options_%02d" %next(idx))

    pc.renderPoints = 1
    pc.pointColor = (255, 255, 0)
    SetPlotOptions(pc) 
    Test("pseudocolor_rendering_options_%02d" %next(idx))

    pc.renderSurfaces = 0
    SetPlotOptions(pc) 
    Test("pseudocolor_rendering_options_%02d" %next(idx))

    pc.renderPoints = 0
    SetPlotOptions(pc) 
    Test("pseudocolor_rendering_options_%02d" %next(idx))

    pc.renderWireframe = 0
    pc.renderPoints = 1
    SetPlotOptions(pc) 
    Test("pseudocolor_rendering_options_%02d" %next(idx))

    pc.pointType = pc.Sphere
    pc.pointSize = 0.02
    SetPlotOptions(pc) 
    Test("pseudocolor_rendering_options_%02d" %next(idx))
    
    pc.renderSurfaces = 1
    SetPlotOptions(pc) 
    Test("pseudocolor_rendering_options_%02d" %next(idx))

    DeleteAllPlots()
    CloseDatabase(silo_data_path("rect3d.silo"))
    
def Main():
    TestScale()
    TestDomainBoundaries()
    PointGlyphing()
    MixedTopology()
    Lines()
    ObjectRenderingOptions()

Main()
Exit()
