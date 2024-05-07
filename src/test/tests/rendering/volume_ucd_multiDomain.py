# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  volume_ucd_multiDomain.py
#
#  Tests:      mesh      - 3D unstructured, multiple domains
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


def ucd_multiDomain():

    # Multiple domain checks plots 30-34
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))
    AddPlot("Volume", "p")
    DrawPlots()

    View3DAtts = GetView3D()
    View3DAtts.viewNormal = (0.409673, 0.375032, 0.831576)
    View3DAtts.focus = (0, 2.5, 10)
    View3DAtts.viewUp = (-0.208428, 0.925952, -0.314914)
    View3DAtts.viewAngle = 30
    View3DAtts.parallelScale = 11.4564
    View3DAtts.nearPlane = -22.9129
    View3DAtts.farPlane = 22.9129
    View3DAtts.imagePan = (0, 0)
    View3DAtts.imageZoom = 1
    View3DAtts.perspective = 0
    View3DAtts.eyeAngle = 2
    View3DAtts.centerOfRotationSet = 0
    View3DAtts.centerOfRotation = (0, 2.5, 10)
    View3DAtts.axis3DScaleFlag = 0
    View3DAtts.axis3DScales = (1, 1, 1)
    View3DAtts.shear = (0, 0, 1)
    View3DAtts.windowValid = 1
    SetView3D(View3DAtts)


  
    TestSection("UCD MultiDomain, Serial, NoResampling")
    # This test is designed to fail, it produces no image that
    # can be compared via 'Test'.
    # Instead, grab the warning message.
    # Should this even be tested?
    try:
        volAtts = VolumeAttributes()
        volAtts.rendererType = volAtts.Serial
        volAtts.resampleType = volAtts.NoResampling
        volAtts.OSPRayEnabledFlag = 0
        SetPlotOptions(volAtts)
        m = GetLastMessage()[0]
        mtype = GetLastMessage()[1]
        # remove the first part that mentions host, since that can change
        w = m.split(":")[1]
        TestText("volume_30", mtype+w)
    except:
        TestText("volume_30", GetLastError())

    TestSection("UCD MultiDomain, Serial, ParallelRedistribute")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Serial
    volAtts.resampleType = volAtts.ParallelRedistribute
    volAtts.OSPRayEnabledFlag = 0
    SetPlotOptions(volAtts)
    Test("volume_31")

    TestSection("UCD MultiDomain, Serial, ParallelRedistribute, ospray")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Serial
    volAtts.resampleType = volAtts.ParallelRedistribute
    volAtts.OSPRayEnabledFlag = 1
    SetPlotOptions(volAtts)
    Test("volume_32")

    TestSection("UCD MultiDomain, Serial, ParallelPerRank")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Parallel
    volAtts.resampleType = volAtts.ParallelPerRank
    volAtts.OSPRayEnabledFlag = 0
    SetPlotOptions(volAtts)
    Test("volume_33")

    TestSection("UCD MultiDomain, Serial, ParallelPerRank, ospray")
    volAtts = VolumeAttributes()
    volAtts.rendererType = volAtts.Parallel
    volAtts.resampleType = volAtts.ParallelPerRank
    volAtts.OSPRayEnabledFlag = 1
    SetPlotOptions(volAtts)
    Test("volume_34")

ucd_multiDomain()
Exit()
