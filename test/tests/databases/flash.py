# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  flash.py 
#
#  Tests:      FLASH data 
#
#  Programmer: Mark C. Miller 
#  Date:       March6, 2007 
#
# ----------------------------------------------------------------------------
import os, string

TurnOffAllAnnotations()

SetTryHarderCyclesTimes(1)
# the following open command doesn't work (#7873)
#OpenDatabase("../data/FLASH_test_data/orbit_hdf5_chk_0* database", 0, "FLASH_1.0")
OpenDatabase("../data/FLASH_test_data/orbit_hdf5_chk_0000", 0, "FLASH_1.0")

AddPlot("Pseudocolor","pden")
AddOperator("Clip")
c = ClipAttributes()
c.funcType = c.Plane
c.plane1Origin = (0.5, 0.5, 0.4)
c.plane1Normal = (0, 0, 1)
SetOperatorOptions(c)
DrawPlots()
v=GetView3D()
v.viewNormal=(-0.707107, 0, 0.707107)
v.viewUp=(0, 1, 0)
SetView3D(v)
Test("flash_01")

Exit()
