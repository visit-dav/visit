# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  vtk.py 
#
#  Programmer: Mark C. Miller 
#  Date:       October 28, 2004 
#
#  Modifications:
#    Kathleen Bonnell, Fri Sep 23 10:55:44 PDT 2005
#    Re-organized code into separate methods.  Added TestXML method.
#
#    Brad Whitlock, Mon May 8 15:57:12 PST 2006
#    Added test for higher_order_triangles.vtk
#
# ----------------------------------------------------------------------------

def TestMaterials():
    TestSection("Materials in VTK")
    OpenDatabase("../data/globe_mats.vtk")

    #
    # Test simple read and display of a material variable 
    #
    AddPlot("FilledBoundary","materials")
    DrawPlots()
    Test("vtk_01")

    #
    # Find set ids using their names
    #
    setId1 = -1
    setId2 = -1
    setId3 = -1
    silr = SILRestriction()
    for i in range(silr.NumSets()):
        if silr.SetName(i) == "1":
            setId1 = i
        elif silr.SetName(i) == "2":
            setId2 = i
        elif silr.SetName(i) == "3":
            setId3 = i

    #
    # Turn on air, but turn void and uranium off
    #
    silr.TurnOffSet(setId2)
    SetPlotSILRestriction(silr)
    Test("vtk_02")

    silr.TurnOnSet(setId2)
    SetPlotSILRestriction(silr)
    AddOperator("Slice")
    Test("vtk_03")

    TurnMaterialsOn()
    DeleteAllPlots()

def TestXML():
    TestSection("VTK XML-style data files")
    OpenDatabase("../data/vtk_xml_test_data/curv2d.vts")
    AddPlot("Pseudocolor", "d");
    DrawPlots();
    Test("vtk_04")
    ChangeActivePlotsVar("p")
    DrawPlots()
    Test("vtk_05")
    ChangeActivePlotsVar("u")
    DrawPlots()
    Test("vtk_06")
    ChangeActivePlotsVar("v")
    DrawPlots()
    Test("vtk_07")
    DeleteActivePlots()
    AddPlot("Vector", "vel")
    DrawPlots()
    Test("vtk_08")
    DeleteActivePlots()

    OpenDatabase("../data/vtk_xml_test_data/curv3d.vts")
    AddPlot("Pseudocolor", "d");
    DrawPlots();
    Test("vtk_09")
    ChangeActivePlotsVar("p")
    DrawPlots()
    Test("vtk_10")
    ChangeActivePlotsVar("t")
    DrawPlots()
    Test("vtk_11")
    ChangeActivePlotsVar("u")
    DrawPlots()
    Test("vtk_12")
    ChangeActivePlotsVar("v")
    DrawPlots()
    Test("vtk_13")
    ChangeActivePlotsVar("w")
    DrawPlots()
    Test("vtk_14")
    DeleteActivePlots()
    AddPlot("Vector", "vel")
    DrawPlots()
    Test("vtk_15")
    DeleteActivePlots()


    OpenDatabase("../data/vtk_xml_test_data/rect3d.vtr")
    AddPlot("Pseudocolor", "d");
    DrawPlots();
    Test("vtk_16")
    ChangeActivePlotsVar("p")
    DrawPlots()
    Test("vtk_17")
    ChangeActivePlotsVar("t")
    DrawPlots()
    Test("vtk_18")
    ChangeActivePlotsVar("u")
    DrawPlots()
    Test("vtk_19")
    ChangeActivePlotsVar("v")
    DrawPlots()
    Test("vtk_20")
    ChangeActivePlotsVar("w")
    DrawPlots()
    Test("vtk_21")
    DeleteActivePlots()
    AddPlot("Vector", "vel")
    DrawPlots()
    Test("vtk_22")
    DeleteActivePlots()

    OpenDatabase("../data/vtk_xml_test_data/ucd2d.vtu")
    AddPlot("Pseudocolor", "d");
    DrawPlots();
    Test("vtk_23")
    ChangeActivePlotsVar("speed")
    DrawPlots()
    Test("vtk_24")
    ChangeActivePlotsVar("u")
    DrawPlots()
    Test("vtk_25")
    ChangeActivePlotsVar("v")
    DrawPlots()
    Test("vtk_26")
    DeleteActivePlots()
    AddPlot("Vector", "vel")
    DrawPlots()
    Test("vtk_27")

    DeleteAllPlots()

def TestHigherOrder():
    TestSection("Quadratic triangles in VTK")
    OpenDatabase("../data/higher_order_triangles.vtk")
    AddPlot("Mesh", "mesh");
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.429056, 0.759111, 0.489553)
    v.focus = (0, 0, 0)
    v.viewUp = (-0.901832, -0.39065, -0.184638)
    v.viewAngle = 30
    v.parallelScale = 1.73205
    v.nearPlane = -3.4641
    v.farPlane = 3.4641
    v.imagePan = (0, 0)
    v.imageZoom = 1.45106
    v.perspective = 1
    v.eyeAngle = 2
    v.centerOfRotationSet = 0
    v.centerOfRotation = (0, 0, 0)
    SetView3D(v)

    Test("vtk_28")
    DeleteAllPlots()

    DefineScalarExpression("zoneid", "zoneid(mesh)")
    AddPlot("Pseudocolor", "zoneid")
    p = PseudocolorAttributes()
    p.colorTableName = "levels"
    SetPlotOptions(p)
    DrawPlots()
    Test("vtk_29")
    DeleteAllPlots()

TestMaterials()
TestXML()
TestHigherOrder()
Exit()
