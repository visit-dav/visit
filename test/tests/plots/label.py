# ----------------------------------------------------------------------------
#  MODES: serial
#  CLASSES: nightly
#
#  Test Case:  label.py 
#
#  Tests:      Tests the label plot
#
#  Programmer: Brad Whitlock
#  Date:       Fri Oct 22 15:50:06 PST 2004
#
#  Modifications:
#    Brad Whitlock, Wed Oct 27 15:08:27 PST 2004
#    I made the baseline images larger.
#
#    Mark C. Miller, Mon Apr 25 21:35:54 PDT 2005
#    I added tests for different data types
#
#    Kathleen Bonnell, Thu Jul 14 07:53:46 PDT 2005
#    Added TestLabeledVTK, to test reading and use of char data from VTK.
#
#    Brad Whitlock, Thu Aug 4 14:25:22 PST 2005
#    Added tests for labelling subsets and I also removed the single cell/node
#    stuff and replaced it with tests for setting individual cell/node
#    colors and text heights.
#
#    Jeremy Meredith, Wed Sep  7 12:06:04 PDT 2005
#    Allowed spaces in variable names.
#
#    Brad Whitlock, Tue Nov 15 09:56:44 PDT 2005
#    Added another test for sliced vectors.
#
#    Brad Whitlock, Tue Apr 25 15:47:21 PST 2006
#    I added tests for making sure that Label plots of structured meshes
#    actually show their logical index nature by default. The tests also
#    show the Label plot with zooms that used to make the labels be incorrectly
#    offset from the other plots.
#
# ----------------------------------------------------------------------------

# Create label attributes with somewhat larger text.
def CreateLabelAttributes():
    L = LabelAttributes()
    L.textHeight1 = 0.03
    L.textHeight2 = 0.03
    return L

def SaveTestImage(name):
    # Save these images somewhat larger than a regular test case image
    # since the images contain a lot of text.
    swa = SaveWindowAttributes()
    swa.width = 500
    swa.height = 500
    swa.screenCapture = 0
    Test(name, swa)

