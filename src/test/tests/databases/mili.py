# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mili.py
#
#  Tests:      mesh      - 3D unstructured, multi-domain
#              plots     - Pseudocolor, material, vector, tensor, label
#
#
#  Programmer: Alister Maguire
#  Date:       May 22, 2019
#
#  Modifications:
#
#    Alister Maguire, Mon Aug  5 13:02:05 MST 2019
#    Added a test that handles .mili files containing integers in 
#    scientific notation. 
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Mili")
single_domain_path = data_path("mili_test_data/single_proc/")
multi_domain_path  = data_path("mili_test_data/multi_proc/")

def TestComponentVis():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)
    
    AddPlot("Pseudocolor", "Primal/Shared/edrate")
    DrawPlots()
    Test("mili_brick_comp")
    
    ChangeActivePlotsVar("Primal/beam/svec/svec_x")
    Test("mili_beam_comp")
    ChangeActivePlotsVar("Primal/node/nodacc/ax")
    Test("mili_nodacc_comp")
    
    DeleteAllPlots()


def TestSharedElementSets():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Pseudocolor", "Primal/Shared/strain/exy")
    DrawPlots()
    Test("mili_shared_es_01")
    ChangeActivePlotsVar("Primal/Shared/edrate")
    Test("mili_shared_es_02")
    ChangeActivePlotsVar("Primal/Shared/stress/sy")
    Test("mili_shared_es_03")

    DeleteAllPlots()


def TestNonSharedElementSets():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    # 
    #  eps is a section of an element set that is only 
    #  defined on beams. 
    # 
    AddPlot("Pseudocolor", "Primal/beam/eps")
    DrawPlots()
    Test("mili_non_shared_es_01")
    DeleteAllPlots()


def TestMaterialVar():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
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


def TestTensors():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Tensor", "Primal/Shared/stress")
    DrawPlots()
    Test("mili_tensors_01")

    ChangeActivePlotsVar("Primal/Shared/strain")
    Test("mili_tensors_02")
    DeleteAllPlots()


def TestVectors():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
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


def TestSandMesh():
    OpenDatabase(single_domain_path + "/m_plot.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(101)
    
    # 
    # First, let's look at the sand variable on a non-sanded mesh. 
    # It should be well structured. 
    # 
    AddPlot("Mesh", "mesh1")
    AddPlot("Pseudocolor", "Primal/Shared/sand")
    DrawPlots()
    Test("mili_sand_mesh_01")
    DeleteAllPlots()
    
    #
    # Now let's view the sand mesh. It's a mess. 
    #
    AddPlot("Mesh", "sand_mesh1")
    AddPlot("Pseudocolor", "Primal/Shared/sand")
    DrawPlots()
    Test("mili_sand_mesh_02")

    #
    # Now let's look at sand in its sanded state. 
    #
    ChangeActivePlotsVar("sand_mesh/Primal/Shared/sand")
    DrawPlots()
    Test("mili_sand_mesh_03")

    #
    # We need to make sure that other variables can also be 
    # viewed in their sanded state. 
    #
    ChangeActivePlotsVar("sand_mesh/Primal/shell/stress_mid/sx")
    Test("mili_sand_mesh_04")
    
    DeleteAllPlots()


def TestMaterials():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(70)
    
    AddPlot("FilledBoundary", "materials1(mesh1)")
    DrawPlots()
    Test("mili_materials_00")
    DeleteAllPlots()


def TestMultiDomain():
    OpenDatabase(multi_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Pseudocolor", "Primal/Shared/strain/exy")
    DrawPlots()
    Test("mili_multi_dom_01")
    ChangeActivePlotsVar("Primal/Shared/stress/sz")
    Test("mili_multi_dom_02")

    DeleteAllPlots()


def TestParticles():
    OpenDatabase(single_domain_path + "/sslide14ball_l.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)

    AddPlot("Pseudocolor", "Primal/particle/stress/sxy")
    DrawPlots()
    Test("mili_particle_01")
    DeleteAllPlots()


def TestStaticNodes():
    OpenDatabase(single_domain_path + "/m1_plot.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)

    AddPlot("Mesh", "mesh1")
    AddPlot("Pseudocolor", "Primal/node/temp")
    SetTimeSliderState(10)
    DrawPlots()
    Test("mili_static_nodes_01")
    DeleteAllPlots()


def TestLabels():
    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)
    
    AddPlot("Pseudocolor", "Primal/Shared/edrate")
    AddPlot("Label", "OriginalZoneLabels")
    DrawPlots()
    Test("mili_zone_labels_01")

    DeleteAllPlots()

    AddPlot("Pseudocolor", "Primal/Shared/edrate")
    AddPlot("Label", "OriginalNodeLabels")
    DrawPlots()
    Test("mili_node_labels_01")
    DeleteAllPlots()


def TestSciNotation():
    #
    # Some .mili files contain integers in scientific notation. 
    # These need to be handled appropriately. 
    #
    OpenDatabase(single_domain_path + "/HexModel1.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    
    AddPlot("Pseudocolor", "Primal/brick/stress/sx")
    DrawPlots()
    Test("mili_from_sci_not")
    DeleteAllPlots()


def Main():
    TestComponentVis()    
    TestNonSharedElementSets()
    TestSharedElementSets()
    TestMaterialVar()
    TestTensors()
    TestVectors()
    TestSandMesh()
    TestMaterials()
    TestMultiDomain()
    TestParticles()
    TestStaticNodes()
    TestLabels()
    TestSciNotation()

Main()
Exit()

