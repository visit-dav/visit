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
#  Date:       Fri Feb  4 17:07:24 EST 2022
#
#  Modifications:
#
# ----------------------------------------------------------------------------
try:
    import numpy as np
    haveNumpy = True
except:
    haveNumpy = False

import os

# NOTE: The following few functions are useful for running this test
#  script through the regular visit CLI
# def silo_data_path(name):
#     return os.path.join("/home/cdl/Development/visit/visit/data/silo_hdf5_test_data",
#         name)

# def Exit():
#     exit()

# def TestSection(name):
#     print(name)

# def TestText(baseline_file, value):
#     print("Loading up baseline file %s" % os.path.abspath(baseline_file))

# def TestValueEQ(name, v0, v1):
#     assert v0 == v1, str(name) + ": " + str(v0) + " != " + str(v1)

# def TestValueIN(name, d, k):
#     assert k in d, str(name) + ": " + str(v0) + " !in " + str(v1)

n = 0
basicTestCases = (
    (silo_data_path('rect2d.silo'), 'quadmesh2d'),
    (silo_data_path('rect3d.silo'), 'quadmesh3d'),
    (silo_data_path('curv2d.silo'), 'curvmesh2d'),
    (silo_data_path('curv3d.silo'), 'curvmesh3d'),
    (silo_data_path('ucd2d.silo'), 'ucdmesh2d'),
    (silo_data_path('ucd3d.silo'), 'ucdmesh3d'),
)


def load_text(name):
    fname = name + '.txt'
    print("Loading up current file %s" % os.path.abspath(fname))
    with open(fname) as f:
        return f.read()

def define_mesh_expressions(mesh_name):
    DefineScalarExpression("nid", "nodeid({})".format(mesh_name))
    DefineScalarExpression("zid", "zoneid({})".format(mesh_name))
    DefineVectorExpression("vec_nid", "{-nid,nid,-nid}")
    DefineVectorExpression("vec_zid", "{-zid,zid,-zid}")

def test_basic_table(table):
    data = np.asarray(table)
    ncol = data.shape[1]
    TestValueEQ("NumCols", ncol, 6)

    badCol0 = -1
    badCol1 = -1
    badCol2 = -1
    badCol3 = -1
    badCol4 = -1
    badCol5 = -1
    nrow = data.shape[0]
    for i in range(0, nrow):
        badCol0 = -1 if np.isclose(data[i, 0],  i) and badCol0 < 0 else i
        badCol1 = -1 if np.isclose(data[i, 1], -i) and badCol1 < 0 else i
        badCol2 = -1 if np.isclose(data[i, 2],  i) and badCol2 < 0 else i
        badCol3 = -1 if np.isclose(data[i, 3], -i) and badCol3 < 0 else i
        badCol4 = -1 if np.isclose(data[i, 4],  i) and badCol4 < 0 else i
        # All the of the basic tests should be 1 domain
        badCol5 = -1 if np.isclose(data[i, 5], 0) and badCol5 < 0 else i

    if badCol0 > -1:
        global n
        np.savetxt("table%d.csv" % n, data, delimiter=',')
        n = n + 1

    TestValueEQ("TestDataCol0", -1, badCol0)
    TestValueEQ("TestDataCol1", -1, badCol1)
    TestValueEQ("TestDataCol2", -1, badCol2)
    TestValueEQ("TestDataCol3", -1, badCol3)
    TestValueEQ("TestDataCol4", -1, badCol4)

