# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  metadata.py
#
#  Tests:      Getting a usable metadata object from the viewer.
#
#  Programmer: Brad Whitlock
#  Date:       Wed Mar 14 16:40:31 PST 2007
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------

def test0():
    TestSection("Getting metadata")

    # Get metadata for various test datasets.
    md = GetMetaData(silo_data_path("noise.silo") )
    TestText("metadata_0_00", str(md))

    # Get the metadata for the first time step
    md = GetMetaData(silo_data_path("wave*.silo database") )
    TestText("metadata_0_01", str(md))

    # Get the metadata for the 17th time step where new variables appear
    md = GetMetaData(silo_data_path("wave*.silo database") , 17)
    TestText("metadata_0_02", str(md))

    # Get something with species and default plots.
    md = GetMetaData(data_path("samrai_test_data/mats-par3/dumps.visit"))
    TestText("metadata_0_03", str(md))

    # Get something with label variables
    md = GetMetaData(data_path("shapefile_test_data/alameda/tgr06001lkA.shp"))
    TestText("metadata_0_04", str(md))

def MakeSafe(varname):
    ret = string.replace(varname, "/", "_")
    ret = string.replace(ret, "(", "[")
    ret = string.replace(ret, ")", "]")
    ret = string.replace(ret, ":", "_")
    ret = string.replace(ret, ",", "_")
    ret = string.replace(ret, ";", "_")
    return ret

def test1():
    TestSection("Use the metadata to plot the variables.")

    db = silo_data_path("noise.silo") 
    OpenDatabase(db)
    md = GetMetaData(db)

    # Test all of the scalars.
    for i in range(md.GetNumScalars()):
        AddPlot("Pseudocolor", md.GetScalars(i).name)
        DrawPlots()
        ResetView()
        testName = "metadata_1_scalar_" + MakeSafe(md.GetScalars(i).name)
        Test(testName)
        DeleteAllPlots()

    # Test all of the vectors.
    for i in range(md.GetNumVectors()):
        AddPlot("Vector", md.GetVectors(i).name)
        DrawPlots()
        ResetView()
        testName = "metadata_1_vector_" + MakeSafe(md.GetVectors(i).name)
        Test(testName)
        DeleteAllPlots()

    # Test all of the expressions
    for i in range(md.exprList.GetNumExpressions()):
        expr = md.exprList.GetExpressions(i)
        if expr.type == expr.ScalarMeshVar:
            AddPlot("Pseudocolor", expr.name)
        elif expr.type == expr.VectorMeshVar:
            AddPlot("Vector", expr.name)
            v = VectorAttributes()
            v.useStride = 1
            v.stride = 1
            SetPlotOptions(v)
        elif expr.type == expr.TensorMeshVar:
            AddPlot("Tensor", expr.name)
            t = TensorAttributes()
            t.useStride = 1
            t.stride = 5
            SetPlotOptions(t)
        elif expr.type == expr.ArrayMeshVar:
            AddPlot("Label", expr.name)
        else:
            print("Add support for plotting expression: ", expr)
        DrawPlots()
        ResetView()
        testName = "metadata_1_expr_" + MakeSafe(expr.name)
        Test(testName)
        DeleteAllPlots()

def main():
    # Draw antialiased lines
    r = GetRenderingAttributes()
    r.antialiasing = 1
    SetRenderingAttributes(r)

    test0()
    test1()

main()
Exit()
