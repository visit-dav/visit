# ----------------------------------------------------------------------------
#  MODES: serial parallel
#  CLASSES: nightly
#
#  Test Case:  flatten.py 
#
#  Tests:      Tests CLI Flatten() function
#              (which is avtFlattenQuery underneath)
#
#  Programmer: Christopher Laganella
#  Date:       Thu Jan 27 11:51:09 EST 2022
#
#  Modifications:
#
# ----------------------------------------------------------------------------
try:
    import numpy as np
    haveNumpy = True
except:
    haveNumpy = False

n = 0

import os

def Exit():
    exit()

def silo_data_path(name):
    return os.path.join("/home/cdl/Development/visit/data/silo_hdf5_test_data",
        name)

def TestValueEQ(name, v0, v1):
    assert v0 == v1, name

def TestValueIN(name, v0, v1):
    assert v0 in v1, name

def define_mesh_expressions(mesh_name):
    DefineScalarExpression("nid", "nodeid({})".format(mesh_name))
    DefineScalarExpression("zid", "zoneid({})".format(mesh_name))
    DefineVectorExpression("vec_nid", "{0,nid,1}")
    DefineVectorExpression("vec_zid", "{0,zid,1}")
    DefineScalarExpression('hardyglobal2', 'hardyglobal / hardyglobal')

def test_basic_table(table):
    data = np.asarray(table)
    ncol = data.shape[1]
    TestValueEQ("NumCols", ncol, 4)

    nrow = data.shape[0]
    # for i in range(0, nrow):
    #     TestValueEQ("TestDataCol0", i, data[i, 0])
    #     TestValueEQ("TestDataCol1", 0, data[i, 1])
    #     TestValueEQ("TestDataCol2", i, data[i, 2])
    #     TestValueEQ("TestDataCol3", 1, data[i, 3])
    global n
    np.savetxt("table%d.csv" % n, data, delimiter=',')
    n = n + 1

def run_basic_test(db_name, mesh_name, test_type):
    print("Flattening {}".format(db_name))

    OpenDatabase(db_name)
    define_mesh_expressions(mesh_name)
    AddPlot("Pseudocolor", "hardyglobal")
    DrawPlots()

    vars = tuple()
    if test_type == 'nz' or test_type == 'zn':
        vars = ('nid', 'zid', 'vec_nid', 'vec_zid')
    elif 'n' in test_type:
        vars = ('nid', 'vec_nid')
    elif 'z' in test_type:
        vars = ('zid', 'vec_zid')

    # vars = ('hardyglobal', 'hardyglobal2')
    os.system('rm *.html')
    tables = Flatten(vars)

    # expect_node_table = 'n' in test_type
    # have_node_table = 'nodeTable' in tables
    # if expect_node_table:
    #     TestValueEQ("ShouldHaveNodeTable", have_node_table, True)
    #     TestValueIN("ShouldHaveNodeColumnNames", 'nodeColumnNames', tables)
    #     column_names = tables['nodeColumnNames']
    #     print(column_names)
    #     TestValueEQ('NodeCol0Name', column_names[0], 'nid')
    #     TestValueEQ('NodeCol1Name', column_names[1], 'vec_nid/c0')
    #     TestValueEQ('NodeCol2Name', column_names[2], 'vec_nid/c1')
    #     TestValueEQ('NodeCol3Name', column_names[3], 'vec_nid/c2')
    #     test_basic_table(tables['nodeTable'])
    # else:
    #     TestValueEQ("ShouldNotHaveNodeTable", have_node_table, False)

    # expect_zone_table = 'z' in test_type
    # have_zone_table = 'zoneTable' in tables
    # if expect_zone_table:
    #     TestValueEQ("ShouldHaveZoneTable", have_zone_table, True)
    #     TestValueIN("ShouldHaveZoneColumnNames", 'zoneColumnNames', tables)
    #     column_names = tables['zoneColumnNames']
    #     print(column_names)
    #     TestValueEQ('ZoneCol0Name', column_names[0], 'zid')
    #     TestValueEQ('ZoneCol1Name', column_names[1], 'vec_zid/c0')
    #     TestValueEQ('ZoneCol2Name', column_names[2], 'vec_zid/c1')
    #     TestValueEQ('ZoneCol3Name', column_names[3], 'vec_zid/c2')
    #     test_basic_table(tables['zoneTable'])
    # else:
    #     TestValueEQ("ShouldNotHaveZoneTable", have_zone_table, False)

    data = np.asarray(tables['nodeTable'])
    # np.savetxt("table.csv", data, delimiter=',')

    DeleteAllPlots()
    CloseDatabase(db_name)
    print("Done flattening {}".format(db_name))

def test_node_data():
    run_basic_test(silo_data_path('noise.silo'), 'Mesh', 'n')
    # run_test(silo_data_path("multi_rect2d"), ('u', 'v', 'w', 'vec', 'mag'))
    # run_test(silo_data_path("multi_rect3d"), ('u', 'v', 'w', 'vec', 'mag'))
    # run_test(silo_data_path("multi_curv2d"), ('u', 'v', 'w', 'vec', 'mag'))
    # run_test(silo_data_path("multi_curv3d"), ('u', 'v', 'w', 'vec', 'mag'))
    # run_test(silo_data_path("mutli_ucd3d"), ('u', 'v', 'w', 'vec', 'mag'))

def test_zone_data():
    run_basic_test(silo_data_path('noise.silo'), 'Mesh', 'z')
    # run_test(silo_data_path("multi_rect2d"), ('d', 'p'))
    # run_test(silo_data_path("mutli_rect3d"), ('d', 'p'))
    # run_test(silo_data_path("multi_curv2d"), ('d', 'p'))
    # run_test(silo_data_path("multi_curv3d"), ('d', 'p'))
    # run_test(silo_data_path("multi_ucd3d"), ('d', 'p', 'hist'))


def test_both():
    run_basic_test(silo_data_path('noise.silo'), 'Mesh', 'nz')


def test_all():
    test_node_data()
    # test_zone_data()
    # test_both()

test_all()
Exit()