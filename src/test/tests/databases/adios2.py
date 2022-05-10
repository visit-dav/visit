# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  adios2.py 
#
#  Tests:      TODO
#
#  Programmer: Justin Privitera
#  Date:       Mon May  9 18:05:05 PDT 2022
#
#  Modifications:
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Adios2")

from os.path import join as pjoin

adios2_test_dir = "adios2_v2.7.1_test_data"

# TODO change name
brill_data = data_path(pjoin(adios2_test_dir,"brill-lindquist-write.it00000000.bp"))

TestSection("Adios2 tests")
OpenDatabase(brill_data, 0, "ADIOS2_1.0")

# TODO finish this

# remove everything else in this file



test(mesh_name,"blueprint_2d_json")
CloseDatabase(braid_2d_json_root)




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
