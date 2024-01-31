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
#    Alister Maguire, Thu Dec 19 13:40:07 PST 2019
#    Added a test to make sure correct subrecord offsets are used.
#
#    Alister Maguire, Wed Jul 15 13:38:17 PDT 2020
#    Added test sections and derived variable test.
#
#    Alister Maguire, Wed Mar 24 16:06:33 PDT 2021
#    Added a test for setting/changing the global integration point.
#
#    Alister Maguire, Fri Mar 26 10:25:08 PDT 2021
#    Added more tests for derived variables (stress, strain, sand mesh).
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("Mili")
single_domain_path = data_path("mili_test_data/single_proc/")
multi_domain_path  = data_path("mili_test_data/multi_proc/")

def TestComponentVis():
    TestSection("Vector components")
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
    TestSection("Shared element sets")
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
    TestSection("Non-shared element sets")
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
    TestSection("Material variables")
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
    TestSection("Tensors")
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
    TestSection("Vectors")
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
    TestSection("Sand mesh")
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
    #
    # The tests need to be in this order to work around a bug in
    # the filled boundary plot with getting colors from a database
    # that causes the colors to be set the same for subsequent
    # filled boundary plots where the colors are not set in the
    # database.
    #
    TestSection("Materials")
    OpenDatabase(single_domain_path + "/sslide14ball_l.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(12)

    AddPlot("FilledBoundary", "materials1")
    DrawPlots()
    Test("mili_materials_00")

    DeleteAllPlots()

    OpenDatabase(single_domain_path + "/d3samp6.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(70)

    AddPlot("FilledBoundary", "materials1(mesh1)")
    DrawPlots()
    Test("mili_materials_01")

    DeleteAllPlots()

def TestMultiDomain():
    TestSection("Multi-domain")
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
    TestSection("Particles")
    OpenDatabase(single_domain_path + "/sslide14ball_l.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(0)

    AddPlot("Pseudocolor", "Primal/particle/stress/sxy")
    DrawPlots()
    Test("mili_particle_01")
    DeleteAllPlots()


def TestStaticNodes():
    TestSection("Static nodes")
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
    TestSection("Labels")
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
    TestSection("Sci notation read")
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


def TestMultiSubrecRead():
    TestSection("Multi-subrecord read")
    #
    # This tests a bug fix that occurred when loading variables
    # that span several subrecords at different offsets.
    #
    OpenDatabase(single_domain_path + "/test4_0.15.plt.mili")
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)

    AddPlot("Pseudocolor", "Primal/brick/stress/sx")
    DrawPlots()
    Test("mili_subrec_offset")
    DeleteAllPlots()


def TestDerivedVariables():
    TestSection("Derived variables")
    OpenDatabase(single_domain_path + "/m_plot.mili")
    v = GetView3D()
    v.viewNormal = (0.49, 0.19, 0.85)
    SetView3D(v)

    SetTimeSliderState(85)

    #
    # Node centered derivations.
    #
    AddPlot("Pseudocolor", "Derived/node/displacement/dispx")
    DrawPlots()
    Test("mili_derived_00")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/node/displacement/dispy")
    DrawPlots()
    Test("mili_derived_01")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/node/displacement/dispz")
    DrawPlots()
    Test("mili_derived_02")
    DeleteAllPlots()

    AddPlot("Vector", "Derived/node/displacement")
    DrawPlots()
    Test("mili_derived_03")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "sand_mesh/Derived/node/displacement/dispz")
    DrawPlots()
    Test("mili_derived_04")
    DeleteAllPlots()

    #
    # Stress/strain derivations. In m_plot, only stress comes from
    # the database, and we derive strain and all of its derivations
    # using VisIt expressions.
    #
    AddPlot("Pseudocolor", "Derived/brick/stress/prin_stress/2")
    DrawPlots()
    Test("mili_derived_05")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/brick/stress/pressure")
    DrawPlots()
    Test("mili_derived_06")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/Shared/strain/almansi/max_shear_almansi")
    DrawPlots()
    Test("mili_derived_07")
    DeleteAllPlots()

    AddPlot("Pseudocolor",
        "Derived/Shared/strain/green_lagrange/prin_dev_green_lagrange/2")
    DrawPlots()
    Test("mili_derived_08")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/Shared/strain/infinitesimal/x")
    DrawPlots()
    Test("mili_derived_09")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/Shared/strain/rate/yz")
    DrawPlots()
    Test("mili_derived_10")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "sand_mesh/Derived/Shared/strain/rate/yz")
    DrawPlots()
    Test("mili_derived_11")
    DeleteAllPlots()

    AddPlot("Tensor", "Derived/Shared/strain/green_lagrange")
    DrawPlots()
    Test("mili_derived_12")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/Shared/relative_volume")
    DrawPlots()
    Test("mili_derived_13")
    DeleteAllPlots()

    AddPlot("Pseudocolor", "Derived/Shared/volumetric_strain")
    DrawPlots()
    Test("mili_derived_14")
    DeleteAllPlots()


def TestGlobalIntegrationPoint():
    TestSection("Global integration point")

    db_path = single_domain_path + "/d3samp6.plt.mili"
    OpenDatabase(db_path)
    v = GetView3D()
    v.viewNormal = (0.9, 0.35, -0.88)
    SetView3D(v)
    SetTimeSliderState(90)

    AddPlot("Pseudocolor", "Primal/Shared/strain/exy")
    DrawPlots()

    mili_options = GetDefaultFileOpenOptions("Mili")
    mili_options["Global integration point"] = "Inner"
    SetDefaultFileOpenOptions("Mili", mili_options)
    ReOpenDatabase(db_path)
    Test("mili_global_ip_00")

    mili_options = GetDefaultFileOpenOptions("Mili")
    mili_options["Global integration point"] = "Middle"
    SetDefaultFileOpenOptions("Mili", mili_options)
    ReOpenDatabase(db_path)
    Test("mili_global_ip_01")

    mili_options = GetDefaultFileOpenOptions("Mili")
    mili_options["Global integration point"] = "Outer"
    SetDefaultFileOpenOptions("Mili", mili_options)
    ReOpenDatabase(db_path)
    Test("mili_global_ip_02")

    DeleteAllPlots()
    CloseDatabase(db_path)


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
    TestMultiSubrecRead()
    TestDerivedVariables()
    TestGlobalIntegrationPoint()

Main()
Exit()

