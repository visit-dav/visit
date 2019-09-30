# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  expressions.py
#
#  Tests:      mesh      - 3D unstructured, single domain
#              plots     - pc
#
#  Defect ID:  '3939 + '4008 + '4022 + '4162 + '4417 + '4435 + '5790 + '5672
#            + '5763 + '5767 + '5874 + '7465 + '7473
#
#  Programmer: Hank Childs
#  Date:       November 18, 2002
#
#  Modifications:
#
#    Hank Childs, Fri Oct 24 09:25:04 PDT 2003
#    Added test for mesh expressions for databases where we communicate
#    ghost zones.
#
#    Hank Childs, Wed Dec 10 14:25:01 PST 2003
#    Add tests for implicit and explicit changes in centering.  Also added
#    a test for the identity expression, which previously crashed the engine.
#
#    Hank Childs, Wed Feb 11 14:49:40 PST 2004
#    Add tests for nmats and constants.
#    
#    Hank Childs, Thu Apr 22 16:19:36 PDT 2004
#    Update comments due to the change in semantics of recenter (it is now
#    always zonal).
#
#    Hank Childs, Wed Dec 22 13:14:29 PST 2004
#    Add tests for variables used both in an expression and in the
#    pipeline ['5790].
#
#    Hank Childs, Thu Jan  6 11:21:21 PST 2005
#    Add tests for multiple identity filters and additional tests for mixed
#    centering ['5672, '5763, '5767].
#
#    Hank Childs, Thu Jan 20 18:29:28 PST 2005
#    Add test for resrad ['5874].
#
#    Hank Childs, Fri Aug 25 17:34:13 PDT 2006
#    Add test for resrad when arguments are missing ['7473].
#
#    Hank Childs, Fri Sep 14 11:52:39 PDT 2007
#    Add test for identify filters and meshes ['7465].
#
#    Cyrus Harrison, Thu Jan 31 09:19:57 PST 2008
#    Added a test for the value_for_material filter. 
#
#    Cyrus Harrison, Tue Feb 19 13:52:01 PST 2008
#    Removed value_for_material tests (they now reside in val4mat.py)
#
#    Kathleen Bonnell, Thu May  7 09:26:36 PDT 2009
#    ident_mesh is no longer an error case, so remove the error test and
#    save an image instead.
#
#    Mark C. Miller, Wed Jan 20 07:37:11 PST 2010
#    Added ability to swtich between Silo's HDF5 and PDB data.
# ----------------------------------------------------------------------------


OpenDatabase(silo_data_path("bigsil.silo"))


DefineScalarExpression("radius_polar", "polar(mesh)[0]")
AddPlot("Contour", "radius_polar")
DrawPlots()
Test("radius_ghosts")

DeleteAllPlots()

OpenDatabase(silo_data_path("globe.silo"))



DefineScalarExpression("mag1", "magnitude(vel)")
AddPlot("Pseudocolor", "mag1")
DrawPlots()

Test("magnitude1")
DeleteAllPlots()

#
# We have a poor gradient scheme that will cause gradients along a band to
# falsely register as 0.  Regardless, I'd like to test it.
#
DefineScalarExpression("mag2", "magnitude(gradient(v))")
AddPlot("Pseudocolor", "mag2")
DrawPlots()

Test("magnitude2")
DeleteAllPlots()


#
# See above comment regarding gradients.
#
DefineScalarExpression("vector_sum", "magnitude(gradient(v)+gradient(u))")
AddPlot("Pseudocolor", "vector_sum")
DrawPlots()

Test("vector_sum")
DeleteAllPlots()


DefineScalarExpression("vector_diff", "magnitude(gradient(v)-gradient(u))")
AddPlot("Pseudocolor", "vector_diff")
DrawPlots()

Test("vector_diff")
DeleteAllPlots()


DefineScalarExpression("theta", "polar(mesh1)[1]")
AddPlot("Pseudocolor", "theta")
DrawPlots()

Test("theta")
DeleteAllPlots()


DefineScalarExpression("phi", "polar(mesh1)[2]")
AddPlot("Pseudocolor", "phi")
DrawPlots()

Test("phi")
DeleteAllPlots()


DefineScalarExpression("cos", "cos(polar(mesh1)[1])")
AddPlot("Pseudocolor", "cos")
DrawPlots()

Test("cos")
DeleteAllPlots()


DefineScalarExpression("abs_cos", "abs(cos(polar(mesh1)[1]))")
AddPlot("Pseudocolor", "abs_cos")
DrawPlots()

Test("abs_cos")
DeleteAllPlots()


DefineScalarExpression("sin", "sin(polar(mesh1)[1])")
AddPlot("Pseudocolor", "sin")
DrawPlots()

Test("sin")
DeleteAllPlots()


DefineScalarExpression("minus_sin", "-sin(polar(mesh1)[1])")
AddPlot("Pseudocolor", "minus_sin")
DrawPlots()

