# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  merge_tree.py
#
#
#  Programmer: Cyrus Harrison
#  Date:       Friday October 28, 2016
#
#  Modifiations:
#
# ----------------------------------------------------------------------------


test_dbs = { "rect2d" : {"var":  "d",
                         "path": silo_data_path("rect2d.silo")} ,
             "noise2d" : {"var": "shepardglobal",
                          "path" :silo_data_path("noise2d.silo")} ,

             "noise3d" : {"var": "shepardglobal",
                          "path": silo_data_path("noise.silo")}
           }

for db_name,db_info in list(test_dbs.items()):
    test_db_file = db_info["path"]
    OpenDatabase(test_db_file)

    # test merge tree, split tree and local threshold exprs

    DefineScalarExpression("mtree", "merge_tree(%s)" % db_info["var"])
    AddPlot("Pseudocolor", "mtree")
    DrawPlots()

    Test(db_name + "_mtree")
    DeleteAllPlots()

    DefineScalarExpression("stree", "split_tree(%s)" % db_info["var"])
    AddPlot("Pseudocolor", "stree")
    DrawPlots()
    
    Test(db_name + "_stree")
    DeleteAllPlots()
    
    DefineScalarExpression("localt_mt", "local_threshold(%s,mtree)" % db_info["var"])

    AddPlot("Pseudocolor", "localt_mt")
    DrawPlots()
    
    Test(db_name + "_localt_mt")
    DeleteAllPlots()

    DefineScalarExpression("localt_st", "local_threshold(%s,stree)" % db_info["var"])

    AddPlot("Pseudocolor", "localt_st")
    DrawPlots()
    
    Test(db_name + "_localt_st")
    DeleteAllPlots()

    
    CloseDatabase(test_db_file)

Exit()
