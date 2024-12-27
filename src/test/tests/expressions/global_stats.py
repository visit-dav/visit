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
#  Description:
# 
# global statistics expressions are sensitive to ghost zones/nodes, so we test
# with two datasets - curv3d.silo which has no ghosts, and curv2d.silo which
# has ghosts. For each one, we look at the variables d (zonal) and u (nodal),
# since zonal and nodal variables have slightly different paths for global 
# stats expressions. For each variable, we look at each of the statistics on 
# offer.
# 
# ----------------------------------------------------------------------------

# no ghosts in curv3d
curv3d_stats = {}

# zonal stats
curv3d_stats["max_d"] = 4.9557
curv3d_stats["min_d"] = 2.03471
curv3d_stats["num_d"] = 36000 # num zones
curv3d_stats["sum_d"] = 125827.3203125
curv3d_stats["avg_d"] = 3.4952
curv3d_stats["std_d"] = 0.864568 # standard deviation
curv3d_stats["var_d"] = 0.747478 # variance
curv3d_stats["rms_d"] = 3.60055 # root mean square

# nodal stats
curv3d_stats["max_u"] = 1
curv3d_stats["min_u"] = -1
curv3d_stats["num_u"] = 39401 # num nodes
curv3d_stats["sum_u"] = -93
curv3d_stats["avg_u"] = -0.00236035
curv3d_stats["std_u"] = 0.740144 # standard deviation
curv3d_stats["var_u"] = 0.547814 # variance
curv3d_stats["rms_u"] = 0.740148 # root mean square

# yes ghosts in curv2d
curv2d_stats = {}

# zonal stats
curv2d_stats["max_d"] = 4.8808
curv2d_stats["min_d"] = 2.1096
curv2d_stats["num_d"] = 988 # num zones
curv2d_stats["sum_d"] = 3453.2609271746946
curv2d_stats["avg_d"] = 3.4952
curv2d_stats["std_d"] = 0.821312 # standard deviation
curv2d_stats["var_d"] = 0.674554 # variance
curv2d_stats["rms_d"] = 3.5904 # root mean square

# nodal stats
curv2d_stats["max_u"] = 1
curv2d_stats["min_u"] = -1
curv2d_stats["num_u"] = 1053 # num nodes
curv2d_stats["sum_u"] = 67.70071411132812
curv2d_stats["avg_u"] = 0.0642932
curv2d_stats["std_u"] = 0.7122947573661804 # standard deviation
curv2d_stats["var_u"] = 0.507364 # variance
curv2d_stats["rms_u"] = 0.715191 # root mean square

baseline_stats = {}
baseline_stats["curvmesh3d"] = curv3d_stats
baseline_stats["curvmesh2d"] = curv2d_stats

def test_stat(shortstatname, longstatname, meshname, varname, vartype):
	AddPlot("Pseudocolor", shortstatname + "_" + varname + "_" + vartype)
	DrawPlots()
	Query("MinMax")
	q = GetQueryOutputObject()
	# we test both the min and max because we want to ensure the variable is constant
	# across the mesh.
	if vartype == "zonal":
		TestValueEQ("Test " + longstatname + " 1 for " + varname, q['min'], 
			baseline_stats[meshname][shortstatname + "_" + varname])
		TestValueEQ("Test " + longstatname + " 2 for " + varname, q['max'], 
			baseline_stats[meshname][shortstatname + "_" + varname])
	else:
		TestValueEQ("Test " + longstatname + " 1 for " + varname, q['min'], 
			baseline_stats[meshname][shortstatname + "_" + varname])
		TestValueEQ("Test " + longstatname + " 2 for " + varname, q['max'], 
			baseline_stats[meshname][shortstatname + "_" + varname])
	DeleteAllPlots()

def test_stats_for_var(meshname, varname, vartype):
	# define our expressions
	if vartype == "zonal":
		# every zone will have a value of 1
		DefineScalarExpression("one", "zonal_constant(" + meshname + ", 1)")
	else:
		# every node will have a value of 1
		DefineScalarExpression("one", "nodal_constant(" + meshname + ", 1)")
	DefineScalarExpression("avg_" + varname + "_" + vartype, "global_avg(" + varname + ")")
	DefineScalarExpression("max_" + varname + "_" + vartype, "global_max(" + varname + ")")
	DefineScalarExpression("min_" + varname + "_" + vartype, "global_min(" + varname + ")")
	DefineScalarExpression("num_" + varname + "_" + vartype, "global_sum(one)")
	DefineScalarExpression("rms_" + varname + "_" + vartype, "global_rms(" + varname + ")")
	DefineScalarExpression("std_" + varname + "_" + vartype, "global_std_dev(" + varname + ")")
	DefineScalarExpression("sum_" + varname + "_" + vartype, "global_sum(" + varname + ")")
	DefineScalarExpression("var_" + varname + "_" + vartype, "global_variance(" + varname + ")")

	test_stat("avg", "Average",            meshname, varname, vartype)
	test_stat("max", "Maximum",            meshname, varname, vartype)
	test_stat("min", "Minimum",            meshname, varname, vartype)
	test_stat("num", "Number",             meshname, varname, vartype)
	test_stat("rms", "Root Mean Square",   meshname, varname, vartype)
	test_stat("std", "Standard Deviation", meshname, varname, vartype)
	test_stat("sum", "Sum",                meshname, varname, vartype)
	test_stat("var", "Variance",           meshname, varname, vartype)

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
