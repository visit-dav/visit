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
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

def Test1():
    OpenDatabase(silo_data_path("noise.silo"))

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


Test1()
Exit()
