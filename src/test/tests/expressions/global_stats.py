# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  global_stats.py
#
#  Tests:      global statistics expressions
#
#  Programmer: Justin Privitera
#  Date:       10/25/24
# ----------------------------------------------------------------------------


# TODO need tests for all 7 kinds of stats
# for both zonal and nodal
# for a file w/ ghosts and w/o
#  |--> so we need 28 tests

def test_stats_for_var(meshname, varname, vartype):
	# define our expressions
	if vartype == "zonal":
		# every zone will have a value of 1
		DefineScalarExpression("one", "zoneid(" + meshname + ") * 0 + 1")
	else:
		# every node will have a value of 1
		DefineScalarExpression("one", "nodeid(" + meshname + ") * 0 + 1")
	DefineScalarExpression("max_" + varname + "_" + vartype,      "global_max(" + varname + ")")
	DefineScalarExpression("min_" + varname + "_" + vartype,      "global_min(" + varname + ")")
	DefineScalarExpression("sum_one_" + varname + "_" + vartype,  "global_sum(one)")
	DefineScalarExpression("sum_" + varname + "_" + vartype,      "global_sum(" + varname + ")")
	DefineScalarExpression("avg_" + varname + "_" + vartype,      "global_avg(" + varname + ")")
	DefineScalarExpression("std_dev_" + varname + "_" + vartype,  "global_std_dev(" + varname + ")")
	DefineScalarExpression("variance_" + varname + "_" + vartype, "global_variance(" + varname + ")")
	DefineScalarExpression("rms_" + varname + "_" + vartype,      "global_rms(" + varname + ")")

	# max
	AddPlot("Pseudocolor", "max_" + varname + "_" + vartype)
	DrawPlots()
	Query("MinMax")
	q = GetQueryOutputObject()
	TestValueEQ("Minimum Field Value", q['min'], 1.0)
	TestValueEQ("Maximum Field Value", q['max'], 1.0)
	DeleteAllPlots()

# no ghosts
OpenDatabase(silo_data_path("curv3d.silo"))
test_stats_for_var("curvmesh3d", "d", "zonal") # zonal var
test_stats_for_var("curvmesh3d", "u", "nodal") # nodal var
CloseDatabase(silo_data_path("curv3d.silo"))
# yes ghosts
OpenDatabase(silo_data_path("curv2d.silo"))
test_stats_for_var("curvmesh2d", "d", "zonal") # zonal var
test_stats_for_var("curvmesh2d", "u", "nodal") # nodal var
CloseDatabase(silo_data_path("curv2d.silo"))

Exit()
