# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  blueprint.py 
#
#  Tests:      blueprint hdf5 files 
#
#  Programmer: Cyrus Harrison
#  Date:       Thu Jun 16 10:56:37 PDT 2016
#
#  Modifications:
#
#    Alister Maguire, Fri Jul 17 16:29:50 PDT 2020
#    Added a test to ensure that variables containing parentheses
#    are able to be visualized.
#
#    Justin Privitera Fri 04 Mar 2022 05:57:49 PM PST
#    Added tests to test new implicit points topologies.
# 
#    Justin Privitera, Mon May 23 17:53:56 PDT 2022
#    Added polytopal tests.
# 
#    Justin Privitera, Mon May 23 17:53:56 PDT 2022
#    Added mfem lor tests.
#
#    Cyrus Harrison, Thu Dec 22 13:53:17 PST 2022
#    Added bp part map and initial strided structured tests/ 
#
#    Cyrus Harrison, Mon, Mar 20, 2023  3:34:04 PM 
#    Added rz test examples
# 
#    Justin Privitera, Wed Mar 22 16:09:52 PDT 2023
#    Added 1d curve test examples.
# 
#    Justin Privitera, Fri Mar 24 17:47:26 PDT 2023
#    Fixed view issue for mfem lor vector field tests.
#    Fixed an issue where test names had two consecutive underscores.
# 
#    Justin Privitera, Tue Sep 19 11:36:45 PDT 2023
#    Added a test for material numbers not in the range [0, N)
# 
#    Justin Privitera, Wed Oct 25 17:29:07 PDT 2023
#    Added a test for a polygonal mesh with no offsets.
# 
#    Justin Privitera, Thu Jan 18 14:53:32 PST 2024
#    Added test for unstructured points.
# 
#    Justin Privitera, Tue Mar 12 13:29:30 PDT 2024
#    Now that blueprint presents material ids alongside material names, we 
#    must use the ids and names to get specific materials.
# 
#    Justin Privitera, Fri Mar 15 15:56:13 PDT 2024
#    Revert prior change.
# 
#    Justin Privitera, Fri May  3 09:55:25 PDT 2024
#    Added test for Blueprint Uniform Coordset + Unstructured Topo.
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Blueprint")

from os.path import join as pjoin

bp_test_dir = "blueprint_v0.3.1_test_data"
bp_venn_test_dir = "blueprint_v0.7.0_venn_test_data"
bp_mfem_test_dir = "blueprint_v0.3.1_mfem_test_data"
bp_0_8_2_test_dir = "blueprint_v0.8.2_braid_examples_test_data"
bp_poly_test_dir = "blueprint_v0.8.2_polytess_test_data"
bp_devilray_mfem_test_dir = "blueprint_v0.8.3_devilray_mfem_test_data"
bp_part_map_test_dir = "blueprint_v0.8.4_part_map_examples"
bp_struct_strided_test_dir = "blueprint_v0.8.4_strided_structured_examples"
bp_rz_test_dir = "blueprint_v0.8.6_rz_examples"
bp_1d_curve_test_dir = "blueprint_v0.8.6_1d_curve_examples"
bp_venn_modded_matnos_dir = "blueprint_v0.8.7_venn_modded_matnos_example"
bp_poly_no_offsets_dir = "blueprint_v0.8.7_polytopal_mesh_no_offsets"
bp_unstructured_points_dir = "blueprint_v0.8.7_unstructured_points"
bp_unstructured_uniform_dir = "blueprint_v0.9.1_uniform_coords_unstructured_topo"
bp_mixed_topos_dir = "blueprint_v0.9.2_mixed_topo_data"

braid_2d_hdf5_root = data_path(pjoin(bp_test_dir,"braid_2d_examples.blueprint_root_hdf5"))
braid_3d_hdf5_root = data_path(pjoin(bp_test_dir,"braid_3d_examples.blueprint_root_hdf5"))

braid_2d_json_root = data_path(pjoin(bp_test_dir,"braid_2d_examples_json.root"))
braid_3d_json_root = data_path(pjoin(bp_test_dir,"braid_3d_examples_json.root"))

braid_2d_sidre_root = data_path(pjoin(bp_test_dir,"braid_2d_sidre_examples.root"))
braid_3d_sidre_root = data_path(pjoin(bp_test_dir,"braid_3d_sidre_examples.root"))

braid_2d_0_8_2_hdf5_root = data_path(pjoin(bp_0_8_2_test_dir,"braid_2d_examples_hdf5.root"))
braid_3d_0_8_2_hdf5_root = data_path(pjoin(bp_0_8_2_test_dir,"braid_3d_examples_hdf5.root"))
braid_2d_0_8_2_yaml_root = data_path(pjoin(bp_0_8_2_test_dir,"braid_2d_examples_yaml.root"))
braid_3d_0_8_2_yaml_root = data_path(pjoin(bp_0_8_2_test_dir,"braid_3d_examples_yaml.root"))

