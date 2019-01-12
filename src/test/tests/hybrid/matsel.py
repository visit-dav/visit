# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  matsel.py
#
#  Tests:      plots     - contour,pseudocolor
#
#  Defect ID:  '4353
#
#  Programmer: Jeremy Meredith
#  Date:       June 24, 2004
#
#    Jeremy Meredith, Tue Jul 13 18:03:11 PDT 2004
#    Added curvilinear case.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------



#
# Do a test of 2D nodal interpolation on material-selected rectilinear meshes
#
OpenDatabase(silo_data_path("rect2d.silo"))


AddPlot("Pseudocolor", "u");

sil = SILRestriction()
sets = sil.SetsInCategory("mat1")
sil.TurnOnAll()
sil.TurnOffSet(sets[4])
SetPlotSILRestriction(sil)
DrawPlots()

Test("hybrid_matsel_01")

DeleteAllPlots();

#
# Do a test of 3D nodal interpolation on material-selected rectilinear meshes
# Ths is easiest to see when doing a contour plot of a matsel variable
#
OpenDatabase(silo_data_path("rect3d.silo"))


AddPlot("Contour", "u");

sil = SILRestriction()
sets = sil.SetsInCategory("mat1")
sil.TurnOnAll()
sil.TurnOffSet(sets[4])
SetPlotSILRestriction(sil)
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.624401, -0.368813, -0.688549)
focus = (0.5, 0.5, 0.5)
v.viewUp = (-0.768963, 0.135421, 0.624786)
SetView3D(v)

Test("hybrid_matsel_02")

DeleteAllPlots();

#
# Do a test of 2D nodal interpolation on material-selected curvilinear meshes
#
OpenDatabase(silo_data_path("curv2d.silo"))


AddPlot("Pseudocolor", "u");

sil = SILRestriction()
sets = sil.SetsInCategory("mat1")
sil.TurnOnAll()
sil.TurnOffSet(sets[0])
SetPlotSILRestriction(sil)
DrawPlots()
v = GetView2D()
v.windowCoords = (0.1, 2.0, 2.8, 3.8)
SetView2D(v)
p = PseudocolorAttributes()
p.min = 0
p.max = .5
p.minFlag = 1
p.maxFlag = 1
SetPlotOptions(p)

Test("hybrid_matsel_03")

DeleteAllPlots();

Exit()
