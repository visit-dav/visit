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
#
#    Mark C. Miller, Thu Jul 31 17:06:06 PDT 2008
#    Removed extraneous printf statements
#
#    Mark C. Miller, Wed Sep 10 15:16:47 PDT 2008
#    Modified for the case where the ITAPS plugin is linked against multiple
#    implementations of the ITAPS interfaces and added tests to test those
#    implementations.
#
#    Mark C. Miller, Mon Mar 30 16:56:03 PDT 2009
#    Replaced bricks.cub input file with brick_cubit10.cub because MOAB
#    was failing to read bricks.cub (a version 9.1 Cubit file).
#    Added threshold operator to mbtest1 because new version of MOAB is
#    serving up 3D mesh with some 2D zones in it. Enabled ptest.cube test.
#
#    Mark C. Miller, Tue Aug 10 20:02:23 PDT 2010
#    Added logic to skip some cases in parallel regardless of skip list
#    setting. This is due to probably parallel hang and putting tests in
#    skip list alone does not prevent execution of the tests. It prevents
#    only having differeing outcome leading to failed test. 
#
#    Mark C. Miller, Wed Aug 11 09:10:52 PDT 2010
#    Made skiping logic (above) trigger in any parallel mode, not just
#    non-scalable, parallel.
#
#    Mark C. Miller, Thu Feb 10 05:37:38 PST 2011
#    Fixed typo in name of database to close.
# ----------------------------------------------------------------------------

def RestrictSetsInCategory(silr, className, setIds):
    catName = className + "s";
    if className == "Vertex":
        catName = "Verticies"
    i = 0
    # want everything ON except sets NOT listed in setIds
    silr.TurnOnAll()
    for set in silr.SetsInCategory(catName):
        if i < len(setIds) and \
            (silr.SetName(set) == "%s_%03d"%(className, setIds[i]) or \
             silr.SetName(set) == "%s%d"%(className, setIds[i])):
            i = i + 1
        else:
            silr.TurnOffSet(set)

# test the main mesh and MOAB implementation
OpenDatabase(data_path("iTaps_test_data/MOAB/mbtest1"),0, "ITAPS_MOAB_1.0")

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
# use threshold to display only 3D zones (upper case ASCII range)
th = ThresholdAttributes()
th.listedVarNames = ("zonetype")
th.lowerBounds = (65)
th.upperBounds = (90)
SetDefaultOperatorOptions(th)
AddOperator("Threshold")
DrawPlots()
v=GetView3D()
v.viewNormal = (-0.761903, 0.456228, -0.45974)
v.viewUp = (0.198721, 0.840238, 0.50449)
SetView3D(v)
Test("itaps_01")
DeleteAllPlots()
CloseDatabase(data_path("iTaps_test_data/MOAB/mbtest1"))


# open a database with some interesting sets
OpenDatabase(data_path("iTaps_test_data/MOAB/brick_cubit10.cub"),0, "ITAPS_MOAB_1.0")

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
RestrictSetsInCategory(silr, "Curve", (0,1,2,6,7,11))
SetPlotSILRestriction(silr)
Test("itaps_03")

# turn off some surfaces
RestrictSetsInCategory(silr, "Surface", (1,2))
SetPlotSILRestriction(silr)
Test("itaps_04")

AddPlot("Pseudocolor","zonetype")
DrawPlots()
Test("itaps_05")

DeleteAllPlots()
CloseDatabase(data_path("iTaps_test_data/MOAB/brick_cubit10.cub"))


OpenDatabase(data_path("iTaps_test_data/MOAB/ptest.cub"),0, "ITAPS_MOAB_1.0")

AddPlot("Mesh","mesh")
DrawPlots()
ResetView()
silr=SILRestriction()
RestrictSetsInCategory(silr, "Volume", (0,2,4,6))
SetPlotSILRestriction(silr)
Test("itaps_06")

DeleteAllPlots()
CloseDatabase(data_path("iTaps_test_data/MOAB/ptest.cub"))


# test another mesh with some different element types
OpenDatabase(data_path("iTaps_test_data/MOAB/globe_mats"),0, "ITAPS_MOAB_1.0")

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
SetActivePlots((0,1))
AddOperator("Slice")
DrawPlots()
ResetView()
Test("itaps_07")

DeleteAllPlots()
CloseDatabase(data_path("iTaps_test_data/MOAB/globe_mats"))


# test another mesh with some different element types
OpenDatabase(data_path("iTaps_test_data/MOAB/mixed-hex-pyr-tet"),0, "ITAPS_MOAB_1.0")

AddPlot("Mesh","mesh")
AddPlot("Pseudocolor","zonetype")
DrawPlots()
ResetView()
v=GetView3D()
v.viewNormal = (-0.707107, 0, 0.707107)
SetView3D(v)
Test("itaps_08")

DeleteAllPlots()
CloseDatabase(data_path("iTaps_test_data/MOAB/mixed-hex-pyr-tet"))


# test some variables now (node-centered, zone-centered, integer valued)
OpenDatabase(data_path("iTaps_test_data/MOAB/globe_mats"),0, "ITAPS_MOAB_1.0")

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

#
# Loop to test all three implementations are built, co-existing and working
#
n=13
for imp in (("MOAB","mixed-hex-pyr-tet"),("FMDB","human-1-fmdb.sms"),("GRUMMP","tire.vmesh")):
    OpenDatabase(data_path("iTaps_test_data/%s/%s")%imp,0, "ITAPS_%s_1.0" % imp[0])
    AddPlot("Mesh","mesh")
    DrawPlots()
    ResetView()
    if imp[0] == "FMDB":
        v = GetView3D()
        v.RotateAxis(0,90.0)
        SetView3D(v)
    Test("itaps_%d"%n)
    DeleteAllPlots()
    CloseDatabase(data_path("iTaps_test_data/%s/%s")%imp)

    n = n + 1

Exit()