poly_2d_hdf5_root = data_path(pjoin(bp_poly_test_dir,"polytess_example_hdf5.root"))
poly_2d_yaml_root = data_path(pjoin(bp_poly_test_dir,"polytess_example_yaml.root"))
poly_3d_hdf5_root = data_path(pjoin(bp_poly_test_dir,"polytess_3d_example_hdf5.root"))
poly_3d_yaml_root = data_path(pjoin(bp_poly_test_dir,"polytess_3d_example_yaml.root"))

uniform_root = data_path(pjoin(bp_test_dir,"uniform.cycle_001038.root"))

unstructured_points = data_path(pjoin(bp_unstructured_points_dir,"unstructured_points.cycle_000100.root"))

uniform_unstructured = data_path(pjoin(bp_unstructured_uniform_dir,"partition.root"))

mixed_topo_2d = data_path(pjoin(bp_mixed_topos_dir, "mixed_mesh_simple_2d_hdf5.root"))
mixed_topo_3d = data_path(pjoin(bp_mixed_topos_dir, "mixed_mesh_simple_3d_hdf5.root"))
mixed_braid_2d = data_path(pjoin(bp_mixed_topos_dir, "braid_2d_examples_hdf5.root"))
mixed_braid_3d = data_path(pjoin(bp_mixed_topos_dir, "braid_3d_examples_hdf5.root"))

#
# venn test data (multi material)
#

venn_full_root  =  data_path(pjoin(bp_venn_test_dir,
                             "venn_small_example_full_hdf5.root"))
venn_s_by_e_root  =  data_path(pjoin(bp_venn_test_dir,
                               "venn_small_example_sparse_by_element_hdf5.root"))

venn_s_by_m_root  =  data_path(pjoin(bp_venn_test_dir,
                               "venn_small_example_sparse_by_material_hdf5.root"))

venn_full_yaml_root  =  data_path(pjoin(bp_venn_test_dir,
                             "venn_small_example_full_yaml.root"))
venn_s_by_e_yaml_root  =  data_path(pjoin(bp_venn_test_dir,
                               "venn_small_example_sparse_by_element_yaml.root"))

venn_s_by_m_yaml_root  =  data_path(pjoin(bp_venn_test_dir,
                               "venn_small_example_sparse_by_material_yaml.root"))

venn_modded_matnos_root = data_path(pjoin(bp_venn_modded_matnos_dir,
                                "venn_w_modded_matnos.root"))

polytopal_mesh_no_offsets_root = data_path(pjoin(bp_poly_no_offsets_dir,
                                "polytopal_mesh_no_offsets.root"))

bp_part_map_root = data_path(pjoin(bp_part_map_test_dir,
                             "tout_custom_part_map_index_hdf5.root"))

bp_part_map_single_file_root = data_path(pjoin(bp_part_map_test_dir,
                                         "tout_single_file_part_map_index_hdf5.root"))

bp_spiral_part_map_examples = []
for i in range(-1,9):
    bp_spiral_part_map_examples.append(data_path(pjoin(bp_part_map_test_dir,
                                       "tout_relay_spiral_mesh_save_nfiles_{0}.cycle_000000.root".format(i))))

bp_sparse_topos_root = data_path(pjoin(bp_part_map_test_dir,
                                        "tout_relay_bp_mesh_sparse_topos_hdf5.root"))


bp_struct_strided_2d_root = data_path(pjoin(bp_struct_strided_test_dir, 
                                      "strided_structured_2d_hdf5.root"));


bp_struct_strided_3d_root = data_path(pjoin(bp_struct_strided_test_dir, 
                                      "strided_structured_3d_hdf5.root"));

bp_rz_examples = []
bp_rz_examples.append(data_path(pjoin(bp_rz_test_dir,"blueprint_rz_cyl_rectilinear.root")))
bp_rz_examples.append(data_path(pjoin(bp_rz_test_dir,"blueprint_rz_cyl_uniform.root")))
bp_rz_examples.append(data_path(pjoin(bp_rz_test_dir,"blueprint_rz_cyl_structured.root")))
bp_rz_examples.append(data_path(pjoin(bp_rz_test_dir,"blueprint_rz_cyl_unstructured.root")))

bp_1d_curve_examples = [data_path(pjoin(bp_1d_curve_test_dir,"curves_1d.root"))]

braid_2d_meshes = ["points", "uniform", "rect", "struct", "tris","quads"]
braid_3d_meshes = ["points", "uniform", "rect", "struct", "tets","hexs"]

