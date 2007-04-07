# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  kurtosis.py
#  Tests:      queries     - kurtosis and skewness
#
#  Defect ID:  VisIt00006466, '6269.
#
#  Programmer: Hank Childs
#  Date:       August 5, 2005
#
# ----------------------------------------------------------------------------

OpenDatabase("../data/distribution.ultra")
AddPlot("Curve", "Laplace Distribution")
DrawPlots()

Query("Kurtosis")
text = GetQueryOutputString()
TestText("kurtosis_01", text)

Query("Skewness")
text = GetQueryOutputString()
TestText("kurtosis_02", text)

ChangeActivePlotsVar("Log Normal Distribution")
Query("Kurtosis")
text = GetQueryOutputString()
TestText("kurtosis_03", text)

Query("Skewness")
text = GetQueryOutputString()
TestText("kurtosis_04", text)

ChangeActivePlotsVar("Exponential Distribution")
Query("Kurtosis")
text = GetQueryOutputString()
TestText("kurtosis_05", text)

Query("Skewness")
text = GetQueryOutputString()
TestText("kurtosis_06", text)



Exit()
