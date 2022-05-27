# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  adios2.py 
#
#  Tests:      mesh      - 3D single domain
#              plots     - Pseudocolor
#              operators - Clip
#
#  Programmer: Justin Privitera
#  Date:       Mon May  9 18:05:05 PDT 2022
#
#  Modifications:
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("ADIOS2")

from os.path import join as pjoin

adios2_test_dir = "adios2_v2.7.1_test_data"
cube_data = data_path(pjoin(adios2_test_dir,"interesting_cube00000000.bp"))

def set_3d_view():
    v = View3DAttributes()
    v.viewNormal = (0.844823, 0.290329, 0.449425)
    v.focus = (35, 35, 35)
    v.viewUp = (-0.239502, 0.956325, -0.167574)
    v.viewAngle = 30
    v.parallelScale = 60.6218
    v.nearPlane = -121.244
    v.farPlane = 121.244
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (35, 35, 35)
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

def test(mesh_name, tag_name):
    AddPlot("Mesh", mesh_name, 1, 1)
    AddPlot("Pseudocolor", "/data/0/meshes/admbase_lapse_rl00/admbase_alp", 1, 1)
    AddOperator("Clip", 1)
    SetActivePlots(1)
    SetActivePlots(1)
    ClipAtts = ClipAttributes()
    ClipAtts.quality = ClipAtts.Fast  # Fast, Accurate
    ClipAtts.funcType = ClipAtts.Plane  # Plane, Sphere
    ClipAtts.plane1Status = 1
    ClipAtts.plane2Status = 0
    ClipAtts.plane3Status = 0
    ClipAtts.plane1Origin = (35, 35, 35)
    ClipAtts.plane2Origin = (0, 0, 0)
    ClipAtts.plane3Origin = (0, 0, 0)
    ClipAtts.plane1Normal = (1, 0, 0)
    ClipAtts.plane2Normal = (0, 1, 0)
    ClipAtts.plane3Normal = (0, 0, 1)
    ClipAtts.planeInverse = 0
    ClipAtts.planeToolControlledClipPlane = ClipAtts.Plane1  # NONE, Plane1, Plane2, Plane3
    ClipAtts.center = (0, 0, 0)
    ClipAtts.radius = 1
    ClipAtts.sphereInverse = 0
    ClipAtts.crinkleClip = 0
    SetOperatorOptions(ClipAtts, 0, 1)
    DrawPlots()
    set_test_view(tag_name)
    Test(tag_name + "_" +  mesh_name + "_mesh")
    DeleteAllPlots()

# requires adios2 to be built with c-blosc support
TestSection("Adios2 Blosc Test")
OpenDatabase(cube_data, 0, "ADIOS2_1.0")
mesh_name = "mesh71x71x71"
tag_name = "adios2_3d_bp"
test(mesh_name, tag_name)
CloseDatabase(cube_data)

Exit()
