# ---------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  revolve.py
#
#  Tests:      mesh      - 2D unstructured, single domain, 
#              plots     - pseudocolor
#              operators - revolve
#
#  Programmer: Alister Maguire
#  Date: Fri Jun 19 13:30:11 PDT 2020
#
#  Modifications:
#
# ----------------------------------------------------------------------------


def TestRevolvingPoints():
    #
    # Make sure that we can revolve points. We weren't always able to do this.
    #
    OpenDatabase(silo_data_path("multi_point2d.silo"))
    AddPlot("Pseudocolor", "v", 1, 1)
    AddOperator("Revolve", 1)
    DrawPlots()
    Test("revolve_points_00")
    DeleteActivePlots()
    CloseDatabase(silo_data_path("multi_point2d.silo"))

def Main():
    TestRevolvingPoints()

Main()
Exit()
