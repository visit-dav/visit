# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  ProteinDataBank.py
#
#  Tests:      mesh      - 3D points
#              plots     - Molecule
#
#  Programmer: Brad Whitlock
#  Date:       Tue Mar 28 15:46:53 PST 2006
#
#  Modifications:
#    Jeremy Meredith, Tue Aug 29 13:23:30 EDT 2006
#    ProteinDataBank files now have models as directories, not time steps.
#
# ----------------------------------------------------------------------------

def LabelTest(testname, var, zoomview):
    AddPlot("Label", var)
    LabelAtts = LabelAttributes()
    LabelAtts.legendFlag = 1
    LabelAtts.showNodes = 0
    LabelAtts.showCells = 1
    LabelAtts.restrictNumberOfLabels = 0
    LabelAtts.drawLabelsFacing = LabelAtts.Front  # Front, Back, FrontAndBack
    LabelAtts.labelDisplayFormat = LabelAtts.Natural  # Natural, LogicalIndex, Index
    LabelAtts.numberOfLabels = 200
    LabelAtts.specifyTextColor1 = 1
    LabelAtts.textColor1 = (0, 255, 0, 255)
    LabelAtts.textHeight1 = 0.03
    LabelAtts.specifyTextColor2 = 0
    LabelAtts.textColor2 = (0, 0, 255, 0)
    LabelAtts.textHeight2 = 0.02
    LabelAtts.horizontalJustification = LabelAtts.HCenter  # HCenter, Left, Right
    LabelAtts.verticalJustification = LabelAtts.VCenter  # VCenter, Top, Bottom
    LabelAtts.depthTestMode = LabelAtts.LABEL_DT_AUTO  # LABEL_DT_AUTO, LABEL_DT_ALWAYS, LABEL_DT_NEVER
    SetPlotOptions(LabelAtts)
    DrawPlots()
    oldview = GetView3D()
    SetView3D(zoomview)
    # Save these images somewhat larger than a regular test case image
    # since the images contain a lot of text.
#    swa = SaveWindowAttributes()
#    swa.width = 500
#    swa.height = 500
#    swa.screenCapture = 0
#    Test(testname, swa)
    Test(testname)
    DeleteActivePlots()
    SetView3D(oldview)


# Set up the annotations
a = AnnotationAttributes()
a.userInfoFlag = 0
a.axes3D.visible = 0
a.axes3D.bboxFlag = 0
a.backgroundMode = a.Solid
a.foregroundColor = (0, 0, 0, 255)
a.backgroundColor = (255, 255, 255, 255)
a.databaseInfoFlag = 0
SetAnnotationAttributes(a)

# Create a plot using the large database
OpenDatabase("../data/ProteinDataBank_test_data/crotamine.pdb")
AddPlot("Molecule", "element")

MoleculeAtts = MoleculeAttributes()
MoleculeAtts.drawAtomsAs = MoleculeAtts.SphereAtoms  # NoAtoms, SphereAtoms, ImposterAtoms
MoleculeAtts.scaleRadiusBy = MoleculeAtts.Fixed  # Fixed, Covalent, Atomic, Variable
MoleculeAtts.drawBondsAs = MoleculeAtts.CylinderBonds  # NoBonds, LineBonds, CylinderBonds
MoleculeAtts.colorBonds = MoleculeAtts.ColorByAtom  # ColorByAtom, SingleColor
MoleculeAtts.bondSingleColor = (128, 128, 128, 255)
MoleculeAtts.radiusVariable = "default"
MoleculeAtts.radiusScaleFactor = 1
MoleculeAtts.radiusFixed = 0.4
MoleculeAtts.atomSphereQuality = MoleculeAtts.Medium  # Low, Medium, High, Super
MoleculeAtts.bondCylinderQuality = MoleculeAtts.Medium  # Low, Medium, High, Super
MoleculeAtts.bondRadius = 0.12
MoleculeAtts.bondLineWidth = 0
MoleculeAtts.bondLineStyle = 0
MoleculeAtts.elementColorTable = "cpk_jmol"
MoleculeAtts.residueTypeColorTable = "amino_shapely"
MoleculeAtts.residueSequenceColorTable = "Default"
MoleculeAtts.continuousColorTable = "Default"
MoleculeAtts.legendFlag = 1
MoleculeAtts.minFlag = 0
MoleculeAtts.scalarMin = 0
MoleculeAtts.maxFlag = 0
MoleculeAtts.scalarMax = 1
SetPlotOptions(MoleculeAtts)
DrawPlots()

