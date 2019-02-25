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
#  Programmer: Alister Maguire
#  Date:       
#
#  Modifications:
#
#    Hank Childs, Fri Oct  7 16:58:05 PDT 2005
#    Test ratio of volumes with material selection (only works on time
#    varying Lagrangian calculations, like those in Mili files).
#
#    Kathleen Bonnell, Wed May  6 17:33:02 PDT 2009
#    Substituted non-existent var "derived/stress/eff_stress" with
#    "derived/eff_stress"
#
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state
#    behavior when an operator is added.
#
#    Cyrus Harrison, Thu Aug 26 14:47:36 PDT 2010
#    Change set ids due to SIL generation change.
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Mili")

def TestComponentVis():
    OpenDatabase("/usr/workspace/wsrzd/maguire7/MiliTestData/single_proc/new/d3samp6new.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)
    
    
    AddPlot("Pseudocolor", "Primal/brick/edrate")
    DrawPlots()
    Test("mili_brick_comp")
    
    ChangeActivePlotsVar("Primal/beam/svec/svec_x")
    Test("mili_beam_comp")
    ChangeActivePlotsVar("Primal/node/nodacc/ax")
    Test("mili_nodacc_comp")
    
    DeleteAllPlots()


def TestElementSetComponents():
    OpenDatabase("/usr/workspace/wsrzd/maguire7/MiliTestData/new/d3samp6new.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Pseudocolor", "Primal/shell/strain/exy")
    DrawPlots()
    Test("mili_es_comp_01")
    ChangeActivePlotsVar("Primal/beam/stress/sz")
    Test("mili_es_comp_02")

    DeleteAllPlots()


def TestMaterialVar():
    OpenDatabase("/usr/workspace/wsrzd/maguire7/MiliTestData/new/d3samp6new.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Pseudocolor", "Primal/mat/matcgy")
    DrawPlots()
    Test("mili_mat_var_01")
    ChangeActivePlotsVar("Primal/mat/matke")
    Test("mili_mat_var_02")
    DeleteAllPlots()


def TestMaterials():
    OpenDatabase("/usr/workspace/wsrzd/maguire7/MiliTestData/new/d3samp6new.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("FilledBoundary", "materials1")
    DrawPlots()
    Test("mili_materials_01")

    DeleteAllPlots()


def TestTensors():
    OpenDatabase("/usr/workspace/wsrzd/maguire7/MiliTestData/new/d3samp6new.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Tensor", "Primal/brick/stress")
    DrawPlots()
    Test("mili_tensors_01")

    ChangeActivePlotsVar("Primal/brick/stress")
    Test("mili_tensors_02")
    
    ChangeActivePlotsVar("Primal/shell/stress")
    Test("mili_tensors_03")
    DeleteAllPlots()


def TestVectors():
    OpenDatabase("/usr/workspace/wsrzd/maguire7/MiliTestData/new/d3samp6new.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Vector", "Primal/node/nodpos")
    DrawPlots()
    Test("mili_vectors_01")
    
    ChangeActivePlotsVar("Primal/shell/bend")
    Test("mili_vectors_02")
    
    ChangeActivePlotsVar("Primal/beam/svec")
    Test("mili_vectors_03")
    DeleteAllPlots()


def Main():
    TestComponentVis()    
    TestElementSetComponents()
    TestMaterialVar()
    TestMaterials()
    TestTensors()
    TestVectors()

Main()
Exit()

