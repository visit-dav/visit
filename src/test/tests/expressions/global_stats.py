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
#  Modifications:
#    Justin Privitera, Tue Oct  7 16:44:16 PDT 2025
#    Added multi domain and multi component tests.
# ----------------------------------------------------------------------------

#################################################

#
# no ghosts, single domain, single component
#
curv3d_stats = {}

# zonal stats
curv3d_stats["max_d/0"] = 4.9557
curv3d_stats["min_d/0"] = 2.03471
curv3d_stats["num_d/0"] = 36000 # num non-ghosted zones
curv3d_stats["sum_d/0"] = 125827.320313
curv3d_stats["avg_d/0"] = 3.4952
curv3d_stats["std_d/0"] = 0.864568 # standard deviation
curv3d_stats["var_d/0"] = 0.747478 # variance
curv3d_stats["rms_d/0"] = 3.60055 # root mean square

# nodal stats
curv3d_stats["max_u/0"] = 1
curv3d_stats["min_u/0"] = -1
curv3d_stats["num_u/0"] = 39401 # num non-ghosted nodes
curv3d_stats["sum_u/0"] = -93
curv3d_stats["avg_u/0"] = -0.002360
curv3d_stats["std_u/0"] = 0.740144 # standard deviation
curv3d_stats["var_u/0"] = 0.547814 # variance
curv3d_stats["rms_u/0"] = 0.740148 # root mean square

#################################################

#
# yes ghosts, multi-domain, single component
#
multi_curv3d_stats = {}

# zonal stats
multi_curv3d_stats["max_d/0"] = 4.9557
multi_curv3d_stats["min_d/0"] = 2.03471
multi_curv3d_stats["num_d/0"] = 36000 # num non-ghosted zones
multi_curv3d_stats["sum_d/0"] = 125827.335938
multi_curv3d_stats["avg_d/0"] = 3.4952
multi_curv3d_stats["std_d/0"] = 0.864568 # standard deviation
multi_curv3d_stats["var_d/0"] = 0.747478 # variance
multi_curv3d_stats["rms_d/0"] = 3.60055 # root mean square

# nodal stats
multi_curv3d_stats["max_u/0"] = 1
multi_curv3d_stats["min_u/0"] = -1
multi_curv3d_stats["num_u/0"] = 31958 # num non-ghosted nodes
multi_curv3d_stats["sum_u/0"] = -57.868796
multi_curv3d_stats["avg_u/0"] = -0.001811
multi_curv3d_stats["std_u/0"] = 0.741448 # standard deviation
multi_curv3d_stats["var_u/0"] = 0.549746 # variance
multi_curv3d_stats["rms_u/0"] = 0.741450 # root mean square

#################################################

#
# yes ghosts, single domain, single component
#
curv2d_stats = {}

# zonal stats
curv2d_stats["max_d/0"] = 4.8808
curv2d_stats["min_d/0"] = 2.1096
curv2d_stats["num_d/0"] = 988 # num non-ghosted zones
curv2d_stats["sum_d/0"] = 3453.260927
curv2d_stats["avg_d/0"] = 3.4952
curv2d_stats["std_d/0"] = 0.821312 # standard deviation
curv2d_stats["var_d/0"] = 0.674554 # variance
curv2d_stats["rms_d/0"] = 3.5904 # root mean square

# nodal stats
curv2d_stats["max_u/0"] = 1
curv2d_stats["min_u/0"] = -1
curv2d_stats["num_u/0"] = 1053 # num non-ghosted nodes
curv2d_stats["sum_u/0"] = 67.700714
curv2d_stats["avg_u/0"] = 0.064293
curv2d_stats["std_u/0"] = 0.712294 # standard deviation
curv2d_stats["var_u/0"] = 0.507364 # variance
curv2d_stats["rms_u/0"] = 0.715191 # root mean square

#################################################

#
# yes ghosts, multi domain, single component
#
multi_curv2d_stats = {}

# zonal stats
multi_curv2d_stats["max_d/0"] = 4.955698
multi_curv2d_stats["min_d/0"] = 2.034708
multi_curv2d_stats["num_d/0"] = 1200 # num non-ghosted zones
multi_curv2d_stats["sum_d/0"] = 4194.244629
multi_curv2d_stats["avg_d/0"] = 3.4952
multi_curv2d_stats["std_d/0"] = 0.864568 # standard deviation
multi_curv2d_stats["var_d/0"] = 0.747477 # variance
multi_curv2d_stats["rms_d/0"] = 3.60054541 # root mean square