braid_2d_meshes_0_8_2 = ["points", "uniform", "rect", "struct", "tris","quads", "points_implicit"]
braid_3d_meshes_0_8_2 = ["points", "uniform", "rect", "struct", "tets","hexs", "points_implicit"]

class mfem_ex9_example:
    def __init__(self, name, dim):
        self.name = name
        self.dim = dim

mfem_ex9_examples = []
mfem_ex9_examples.append(mfem_ex9_example("periodic_cube", "3d"))
mfem_ex9_examples.append(mfem_ex9_example("star_q3", "3d"))
mfem_ex9_examples.append(mfem_ex9_example("periodic_hexagon", "2d"))
mfem_ex9_protocols = ["json","conduit_bin","conduit_json","hdf5"]

class devilray_mfem_example:
    def __init__(self, name, number, pseudo_fields, vector_fields, dim):
        self.name = name
        self.number = number
        self.pseudo_fields = pseudo_fields
        self.vector_fields = vector_fields
        self.dim = dim

devilray_mfem_examples = []
devilray_mfem_examples.append(devilray_mfem_example("esher", "000000", ["mesh_nodes_magnitude"], [], "3d"))
devilray_mfem_examples.append(devilray_mfem_example("laghos_tg.cycle", "000350", ["mesh_nodes_magnitude", "density", "specific_internal_energy", "velocity_magnitude"], ["velocity"], "3d"))
devilray_mfem_examples.append(devilray_mfem_example("taylor_green.cycle", "001860", ["mesh_nodes_magnitude", "density", "specific_internal_energy", "velocity_magnitude"], ["velocity"], "3d"))
devilray_mfem_examples.append(devilray_mfem_example("tri_beam", "000000", ["mesh_nodes_magnitude"], ["mesh_nodes"], "2d"))
devilray_mfem_examples.append(devilray_mfem_example("warbly_cube", "000000", ["mesh_nodes_magnitude"], [], "3d"))

def full_mesh_name(mesh_name):
    return mesh_name + "_mesh"

def full_var_name(mesh_name,var_name):
    return full_mesh_name(mesh_name) + "/" + var_name

def mfem_test_file(name, protocol):
    return data_path(pjoin(bp_mfem_test_dir,"bp_mfem_ex9_%s_%s_000000.root" % ( name, protocol)))

def devilray_mfem_test_file(name, number):
    return data_path(pjoin(bp_devilray_mfem_test_dir, name + "_" + number + ".root"))

def set_3d_view():
    v = View3DAttributes()
    v.viewNormal = (-0.510614, 0.302695, 0.804767)
    v.focus = (0.5, 0.5, 0.5)
    v.viewUp = (-0.0150532, 0.932691, -0.360361)
    v.viewAngle = 30
    v.parallelScale = 0.866025
    v.nearPlane = -1.73205
    v.farPlane = 1.73205
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.5, 0.5, 0.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)

def set_3d_poly_view():
    v = View3DAttributes()
    v.viewNormal = (-0.50151, -0.632781, 0.589979)
    v.focus = (-2.46025e-13, 6.43929e-14, 4.5)
    v.viewUp = (0.364523, 0.463898, 0.807416)
    v.viewAngle = 30
    v.parallelScale = 7.93494
    v.nearPlane = -15.8699
    v.farPlane = 15.8699
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (-2.46025e-13, 6.43929e-14, 4.5)
    v.axis3DScaleFlag = 0
    v.axis3DScales = (1, 1, 1)
    v.shear = (0, 0, 1)
    v.windowValid = 1
    SetView3D(v)


def set_test_view(tag_name):
    if "3d" in tag_name:
        if "poly" in tag_name:
            set_3d_poly_view()
        else:
            set_3d_view()
    else:
        ResetView()

def test(mesh_name,tag_name):
    #
    AddPlot("Mesh","%s" % full_mesh_name(mesh_name))
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  mesh_name + "_mesh")
    DeleteAllPlots()
    ResetView()
    #
    AddPlot("Pseudocolor", full_var_name(mesh_name,"braid"))
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  mesh_name + "_braid")
    DeleteAllPlots()
    ResetView()
    #
    AddPlot("Pseudocolor", full_var_name(mesh_name,"radial"))
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  mesh_name + "_radial")
    DeleteAllPlots()
    ResetView()

def test_poly(tag_name):
    #
    AddPlot("Mesh", "mesh_topo")
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_mesh_topo_mesh")
    DeleteAllPlots()
    ResetView()
    #
    AddPlot("Pseudocolor", "mesh_topo/level")
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_mesh_topo_level")
    DeleteAllPlots()
    ResetView()

