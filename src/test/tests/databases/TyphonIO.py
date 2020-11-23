# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  TyphonIO.py
#
#  Tests:      TyphonIO file format
#
#  Programmer: Paul Selby
#  Date:       Thu 12 Feb 11:17:16 GMT 2015
#
#  Modifications:
#    Paul Selby, Thu 19 Mar 12:05:50 GMT 2015
#    Added Quad and Unstructured meshes along with Mesh & Subset plots
#
#    Paul Selby, Wed 25 Mar 13:20:34 GMT 2015
#    Added 2D LAMMPS example
#
#    Paul Selby, Wed 25 Mar 18:07:07 GMT 2015
#    Changed blocks to chunks in Subset plots
#
#    Paul Selby, Thu 26 Mar 13:13:59 GMT 2015
#    Added Pseudocolor plot for Point & LAMMPS meshes
#
#    Paul Selby, Thu 18 Jun 15:00:39 BST 2015
#    Added Material plot for Point & LAMMPS meshes
#
#    Paul Selby, Tue 23 Jun 16:06:46 BST 2015
#    Added Non-Colinear Quad mesh
#
# ----------------------------------------------------------------------------
RequiredDatabasePlugin("TyphonIO")

TurnOffAllAnnotations()

#
# Point Mesh
#
TestSection("Point Mesh")
db = data_path("TyphonIO_test_data/pointv1.h5")
OpenDatabase(db)

#
# Point Mesh metadata
#
md = GetMetaData(db)
TestText("typhonio_01_01", str(md))

#
# Point Mesh mesh plot
#
AddPlot("Mesh", "Particles")
DrawPlots()
Test("typhonio_01_02")

#
# Point Mesh subset plot
# - remove mesh as won't see otherwise
#
DeleteAllPlots()
AddPlot("Subset", "chunks")
DrawPlots()
Test("typhonio_01_03")

#
# Pseudocolor plot
#
DeleteAllPlots()
AddPlot("Pseudocolor", "Value")
DrawPlots()
Test("typhonio_01_04")

#
# Point Mesh material plot
#
DeleteAllPlots()
AddPlot("FilledBoundary", "Material")
DrawPlots()
Test("typhonio_01_05")

DeleteAllPlots()
CloseDatabase(db)

# ----------------------------------------------------------------------------

#
# Quad Mesh
#
TestSection("Quad Mesh")
db = data_path("TyphonIO_test_data/qfile.h5")
OpenDatabase(db)

#
# Quad Mesh metadata
#
md = GetMetaData(db)
TestText("typhonio_02_01", str(md))

#
# Quad Mesh mesh plot
#
AddPlot("Mesh", "qmesh2")
DrawPlots()
Test("typhonio_02_02")

#
# Quad Mesh subset plot
#
AddPlot("Subset", "chunks(qmesh2)")
DrawPlots()
Test("typhonio_02_03")

DeleteAllPlots()
CloseDatabase(db)

# ----------------------------------------------------------------------------

#
# Unstructured Mesh
#
TestSection("Unstructured Mesh")
db = data_path("TyphonIO_test_data/Test_Unstr.h5")
OpenDatabase(db)

#
# Unstructured Mesh metadata
#
md = GetMetaData(db)
TestText("typhonio_03_01", str(md))

#
# UnstructuredMesh mesh plot
#
AddPlot("Mesh", "Mesh1")
DrawPlots()
Test("typhonio_03_02")

#
# Unstructured Mesh subset plot
#
AddPlot("Subset", "blocks")
DrawPlots()
Test("typhonio_03_03")

DeleteAllPlots()
CloseDatabase(db)

# ----------------------------------------------------------------------------

#
# LAMMPS example
#
TestSection("LAMMPS output")
db = data_path("TyphonIO_test_data/dump4_step.h5")
OpenDatabase(db)

#
# metadata
#
md = GetMetaData(db)
TestText("typhonio_04_01", str(md))

#
# Mesh plot
#
AddPlot("Mesh", "Atom Coordinates")
DrawPlots()
Test("typhonio_04_02")

#
# Advance time
#
TimeSliderNextState()
Test("typhonio_04_03")

#
# Pseudocolor plot
#
DeleteAllPlots()
AddPlot("Pseudocolor", "Force_X")
DrawPlots()
Test("typhonio_04_04")

#
# Material plot
#
DeleteAllPlots()
AddPlot("FilledBoundary", "Atom Types")
DrawPlots()
Test("typhonio_04_05")

DeleteAllPlots()
CloseDatabase(db)

# ----------------------------------------------------------------------------

#
# Non-colinear Quad Mesh
#
TestSection("Non-Colinear Quad Mesh")
db = data_path("TyphonIO_test_data/qfile.h5")
OpenDatabase(db)

#
# Non-colinear Quad Mesh metadata
#
md = GetMetaData(db)
TestText("typhonio_05_01", str(md))

#
# Non-colinear Quad Mesh mesh plot
#
AddPlot("Mesh", "cncmesh")
DrawPlots()
Test("typhonio_05_02")

#
# Non-colinear Quad Mesh subset plot
#
AddPlot("Subset", "chunks(cncmesh)")
DrawPlots()
Test("typhonio_05_03")

DeleteAllPlots()
CloseDatabase(db)


Exit()
