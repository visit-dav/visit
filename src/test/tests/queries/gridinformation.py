# ---------------------------------------------------------------------------- 
#  CLASSES: nightly
#
#  Test Case:  database.py #
#  Tests:      Grid Information
#
#  Programmer: Brad Whitlock
#  Date:       Wed Jan  3 16:38:02 PST 2018
#
#  Modifications:
#
# ---------------------------------------------------------------------------- 

newline = "=================================================\n"

tests = (
    ("gridinformation_00", silo_data_path("noise.silo"), "Pseudocolor", "radial"),
    ("gridinformation_01", silo_data_path("globe.silo"), "Pseudocolor", "speed"),
    ("gridinformation_02", silo_data_path("multi_rect3d.silo"), "Pseudocolor", "d"),
    ("gridinformation_03", silo_data_path("multi_curv3d.silo"), "Pseudocolor", "d"),
    ("gridinformation_04", silo_data_path("multi_ucd3d.silo"), "Pseudocolor", "d"))

for t in tests:
    OpenDatabase(t[1])
    AddPlot(t[2], t[3])
    DrawPlots()
    Query("Grid Information")
    s = GetQueryOutputString()
    s = s + newline

    Query("Grid Information", get_extents=1)
    s = s + GetQueryOutputString()
    s = s + newline

    Query("Grid Information", get_extents=1, get_ghosttypes=1)
    s = s + GetQueryOutputString()

    TestText(t[0], s)
    DeleteAllPlots()  
    CloseDatabase(t[1])

Exit()
