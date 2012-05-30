# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  mir.py
#
#  Tests:      mesh      - 2D, 3D, structured, unstructured, single domain
#              plots     - FilledBoundary
#
#  Programmer: Jeremy Meredith
#  Date:       March 24, 2010
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def TestOneMode(file, mat, name):
    SetMaterialAttributes(mat)
    ReOpenDatabase(file)
    Test(name)
    

def TestOneFile(isRectilinear, file, matname, prefix):
    OpenDatabase(file)

    AddPlot("FilledBoundary", matname)
    DrawPlots()
    
    v3 = GetView3D()
    v3.viewNormal = (0.3, 0.4, 0.8)
    SetView3D(v3)

    m = MaterialAttributes()

    m.algorithm = m.EquiT
    TestOneMode(file, m, prefix + "_equi_t")

    m.algorithm = m.EquiZ
    m.iterationEnabled = False
    TestOneMode(file, m, prefix + "_equi_z")

    m.algorithm = m.EquiZ
    m.iterationEnabled = True
    m.numIterations = 5
    m.iterationDamping = 0.4
    TestOneMode(file, m, prefix + "_equi_z_i5")
    m.iterationEnabled = False
    
    m.algorithm = m.PLIC
    TestOneMode(file, m, prefix + "_plic")

    m.algorithm = m.Isovolume
    TestOneMode(file, m, prefix + "_isovol")

    # Discrete currently only works on rectilinear
    if isRectilinear:
        m.algorithm = m.Discrete
        # Discrete is stochastic, we can't test it reliably
        #TestOneMode(file, m, prefix + "_discrete")

    DeleteAllPlots()
    CloseDatabase(file)

TestSection("Curvilinear, 2D")
TestOneFile(False,silo_data_path("specmix_quad.silo") , "Material", "mir_curv2d")
TestSection("Unstructured, 2D")
TestOneFile(False,silo_data_path("specmix_ucd.silo") , "Material", "mir_ucd2d")
TestSection("Rectilinear, 2D")
TestOneFile(True,silo_data_path("rect2d.silo") , "mat1", "mir_rect2d")
TestSection("Rectilinear, 3D")
TestOneFile(True,silo_data_path("rect3d.silo") , "mat1", "mir_rect3d")

Exit()
