# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  queryMultiWindow.py #
#  Tests:      queries     - Database
#
#  Defect ID:  none
#
#  Programmer: Kathleen Bonnell 
#  Date:       May 19, 2004 
#
#  Modifications:
#    Kathleen Bonnell, Mon Dec 20 15:54:04 PST 2004
#    Changed 'Variable by Zone' to 'PickByZone' and 'Variable by Node' to
#    'PickByNode'.
# ----------------------------------------------------------------------------

def QueryMultiWindow():
    OpenDatabase("../data/wave*.silo database")
    AddPlot("Pseudocolor", "pressure")
    SetTimeSliderState(31)
    DrawPlots()

    v = GetView3D()
    v.viewNormal = (0, -1, 0)
    v.viewUp = (0, 0, 1)
    SetView3D(v)

    CloneWindow()
    SetTimeSliderState(64)
    DrawPlots()
    ChangeActivePlotsVar("v")

    s = "Window 1 queries:\n"
    SetActiveWindow(1)
    Query("MinMax")
    s = s + GetQueryOutputString() 
    Query("3D surface area")
    s = s + GetQueryOutputString() + "\n"
    Query("Volume")
    s = s + GetQueryOutputString() + "\n"
    PickByZone(7072)
    s = s + GetPickOutput() + "\n"
    PickByNode(11168)
    s = s + GetPickOutput() + "\n"
    Query("Variable Sum") 
    s = s + GetQueryOutputString() + "\n"
    Query("Weighted Variable Sum") 
    s = s + GetQueryOutputString() + "\n"
    Query("NumNodes", "original") 
    s = s + GetQueryOutputString() + "\n"
    Query("NumZones", "original") 
    s = s + GetQueryOutputString() + "\n"


    s = s + "\nWindow 2 queries:\n"
    SetActiveWindow(2)
    Query("MinMax")
    s = s + GetQueryOutputString() 
    Query("3D surface area")
    s = s + GetQueryOutputString() + "\n"
    Query("Volume")
    s = s + GetQueryOutputString() + "\n"
    PickByZone(7072)
    s = s + GetPickOutput() + "\n"
    PickByNode(11168)
    s = s + GetPickOutput() + "\n"
    Query("Variable Sum") 
    s = s + GetQueryOutputString() + "\n"
    Query("Weighted Variable Sum") 
    s = s + GetQueryOutputString() + "\n"
    Query("NumNodes", "original") 
    s = s + GetQueryOutputString() + "\n"
    Query("NumZones", "original") 
    s = s + GetQueryOutputString() + "\n"
    
    TestText("QueryMultiWindow", s)
    #Delete window 2
    DeleteWindow()
    #Delete plots in window 1
    DeleteAllPlots()


# Call the main function
QueryMultiWindow()
Exit()

