# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  pickNamedArgs.py #
#  Tests:      queries     - Pick using named-arguments
#                            Pick using query results as arguments
#
#  Defect ID:  1781
#
#  Programmer: Kathleen Biagas
#  Date:       July 11, 2014 
#
#  Modifications:
#    Kathleen Biagas, Tue Jul 22 11:51:27 MST 2014
#    Added Pick_ByGlobalId.
#
# ---------------------------------------------------------------------------- 

def TimePick():
    OpenDatabase(silo_data_path("wave.visit"))

    AddPlot("Pseudocolor", "pressure")
    DrawPlots()

    # to demonstrate the effect of changing time options, use timestep for x axis.
    q = GetQueryOverTimeAttributes()
    q.timeType = q.Timestep
    SetQueryOverTimeAttributes(q)

    # Controlling which time steps are queried, showing defaults used
    #     stride     => int    default: 1
    #     start_time => int    default: 0 (min time index)
    #     end_time   => int    default: max time index, as determined from DB

    # Controlling the plot type when multiple vars are queried
    #     curve_plot_type: 0 (Single Y Axis), or  1 (Multiple Y Axes) default: 0

    # Which is used for subsequent timesteps, the coordinate or the element?
    #     preserve_coord:  0 (use the element)  or 1 (use the coordinate)

    vars =("pressure", "v", "direction_magnitude")
    PickByNode(element=8837, vars=vars, do_time=1, preserve_coord=0,curve_plot_type=0)
    SetActiveWindow(2);
    Test("TimePick_NamedArgs_00")
    DeleteAllPlots()

    SetActiveWindow(1)
    PickByNode(element=8837, vars=vars, do_time=1, preserve_coord=0,curve_plot_type=0,start_time=15, end_time=50, stride=7)
    SetActiveWindow(2);
    Test("TimePick_NamedArgs_01")
    DeleteAllPlots()

    SetActiveWindow(1)
    # different way for specifying time options, result should be same as previopus:
    time_opt = {'stride':7, 'start_time': 15, 'end_time':50}
    PickByNode(element=8837, vars=vars, do_time=1, preserve_coord=0, time_options=time_opt)
    SetActiveWindow(2);
    Test("TimePick_NamedArgs_02")
    DeleteAllPlots()

    SetActiveWindow(1)
    PickByNode(element=8837, vars=vars, do_time=1, preserve_coord=0,curve_plot_type=1)
    SetActiveWindow(2);
    Test("TimePick_NamedArgs_03")
    DeleteAllPlots()

    SetActiveWindow(1);
    PickByNode(element=8837, vars=vars, do_time=1, preserve_coord=0,curve_plot_type=1,stride=10)
    SetActiveWindow(2);
    Test("TimePick_NamedArgs_04")
    DeleteAllPlots()
   
    SetActiveWindow(1)
    DeleteAllPlots()
    CloseDatabase(silo_data_path("wave.visit"))