def TestGeneralFeatures():
    TestSection("General label plot features")
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Label", "curvmesh2d")
    DrawPlots()
    SaveTestImage("label_0_00")

    # Change the zoom a few times to see the number of labels adaptively change.
    v0 = View2DAttributes()
    v0.windowCoords = (-11.6154, 11.8294, -3.18886, 8.33075)
    v0.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v0.fullFrameActivationMode = v0.Off 
    SetView2D(v0)
    SaveTestImage("label_0_01")

    v1 = View2DAttributes()
    v1.windowCoords = (-1.0815, 1.29556, 1.98696, 3.15493)
    v1.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v1.fullFrameActivationMode = v1.Off 
    SetView2D(v1)
    SaveTestImage("label_0_02")

    v2 = View2DAttributes()
    v2.windowCoords = (1.78125, 3.00819, 0.209532, 0.812385)
    v2.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v2.fullFrameActivationMode = v2.Off 
    AddPlot("Mesh", "curvmesh2d")
    DrawPlots()
    SetActivePlots(0)
    SetView2D(v2)
    SaveTestImage("label_0_03")

    # Turn on nodes
    L = CreateLabelAttributes()
    L.showNodes = 1
    SetPlotOptions(L)
    SaveTestImage("label_0_04")

    # Turn off cells
    L.showCells = 0
    SetPlotOptions(L)
    SaveTestImage("label_0_05")

    # Try showing nodes and cells and alter their respective sizes
    L.showCells = 1
    L.showNodes = 1
    L.textHeight1 = 0.04
    L.textHeight2 = 0.03  
    SetPlotOptions(L)
    SaveTestImage("label_0_06")

    # Change the cell color.
    L.textColor1 = (0,255,0,255)
    L.specifyTextColor1 = 1
    SetPlotOptions(L)
    SaveTestImage("label_0_07")

    # Change the node color
    L.textColor2 = (100,10,255,255)
    L.specifyTextColor2 = 1
    SetPlotOptions(L)
    SaveTestImage("label_0_08")
    L.textColor1 = (0,255,0,255)
    L.textColor2 = (0,255,0,255)

    # Change the structured indices to regular indices
    L.showCells = 1
    L.labelDisplayFormat = L.Index
    L.textHeight1 = 0.03
    SetPlotOptions(L)
    SaveTestImage("label_0_09")

    # Make the text a little bigger
    L.textHeight1 = 0.06
    L.textHeight2 = 0.06
    L.showCells = 0
    SetPlotOptions(L)
    SaveTestImage("label_0_10")

    # Test different text alignments
    L.textHeight1 = 0.04
    L.textHeight2 = 0.04
    L.horizontalJustification = L.Left
    SetPlotOptions(L)
    SaveTestImage("label_0_11")
    L.horizontalJustification = L.Right
    SetPlotOptions(L)
    SaveTestImage("label_0_12")
    L.horizontalJustification = L.HCenter
    L.verticalJustification = L.Top
    SetPlotOptions(L)
    SaveTestImage("label_0_13")
    L.verticalJustification = L.Bottom
    SetPlotOptions(L)
    SaveTestImage("label_0_14")

    # Test changing the number of labels.
    L.verticalJustification = L.VCenter
    SetPlotOptions(L)
    ResetView()
    SaveTestImage("label_0_15")
    L.numberOfLabels = 40
    SetPlotOptions(L)
    SaveTestImage("label_0_16")

    # Turn off restricting the number of labels.
    L.restrictNumberOfLabels = 0
    SetPlotOptions(L)
    SaveTestImage("label_0_17")
    DeleteAllPlots()

    # test different data types from the file
    OpenDatabase("../data/wave0000.silo")
    AddPlot("Label", "chars")
    DrawPlots()
    v3d=GetView3D()
    v3d.viewNormal=(0, 1, 0)
    v3d.viewUp=(0, 0, -1)
    SetView3D(v3d)
    SaveTestImage("label_0_18")
    DeleteAllPlots()

    AddPlot("Label", "shorts")
    DrawPlots()
    SaveTestImage("label_0_19")
    DeleteAllPlots()

    AddPlot("Label", "ints")
    DrawPlots()
    SaveTestImage("label_0_20")
    ResetView()
    DeleteAllPlots()

def TestCurvilinear2D():
    TestSection("Curvilinear 2D")
    OpenDatabase("../data/curv2d.silo")
    AddPlot("Pseudocolor", "u")
    AddPlot("Mesh", "curvmesh2d")
    m = MeshAttributes()
    m.opaqueMode = m.Auto
    SetPlotOptions(m)
    # Do a point variable
    AddPlot("Label", "u")
    DrawPlots()
    ResetView()
    SaveTestImage("label_1_00")

    # Make the labels a little larger and zoom in on the cells
    v0 = View2DAttributes()
    v0.windowCoords = (-0.632297, 0.483674, 3.38963, 4.57713)
    v0.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v0.fullFrameActivationMode = v0.Off 
    SetView2D(v0)
    L = CreateLabelAttributes()
    L.textHeight1 = 0.03
    L.textHeight2 = 0.03
    SetPlotOptions(L)
    SaveTestImage("label_1_01")

    # Change the a cell centered variable
    ChangeActivePlotsVar("d")
    SaveTestImage("label_1_02")

    # Change to a node centered vector variable
    v0.windowCoords = (-0.662647, 0.179177, 3.52611, 4.4219)
    SetView2D(v0)
    ChangeActivePlotsVar("vel")
    SaveTestImage("label_1_03")

    # Change to a mesh variable
    ChangeActivePlotsVar("curvmesh2d")
    L.showNodes = 1
    SetPlotOptions(L)
    SaveTestImage("label_1_04")

    # Turn off a material
    v0.windowCoords = (-1.73904, -0.385797, 2.20216, 3.64214)
    SetView2D(v0)
    SaveTestImage("label_1_05")
    SetActivePlots((0,1,2))
    TurnMaterialsOff("2")
    SaveTestImage("label_1_06")
    DeleteAllPlots()

