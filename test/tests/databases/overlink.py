# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  overlink.py 
#
#  Programmer: Mark C. Miller 
#  Date:       November 11, 2004 
#
#  Modifications:
#    Mark C. Miller, Mon Mar  2 12:44:59 PST 2009
#    Added tests for 3D and 2D ANNOTTAION_INT nodelists
#
#    Mark C. Miller, Thu Jun 18 21:43:45 PDT 2009
#    Added TestSection() for annotation int nodelists.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Mark C. Miller, Fri May  4 22:54:55 PDT 2012
#    Add a test for modestly large number of nodesets to test new vtkBitArray
#    approach to handling nodesets.
# ----------------------------------------------------------------------------


OpenDatabase("../data/overlink_test_data/regrovl_qh_1000_10001_4/OvlTop.silo")

#
# This is an interesting mesh 
#
AddPlot("Mesh","MMESH")
DrawPlots()
v=GetView3D()
v.viewNormal = (0.530656, 0.558421, 0.637629)
v.viewUp = (-0.401835, 0.82812, -0.390828)
SetView3D(v)
Test("overlink_01")

AddPlot("Pseudocolor","nvar1")
DrawPlots()
Test("overlink_02")

DeleteAllPlots()

AddPlot("Subset","domains(MMESH)")
DrawPlots()
Test("overlink_03")

silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "domain1":
        silr.TurnOffSet(i)
    elif silr.SetName(i) == "domain4":
        silr.TurnOffSet(i)
SetPlotSILRestriction(silr)
Test("overlink_04")

#
# Test ANNOTATION_INT objects and read options
#
TestSection("ANNOTATION_INT nodelists")
DeleteAllPlots()
CloseDatabase("../data/overlink_test_data/regrovl_qh_1000_10001_4/OvlTop.silo")

# First test open with search for ANNOT_INT objects turned on
# but none actually in the database.
readOptions=GetDefaultFileOpenOptions("Silo")
readOptions["Search For ANNOTATION_INT (!!Slow!!)"] = 1
SetDefaultFileOpenOptions("Silo", readOptions)
OpenDatabase("../data/silo_%s_test_data/multipart_multi_ucd3d.silo"%SILO_MODE)
AddPlot("Pseudocolor","d")
DrawPlots()
ResetView()
Test("overlink_05")

# likewise on single domain, single file
DeleteAllPlots()
CloseDatabase("../data/silo_%s_test_data/multipart_multi_ucd3d.silo"%SILO_MODE)
OpenDatabase("../data/silo_%s_test_data/globe.silo"%SILO_MODE)
AddPlot("Pseudocolor","dx")
DrawPlots()
ResetView()
Test("overlink_06")
DeleteAllPlots()
CloseDatabase("../data/silo_%s_test_data/globe.silo"%SILO_MODE)

# Ok, now lets do some real annot_int work
DeleteAllPlots()
CloseDatabase("../data/silo_%s_test_data/globe.silo"%SILO_MODE)
OpenDatabase("../data/overlink_test_data/annotInt/sweptCellTagTest.silo")
AddPlot("Mesh","MMESH")
DrawPlots()
Test("overlink_07")

silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "negYNodes":
        silr.TurnOffSet(i)
        break
SetPlotSILRestriction(silr)
Test("overlink_08")

for i in range(silr.NumSets()):
    if silr.SetName(i) == "negZNodes":
        silr.TurnOffSet(i)
        break
SetPlotSILRestriction(silr)
Test("overlink_09")

DeleteAllPlots()
CloseDatabase("../data/overlink_test_data/annotInt/sweptCellTagTest.silo")
OpenDatabase("../data/overlink_test_data/annotInt/overlink2dTest.silo")
AddPlot("Mesh","MMESH")
DrawPlots()
ResetView()
v = GetView2D()
v.windowCoords = (-0.360608, 6.36061, -0.115684, 6.11568)
SetView2D(v)
silr = SILRestriction()
for i in range(silr.NumSets()):
    if silr.SetName(i) == "posR":
        silr.TurnOffSet(i)
        break
SetPlotSILRestriction(silr)
Test("overlink_10")

TestSection("Large number of nodelists")
DeleteAllPlots()
CloseDatabase("../data/overlink_test_data/annotInt/overlink2dTest.silo")
OpenDatabase("../data/overlink_test_data/annotInt/overlink2dTstA.silo")
AddPlot("Mesh","MMESH")
DrawPlots()
silr = SILRestriction()
for i in range(silr.NumSets()):
    if (i%2 and silr.SetName(i)[0:5] == "cell_"):
        silr.TurnOffSet(i)
SetPlotSILRestriction(silr)
Test("overlink_11")

Exit()