def run_basic_test(db_name, mesh_name, test_type):
    OpenDatabase(db_name)
    define_mesh_expressions(mesh_name)
    AddPlot('Mesh', mesh_name)
    DrawPlots()

    vars = tuple()
    if test_type == 'nz' or test_type == 'zn':
        vars = ('nid', 'zid', 'vec_nid', 'vec_zid')
    elif 'n' in test_type:
        vars = ('nid', 'vec_nid')
    elif 'z' in test_type:
        vars = ('zid', 'vec_zid')

    expect_node_table = 'n' in test_type
    expect_zone_table = 'z' in test_type

    tables = Flatten(vars, nodeIds=expect_node_table,
        zoneIds=expect_zone_table, nodeIJK=False, zoneIJK=False)

    have_node_table = 'nodeTable' in tables
    if expect_node_table:
        TestValueEQ("ShouldHaveNodeTable", have_node_table, True)
        TestValueIN("ShouldHaveNodeColumnNames", tables, 'nodeColumnNames')
        column_names = tables['nodeColumnNames']
        # print(column_names)
        TestValueEQ('NodeCol0Name', column_names[0], 'nid')
        TestValueEQ('NodeCol1Name', column_names[1], 'vec_nid/c0')
        TestValueEQ('NodeCol2Name', column_names[2], 'vec_nid/c1')
        TestValueEQ('NodeCol3Name', column_names[3], 'vec_nid/c2')
        TestValueEQ('NodeCol4Name', column_names[4], 'nodeIds')
        TestValueEQ('NodeCol5Name', column_names[5], 'nodeDomains')
        test_basic_table(tables['nodeTable'])
    else:
        TestValueEQ("ShouldNotHaveNodeTable", have_node_table, False)

    have_zone_table = 'zoneTable' in tables
    if expect_zone_table:
        TestValueEQ("ShouldHaveZoneTable", have_zone_table, True)
        TestValueIN("ShouldHaveZoneColumnNames", tables, 'zoneColumnNames')
        column_names = tables['zoneColumnNames']
        # print(column_names)
        TestValueEQ('ZoneCol0Name', column_names[0], 'zid')
        TestValueEQ('ZoneCol1Name', column_names[1], 'vec_zid/c0')
        TestValueEQ('ZoneCol2Name', column_names[2], 'vec_zid/c1')
        TestValueEQ('ZoneCol3Name', column_names[3], 'vec_zid/c2')
        TestValueEQ('ZoneCol4Name', column_names[4], 'zoneIds')
        TestValueEQ('ZoneCol5Name', column_names[5], 'zoneDomains')
        test_basic_table(tables['zoneTable'])
    else:
        TestValueEQ("ShouldNotHaveZoneTable", have_zone_table, False)

    DeleteAllPlots()
    CloseDatabase(db_name)

def test_box(forceNoShm):
    db_name = silo_data_path('multi_rect2d.silo')
    mesh_name = 'mesh1'

    OpenDatabase(db_name)
    define_mesh_expressions(mesh_name)
    AddPlot('Mesh', mesh_name)
    AddOperator('Box')
    opts = GetOperatorOptions(0)
    opts.maxy = 0.1
    SetOperatorOptions(opts)
    DrawPlots()

    vars = ('vec_nid', 'vec_zid')
    tables = Flatten(vars, nodeIds=True,
        zoneIds=True, nodeIJK=False, zoneIJK=False,
        forceNoSharedMemory=forceNoShm)

    TestValueIN('ShouldHaveNodeColumnNames', tables, 'nodeColumnNames')
    TestValueIN('ShouldHaveNodeTable', tables, 'nodeTable')
    nodeTable = np.asarray(tables['nodeTable'])

    TestValueIN('ShouldHaveZoneColumnNames', tables, 'zoneColumnNames')
    TestValueIN('ShouldHaveZoneTable', tables, 'zoneTable')
    zoneTable = np.asarray(tables['zoneTable'])

    ntName = 'multi_rect2d_box_nodes'
    ztName = 'multi_rect2d_box_zones'
    np.savetxt(ntName+'.txt', nodeTable, delimiter=',',
        header=','.join(tables['nodeColumnNames']))
    np.savetxt(ztName+'.txt', zoneTable, delimiter=',',
        header=','.join(tables['zoneColumnNames']))
    TestText(ntName, load_text(ntName))
    TestText(ztName, load_text(ztName))

    DeleteAllPlots()
    CloseDatabase(db_name)

def test_tensor():
    TestSection('NoiseWithTensor')
    db_name = silo_data_path('noise.silo')
    mesh_name = 'Mesh'

    OpenDatabase(db_name)
    AddPlot('Mesh', mesh_name)

    # Only want a small section for the csv file
    AddOperator('Box')
    opts = GetOperatorOptions(0)
    opts.minx = -10.0
    opts.maxx = -9.0
    opts.miny = -10.0
    opts.maxy = -9.0
    opts.minz = -10.0
    opts.maxz = -9.0
    SetOperatorOptions(opts)
    DrawPlots()

    vars = ('hardyglobal', 'grad_tensor')
    tables = Flatten(vars, nodeIds=True, zoneIds=False,
                        nodeIJK=False, zoneIJK=False)

    TestValueIN('ShouldHaveNodeColumnNames', tables, 'nodeColumnNames')
    TestValueIN('ShouldHaveNodeTable', tables, 'nodeTable')
    nodeTable = np.asarray(tables['nodeTable'])

    ntName = 'noise_nodes'
    np.savetxt(ntName+'.txt', nodeTable, delimiter=',',
        header=','.join(tables['nodeColumnNames']))
    TestText(ntName, load_text(ntName))

    DeleteAllPlots()
    CloseDatabase(db_name)

