# ----------------------------------------------------------------------------
#  CLASSES: nightly
#  LIMIT: 2000
#  Test Case:  mfem.py 
#
#  Tests:      mfem files 
#
#  Programmer: Cyrus Harrison
#  Date:       Thu Jun 12 14:58:03 PDT 2014
#
#  Modifications:
#    Brad Whitlock, Wed Sep  2 20:56:13 PDT 2020
#    The MFEM reader now provides original cells so artificial mesh lines
#    get removed. There's less of a point to seeing them here. Keep min/max
#    refinement levels only. 
# 
#    Justin Privitera, Mon Oct 17 17:33:30 PDT 2022
#    Added new tests for the new LOR settings. All prior tests use the legacy
#    LOR setting, while new tests use a mix of both.
#
#    Cyrus Harrison, Tue Oct 29 10:52:33 PDT 2024
#    Added wedge example mesh.
#
#    Cyrus Harrison, Mon Oct 20 11:17:19 PDT 2025
#    Added quadrature function examples
# 
#    Justin Privitera, Wed Dec 17 14:01:55 PST 2025
#    Reworked and added tests for MFEM LOR.
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("MFEM")

mfem_roots  = glob.glob(data_path("mfem_test_data/*.mfem_root"))
mfem_roots.extend(glob.glob(data_path("mfem_wedge_and_pyramid_examples/wedge_*.mfem_root")))
input_meshes = [ f for f in mfem_roots if f.count("ex0") == 0]
ex01_results = [ f for f in mfem_roots if f.count("ex01") == 1]
ex02_results = [ f for f in mfem_roots if f.count("ex02") == 1]
mfem_mesh_files = glob.glob(data_path("mfem_test_data/*.mesh"))
mfem_quad_func_files = glob.glob(data_path("mfem_quad_func_test_data/*.mfem_root"))
mfem_hdiv_hcurl_files = glob.glob(data_path("mfem_hdiv_hcurl_examples/*.mfem_root"))

# A selection of interesting meshes that effectively demonstrate 
# discontinuous versus continuous LOR settings
mfem_selected_meshes = glob.glob(data_path("mfem_test_data/amr-hex.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/ball-nurbs.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/fichera.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/klein-bottle.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/klein-donut.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/periodic-cube.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/periodic-hexagon.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/pipe-nurbs.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/square-disc-surf.mfem_root")) +\
                       glob.glob(data_path("mfem_test_data/star.mfem_root"))

def set_test_view():
    v = View3DAttributes()
    v.viewNormal = (-0.510614, 0.302695, 0.804767)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.0150532, 0.932691, -0.360361)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

def test_input_mesh_files():
    TestSection("Input Mesh Files")
    for f in input_meshes:
        base = os.path.splitext(os.path.basename(f))[0]
        DeleteAllPlots()
        OpenDatabase(f)
        AddPlot("Pseudocolor","main_element_coloring")
        #pc_atts = PseudocolorAttributes()
        #pc_atts.colorTableName = "PuBuGn"
        #SetPlotOptions(pc_atts)
        AddOperator("MultiresControl")
        AddPlot("Mesh","main")
        AddOperator("MultiresControl")
        SetActivePlots((0,1))
        levels = [0,20]
        if f.count("amr") > 0:
            levels = [0,5]
        for mres in levels:
            mc_atts = MultiresControlAttributes()
            mc_atts.resolution = mres
            mc_atts.meshRefMethod = mc_atts.Discontinuous_LOR
            SetOperatorOptions(mc_atts)
            ResetView()
            DrawPlots()
            Test("input_mesh_%s_mres_%04d" % (base,mres))
        DeleteAllPlots()
        CloseDatabase(f)

def test_mfem_expressions():
    TestSection("MFEM Expressions")
    OpenDatabase(data_path("mfem_test_data/ex02-beam-tet.mfem_root"))
    AddPlot("Pseudocolor","mag-gf")
    ResetView()
    DrawPlots()
    Test("mfem_expressions_1")
    DeleteAllPlots()
    AddPlot("Pseudocolor","comp0")
    DrawPlots()
    Test("mfem_expressions_2")
    DeleteAllPlots()
    AddPlot("Vector","curl-gf")
    DrawPlots()
    Test("mfem_expressions_3")
    DeleteAllPlots()
    CloseDatabase(data_path("mfem_test_data/ex02-beam-tet.mfem_root"))

def test_input_mesh_boundary_topology():
    TestSection("Input Mesh Boundary Topology")
    for f in input_meshes:
        base = os.path.splitext(os.path.basename(f))[0]
        DeleteAllPlots()
        OpenDatabase(f)
        AddPlot("Pseudocolor","main_boundary_attribute")
        AddPlot("Mesh","main_boundary")
        mesh_atts = MeshAttributes()
        mesh_atts.lineWidth = 2
        SetPlotOptions(mesh_atts)
        ResetView()
        DrawPlots()
        Test("input_mesh_%s_boundary_topo" % (base))
        DeleteAllPlots()
        CloseDatabase(f)

