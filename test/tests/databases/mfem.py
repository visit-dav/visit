# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mfem.py 
#
#  Tests:      mfem files 
#
#  Programmer: Cyrus Harrison
#  Date:       Thu Jun 12 14:58:03 PDT 2014
#
# ----------------------------------------------------------------------------


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
    for mres in [0,10,15,20]:
        mc_atts  = MultiresControlAttributes()
        mc_atts.resolution = mres
        SetOperatorOptions(mc_atts)
        DrawPlots()
        Test("input_mesh_%s_mres_%04d" % (base,mres))

TestSection("ex01 results")
for f in ex01_results:
    base = os.path.splitext(os.path.basename(f))[0]
    DeleteAllPlots()
    OpenDatabase(f)
    AddPlot("Pseudocolor","gf")
    #AddPlot("Mesh","main")
    DrawPlots()
    Test("ex01_%s" % (base))


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


Exit()