v0 = View3DAttributes()
v0.viewNormal = (-0.967329, 0.252251, -0.0253765)
v0.focus = (31.726, -54.1675, 13.645)
v0.viewUp = (0.252129, 0.967661, 0.0079404)
v0.viewAngle = 30
v0.parallelScale = 24.9831
v0.nearPlane = -49.9661
v0.farPlane = 49.9661
v0.imagePan = (0, 0)
v0.imageZoom = 1.44471
v0.perspective = 1
v0.eyeAngle = 2
v0.centerOfRotationSet = 0
v0.centerOfRotation = (31.726, -54.1675, 13.645)
SetView3D(v0)

v0zoom = View3DAttributes()
v0zoom.viewNormal = (-0.967329, 0.252251, -0.0253765)
v0zoom.focus = (31.726, -54.1675, 13.645)
v0zoom.viewUp = (0.252129, 0.967661, 0.0079404)
v0zoom.viewAngle = 30
v0zoom.parallelScale = 24.9831
v0zoom.nearPlane = -49.9661
v0zoom.farPlane = 49.9661
v0zoom.imagePan = (0, 0)
v0zoom.imageZoom = 7.15293
v0zoom.perspective = 1
v0zoom.eyeAngle = 2
v0zoom.centerOfRotationSet = 0
v0zoom.centerOfRotation = (31.726, -54.1675, 13.645)

TestSection("Testing Rattlesnake venom")
Test("proteindb_0_00")
LabelTest("proteindb_0_01", "elementname", v0zoom)

ChangeActivePlotsVar("resseq")
Test("proteindb_0_02")
LabelTest("proteindb_0_03", "resseq", v0zoom)

ChangeActivePlotsVar("backbone")
Test("proteindb_0_04")

ChangeActivePlotsVar("restype")
Test("proteindb_0_05")
LabelTest("proteindb_0_06", "resname", v0zoom)

LabelTest("proteindb_0_07", "longresname", v0zoom)


#
# Try with a different dataset
#
TestSection("Testing small DNA")
ReplaceDatabase("../data/ProteinDataBank_test_data/1NTS.pdb")
ChangeActivePlotsVar("element")

v1 = View3DAttributes()
v1.viewNormal = (-0.320353, 0.944248, 0.075961)
v1.focus = (-0.0580001, 0.0915003, 0.3815)
v1.viewUp = (0.342959, 0.190354, -0.919861)
v1.viewAngle = 30
v1.parallelScale = 22.575
v1.nearPlane = -45.1501
v1.farPlane = 45.1501
v1.imagePan = (-0.0021177, -0.0481532)
v1.imageZoom = 1.27797
v1.perspective = 1
v1.eyeAngle = 2
v1.centerOfRotationSet = 0
v1.centerOfRotation = (-0.0580001, 0.0915003, 0.3815)
SetView3D(v1)

v1zoom = View3DAttributes()
v1zoom.viewNormal = (-0.320353, 0.944248, 0.075961)
v1zoom.focus = (-0.0580001, 0.0915003, 0.3815)
v1zoom.viewUp = (0.342959, 0.190354, -0.919861)
v1zoom.viewAngle = 30
v1zoom.parallelScale = 22.575
v1zoom.nearPlane = -45.1501
v1zoom.farPlane = 45.1501
v1zoom.imagePan = (-0.00906313, 0.0442979)
v1zoom.imageZoom = 6.4294
v1zoom.perspective = 1
v1zoom.eyeAngle = 2
v1zoom.centerOfRotationSet = 0
v1zoom.centerOfRotation = (-0.0580001, 0.0915003, 0.3815)

Test("proteindb_1_00")
LabelTest("proteindb_1_01", "elementname", v1zoom)

ChangeActivePlotsVar("resseq")
Test("proteindb_1_02")
LabelTest("proteindb_1_03", "resseq", v1zoom)

ChangeActivePlotsVar("backbone")
Test("proteindb_1_04")

ChangeActivePlotsVar("restype")
Test("proteindb_1_05")
LabelTest("proteindb_1_06", "resname", v1zoom)

LabelTest("proteindb_1_07", "longresname", v1zoom)


#
# Try with a different dataset
#
ReplaceDatabase("../data/ProteinDataBank_test_data/1UZ9.pdb")
ChangeActivePlotsVar("element")

