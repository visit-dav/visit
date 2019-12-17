from visit_utils import *
import math



def setup_plot():
    DeleteAllPlots()
    OpenDatabase(silo_data_path("rect3d.silo"))
    exprs.define("coords", "coord(quadmesh3d)",etype="vector")
    exprs.define("mesh_x_zonal","recenter(coords[0])")
    exprs.define("mesh_y_zonal","recenter(coords[1])")
    exprs.define("mass","d * volume(quadmesh3d)")
    AddPlot("Pseudocolor","mass")
    DrawPlots()
    

def ddf(opts):
    # work around quirks related to the ddf pipeline expecting
    # vars to already exist
    predraw_vars = [ opts["codomain"]]
    predraw_vars.extend(opts["varnames"])
    
    for v in predraw_vars:
        ChangeActivePlotsVar(v)
    
    atts = visit.ConstructDDFAttributes()
    ddf_op_map = {"avg": atts.Average,
                  "min": atts.Minimum,
                  "max": atts.Maximum,
                  "stddev": atts.StandardDeviation,
                  "var":    atts.Variance,
                  "sum":    atts.Sum,
                  "count":  atts.Count,
                  "rms": atts.RMS,
                  "pdf": atts.PDF}
    atts.ddfName         = opts["name"]
    atts.codomainName    = opts["codomain"]
    atts.varnames   = opts["varnames"]
    atts.ranges     = opts["ranges"]
    atts.numSamples = opts["samples"]
    atts.statisticalOperator = ddf_op_map[opts["op"]]
    visit.ConstructDDF(atts)
    
    
    ndims = len(atts.numSamples)
    ddf_varname = "%s_%s_%dd" % (opts["codomain"],opts["op"],ndims)
    if len(atts.numSamples) == 1:
        src_fname = "%s.ultra" % atts.ddfName
        des_fname = "%s.ult" % (atts.ddfName)
        common.sexe("mv %s %s" % (src_fname, des_fname))
        lines = open(des_fname).readlines()
        f     = open(des_fname, "w")
        f.write("# %s\n" % (ddf_varname))
        for l in lines[1:]:
            f.write(l)
        f.close()
    else:
        ofname = "%s.vtk" % atts.ddfName
        orig_vtk_var = "SCALARS %s float" % opts["codomain"]
        ddf_vtk_var  = "SCALARS %s float" % ddf_varname
        data = open(ofname).read()
        f = open(ofname, "w")
        data = data.replace(orig_vtk_var,ddf_vtk_var)
        f.write(data)
    print("[ddf output: %s]" % ofname)
    return ofname



def test_orig_mass():
    setup_plot()
    Test("ddf_vs_dbinning_input_plot")
    res = query("Variable Sum")
    DeleteAllPlots()
    return res

def test_dbinning_using_coords():
    setup_plot()
    AddOperator("DataBinning")
    datts = DataBinningAttributes()
    datts.numDimensions = datts.Two 
    datts.dim1BinBasedOn = datts.X 
    datts.dim1SpecifyRange = 0
    datts.dim1NumBins = 10
    datts.dim2BinBasedOn = datts.Y 
    datts.dim2SpecifyRange = 0
    datts.dim2NumBins = 10
    datts.outOfBoundsBehavior = datts.Clamp
    datts.reductionOperator = datts.Sum  
    datts.varForReduction = "mass"
    datts.emptyVal = 0
    datts.outputType = datts.OutputOnBins  
    SetOperatorOptions(datts)
    DrawPlots()
    # we have to export b/c we can't query the 
    # result of the operated created expr ...
    ofname  = "dbin_mass_sum_using_coords"
    eatts = ExportDBAttributes()
    eatts.db_type = "VTK"
    eatts.filename = ofname
    ExportDatabase(eatts)
    DeleteAllPlots()    
    dbin_varname = "%s_%s_%dd" % ("mass","sum",2)
    ofname += ".vtk"
    orig_vtk_var = "SCALARS %s float" % "operators/DataBinning"
    ddf_vtk_var  = "SCALARS %s float" % dbin_varname
    data = open(ofname).read()
    f = open(ofname, "w")
    data = data.replace(orig_vtk_var,ddf_vtk_var)
    f.write(data)
    f.close()
    OpenDatabase(ofname)
    AddPlot("Pseudocolor","mass_sum_2d")
    DrawPlots()
    Test("ddf_vs_dbinning_dbin_coords_result")
    res = query("Variable Sum")
    DeleteAllPlots()
    CloseDatabase(ofname)
    return res