def PickUsingQueryResults():
    ResetPickLetter()
    OpenDatabase(silo_data_path("wave.visit"))
    AddPlot("FilledBoundary", "Material")
    SetTimeSliderState(31)
    DrawPlots()

    # Make it easier to get coordinate values
    SetQueryOutputToValue()
    s = "Pick using results of Node Coords query:\n"

    node_8837_coords = Query("Node Coords", element=8837)
   
    # Now test that NodePick using those coords will return the correct node id
    # Utilizes named args: coord (a tuple containing spatial coordinates)

    pick_out = NodePick(coord=node_8837_coords)
    if pick_out['node_id'] == 8837:
        s = s + "    NodePick with coordinate succeeded.\n"
    else:
        s = s + "    NodePick with coordinate failed to return correct node id.\n"


    s = "Pick using results of Zone Center query:\n"
    # select a zone that is incident to node 8837
    adjacent_zone = pick_out['incident_zones'][0]
    # Get it's coords and try a pick
    adjacent_zone_coords = Query("Zone Center", element=adjacent_zone)
    pick_out = ZonePick(coord=adjacent_zone_coords)
    if pick_out['zone_id'] == adjacent_zone:
        s = s + "    ZonePick with coordinate succeeded.\n"
    else:
        s = s + "    ZonePick with coordinate failed to return correct node id\n"

    # Now, does Pick report that our original node id is incident to the picked zone?
    if 8837 in pick_out['incident_nodes']:
        s = s + "    ZonePick correctly reported incident node\n"
    else: 
        s = s + "    ZonePick failed to correctly report incident node\n"

    DeleteAllPlots()
    CloseDatabase(silo_data_path("wave.visit"))


    OpenDatabase(silo_data_path("multi_rect3d.silo"))
    # d is zonal quantity, so picks will be by-zone
    AddPlot("Pseudocolor", "d")
    DrawPlots()

    # Change query output type for parse-ability of MinMax results
    SetQueryOutputToObject()

    # Do a MinMax query, and use the results to drive a pick.
    res = Query("MinMax")
    s = s + "\nQuery for MinMax of 'd': \n" + GetQueryOutputString()

    s = s + "Pick using results of MinMax Query:\n\n"

    # Pick using MinMax results
    # Utilizes named args: 
    #     domain (an integer), 
    #     element (an integer representing node id or zone id depending on the type of Pick)
    pick_out = PickByZone(domain=res['min_domain'], element=res['min_element_num'])
    s = s + "Pick for zone at 'd' minimum: \n" + GetPickOutput()
    pick_out = PickByZone(domain=res['max_domain'], element=res['max_element_num'])
    s = s + "Pick for zone at 'd' maximum: \n" + GetPickOutput() 
    TestText("Pick_UsingQueryResults", s)
    ResetPickLetter()
    DeleteAllPlots()
    CloseDatabase(silo_data_path("multi_rect3d.silo"))
 


def doGlobalPicks(centering):
    s = "Global node pick on %s_centered data:\n\n" %centering
    d = PickByGlobalNode(element=246827)
    s = s + "dictionary output:\n" + str(d) + "\n\n"
    s = s + "string output: " + GetPickOutput() + "\n\n"

    s = s + "\nGlobal zone pick on %s_centered data:\n\n" %centering
    d = PickByGlobalZone(element=237394)
    s = s + "dictionary output:\n" + str(d) + "\n\n"
    s = s + "string output: " + GetPickOutput() + "\n\n"

    return s

def PickByGlobalElement():
    OpenDatabase(silo_data_path("global_node.silo"))

    AddPlot("Pseudocolor", "dist")
    DrawPlots()
    ResetView()
    p = GetPickAttributes()
    p.showGlobalIds = 0
    SetPickAttributes(p)

    s = "NOT SHOWING GLOBAL IDS\n\n"
    s = s + doGlobalPicks("node")

    #  Repeat, showing global ids
    p.showGlobalIds = 1
    SetPickAttributes(p)

    s = s + "SHOWING GLOBAL IDS\n\n"
    s = s + doGlobalPicks("node")

    p.showGlobalIds = 0
    SetPickAttributes(p)

    # now use a zone-centered var
    ChangeActivePlotsVar("p")

    s = s + "NOT SHOWING GLOBAL IDS\n\n"
    s = s + doGlobalPicks("zone")

    #  Repeat, showing global ids
    p.showGlobalIds = 1
    SetPickAttributes(p)

    s = s + "SHOWING GLOBAL IDS\n\n"
    s = s + doGlobalPicks("zone")
   
    TestText("Pick_ByGlobalElement", s)
    # cleanup
    p.showGlobalIds = 0
    SetPickAttributes(p)
    DeleteAllPlots()
    ResetPickLetter()

TimePick()
PickUsingQueryResults()
PickByGlobalElement()
Exit()