def test_mfem(tag_name, example_name, protocol):
    readOptions = GetDefaultFileOpenOptions("Blueprint")
    readOptions["MFEM LOR Setting"] = "Legacy LOR"
    SetDefaultFileOpenOptions("Blueprint", readOptions)

    dbfile = mfem_test_file(example_name,protocol)
    OpenDatabase(dbfile)
    #
    AddPlot("Pseudocolor", "mesh_main/solution")
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  example_name + "_" + protocol + "_sol")
    #
    DeleteAllPlots()
    ResetView()
    AddPlot("Pseudocolor", "mesh_main/element_coloring")
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  example_name + "_" + protocol + "_ele_coloring")
    #
    DeleteAllPlots()
    ResetView()
    AddPlot("Pseudocolor", "mesh_main/element_attribute")
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  example_name + "_" + protocol + "_ele_att")
    #
    DeleteAllPlots()
    ResetView()

    CloseDatabase(dbfile)

    # reset read options to default
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("Blueprint", readOptions)

def test_mfem_lor_mesh(tag_name, example_name, protocol, devilray = False, number = "000000"):
    dbfile = ""
    if (devilray):
        dbfile = devilray_mfem_test_file(example_name, number)
    else:
        dbfile = mfem_test_file(example_name, protocol)

    # get default options
    readOptions = GetDefaultFileOpenOptions("Blueprint")
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("Blueprint", readOptions)
    OpenDatabase(dbfile)

    # we want to test a picture of a wireframe
    # new LOR should only have the outer edge
    AddPlot("Subset", "mesh_main")
    SubsetAtts = SubsetAttributes()
    SubsetAtts.wireframe = 1
    SetPlotOptions(SubsetAtts)
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  example_name + ("_" + protocol if not devilray else "") + "_lor")
    DeleteAllPlots()
    ResetView()
    CloseDatabase(dbfile)

    ##############################

    # examine legacy
    readOptions = GetDefaultFileOpenOptions("Blueprint")
    readOptions["MFEM LOR Setting"] = "Legacy LOR"
    SetDefaultFileOpenOptions("Blueprint", readOptions)
    OpenDatabase(dbfile)

    # old LOR leaves a busy wireframe
    AddPlot("Subset", "mesh_main")
    SubsetAtts = SubsetAttributes()
    SubsetAtts.wireframe = 1
    SetPlotOptions(SubsetAtts)
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  example_name + ("_" + protocol if not devilray else "") + "_legacy_lor")
    DeleteAllPlots()
    ResetView()
    CloseDatabase(dbfile)

    # restore default
    readOptions = GetDefaultFileOpenOptions("Blueprint")
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("Blueprint", readOptions)

def test_mfem_lor_field(tag_name, name, number, pseudocolor_fields, vector_fields):
    readOptions = GetDefaultFileOpenOptions("Blueprint")
    readOptions["MFEM LOR Setting"] = "MFEM LOR"
    SetDefaultFileOpenOptions("Blueprint", readOptions)
    dbfile = devilray_mfem_test_file(name, number)
    OpenDatabase(dbfile)
    
    for field in pseudocolor_fields:
        AddPlot("Pseudocolor", "mesh_main/" + field, 1, 1)
        AddOperator("MultiresControl", 1)
        SetActivePlots(0)
        MultiresControlAtts = MultiresControlAttributes()
        MultiresControlAtts.resolution = 3
        SetOperatorOptions(MultiresControlAtts, 0, 1)
        set_test_view(tag_name)
        DrawPlots()
        Test(tag_name + "_" + name + "_pseudocolor_" + field + "_lor")
        DeleteAllPlots()
        ResetView()

    for field in vector_fields:
        AddPlot("Vector", "mesh_main/" + field, 1, 1)  
        AddOperator("MultiresControl", 1)
        SetActivePlots(0)
        MultiresControlAtts = MultiresControlAttributes()
        MultiresControlAtts.resolution = 3
        SetOperatorOptions(MultiresControlAtts, 0, 1)
        set_test_view(tag_name)
        DrawPlots()
        Test(tag_name + "_" + name + "_vector_" + field + "_lor")
        DeleteAllPlots()
        ResetView()

    CloseDatabase(dbfile)

