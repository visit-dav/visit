# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mili.py
#
#  Tests:      mesh      - 3D unstructured,multi-domain, 
#              plots     - Pseudocolor, mesh
#
#  Defect ID:  none
#
#  Programmer: Hank Childs
#  Date:       March 31, 2005
#
#  Modifications:
#
#    Hank Childs, Fri Oct  7 16:58:05 PDT 2005
#    Test ratio of volumes with material selection (only works on time
#    varying Lagrangian calculations, like those in Mili files).
#
# ----------------------------------------------------------------------------

# Turn off all annotation
TurnOffAllAnnotations() # defines global object 'a'

OpenDatabase("../data/mili_test_data/m_plot.mili")
AddPlot("FilledBoundary", "materials1_no_free_nodes(mesh1_no_free_nodes)")
DrawPlots()

v = GetView3D()
v.viewNormal = (-0.31, 0.35, 0.88)
SetView3D(v)

Test("mili_01")

SetTimeSliderState(101)
v = GetView3D()
v.viewNormal = (0.38, 0.34, 0.85)
v.focus = (14.1, 0, 0)
v.viewUp = (0, 1, 0)
SetView3D(v)
Test("mili_02")

thres = ThresholdAttributes()
thres.lowerBounds = (0.5)
thres.listedVarNames = ("sand")
SetDefaultOperatorOptions(thres)

AddOperator("Threshold")
Test("mili_03")

DeleteAllPlots()
# We have to test a *lot* of scalar quantities, because they correspond to
# a lot of different expressions.  On the plus side, we are getting a lot
# of code coverage out of this.
AddPlot("Pseudocolor", "derived/acceleration/mag")
AddOperator("Threshold")
DrawPlots()
Test("mili_04")

ChangeActivePlotsVar("derived/pressure")
Test("mili_05")
ChangeActivePlotsVar("derived/stress/x")
Test("mili_06")
ChangeActivePlotsVar("derived/stress/z")
Test("mili_07")
ChangeActivePlotsVar("derived/stress/xy")
Test("mili_08")
ChangeActivePlotsVar("derived/stress/yz")
Test("mili_09")
ChangeActivePlotsVar("derived/stress/xz")
Test("mili_10")
ChangeActivePlotsVar("derived/stress/eff_stress")
Test("mili_11")
ChangeActivePlotsVar("derived/prin_dev_stress/1")
Test("mili_12")
ChangeActivePlotsVar("derived/prin_dev_stress/2")
Test("mili_13")
ChangeActivePlotsVar("derived/prin_dev_stress/3")
Test("mili_14")
ChangeActivePlotsVar("derived/max_shear_stress")
Test("mili_15")
ChangeActivePlotsVar("derived/prin_stress/1")
Test("mili_16")
ChangeActivePlotsVar("derived/prin_stress/2")
Test("mili_17")
ChangeActivePlotsVar("derived/prin_stress/3")
Test("mili_18")

DeleteAllPlots()

AddPlot("Mesh", "mesh1")
AddOperator("Threshold")
DrawPlots()
Test("mili_19")

SetTimeSliderState(60)
Test("mili_20")

AddPlot("Vector", "bend")
DrawPlots()
Test("mili_21")

v = VectorAttributes()
v.autoScale = 0
SetPlotOptions(v)
Test("mili_22")

DeleteAllPlots()


AddPlot("Tensor", "stress")
AddOperator("Threshold")
DrawPlots()
Test("mili_23")

t = TensorAttributes()
t.useStride = 1
t.colorTableName = "calewhite"
SetPlotOptions(t)
Test("mili_24")

s = SILRestriction()
s.TurnOffSet(2)
s.TurnOffSet(4)
s.TurnOffSet(5)
SetPlotSILRestriction(s)
Test("mili_25")

DeleteAllPlots()

AddPlot("Mesh", "mesh1")
AddOperator("Threshold")
AddPlot("Mesh", "mesh1_free_nodes")
matts = MeshAttributes()
matts.pointType = matts.Icosahedron
matts.pointSize = 0.5
SetPlotOptions(matts)
DrawPlots()
Test("mili_26")

AddPlot("Vector","free_nodes/nodvel")
DrawPlots()
Test("mili_27")

SetTimeSliderState(90)
Test("mili_28")

DeleteAllPlots()
DefineScalarExpression("ratio", "volume(mesh1_no_free_nodes) / conn_cmfe(volume(<[0]i:mesh1_no_free_nodes>), mesh1_no_free_nodes)")
AddPlot("Pseudocolor", "ratio")
DrawPlots()
Test("mili_29")
sil = SILRestriction()
sil.TurnOffAll()
sil.TurnOnSet(9)
SetPlotSILRestriction(sil)
Test("mili_30")

DeleteAllPlots()
CloseDatabase("../data/mili_test_data/m_plot.mili")

#
# Test a mili database with time-invariant nodal positions
#
OpenDatabase("../data/mili_test_data/m1_plot.mili")
SetTimeSliderState(5)
AddPlot("Pseudocolor","temp")
ResetView()
DrawPlots()
Test("mili_31")
SetTimeSliderState(10)
Test("mili_32")
DeleteAllPlots()
AddPlot("Mesh","mesh1")
DrawPlots()
Test("mili_33")
SetTimeSliderState(6)
Test("mili_34")

Exit()


