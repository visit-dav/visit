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
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Blueprint")

from os.path import join as pjoin

bp_test_dir = "blueprint_v0.3.1_test_data"
bp_mfem_test_dir = "blueprint_v0.3.1_mfem_test_data"

braid_2d_hdf5_root = data_path(pjoin(bp_test_dir,"braid_2d_examples.blueprint_root_hdf5"))
braid_3d_hdf5_root = data_path(pjoin(bp_test_dir,"braid_3d_examples.blueprint_root_hdf5"))

braid_2d_json_root = data_path(pjoin(bp_test_dir,"braid_2d_examples_json.root"))
braid_3d_json_root = data_path(pjoin(bp_test_dir,"braid_3d_examples_json.root"))

braid_2d_sidre_root = data_path(pjoin(bp_test_dir,"braid_2d_sidre_examples.root"))
braid_3d_sidre_root = data_path(pjoin(bp_test_dir,"braid_3d_sidre_examples.root"))

uniform_root = data_path(pjoin(bp_test_dir,"uniform.cycle_001038.root"))


braid_2d_meshes = ["points", "uniform", "rect", "struct", "tris","quads"]
braid_3d_meshes = ["points", "uniform", "rect", "struct", "tets","hexs"]


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

Exit()