# nodal stats
multi_curv2d_stats["max_u/0"] = 1
multi_curv2d_stats["min_u/0"] = -1
multi_curv2d_stats["num_u/0"] = 1435 # num non-ghosted nodes
multi_curv2d_stats["sum_u/0"] = 2.05621
multi_curv2d_stats["avg_u/0"] = 0.0014329
multi_curv2d_stats["std_u/0"] = 0.73645 # standard deviation
multi_curv2d_stats["var_u/0"] = 0.542359 # variance
multi_curv2d_stats["rms_u/0"] = 0.73645139 # root mean square

#################################################

#
# no ghosts, single domain, multi component
#
curv3d_3_comp_stats = {}

# zonal stats
# unchanged from "d"
curv3d_3_comp_stats["max_darr/0"] = 4.9557
curv3d_3_comp_stats["min_darr/0"] = 2.03471
curv3d_3_comp_stats["num_darr/0"] = 36000 # num non-ghosted zones
curv3d_3_comp_stats["sum_darr/0"] = 125827.320313
curv3d_3_comp_stats["avg_darr/0"] = 3.4952
curv3d_3_comp_stats["std_darr/0"] = 0.864568 # standard deviation
curv3d_3_comp_stats["var_darr/0"] = 0.747478 # variance
curv3d_3_comp_stats["rms_darr/0"] = 3.60055 # root mean square

# "d" times 6
curv3d_3_comp_stats["max_darr/1"] = 29.7342
curv3d_3_comp_stats["min_darr/1"] = 12.20824528
curv3d_3_comp_stats["num_darr/1"] = 36000 # num non-ghosted zones
curv3d_3_comp_stats["sum_darr/1"] = 754963.9375
curv3d_3_comp_stats["avg_darr/1"] = 20.971220
curv3d_3_comp_stats["std_darr/1"] = 5.187408 # standard deviation
curv3d_3_comp_stats["var_darr/1"] = 26.909216 # variance
curv3d_3_comp_stats["rms_darr/1"] = 21.603270 # root mean square

# "d" times 3
curv3d_3_comp_stats["max_darr/2"] = 14.8671
curv3d_3_comp_stats["min_darr/2"] = 6.104123
curv3d_3_comp_stats["num_darr/2"] = 36000 # num non-ghosted zones
curv3d_3_comp_stats["sum_darr/2"] = 377481.96875
curv3d_3_comp_stats["avg_darr/2"] = 10.485610
curv3d_3_comp_stats["std_darr/2"] = 2.593704 # standard deviation
curv3d_3_comp_stats["var_darr/2"] = 6.727302 # variance
curv3d_3_comp_stats["rms_darr/2"] = 10.801635 # root mean square

# nodal stats
# unchanged from "u"
curv3d_3_comp_stats["max_uarr/0"] = 1
curv3d_3_comp_stats["min_uarr/0"] = -1
curv3d_3_comp_stats["num_uarr/0"] = 39401 # num non-ghosted nodes
curv3d_3_comp_stats["sum_uarr/0"] = -93
curv3d_3_comp_stats["avg_uarr/0"] = -0.002360
curv3d_3_comp_stats["std_uarr/0"] = 0.740144 # standard deviation
curv3d_3_comp_stats["var_uarr/0"] = 0.547814 # variance
curv3d_3_comp_stats["rms_uarr/0"] = 0.740148 # root mean square

# "u" times 6
curv3d_3_comp_stats["max_uarr/1"] = 6
curv3d_3_comp_stats["min_uarr/1"] = -6
curv3d_3_comp_stats["num_uarr/1"] = 39401 # num non-ghosted nodes
curv3d_3_comp_stats["sum_uarr/1"] = -558
curv3d_3_comp_stats["avg_uarr/1"] = -0.014162077568471432
curv3d_3_comp_stats["std_uarr/1"] = 4.440866947174072 # standard deviation
curv3d_3_comp_stats["var_uarr/1"] = 19.721298217773438 # variance
curv3d_3_comp_stats["rms_uarr/1"] = 4.440889358520508 # root mean square

