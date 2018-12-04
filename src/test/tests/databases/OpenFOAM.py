# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  OpenFOAM.py
#
#  Programmer: Hank Childs
#  Date:       September 8, 2006
#
#  Modifications:
#    Jeremy Meredith, Tue Jul 15 10:43:58 EDT 2008
#    Changed number of vectors in vector plot to match the old behavior.
#    (We now account for how many domains there are.)
#
#    Kathleen Biagas, Tue Aug 20 14:06:46 PDT 2013
#    Reader has been updated, default mesh is now 'internalMesh', and
#    variables need mesh name.
#
#    Kathleen Biagas, Wed Aug 28 12:04:03 PDT 2013
#    Added tests for new data, different Open Options.
#
# ----------------------------------------------------------------------------

def OrigTests():
    OpenDatabase(data_path("OpenFOAM_test_data/bubbleColumn/system/controlDict"))

    AddPlot("Vector", "internalMesh/Ua")
    v = VectorAttributes()
    v.nVectors = 400*5
    SetPlotOptions(v)
    DrawPlots()
    Test("OpenFOAM_01")

    TimeSliderNextState()
    Test("OpenFOAM_02")
    TimeSliderNextState()
    Test("OpenFOAM_03")

    AddPlot("Mesh", "internalMesh")
    DrawPlots()
    Test("OpenFOAM_04")

    DeleteAllPlots()

    AddPlot("Pseudocolor", "internalMesh/epsilon")
    DrawPlots()
    Test("OpenFOAM_05")

    DeleteAllPlots()

    OpenDatabase(data_path("OpenFOAM_test_data/mixer2D_ascii/system/controlDict"))

    AddPlot("Vector", "internalMesh/U")
    v = VectorAttributes()
    v.nVectors = 400*6
    SetPlotOptions(v)
    DrawPlots()
    Test("OpenFOAM_06")

    TimeSliderNextState()
    Test("OpenFOAM_07")
    TimeSliderNextState()
    Test("OpenFOAM_08")

    AddPlot("Mesh", "internalMesh")
    DrawPlots()
    Test("OpenFOAM_09")

    DeleteAllPlots()

    AddPlot("Pseudocolor", "internalMesh/p")
    DrawPlots()
    Test("OpenFOAM_10")

    DeleteAllPlots()

def TestLagrangian():
    OpenDatabase(data_path("OpenFOAM_test_data/simplifiedSiwek/system/controlDict"))
    # Point clouds don't exist at timestate 0, so advance by 1.
    TimeSliderNextState()

    AddPlot("Subset", "internalMesh")
    subset = SubsetAttributes()
    subset.wireframe = 1
    subset.colorType = subset.ColorBySingleColor
    SetPlotOptions(subset)
    AddPlot("Mesh", "lagrangian/coalCloud1")
    coalMesh = MeshAttributes()
    coalMesh.meshColorSource = coalMesh.MeshCustom
    coalMesh.meshColor = (255, 0, 0, 255)
    coalMesh.pointType = coalMesh.Sphere
    coalMesh.pointSizePixels = 15
    SetPlotOptions(coalMesh)
    AddPlot("Mesh", "lagrangian/limestoneCloud1")
    limestoneMesh = MeshAttributes()
    limestoneMesh.meshColorSource = limestoneMesh.MeshCustom
    limestoneMesh.meshColor = (0, 255, 0, 255)
    limestoneMesh.pointType = limestoneMesh.Sphere
    limestoneMesh.pointSizePixels = 15
    SetPlotOptions(limestoneMesh)

    coalCloudText = CreateAnnotationObject("Text2D")
    coalCloudText.text = "coalCloud1"
    coalCloudText.height = 0.04
    coalCloudText.position = (0.4, 0.85)
    coalCloudText.useForegroundForTextColor = 0
    coalCloudText.textColor = (255, 0, 0, 255)

    limestoneCloudText = CreateAnnotationObject("Text2D")
    limestoneCloudText.text = "limestoneCloud1"
    limestoneCloudText.height = 0.04
    limestoneCloudText.position = (0.4, 0.75)
    limestoneCloudText.useForegroundForTextColor = 0
    limestoneCloudText.textColor = (0, 255, 0, 255)

    slider = CreateAnnotationObject("TimeSlider")
    slider.position = (0.4, 0.6)
    slider.width = (0.5)
    slider.height = (0.1)
    
    DrawPlots()
    Test("OpenFOAM_lagrangian_01")

    TimeSliderSetState(4)
    Test("OpenFOAM_lagrangian_02")
    TimeSliderSetState(7)
    Test("OpenFOAM_lagrangian_03")
    SetActivePlots((1, 2))
    DeleteActivePlots()
    TimeSliderSetState(10)
    AddPlot("Pseudocolor", "lagrangian/coalCloud1/tTurb")
    pc = PseudocolorAttributes()
    pc.pointType = pc.Sphere
    pc.pointSizePixels = 15
    SetPlotOptions(pc)
    DrawPlots()
    coalCloudText.textColor = (0, 0, 0, 255)
    coalCloudText.text = "coalcloud1/tTurb"
    limestoneCloudText.visible = 0
    Test("OpenFOAM_lagrangian_04")
    TimeSliderSetState(13)
    Test("OpenFOAM_lagrangian_05")
    ChangeActivePlotsVar("lagrangian/limestoneCloud1/tTurb")
    coalCloudText.visible = 0
    limestoneCloudText.visible = 1
    limestoneCloudText.textColor = (0, 0, 0, 255)
    limestoneCloudText.text = "limestoneCloud1/tTurb"
    Test("OpenFOAM_lagrangian_06")
    TimeSliderSetState(16)
    Test("OpenFOAM_lagrangian_07")
    
    coalCloudText.Delete()
    limestoneCloudText.Delete()
    slider.Delete()

    DeleteAllPlots()

