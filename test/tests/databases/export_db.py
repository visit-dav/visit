# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  export_db.py
#
#  Tests:      mesh      - 3D unstructured
#              plots     - Pseudocolor
#
#  Defect ID:  '6288, '6290, '6587, '6366
#
#  Programmer: Hank Childs
#  Date:       July 1, 2005
#
#  Modifications:
#
#    Hank Childs, Thu Sep 15 16:33:48 PDT 2005
#    Add test for exporting CMFEs as secondary variables ('6587)
#
#    Kathleen Bonnell, Tue May  2 08:58:01 PDT 2006 
#    Corrected exported database names (from .visit to .vtk). 
#
#    Hank Childs, Wed Mar 28 11:33:16 PDT 2007
#    Uncommented tests for '6366, which were previously checked in, but
#    commented out.
#
# ----------------------------------------------------------------------------

# Turn off all annotation

a = AnnotationAttributes()
a.axesFlag2D = 0
a.axesFlag = 0
a.triadFlag = 0
a.bboxFlag = 0
a.userInfoFlag = 0
a.databaseInfoFlag = 0
a.legendInfoFlag = 0
a.backgroundMode = 0
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
SetAnnotationAttributes(a)


OpenDatabase("../data/globe.silo")
AddPlot("Pseudocolor", "t")
DrawPlots()

# Set the export database attributes.
e = ExportDBAttributes()
e.db_type = "VTK"
e.filename = "test_ex_db"
ExportDatabase(e)

e.variables = ("u", "v")
e.filename = "test_ex_db2"
e.db_type = "Silo"
ExportDatabase(e)

DeleteAllPlots()
OpenDatabase("test_ex_db.vtk")
AddPlot("Pseudocolor", "t")
DrawPlots()
Test("export_db_01")

DeleteAllPlots
OpenDatabase("test_ex_db2.silo")
AddPlot("Pseudocolor", "u")
DrawPlots()
Test("export_db_02")

DeleteAllPlots()
OpenDatabase("../data/wave.visit")
DefineScalarExpression("cmfe", "conn_cmfe(coord(<../data/wave0020.silo:quadmesh>)[1], quadmesh)")
AddPlot("Pseudocolor", "pressure")
DrawPlots()
e.variables = ("cmfe")
e.filename = "test_ex_db3"
e.db_type = "VTK"
ExportDatabase(e)
DeleteAllPlots()

DeleteExpression("cmfe")
OpenDatabase("test_ex_db3.vtk")
AddPlot("Pseudocolor", "cmfe")
DrawPlots()
Test("export_db_03")

Exit()

