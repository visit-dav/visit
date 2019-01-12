# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  curve_expressions.py
#
#  Programmer: Kathleen Bonnell 
#  Date:       November 18, 2008
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def TurnOffCurveLabels():
    c = CurveAttributes()
    c.showLabels = 0
    SetDefaultPlotOptions(c)

def DoBinaryOp(db1, curve1, db2, curve2, op, index):
    # name = db:curve1 op curve_cmfe(<db2:curve2>, <db1, curve1>)
    var1 = "<%s>" % (curve1)
    var2 = "<%s:%s>" % (db2, curve2)
    cmfe = "curve_cmfe(%s,%s)" % (var2, var1)
    name = "<A %s B>" %(op)
    DefineCurveExpression(name, "%s %s %s" %(var1, op, cmfe))

    AddPlot("Curve", name);
    DrawPlots()

    Test("curve_exp_bin_%02d" %index)
    DeleteAllPlots()

def TestBinary():
    binops = ("+", "-", "*", "/")
    cdb = cmfe_data_path("curve_test_data/distribution.ultra")
    db = data_path("curve_test_data/distribution.ultra")
    logNormal = "Log Normal Distribution"
    exponential = "Exponential Distribution"

    TestSection("binary operations on curves")
    OpenDatabase(db)
    for i in range(4):
        DoBinaryOp(cdb, logNormal, cdb, exponential, binops[i], i)

    CloseDatabase(db)


TurnOffCurveLabels()
TestBinary()
Exit()