def TestRectilinear2D():
    TestSection("Rectilinear 2D")
    OpenDatabase("../data/rect2d.silo")
    AddPlot("Pseudocolor", "ascii")
    AddPlot("Mesh", "quadmesh2d")
    m = MeshAttributes()
    m.opaqueMode = m.Auto
    SetPlotOptions(m)
    AddPlot("Label", "ascii")
    DrawPlots()
    ResetView()
    v0 = View2DAttributes()
    v0.windowCoords = (0.425121, 0.574879, 0.566829, 0.766505)
    v0.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v0.fullFrameActivationMode = v0.Off 
    SetView2D(v0)
    L = CreateLabelAttributes()
    L.textHeight1 = 0.03
    L.textHeight2 = 0.03
    SetPlotOptions(L)
    SaveTestImage("label_2_00")

    # Change to a cell centered float variable.
    ChangeActivePlotsVar("d")
    SaveTestImage("label_2_01")

    # Change to a node centered variable
    ChangeActivePlotsVar("u")
    SaveTestImage("label_2_02")

    # Change to a node centered vector variable
    ChangeActivePlotsVar("vel")
    v1 = View2DAttributes()
    v1.windowCoords = (0.478575, 0.543393, 0.650721, 0.737145)
    SetView2D(v1)
    SaveTestImage("label_2_03")

    # Change to the mesh variable
    SetActivePlots((0,2))
    DeleteActivePlots()
    AddPlot("FilledBoundary", "mat1")
    AddPlot("Label", "quadmesh2d")
    DrawPlots()
    L.showNodes = 1
    L.textHeight1 = 0.03
    L.textHeight2 = 0.03
    SetPlotOptions(L)
    SaveTestImage("label_2_04")

    # Turn off a material
    SetActivePlots((0,1,2))
    TurnMaterialsOff("16")
    SaveTestImage("label_2_05")
    DeleteAllPlots()


def TestUnstructured2D():
    TestSection("Unstructured 2D")
    OpenDatabase("../data/ucd2d.silo")
    AddPlot("Mesh", "ucdmesh2d")
    m = MeshAttributes()
    m.opaqueMode = m.Auto
    SetPlotOptions(m)
    AddPlot("Label", "ucdmesh2d")
    L = CreateLabelAttributes()
    L.showNodes = 1
    L.textHeight1 = 0.04
    L.textHeight2 = 0.04
    L.textColor1 = (255,0,0,255)
    L.textColor2 = (255,0,0,255)
    L.specifyTextColor1 = 1
    L.specifyTextColor2 = 1
    L.restrictNumberOfLabels = 0
    SetPlotOptions(L)
    DrawPlots()
    v0 = View2DAttributes()
    v0.windowCoords = (-0.154956, 4.15496, -0.154956, 4.15496)
    v0.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v0.fullFrameActivationMode = v0.Off 
    SetView2D(v0)
    SaveTestImage("label_3_00")

    # Change to a nodal variable
    ChangeActivePlotsVar("d")
    SaveTestImage("label_3_01")

    # Change to a cell centered variable
    ChangeActivePlotsVar("p")
    SaveTestImage("label_3_02")

    # Change to a mesh variable and remove a material.
    ChangeActivePlotsVar("ucdmesh2d")
    AddPlot("FilledBoundary", "mat1")
    DrawPlots()
    L.specifyTextColor1 = 0
    L.specifyTextColor2 = 0
    SetPlotOptions(L)
    SetActivePlots((0,1,2))
    TurnMaterialsOff("2")
    SaveTestImage("label_3_03")
    DeleteAllPlots()