Test("minus_sin")
DeleteAllPlots()

DefineScalarExpression("four_phase", "rad2deg(acos(sin(polar(mesh1)[1]))-polar(mesh1)[1])")
AddPlot("Pseudocolor", "four_phase")
DrawPlots()

Test("four_phase")
DeleteAllPlots()


atts = PseudocolorAttributes()
atts.minFlag = 1
atts.min = 0.99
atts.maxFlag = 1
atts.max = 1.01
SetDefaultPlotOptions(atts)

DefineScalarExpression("one", "sin(polar(mesh1)[1])*sin(polar(mesh1)[1]) + cos(polar(mesh1)[1])*cos(polar(mesh1)[1])")
AddPlot("Pseudocolor", "one")
DrawPlots()

Test("one")
DeleteAllPlots()

atts.minFlag = 0
atts.maxFlag = 0
SetDefaultPlotOptions(atts)

DefineScalarExpression("X", "coord(mesh1)[0]")
AddPlot("Pseudocolor", "X")
DrawPlots()

Test("X")
DeleteAllPlots()


DefineScalarExpression("radius", "sqrt(coords(mesh1)[0]*coords(mesh1)[0]+coords(mesh1)[1]*coords(mesh1)[1]+coords(mesh1)[2]*coords(mesh1)[2])")
AddPlot("Pseudocolor", "radius")
DrawPlots()

Test("radius")
DeleteAllPlots()


DefineScalarExpression("zero", "(polar(mesh1))[0] - sqrt(coords(mesh1)[0]*coords(mesh1)[0]+coords(mesh1)[1]*coords(mesh1)[1]+coords(mesh1)[2]*coords(mesh1)[2])")
AddPlot("Pseudocolor", "zero")
DrawPlots()

Test("zero")
DeleteAllPlots()

DefineVectorExpression("dd", "disp+{1,2,3}")
AddPlot("Vector", "dd")
DrawPlots()

Test("vector_add_const")
DeleteAllPlots()

DefineVectorExpression("cr", "cross(disp,dd)")
AddPlot("Vector", "cr")
DrawPlots()

Test("vector_cross")
DeleteAllPlots()

DefineScalarExpression("a", "t")
AddPlot("Pseudocolor", "a")
DrawPlots()
Test("identity_expr")
DeleteAllPlots()

DefineScalarExpression("sum1", "t+100*u")
AddPlot("Pseudocolor", "sum1")
DrawPlots()
Test("diff_centering_expr_01")
DeleteAllPlots()

DefineScalarExpression("sum2", "(u+t/100.)*100.")
AddPlot("Pseudocolor", "sum2")
DrawPlots()
Test("diff_centering_expr_02")
DeleteAllPlots()

DefineScalarExpression("diff", "sum2-sum1")
AddPlot("Pseudocolor", "diff")
DrawPlots()
Test("diff_centering_expr_03")
DeleteAllPlots()

DefineScalarExpression("sum_rc1", "recenter(t)+100*u")
AddPlot("Pseudocolor", "sum_rc1")
DrawPlots()
Test("diff_centering_expr_04")
DeleteAllPlots()

# This one will recenter the first t.  When the second one is subtracted,
# will recenter the first t again to make it zonal again.
DefineScalarExpression("diff_rc1", "recenter(t)-t")
AddPlot("Pseudocolor", "diff_rc1")
DrawPlots()
Test("diff_centering_expr_05")
DeleteAllPlots()

# This one will recenter the first t twice.  The second one will not be 
# touched.
DefineScalarExpression("diff_rc2", "recenter(recenter(t))-t")
AddPlot("Pseudocolor", "diff_rc2")
DrawPlots()
Test("diff_centering_expr_06")
DeleteAllPlots()

# The first t will not be touched.  The second one will be recentered once
# explicitly and once implicitly (from the minus).
DefineScalarExpression("diff_rc3", "t-recenter(t)")
AddPlot("Pseudocolor", "diff_rc2")
DrawPlots()
Test("diff_centering_expr_07")
DeleteAllPlots()

# The constant (3.14159) will be the only variable in the dataset.  Test
# that the constant creation logic can figure out that we must be making
# an expression based on the mesh downstream.
OpenDatabase(silo_data_path("rect3d.silo"))

DefineScalarExpression("sin_x", "sin(3.14159*coord(quadmesh3d)[0])")
AddPlot("Pseudocolor", "sin_x")
DrawPlots()
Test("const_with_no_other_vars")
DeleteAllPlots()

# The nmats expression needs to use the original zone numbers array to
# find the correct index into an avtMaterial object.  Make sure that this is
# being done correctly by splitting the zones before we even get to the
# expression via MIR.
DefineScalarExpression("nmats", "nmats(mat1)")
AddPlot("FilledBoundary", "mat1")
AddOperator("Threshold")
thresh = ThresholdAttributes()
thresh.lowerBounds = (1.5)
thresh.listedVarNames = ("nmats")
SetOperatorOptions(thresh)
DrawPlots()
Test("nmats_with_mir")
DeleteAllPlots()