def test_venn(tag_name, venn_db_file):
    TestSection("Blueprint Matset Example Tests: {0} ".format(tag_name))
    OpenDatabase(venn_db_file)
    AddPlot("Pseudocolor", "mesh_topo/mat_check")
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_mat_check")
    # value check
    Query("minmax")
    res = GetQueryOutputObject()
    print(res)
    # We expect:
    #int(res["max"]) == 4320
    #int(res["min"]) == 1
    TestValueEQ("testeq_t_mat_check_max",int(res["max"]),4320)
    TestValueEQ("testeq_mat_check_min",int(res["min"]),1)
    DeleteAllPlots()
    ResetView()


    TestSection("Blueprint Matset Example Tests: {0} Matvf Exprs".format(tag_name))

    # check all volume fractions
    vf_exprs = {"vf_bg":'matvf(mesh_topo_matset,"background")',
                "vf_c_a":'matvf(mesh_topo_matset,"circle_a")',
                "vf_c_b":'matvf(mesh_topo_matset,"circle_b")',
                "vf_c_c":'matvf(mesh_topo_matset,"circle_c")'}

    for ename,edef in vf_exprs.items():
        DefineScalarExpression(ename,edef);
        AddPlot("Pseudocolor", ename)
        set_test_view(tag_name)
        DrawPlots()
        Test(tag_name + "_" + ename)
        # value check
        Query("minmax")
        res = GetQueryOutputObject()
        print(res)
        # We expect:
        #res["max"] == 1.0
        #res["min"] == 0.0
        TestValueEQ("testeq_" + ename + "_max",res["max"],1.0)
        TestValueEQ("testeq_" + ename + "_min",res["min"],0.0)
        DeleteAllPlots()
        ResetView()

    TestSection("Blueprint Matset Example Tests: {0} Val4mat Exprs".format(tag_name))
    
    # check all volume fractions
    v4m_exprs = {"v4m_bg":'val4mat(<mesh_topo/mat_check>,"background")',
                 "v4m_c_a":'val4mat(<mesh_topo/mat_check>,"circle_a")',
                 "v4m_c_b":'val4mat(<mesh_topo/mat_check>,"circle_b")',
                 "v4m_c_c":'val4mat(<mesh_topo/mat_check>,"circle_c")'}

    v4m_test_vals = {"v4m_bg":     1,
                     "v4m_c_a":   20,
                     "v4m_c_b":  300,
                     "v4m_c_c": 4000}

    for ename,edef in v4m_exprs.items():
        DefineScalarExpression(ename,edef);
        AddPlot("Pseudocolor", ename)
        set_test_view(tag_name)
        DrawPlots()
        Test(tag_name + "_" + ename)
        # value check
        Query("minmax")
        # We expect:
        res = GetQueryOutputObject()
        print(res)
        #int(res["max"]) == v4m_test_vals[ename]
        #int(res["min"]) == 0
        TestValueEQ("testeq_" + ename + "_max",int(res["max"]),v4m_test_vals[ename])
        TestValueEQ("testeq_" + ename + "_min",int(res["min"]),0)
        DeleteAllPlots()
        ResetView()
    

    CloseDatabase(venn_db_file)

def test_rz_example(tag_name, rz_db_file):
    OpenDatabase(rz_db_file)
    AddPlot("Pseudocolor", "mesh_topo/cyl")
    AddPlot("Mesh", "mesh_topo")
    DrawPlots()
    ResetView()
    Test(tag_name + "_plot_2D")
    DeleteAllPlots()
    ResetView()
    # now revolve
    AddPlot("Pseudocolor", "mesh_topo/cyl")
    AddOperator("Revolve")
    DrawPlots()
    ResetView()
    Test(tag_name + "_plot_revolved_to_3D")
    DeleteAllPlots()
    ResetView()
    CloseDatabase(rz_db_file)


def test_paren_vars():
    TestSection("Variables With Parens")

    #
    # Testing bugfix for issue #4882.
    #
    OpenDatabase(uniform_root)
    AddPlot("Pseudocolor", "mesh_topo/rho.Y_lp_CH2O_rp_")
    set_test_view("3d")
    DrawPlots()
    Test("paren_vars_00")

    DeleteAllPlots()
    ResetView()
    CloseDatabase(uniform_root)


TestSection("2D Example JSON Mesh Files")
OpenDatabase(braid_2d_json_root)
for mesh_name in braid_2d_meshes:
    test(mesh_name,"blueprint_2d_json")
CloseDatabase(braid_2d_json_root)

TestSection("2D Example HDF5 Mesh Files")
OpenDatabase(braid_2d_hdf5_root)
for mesh_name in braid_2d_meshes:
    test(mesh_name,"blueprint_2d_hdf5")
CloseDatabase(braid_2d_hdf5_root)

TestSection("2D Example Sidre HDF5 Mesh Files")
OpenDatabase(braid_2d_sidre_root)
for mesh_name in braid_2d_meshes:
    test(mesh_name,"blueprint_2d_sidre_hdf5")
CloseDatabase(braid_2d_sidre_root)