# "u" times 3
curv3d_3_comp_stats["max_uarr/2"] = 3
curv3d_3_comp_stats["min_uarr/2"] = -3
curv3d_3_comp_stats["num_uarr/2"] = 39401 # num non-ghosted nodes
curv3d_3_comp_stats["sum_uarr/2"] = -279
curv3d_3_comp_stats["avg_uarr/2"] = -0.007081038784235716
curv3d_3_comp_stats["std_uarr/2"] = 2.220433473587036 # standard deviation
curv3d_3_comp_stats["var_uarr/2"] = 4.930324554443359 # variance
curv3d_3_comp_stats["rms_uarr/2"] = 2.220444679260254 # root mean square

#################################################

#
# yes ghosts, multi domain, multi component
#
multi_curv2d_2_comp_stats = {}

# zonal stats
# unchanged from "d"
multi_curv2d_2_comp_stats["max_darr/0"] = 4.955698
multi_curv2d_2_comp_stats["min_darr/0"] = 2.034708
multi_curv2d_2_comp_stats["num_darr/0"] = 1200 # num non-ghosted zones
multi_curv2d_2_comp_stats["sum_darr/0"] = 4194.244629
multi_curv2d_2_comp_stats["avg_darr/0"] = 3.4952
multi_curv2d_2_comp_stats["std_darr/0"] = 0.864568 # standard deviation
multi_curv2d_2_comp_stats["var_darr/0"] = 0.747477 # variance
multi_curv2d_2_comp_stats["rms_darr/0"] = 3.60054541 # root mean square

# "d" times 2
multi_curv2d_2_comp_stats["max_darr/1"] = 9.911396026611328
multi_curv2d_2_comp_stats["min_darr/1"] = 4.069415092468262
multi_curv2d_2_comp_stats["num_darr/1"] = 1200 # num non-ghosted zones
multi_curv2d_2_comp_stats["sum_darr/1"] = 8388.4892578125
multi_curv2d_2_comp_stats["avg_darr/1"] = 6.990407466888428
multi_curv2d_2_comp_stats["std_darr/1"] = 1.7291361093521118 # standard deviation
multi_curv2d_2_comp_stats["var_darr/1"] = 2.9899115562438965 # variance
multi_curv2d_2_comp_stats["rms_darr/1"] = 7.2010908126831055 # root mean square

# nodal stats
# unchanged from "u"
multi_curv2d_2_comp_stats["max_uarr/0"] = 1
multi_curv2d_2_comp_stats["min_uarr/0"] = -1
multi_curv2d_2_comp_stats["num_uarr/0"] = 1435 # num non-ghosted nodes
multi_curv2d_2_comp_stats["sum_uarr/0"] = 2.05621
multi_curv2d_2_comp_stats["avg_uarr/0"] = 0.0014329
multi_curv2d_2_comp_stats["std_uarr/0"] = 0.73645 # standard deviation
multi_curv2d_2_comp_stats["var_uarr/0"] = 0.542359 # variance
multi_curv2d_2_comp_stats["rms_uarr/0"] = 0.73645139 # root mean square

# "u" times 2
multi_curv2d_2_comp_stats["max_uarr/1"] = 2
multi_curv2d_2_comp_stats["min_uarr/1"] = -2
multi_curv2d_2_comp_stats["num_uarr/1"] = 1435 # num non-ghosted nodes
multi_curv2d_2_comp_stats["sum_uarr/1"] = 4.1124138832092285
multi_curv2d_2_comp_stats["avg_uarr/1"] = 0.002865793649107218
multi_curv2d_2_comp_stats["std_uarr/1"] = 1.4728999137878418 # standard deviation
multi_curv2d_2_comp_stats["var_uarr/1"] = 2.1694343090057373 # variance
multi_curv2d_2_comp_stats["rms_uarr/1"] = 1.472902774810791 # root mean square

#################################################

baseline_stats = {}
baseline_stats["curv3d.silo/curvmesh3d"]             = curv3d_stats
baseline_stats["multi_curv3d.silo/mesh1"]            = multi_curv3d_stats
baseline_stats["curv2d.silo/curvmesh2d"]             = curv2d_stats
baseline_stats["multi_curv2d.silo/mesh1"]            = multi_curv2d_stats
baseline_stats["curv3d_multi_comp.silo/curvmesh3d"]  = curv3d_3_comp_stats
baseline_stats["multi_curv2d_multi_comp.silo/mesh1"] = multi_curv2d_2_comp_stats

