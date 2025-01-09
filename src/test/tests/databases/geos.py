# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  geos.py 
#
#  Programmer: Kathleen Biagas 
#  Date:       January 7, 2025
#
#  Modifications:
#
# ----------------------------------------------------------------------------


def TestMPMdata():
    OpenDatabase(data_path("geos_test_data/mpm_moving_through_partition/vtkOutput.pvd"))

    AddPlot("Mesh", "mesh")
    AddPlot("Pseudocolor", "particleDensity")

    DrawPlots()
    Test("geos_mpm_01")

    TimeSliderNextState()
    Test("geos_mpm_02")
    TimeSliderNextState()
    Test("geos_mpm_03")

    SetActivePlots(1)
    DeleteActivePlots()

    AddPlot("Subset", "groups")
    DrawPlots()
    Test("geos_mpm_subset_groups_01")

    silr = SILRestriction()
    silr.TurnOffAll()
    for silSet in (7,8):
        silr.TurnOnSet(silSet)
    SetPlotSILRestriction(silr ,0)

    Test("geos_mpm_subset_groups_02")

    ChangeActivePlotsVar("blocks")

    # needed due to bug #20180
    defaultSubsetAtts = SubsetAttributes()
    #

    s = SubsetAttributes()
    s.SetMultiColor(0,(255,0,0,68))
    s.SetMultiColor(1,(0,255,0,68))
    s.SetMultiColor(2,(0,0,255,68))
    s.SetMultiColor(3,(0,255,255,68))
    SetPlotOptions(s)

    silr.TurnOnAll()
    SetPlotSILRestriction(silr ,0)

    DrawPlots()
    Test("geos_mpm_subset_blocks_01")

    silr.TurnOffSet(3)
    silr.TurnOffSet(4)
    silr.TurnOffSet(5)
    silr.TurnOffSet(6)
    SetPlotSILRestriction(silr,0)

    Test("geos_mpm_subset_blocks_02")

    # reset Subset attributes to default so the next test isn't affected
    #  by atts set here
    # (needed until bug #20180 is fixed)
    SetDefaultPlotOptions(defaultSubsetAtts)
    #

    DeleteAllPlots()
    CloseDatabase(data_path("geos_test_data/mpm_moving_through_partition/vtkOutput.pvd"))

    # test opening the grouped *.vtm files
    # If it doesn't get opened as 'geos' flavor,
    # the number of scalars will be wrong
    md = GetMetaData(data_path("geos_test_data/mpm_moving_through_partition/vtkOutput.pvd"))
    TestValueEQ("geos_mpm_grouped_vtm_num_scalars", md.GetNumScalars(), 18)
    CloseDatabase(data_path("geos_test_data/mpm_moving_through_partition/vtkOutput.pvd"))



def TestTHMdata():
    OpenDatabase(data_path("geos_test_data/thm_single_phase_with_fractures/fractureMatrixThermalFlow_conforming.pvd"))

    AddPlot("Mesh", "mesh")
    AddPlot("Pseudocolor", "pressure")
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (-0.55, 0.13, 0.83)
    SetView3D(v)

    Test("geos_thm_01")

    # turn off the CellElementRegion (RockMatrix)
    silr = SILRestriction()
    silr.TurnOffSet(3)
    SetPlotSILRestriction(silr,0)
    Test("geos_thm_02")

    TimeSliderNextState()   
    Test("geos_thm_03")

    SetActivePlots(1)
    DeleteActivePlots()

    AddPlot("Subset", "groups")
    DrawPlots()

    Test("geos_thm_subset_groups_01")

    s = SubsetAttributes()
    s.SetMultiColor(0,(255,0,0,20))
    SetPlotOptions(s)
    Test("geos_thm_subset_groups_02")

    silr.TurnOnSet(3)
    SetPlotSILRestriction(silr,0)

    SetActivePlots(1)
    DeleteActivePlots()

    m = MeshAttributes()
    m.opacity = 0.25
    SetPlotOptions(m)

    AddPlot("Vector", "elementCenter")
    vec = VectorAttributes()
    vec.headSize=0.5
    vec.lineWidth=2
    SetPlotOptions(vec)
    DrawPlots()

    Test("geos_thm_vector_01")

    silr.TurnOffSet(3)
    SetPlotSILRestriction(silr,0)
    Test("geos_thm_vector_02")

    DeleteAllPlots()
    CloseDatabase(data_path("geos_test_data/thm_single_phase_with_fractures/fractureMatrixThermalFlow_conforming.pvd"))

    # test opening the grouped *.vtm files
    # If it doesn't get opened as 'geos' flavor,
    # the number of scalars will be wrong
    md = GetMetaData(data_path("geos_test_data/thm_single_phase_with_fractures/fractureMatrixThermalFlow_conforming/*.vtm database"))
    TestValueEQ("geos_mpm_grouped_vtm_num_scalars", md.GetNumScalars(), 24)
    CloseDatabase(data_path("geos_test_data/thm_single_phase_with_fractures/fractureMatrixThermalFlow_conforming/*.vtm database"))


TestMPMdata()
TestTHMdata()

Exit()
