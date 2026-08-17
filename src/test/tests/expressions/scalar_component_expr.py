# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  scalar_component_expr.py
#
#  Tests:      Automatic construction of vector, tensor and symmetric tensor
#              expressions from related scalar component variable names.
#
#              Uses small 2D and 3D legacy VTK files containing deliberately
#              named nodal and zonal scalar variables.  The tests confirm that
#              the expected aggregate expressions are created, that known
#              false-positive names are not created, and that representative
#              inferred vector and tensor expressions can be plotted without
#              expression-evaluation errors.
#
#  ChatGPT via Mark C. Miller, Sun Aug 16 2026
#
# ----------------------------------------------------------------------------

#
# Return the names of all expressions advertised in database metadata.
#
def GetExpressions(db):
    md = GetMetaData(db)
    exprs = {}
    for i in range(md.GetExprList().GetNumExpressions()):
        e = md.GetExprList().GetExpressions(i)
        exprs[e.GetName()] = e.GetType()
    return exprs

#
# Test that all names in expected are present and all names in forbidden
# are absent.
#
def TestExpressionNamesAndTypes(section, db, expected, forbidden):
    TestSection(section)

    exprs = GetExpressions(db)

    for name, expectedType in expected.items():
        TestValueEQ("%s: expression '%s' exists" % (section, name),
                    name in exprs, True)

        if name in exprs:
            TestValueEQ("%s: expression '%s' type" % (section, name),
                        exprs[name], expectedType)

    for name in forbidden:
        TestValueEQ("%s: expression '%s' absent" % (section, name),
                    name in exprs, False)

#
# Exercise expression evaluation.  We deliberately do not call Test() because
# the rendered image is not important here.  Success means the plot can be
# added and drawn without an expression error.
#
def TestExpressionPlot(section, plotType, varName):
    TestSection(section)

    addResult = AddPlot(plotType, varName)
    TestValueEQ("%s: AddPlot" % section, addResult, 1)

    drawResult = DrawPlots()
    TestValueEQ("%s: DrawPlots" % section, drawResult, 1)

    DeleteAllPlots()


# ----------------------------------------------------------------------------
# 2D
# ----------------------------------------------------------------------------
def test_2d_scalar_comp_exprs():

    db2d = data_path("vtk_test_data/scalar_aggregate_test_2d.vtk")
    OpenDatabase(db2d)

    #
    # Expected automatic expressions:
    #
    #   disp        disp1, disp2
    #   vel         vel_x, vel_y, vel_z
    #   stress      stress_xx, stress_xy, stress_yx, stress_yy
    #   strain      strain.11, strain.12, strain.22
    #   metric      metric_11, metric_21, metric_22
    #
    # temp and zonev already exist as scalars and therefore must not be replaced
    # by inferred aggregate expressions.  badvec and partial are incomplete.
    #
    expected2d = {
        "disp"   : Expression().VectorMeshVar,
        "vel"    : Expression().VectorMeshVar,
        "v"      : Expression().VectorMeshVar,
        "stress" : Expression().TensorMeshVar,
        "strain" : Expression().SymmetricTensorMeshVar,
        "metric" : Expression().SymmetricTensorMeshVar
    }

    forbidden2d = (
        "badvec",
        "partial",

        # Existing scalar names must prevent aggregate expressions of same name.
        "temp",
        "zonev",

        # False positives produced by interpreting the final character of a
        # tensor component name as a vector component.
        "metric_2",
        "partial_1",
        "strain.1",
        "stress_x",
        "stress_y",

        # False positives formerly produced by the no-separator tensor RE.
        "metric_",
        "strain.",
        "stress_"
    )

    TestExpressionNamesAndTypes("2D automatic aggregate expressions",
                        db2d, expected2d, forbidden2d)

    TestExpressionPlot("2D inferred vector plots", "Vector", "vel")
    TestExpressionPlot("2D inferred tensor plots", "Tensor", "stress")

    CloseDatabase(db2d)


# ----------------------------------------------------------------------------
# 3D
# ----------------------------------------------------------------------------
def test_3d_scalar_comp_exprs():

    db3d = data_path("vtk_test_data/scalar_aggregate_test_3d.vtk")
    OpenDatabase(db3d)

    #
    # Expected automatic expressions:
    #
    #   disp        disp1, disp2, disp3
    #   vel         vel_x, vel_y, vel_z
    #   stress      complete 3x3 Cartesian tensor
    #   strain      6-component upper-triangular symmetric tensor
    #   metric      6-component lower-triangular u/v/w symmetric tensor
    #
    expected3d = {
        "disp"   : Expression().VectorMeshVar,
        "vel"    : Expression().VectorMeshVar,
        "v"      : Expression().VectorMeshVar,
        "stress" : Expression().TensorMeshVar,
        "strain" : Expression().SymmetricTensorMeshVar,
        "metric" : Expression().SymmetricTensorMeshVar
    }

    forbidden3d = (
        # Only x/y components are present; a 3D vector requires z too.
        "badvec",

        # Five components are insufficient for the 3D symmetric tensor.
        "partial",

        # Existing scalar names must win.
        "temp",
        "zonev",

        # Vector interpretations of tensor component names.
        "metric_u",
        "metric_v",
        "metric_w",
        "partial1",
        "partial2",
        "strain.1",
        "strain.2",
        "strain.3",
        "stress_x",
        "stress_y",
        "stress_z",

        # Greedy/no-separator tensor false positives.
        "metric_",
        "strain.",
        "stress_"
    )

    TestExpressionNamesAndTypes("3D automatic aggregate expressions",
                                db3d, expected3d, forbidden3d)

    TestExpressionPlot("3D inferred vector plots", "Vector", "vel")
    TestExpressionPlot("3D inferred tensor plots", "Tensor", "stress")

    CloseDatabase(db3d)

test_2d_scalar_comp_exprs()
test_3d_scalar_comp_exprs()

Exit()
