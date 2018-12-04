# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  multi_format2.py 
#
#  Tests:      Using multiple file formats types in the same engine.
#
#  Programmer: Jeremy Meredith
#  Creation:   March 23, 2004
#
#  Tests a strange case related to the original multi_format.py where the
#  engine is restarted implicitly.  This is currently skipped in parallel
#  since an implicit engine restart does not pick up the same paralell args.
#
#  Modifications:
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------


db = (silo_data_path("rect2d.silo") ,
      data_path("pdb_test_data/allinone00.pdb"))

# Create a Pseudocolor plot of Rect2d.
OpenDatabase(db[0])
AddPlot("Pseudocolor", "p")

# Move it off to the side
t=TransformAttributes()
t.doTranslate = 1
t.translateX = 11.0
t.doScale = 1;
t.scaleX = 22.5;
t.scaleY = 22.5;
SetDefaultOperatorOptions(t)
AddOperator("Transform")

# Create a Pseudocolor plot of AllInOne
OpenDatabase(db[1])
AddPlot("FilledBoundary", "material(mesh)")

# Close the compute engine!
CloseComputeEngine("localhost");

# Allow it to restart
DrawPlots()

# Test it!
Test("multi_format2_01")

Exit()
