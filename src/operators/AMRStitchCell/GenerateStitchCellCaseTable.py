#!/usr/bin/python
#     3----2------2
#    /|          /|
#   A |         B |              A=10 B=11
#  /  |        /  |
# 7---+-6-----6   1
# |   3       |   |
# |   |       |   |
# |   |       5   |<-back
# 7   0----0--+---1
# |  /        |  /
# | 8         | 9
# |/          |/
# 4-----4-----5 
#
#  y
#  ^
#  |
#  *->x
# /
#\/
# z

import sys

if (sys.version_info > (3, 0)):
    from functools import reduce

rotXVtxPermutation = [ 4, 5, 1, 0, 7, 6, 2, 3 ]
rotYVtxPermutation = [ 1, 5, 6, 2, 0, 4, 7, 3 ]
rotZVtxPermutation = [ 3, 0, 1, 2, 7, 4, 5, 6 ]

rotXAxisPermutation = [ 0, 2, 1 ]
rotYAxisPermutation = [ 2, 1, 0 ]
rotZAxisPermutation = [ 1, 0, 2 ]

def chainPermutations(a,b):
   return [ b[x] for x in a ]

vtxRotations = [ rotYVtxPermutation, rotZVtxPermutation ]
axisRotations = [ rotYAxisPermutation, rotZAxisPermutation ]

rotY = 0
rotZ = 1

permutationTemplates = [
  [ ],
  [ rotY ],
  [ rotZ ],
  [ rotY, rotY ],
  [ rotY, rotZ ],
  [ rotZ, rotY ],
  [ rotZ, rotZ ],
  [ rotY, rotY, rotY ],
  [ rotY, rotY, rotZ ],
  [ rotY, rotZ, rotY ],
  [ rotY, rotZ, rotZ ],
  [ rotZ, rotY, rotY ],
  [ rotZ, rotZ, rotY ],
  [ rotZ, rotZ, rotZ ],
  [ rotY, rotY, rotY, rotZ ],
  [ rotY, rotY, rotZ, rotY ],
  [ rotY, rotY, rotZ, rotZ ],
  [ rotY, rotZ, rotY, rotY ],
  [ rotY, rotZ, rotZ, rotZ ],
  [ rotZ, rotY, rotY, rotY ],
  [ rotZ, rotZ, rotZ, rotY ],
  [ rotY, rotY, rotY, rotZ, rotY ],
  [ rotY, rotZ, rotY, rotY, rotY ],
  [ rotY, rotZ, rotZ, rotZ, rotY ] ]

allVtxPermutations = [ reduce(lambda x,y: chainPermutations(x, vtxRotations[y]), template, [0, 1, 2, 3, 4, 5, 6, 7]) for template in permutationTemplates ]
allAxisPermutations = [ reduce(lambda x,y: chainPermutations(x, axisRotations[y]), template, [0, 1, 2]) for template in permutationTemplates ]

def invertPermutation(perm):
   return [invertedSequence[1] for invertedSequence in sorted([(numberedTuple[1],numberedTuple[0]) for numberedTuple in enumerate(perm) ])]

allInverseVtxPermutations = [ invertPermutation(perm) for perm in allVtxPermutations ]
allInverseAxisPermutations = [ invertPermutation(perm) for perm in allAxisPermutations ]

def numberToBinaryList(number):
    if number < 2:
       return [ number ]
    else:
       return numberToBinaryList(number / 2) + [number % 2]

def numberToReverseBinaryList(number):
    if number < 2:
       return [ number ]
    else:
       return [number % 2] + numberToReverseBinaryList(number / 2)

def fillLeftToLen(list, dlen, fill):
    if len(list) >= dlen:
       return list
    else:
       return fillLeftToLen([fill]+list, dlen, fill)

def fillRightToLen(list, dlen, fill):
    if len(list) >= dlen:
       return list
    else:
       return fillRightToLen(list+[fill], dlen, fill)

#def configForCase(caseNo):
#    return fillLeftToLen(numberToBinaryList(caseNo), 8, 0)

def configForCase(caseNo):
    return fillRightToLen(numberToReverseBinaryList(caseNo), 8, 0)

allCases = [configForCase(i) for i in range(0, 256)]

