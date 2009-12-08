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

def DoBinaryOp(db1, curve1, db2, curve2, op, index, textObj):
    # name = db:curve1 op curve_cmfe(<db2:curve2>, <db1, curve1>)
    var1 = "<%s>" % (curve1)
    var2 = "<%s:%s>" % (db2, curve2)
    cmfe = "curve_cmfe(%s,%s)" % (var2, var1)
    name = "<A %s B>" %(op)
    DefineCurveExpression(name, "%s %s %s" %(var1, op, cmfe))

    AddPlot("Curve", name);
    DrawPlots()

    # get the current curve's color to use as the textObject's color
    c = CurveAttributes(1)
    textObj.textColor = c.color
    # Set the textObject's text to the name of the curve
    textObj.text = name[1:6]

    Test("curve_exp_bin_%02d" %index)
    DeleteAllPlots()

def TestBinary():
    binops = ("+", "-", "*", "/")
    db = "../data/curve_test_data/distribution.ultra"
    logNormal = "Log Normal Distribution"
    exponential = "Exponential Distribution"

    TestSection("binary operations on curves")
    OpenDatabase(db)
    text = CreateAnnotationObject("Text2D")
    text.width = 0.2
    text.position = (0.4, 0.5)
    text.useForegroundForTextColor = 0

    for i in range(4):
        DoBinaryOp(db, logNormal, db, exponential, binops[i], i, text)
   
    text.Delete()
    CloseDatabase(db)


TurnOffCurveLabels()
TestBinary()
Exit()
