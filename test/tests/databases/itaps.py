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
#    Mark C. Miller, Tue Apr 22 23:20:43 PDT 2008
#    Modified to test new subsetting functionality.
# ----------------------------------------------------------------------------

def RestrictSetsInCategory(silr, className, setIds):
    catName = className + "s";
    if className == "Vertex":
        catName = "Verticies"
    i = 0
    silr.TurnOnAll()
    for set in silr.SetsInCategory(catName):
        if i < len(setIds) and silr.SetName(set) == "%s_%03d"%(className, setIds[i]):
            i = i + 1;
            silr.TurnOnSet(set)
        elif i < len(setIds) and silr.SetName(set) == "%s%d"%(className, setIds[i]):
            print "Turning on set %s"%silr.SetName(set)
            i = i + 1;
            silr.TurnOnSet(set)
        else:
            print "Turning OFF set %s"%silr.SetName(set)
            silr.TurnOffSet(set)
        


# test the main mesh
OpenDatabase("../data/iTaps_test_data/mbtest1", 0, "ITAPS_C_1.0")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
DrawPlots()
v=GetView3D()
v.viewNormal = (-0.761903, 0.456228, -0.45974)
v.viewUp = (0.198721, 0.840238, 0.50449)
SetView3D(v)
Test("itaps_01")
DeleteAllPlots()
CloseDatabase("../data/iTaps_test_data/mbtest1")

# open a database with some interesting sets
OpenDatabase("../data/iTaps_test_data/bricks.cub")
AddPlot("Mesh","mesh")
DrawPlots()
ResetView()
v=GetView3D()
v.viewNormal = (-0.883969, 0.342986, 0.31774)
v.viewUp = (0.313293, 0.938981, -0.141992)
SetView3D(v)
Test("itaps_02")

SetActivePlots((0,1))
silr=SILRestriction()

# turn off some curves 
RestrictSetsInCategory(silr, "Curve", (0,1,2,3,4,5,6,7,8,9,10,11))
SetPlotSILRestriction(silr)
Test("itaps_03")

# turn off some surfaces
RestrictSetsInCategory(silr, "Surface", (1,2,7))
SetPlotSILRestriction(silr)
Test("itaps_04")

AddPlot("Pseudocolor","zonetype")
DrawPlots()
Test("itaps_05")

DeleteAllPlots()
CloseDatabase("../data/iTaps_test_data/bricks.cub")

#OpenDatabase("../data/iTaps_test_data/ptest.cub")
#AddPlot("Mesh","mesh")
#DrawPlots()
#ResetView()
#silr=SILRestriction()
#RestrictSetsInCategory(silr, "Volume", (0,2,4,6))
#SetPlotSILRestriction(silr)
#Test("itaps_06")

DeleteAllPlots()
CloseDatabase("../data/iTaps_test_data/ptest.cub")

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
CloseDatabase("../data/iTaps_test_data/globe_mats")

# test another mesh with some different element types
OpenDatabase("../data/iTaps_test_data/mixed-hex-pyr-tet", 0, "ITAPS_C_1.0")
AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
DrawPlots()
ResetView()
v=GetView3D()
v.viewNormal = (-0.707107, 0, 0.707107)
SetView3D(v)
Test("itaps_08")

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