def TestSlice():
    TestSection("Testing slicing 3D data to 2D")
    OpenDatabase("../data/noise.silo")
    AddPlot("Mesh", "Mesh")
    m = MeshAttributes()
    m.opaqueMode = m.Auto
    SetPlotOptions(m)
    AddPlot("Pseudocolor", "chromeVf")
    AddPlot("Label", "chromeVf")
    SetActivePlots((0,1,2))
    # Add a slice operator to all of the plots
    AddOperator("Slice")
    s = SliceAttributes()
    s.normal = (0,0,1)
    s.upAxis = (0,1,0)
    s.project2d = 0
    SetOperatorOptions(s)
    # Make the labels a little bigger
    L = CreateLabelAttributes()
    L.textHeight1 = 0.05
    L.textHeight2 = 0.05
    L.depthTestMode = L.LABEL_DT_NEVER
    SetPlotOptions(L)
    DrawPlots()

    # Look at the slice from the front
    v0 = View3DAttributes()
    v0.viewNormal = (-0.550782, 0.318825, 0.771355)
    v0.focus = (0, 0, 0)
    v0.viewUp = (0.200365, 0.94765, -0.248624)
    v0.viewAngle = 30
    v0.parallelScale = 17.3205
    v0.nearPlane = -34.641
    v0.farPlane = 34.641
    v0.imagePan = (0, 0)
    v0.imageZoom = 1.82449
    v0.perspective = 0
    v0.eyeAngle = 2
    v0.centerOfRotationSet = 0
    v0.centerOfRotation = (0, 0, 0)
    SetView3D(v0)
    SaveTestImage("label_4_00")

    # Look at the slice from the back. It will only be visible if the
    # quantized normals were thrown out.
    v1 = View3DAttributes()
    v1.viewNormal = (-0.681715, 0.349442, -0.642771)
    v1.focus = (0, 0, 0)
    v1.viewUp = (0.26946, 0.936726, 0.223465)
    v1.viewAngle = 30
    v1.parallelScale = 17.3205
    v1.nearPlane = -34.641
    v1.farPlane = 34.641
    v1.imagePan = (0, 0)
    v1.imageZoom = 1.82449
    v1.perspective = 0
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (0, 0, 0)
    SetView3D(v1)
    SaveTestImage("label_4_01")

    # Project the slice to 2D
    s.project2d = 1
    SetOperatorOptions(s)
    v2 = View2DAttributes()
    v2.windowCoords = (-8.64231, -5.11567, 2.53613, 6.24948)
    SetView2D(v2)
    SaveTestImage("label_4_02")

    # Set the slice normal so it does not line up with an axis.
    s.normal = (1,2,3)
    SetOperatorOptions(s)
    SaveTestImage("label_4_03")
    DeleteAllPlots()

def TestLabeledVTK():
    TestSection("VTK with labels")
    OpenDatabase("../data/labeledBox.vtk")
    AddPlot("Mesh", "mesh")
    AddPlot("Label", "cellLabel")
    AddPlot("Label", "nodeLabels")
    DrawPlots()
    SetActivePlots((1, 2))
    l = LabelAttributes()
    l.textHeight1 = 0.06
    l.textHeight2 = 0.06
    l.depthTestMode = l.LABEL_DT_NEVER
    SetPlotOptions(l)

    v = GetView3D()
    v.viewNormal = (-0.826308, 0.365749, 0.428303) 
    v.focus = (0, 0, 0)
    v.viewUp = (0.262408, 0.92288, -0.28184)
    v.parallelScale = 1.55885
    v.nearPlane = -3.11769
    v.farPlane  =  3.11769
    v.imageZoom = 0.941919
    SetView3D(v)
    
    SaveTestImage("label_5_01")
    DeleteAllPlots()

