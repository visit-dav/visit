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
# ----------------------------------------------------------------------------

from os.path import join as pjoin

bp_test_dir = "blueprint_4cda12d_test_data"

braid_2d_root = data_path(pjoin(bp_test_dir,"braid_2d_examples.blueprint_root_hdf5"))
braid_3d_root = data_path(pjoin(bp_test_dir,"braid_3d_examples.blueprint_root_hdf5"))

braid_2d_sidre_root = data_path(pjoin(bp_test_dir,"braid_2d_sidre_examples.root"))
braid_3d_sidre_root = data_path(pjoin(bp_test_dir,"braid_3d_sidre_examples.root"))


braid_2d_meshes = ["points", "uniform", "rect", "struct", "tris","quads"]
braid_3d_meshes = ["points", "uniform", "rect", "struct", "tets","hexs"]


def full_mesh_name(mesh_name):
    return mesh_name + "_mesh"

def full_var_name(mesh_name,var_name):
    return full_mesh_name(mesh_name) + "/" + var_name

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


def test(mesh_name,tag_name):
    #
    AddPlot("Mesh","%s" % full_mesh_name(mesh_name))
    DrawPlots()
    if "3d" in tag_name:
        set_3d_view()
    Test(tag_name + "_" +  mesh_name + "_mesh")
    DeleteAllPlots()
    #
    AddPlot("Pseudocolor", full_var_name(mesh_name,"braid"))
    DrawPlots()
    if "3d" in tag_name:
        set_3d_view()
    Test(tag_name + "_" +  mesh_name + "_braid")
    #
    DeleteAllPlots()
    AddPlot("Pseudocolor", full_var_name(mesh_name,"radial"))
    if "3d" in tag_name:
        set_3d_view()
    DrawPlots()
    Test(tag_name + "_" +  mesh_name + "_radial")
    DeleteAllPlots()

TestSection("2D Example HDF5 Mesh Files")
OpenDatabase(braid_2d_root)
for mesh_name in braid_2d_meshes:
    test(mesh_name,"blueprint_2d_hdf5")
CloseDatabase(braid_2d_root)

TestSection("2D Example Sidre HDF5 Mesh Files")
OpenDatabase(braid_2d_sidre_root)
for mesh_name in braid_2d_meshes:
    test(mesh_name,"blueprint_2d_sidre_hdf5")
CloseDatabase(braid_2d_sidre_root)


TestSection("3D Example HDF5 Mesh Files")
OpenDatabase(braid_3d_root)
for mesh_name in braid_3d_meshes:
    test(mesh_name,"blueprint_3d_hdf5")
CloseDatabase(braid_3d_root)

TestSection("3D Example Sidre HDF5 Mesh Files")
OpenDatabase(braid_3d_sidre_root)
for mesh_name in braid_3d_meshes:
    test(mesh_name,"blueprint_3d_sidre_hdf5")
CloseDatabase(braid_3d_sidre_root)

Exit()
