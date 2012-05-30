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
#
#    Kathleen Bonnell, Thu Apr  3 10:00:48 PDT 2008 
#    Added tests from bug 8425. 
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Kathleen Biagas, Thu Jul 14 10:44:55 PDT 2011
#    Clean up unused / unnecessary query arguments. 
#
# ----------------------------------------------------------------------------

def QueryMultiWindow():
    OpenDatabase(silo_data_path("wave*.silo database"))

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
    Query("NumNodes")
    s = s + GetQueryOutputString() + "\n"
    Query("NumZones")
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
    Query("NumNodes")
    s = s + GetQueryOutputString() + "\n"
    Query("NumZones")
    s = s + GetQueryOutputString() + "\n"
    
    #Delete window 2
    DeleteWindow()
    #Delete plots in window 1
    DeleteAllPlots()

    #bug 8425 (multiple windows, same db, same timestate)
    OpenDatabase(silo_data_path("rect3d.silo"))

    DefineScalarExpression("X", "coord(quadmesh3d)[0]")
    DefineScalarExpression("unnamed1", "X-1")
    DefineScalarExpression("unnamed2", "X-2")
    DefineScalarExpression("unnamed3", "X-3")
    SetWindowLayout(4)
    AddPlot("Pseudocolor", "unnamed1")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + "\n" + GetQueryOutputString() + "\n"
    SetActiveWindow(2)
    AddPlot("Pseudocolor", "unnamed2")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n"
    SetActiveWindow(3)
    AddPlot("Pseudocolor", "unnamed3")
    DrawPlots()
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n"
    SetActiveWindow(2)
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n"

    SetActiveWindow(4)
    DeleteWindow()
    SetActiveWindow(3)
    DeleteWindow()
    SetActiveWindow(2)
    DeleteWindow()
    DeleteAllPlots()

    #bug 8425 (multiple plots, same window, same db, same root var)
    OpenDatabase(silo_data_path("multi_rect2d.silo"))

    DefineScalarExpression("p2", "p*p")
    DefineScalarExpression("p3", "p*p*p")
    AddPlot("Pseudocolor", "p")
    AddPlot("Pseudocolor", "p2")
    AddPlot("Pseudocolor", "p3")
    DrawPlots()
    SetActivePlots(0)
    Query("Weighted Variable Sum")
    s = s + "\n" + GetQueryOutputString() + "\n"
    SetActivePlots(1)
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n"
    SetActivePlots(2)
    Query("Weighted Variable Sum")
    s = s + GetQueryOutputString() + "\n"

    TestText("QueryMultiWindow", s)
    
    DeleteAllPlots() 

# Call the main function
TurnOnAllAnnotations()
QueryMultiWindow()
Exit()

