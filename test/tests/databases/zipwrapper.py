# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  zipwrapper.py 
#
#  Tests:      STSD operation (VTK) 
#              STMD operation (Silo)
#              .visit file and .* database
#              Operation over time
#              Cache pre-emption works correctly 
#              Various plots/SIL Restrictions
#
#  Programmer: Mark C. Miller 
#  Date:       August 7, 2007 
#
# ----------------------------------------------------------------------------
TurnOffAllAnnotations() # defines global object 'a'

#
# Create a database of compressed VTK files in a .visit file
#
OpenDatabase("../data/multi_ucd3d.silo")
AddPlot("Pseudocolor","d")
DrawPlots()
e = ExportDBAttributes()
e.db_type = "VTK"
e.filename = "multi_ucd"
e.dirname = "../data"
ExportDatabase(e)
DeleteAllPlots()
CloseDatabase("../data/multi_ucd3d.silo")

cwd = os.getcwd()

os.chdir("../data")
visitFile = open("multi_ucd.visit", 'w')
visitFile.write("!NBLOCKS 36\n")
for i in range(36):
    os.system("gzip -f multi_ucd.%d.vtk"%i)
    visitFile.write("multi_ucd.%d.vtk.gz\n"%i)
visitFile.close()

#
# Compress a fraction of files from hist_ucd
#
for i in range(5,10):
    os.system("cp hist_ucd3d_%04d histz_%04d.silo ; gzip -f histz_%04d.silo"%(i,i,i))

os.chdir(cwd)

#
# Ok, before we start processing any compressed files, set
# ZipWrapper default read options 
#
readOptions=GetDefaultFileOpenOptions("ZipWrapper")
readOptions["Max. # decompressed files"] = 10
SetDefaultFileOpenOptions("ZipWrapper", readOptions)


# Test 36 block VTK (only 10 files decompressed at any one time)
OpenDatabase("../data/multi_ucd.visit")
AddPlot("Pseudocolor","d")
DrawPlots()
v=GetView3D()
v.viewNormal=(-0.5, 0.296198, 0.813798)
SetView3D(v)
Test("ZipWrapper_01")
DeleteAllPlots()
CloseDatabase("../data/multi_ucd.visit")

OpenDatabase("../data/histz_*.silo.gz database", 3)
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

OpenDatabase("../data/sphere.2d.hdf5.gz")
AddPlot("FilledBoundary","materials")
DrawPlots()
Test("ZipWrapper_07")

Exit()