def test_direct_mesh_open():
    TestSection("Direct Mesh Open")
    for f in mfem_mesh_files:
        base = os.path.splitext(os.path.basename(f))[0]
        DeleteAllPlots()
        OpenDatabase(f)
        AddPlot("Pseudocolor","main_element_coloring")
        ResetView()
        DrawPlots()
        Test("direct_open_mesh_file_%s" % (base))
        DeleteAllPlots()
        CloseDatabase(f)

def test_mfem_quad_func(tag_name, dbfile, var_name):
    ResetView()
    base = os.path.splitext(os.path.basename(dbfile))[0]
    OpenDatabase(dbfile)
    # get the mesh metadata and find the mesh with "quad_func"
    md = GetMetaData(dbfile)
    plot_type = ""
    qf_mesh_name = ""
    for i in range(md.GetNumScalars()):
        smd = md.GetScalars(i)
        if smd.name == var_name:
            plot_type = "Pseudocolor"
            qf_mesh_name = smd.meshName
    for i in range(md.GetNumVectors()):
        vmd = md.GetVectors(i)
        if vmd.name == var_name:
            plot_type = "Vector"
            qf_mesh_name = vmd.meshName

    AddPlot(plot_type, var_name)
    AddPlot("Mesh",qf_mesh_name)
    mesh_atts = MeshAttributes()
    mesh_atts.lineWidth = 2
    mesh_atts.meshColor = (0, 0, 0, 255)
    mesh_atts.meshColorSource = mesh_atts.MeshCustom
    SetPlotOptions(mesh_atts)
    DrawPlots()

    AddPlot("Mesh","main")
    mesh_atts = MeshAttributes()
    mesh_atts.lineWidth = 4
    mesh_atts.meshColor = (255, 0, 255, 255)
    mesh_atts.meshColorSource = mesh_atts.MeshCustom
    SetPlotOptions(mesh_atts)
    DrawPlots()

    AddOperator("MultiresControl", 1)
    SetActivePlots(0)
    DrawPlots()

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_qf_plot_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_qf_plot_closeduniform_basis")

    DeleteAllPlots()
    ResetView()
    CloseDatabase(dbfile)

def test_mfem_quad_func_array(tag_name, dbfile, var_name):
    ResetView()
    base = os.path.splitext(os.path.basename(dbfile))[0]
    OpenDatabase(dbfile)

    qf_mesh_name = "main_quad_func_o5"

    AddPlot("Mesh", qf_mesh_name)
    mesh_atts = MeshAttributes()
    mesh_atts.lineWidth = 2
    mesh_atts.meshColor = (0, 0, 0, 255)
    mesh_atts.meshColorSource = mesh_atts.MeshCustom
    SetPlotOptions(mesh_atts)
    DrawPlots()

    AddPlot("Mesh", "main")
    mesh_atts = MeshAttributes()
    mesh_atts.lineWidth = 4
    mesh_atts.meshColor = (255, 0, 255, 255)
    mesh_atts.meshColorSource = mesh_atts.MeshCustom
    SetPlotOptions(mesh_atts)
    DrawPlots()

    AddOperator("MultiresControl", 1)
    SetActivePlots(0)
    DrawPlots()

    v = GetView2D()
    v.windowCoords = (-0.388962, 0.391808, -1.02008, -0.242779)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.windowValid = 1
    SetView2D(v)

    # Pick the quad function's mesh with the quad function variable.
    # These zones are quad points within a single main mesh zone.
    zones = (369, 370, 371, 372, 373, 374, 375, 376, 377)
    s = ""
    for z in zones:
       res = PickByZone(domain=0, element=z, vars=("dist_bins_qf",))
       keys = ("dist_bins_qf", "incident_nodes", "zone_id")
       subset = {}
       for k in keys:
         subset[k] = res[k]
       s = s + str(subset) + "\n"

    TestText(tag_name + "_" + base + "_picks", s)

    # Pick may have created a window. Delete it so we can close the database.
    SetActiveWindow(2)
    DeleteWindow()

    DeleteAllPlots()
    ResetView()
    CloseDatabase(dbfile)

def test_quadrature_functions():
    TestSection("Quadrature Functions")
    print(mfem_quad_func_files)
    for dbfile in mfem_quad_func_files:
        if dbfile.find("disk_quadfunc") != -1:
            test_mfem_quad_func_array("quadrature_data", dbfile, "dist_bins_qf")
        else:
            test_mfem_quad_func("quadrature_data", dbfile, "quad_field")

def test_mfem_lor_controls_on_mesh(tag_name, dbfile):
    ResetView()
    base = os.path.splitext(os.path.basename(dbfile))[0]

    OpenDatabase(dbfile)

    # we want to test a picture of a wireframe
    # new LOR should only have the outer edge
    AddPlot("Subset", "main")
    SubsetAtts = SubsetAttributes()
    SubsetAtts.wireframe = 1
    SetPlotOptions(SubsetAtts)
    AddOperator("MultiresControl", 1)
    SetActivePlots(0)
    set_test_view()
    DrawPlots()
    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Default_LOR
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_default_lor_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Default_LOR
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_default_lor_closeduniform_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_closeduniform_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Discontinuous_LOR
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_discontinuous_lor_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Discontinuous_LOR
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_discontinuous_lor_closeduniform_basis")

    DeleteAllPlots()
    ResetView()
    CloseDatabase(dbfile)