TestSection("3D Example JSON Mesh Files")
OpenDatabase(braid_3d_json_root)
for mesh_name in braid_3d_meshes:
    test(mesh_name,"blueprint_3d_json")
CloseDatabase(braid_3d_json_root)

TestSection("3D Example HDF5 Mesh Files")
OpenDatabase(braid_3d_hdf5_root)
for mesh_name in braid_3d_meshes:
    test(mesh_name,"blueprint_3d_hdf5")
CloseDatabase(braid_3d_hdf5_root)

TestSection("3D Example Sidre HDF5 Mesh Files")
OpenDatabase(braid_3d_sidre_root)
for mesh_name in braid_3d_meshes:
    test(mesh_name,"blueprint_3d_sidre_hdf5")
CloseDatabase(braid_3d_sidre_root)

TestSection("MFEM Blueprint Example Data Tests")
for example in mfem_ex9_examples:
    for protocol in mfem_ex9_protocols:
        test_mfem("blueprint_mfem_" + example.dim, example.name, protocol)

TestSection("MFEM LOR Mesh Blueprint Tests")
for example in mfem_ex9_examples:
    for protocol in mfem_ex9_protocols:
        test_mfem_lor_mesh("blueprint_mfem_" + example.dim, example.name, protocol)
for example in devilray_mfem_examples:
    test_mfem_lor_mesh("blueprint_mfem_" + example.dim, example.name, "", devilray = True, number = example.number)

TestSection("MFEM LOR Field Blueprint Tests")
for example in devilray_mfem_examples:
    test_mfem_lor_field("blueprint_mfem_" + example.dim, example.name, example.number, example.pseudo_fields, example.vector_fields)

TestSection("Blueprint Expressions")
OpenDatabase(braid_2d_json_root)
AddPlot("Pseudocolor", "uniform_mesh/scalar_expr")
AddPlot("Vector", "uniform_mesh/vector_expr")
DrawPlots()
set_test_view("2d")
Test("blueprint_expressions")
DeleteAllPlots()
ResetView()
CloseDatabase(braid_2d_json_root)

test_paren_vars()

test_venn("venn_small_full", venn_full_root)
test_venn("venn_small_sparse_by_element", venn_s_by_e_root)
test_venn("venn_small_sparse_by_material", venn_s_by_m_root)

test_venn("venn_small_full_yaml", venn_full_yaml_root)
test_venn("venn_small_sparse_by_element_yaml", venn_s_by_e_yaml_root)
test_venn("venn_small_sparse_by_material_yaml", venn_s_by_m_yaml_root)

TestSection("Venn With Modded Material Numbers, 0.8.7")
OpenDatabase(venn_modded_matnos_root)
AddPlot("FilledBoundary", "mesh_topo_matset")
DrawPlots()
ResetView()
Test("Venn_with_modded_matnos")
DeleteAllPlots()
ResetView()

TestSection("2D Example HDF5 Mesh Files, 0.8.2")
OpenDatabase(braid_2d_0_8_2_hdf5_root)
for mesh_name in braid_2d_meshes_0_8_2:
    test(mesh_name,"blueprint_2d_hdf5_0_8_2")
CloseDatabase(braid_2d_0_8_2_hdf5_root)

TestSection("2D Example YAML Mesh Files, 0.8.2")
OpenDatabase(braid_2d_0_8_2_yaml_root)
for mesh_name in braid_2d_meshes_0_8_2:
    test(mesh_name,"blueprint_2d_yaml_0_8_2")
CloseDatabase(braid_2d_0_8_2_yaml_root)

TestSection("3D Example HDF5 Mesh Files, 0.8.2")
OpenDatabase(braid_3d_0_8_2_hdf5_root)
for mesh_name in braid_3d_meshes_0_8_2:
    test(mesh_name,"blueprint_3d_hdf5_0_8_2")
CloseDatabase(braid_3d_0_8_2_hdf5_root)

TestSection("3D Example YAML Mesh Files, 0.8.2")
OpenDatabase(braid_3d_0_8_2_yaml_root)
for mesh_name in braid_3d_meshes_0_8_2:
    test(mesh_name,"blueprint_3d_yaml_0_8_2")
CloseDatabase(braid_3d_0_8_2_yaml_root)

# test polygonal mesh
TestSection("Polygonal 2D Example HDF5 Mesh Files, 0.8.2")
OpenDatabase(poly_2d_hdf5_root)
test_poly("blueprint_poly_2d_hdf5_0_8_2")
CloseDatabase(poly_2d_hdf5_root)

TestSection("Polygonal 2D Example YAML Mesh Files, 0.8.2")
OpenDatabase(poly_2d_yaml_root)
test_poly("blueprint_poly_2d_yaml_0_8_2")
CloseDatabase(poly_2d_yaml_root)

