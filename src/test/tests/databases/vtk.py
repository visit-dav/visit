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
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Brad Whitlock, Tue Oct 23 11:18:12 PDT 2012
#    I added tests for NBLOCKS .visit files, cycle & time, and PVTU files.
#
#    Eric Brugger, Tue Jun 25 18:20:31 PDT 2013
#    I added a test for PVTI files.
#
#    Kathleen Biagas, Wed Feb 25 14:10:13 PST 2015
#    Mesh name has been added to blocks.vtu test data, so add mesh plot
#    to test is has been read correctly.
#
#    Kathleen Biagas, Tue Aug 18 14:00:16 PDT 2015
#    Added a test for VTM files.
#
#    Kathleen Biagas, Thu Sep 21 14:56:25 MST 2017
#    Added a test for PVTK files.
#
#    Edward Rusu, Mon Oct 01 15:09:24 PST 2018
#    Added a test for vtkGhostType.
#
#    Kathleen Biagas, Tue Aug 24, 2021
#    Added a test for PVD files.
#
# ----------------------------------------------------------------------------

def TestMaterials():
    TestSection("Materials in VTK")
    OpenDatabase(data_path("vtk_test_data/globe_mats.vtk"))


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
    DrawPlots()
    Test("vtk_03")

    TurnMaterialsOn()
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/globe_mats.vtk"))

def TestXML():
    TestSection("VTK XML-style data files")
    OpenDatabase(data_path("vtk_xml_test_data/curv2d.vts"))

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
    CloseDatabase(data_path("vtk_xml_test_data/curv2d.vts"))

    OpenDatabase(data_path("vtk_xml_test_data/curv3d.vts"))

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
    CloseDatabase(data_path("vtk_xml_test_data/curv3d.vts"))

    OpenDatabase(data_path("vtk_xml_test_data/rect3d.vtr"))

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
    CloseDatabase(data_path("vtk_xml_test_data/rect3d.vtr"))

    OpenDatabase(data_path("vtk_xml_test_data/ucd2d.vtu"))

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
    CloseDatabase(data_path("vtk_xml_test_data/ucd2d.vtu"))

def TestHigherOrder():
    TestSection("Quadratic triangles in VTK")
    OpenDatabase(data_path("vtk_test_data/higher_order_triangles.vtk"))

    AddPlot("Mesh", "highorder_triangles");
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

    DefineScalarExpression("zoneid", "zoneid(highorder_triangles)")
    AddPlot("Pseudocolor", "zoneid")
    p = PseudocolorAttributes()
    p.colorTableName = "levels"
    SetPlotOptions(p)
    DrawPlots()
    Test("vtk_29")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/higher_order_triangles.vtk"))

def TestNBLOCKS():
    TestSection("!NBLOCKS in a .visit file")
    OpenDatabase(data_path("vtk_test_data/visitfile/dbA.visit"))
    AddPlot("Pseudocolor", "mesh/ireg")
    DrawPlots()
    ResetView()
    Test("vtk_30")

    # See that there are multiple time steps.
    SetTimeSliderState(5)
    Test("vtk_31")
    DeleteAllPlots()

    # See that there are 2 domains
    AddPlot("Subset", "domains")
    DrawPlots()
    Test("vtk_32")

    # Test that we can get the cycle and time out.
    txt = ""
    for i in range(10):
        SetTimeSliderState(i)
        Query("Cycle")
        cycle = GetQueryOutputValue()
        Query("Time")
        time = GetQueryOutputValue()
        txt = txt + "Cycle: %d, Time: %g\n" % (cycle, time)
    TestText("vtk_33", txt)
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/visitfile/dbA.visit"))

def TestPVTU():
    TestSection("PVTU files")
    OpenDatabase(data_path("vtk_test_data/blocks.pvtu"))
    AddPlot("Pseudocolor", "MatNum")
    DrawPlots()
    ResetView()
    Test("vtk_34")
    # MeshName recently added, so test.
    AddPlot("Mesh", "blocks")
    DrawPlots()
    Test("vtk_34a")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/blocks.pvtu"))

def TestPVTI():
    TestSection("PVTI files")
    OpenDatabase(data_path("vtk_xml_test_data/earth.pvti"))
    AddPlot("Truecolor", "PNGImage")
    DrawPlots()
    ResetView()
    Test("vtk_35")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_xml_test_data/earth.pvti"))
    
def TestMixedTopology():
    TestSection("2D Unstructured grids with lines")
    OpenDatabase(data_path("vtk_test_data/test_bound.vtk"))
    AddPlot("Pseudocolor", "data_cells_with_bound")
    DrawPlots()
    ResetView()
    Test("vtk_36")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/test_bound.vtk"))

def TestVTM():
    TestSection("VTM files")
    f = "files.vtm"
    fN = data_path("vtk_xml_test_data/MultiBlock/%s"%f)
    OpenDatabase(fN)
    AddPlot("Subset", "Blocks")
    DrawPlots()
    ResetView()
    Test("vtk_37")

    mmd = GetMetaData(fN).GetMeshes(0)
    s = f + ":    Groups: " + ", ".join(mmd.groupNames)
    s = s + "     Blocks: " + ", ".join(mmd.blockNames) + "\n"

    DeleteAllPlots()
    CloseDatabase(fN)
    
    f = "files_grouped.vtm"
    fN = data_path("vtk_xml_test_data/MultiBlock/%s"%f)
    OpenDatabase(fN)
    AddPlot("Subset", "Blocks")
    DrawPlots()
    ResetView()
    Test("vtk_38")

    ChangeActivePlotsVar("Groups")
    DrawPlots()
    ResetView()
    Test("vtk_39")

    mmd = GetMetaData(fN).GetMeshes(0)
    s = s + f + ":    Groups: " + ", ".join(mmd.groupNames)
    s = s + "    Blocks: " + ", ".join(mmd.blockNames) + "\n"
    TestText("vtk_40", s)   
    DeleteAllPlots()
    CloseDatabase(fN)

