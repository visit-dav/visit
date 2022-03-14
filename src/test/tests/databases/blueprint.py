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
#    Added tests to test new implicit points topologies
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Blueprint")

from os.path import join as pjoin

bp_test_dir = "blueprint_v0.3.1_test_data"
bp_venn_test_dir = "blueprint_v0.7.0_venn_test_data"
bp_mfem_test_dir = "blueprint_v0.3.1_mfem_test_data"
bp_0_8_2_test_dir = "blueprint_v0.8.2_braid_examples_test_data"

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

uniform_root = data_path(pjoin(bp_test_dir,"uniform.cycle_001038.root"))

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



braid_2d_meshes = ["points", "uniform", "rect", "struct", "tris","quads"]
braid_3d_meshes = ["points", "uniform", "rect", "struct", "tets","hexs"]

braid_2d_meshes_0_8_2 = ["points", "uniform", "rect", "struct", "tris","quads", "points_implicit"]
braid_3d_meshes_0_8_2 = ["points", "uniform", "rect", "struct", "tets","hexs", "points_implicit"]

mfem_ex9_examples   = ["periodic_cube","star_q3","periodic_hexagon"]
mfem_ex9_protocols = ["json","conduit_bin","conduit_json","hdf5"]



def full_mesh_name(mesh_name):
    return mesh_name + "_mesh"

def full_var_name(mesh_name,var_name):
    return full_mesh_name(mesh_name) + "/" + var_name

def mfem_test_file(name, protocol):
    return data_path(pjoin(bp_mfem_test_dir,"bp_mfem_ex9_%s_%s_000000.root" % ( name, protocol)))

def set_3d_view():
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


def set_test_view(tag_name):
    if "3d" in tag_name:
        set_3d_view()
    else:
        ResetView()

def test(mesh_name,tag_name):
    #
    AddPlot("Mesh","%s" % full_mesh_name(mesh_name))
    DrawPlots()
    set_test_view(tag_name)
    Test(tag_name + "_" +  mesh_name + "_mesh")
    DeleteAllPlots()
    #
    AddPlot("Pseudocolor", full_var_name(mesh_name,"braid"))
    DrawPlots()
    set_test_view(tag_name)
    Test(tag_name + "_" +  mesh_name + "_braid")
    #
    DeleteAllPlots()
    AddPlot("Pseudocolor", full_var_name(mesh_name,"radial"))
    set_test_view(tag_name)
    DrawPlots()
    Test(tag_name + "_" +  mesh_name + "_radial")
    DeleteAllPlots()

def test_mfem(tag_name, example_name, protocol):
    dbfile =mfem_test_file(example_name,protocol)
    OpenDatabase(dbfile)
    #
    AddPlot("Pseudocolor", "mesh_main/solution")
    DrawPlots()
    Test(tag_name + "_" +  example_name + "_" + protocol + "_sol")
    #
    DeleteAllPlots()
    AddPlot("Pseudocolor", "mesh_main/element_coloring")
    DrawPlots()
    Test(tag_name + "_" +  example_name + "_" + protocol + "_ele_coloring")
    #
    DeleteAllPlots()
    AddPlot("Pseudocolor", "mesh_main/element_attribute")
    DrawPlots()
    Test(tag_name + "_" +  example_name + "_" + protocol + "_ele_att")
    #
    DeleteAllPlots()

    CloseDatabase(dbfile)

def test_venn(tag_name, venn_db_file):
    TestSection("Blueprint Matset Example Tests: {0} ".format(tag_name))
    OpenDatabase(venn_db_file)
    AddPlot("Pseudocolor", "mesh_topo/mat_check")
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


    TestSection("Blueprint Matset Example Tests: {0} Matvf Exprs".format(tag_name))

    # check all volume fractions
    vf_exprs = {"vf_bg":'matvf(mesh_topo_matset,"background")',
                "vf_c_a":'matvf(mesh_topo_matset,"circle_a")',
                "vf_c_b":'matvf(mesh_topo_matset,"circle_b")',
                "vf_c_c":'matvf(mesh_topo_matset,"circle_c")'}

    for ename,edef in vf_exprs.items():
        DefineScalarExpression(ename,edef);
        AddPlot("Pseudocolor", ename)
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
    

    CloseDatabase(venn_db_file)

def test_paren_vars():
    TestSection("Variables With Parens")

    #
    # Testing bugfix for issue #4882.
    #
    OpenDatabase(uniform_root)
    AddPlot("Pseudocolor", "mesh_topo/rho.Y_lp_CH2O_rp_")
    DrawPlots()
    Test("paren_vars_00")

    DeleteAllPlots()
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
for example_name in mfem_ex9_examples:
    for protocol in mfem_ex9_protocols:
        test_mfem("blueprint_mfem", example_name, protocol)

TestSection("Blueprint Expressions")
OpenDatabase(braid_2d_json_root)
AddPlot("Pseudocolor", "uniform_mesh/scalar_expr")
AddPlot("Vector", "uniform_mesh/vector_expr")
DrawPlots()
Test("blueprint_expressions")
DeleteAllPlots()
CloseDatabase(braid_2d_json_root)

test_paren_vars()

test_venn("venn_small_full", venn_full_root)
test_venn("venn_small_sparse_by_element", venn_s_by_e_root)
test_venn("venn_small_sparse_by_material", venn_s_by_m_root)

test_venn("venn_small_full_yaml", venn_full_yaml_root)
test_venn("venn_small_sparse_by_element_yaml", venn_s_by_e_yaml_root)
test_venn("venn_small_sparse_by_material_yaml", venn_s_by_m_yaml_root)

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

Exit()
