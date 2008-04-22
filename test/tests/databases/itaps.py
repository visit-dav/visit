# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  itaps.py 
#
#  Tests:      ITAPS-MOAB files (the MOAB implementation of ITAPS interface)
#
#  Programmer: Mark C. Miller 
#  Date:       March 15, 2007 
#
#  Modifications
#
#    Mark C. Miller, Thu Mar 22 11:01:08 PDT 2007
#    Added tests for some varaibles
#
#    Mark C. Miller, Wed Jun 27 10:57:17 PDT 2007
#    Changed name from iTaps to ITAPS
#
#    Mark C. Miller, Tue Jul  3 15:26:41 PDT 2007
#    Changed to use ITAPS_C plugin
#
# ----------------------------------------------------------------------------

edgeSetId = -1
faceSetId = -1
regSetId = -1
silr = 0

def InitSetIds():
    global edgeSetId
    global faceSetId
    global regSetId
    global silr
    edgeSetId = -1
    faceSetId = -1
    regSetId = -1
    silr = SILRestriction()
    for i in range(silr.NumSets()):
        if silr.SetName(i) == "EDGE":
            edgeSetId = i
        if silr.SetName(i) == "FACE":
            faceSetId = i
        if silr.SetName(i) == "REGION":
            regSetId = i

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

# test the main mesh
OpenDatabase("../data/iTaps_test_data/mbtest1", 0, "ITAPS_C_1.0")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
DrawPlots()
v=GetView3D()
v.viewNormal = (-0.761903, 0.456228, -0.45974)
v.viewUp = (0.198721, 0.840238, 0.50449)
SetView3D(v)
InitSetIds()
silr.TurnOffSet(edgeSetId)
silr.TurnOffSet(faceSetId)
SetPlotSILRestriction(silr)
Test("itaps_01")
silr.TurnOnSet(edgeSetId)
silr.TurnOnSet(faceSetId)
SetPlotSILRestriction(silr)

SetActivePlots((0,1))

# turn on/off various combinations of EDGEs, FACEs, and REGIONs
silr.TurnOffSet(edgeSetId)
silr.TurnOffSet(regSetId)
SetPlotSILRestriction(silr)
DrawPlots()
Test("itaps_02")

silr.TurnOffSet(faceSetId)
silr.TurnOnSet(edgeSetId)
SetPlotSILRestriction(silr)
DrawPlots()
Test("itaps_03")

silr.TurnOffSet(edgeSetId)
silr.TurnOnSet(regSetId)
SetPlotSILRestriction(silr)
DrawPlots()
Test("itaps_04")

silr.TurnOnSet(edgeSetId)
silr.TurnOnSet(faceSetId)
silr.TurnOffSet(regSetId)
SetPlotSILRestriction(silr)
DrawPlots()
Test("itaps_05")

DeleteAllPlots()
CloseDatabase("../data/iTaps_test_data/mbtest1")

OpenDatabase("../data/iTaps_test_data/bricks.cub", 0, "ITAPS_C_1.0")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
DrawPlots()
ResetView()
InitSetIds()
SetActivePlots((0,1))
silr.TurnOffSet(faceSetId)
silr.TurnOffSet(regSetId)
SetPlotSILRestriction(silr)
DrawPlots()
Test("itaps_06")

DeleteAllPlots()
CloseDatabase("../data/iTaps_test_data/bricks.cub")

# test another mesh with some different element types
OpenDatabase("../data/iTaps_test_data/globe_mats", 0, "ITAPS_C_1.0")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
SetActivePlots((0,1))
AddOperator("Slice")
DrawPlots()
ResetView()
Test("itaps_07")

DeleteAllPlots()
CloseDatabase("../data/iTaps_test_data/bricks.cub")

# test another mesh with some different element types
OpenDatabase("../data/iTaps_test_data/mixed-hex-pyr-tet", 0, "ITAPS_C_1.0")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
DrawPlots()
InitSetIds()
ResetView()
v=GetView3D()
v.viewNormal = (-0.707107, 0, 0.707107)
SetView3D(v)
Test("itaps_08")

setIdsTxt="entity type set ids = %d %d %d"%(edgeSetId, faceSetId, regSetId)
TestText("itaps_09", setIdsTxt)

DeleteAllPlots()
CloseDatabase("../data/iTaps_test_data/mixed-hex-pyr-tet")

# test some variables now (node-centered, zone-centered, integer valued)
OpenDatabase("../data/iTaps_test_data/globe_mats", 0, "ITAPS_C_1.0")
AddPlot("Pseudocolor","dx")
DrawPlots()
Test("itaps_10")
DeleteAllPlots()

AddPlot("Pseudocolor","u")
DrawPlots()
Test("itaps_11")
DeleteAllPlots()

AddPlot("Pseudocolor","Subsets")
AddOperator("Slice")
DrawPlots()
Test("itaps_12")
DeleteAllPlots()

Exit()
