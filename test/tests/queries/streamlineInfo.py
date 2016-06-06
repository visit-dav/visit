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
#    Kathleen Biagas, Thu Jul 14 10:44:55 PDT 2011
#    Use named arguments. 
#
# ----------------------------------------------------------------------------

def QueryMain() :
    OpenDatabase(silo_data_path("noise.silo"))

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
    
    Query("Streamline Info", dump_steps=1)
    s = s + GetQueryOutputString()
    s = s + "\n"
    
    TestText("StreamlineInfo", s)
    DeleteAllPlots()

# Call the main function
TurnOnAllAnnotations()
QueryMain()
Exit()

