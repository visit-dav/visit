# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  zipwrapper.py 
#
#  Tests:      STSD operation (STL) 
#              STMD operation (Silo)
#              .visit file and .* database
#              Operation over time
#              Cache pre-emption works correctly 
#              Various plots/SIL Restrictions
#
#  Programmer: Mark C. Miller 
#  Date:       August 7, 2007 
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Mark C. Miller, Wed Jun 27 12:25:12 PDT 2012
#    Adjusted to use STL as STSD test because VTK was changed from STSD
#    to STMD. Removed code to generate test data because the test data is
#    now stored as zipwrapper_test_data.tar.gz 
# ----------------------------------------------------------------------------

#
# Ok, before we start processing any compressed files, set
# ZipWrapper default read options 
#
OpenMDServer("localhost")
readOptions=GetDefaultFileOpenOptions("ZipWrapper")
readOptions["Max. # decompressed files"] = 10
SetDefaultFileOpenOptions("ZipWrapper", readOptions)


# Test 36 block STL (only 10 files decompressed at any one time)
OpenDatabase(data_path("zipwrapper_test_data/multi_ucd.visit"))

AddPlot("Pseudocolor","mesh_quality/condition")
DrawPlots()
v=GetView3D()
v.viewNormal=(-0.5, 0.296198, 0.813798)
SetView3D(v)
Test("ZipWrapper_01")
DeleteAllPlots()
CloseDatabase(data_path("zipwrapper_test_data/multi_ucd.visit"))

OpenDatabase(data_path("zipwrapper_test_data/histz_*.silo.gz database"), 3)

AddPlot("Pseudocolor","d_dup")
DrawPlots()
v=GetView3D()
v.viewNormal=(-0.5, 0.296198, 0.813798)
SetView3D(v)
Test("ZipWrapper_02")
TimeSliderNextState()
TimeSliderNextState()
Test("ZipWrapper_03")
TimeSliderSetState(0)
Test("ZipWrapper_04")
TimeSliderNextState()
Test("ZipWrapper_05")
DeleteAllPlots()

AddPlot("FilledBoundary","mat1")
DrawPlots()
silr=SILRestriction()
silr.TurnOffSet(silr.SetsInCategory("mat1")[0])
SetPlotSILRestriction(silr)
Test("ZipWrapper_06")
DeleteAllPlots()

OpenDatabase(data_path("zipwrapper_test_data/sphere.2d.hdf5.gz"))

AddPlot("FilledBoundary","materials")
DrawPlots()
Test("ZipWrapper_07")

Exit()
