# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  offscreensave.py
#
#  Tests:      save window options
#              mesh      - 3D unstructured, multiple domains
#              plots     - pc, mesh 
#              operators - slice 
#
#  Programmer: Mark C. Miller 
#  Date:       29Mar04 
#
#  Modifications:
#
#    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
#    Changed scalable rendering controls to use activation mode
#
#    Mark C. Miller, Thu Jan  6 15:09:06 PST 2005
#    Added a test that involves annotation objects
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Eric Brugger, Mon Aug  2 09:42:50 PDT 2021
#    Added test of 16384 x 16384 image save.
#
#    Kathleen Biagas, Fri Aug 27, 2021
#    Put tests into functions. Skip the large image test on Windows as it
#    doesn't yield the correct size image. Ensure the large image is saved
#    to run_dir so the test will execute on Windows properly once it can
#    be enabled.
#
#    Eric Brugger, Mon Nov 18 11:44:28 PST 2024
#    Updated the large image test and added several more.
#
# ----------------------------------------------------------------------------

import hashlib

def GeneralTests():
    TurnOnAllAnnotations()
    OpenDatabase(silo_data_path("multi_ucd3d.silo"))

    AddPlot("Mesh", "mesh1")
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    view=GetView3D()
    view.viewNormal=(-0.528567, 0.374238, 0.761946)
    view.viewUp=(0.200044, 0.927212, -0.316639)
    SetView3D(view)
    Test("offscreen_01")

    # build some savewindow attributes that DO NOT
    # use screen capture
    swa=SaveWindowAttributes()
    swa.screenCapture=0
    swa.width=400
    swa.height=400

    # simple test to make sure we get all the annoations
    # when we save window using off-screen save
    Test("offscreen_02",swa)

    # now, test saving at a different size
    swa.width=600
    swa.height=600
    Test("offscreen_03",swa)

    # Ok, now diddle with annoations a bit and do some
    # more saves
    swa.width=300
    swa.height=300
    a = AnnotationAttributes()
    a.userInfoFlag = 0
    a.databaseInfoFlag = 0
    SetAnnotationAttributes(a)
    Test("offscreen_04",swa)
    a.legendInfoFlag = 0
    SetAnnotationAttributes(a)
    Test("offscreen_05",swa)

    # Ok, now put the window into SR mode
    ra = GetRenderingAttributes()
    ra.scalableActivationMode = ra.Always
    SetRenderingAttributes(ra)
    Test("offscreen_06",swa)

    # diddle some more with annoations
    a.databaseInfoFlag = 1
    a.legendInfoFlag = 1
    SetAnnotationAttributes(a)
    Test("offscreen_07",swa)

    # Now, use screen capture mode to save the window
    Test("offscreen_08")

    DeleteAllPlots()

    OpenDatabase(silo_data_path("curv2d.silo"))


    # Test off screen save of a 2D window
    AddPlot("Mesh", "curvmesh2d")
    AddPlot("Pseudocolor", "d")
    DrawPlots()
    swa.width=400
    swa.height=400
    Test("offscreen_09",swa)

    # Test off screen save of a full frame 2D window
    view2=GetView2D()
    view2.fullFrameActivationMode=view2.On
    SetView2D(view2)
    swa.width=500
    swa.height=500
    Test("offscreen_10",swa)

    # Now test some annotation objects

    DeleteAllPlots()

    OpenDatabase(silo_data_path("multi_ucd3d.silo"))

    AddPlot("Mesh", "mesh1")
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    view=GetView3D()
    view.viewNormal=(-0.528567, 0.374238, 0.761946)
    view.viewUp=(0.200044, 0.927212, -0.316639)
    SetView3D(view)

    # Create a time slider
    slider = CreateAnnotationObject("TimeSlider")
    slider.position = (0.5, 0.5)
    swa.width=300
    swa.height=300
    Test("offscreen_11",swa)

    slider.Delete()
    DeleteAllPlots()

def TestLargeImage(width, height, md5sum):
    # Test saving a large image
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    view2=GetView2D()
    view2.fullFrameActivationMode=view2.Off
    SetView2D(view2)
    
    swa=SaveWindowAttributes()
    # ensure the image is written to the run_dir by providing full path
    imgOutName=pjoin(TestEnv.params["run_dir"], "image_%dx%d.png" % (width,height))
    swa.width = width
    swa.height = height
    swa.resConstraint = swa.NoConstraint
    swa.fileName = imgOutName
    swa.family = 0
    SetSaveWindowAttributes(swa)
    SaveWindow()

    DeleteAllPlots()

    # Comparing md5 sum instead of image, since the image is large.
    md5_hash = hashlib.md5()
    with open(imgOutName, "rb") as f:
        # Read and update hash in chunks of 4K
        for byte_block in iter(lambda: f.read(4096),b""):
            md5_hash.update(byte_block)
    TestValueEQ("md5 hash for %dx%d image" % (width,height), md5_hash.hexdigest(), md5sum)

GeneralTests()

if not sys.platform.startswith("win"):
    # Cannot perform this test on Windows as image size is limited
    OpenDatabase(silo_data_path("curv2d.silo"))

    TestLargeImage(16384, 16384, "19d173a8af27d8b552a58bf7b99fa771")
    TestLargeImage(23168, 23168, "e76139eb88920334c6f931b5fec9c09d")
    TestLargeImage(32768, 16380, "5a6315eae4bf7912a0ab6bd15f62d624")

    # The images sizes for the last 2 tests are too large so they
    # will get scaled to be within the size allowed. The 32768x32768
    # image will be 23168x23168 and the 32768x24000 will be 27071x19827.
    TestLargeImage(32768, 32768, "e76139eb88920334c6f931b5fec9c09d")
    TestLargeImage(32768, 24000, "8c9c5ba01ab8132a42b7d0fa457cabba")

    CloseDatabase(silo_data_path("curv2d.silo"))

Exit()
