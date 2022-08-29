# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  expressions.py
#
#  Programmer: Eric Brugger
#  Date:       August 26, 2022
#
#  Modificatons:
#
# ----------------------------------------------------------------------------


# NOTE: Sections of this test file are 'literalinclude'd in quickrecipes.rst.
# After modifying this file, ensure the proper content is still displayed in the doc.

# comments of the form '# sometext {' and '# sometext }' bracket the sections
# that are 'literalinclude'd in quickrecipes.rst.

# mapMaterialsToValues {
# Create an expression that maps material numbers to scalar values.
#
# var is the name of the expression.
# mat is the name of the material variable.
# mesh is the name of the mesh variable.
# pairs is a list of tuples of material number and scalar value.
# The material number of the last tuple of the list is ignored and the value
# will be used for all the remaining materials.

def create_mat_value_expr(var, mat, mesh, pairs):
    expr=""
    parens=""
    nlist = len(pairs)
    ilist = 0
    for pair in pairs:
        ilist = ilist + 1
        parens = parens + ")"
        if (ilist == nlist):
            expr = expr + "zonal_constant(%s,%f" % (mesh, pair[1]) + parens
        else:
            expr=expr + "if(eq(dominant_mat(%s),zonal_constant(%s,%d)),zonal_constant(%s,%f)," % (mat, mesh, pair[0], mesh, pair[1])

    DefineScalarExpression(var, expr)

# Call the function to create the expression.
mat_val_pairs = [(1, 0.75), (3, 1.2), (6, 0.2), (7, 1.6), (8, 1.8), (11, 2.2), (-1, 2.5)]

create_mat_value_expr("myvar", "mat1", "quadmesh2d", mat_val_pairs)

# Create a pseudocolor plot of the expression.
OpenDatabase("/usr/gapps/visit/data/rect2d.silo")
AddPlot("Pseudocolor", "myvar")
DrawPlots()
# mapMaterialsToValues }

Test("materials_to_values")

Exit()