def test_stat(shortstatname, longstatname, filename, meshname, varname, vartype, ncomps=1):
	plot_varname = shortstatname + "_" + varname + "_" + vartype
	complist = ["0"]
	if ncomps > 1:
		complist.append("1")
	if ncomps > 2:
		complist.append("2")
	for comp in complist:
		if ncomps > 1:
			DefineScalarExpression(plot_varname + comp, plot_varname + "[" + comp + "]")
			AddPlot("Pseudocolor", plot_varname + comp)
		else:
			AddPlot("Pseudocolor", plot_varname)
		DrawPlots()
		Query("MinMax")
		q = GetQueryOutputObject()
		# we test both the min and max because we want to ensure the variable is constant
		# across the mesh.
		TestValueEQ("Test " + longstatname + " 1 for " + varname + comp, q['min'], 
			baseline_stats[filename + "/" + meshname][shortstatname + "_" + varname + "/" + comp])
		TestValueEQ("Test " + longstatname + " 2 for " + varname + comp, q['max'], 
			baseline_stats[filename + "/" + meshname][shortstatname + "_" + varname + "/" + comp])
		DeleteAllPlots()

def test_stats_for_var(filename, meshname, varname, vartype, ncomps=1):
	if ncomps > 1:
		# define our expressions
		if vartype == "zonal":
			# every zone will have a value of 1
			DefineScalarExpression("one0", "zonal_constant(" + meshname + ", 1)")
			DefineScalarExpression("one1", "zonal_constant(" + meshname + ", 1)")
			DefineScalarExpression("one2", "zonal_constant(" + meshname + ", 1)")
		else:
			# every node will have a value of 1
			DefineScalarExpression("one0", "nodal_constant(" + meshname + ", 1)")
			DefineScalarExpression("one1", "nodal_constant(" + meshname + ", 1)")
			DefineScalarExpression("one2", "nodal_constant(" + meshname + ", 1)")
		DefineArrayExpression("one", "array_compose(one0,one1,one2)")
		DefineArrayExpression("avg_" + varname + "_" + vartype, "global_avg(" + varname + ")")
		DefineArrayExpression("max_" + varname + "_" + vartype, "global_max(" + varname + ")")
		DefineArrayExpression("min_" + varname + "_" + vartype, "global_min(" + varname + ")")
		DefineArrayExpression("num_" + varname + "_" + vartype, "global_sum(one)")
		DefineArrayExpression("rms_" + varname + "_" + vartype, "global_rms(" + varname + ")")
		DefineArrayExpression("std_" + varname + "_" + vartype, "global_std_dev(" + varname + ")")
		DefineArrayExpression("sum_" + varname + "_" + vartype, "global_sum(" + varname + ")")
		DefineArrayExpression("var_" + varname + "_" + vartype, "global_variance(" + varname + ")")
	else:
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

	test_stat("avg", "Average",            filename, meshname, varname, vartype, ncomps)
	test_stat("max", "Maximum",            filename, meshname, varname, vartype, ncomps)
	test_stat("min", "Minimum",            filename, meshname, varname, vartype, ncomps)
	test_stat("num", "Number",             filename, meshname, varname, vartype, ncomps)
	test_stat("rms", "Root Mean Square",   filename, meshname, varname, vartype, ncomps)
	test_stat("std", "Standard Deviation", filename, meshname, varname, vartype, ncomps)
	test_stat("sum", "Sum",                filename, meshname, varname, vartype, ncomps)
	test_stat("var", "Variance",           filename, meshname, varname, vartype, ncomps)

	DeleteExpression("one")
	DeleteExpression("avg_" + varname + "_" + vartype)
	DeleteExpression("max_" + varname + "_" + vartype)
	DeleteExpression("min_" + varname + "_" + vartype)
	DeleteExpression("num_" + varname + "_" + vartype)
	DeleteExpression("rms_" + varname + "_" + vartype)
	DeleteExpression("std_" + varname + "_" + vartype)
	DeleteExpression("sum_" + varname + "_" + vartype)
	DeleteExpression("var_" + varname + "_" + vartype)

	if ncomps > 1:
		DeleteExpression("one0")
		DeleteExpression("one1")
		DeleteExpression("one2")