def TestLabellingSubsets():
    TestSection("Labelling subsets")
    OpenDatabase("../data/bigsil.silo")
    AddPlot("Mesh", "mesh")
    AddPlot("Subset", "blocks")
    AddPlot("Label", "blocks")
    l = LabelAttributes()
    l.restrictNumberOfLabels = 0
    l.textHeight1 = 0.03
    l.depthTestMode = l.LABEL_DT_NEVER
    SetPlotOptions(l)
    SetActivePlots((0,1,2))
    TurnDomainsOff()
    TurnDomainsOn(("domain1","domain10","domain19"))
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (0.465057, -0.448026, -0.763541)
    v.focus = (0.166667, 0.166667, 0.5)
    v.viewUp = (-0.243345, -0.893956, 0.376333)
    v.viewAngle = 30
    v.parallelScale = 0.552771
    v.nearPlane = -1.10554
    v.farPlane = 1.10554
    v.imagePan = (0.0545619, 0.050939)
    v.imageZoom = 1.32578
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0.166667, 0.166667, 0.5)
    SetView3D(v)
    SaveTestImage("label_6_00")

    SetActivePlots((1,2))
    ChangeActivePlotsVar("domains")
    SaveTestImage("label_6_01")

    SetActivePlots(1)
    DeleteActivePlots()
    AddPlot("FilledBoundary", "mat")
    SetActivePlots(1)
    DrawPlots()
    ChangeActivePlotsVar("mat")
    SaveTestImage("label_6_02")

    # Try an AMR dataset
    DeleteAllPlots()
    OpenDatabase("../data/boxlib_test_data/2D/plt0822/Header")
    AddPlot("Mesh", "Mesh")
    AddPlot("Subset", "patches")
    s = SubsetAttributes()
    s.legendFlag = 0
    SetPlotOptions(s)
    AddPlot("Label", "patches")
    l = LabelAttributes()
    l.restrictNumberOfLabels = 0
    l.textHeight1 = 0.027
    SetPlotOptions(l)
    DrawPlots()
    v = View2DAttributes()
    v.windowCoords = (0.0126663, 0.0146908, 0.115281, 0.117298)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    SaveTestImage("label_6_03")

    SetActivePlots((1,2))
    ChangeActivePlotsVar("levels")
    v = View2DAttributes()
    v.windowCoords = (0.0250949, 0.0437515, 0.11359, 0.132218)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    SetActivePlots(2)
    l.textHeight1 = 0.02
    SetPlotOptions(l)
    SaveTestImage("label_6_04")
    DeleteAllPlots()

    # Try a dataset with real material names.
    OpenDatabase("../data/noise.silo")
    AddPlot("Mesh", "Mesh")
    AddPlot("FilledBoundary", "mat1")
    AddPlot("Label", "mat1")
    l = LabelAttributes()
    l.textHeight1 = 0.03
    SetPlotOptions(l)
    SetActivePlots((0,1,2))
    AddOperator("Slice")
    DrawPlots()
    v = View2DAttributes()
    v.windowCoords = (-9.37363, -6.58974, -2.85531, -0.327839)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    SaveTestImage("label_6_05")
    DeleteAllPlots()


def TestLabellingTensors():
    TestSection("Labelling tensors")
    OpenDatabase("../data/noise.silo")
    AddPlot("Mesh", "Mesh")
    AddPlot("Tensor", "grad_tensor")
    t = TensorAttributes()
    t.useStride = 1
    SetPlotOptions(t)
    AddPlot("Label", "grad_tensor")
    SetActivePlots((0,1,2))
    AddOperator("Slice")
    DrawPlots()
    ResetView()
    SaveTestImage("label_7_00")

    v = View2DAttributes()
    v.windowCoords = (-9.13799, -4.37695, -7.71782, -2.97379)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    SaveTestImage("label_7_01")

    v.windowCoords = (-8.50106, -7.83502, -6.43747, -5.77381)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    SaveTestImage("label_7_02")

    # Now see how changing the text height affects binning
    ResetView()
    l = LabelAttributes()
    l.textHeight1 = 0.01
    SetActivePlots(2)
    SetPlotOptions(l)
    SaveTestImage("label_7_03")

    l.textHeight1 = 0.04
    SetPlotOptions(l)
    SaveTestImage("label_7_04")

    SetView2D(v)
    SaveTestImage("label_7_05")

    DeleteAllPlots()

