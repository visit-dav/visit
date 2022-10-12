# ----------------------------------------------------------------------------
#  CLASSES: nightly
#  LIMIT: 900
#  Test Case:  mfem.py 
#
#  Tests:      mfem files 
#
#  Programmer: Cyrus Harrison
#  Date:       Thu Jun 12 14:58:03 PDT 2014
#
#  Modifications:
#    Brad Whitlock, Wed Sep  2 20:56:13 PDT 2020
#    The MFEM reader now provides original cells so artificial mesh lines
#    get removed. There's less of a point to seeing them here. Keep min/max
#    refinement levels only. 
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("MFEM")

readOptions = GetDefaultFileOpenOptions("MFEM")
readOptions["MFEM LOR Setting"] = "Legacy LOR"
SetDefaultFileOpenOptions("MFEM", readOptions)

mfem_roots  = glob.glob(data_path("mfem_test_data/*.mfem_root"))
input_meshs  = [ f for f in mfem_roots if f.count("ex0") == 0]
ex01_results = [ f for f in mfem_roots if f.count("ex01") == 1]
ex02_results = [ f for f in mfem_roots if f.count("ex02") == 1]

TestSection("Input Mesh Files")
for f in input_meshs:
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    AddPlot("Pseudocolor","element_coloring")
    #pc_atts = PseudocolorAttributes()
    #pc_atts.colorTableName = "PuBuGn"
    #SetPlotOptions(pc_atts)
    AddOperator("MultiresControl")
    AddPlot("Mesh","main")
    AddOperator("MultiresControl")
    SetActivePlots((0,1))
    levels = [0,20]
    if f.count("amr") > 0:
        levels = [0,5]
    for mres in levels:
        mc_atts  = MultiresControlAttributes()
        mc_atts.resolution = mres
        SetOperatorOptions(mc_atts)
        DrawPlots()
        Test("input_mesh_%s_mres_%04d" % (base,mres))
    DeleteAllPlots()
    CloseDatabase(f)

TestSection("ex01 results")
for f in ex01_results:
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    AddPlot("Pseudocolor","gf")
    #AddPlot("Mesh","main")
    DrawPlots()
    Test("ex01_%s" % (base))
    DeleteAllPlots()
    CloseDatabase(f)

TestSection("ex02 results")
for f in ex02_results:
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    AddPlot("Pseudocolor","element_attribute")
    #AddPlot("Mesh","main")
    DrawPlots()
    Test("ex02_element_attribute_%s" % (base))
    ChangeActivePlotsVar("gf_magnitude");
    DrawPlots()
    Test("ex02_gf_mag_%s" % (base))
    DeleteAllPlots()
    CloseDatabase(f)

TestSection("MFEM Expressions")
OpenDatabase(data_path("mfem_test_data/ex02-beam-tet.mfem_root"))
AddPlot("Pseudocolor","mag-gf")
DrawPlots()
Test("mfem_expressions_1")
DeleteAllPlots()
AddPlot("Pseudocolor","comp0")
DrawPlots()
Test("mfem_expressions_2")
DeleteAllPlots()
AddPlot("Vector","curl-gf")
DrawPlots()
Test("mfem_expressions_3")
DeleteAllPlots()
CloseDatabase(data_path("mfem_test_data/ex02-beam-tet.mfem_root"))

Exit()