v2 = View3DAttributes()
v2.viewNormal = (0.215329, 0.245957, 0.94506)
v2.focus = (23.441, 26.835, 23.6865)
v2.viewUp = (-0.351063, 0.922561, -0.160113)
v2.viewAngle = 30
v2.parallelScale = 29.1931
v2.nearPlane = -58.3862
v2.farPlane = 58.3862
v2.imagePan = (0.0260607, 0.00408113)
v2.imageZoom = 1.8463
v2.perspective = 1
v2.eyeAngle = 2
v2.centerOfRotationSet = 0
v2.centerOfRotation = (23.441, 26.835, 23.6865)
SetView3D(v2)

v2zoom = View3DAttributes()
v2zoom.viewNormal = (0.685414, 0.259247, 0.68044)
v2zoom.focus = (23.441, 26.835, 23.6865)
v2zoom.viewUp = (0.700183, 0.02186, -0.713629)
v2zoom.viewAngle = 30
v2zoom.parallelScale = 29.1931
v2zoom.nearPlane = -58.3862
v2zoom.farPlane = 58.3862
v2zoom.imagePan = (-0.0257104, -0.00810227)
v2zoom.imageZoom = 10.3892
v2zoom.perspective = 1
v2zoom.eyeAngle = 2
v2zoom.centerOfRotationSet = 0
v2zoom.centerOfRotation = (23.441, 26.835, 23.6865)

TestSection("Testing insulin")
Test("proteindb_2_00")
LabelTest("proteindb_2_01", "elementname", v2zoom)

ChangeActivePlotsVar("resseq")
Test("proteindb_2_02")
LabelTest("proteindb_2_03", "resseq", v2zoom)

ChangeActivePlotsVar("backbone")
Test("proteindb_2_04")

ChangeActivePlotsVar("restype")
Test("proteindb_2_05")
LabelTest("proteindb_2_06", "resname", v2zoom)

LabelTest("proteindb_2_07", "longresname", v2zoom)

#
# Try with a different dataset
#
ReplaceDatabase("../data/ProteinDataBank_test_data/1TFS.pdb")
ChangeActivePlotsVar("element")

v3 = View3DAttributes()
v3.viewNormal = (-0.242177, -0.689536, 0.682562)
v3.focus = (-1.73, -1.927, -0.202)
v3.viewUp = (0.243612, 0.637752, 0.730702)
v3.viewAngle = 30
v3.parallelScale = 25.6826
v3.nearPlane = -51.3652
v3.farPlane = 51.3652
v3.imagePan = (0.0337528, -0.0400135)
v3.imageZoom = 1.49054
v3.perspective = 1
v3.eyeAngle = 2
v3.centerOfRotationSet = 0
v3.centerOfRotation = (-1.73, -1.927, -0.202)
SetView3D(v3)

v3zoom = View3DAttributes()
v3zoom.viewNormal = (-0.558032, -0.716666, 0.418318)
v3zoom.focus = (-1.73, -1.927, -0.202)
v3zoom.viewUp = (0.120358, 0.428875, 0.89531)
v3zoom.viewAngle = 30
v3zoom.parallelScale = 25.6826
v3zoom.nearPlane = -51.3652
v3zoom.farPlane = 51.3652
v3zoom.imagePan = (0.0337528, -0.0400135)
v3zoom.imageZoom = 8.39928
v3zoom.perspective = 1
v3zoom.eyeAngle = 2
v3zoom.centerOfRotationSet = 0
v3zoom.centerOfRotation = (-1.73, -1.927, -0.202)

TestSection("Testing Black Mamba venom")
Test("proteindb_3_00")
LabelTest("proteindb_3_01", "elementname", v3zoom)

ChangeActivePlotsVar("resseq")
Test("proteindb_3_02")
LabelTest("proteindb_3_03", "resseq", v3zoom)

ChangeActivePlotsVar("backbone")
Test("proteindb_3_04")

ChangeActivePlotsVar("restype")
Test("proteindb_3_05")
LabelTest("proteindb_3_06", "resname", v3zoom)

LabelTest("proteindb_3_07", "longresname", v3zoom)

# Make sure that there are multiple models in the file.
ChangeActivePlotsVar("element")
ChangeActivePlotsVar("models/model_09/element")
Test("proteindb_3_08")
ChangeActivePlotsVar("models/model_19/element")
Test("proteindb_3_09")

Exit()