def TestLabelling3D():
    TestSection("Labelling in 3D with Z buffer")
    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "airVf")
    AddPlot("Label", "airVf")
    L = LabelAttributes()
    L.restrictNumberOfLabels = 0
    L.depthTestMode = L.LABEL_DT_NEVER
    SetPlotOptions(L)
    DrawPlots()
    ResetView()
    SaveTestImage("label_8_00")

    # Turn on depth testing
    L.depthTestMode = L.LABEL_DT_AUTO
    SetPlotOptions(L)
    SaveTestImage("label_8_01")

    # Apply an Isosurface operator to make things really convoluted
    SetActivePlots((0,1))
    AddOperator("Isosurface")
    iso = IsosurfaceAttributes()
    iso.contourNLevels = 1
    iso.variable="hardyglobal"
    SetOperatorOptions(iso)
    v = View3DAttributes()
    v.viewNormal = (0.883239, -0.244693, -0.400019)
    v.focus = (0, 0, 0)
    v.viewUp = (0.435303, 0.745036, 0.505404)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0.022398, -0.134568)
    v.imageZoom = 3.35882
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)
    SaveTestImage("label_8_02")

    #
    # Try labelling a mesh that has interior stuff
    #
    DeleteAllPlots()
    OpenDatabase("../data/globe.silo")
    AddPlot("Pseudocolor", "speed")
    AddPlot("Mesh", "mesh1")
    AddPlot("Label", "mesh1")
    L = CreateLabelAttributes()
    L.restrictNumberOfLabels = 0
    L.drawLabelsFacing = L.FrontAndBack
    L.depthTestMode = L.LABEL_DT_AUTO
    SetPlotOptions(L)
    DrawPlots()
    ResetView()
    v.viewNormal = (-0.735192, 0.371514, 0.566984)
    v.focus = (0, 0, 0)
    v.viewUp = (0.2695, 0.927683, -0.258407)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0, 0)
    v.imageZoom = 1.60959
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)
    SaveTestImage("label_8_03")

    v1 = View3DAttributes()
    v1.viewNormal = (0, 0, 1)
    v1.focus = (0, 0, 0)
    v1.viewUp = (0, 1, 0)
    v1.viewAngle = 30
    v1.parallelScale = 17.3205
    v1.nearPlane = -34.641
    v1.farPlane = 34.641
    v1.imagePan = (0, 0)
    v1.imageZoom = 5.06081
    v1.perspective = 1
    v1.eyeAngle = 2
    v1.centerOfRotationSet = 0
    v1.centerOfRotation = (0, 0, 0)
    SetView3D(v1)
    SaveTestImage("label_8_04")

    # Turn on node labels
    L.showNodes = 1
    SetPlotOptions(L)
    SaveTestImage("label_8_05")
    
    # Label a variable that would normally have labels that protrude into
    # the dataset
    ChangeActivePlotsVar("speed")
    ResetView()
    SaveTestImage("label_8_06")

    SetView3D(v)
    SaveTestImage("label_8_07")
    DeleteAllPlots()

    #
    # Try labelling a 3D AMR dataset
    #
    OpenDatabase("../data/samrai_test_data/sil_changes/dumps.visit")
    AddPlot("Pseudocolor", "Primitive Var _number_0")
    AddPlot("Mesh", "amr_mesh")
    AddPlot("Label", "levels")
    L = CreateLabelAttributes()
    L.showNodes = 1
    L.showCells = 0
    L.restrictNumberOfLabels = 0
    L.depthTestMode = L.LABEL_DT_NEVER
    SetPlotOptions(L)
    DrawPlots()
    v2 = View3DAttributes()
    v2.viewNormal = (-0.643521, -0.624962, -0.441931)
    v2.focus = (15, 10, 10)
    v2.viewUp = (0.320552, 0.304253, -0.897038)
    v2.viewAngle = 30
    v2.parallelScale = 20.6155
    v2.nearPlane = -41.2311
    v2.farPlane = 41.2311
    v2.imagePan = (-0.0412674, 0.141248)
    v2.imageZoom = 1.871
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (15, 10, 10)
    SetView3D(v2)
    SaveTestImage("label_8_08")

    # Turn on zbuffering
    L.depthTestMode = L.LABEL_DT_AUTO
    SetPlotOptions(L)
    SaveTestImage("label_8_09")

    # Restrict the number of labels
    L.restrictNumberOfLabels = 1
    L.numberOfLabels = 900
    L.depthTestMode = L.LABEL_DT_NEVER
    SetPlotOptions(L)
    SaveTestImage("label_8_10")

    # Turn on zbuffering
    L.depthTestMode = L.LABEL_DT_AUTO
    SetPlotOptions(L)
    SaveTestImage("label_8_11")
    DeleteAllPlots()

