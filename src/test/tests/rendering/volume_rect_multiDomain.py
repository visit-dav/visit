# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  volume_rect_multiDomain.py
#
#  Tests:      mesh      - 3D rectilinear, multiple domains
#              plots     - volume
#              operators - none
#              selection - no
#
#  Programmer: Kathleen Biagas
#  Date:       May 7, 2024
#
#  Notes:   Moved from volume.py due to interference between a 'Serial'
#           plot following a 'Composite' or 'Parallel' plot causing the
#           'Serial' plot to yield a blank image.
#
#  Modifications:
#
# ----------------------------------------------------------------------------


def multiDomainRect():
    OpenDatabase(silo_data_path("multi_rect3d.silo"))
    AddPlot("Volume", "d")
    DrawPlots()

    View3DAtts = GetView3D()
    View3DAtts.viewNormal = (0.628533, 0.511363, 0.58605)
    View3DAtts.focus = (0.5, 0.5, 0.5)
    View3DAtts.viewUp = (-0.169088, 0.825303, -0.538779)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 0.866025
    View3DAtts.nearPlane = -1.73205
    View3DAtts.farPlane = 1.73205
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 0
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0.5, 0.5, 0.5)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)

    TestSection("MultiDomain, Serial, NoResampling")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Serial
    volAtts.resampleType = volAtts.NoResampling
    volAtts.OSPRayEnabledFlag = 0
    SetPlotOptions(volAtts)
    Test("volume_20")

    TestSection("MultiDomain, Serial, ParallelResistribute")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Serial
    volAtts.resampleType = volAtts.ParallelRedistribute
    volAtts.OSPRayEnabledFlag = 0
    SetPlotOptions(volAtts)
    Test("volume_21")

    TestSection("MultiDomain, Serial, ParallelResistribute, ospray")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Serial
    volAtts.resampleType = volAtts.ParallelRedistribute
    volAtts.OSPRayEnabledFlag = 1
    SetPlotOptions(volAtts)
    Test("volume_22")

    TestSection("MultiDomain, Parallel, ParallelPerRank")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Parallel
    volAtts.resampleType = volAtts.ParallelPerRank
    volAtts.OSPRayEnabledFlag = 0
    SetPlotOptions(volAtts)
    Test("volume_23")

    TestSection("MultiDomain, Parallel, ParallelPerRank, ospray")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Parallel
    volAtts.resampleType = volAtts.ParallelPerRank
    volAtts.OSPRayEnabledFlag = 1
    SetPlotOptions(volAtts)
    Test("volume_24")

    ResetView()
    DeleteAllPlots()

    CloseDatabase(silo_data_path("multi_rect3d.silo"))

multiDomainRect()
Exit()
