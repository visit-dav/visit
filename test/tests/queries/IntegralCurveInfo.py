# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  IntegralCurveInfo.py #
#  Tests:      queries     - IntegralCurveInfo
#
#  Programmer: Dave Pugmire
#  Date:       Nov 10, 2010
#
#  Modifications:
#    Kathleen Biagas, Thu Jul 14 10:44:55 PDT 2011
#    Use named arguments. 
#
# ----------------------------------------------------------------------------

def QueryMain() :
    OpenDatabase(silo_data_path("noise.silo"))
    AddPlot("Pseudocolor", "operators/IntegralCurve/grad", 1, 0)

    IntegralCurveAtts = IntegralCurveAttributes()
    IntegralCurveAtts.sourceType = IntegralCurveAtts.SpecifiedLine
    IntegralCurveAtts.lineStart = (-8,0,0)
    IntegralCurveAtts.lineEnd = (8,0,0)
    IntegralCurveAtts.sampleDensity0 = 3
    SetOperatorOptions(IntegralCurveAtts)
    DrawPlots()

    s = ""
    Query("Integral Curve Info")
    s = s + GetQueryOutputString()
    s = s + "\n"
    
    Query("Integral Curve Info", dump_index=1, dump_coordinates=1, dump_arclength=1, dump_values=1)
    s = s + GetQueryOutputString()
    s = s + "\n"
    
    TestText("IntegralCurveInfo", s)
    DeleteAllPlots()

# Call the main function
TurnOnAllAnnotations()
QueryMain()
Exit()

