# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  pickQueryCombo.py #
#  Tests:      queries     - Pick, Zone Center, Node Coords,
#              operators   - OnionPeel
#
#  Notes:
#     Tests consistency between Pick/Query/OnionPeel when using the same
#     node/zone ids for each.
#
#  Programmer: Kathleen Biagas
#  Date:       September 9, 2014
#
#  Modifications:
#
# ----------------------------------------------------------------------------
testNum = 0

def PickCurv2D():
    # Ensures that Pick/Query/OnionPeel operator correctly and consistently
    # with the ghost data from this ds, which affects the numbering of zones
    global testNum
    OpenDatabase(silo_data_path("curv2d.silo"))
    AddPlot("Mesh", "curvmesh2d")
    AddPlot("Pseudocolor", "d")
    DrawPlots()
    zoneID = 482
    #do a pick before Onion peel
    p1 = PickByZone(zoneID)
    nodeID = p1["incident_nodes"][0]
    #Add Onion peel of same zone, see if it ends up under the pick letter
    AddOperator("OnionPeel")
    op = OnionPeelAttributes()
    op.index = zoneID
    SetOperatorOptions(op)
    DrawPlots()
    v = GetView2D()
    v.windowCoords =(1.33, 4.89, 0.18, 3.43)
    SetView2D(v)
    Test("consistencyChecks_%02d"%testNum)
    testNum = testNum+1

    ClearPickPoints()

    #Now add a Boundary plot, do the same pick, see if get same results.
    AddPlot("Boundary", "mat1")
    DrawPlots()
    p2 = PickByZone(zoneID)
    s = ""
    if p2['point'] == p1['point']:
        s = s + "PickByZone on Pseudcolor and Boundary plots yielded same point.\n"
    else:
        s = s + "PickByZone on Pseudcolor and Boundary plots DID NOT yield same point.\n"
    SetQueryOutputToObject()
    q = Query("Zone Center", element=zoneID)
    if (q['center'] == p1['point']):
        s = s + "Zone Center query returned same coords as Pick.\n" 
    else:
        s = s + "Zone Center query returned different coords than Pick.\n" 

    #Now feed the zone center as coordinates into a pick:
    p3 = ZonePick(coord = q['center'])
    s = s + "Pick using coord from Zone Center query returned zone: %d, (should be: %d).\n" %(p3['zone_id'], zoneID)

    Test("consistencyChecks_%02d"%testNum)
    testNum = testNum+1
    ClearPickPoints()

    # Now do the same type of tests for Node Pick
    DeleteActivePlots()
    SetActivePlots(1)
    RemoveLastOperator()

    p1n = PickByNode(nodeID)
    AddOperator("OnionPeel")
    op.seedType = op.SeedNode
    op.index = nodeID
    SetOperatorOptions(op)
    DrawPlots()
    Test("consistencyChecks_%02d"%testNum)
    testNum = testNum+1

    ClearPickPoints()
    #Now add a Boundary plot, do the same pick, see if get same results.
    AddPlot("Boundary", "mat1")
    DrawPlots()

    p2n = PickByNode(nodeID)
    if p2n['point'] == p1n['point']:
        s = s + "PickByNode on Pseudcolor and Boundary plots yielded same point.\n"
    else:
        s = s + "PickByNode on Pseudcolor and Boundary plots DID NOT yield same point.\n"
    qn = Query("Node Coords", element=nodeID)
    if (qn['coord'] == p1n['point']):
        s = s + "Node Coords query returned same coords as Pick.\n" 
    else:
        s = s + "Node Coords query returned different coords than Pick.\n" 

    p3n = NodePick(coord = qn['coord'])
    s = s + "Pick using coord from Node Coord query returned node: %d, (should be: %d).\n" %(p3n['node_id'], nodeID)

    Test("consistencyChecks_%02d"%testNum)
    testNum = testNum+1

    TestText("consistencyChecks_%02d"%testNum, s)
    testNum = testNum+1

    DeleteAllPlots()
    CloseDatabase(silo_data_path("curv2d.silo"))

PickCurv2D()
Exit()
