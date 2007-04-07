# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  threeslice.py
#
#  Programmer: Kathleen Bonnell 
#  Date:       July 6, 2006
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def InitAnnotation():
    # Turn off all annotation
    a = AnnotationAttributes()
    a.axesFlag2D = 0
    a.axesFlag = 0
    a.triadFlag = 0
    a.bboxFlag = 0
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    a.legendInfoFlag = 0
    a.backgroundMode = 0
    a.foregroundColor = (0, 0, 0, 255)
    a.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(a)

def Test1():
    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    AddOperator("ThreeSlice")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0.456241, 0.47741, 0.750949)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.0855449, 0.863521, -0.497004)
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0, 0)
    v.imageZoom = 1
    v.perspective = 1
    v.eyeAngle = 2
    SetView3D(v)

    Test("ThreeSlice_01")

    slice = ThreeSliceAttributes()
    slice.x = -10
    SetOperatorOptions(slice)
    Test("ThreeSlice_02")

    slice.y = -10
    SetOperatorOptions(slice)
    Test("ThreeSlice_03")


InitAnnotation()
Test1()
Exit()