# test 3d polygonal mesh
TestSection("Polygonal 3D Example HDF5 Mesh Files, 0.8.2")
OpenDatabase(poly_3d_hdf5_root)
test_poly("blueprint_poly_3d_hdf5_0_8_2")
CloseDatabase(poly_3d_hdf5_root)

TestSection("Polygonal 3D Example YAML Mesh Files, 0.8.2")
OpenDatabase(poly_3d_yaml_root)
test_poly("blueprint_poly_3d_yaml_0_8_2")
CloseDatabase(poly_3d_yaml_root)


TestSection("Blueprint with Partition Map Style Index, 0.8.4")
OpenDatabase(bp_part_map_root)
AddPlot("Subset","domains")
DrawPlots()
Test("bp_part_map_index_example")
DeleteAllPlots()
CloseDatabase(bp_part_map_root)
# single file test case
OpenDatabase(bp_part_map_single_file_root)
AddPlot("Pseudocolor","mesh_mesh/braid")
DrawPlots()
Test("bp_part_map_index_single_file_example")
DeleteAllPlots()
CloseDatabase(bp_part_map_single_file_root)
# sparse topos case
# single file test case
OpenDatabase(bp_sparse_topos_root)
AddPlot("Pseudocolor","mesh_topo/topo_field")
AddPlot("Pseudocolor","mesh_pts_topo/pts_field")
# change plot adds so we can see the points
pc_atts = PseudocolorAttributes()
pc_atts.pointSize = 1
pc_atts.pointType = pc_atts.Sphere
SetPlotOptions(pc_atts)
DrawPlots()
Test("bp_sparse_topos_example")
DeleteAllPlots()
CloseDatabase(bp_sparse_topos_root)

# other spiral test cases, which have both index styles
for i, root_file in enumerate(bp_spiral_part_map_examples):
    OpenDatabase(root_file)
    AddPlot("Subset","domains")
    DrawPlots()
    # spiral cases are labeled -1 to 8
    case_name = "bp_part_map_spiral_case_{0}".format(i-1)
    Test(case_name)
    DeleteAllPlots()
    CloseDatabase(root_file)


TestSection("Blueprint Strided Structured, 0.8.4")
# 2d
OpenDatabase(bp_struct_strided_2d_root)
# ele vals
AddPlot("Pseudocolor", "mesh_mesh/ele_vals")
AddPlot("Pseudocolor", "mesh_orig_pts/orig_ele_vals")
pc_atts = PseudocolorAttributes()
pc_atts.pointSize = 0.5
pc_atts.pointType = pc_atts.Sphere
SetPlotOptions(pc_atts)
DrawPlots()
Test("bp_strided_struct_2d_ele_vals")
DeleteAllPlots()
# vert vals
AddPlot("Pseudocolor", "mesh_mesh/vert_vals")
AddPlot("Pseudocolor", "mesh_orig_pts/orig_vert_vals")
pc_atts = PseudocolorAttributes()
pc_atts.pointSize = 0.5
pc_atts.pointType = pc_atts.Sphere
SetPlotOptions(pc_atts)
DrawPlots()
Test("bp_strided_struct_2d_vert_vals")
DeleteAllPlots()
CloseDatabase(bp_struct_strided_2d_root)

# 2d
OpenDatabase(bp_struct_strided_3d_root)
# ele vals
AddPlot("Pseudocolor", "mesh_mesh/ele_vals")
AddPlot("Pseudocolor", "mesh_orig_pts/orig_ele_vals")
pc_atts = PseudocolorAttributes()
pc_atts.pointSize = 0.5
pc_atts.pointType = pc_atts.Sphere
SetPlotOptions(pc_atts)
DrawPlots()
Test("bp_strided_struct_3d_ele_vals")
DeleteAllPlots()
# vert vals
AddPlot("Pseudocolor", "mesh_mesh/vert_vals")
AddPlot("Pseudocolor", "mesh_orig_pts/orig_vert_vals")
pc_atts = PseudocolorAttributes()
pc_atts.pointSize = 0.5
pc_atts.pointType = pc_atts.Sphere
SetPlotOptions(pc_atts)
DrawPlots()
Test("bp_strided_struct_3d_vert_vals")
DeleteAllPlots()
CloseDatabase(bp_struct_strided_3d_root)

TestSection("Blueprint RZ Examples, 0.8.6")
for db in bp_rz_examples:
    tag_name = os.path.basename(os.path.split(db)[1])
    test_rz_example(tag_name,db)