baseCases = []
while len(allCases) > 0:
    currCase = allCases[0]
    baseCases = baseCases + [ currCase ]
    derivedCases = [ chainPermutations(perm, currCase) for perm in allVtxPermutations ]
    allCases = [ elem for elem in allCases if elem not in derivedCases ]

print("***Base cases (%d):" % len(baseCases))
print([ x for x in enumerate(baseCases) ])

def subCasesInXPossible(l):
   return (not l[0] or not l[3] or not l[4] or not l[7]) and l[0] == l[1] and l[3] == l[2] and l[4] == l[5] and l[7] == l[6]

def subCasesInYPossible(l):
   return (not l[0] or not l[1] or not l[4] or not l[5]) and l[0] == l[3] and l[1] == l[2] and l[4] == l[7] and l[5] == l[6]

def subCasesInZPossible(l):
   return (not l[0] or not l[1] or not l[2] or not l[3]) and l[0] == l[4] and l[1] == l[5] and l[2] == l[6] and l[3] == l[7]

def boolToInt(b):
   if b:
      return 1
   else:
      return 0

def allZero(l):
    if len(l) == 0:
        return True
    elif l[0] != 0:
        return False
    else:
        return allZero(l[1:])
        
def numSubCases(l):
   if allZero(l):
       return 0
   else:
       return 2 ** (boolToInt(subCasesInXPossible(l)) +  boolToInt(subCasesInYPossible(l)) + boolToInt(subCasesInZPossible(l)))
 
print("***Number of subcases:")
numSubCasesForBaseCase = [numSubCases(case) for case in baseCases]
print(numSubCasesForBaseCase)

allCases = [configForCase(i) for i in range(0, 256)]

baseCaseForCase = [ x[0] for x in [ [baseCase[0] for baseCase in enumerate(baseCases) if (ac in [ chainPermutations(perm, baseCase[1]) for perm in allVtxPermutations ] ) ] for ac in allCases]]

print("***Base case for case:")
print(baseCaseForCase) 

print("***Permutation of base case for case:")
permForCase = [perm[0] for perm in  [[permutation[0] for permutation in enumerate(allVtxPermutations) if allCases[i] == chainPermutations(permutation[1], baseCases[baseCaseForCase[i]])] for i in range(0,256)]]
print(permForCase)