def test_mfem_lor_controls_on_grid_function(tag_name, f, vector=False, varname="gf"):
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    if vector is True:
        AddPlot("Vector",varname)
        VectorAtts = VectorAttributes()
        VectorAtts.origOnly = 0
        VectorAtts.useStride = 1
        VectorAtts.stride = 1
        SetPlotOptions(VectorAtts)
        AddPlot("Pseudocolor",varname + "_magnitude")
    else:
        AddPlot("Pseudocolor",varname)
    AddOperator("MultiresControl", 1)
    SetActivePlots(0)
    ResetView()
    DrawPlots()
    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Default_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Default_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_default_lor_default_proj_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Default_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Default_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_default_lor_default_proj_closeduniform_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Default_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_default_proj_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Default_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_default_proj_closeduniform_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Zonal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_zonal_proj_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Zonal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_zonal_proj_closeduniform_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Nodal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_nodal_proj_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Nodal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_continuous_lor_nodal_proj_closeduniform_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Discontinuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Default_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_discontinuous_lor_default_proj_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Discontinuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Default_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_discontinuous_lor_default_proj_closeduniform_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Discontinuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Nodal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_discontinuous_lor_nodal_proj_gausslobatto_basis")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 3
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Discontinuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Nodal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Closed_Uniform
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    Test(tag_name + "_" + base + "_discontinuous_lor_nodal_proj_closeduniform_basis")

    DeleteAllPlots()
    CloseDatabase(f)

def test_mfem_lor_controls_on_special_grid_function(tag_name, dbfile):
    base = os.path.splitext(os.path.basename(dbfile))[0]
    DeleteAllPlots()
    OpenDatabase(dbfile)
    AddPlot("Vector","gf")
    VectorAtts = VectorAttributes()
    VectorAtts.origOnly = 0
    VectorAtts.useStride = 1
    VectorAtts.stride = 1
    SetPlotOptions(VectorAtts)
    AddPlot("Mesh","main")
    MeshAtts = MeshAttributes()
    MeshAtts.showGenerated = 1
    SetPlotOptions(MeshAtts)
    SetActivePlots(0)
    ResetView()
    AddOperator("MultiresControl", 1)
    DrawPlots()
    View2DAtts = View2DAttributes()
    View2DAtts.windowCoords = (6, 8, -1.0, 0.5)
    View2DAtts.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    SetView2D(View2DAtts)
    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Zonal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)

    Test(tag_name + "_" + base + "_zonal_continuous")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Continuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Nodal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    
    Test(tag_name + "_" + base + "_nodal_continuous")

    MultiresControlAtts = MultiresControlAttributes()
    MultiresControlAtts.resolution = 0
    MultiresControlAtts.meshRefMethod = MultiresControlAtts.Discontinuous_LOR
    MultiresControlAtts.fieldProjMethod = MultiresControlAtts.Nodal_Projection
    MultiresControlAtts.refBasisType = MultiresControlAtts.Gauss_Lobatto_Default
    SetOperatorOptions(MultiresControlAtts, 0, 1)
    
    Test(tag_name + "_" + base + "_nodal_discontinuous")

    DeleteAllPlots()
    CloseDatabase(dbfile)

def test_lor_mesh_controls():
    TestSection("LOR Mesh Controls")
    # these are hand-picked meshes that clearly demonstrate the refinement differences
    for dbfile in mfem_selected_meshes:
        test_mfem_lor_controls_on_mesh("LOR_mesh", dbfile)

# ex01 results all have scalar grid functions
def test_lor_scalar_field_controls():
    TestSection("LOR Scalar Field Controls")
    for dbfile in ex01_results:
        test_mfem_lor_controls_on_grid_function("LOR_gf", dbfile)

def test_lor_vector_field_controls_close_up():
    TestSection("LOR Vector Field Controls Close Up")
    for dbfile in glob.glob(data_path("mfem_test_data/ex02-beam-quad.mfem_root")):
        test_mfem_lor_controls_on_special_grid_function("LOR_vector_gf_closeup", dbfile)

def test_lor_vector_field_controls():
    TestSection("LOR Vector Field Controls")
    # ex02 results all have scalar FE vector grid functions
    for dbfile in ex02_results:
        test_mfem_lor_controls_on_grid_function("LOR_vector_gf", dbfile, vector=True)
    # hdiv hcurl examples all have vector FE vector grid functions
    for dbfile in mfem_hdiv_hcurl_files:
        test_mfem_lor_controls_on_grid_function("LOR_vector_gf", dbfile, vector=True, varname="solution")

def main():
    test_input_mesh_files()
    test_mfem_expressions()
    test_input_mesh_boundary_topology()
    test_direct_mesh_open()
    test_quadrature_functions()
    test_lor_mesh_controls()
    test_lor_scalar_field_controls()
    test_lor_vector_field_controls_close_up()
    test_lor_vector_field_controls()

main()
Exit()