def TestSlicedVectors():
    TestSection("Labelling sliced vectors")
    OpenDatabase("../data/noise.silo")
    AddPlot("Mesh", "Mesh")
    AddPlot("Label", "grad")
    L = CreateLabelAttributes()
    L.depthTestMode = L.LABEL_DT_NEVER
    SetPlotOptions(L)
    SetActivePlots((0,1))
    AddOperator("Slice")
    DrawPlots()

    v = View2DAttributes()
    v.windowCoords = (-0.370383, 0.566874, -0.275836, 0.66177)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    a = GetAnnotationAttributes()
    a.xAxisLabels2D = 0
    a.yAxisLabels2D = 0
    a.xAxisTitle2D = 0
    a.yAxisTitle2D = 0
    SetAnnotationAttributes(a)
    SaveTestImage("label_9_00")

    AddPlot("Vector", "grad")
    vec = VectorAttributes()
    vec.useStride = 1
    vec.scale = 0.5
    SetPlotOptions(vec)
    AddOperator("Slice")
    SetActivePlots((0,1,2))
    s = SliceAttributes()
    s.project2d = 0
    SetOperatorOptions(s)
    DrawPlots()
    v = View3DAttributes()
    v.viewNormal = (-0.332304, 0.933436, 0.135169)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.353747, 0.00950422, -0.935293)
    v.viewAngle = 30
    v.parallelScale = 14.1421
    v.nearPlane = -28.2843
    v.farPlane = 28.2843
    v.imagePan = (0, 0)
    v.imageZoom = 17.3567
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)
    SaveTestImage("label_9_01")

    # Test a case where having the vector be an expression was causing
    # VisIt to mess up.
    DeleteAllPlots()
    OpenDatabase("../data/rect3d.silo")
    AddPlot("Mesh", "quadmesh3d")
    AddPlot("Label", "vel")
    L = LabelAttributes()
    L.textHeight1 = 0.04
    L.textColor1 = (255,0,0,255)
    L.specifyTextColor1 = 1
    SetPlotOptions(L)
    SetActivePlots((0,1))
    AddOperator("Slice")
    s = SliceAttributes()
    s.originType = s.Intercept
    s.originIntercept = 0
    s.axisType = s.YAxis
    s.project2d = 1
    SetOperatorOptions(s)
    DrawPlots()
    v = View2DAttributes()
    v.windowCoords = (0.0476715, 0.155784, 0.874164, 0.991825)
    v.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    v.fullFrameActivationMode = v.Off  # On, Off, Auto
    v.fullFrameAutoThreshold = 100
    SetView2D(v)
    SaveTestImage("label_9_02")