def single_component_tests():
	TestSection("No Ghosts, Single Domain, Single Component")
	OpenDatabase(silo_data_path("curv3d.silo"))
	test_stats_for_var(filename="curv3d.silo", meshname="curvmesh3d", varname="d", vartype="zonal")
	test_stats_for_var(filename="curv3d.silo", meshname="curvmesh3d", varname="u", vartype="nodal")
	CloseDatabase(silo_data_path("curv3d.silo"))

	TestSection("Yes Ghosts, Multi Domain, Single Component")
	OpenDatabase(silo_data_path("multi_curv3d.silo"))
	test_stats_for_var(filename="multi_curv3d.silo", meshname="mesh1", varname="d", vartype="zonal")
	test_stats_for_var(filename="multi_curv3d.silo", meshname="mesh1", varname="u", vartype="nodal")
	CloseDatabase(silo_data_path("multi_curv3d.silo"))

	TestSection("Yes Ghosts, Single Domain, Single Component")
	OpenDatabase(silo_data_path("curv2d.silo"))
	test_stats_for_var(filename="curv2d.silo", meshname="curvmesh2d", varname="d", vartype="zonal")
	test_stats_for_var(filename="curv2d.silo", meshname="curvmesh2d", varname="u", vartype="nodal")
	CloseDatabase(silo_data_path("curv2d.silo"))

	TestSection("Yes Ghosts, Multi Domain, Single Component")
	OpenDatabase(silo_data_path("multi_curv2d.silo"))
	test_stats_for_var(filename="multi_curv2d.silo", meshname="mesh1", varname="d", vartype="zonal")
	test_stats_for_var(filename="multi_curv2d.silo", meshname="mesh1", varname="u", vartype="nodal")
	CloseDatabase(silo_data_path("multi_curv2d.silo"))

def multi_component_tests():
	TestSection("No Ghosts, Single Domain, Multi Component")
	OpenDatabase(silo_data_path("curv3d.silo"))
	DefineScalarExpression("d1", 'd')
	DefineScalarExpression("d2", 'd * 6')
	DefineScalarExpression("d3", 'd * 3')
	DefineScalarExpression("u1", 'u')
	DefineScalarExpression("u2", 'u * 6')
	DefineScalarExpression("u3", 'u * 3')
	DefineArrayExpression("darr", "array_compose(d1,d2,d3)")
	DefineArrayExpression("uarr", "array_compose(u1,u2,u3)")
	test_stats_for_var(filename="curv3d_multi_comp.silo", meshname="curvmesh3d", varname="darr", vartype="zonal", ncomps=3)
	test_stats_for_var(filename="curv3d_multi_comp.silo", meshname="curvmesh3d", varname="uarr", vartype="nodal", ncomps=3)
	DeleteExpression("d1")
	DeleteExpression("d2")
	DeleteExpression("d3")
	DeleteExpression("u1")
	DeleteExpression("u2")
	DeleteExpression("u3")
	DeleteExpression("darr")
	DeleteExpression("uarr")
	CloseDatabase(silo_data_path("curv3d.silo"))

	TestSection("Yes Ghosts, Multi Domain, Multi Component")
	OpenDatabase(silo_data_path("multi_curv2d.silo"))
	DefineScalarExpression("d1", 'd')
	DefineScalarExpression("d2", 'd * 2')
	DefineScalarExpression("u1", 'u')
	DefineScalarExpression("u2", 'u * 2')
	DefineArrayExpression("darr", "array_compose(d1,d2)")
	DefineArrayExpression("uarr", "array_compose(u1,u2)")
	test_stats_for_var(filename="multi_curv2d_multi_comp.silo", meshname="mesh1", varname="darr", vartype="zonal", ncomps=2)
	test_stats_for_var(filename="multi_curv2d_multi_comp.silo", meshname="mesh1", varname="uarr", vartype="nodal", ncomps=2)
	DeleteExpression("d1")
	DeleteExpression("d2")
	DeleteExpression("u1")
	DeleteExpression("u2")
	DeleteExpression("darr")
	DeleteExpression("uarr")
	CloseDatabase(silo_data_path("multi_curv2d.silo"))

single_component_tests()
multi_component_tests()

Exit()
