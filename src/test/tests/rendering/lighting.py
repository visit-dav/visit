# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  lighting.py
#
#  Tests:      mesh      - 3D rectilinear, one domain
#              plots     - pseudocolor
#              operators - slicing, elevation
#              selection - no
#
#  Defect ID:  8017
#
#  Programmer: Hank Childs
#  Date:       May 6, 2007
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Alister Maguire, Wed Mar 18 16:45:02 PDT 2020
#    Moved previous two tests into a function called TestBasicLighting and
#    added TestNormals.
#
# ----------------------------------------------------------------------------


def TestBasicLighting():
    OpenDatabase(silo_data_path("rect3d.silo"))

    AddPlot("Pseudocolor", "d")
    AddOperator("Slice")
    AddOperator("Elevate")
    DrawPlots()

    View3DAtts = View3DAttributes()
    View3DAtts.viewNormal = (-0.58136, 0.782415, -0.223267)
    View3DAtts.focus = (0.5, 0.5, 0.66382)
    View3DAtts.viewUp = (0.259676, -0.0816327, -0.962239)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 0.728621
    View3DAtts.nearPlane = -1.45724
    View3DAtts.farPlane = 1.45724
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 1
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0.5, 0.5, 0.66382)
    SetView3D(View3DAtts)

    light0 = GetLight(0)
    light0.enabledFlag = 1
    light0.type = light0.Camera  # Ambient, Object, Camera
    light0.direction = (0, 0, -1)
    light0.color = (255, 255, 255, 255)
    light0.brightness = 1
    SetLight(0, light0)
    light1 = GetLight(1)
    light1.enabledFlag = 1
    light1.type = light1.Ambient  # Ambient, Object, Camera
    light1.direction = (0, 0, -1)
    light1.color = (255, 255, 255, 255)
    light1.brightness = 0.47
    SetLight(1, light1)

    # In SR mode, bug '8017 was that the ambient would get turned off on
    # the second save.
    Test("lighting_01")
    Test("lighting_02")

    DeleteAllPlots()
    CloseDatabase(silo_data_path("rect3d.silo"))


def TestNormals():

    v = GetView3D()
    v.viewNormal = (-0.03622833898009251, 0.7402344653499199, 0.6713720606063838)
    SetView3D(v)

    light0 = GetLight(0)
    light0.enabledFlag = 1
    light0.type = light0.Camera  # Ambient, Object, Camera
    light0.direction = (0, 0, -1)
    light0.brightness = .65
    SetLight(0, light0)
    light1 = GetLight(1)
    light1.enabledFlag = 1
    light1.type = light1.Ambient  # Ambient, Object, Camera
    light1.direction = (0, 0, -1)
    light0.brightness = .65
    SetLight(1, light1)

    #
    # In the past, our normals filter wasn't able to handle triangle strips.
    # This test ensures that we now can.
    #
    OpenDatabase(data_path("vtk_test_data/polyWithStrips.vtk"))
    AddPlot("Pseudocolor", "fooData")
    DrawPlots()
    Test("normals_00")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/polyWithStrips.vtk"))


def main():
    TestBasicLighting()
    TestNormals()
    Exit()

main()

