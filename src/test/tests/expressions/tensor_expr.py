# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  tensor_expr.py
#
#  Tests:      tensor expressions using simple, constant valued tensors for
#              which answers are known. The known answers were obtained by
#              scouring the internet for example problems that were worked.
#
#  Mark C. Miller, Mon Nov 11 14:52:02 PST 2019
#
# ----------------------------------------------------------------------------
import math, re

#
# Scans a string for possible python iterables, builds a list of them
# and returns them with their values rounded to specific numbers of
# digits.
#
def ExtractIterablesFromString(s,pair='()',rnd=4):
    retval = []
    for q in s.split(pair[0]):
        for r in q.split(pair[1]):
            if re.match('^[ 0-9eE.,+-]+$', r):
                retval.append([round(float(x),rnd) for x in r.split(',')])
    return retval

def HasEigval(vals, cval, rnd=4):
    vrnd = [round(float(x),rnd) for x in vals]
    crnd = round(float(cval), rnd)
    for i in range(len(vrnd)):
        if vrnd[i] == crnd:
           vals.remove(vals[i])
           return True
    return False

#
# Checks if a given vector, cvec, is amoung a list of vectors, vecs,
# within the specified rounding tolerance, rnd. If so, removes the
# matching entry and returns True.
#
def HasEigvec(vecs, cvec, rnd=4):
    for v in vecs:
        r = []
        for i in range(len(v)):
            if v[i] != 0:
                ratio = float(cvec[i]) / float(v[i]);
                r.append(round(ratio, rnd))
        if (max(r) - min(r)) <= pow(10,-rnd+1):
            vecs.remove(v)
            return True
    return False

def ValuesAgree(vec, val, rnd=4):
    valrnd = round(float(val), rnd)
    vecrnd = [round(float(x), rnd) for x in vec]
    for i in range(len(vecrnd)):
        if vecrnd[i] != valrnd:
            return False
    return True

#
# Creates a group of related expressions; 9 constant scalar expressions and
# from them a constant tensor expression with specified centering all in a
# sub-menu with of the given name. For 2D meshes, you would still create a
# 9 component (3x3) tensor but the z-dimension values would all be zeros.
#
def CreateConstantTensorExpr(name, meshName, constType, vals):
    comps = ("s11","s12","s13","s21","s22","s23","s31","s32","s33")
    for i in range(len(vals)):
        DefineScalarExpression("%s/%s"%(name,comps[i]),\
            "%s_constant(<%s>, %g)"%(constType,meshName,vals[i]))
    DefineTensorExpression("%s/tensor"%name,
        "{{<%s/s11>,<%s/s12>,<%s/s13>},\
          {<%s/s21>,<%s/s22>,<%s/s23>},\
          {<%s/s31>,<%s/s32>,<%s/s33>}}"%(name, name, name, name,\
              name, name, name, name, name))

#
# Set precision for rounding operations
#
Prec = 3

#  Since we use the expression system to construct mesh-wide constant values,
#  all we need as far as a database is a simple, small mesh. Maybe for both
#  two and three dimensions.
OpenDatabase(silo_data_path("arbpoly.silo"))
meshName = "clipped_hex"

TestSection("2D Tensor Maximum Shear")
CreateConstantTensorExpr("max_shear_2d", meshName, "nodal",\
   (50,  30,   0,\
    30, -20,   0,\
     0,   0,   0))
DefineScalarExpression("max_shear_2d/result", "tensor_maximum_shear(<max_shear_2d/tensor>)")
AddPlot("Pseudocolor", "max_shear_2d/result")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
AssertTrue("Maximum Shear 2D", ValuesAgree((q['min'], q['max']), 46.098, Prec))
DeleteAllPlots()

TestSection("3D Tensor Maximum Shear")
CreateConstantTensorExpr("max_shear", meshName, "nodal",\
   (5,   0,   0,\
    0,  -6, -12,\
    0, -12,   1))
DefineScalarExpression("max_shear/result", "tensor_maximum_shear(<max_shear/tensor>)")
AddPlot("Pseudocolor", "max_shear/result")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
AssertTrue("Maximum Shear 3D", ValuesAgree((q['min'], q['max']), 12.5, Prec))
DeleteAllPlots()

TestSection("2D Effective Tensor")
CreateConstantTensorExpr("eff_tensor_2d", meshName, "nodal",\
   (50,  30,   0,\
    30, -20,   0,\
     0,   0,   0))
DefineScalarExpression("eff_tensor_2d/result", "effective_tensor(<eff_tensor_2d/tensor>)")
AddPlot("Pseudocolor", "eff_tensor_2d/result")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
AssertTrue("2D Effective Tensor", ValuesAgree((q['min'], q['max']), 81.240, Prec))
DeleteAllPlots()