baseCaseTesselations = [
    [ ], # Case 0 
    [ [ [ 1, 5, 6, 2, 0 ], [ 4, 7, 6, 5, 0 ], [ 2, 6, 7, 3, 0 ] ] ], # Case 1 
    [ [ [ 0, 3, 7, 1, 2, 6 ], [ 0, 7, 4, 1, 6, 5 ] ], [ [ 0, 1, 2, 6 ], [ 0, 4, 1, 6 ] ] ], # Case 2 
    [ [ [ 4, 7, 6, 5, 0 ], [ 1, 5, 2, 0 ], [ 5, 6, 2, 0 ], [ 2, 7, 3, 0 ], [ 2, 6, 7, 0 ] ] ], # Case 3 
    [ [ [ 0, 2, 3, 7 ], [ 4, 5, 1, 0, 7, 6, 2 ] ] ] , # Case 4 -> INCORRECT (degenerate cell)
    [ [ [ 0, 1, 2, 3, 4, 5, 6, 7 ] ], [ [ 0, 4, 1, 3, 6, 2 ] ], [ [ 1, 5, 2, 0, 4, 3 ] ], [ [ 0, 1, 2, 3, 4 ] ] ], # Case 5 
    [ [ [ 0, 1, 2, 4 ], [ 0, 2, 3, 4 ], [ 1, 5, 2, 4 ], [ 2, 5, 6, 4 ], [ 4, 7, 6, 2 ], [ 3, 7, 4, 2 ] ] ], # Case 6 
    [ [ [ 0, 4, 5, 1, 2 ], [ 0, 3, 7, 4, 2 ], [ 2, 5, 6, 4 ], [ 4, 7, 6, 2 ] ]  ], # Case 7 
    [ [ [ 0, 1, 2, 4 ], [ 1, 5, 6, 2, 4 ], [ 0, 3, 7, 4, 2 ], [ 4, 7, 6, 2 ] ] ], # Case 8 
    [ [ [ 1, 5, 6, 4 ], [ 1, 6, 2, 3 ], [ 4, 3, 7, 6 ], [ 0, 3, 4, 1 ], [ 3, 6, 4, 1 ] ] ], # Case 9 
    [ [ [ 1, 5, 6, 4 ], [ 1, 6, 2, 3 ], [ 4, 3, 7, 6 ], [ 0, 3, 4, 1 ], [ 3, 6, 4, 1 ] ] ], # Case 10 
    [ [ [ 2, 6, 7, 3, 4 ], [ 0, 4, 5, 1, 2 ], [ 2, 5, 6,  4 ], [ 0, 3, 4, 2 ] ] ], # Case 11 
    [ [ [ 0, 1, 3, 4 ], [ 1, 2, 3, 4 ], [ 1, 5, 6, 2, 4 ], [ 2, 6, 7, 3, 4 ] ] ], # Case 12 
    [ [ [ 0, 1, 2, 3, 4 ], [ 1, 5, 6, 2, 4 ], [ 2, 6, 7, 3, 4 ] ] ], # Case 13 
    [ [ [ 0, 3, 4, 1, 2, 5 ], [ 3, 7, 4, 2, 6, 5 ] ], [ [ 2, 5, 4, 3, 0 ], [ 2, 3, 4, 5, 6 ] ] ], # Case 14 
    [ [ [ 3, 7, 4, 2, 6, 5 ], [ 2, 5, 4, 3, 0 ], [ 0, 1, 2, 5 ] ] ], # Case 15
    [ [ [ 0, 3, 4, 1, 2, 5 ], [ 3, 7, 4, 2, 6, 5 ] ], [ [ 0, 3, 4, 1, 2, 5 ], [ 2, 3, 4, 5, 7 ] ] ], # Case 16
    [ [ [ 1, 5, 6, 4 ], [ 1, 6, 2, 3 ], [ 4, 3, 7, 6 ], [ 0, 3, 4, 1 ], [ 3, 6, 4, 1 ] ] ], # Case 17
    [ [ [ 1, 5, 6, 4 ], [ 1, 6, 2, 3 ], [ 4, 3, 7, 6 ], [ 0, 3, 4, 1 ], [ 3, 6, 4, 1 ] ] ], # Case 18
    [ [ [ 0, 1, 2, 3, 6 ], [ 1, 4, 5, 6 ], [ 0, 4, 1, 6 ], [ 0, 3, 4, 6 ], [ 3, 7, 4, 6 ] ] ], # Case 19
    [ [ [ 0, 1, 2, 5 ], [ 0, 2, 3, 6 ], [ 0, 3, 4, 6 ], [ 3, 7, 4, 6 ], [ 0, 4, 5, 6 ], [ 2, 5, 6, 0 ] ] ], # Case 20
    [ [ [ 3, 7, 4, 6 ], [ 5, 1, 0, 4, 6, 2, 3 ] ] ], # Case 21
    [ [ [ 0, 1, 2, 3, 4, 5, 6, 7 ] ] ] # Case 22 
]

cubeFaces = [ [ 0, 1, 2, 3 ], [ 1, 5, 6, 2 ], [ 4, 7, 6, 5 ], [ 0, 3, 7, 4 ], [ 2, 6,7, 3 ], [ 0, 4, 5, 1 ] ]
pyramidFaces = [ [ 0, 1, 2, 3 ], [ 1, 4, 2 ], [ 0, 4, 1 ], [ 0, 3, 4 ], [ 2, 4, 3 ] ]
tetFaces = [ [ 0, 1, 2 ], [ 0, 3, 1 ], [ 1, 3, 2 ], [ 0, 2, 3 ] ]
wedgeFaces = [ [ 0, 3, 4, 1 ], [ 0, 1, 2 ], [ 1, 4, 5, 2 ], [ 3, 5, 4 ], [ 0, 2, 5, 3 ] ]
XFaces = [ [ 0, 1, 2, 3,], [ 1, 5, 6, 2 ], [ 0, 4, 5, 1 ], [ 2, 6, 3 ], [ 3, 6, 4 ], [ 0, 3, 4 ], [ 4, 6, 5 ] ]

facesForTessElementLen = [ [], [], [], [], tetFaces, pyramidFaces, wedgeFaces, XFaces, cubeFaces ]