TestSection("Blueprint 1D Curve Examples, 0.8.6")
for db in bp_1d_curve_examples:
    OpenDatabase(db)
    AddPlot("Curve", "mesh_topo/field_v")
    DrawPlots()
    Test("blueprint_1d_curve_vertex_assoc")
    DeleteAllPlots()
    AddPlot("Curve", "mesh_topo/field_e")
    DrawPlots()
    Test("blueprint_1d_curve_element_assoc")
    DeleteAllPlots()

TestSection("Blueprint Polytopal Mesh Missing Offsets, 0.8.7")
OpenDatabase(polytopal_mesh_no_offsets_root)
AddPlot("Mesh", "mesh_test")
AddPlot("Pseudocolor", "mesh_test/field")
DrawPlots()
ResetView()
Test("Polytopal_mesh_missing_offsets")
DeleteAllPlots()
ResetView()

TestSection("Blueprint Unstructured Points not using the entire coordset, 0.8.7")
OpenDatabase(unstructured_points)
AddPlot("Pseudocolor", "mesh_mesh/braid", 1, 1)
SetActivePlots(0)
PseudocolorAtts = PseudocolorAttributes()
PseudocolorAtts.pointSizePixels = 20
SetPlotOptions(PseudocolorAtts)
AddPlot("Label", "mesh_mesh/braid", 1, 1)
DrawPlots()
View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (-0.64536, -0.104723, 0.756666)
View3DAtts.focus = (10, 0, 0)
View3DAtts.viewUp = (-0.0863273, 0.994211, 0.0639709)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 14.1421
View3DAtts.nearPlane = -28.2843
View3DAtts.farPlane = 28.2843
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (10, 0, 0)
SetView3D(View3DAtts)
DrawPlots()
Test("Unstructured_points_not_using_entire_coordset")
DeleteAllPlots()
ResetView()

TestSection("Blueprint Uniform Coordset + Unstructured Topo, 0.9.1")
OpenDatabase(uniform_unstructured)
AddPlot("Mesh", "mesh_topo", 1, 1)
DrawPlots()
AddPlot("Subset", "domains", 1, 1)
DrawPlots()
View3DAtts = View3DAttributes()
View3DAtts.viewNormal = (-0.520784, 0.423114, 0.741457)
View3DAtts.focus = (5, 5, 5)
View3DAtts.viewUp = (0.238073, 0.906059, -0.349827)
View3DAtts.viewAngle = 30
View3DAtts.parallelScale = 8.66025
View3DAtts.nearPlane = -17.3205
View3DAtts.farPlane = 17.3205
View3DAtts.imagePan = (0, 0)
View3DAtts.imageZoom = 0.826446
View3DAtts.perspective = 1
View3DAtts.eyeAngle = 2
View3DAtts.centerOfRotationSet = 0
View3DAtts.centerOfRotation = (5, 5, 5)
View3DAtts.axis3DScaleFlag = 0
View3DAtts.axis3DScales = (1, 1, 1)
View3DAtts.shear = (0, 0, 1)
View3DAtts.windowValid = 1
SetView3D(View3DAtts)
Test("Uniform_coordset_and_unstructured_topo")
DeleteAllPlots()
ResetView()

TestSection("Blueprint Mixed Topos, 0.9.2")
OpenDatabase(mixed_topo_2d)
AddPlot("Mesh", "mesh_topo")
AddPlot("Pseudocolor", "mesh_topo/ele_id")
AddPlot("Label", "mesh_pts")
DrawPlots()
Test("Mixed_topo_simple_2d")
ResetView()
DeleteAllPlots()
CloseDatabase(mixed_topo_2d)

OpenDatabase(mixed_topo_3d)
AddPlot("Mesh", "mesh_topo")
AddPlot("Pseudocolor", "mesh_topo/ele_id")
AddPlot("Label", "mesh_pts")
DrawPlots()
Test("Mixed_topo_simple_3d")
ResetView()
DeleteAllPlots()
CloseDatabase(mixed_topo_3d)

OpenDatabase(mixed_braid_2d)
AddPlot("Mesh", "mixed_2d_mesh")
AddPlot("Pseudocolor", "mixed_2d_mesh/braid")
AddPlot("Label", "mixed_2d_mesh")
DrawPlots()
Test("Mixed_braid_2d")
ResetView()
DeleteAllPlots()
CloseDatabase(mixed_braid_2d)

OpenDatabase(mixed_braid_3d)
AddPlot("Mesh", "mixed_3d_mesh")
AddPlot("Pseudocolor", "mixed_3d_mesh/braid")
AddPlot("Label", "mixed_3d_mesh")
DrawPlots()
Test("Mixed_braid_3d")
ResetView()
DeleteAllPlots()
CloseDatabase(mixed_braid_3d)

Exit()