def TestPVTK():
    TestSection("PVTK files")
    f = "earth.pvtk"
    fN = data_path("vtk_test_data/pvtk/%s"%f)
    OpenDatabase(fN)
    AddPlot("Truecolor", "PNGImage")
    DrawPlots()
    ResetView()
    Test("vtk_41")

    DeleteAllPlots()
    CloseDatabase(fN)
    
    f = "dbA.pvtk"
    fN = data_path("vtk_test_data/pvtk/dbA/%s"%f)
    OpenDatabase(fN)
    AddPlot("Pseudocolor", "mesh/ireg")
    DrawPlots()
    ResetView()
    Test("vtk_42")

    DeleteAllPlots()
    CloseDatabase(fN)

def TestVTKGhostType():
    TestSection("VTK ghost types")
    OpenDatabase(data_path("vtk_test_data/vtkGhostType.vtk"))
    ResetView()
    AddPlot("Pseudocolor", "internal_var_GhostZones")
    DrawPlots()
    Test("vtk_43")

    DeleteAllPlots()
    AddPlot("Pseudocolor", "internal_var_GhostNodes")
    DrawPlots()
    Test("vtk_44")

    DeleteAllPlots()
    AddPlot("Mesh", "Mesh_gnz")
    DrawPlots()
    Test("vtk_45")
    
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/vtkGhostType.vtk"))

def TestDBExpressions():
    TestSection("Database Expressions in VTK")
    OpenDatabase(data_path("vtk_test_data/higher_order_triangles.vtk"))

    AddPlot("Mesh", "highorder_triangles");
    AddPlot("Pseudocolor", "x")
    AddPlot("Vector", "posvec")
    DrawPlots()

    v = View3DAttributes()
    v.viewNormal = (-0.429056, 0.759111, 0.489553)
    v.viewUp = (-0.901832, -0.39065, -0.184638)
    v.parallelScale = 1.73205
    v.nearPlane = -3.4641
    v.farPlane = 3.4641
    v.imageZoom = 1.45106
    SetView3D(v)

    Test("vtk_46")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_test_data/higher_order_triangles.vtk"))

def TestPVD():

    TestSection("PVD, single pvti file, no time, no groups")
    OpenDatabase(data_path("vtk_xml_test_data/earth.pvd"))
    AddPlot("Truecolor", "PNGImage")
    DrawPlots()
    ResetView()
    Test("vtk_pvd_01")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_xml_test_data/earth.pvd"))

    TestSection("PVD, multiple vtr files, single time, no groups")
    OpenDatabase(data_path("vtk_xml_test_data/multiRect3d.pvd"))
    AddPlot("Pseudocolor", "u")
    DrawPlots()
    ResetView()
    Test("vtk_pvd_02")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_xml_test_data/multiRect3d.pvd"))

    TestSection("PVD, multiple pvtr files, multiple times, no groups")
    OpenDatabase(data_path("vtk_xml_test_data/multi_dir/multi_time_pvtr.pvd"))
    AddPlot("Pseudocolor", "radial")
    DrawPlots()
    ResetView()
    Test("vtk_pvd_03")
    TimeSliderSetState(5)
    Test("vtk_pvd_04")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_xml_test_data/multi_dir/multi_time_pvtr.pvd"))

    TestSection("PVD, multiple vtr files, multiple times, groups")
    OpenDatabase(data_path("vtk_xml_test_data/multi_dir/multi_time_part_vtr.pvd"))
    AddPlot("Subset", "parts")
    DrawPlots()
    Test("vtk_pvd_05")
    ChangeActivePlotsVar("blocks")
    Test("vtk_pvd_06")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_xml_test_data/multi_dir/multi_time_part_vtr.pvd"))

    TestSection("PVD, multiple vtm files,multiple times")
    OpenDatabase(data_path("vtk_xml_test_data/multi_dir/multi_time_vtm.pvd"))
    AddPlot("Pseudocolor", "radial")
    DrawPlots()
    ResetView()
    Test("vtk_pvd_07")
    TimeSliderSetState(5)
    Test("vtk_pvd_08")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_xml_test_data/multi_dir/multi_time_vtm.pvd"))

    TestSection("PVD, multiple pvti files, multiple times")
    OpenDatabase(data_path("vtk_xml_test_data/earth_multiTime/earth_time.pvd"))
    AddPlot("Truecolor", "PNGImage")
    DrawPlots()
    ResetView()
    Test("vtk_pvd_09")
    TimeSliderNextState()
    Test("vtk_pvd_10")
    TimeSliderNextState()
    Test("vtk_pvd_11")
    TimeSliderNextState()
    Test("vtk_pvd_12")
    DeleteAllPlots()
    CloseDatabase(data_path("vtk_xml_test_data/earth_multiTime/earth_time.pvd"))


TestMaterials()
TestXML()
TestHigherOrder()
TestNBLOCKS()
TestPVTU()
TestPVTI()
TestMixedTopology()
TestVTM()
TestPVTK()
TestVTKGhostType()
TestDBExpressions()
TestPVD()

Exit()