def normalizedFace(f):
    shift = min(enumerate(f), key=lambda x: x[1])[0]
    return f[shift:] + f[:shift]

faceBaseCases = [ [ 0, 0, 0, 0 ], [ 1, 0, 0, 0 ], [ 1, 1, 0, 0 ], [ 1, 0, 1, 0 ], [ 1, 1, 1, 0 ], [ 1, 1, 1, 1 ] ]
faceBaseCaseTesselations = [ [ [ 0, 1, 2, 3 ] ], [ [ 0, 1, 2 ], [ 0, 2, 3 ] ], [ [ 0, 1, 2, 3 ] ], [ [ 0, 1, 2 ], [ 0, 2, 3 ] ], [ [ 0, 1, 2 ], [ 0, 2, 3 ] ], [ [ 0, 1, 2, 3 ] ] ]

def faceConfToNumber(f):
   if len(f) == 0:
      return 0
   else:
      return f[0] + 2*faceConfToNumber(f[1:])

def derivedFaceCase(conf, shift):
    return conf[-shift:] + conf[:-shift]

def configForFaceCase(caseNo):
    return fillRightToLen(numberToReverseBinaryList(caseNo), 4, 0)

for i in range(0, len(baseCaseTesselations)):
    if len(baseCaseTesselations[i]) != numSubCasesForBaseCase[i]:
        print("Error: Number of tessleations does not match number of subcases for case %d" % i)

faceBaseCaseAndShiftForCase = [ [ (x[1], x[2]) for x in  [ (derivedFaceCase(baseCase[1], shift), baseCase[0], shift)
for baseCase in enumerate(faceBaseCases) for shift in range(0,4) ] if x[0] == configForFaceCase(faceCase) ][0] for faceCase in range(0,16)]

faceTesselationForCase = [ [ [ (x + shift) % 4 for x in tess ]  for tess in faceBaseCaseTesselations[baseCaseNo]] for (baseCaseNo, shift) in faceBaseCaseAndShiftForCase ]

def outerFaces(caseConfig):
    l=[ (faceNo, faceTesselationForCase[faceConfToNumber([caseConfig[vtx] for vtx in face])]) for faceNo, face in enumerate(cubeFaces)] 
    return sum([ [ [ cubeFaces[faceNo][vtx] for vtx in tessElem ] for tessElem in tess ] for faceNo, tess in l], [])

def listify(x):
     r = []
     i = 0
     while i < len(x):
         r = r + [ x[i+1:i+x[i]+1] ]
         i += x[i] + 1
     return r

def delistify(x):
    return reduce(lambda x,y: x + [ len(y) ] + y, x, [])

def applyPermutation(perm, tess):
    return [ [ perm[vtxNo] for vtxNo in cell ] for cell in tess ] 

def checkTess(caseConfig, caseTess):
    l =sum([ [ normalizedFace([ cell[vtx] for vtx in face]) for face in facesForTessElementLen[len(cell)] ] for cell in caseTess ],[]) + [ normalizedFace(x[::-1]) for x in outerFaces(caseConfig) ]
    l1 = [ x for x in l if l.count(x) != 1 or  l.count(normalizedFace(x[::-1])) != 1 ]
    if len(l1) != 0:
        print("Incorrect tesselation!")
        print("Case configuration: ", caseConfig)
        print("Case tesselation: ", caseTess)
        print("Outer faces: ", [normalizedFace(x) for x in outerFaces(caseConfig)])
        print("Tesselation boundary faces: ", sum([ [ normalizedFace([ cell[vtx] for vtx in face]) for face in facesForTessElementLen[len(cell)] ] for cell in caseTess ],[]))
        print("Leftover faces: ", l1)
        print("*********************")

for i in range(1, len(baseCases)):
   print("Checking base case tesselation %d" % i)
   checkTess(baseCases[i], baseCaseTesselations[i][0])


tesselations = [ [ applyPermutation(allInverseVtxPermutations[permForCase[caseNo]], baseCaseTesselations[baseCaseForCase[caseNo]][subCaseNo]) for subCaseNo in range(0, numSubCases(configForCase(caseNo))) ] for caseNo in range(0, 256) ] 
#tesselations = [ [ [ baseCaseForCase[caseNo], subCaseNo ] for subCaseNo in range(0, numSubCases(configForCase(caseNo))) ] for caseNo in range(0, 256) ] 