TestSection("3D Effective Tensor")
CreateConstantTensorExpr("eff_tensor", meshName, "nodal",\
   (2,  -3,   4,\
   -3,  -5,   1,\
    4,   1,   6))
DefineScalarExpression("eff_tensor/result", "effective_tensor(<eff_tensor/tensor>)")
AddPlot("Pseudocolor", "eff_tensor/result")
DrawPlots()
Query("MinMax")
q = GetQueryOutputObject()
AssertTrue("Effective Tensor", ValuesAgree((q['min'], q['max']), 13.077, Prec))
DeleteAllPlots()

TestSection("3D, Symmetric Eigenvalues and Eigenvectors")
CreateConstantTensorExpr("eigvals_symm2", meshName, "nodal",\
    (2, -1,  0,\
    -1,  2, -1,\
     0, -1,  2))
DefineVectorExpression("eigvals_symm2/result", "eigenvalue(<eigvals_symm2/tensor>)")
AddPlot("Vector", "eigvals_symm2/result")
DrawPlots()
p = PickByNode(0,('eigvals_symm2/result',))
eigvals = list(p['eigvals_symm2/result'])
AssertTrue("First Eigenvalue of 2", HasEigval(eigvals, 2, Prec))
AssertTrue("Second Eigenvalue of 2+sqrt(2)", HasEigval(eigvals, 2+math.sqrt(2), Prec))
AssertTrue("Third Eigenvalue of 2-sqrt(2)",  HasEigval(eigvals, 2-math.sqrt(2), Prec))
DeleteAllPlots()
DefineTensorExpression("eigvals_symm2/result2", "transpose(eigenvector(<eigvals_symm2/tensor>))")
AddPlot("Tensor", "eigvals_symm2/result2")
DrawPlots()
PickByNode(0)
s = GetPickOutput()
vecs = ExtractIterablesFromString(s, '()', Prec)
AssertTrue("First Eigenvector of (1,0,-1)", HasEigvec(vecs, (1,0,-1), Prec))
AssertTrue("Second Eigenvector of (1,-sqrt(2),1)", HasEigvec(vecs, (1,-math.sqrt(2),1), Prec))
AssertTrue("Third Eigenvector of (1,sqrt(2),1)", HasEigvec(vecs, (1,math.sqrt(2),1), Prec))

TestSection("3D, Symmetric Eigenvalues and Eigenvectors with Repeated values")
CreateConstantTensorExpr("eigvals_symm", meshName, "nodal",\
    (3,2,4,\
     2,0,2,\
     4,2,3))
DefineVectorExpression("eigvals_symm/result", "eigenvalue(<eigvals_symm/tensor>)")
AddPlot("Vector", "eigvals_symm/result")
DrawPlots()
p = PickByNode(0)
eigvals = list(p['eigvals_symm/result'])
AssertTrue("First Eigenvalue of -1", HasEigval(eigvals, -1, Prec))
AssertTrue("Second Eigenvalue of -1", HasEigval(eigvals, -1, Prec))
AssertTrue("Third Eigenvalue of 8", HasEigval(eigvals, 8, Prec))
DeleteAllPlots()
DefineTensorExpression("eigvals_symm/result2", "transpose(eigenvector(<eigvals_symm/tensor>))")
AddPlot("Tensor", "eigvals_symm/result2")
DrawPlots()
PickByNode(0)
s = GetPickOutput()
vecs = ExtractIterablesFromString(s, '()', Prec+1)
AssertTrue("First Eigenvector of (1,-2,0)", HasEigvec(vecs, (1,-2,0), Prec))
AssertTrue("Second Eigenvector of (4,2,-5)", HasEigvec(vecs, (4,2,-5), Prec))
AssertTrue("Third Eigenvector of (2,1,2)", HasEigvec(vecs, (2,1,2), Prec))
DeleteAllPlots()

TestSection("2D, Symmetric Eigenvalues and Eigenvectors")
CreateConstantTensorExpr("eigvals_symm_2d", meshName, "nodal",\
    (2,  3,  0,\
     3,  2,  0,\
     0,  0,  0))
