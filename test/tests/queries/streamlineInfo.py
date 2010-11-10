# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  streamlineInfo.py #
#  Tests:      queries     - streamlineInfo
#
#  Programmer: Dave Pugmire
#  Date:       Nov 10, 2010
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def QueryMain() :
    OpenDatabase("../data/silo_%s_test_data/noise.silo"%SILO_MODE)
    AddPlot("Streamline", "grad")
    sa = StreamlineAttributes()
    sa.sourceType = sa.SpecifiedLine
    sa.lineStart = (-8,0,0)
    sa.lineEnd = (8,0,0)
    sa.sampleDensity0 = 10
    SetPlotOptions(sa)
    DrawPlots()

    s = ""
    Query("Streamline Info")
    s = s + GetQueryOutputString()
    s = s + "\n"
    
    Query("Streamline Info", "DumpSteps")
    s = s + GetQueryOutputString()
    s = s + "\n"
    
    TestText("StreamlineInfo", s)
    DeleteAllPlots()

# Call the main function
TurnOnAllAnnotations()
QueryMain()
Exit()