print(configForCase(246)," ", baseCaseForCase[246], " ", permForCase[246], " ", allVtxPermutations[permForCase[246]], " ", allInverseVtxPermutations[permForCase[246]], " ", baseCaseTesselations[16], " ", tesselations[246])
def subCaseNumberAfterPermutation(caseNo, subCaseNo):
    l= fillRightToLen(numberToReverseBinaryList(subCaseNo), 3, 0)
    axisPerm = allAxisPermutations[permForCase[caseNo]]
    lp =[ l[axisPerm[i]] for i in range(0,3) ]
    cc = configForCase(caseNo)
    if not subCasesInXPossible(cc):
        lp.pop(0)
    elif not subCasesInYPossible(cc):
        lp.pop(1)
    else:
        lp.pop(2)
    return faceConfToNumber(lp)

tesselationCasesWithFourSubcases = [ x[0] for x in enumerate(baseCaseForCase) if x[1] == 5 ] # Only base case 5 has four subcases

for caseNo in tesselationCasesWithFourSubcases:
    newOrder = [ subCaseNumberAfterPermutation(caseNo, x) for x in range(0,4) ]
    tesselations[caseNo] = [ tesselations[caseNo][newOrder[i]] for i in range(0,4) ]
    print("Case no.", caseNo, " config. ", configForCase(caseNo), " tesselations ", tesselations[caseNo])

tesselationList = []
tesselationStart = []

for caseNo in range(0,256):
   tesselationStart = tesselationStart + [[]]
   for subCaseNo in range(0, numSubCases(configForCase(caseNo))):
       tesselationStart[-1] = tesselationStart[-1] + [ len(tesselationList) ]
       tesselationList = tesselationList + delistify(tesselations[caseNo][subCaseNo]) + [ -1 ]
   tesselationStart[-1] = fillRightToLen(tesselationStart[-1], 4, -1)

print("Checking tesselations for derived cases.") 
for i in range(1, 256):
    print("Checking tesselation for case %d derived from base case %d" % (i, baseCaseForCase[i]))
    checkTess(configForCase(i), tesselations[i][0])

implFile = open("AMRStitchCellTesselations3D.C", "w");
implFile.write("// Automatically generated by generateCaseTable.py. Do not edit.\n\n");
implFile.write("#include \"AMRStitchCellTesselations3D.h\"\n")
implFile.write("\n")
implFile.write("int tesselationSubCaseDir3D[256] = {");
for i in range(0, 256):
    if (i % 16) == 0:
       implFile.write("\n    ")
    caseConfig = configForCase(i)
    subCaseDir = (boolToInt(subCasesInXPossible(caseConfig)))
    subCaseDir |= (boolToInt(subCasesInYPossible(caseConfig)) << 1)
    subCaseDir |= (boolToInt(subCasesInZPossible(caseConfig)) << 2)
    if i<255:
       implFile.write("%d, " % subCaseDir)
    else:
       implFile.write("%d" % subCaseDir)
implFile.write("\n};\n\n")

implFile.write("int tesselationArray3D[%d] = {" % len(tesselationList))
for i in range(0, len(tesselationList)):
    if (i % 25) == 0:
        implFile.write("\n    ")
    if i<len(tesselationList)-1:
        implFile.write("%d, " % tesselationList[i])
    else:
        implFile.write("%d" % tesselationList[i])
implFile.write("\n};\n\n")

implFile.write("int tesselationCaseStart3D[256][4] = {")

currLineLen = 80 # Enforce start with newline
for caseStart in tesselationStart[:-1]:
    caseStr = "{ %d, %d, %d, %d }, " % tuple(caseStart)
    if currLineLen + len(caseStr) > 80: 
        implFile.write("\n    ")
        currLineLen = 0
    implFile.write(caseStr)
    currLineLen += len(caseStr)
caseStr = "{ %d, %d, %d, %d }" % tuple(caseStart)
if currLineLen + len(caseStr) > 80: 
    implFile.write("\n    ")
    currLineLen = 0
implFile.write(caseStr)
implFile.write("\n};\n")
