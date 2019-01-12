# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  imgvol.py 
#
#  Tests:      imgvol reader
#
#  Mark C. Miller, Fri Apr 23 23:01:06 PDT 2010
#
#  Modifications:
# 
#    Mark C. Miller, Sun Apr 25 21:30:29 PDT 2010
#    Added missing Slice operator for contour plot.
# ----------------------------------------------------------------------------

OpenDatabase(data_path("imgvol_test_data/tcup.imgvol"))

AddPlot("Pseudocolor","intensity")
sa=SliceAttributes()
sa.originType = sa.Percent
sa.originPercent = 50 
AddOperator("Slice")
SetOperatorOptions(sa)
DrawPlots()
Test("imgvol_01")

ChangeActivePlotsVar("intensity_nodal")
DrawPlots()
Test("imgvol_02")

AddOperator("InverseGhostZone")
DrawPlots()
Test("imgvol_03")
RemoveLastOperator()

AddPlot("Contour","intensity")
AddOperator("Slice")
SetOperatorOptions(sa)
ca=ContourAttributes()
ca.contourValue = (65,)
ca.contourMethod = ca.Value
ca.lineWidth = 2
SetPlotOptions(ca)
DrawPlots()
Test("imgvol_04")

ChangeActivePlotsVar("intensity")
DrawPlots()
Test("imgvol_05")

DeleteActivePlots()
RemoveLastOperator()

iv=IsovolumeAttributes()
iv.lbound = 65 
AddOperator("Isovolume")
SetOperatorOptions(iv)
AddOperator("Slice")
SetOperatorOptions(sa)
DrawPlots()
Test("imgvol_06")

ChangeActivePlotsVar("intensity_nodal")
Test("imgvol_07")

Exit()