# Test that a variable (u) can be used in both the expression and downstream
# in the pipeline.
OpenDatabase(silo_data_path("globe.silo"))

AddPlot("Pseudocolor", "speed")
AddOperator("Threshold")
thresh = ThresholdAttributes()
thresh.lowerBounds = (0.0)
thresh.listedVarNames = ("u")
SetOperatorOptions(thresh)
DrawPlots()
Test("mult_var_usage")
DeleteAllPlots()

# The binary math filter forces mixed centering to be zonal.  The base class
# used to get confused and would sometimes declare the variable as nodal.
# Test to make sure this works.  By forcing the centering, we can confirm
# the base class is declaring the variable the right way.
DefineScalarExpression("prod", "u*t")
AddPlot("Pseudocolor", "prod")
pc = PseudocolorAttributes()
pc.centering = pc.Nodal
SetPlotOptions(pc)
DrawPlots()
Test("mixed_centering_nodal")
pc = PseudocolorAttributes()
pc.centering = pc.Zonal
SetPlotOptions(pc)
Test("mixed_centering_zonal")

# Test that we can handle multiple identity expressions.
DefineScalarExpression("sub1", "u")
DefineScalarExpression("sub2", "v")
DefineScalarExpression("sum", "sub1+sub2")
DeleteAllPlots()
AddPlot("Pseudocolor", "sum")
DrawPlots()
Test("mult_identity_expr")

# Test that we can handle identity expressions that are not the first node
# of the expression tree.
DefineVectorExpression("C", "coord(mesh1)")
DefineScalarExpression("X", "C[0]")
DefineScalarExpression("Y", "C[1]")
DefineScalarExpression("Z", "C[2]")
DefineScalarExpression("TX", "X")
DefineScalarExpression("TY", "cos_angle*Y - sin_angle*Z + Zt*sin_angle - Yt*cos_angle")
DefineScalarExpression("TZ", "sin_angle*Y + cos_angle*Z + Yt*sin_angle - Zt*cos_angle")
DefineScalarExpression("Yt", "5")
DefineScalarExpression("Zt", "10")
DefineScalarExpression("angle", "rad2deg(30)")
DefineScalarExpression("sin_angle", "sin(angle)")
DefineScalarExpression("cos_angle", "cos(angle)")
DefineScalarExpression("rad", "TX*TX + TY*TY + TZ*TZ")
AddPlot("Pseudocolor", "rad")
DrawPlots()
Test("long_identity_expr")

DeleteAllPlots()
OpenDatabase(silo_data_path("rect2d.silo"))

# Test divide expression
DefineScalarExpression('divide1', 'divide(d,p)')
AddPlot('Pseudocolor','divide1')
DrawPlots()
Test('divide1')

DefineScalarExpression('divide2', 'divide(d,p,0.0,2.0)')
AddPlot('Pseudocolor','divide2')
DrawPlots()
Test('divide2')
DeleteAllPlots()

DefineScalarExpression('divide3', 'divide(d,p,1.0,2.0)')
AddPlot('Pseudocolor','divide3')
DrawPlots()
Test('divide3')
DeleteAllPlots()

# Test min/max expression
DefineScalarExpression('min1', 'min(10.0, 5.0, d+p)')
AddPlot('Pseudocolor', 'min1')
DrawPlots()
Test('min1')
DeleteAllPlots()

DefineScalarExpression('min2', 'min(d+p, 5.0, 10.0)')
AddPlot('Pseudocolor', 'min2')
DrawPlots()
Test('min2')
DeleteAllPlots()

DefineScalarExpression('max1', 'max(10.0, 5.0, d+p)')
AddPlot('Pseudocolor', 'max1')
DrawPlots()
Test('max1')
DeleteAllPlots()

DefineScalarExpression('min3', 'min(2.0, d+p, d*p+2*d)')
AddPlot('Pseudocolor', 'min3')
DrawPlots()
Test('min3')
DeleteAllPlots()

# Test resrad
DefineScalarExpression("resrad", "resrad(recenter(u), 0.1)")
AddPlot("Pseudocolor", "resrad")
DrawPlots()
Test("resrad")

DeleteAllPlots()
DefineScalarExpression("resrad2", "resrad(recenter(u))")
AddPlot("Pseudocolor", "resrad2")
DrawPlots()
txt = GetLastError()
TestText("resrad_error", txt)

DeleteAllPlots()
DefineScalarExpression("ident_mesh", "quadmesh2d")
AddPlot("Pseudocolor", "ident_mesh")
DrawPlots()
Test("ident_mesh")

Exit()
