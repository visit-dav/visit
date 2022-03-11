# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ddf.py
#
#  Defect ID:  '5203
#
#  Programmer: Hank Childs
#  Date:       February 20, 2006
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Tue Feb 23 12:40:36 PST 2021
#    Added spatial collapse with ghost zones test.
#
#    Kathleen Biagas, Thu Feb 17 07:38:01 PST 2022
#    Replace old ConstructDDFAttributes attribute names for new:
#    ddfName -> name, ranges -> binBoundaries, numSamples -> numBins,
#    statisticalOperator -> reductionOperator,
#    codomainName -> varForReductionOperator.
#
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("curv2d.silo"))

AddPlot("Pseudocolor", "d")
DrawPlots()

t = ConstructDDFAttributes()
t.name = "ddf1"
t.varnames = ("u")
t.binBoundaries = (-1, 1)
t.numBins = (4)
t.varForReductionOperator = "u"
t.reductionOperator = t.Average
ConstructDDF(t)

DefineScalarExpression("e1", "u - apply_ddf(curvmesh2d, ddf1)")
ChangeActivePlotsVar("e1")
Test("ddf_01")

t.name = "ddf2"
t.reductionOperator = t.Maximum
t.varForReductionOperator = "v"
t.varnames = ("v")
ConstructDDF(t)
DefineScalarExpression("e2", "v - apply_ddf(curvmesh2d, ddf2)")
ChangeActivePlotsVar("e2")
Test("ddf_02")

t.name = "ddf3"
t.varnames = ("u", "v")
t.binBoundaries = (-1, 1, -1, 1)
t.numBins = (25, 25)
t.varForReductionOperator = "u"
t.reductionOperator = t.Minimum
ConstructDDF(t)

DefineScalarExpression("e3", "u - apply_ddf(curvmesh2d, ddf3)")
ChangeActivePlotsVar("e3")
Test("ddf_03")


ChangeActivePlotsVar("u")
t.name = "ddf4"
t.varnames = ("u", "v")
t.binBoundaries = (-1, 1, -1, 1)
t.numBins = (25, 25)
t.varForReductionOperator = "u"
t.reductionOperator = t.RMS
ConstructDDF(t)

DefineScalarExpression("e4", "apply_ddf(curvmesh2d, ddf4)")
ChangeActivePlotsVar("e4")
Test("ddf_04")


#
# DDF Spatial Collapse with ghost zones
#

def ddf(atts,var_name,ddf_op):
    # ddf helper used in the wild to normalize var
    # and file names
    ddf_op_map = {"avg": atts.Average,
                  "min": atts.Minimum,
                  "max": atts.Maximum,
                  "stddev": atts.StandardDeviation,
                  "var":    atts.Variance,
                  "sum":    atts.Sum,
                  "count":  atts.Count,
                  "rms": atts.RMS,
                  "pdf": atts.PDF}
    atts.reductionOperator = ddf_op_map[ddf_op]
    visit.ConstructDDF(atts)
    ndims = len(atts.numBins)
    ddf_oname = "%s_%s_%dd" % (var_name,ddf_op,ndims)
    if len(atts.numBins) == 1:
        src_fname = "%s.ultra" % atts.name
        des_fname = "%s.ult" % (atts.name)
        os.rename(src_fname,des_fname)
        lines = open(des_fname).readlines()
        f     = open(des_fname, "w")
        f.write("# %s\n" % (ddf_oname))
        for l in lines[1:]:
            f.write(l)
        f.close()
    else:
        src_fname = "%s.vtk" % atts.name
        des_fname = src_fname
        orig_vtk_var = "SCALARS %s float" % var_name
        ddf_vtk_var  = "SCALARS %s float" % ddf_oname
        data = open(des_fname).read()
        f = open(des_fname, "w")
        data = data.replace(orig_vtk_var,ddf_vtk_var)
        f.write(data)
    print("[ddf output: %s]" % des_fname)
    return des_fname

#################################################
# bigsil has ghost zones, this test makes
# sure we are taking care of ghost zones property
# in ddfs
# use ddf to sum collapse spatially in 1d, then 2d
def ddf_collapse_test():
    TestSection("DDF Spatial Collapse with Ghost Zones")
    DeleteAllPlots()
    OpenDatabase(silo_data_path("bigsil.silo"))
    DefineScalarExpression("mesh_x_nodal","coord(mesh)[0]")
    DefineScalarExpression("mesh_y_nodal","coord(mesh)[1]")
    DefineScalarExpression("mesh_z_nodal","coord(mesh)[2]")
    DefineScalarExpression("mesh_x_zonal","recenter(coord(mesh)[0])")
    DefineScalarExpression("mesh_y_zonal","recenter(coord(mesh)[1])")
    DefineScalarExpression("mesh_z_zonal","recenter(coord(mesh)[2])")
    AddPlot("Pseudocolor","dist")
    DrawPlots()
    atts = visit.ConstructDDFAttributes()
    atts.name = "ddf_dist_1d"
    atts.varForReductionOperator = "dist"
    atts.varnames = ("mesh_x_zonal",)
    atts.binBoundaries = (0,1)
    atts.numBins = (21,)
    ddf(atts,"dist","sum")
    atts = visit.ConstructDDFAttributes()
    atts.name = "ddf_dist_2d"
    atts.varForReductionOperator = "dist"
    atts.varnames = ("mesh_x_zonal", "mesh_y_zonal")
    atts.binBoundaries = (0,1, 0,1)
    atts.numBins = (21,21)
    ddf(atts,"dist","sum")
    # plot 1d result
    DeleteAllPlots()
    OpenDatabase("ddf_dist_1d.ult")
    AddPlot("Curve","dist_sum_1d")
    DrawPlots()
    Test("ddf_spatial_1d")
    # plot 2d result
    DeleteAllPlots()
    OpenDatabase("ddf_dist_2d.vtk")
    AddPlot("Pseudocolor","dist_sum_2d")
    DrawPlots()
    Test("ddf_spatial_2d")

ddf_collapse_test()


Exit()
