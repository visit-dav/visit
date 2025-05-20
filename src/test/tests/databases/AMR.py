# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  amr.py
#
#  Tests:      EOS:
#                  - ideal gas,
#                  - JWLBTD,
#                  - JWL-JWL mixture - with mesh,
#                  - SESAME,
#                  - Ideal Gas - Tillotson,  and
#                  - general mixture
#              Multiple roots
#
#  Programmer: Sam Briney
#  Date:       Fri May  16 3:38:00 EDT 2025
#
#  Modifications:
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("AMR")

from os.path import join as pjoin

AMR_test_dir = "AMR_test_data"

def set_3d_view():
    v = View3DAttributes()
    v.viewNormal = (-0.47, -0.36, -0.81)
    v.focus = (10, 10, 10)
    v.viewUp = (-0.78, 0.60, 0.18)
    v.viewAngle = 30
    v.parallelScale = 17.32
    v.nearPlane = -120.244
    v.farPlane = 121.244
    v.imagePan = (0, 0)
    v.imageZoom = 0.55
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

def test(mesh_name, tag_name, var_name, mesh_plot = False):
    if mesh_plot:
        AddPlot("Mesh", mesh_name)
    AddPlot("Pseudocolor", var_name)
    if "3d" in tag_name:
        AddOperator("Clip", 1)
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

# requires AMR to be built
def test_amr_ideal_gas():
    TestSection("AMR Ideal Gas Test")
    ideal_gas_data = data_path(pjoin(AMR_test_dir,"EOS/ideal_gas/velodyne_amr_5.amr"))
    OpenDatabase(ideal_gas_data, 0, "AMR")
    mesh_name = "AMR"
    tag_name = "ideal_gas_den_3d"
    var_name = "AMR/density"
    do_mesh_plot = False
    test(mesh_name, tag_name, var_name, do_mesh_plot)
    CloseDatabase(ideal_gas_data)

def test_amr_jwlbtd():
    TestSection("AMR JWLBTD Test")
    data = data_path(pjoin(AMR_test_dir,"EOS/jwlbtd/velodyne_amr_5.amr"))
    OpenDatabase(data, 0, "AMR")
    mesh_name = "AMR"
    tag_name = "jwlbtd_press_3d"
    var_name = "AMR/pressure"
    do_mesh_plot = False
    test(mesh_name, tag_name, var_name, do_mesh_plot)
    CloseDatabase(data)

def test_amr_jwljwl():
    TestSection("AMR JWLJWL Test")
    data = data_path(pjoin(AMR_test_dir,"EOS/jwljwl/velodyne_amr_5.amr"))
    OpenDatabase(data, 0, "AMR")
    mesh_name = "AMR"
    tag_name = "jwljwl_spec_den_3d"
    var_name = "AMR/Species Density/1"
    do_mesh_plot = True
    test(mesh_name, tag_name, var_name, do_mesh_plot)
    CloseDatabase(data)

def test_amr_sesame():
    TestSection("AMR SESAME Test")
    data = data_path(pjoin(AMR_test_dir,"EOS/sesame/velodyne_amr_3.amr"))
    OpenDatabase(data, 0, "AMR")
    mesh_name = "AMR"
    tag_name = "sesame_sndv_3d"
    var_name = "AMR/sound speed"
    do_mesh_plot = False
    test(mesh_name, tag_name, var_name, do_mesh_plot)
    CloseDatabase(data)

def test_amr_igtil():
    TestSection("AMR Ideal Gas TIL Test")
    data = data_path(pjoin(AMR_test_dir,"EOS/ig_til/velodyne_amr_5.amr"))
    OpenDatabase(data, 0, "AMR")
    mesh_name = "AMR"
    tag_name = "IG_TIL_spec_mass_frac_3d"
    var_name = "AMR/Species Mass Fraction/2"
    do_mesh_plot = False
    test(mesh_name, tag_name, var_name, do_mesh_plot)
    CloseDatabase(data)

def test_amr_genmix():
    TestSection("AMR General Mixture Test")
    data = data_path(pjoin(AMR_test_dir,"EOS/genmix/velodyne_amr_1.amr"))
    OpenDatabase(data, 0, "AMR")
    mesh_name = "AMR"
    tag_name = "GenMix_spec_dens_3d"
    var_name = "AMR/Species Density/2"
    do_mesh_plot = False
    test(mesh_name, tag_name, var_name, do_mesh_plot)
    CloseDatabase(data)

def test_amr_multi_root():
    TestSection("AMR Multiple Root Test")
    data = data_path(pjoin(AMR_test_dir,"multi_block.amr"))
    OpenDatabase(data, 0, "AMR")
    mesh_name = "AMR"
    tag_name = "Multiple_root_3d"
    var_name = "AMR/density"
    do_mesh_plot = False
    test(mesh_name, tag_name, var_name, do_mesh_plot)
    CloseDatabase(data)

def main():
    test_amr_ideal_gas()
    test_amr_jwlbtd()
    test_amr_jwljwl()
    test_amr_sesame()
    test_amr_igtil()
    test_amr_genmix()
    test_amr_multi_root()

main()
Exit()
