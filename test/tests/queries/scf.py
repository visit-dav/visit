# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scf.py
#  Tests:      queries     - spherical compactness factor
#
#  Defect ID:  VisIt00006385
#
#  Programmer: Hank Childs
#  Date:       July 14, 2005
#
#  Modifications:
#    Mark C. Miller, Fri Nov 17 22:03:34 PST 2006
#    Accounted for fact that threshold operator no longer has a default var
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
#
#    Cyrus Harrison, Thu Mar 25 09:57:34 PDT 2010
#    Added call(s) to DrawPlots() b/c of changes to the default plot state 
#    behavior when an operator is added.
#
#    Kathleen Biagas, Thu Jul 14 10:44:55 PDT 2011
#    Use named arguments. 
#
# ----------------------------------------------------------------------------

# Turn off all annotation
a = AnnotationAttributes()
TurnOffAllAnnotations(a)

OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Pseudocolor", "d")
AddOperator("Isovolume")
iso_atts = IsovolumeAttributes()
iso_atts.ubound = 0.7
SetOperatorOptions(iso_atts)
DrawPlots()

Query("Spherical Compactness Factor")
text = GetQueryOutputString()
TestText("scf_01", text)

AddOperator("Revolve")
DrawPlots()
Query("Spherical Compactness Factor")
text = GetQueryOutputString()
TestText("scf_02", text)

DeleteAllPlots()

OpenDatabase(silo_data_path("wave.visit"))

AddPlot("Pseudocolor", "pressure")
DrawPlots()
QueryOverTime("Spherical Compactness Factor", stride=10)

SetActiveWindow(2)
SetAnnotationAttributes(a)
Test("scf_03")

DeleteAllPlots()
OpenDatabase(silo_data_path("rect2d.silo"))

AddPlot("Pseudocolor", "d")
i = ThresholdAttributes()
i.lowerBounds = (0.0)
i.upperBounds = (0.5)
i.listedVarNames = ("d")
SetDefaultOperatorOptions(i)
AddOperator("Threshold")
DrawPlots()
# Don't test localized compactness factor until radii is sorted out.
#Query("Localized Compactness Factor")
#t = GetQueryOutputString()
#TestText("scf_04", t)
Query("Elliptical Compactness Factor")
t = GetQueryOutputString()
TestText("scf_05", t)

AddOperator("Revolve")
DrawPlots()
#Query("Localized Compactness Factor")
#t = GetQueryOutputString()
#TestText("scf_06", t)
Query("Elliptical Compactness Factor")
t = GetQueryOutputString()
TestText("scf_07", t)

Exit()
