# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  arbpoly.py
#
#  Tests:      mesh      - a ucd mesh with arbitrary polyhedral zones
#              plots     - pc, contour, mat subset, mesh
#              operators - OnionPeel
#              selection - material
#
#  Defect ID:  none
#
#  Programmer: Mark C. Miller
#  Date:       October 24, 2004
#
#  Modifications
#    Mark C. Miller, Tue Oct 20 16:30:31 PDT 2009
#    Added tests for fully arbitrary mesh
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Aug 26 08:55:04 PDT 2010
#    Update set index after SIL generation changes.
#
#    Kathleen Biagas, Wed Sep 10 14:15:44 PDT 2014
#    Added tests for OnionPeel'd arb poly, and a single-zone arb-poly set.
#
#    Kathleen Biagas, Wed Feb 16 09:15:45 PST 2022
#    Replace use of meshatts 'backgroundFlag' with opaqueColorSource.
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("poly3d.silo"))


AddPlot("Mesh", "ucdmesh3d")
DrawPlots()

v = GetView3D()
v.SetViewNormal(-0.0356529, 0.010858, 0.999305)
v.SetViewUp(0.0100239, 0.999895, -0.0105068)
v.SetImageZoom(2.75)
SetView3D(v)

Test("arbpoly_01")

#
# See if we can handle a zone centered variable correctly
#
AddPlot("Pseudocolor", "d")
DrawPlots()
Test("arbpoly_02")

DeleteActivePlots()

#
# See if we can handle a material correctly
# Turn off alternate materials.
AddPlot("FilledBoundary", "mat1")
silr=SILRestriction()
silr.TurnOffSet(2)
SetPlotSILRestriction(silr)
DrawPlots()

Test("arbpoly_03")

DeleteAllPlots()
CloseDatabase(silo_data_path("poly3d.silo"))


OpenDatabase(data_path("overlink_test_data/ev_0_0_100/OvlTop.silo"))

AddPlot("Mesh","MMESH")
DrawPlots()
ma = MeshAttributes()
ma.opaqueColor = (0, 200, 200, 255)
ma.opaqueMode = ma.On
ma.opaqueColorSource = ma.OpaqueCustom
SetPlotOptions(ma)
ResetView()
Test("arbpoly_04")
v = GetView3D()
v.SetViewNormal(-0.433013, 0.5, 0.75)
v.SetViewUp(0.25, 0.866025, -0.433013)
SetView3D(v)
Test("arbpoly_05")
ma.opaqueMode = ma.Off
SetPlotOptions(ma)

AddPlot("Pseudocolor","nvar0")
DrawPlots()
Test("arbpoly_06")
DeleteActivePlots()

AddPlot("Pseudocolor","zvar3")
DrawPlots()
Test("arbpoly_07")

DeleteActivePlots()
ma.lineWidth = 3
SetPlotOptions(ma)
AddPlot("FilledBoundary", "MMATERIAL")
AddPlot("Subset", "domains")
AddOperator("OnionPeel", 1)
op = OnionPeelAttributes()
op.index = 99
SetOperatorOptions(op,0,1)
DrawPlots()
Test("arbpoly_08")
DeleteAllPlots()
CloseDatabase(data_path("overlink_test_data/ev_0_0_100/OvlTop.silo"))

OpenDatabase(silo_data_path("arbpoly.silo"))
AddPlot("Mesh", "clipped_hex")
ma.opaqueMode = ma.On
SetPlotOptions(ma)
DrawPlots()
Test("arbpoly_09")


Exit()