def TurnOffSetsByName(silr, cat, names):
    sets = silr.SetsInCategory(cat)
    for s in sets:
        setname = silr.SetName(s)
        try:
            if setname in names:
                silr.TurnOffSet(s)
        except:
            if setname == names:
                silr.TurnOffSet(s)

def TestMultiRegion():
    OpenDatabase(data_path("OpenFOAM_test_data/snappyMultiRegionHeater/system/controlDict"))
    AddPlot("Subset", "Region")
    DrawPlots()
    Test("OpenFOAM_MultiRegion_01")

    silr = SILRestriction()
    TurnOffSetsByName(silr, "Region", ("bottomAir, topAir"))
    SetPlotSILRestriction(silr)
    Test("OpenFOAM_MultiRegion_02")

    ChangeActivePlotsVar("boundary(Patches)")
    silr = SILRestriction()
    silr.TurnOnAll()
    v = GetView3D()
    v.viewNormal = (-0.566091, -0.118534, 0.815776)
    v.viewUp = (0.0223539, 0.987037, 0.15893)
    SetView3D(v)

    Test("OpenFOAM_MultiRegion_03")
    TurnDomainsOff(("maxY", "maxZ"))
    Test("OpenFOAM_MultiRegion_04")

    silr.TurnOnAll()
    DeleteAllPlots()

    AddPlot("Pseudocolor", "heater/internalMesh/T")
    v = GetView3D()
    v.viewNormal = (-1, 0, 0)
    SetView3D(v)
    DrawPlots()
    Test("OpenFOAM_MultiRegion_05")
    TimeSliderNextState()
    Test("OpenFOAM_MultiRegion_06")
    DeleteAllPlots()
    
def TestDecomposed():
    opts = GetDefaultFileOpenOptions("OpenFOAM")
    opts['Case Type'] = 0 #Decomposed
    opts['Convert Cell Data To Point Data'] = 1 
    SetDefaultFileOpenOptions("OpenFOAM", opts)
    OpenDatabase(data_path("OpenFOAM_test_data/damBreakFineDecomposed/system/controlDict"))
    AddPlot("Pseudocolor", "internalMesh/U_magnitude")
    ResetView()
    TimeSliderSetState(2)
    DrawPlots()
    Test("OpenFOAM_Decomposed_01")
    TimeSliderSetState(6)
    Test("OpenFOAM_Decomposed_02")
    TimeSliderSetState(8)
    Test("OpenFOAM_Decomposed_03")
    DeleteAllPlots() 

def TestReadZones():
    opts = GetDefaultFileOpenOptions("OpenFOAM")
    opts['Case Type'] = 1 #Reconstructed
    opts['Convert Cell Data To Point Data'] = 0 
    opts['Read Zones'] = 1 
    SetDefaultFileOpenOptions("OpenFOAM", opts)
    # need a ReOpen because we changed the Open options, and the file
    # wasn't closed from previous test.
    ReOpenDatabase(data_path("OpenFOAM_test_data/snappyMultiRegionHeater/system/controlDict"))
    AddPlot("Mesh", "faceZones")
    DrawPlots()
    v = GetView3D()
    v.viewNormal = (-0.407812, -0.112136, 0.906154)
    v.viewUp = (0.00527397, 0.992124, 0.125148)
    SetView3D(v)
    Test("OpenFOAM_ReadZones_01")
    ChangeActivePlotsVar("cellZones")
    Test("OpenFOAM_ReadZones_02")
    TurnDomainsOff(("topAir", "bottomAir"))
    Test("OpenFOAM_ReadZones_03")
    DeleteAllPlots()    

OrigTests()
TestLagrangian()
TestMultiRegion()
TestDecomposed()
TestReadZones()

Exit()