DefineVectorExpression("eigvals_symm_2d/result", "eigenvalue(<eigvals_symm_2d/tensor>)")
AddPlot("Vector", "eigvals_symm_2d/result")
DrawPlots()
p = PickByNode(0,('eigvals_symm_2d/result',))
eigvals = list(p['eigvals_symm_2d/result'])
AssertTrue("First Eigenvalue of -1", HasEigval(eigvals, -1, Prec))
AssertTrue("Second Eigenvalue of 5", HasEigval(eigvals, 5, Prec))
DeleteAllPlots()
DefineTensorExpression("eigvals_symm_2d/result2", "transpose(eigenvector(<eigvals_symm_2d/tensor>))")
AddPlot("Tensor", "eigvals_symm_2d/result2")
DrawPlots()
PickByNode(0)
s = GetPickOutput()
vecs = ExtractIterablesFromString(s, '()', Prec)
vecs.remove([0,0,1]) # we have to take out the Z eigenvector
AssertTrue("First Eigenvector of (1,-1)", HasEigvec(vecs, (1,-1), Prec))
AssertTrue("Second Eigenvector of (1,1)", HasEigvec(vecs, (1,1), Prec))

#
# Test a case where eigenvalues are complex (e.g. imaginary)
# The real eigenvalues are 2, (4+3i)/5, (4-3i)/5 but what you
# get from VisIt is 2, 7/5, 1/5 (as though i==1 in the above).
#
TestSection("3D, Complex Eigenvalues and Eigenvectors")
CreateConstantTensorExpr("eigvals_complex", meshName, "nodal",\
    (4.0/5.0, -3.0/5.0,     0,\
     3.0/5.0,  4.0/5.0,     0,\
        1,        2,        2))
DefineVectorExpression("eigvals_complex/result", "eigenvalue(<eigvals_complex/tensor>)")
AddPlot("Vector", "eigvals_complex/result")
DrawPlots()
p = PickByNode(0)
eigvals = list(p['eigvals_complex/result'])
AssertTrue("First Eigenvalue of 2", HasEigval(eigvals, 2, Prec))
AssertTrue("Second Eigenvalue of (4+3i)/5", HasEigval(eigvals, float(4+3)/5.0, Prec))
AssertTrue("Third Eigenvalue of (4-3i)/5", HasEigval(eigvals, float(4-3)/5.0, Prec))
DeleteAllPlots()
DefineTensorExpression("eigvals_complex/result2", "transpose(eigenvector(<eigvals_complex/tensor>))")
AddPlot("Tensor", "eigvals_complex/result2")
DrawPlots()
PickByNode(0)
s = GetPickOutput()
vecs = ExtractIterablesFromString(s, '()', Prec)
AssertTrue("First Eigenvector of (0,0,1)", HasEigvec(vecs, (0,0,1), Prec))
DeleteAllPlots()

# Confirm principal_tensor function gives same result
TestSection("Cross Principal Stresses and Eigenvalues")
DefineVectorExpression("pcomps_complex/result", "principal_tensor(<eigvals_complex/tensor>)")
AddPlot("Vector", "pcomps_complex/result")
DrawPlots()
p = PickByNode(0)
pcomps = list(p['pcomps_complex/result'])
AssertTrue("First principal component is first eigenvalue", HasEigval(pcomps, 2, Prec))
AssertTrue("Second principal component is second eigenvalue", HasEigval(pcomps, float(4+3)/5.0, Prec))
AssertTrue("Third principal component is third eigenvalue", HasEigval(pcomps, float(4-3)/5.0, Prec))

# Re-use eigvals_symm here
TestSection("Cross Check Deviatoric and Principal Stresses")
DefineVectorExpression("eigvals_symm/dev", "principal_deviatoric_tensor(<eigvals_symm/tensor>)")
AddPlot("Tensor", "eigvals_symm/dev")
DrawPlots()
PickByNode(0)
s = GetPickOutput()
dev_vec = ExtractIterablesFromString(s, '()', Prec)
DeleteAllPlots()
DefineScalarExpression("eigvals_symm/tr3", "trace(<eigvals_symm/tensor>)/3.0")
DefineTensorExpression("eigvals_symm/tensor3",\
"""
    {{<eigvals_symm/tensor>[0][0]-<eigvals_symm/tr3>, <eigvals_symm/tensor>[0][1], <eigvals_symm/tensor>[0][2]},
     {<eigvals_symm/tensor>[1][0], <eigvals_symm/tensor>[1][1]-<eigvals_symm/tr3>, <eigvals_symm/tensor>[1][2]},
     {<eigvals_symm/tensor>[2][0], <eigvals_symm/tensor>[2][1], <eigvals_symm/tensor>[2][2]-<eigvals_symm/tr3>}}
"""
)
DefineVectorExpression("eigvals_symm/dev2", "principal_tensor(<eigvals_symm/tensor3>)")
AddPlot("Tensor", "eigvals_symm/dev2")
DrawPlots()
PickByNode(0)
s = GetPickOutput()
dev2_vec = ExtractIterablesFromString(s, '()', Prec)
DeleteAllPlots()
AssertTrue("Principal deviatoric and principal-tr()/3 agree", dev_vec == dev2_vec)

Exit()
