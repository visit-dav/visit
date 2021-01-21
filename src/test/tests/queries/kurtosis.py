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

TurnOnAllAnnotations()
OpenDatabase(data_path("curve_test_data/distribution.ultra"))

AddPlot("Curve", "Laplace Distribution")
DrawPlots()

Query("Kurtosis")
TestValueEQ("kurtosis_01", GetQueryOutputValue(), 5.9999)

Query("Skewness")
TestValueEQ("kurtosis_02", GetQueryOutputValue(), -2.29557e-09)

ChangeActivePlotsVar("Log Normal Distribution")
Query("Kurtosis")
TestValueEQ("kurtosis_03", GetQueryOutputValue(), 19.73632)

Query("Skewness")
TestValueEQ("kurtosis_04", GetQueryOutputValue(), 3.38354)

ChangeActivePlotsVar("Exponential Distribution")
Query("Kurtosis")
TestValueEQ("kurtosis_05", GetQueryOutputValue(), 8.99968)

Query("Skewness")
TestValueEQ("kurtosis_06", GetQueryOutputValue(), 1.99999)

Exit()
