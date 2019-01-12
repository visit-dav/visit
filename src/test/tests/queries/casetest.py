# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  casetest.py #
#  Tests:      queries     - Database
#
#  Description: Tests case insensitive name dispatch of queries.
#
#  Programmer: Cyrus Harrison
#  Date:       Mon Sep 19 15:09:02 PDT 2011 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def QueryRect2d():
    OpenDatabase(silo_data_path("rect2d.silo"))

    AddPlot("Pseudocolor", "d")
    DrawPlots()
    s = ""
    Query("Revolved volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("2D area")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("MinMax", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("SpatialExtents", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumNodes", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NumZones", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("revolved volume")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("2D AREA")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("mINmAX", "actual")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("spatialextents", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NUMNODES", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    Query("NUMZONES", "original")
    s = s + GetQueryOutputString()
    s = s + "\n"
    TestText("CaseQueryRect2dTest", s)
    DeleteAllPlots()  


def QueryMain():
    QueryRect2d()
 
# Call the main function
TurnOnAllAnnotations()
QueryMain()
Exit()

