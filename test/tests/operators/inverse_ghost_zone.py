# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  inverse_ghost_zone.py
#
#  Defect ID:  '6724
#
#  Programmer: Hank Childs
#  Date:       March 3, 2006
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to switch between Silo's HDF5 and PDB data.
#
#    Hank Childs, Sun Jan  2 10:45:22 PST 2011
#    Add tests for new features.
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("bigsil.silo"))

AddPlot("Pseudocolor", "dist")
# By default, we won't create ghost zones, because we don't need them
# for this case.  Do something that makes us need them.
pc = PseudocolorAttributes()
pc.centering = pc.Zonal
SetPlotOptions(pc)
AddOperator("InverseGhostZone")
DrawPlots()
Test("ops_inverse_ghost_zone01")

sil = SILRestriction()
sil.TurnOffSet(1)
SetPlotSILRestriction(sil)
Test("ops_inverse_ghost_zone02")

DeleteAllPlots()
OpenDatabase(silo_data_path("multi_ucd3d.silo"))

AddPlot("Pseudocolor", "d")
AddOperator("InverseGhostZone")
DrawPlots()
Test("ops_inverse_ghost_zone03")

sil = SILRestriction()
sil.TurnOffSet(1)
SetPlotSILRestriction(sil)
Test("ops_inverse_ghost_zone04")

DeleteAllPlots()

OpenDatabase(data_path("boxlib_test_data/2D/plt0822/Header"))

AddPlot("Pseudocolor", "density")
AddOperator("InverseGhostZone")
i = InverseGhostZoneAttributes()
i.requestGhostZones = 1
i.showDuplicated = 1
i.showEnhancedConnectivity = 0
i.showReducedConnectivity = 0
i.showAMRRefined = 0
i.showExterior = 0
i.showNotApplicable = 0
SetOperatorOptions(i)
DrawPlots()
Test("ops_inverse_ghost_zone05")

i.showDuplicated = 0
i.showAMRRefined = 1
SetOperatorOptions(i)
Test("ops_inverse_ghost_zone06")

Exit()