def TestRectilinearLogicalDisplay():
    TestSection("Test 3D zooming and logical mesh display")
    DeleteAllPlots()

    aa = GetAnnotationAttributes()
    aa.bboxFlag = 0
    aa.axesFlag = 0
    SetAnnotationAttributes(aa)

    OpenDatabase("../data/noise.silo")
    AddPlot("Pseudocolor", "hardyglobal")
    AddPlot("Mesh", "Mesh")
    AddPlot("Label", "Mesh")
    L = CreateLabelAttributes()
    SetPlotOptions(L)
    DrawPlots()

    # Also zoom in using some views obtained from rubber-band style
    # views that would have made the labels migrate from their
    # correct locations in older versions of VisIt.
    v = View3DAttributes()
    v.viewNormal = (-0.566084, -0.288713, 0.772135)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.357175, 0.930078, 0.0859103)
    v.viewAngle = 30
    v.parallelScale = 17.3205
    v.nearPlane = -34.641
    v.farPlane = 34.641
    v.imagePan = (0.104981, 0.174394)
    v.imageZoom = 12.8916
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)
    SaveTestImage("label_10_00")

    # Make it be displayed using index style numbers.
    L.labelDisplayFormat = L.Index
    SetPlotOptions(L)
    SaveTestImage("label_10_01")

    # Make sure the labels are in the right place when we tell VisIt
    # to draw them all instead of binning.
    L.restrictNumberOfLabels = 0
    SetPlotOptions(L)
    SaveTestImage("label_10_02")

    # Try a nodal variable.
    ChangeActivePlotsVar("hardyglobal")
    SaveTestImage("label_10_03")

    # Try a 2D rectilinear mesh
    DeleteAllPlots()
    AddPlot("Pseudocolor", "hgslice")
    AddPlot("Mesh", "Mesh2D")
    AddPlot("Label", "Mesh2D")
    L = CreateLabelAttributes()
    SetPlotOptions(L)
    DrawPlots()

    v1 = GetView2D()
    v1.windowCoords = (-7.52178, -5.09684, -8.8535, -6.43779)
    v1.viewportCoords = (0.2, 0.95, 0.15, 0.95)
    SetView2D(v1)
    SaveTestImage("label_10_04")

    # Try a 3D curvilinear mesh
    DeleteAllPlots()
    OpenDatabase("../data/curv3d.silo")
    AddPlot("Mesh", "curvmesh3d")
    m = MeshAttributes()
    m.opaqueMode = m.On
    SetPlotOptions(m)
    AddPlot("Label", "curvmesh3d")
    L = CreateLabelAttributes()
    SetPlotOptions(L)
    DrawPlots()
    v2 = GetView3D()
    v2.viewNormal = (0.405434, 0.494827, 0.768615)
    v2.focus = (0, 2.5, 15)
    v2.viewUp = (-0.296176, 0.866572, -0.401662)
    v2.viewAngle = 30
    v2.parallelScale = 16.0078
    v2.nearPlane = -32.0156
    v2.farPlane = 32.0156
    v2.imagePan = (0.129303, 0.36944)
    v2.imageZoom = 43.2454
    v2.perspective = 1
    v2.eyeAngle = 2
    v2.centerOfRotationSet = 0
    v2.centerOfRotation = (0, 2.5, 15)
    SetView3D(v2)
    SaveTestImage("label_10_05")

    ChangeActivePlotsVar("u")
    SaveTestImage("label_10_06")
    DeleteAllPlots()


def main():
    TestGeneralFeatures()
    TestCurvilinear2D()
    TestRectilinear2D()
    TestUnstructured2D()
    TestSlice()
    TestLabeledVTK()
    TestLabellingSubsets()
    TestLabellingTensors()
    TestLabelling3D()
    TestSlicedVectors()
    TestRectilinearLogicalDisplay()

# Run all of the tests
main()

# Exit the test
Exit()
