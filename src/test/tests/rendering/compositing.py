# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  compositing.py
#
#  Tests: postprocessing of transparent images in SR mode.
#
#  Programmer: Tom Fogal
#  Date:       June 3, 2009
#
# Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

ds_noise = silo_data_path("noise.silo")
OpenDatabase(ds_noise)

ra = GetRenderingAttributes()
ra.scalableActivationMode = ra.Always
ra.doShadowing = 1
SetRenderingAttributes(ra)

AddPlot("Contour", "hardyglobal")

ca = ContourAttributes()
ca.contourNLevels = 1
ca.contourPercent = (0.5, 0.5)
ca.SetMultiColor(0, 255,0,255,128)
SetPlotOptions(ca)

DrawPlots()
Test("compositing_01")

DeleteAllPlots()
CloseDatabase(ds_noise)

Exit()