def test_dbinning_using_coords_exprs():
    setup_plot()
    AddOperator("DataBinning")
    datts = DataBinningAttributes()
    datts.numDimensions = datts.Two 
    datts.dim1BinBasedOn = datts.Variable
    datts.dim1Var = "mesh_x_zonal"
    datts.dim1SpecifyRange = 0
    datts.dim1NumBins = 10
    datts.dim2BinBasedOn = datts.Variable
    datts.dim2Var = "mesh_y_zonal"
    datts.dim2SpecifyRange = 0
    datts.dim2NumBins = 10
    datts.outOfBoundsBehavior = datts.Clamp
    datts.reductionOperator = datts.Sum  
    datts.varForReduction = "mass"
    datts.emptyVal = 0
    datts.outputType = datts.OutputOnBins  
    SetOperatorOptions(datts)
    DrawPlots()
    # we have to export b/c we can't query the 
    # result of the operated created expr ...
    ofname  = "dbin_mass_sum_using_coords_exprs"
    eatts = ExportDBAttributes()
    eatts.db_type = "VTK"
    eatts.filename = ofname
    ExportDatabase(eatts)
    DeleteAllPlots()    
    dbin_varname = "%s_%s_%dd" % ("mass","sum",2)
    ofname += ".vtk"
    orig_vtk_var = "SCALARS %s float" % "operators/DataBinning"
    ddf_vtk_var  = "SCALARS %s float" % dbin_varname
    data = open(ofname).read()
    f = open(ofname, "w")
    data = data.replace(orig_vtk_var,ddf_vtk_var)
    f.write(data)
    f.close()
    OpenDatabase(ofname)
    AddPlot("Pseudocolor","mass_sum_2d")
    DrawPlots()
    Test("ddf_vs_dbinning_dbin_coords_exprs_result")
    res = query("Variable Sum")
    DeleteAllPlots()
    CloseDatabase(ofname)
    return res



def test_ddf():
    setup_plot()
    ddf_opts = {"name": "ddf_mass_sum",
                "op"  : "sum",
                "codomain" : "mass",
                "varnames" : ("mesh_x_zonal",
                              "mesh_y_zonal"),
                "ranges"   : (0,1, 
                              0,1),
                "samples"  : (10,10)}
    ddf(ddf_opts)
    DeleteAllPlots()
    OpenDatabase("ddf_mass_sum.vtk")
    AddPlot("Pseudocolor","mass_sum_2d")
    DrawPlots()
    Test("ddf_vs_dbinning_ddf_result")
    res = query("Variable Sum")
    DeleteAllPlots()
    CloseDatabase("ddf_mass_sum.vtk")
    return res



orig_val = test_orig_mass()
ddf_val  = test_ddf()
dbin_coords_val = test_dbinning_using_coords()
dbin_cexprs_val = test_dbinning_using_coords_exprs()


TestText("Orig","Mass Sum = %s" % orig_val)
TestText("DDF","Mass Sum = %s" % ddf_val)
TestText("DBIN with Coords","Mass Sum = %s" % dbin_coords_val)
TestText("DBIN with Coords Exprs","Mass Sum = %s" % dbin_cexprs_val)

AssertTrue("Orig Equals DDF",abs(orig_val - ddf_val)  < 1e-4 )
AssertTrue("Orig Equals DBIN with Coords",abs(orig_val - dbin_coords_val) < 1e-4 )
AssertTrue("Orig Equals DBIN with Coords Exprs",abs(orig_val - dbin_cexprs_val) < 1e-4 )

Exit()



    