def test_ijk(do3d):
    db_name = silo_data_path('rect3d.silo') if do3d else silo_data_path('rect2d.silo')
    mesh_name = 'quadmesh3d' if do3d else 'quadmesh2d'

    OpenDatabase(db_name)
    AddPlot('Mesh', mesh_name)
    DrawPlots()

    vars = ('d',)
    tables = Flatten(vars, nodeIds=False, zoneIds=False, nodeIJK=True, zoneIJK=True)

    TestValueIN('ShouldHaveNodeColumnNames', tables, 'nodeColumnNames')
    TestValueIN('ShouldHaveNodeTable', tables, 'nodeTable')
    TestValueIN('ShouldHaveZoneColumnNames', tables, 'zoneColumnNames')
    TestValueIN('ShouldHaveZoneTable', tables, 'zoneTable')

    # Test rect2D and rect3D nodes
    nodeTable = np.asarray(tables['nodeTable'])
    row = -1
    dims = (31, 41, 31) if do3d else (31, 41, 1)
    for k in range(0, dims[2]):
        knxny = k * dims[0] * dims[1]
        for j in range(0, dims[1]):
            jnx = j * dims[0]
            for i in range(0, dims[0]):
                id = knxny + jnx + i
                iok = np.isclose(nodeTable[id, 0], i)
                jok = np.isclose(nodeTable[id, 1], j)
                kok = np.isclose(nodeTable[id, 2], k)
                if not (iok and jok and kok):
                    print('(%d,%d,%d)' % (i, j, k))
                    print('(%g,%g,%g)' % (nodeTable[id,0], nodeTable[id,1], nodeTable[id,2]))
                    row = id
                    break
            if row > -1:
                break
        if row > -1:
            break

    if row > -1:
        baseline_name = 'NodeIJK3D' if do3d else 'NodeIJK2D'
        np.savetxt(baseline_name + '.txt', nodeTable, delimiter=',',
            header=','.join(tables['nodeColumnNames']))
    TestValueEQ('NodeInvalidRow', row, -1)

    # Test rect2D and rect3D zones
    zoneTable = np.asarray(tables['zoneTable'])
    row = -1
    dims = (30, 40, 30) if do3d else (30, 40, 1)
    for k in range(0, dims[2]):
        knxny = k * dims[0] * dims[1]
        for j in range(0, dims[1]):
            jnx = j * dims[0]
            for i in range(0, dims[0]):
                id = knxny + jnx + i
                # NOTE: Column 0 is 'd'
                iok = np.isclose(zoneTable[id, 1], i)
                jok = np.isclose(zoneTable[id, 2], j)
                kok = np.isclose(zoneTable[id, 3], k)
                if not (iok and jok and kok):
                    print('(%d,%d,%d)' % (i, j, k))
                    print('(%g,%g,%g)' % (zoneTable[id,1], zoneTable[id,2], zoneTable[id,3]))
                    row = id
                    break
            if row > -1:
                break
        if row > -1:
            break

    if row > -1:
        baseline_name = 'ZoneIJK3D' if do3d else 'ZoneIJK2D'
        np.savetxt(baseline_name + '.txt', zoneTable, delimiter=',',
            header=','.join(tables['zoneColumnNames']))
    TestValueEQ('ZoneInvalidRow', row, -1)

    DeleteAllPlots()
    CloseDatabase(db_name)

def test_node_data():
    for case in basicTestCases:
        TestSection(case[0] + '_' + 'NodeData')
        run_basic_test(case[0], case[1], 'n')

def test_zone_data():
    for case in basicTestCases:
        TestSection(case[0] + '_' + 'ZoneData')
        run_basic_test(case[0], case[1], 'z')

def test_both():
    for case in basicTestCases:
        TestSection(case[0] + '_' + 'BothData')
        run_basic_test(case[0], case[1], 'nz')

def test_box_selection():
    # These test verify that the Query operates on
    #   the actual data and not the original
    # A second test is used to test the non shared
    #   memory implementation since the data is small.
    TestSection('BoxSelection')
    test_box(False)
    TestSection('BoxSelectionNoSharedMemory')
    test_box(True)

def test_ijks():
    TestSection('IJKs2D')
    test_ijk(False)
    TestSection('IJKs3D')
    test_ijk(True)

def test_all():
    test_node_data()
    test_zone_data()
    test_both()
    test_box_selection()
    test_tensor()
    test_ijks()

if haveNumpy:
    test_all()
Exit()