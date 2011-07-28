# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  leos.py 
#
#  Tests:      Auto Full Frame Mode
#              LEOSFileReader 
#
#  Programmer: Mark C. Miller 
#  Date:       July 20, 2005 
#
#  Modifications:
#    Tom Fogal, Wed Dec 16 13:12:57 MST 2009
#    Fixed a bad path.
#
#    Mark C. Miller, Fri Jan 22 17:30:29 PST 2010
#    I replaced OpenDatabase with FindAnd... variant to deal with data
#    we don't keep in repo.
# ----------------------------------------------------------------------------
OpenMDServer("localhost")
readOptions=GetDefaultFileOpenOptions("PDB")
readOptions["LEOS try harder level [set to 0, 1 or 2]"] = 1 
SetDefaultFileOpenOptions("PDB", readOptions)

(err, dbname) = FindAndOpenDatabase("sesame.pdb")
if (err != 1): Exit(116) # Indicate skip

#
# Test default full frame (became 'auto' 07Mar06)
#
AddPlot("Pseudocolor","Calcium_Ca/Pt")
DrawPlots()
Test("leos_01")

v=GetView2D()
#
# Test auto full frame. This test was obsoleted when
# auto ff became the default
#
#v.fullFrameActivationMode = v.Auto
#SetView2D(v)
#Test("leos_02")

#
# Test turning off full frame
#
v.fullFrameActivationMode = v.Off
SetView2D(v)
Test("leos_03")

#
# Test turning on full frame 
#
v.fullFrameActivationMode = v.On
SetView2D(v)
Test("leos_04")

v.fullFrameActivationMode = v.Auto
SetView2D(v)
DeleteAllPlots()

#
# Test Auto full frame when changing plots
#
AddPlot("Pseudocolor","Water_H2O/Et")
DrawPlots()
ResetView()
v=GetView2D()
v.fullFrameActivationMode = v.Auto
SetView2D(v)
Test("leos_05")

#
# Test the "catch all" mesh
#
AddPlot("Mesh","for active plot")
DrawPlots()
Test("leos_06")

DeleteAllPlots()
CloseDatabase(dbname)

Exit()
