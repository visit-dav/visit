# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  molecule.py
#
#  Tests:      mesh      - 3D points
#              plots     - Molecule
#              operators - CreateBonds, Replicate
#
#  Programmer: Kathleen Biagas 
#  Date:       June 15, 2021 
#
#  Modifications:
#
# ----------------------------------------------------------------------------

def SetGradientBackground():
    annot = GetAnnotationAttributes() 
    annot.backgroundMode = annot.Gradient
    annot.gradientBackgroundStyle = annot.Radial
    annot.gradientColor1 = (102, 102, 153, 255)
    annot.gradientColor2 = (0, 0, 0, 255)
    annot.backgroundColor = (0, 0, 0, 255)
    annot.foregroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(annot)

def SetWhiteBackground():
    annot = GetAnnotationAttributes() 
    annot.backgroundMode = annot.Solid
    annot.foregroundColor = (0, 0, 0, 255)
    annot.backgroundColor = (255, 255, 255, 255)
    SetAnnotationAttributes(annot)

def SaveTestImage(name):
    # Save these images somewhat larger than a regular test case image
    # to better see the molecules 
    backup = GetSaveWindowAttributes()
    swa = SaveWindowAttributes()
    swa.width = 500
    swa.height = 500
    swa.screenCapture = 1
    Test(name, swa)
    SetSaveWindowAttributes(backup)


def MoleculeOnly():
    # images similar to those in Molecule Plot docs

    SetGradientBackground()

    # add specular highlighting
    renAtts = GetRenderingAttributes()
    renAtts.specularFlag = 1
    SetRenderingAttributes(renAtts)

    OpenDatabase(data_path("ProteinDataBank_test_data/crotamine.pdb"))

    # color by element, Covalent radius, no bonds
    AddPlot("Molecule", "element")
    mol = MoleculeAttributes()
    mol.drawAtomsAs   = mol.SphereAtoms
    mol.scaleRadiusBy = mol.Covalent
    mol.drawBondsAs   = mol.NoBonds
    SetPlotOptions(mol)

    DrawPlots()

    v3d = GetView3D()
    v3d.viewNormal = (0.784142, -0.592494, -0.184587)
    v3d.viewUp = (-0.554863, -0.536159, -0.636129)
    v3d.imageZoom = 1.77156
    SetView3D(v3d)

    SaveTestImage("molecule_01")

    # color by residue, cylinder bonds, radius proportional to covalent radius
    ChangeActivePlotsVar("restype")
    mol.drawBondsAs = mol.CylinderBonds
    mol.radiusScaleFactor = 0.5
    SetPlotOptions(mol)
    DrawPlots()
    SaveTestImage("molecule_02")

    # color by scalar (x-coord), no bonds
    DefineScalarExpression("x", "coord(mesh)[0]")
    ChangeActivePlotsVar("x")    
    mol.drawAtomsAs = mol.NoAtoms
    SetPlotOptions(mol)
    DrawPlots()
    SaveTestImage("molecule_03")

    # Create a blue-purple color table
    ccpl = ColorControlPointList()
    ccpl.discreteFlag=1
    ccpl.smoothing=ccpl.NONE
    ccpl.equalSpacingFlag=1
    p1 = ColorControlPoint()
    p1.colors = (51, 51, 153, 255)
    p1.position =  0
    ccpl.AddControlPoints(p1)
    p2 = ColorControlPoint()
    p2.colors = (204,153, 255, 255)
    p2.position =  1
    ccpl.AddControlPoints(p2)
    AddColorTable("BluePurp2", ccpl)

    # color by backbone, atom and cylinder-bonds same fixed radius
    # special color table
    ChangeActivePlotsVar("backbone")
    mol.continuousColorTable="BluePurp2"
    mol.drawAtomsAs = mol.SphereAtoms
    mol.drawBondsAs = mol.CylinderBonds
    mol.scaleRadiusBy = mol.Fixed
    mol.radiusFixed = 0.2
    mol.bondRadius = 0.2
    SetPlotOptions(mol)

    DrawPlots()
    # turn off specular highlighting
    renAtts = RenderingAttributes()
    renAtts.specularFlag = 0
    SetRenderingAttributes(renAtts)
    SaveTestImage("molecule_04")

    DeleteAllPlots()

    # ensure engine/viewer doesn't crash when using ImposterAtoms and scaleRadiusBy option is changed
    AddPlot("Molecule", "element")
    # get Fresh atts
    mol = MoleculeAttributes()
    # set up sphere imposters
    mol.drawAtomsAs = mol.ImposterAtoms
    SetPlotOptions(mol)
    DrawPlots()
    SaveTestImage("molecule_05")
    # now change scaleRadiusBy
    mol.scaleRadiusBy = mol.Covalent
    mol.radiusScaleFactor=4
    SetPlotOptions(mol)
    DrawPlots()
    SaveTestImage("molecule_06")

    # changing radiusScaleFactor while using Imposter atoms used to crash the viewer
    mol.radiusScaleFactor=0.002
    SetPlotOptions(mol)
    DrawPlots()
    SaveTestImage("molecule_07")

    DeleteAllPlots()
    CloseDatabase(data_path("ProteinDataBank_test_data/crotamine.pdb"))

def ReplicateAddBonds():
    TestSection("Replicate and CreateBonds operators with Molecule plot")
    SetWhiteBackground()
    OpenDatabase(data_path("vasp_test_data", "GaO40W12", "OUTCAR"))
    AddPlot("Mesh", "unitCell")
    AddPlot("Molecule", "element")
    mol = MoleculeAttributes()
    mol.radiusFixed=0.5
    SetPlotOptions(mol)
    DrawPlots()
    ResetView()

    v3D = GetView3D()
    v3D.viewNormal = (0.0378647, -0.776117, -0.62945) #(-0.465303, -0.758273, 0.456634)
    v3D.viewUp = (0.839533, -0.366352, -0.401212) #(-0.0844518, 0.55156, 0.829849)
    v3D.imageZoom=2.177156
    v3D.imagePan=(0.0217552, 0.0165363)
    SetView3D(v3D)

    SaveTestImage("mol_rep_bonds_01")
  
    # Add replicate operator 
    SetActivePlots((0,1)) 
    AddOperator("Replicate", 1)
    repl = ReplicateAttributes()
    repl.useUnitCellVectors = 1
    repl.mergeResults = 1
    # replicate along unit cell boundaries
    repl.replicateUnitCellAtoms = 1
    SetOperatorOptions(repl, 0, 1)
    DrawPlots()  
    SaveTestImage("mol_rep_bonds_02")

    # create a replication along y
    repl.yReplications=2
    SetOperatorOptions(repl, 1, 1)
    DrawPlots()  

    SaveTestImage("mol_rep_bonds_03")

    # Create bonds 
    SetActivePlots(1) 
    AddOperator("CreateBonds")
    cb = CreateBondsAttributes()
    cb.atomicNumber1 = (31, 74)
    cb.atomicNumber2 = (8, 8)
    cb.minDist = (0.4, 0.4)
    cb.maxDist = (1.9, 2.4)
    SetOperatorOptions(cb)
    DrawPlots()  

    SaveTestImage("mol_rep_bonds_04")

    # have the CreateBonds operator create periodic bonds
    cb.addPeriodicBonds = 1
    cb.useUnitCellVectors = 1
    SetOperatorOptions(cb)
    DrawPlots()  

    SaveTestImage("mol_rep_bonds_05")

    # Change bond style to lines
    mol.drawBondsAs = mol.LineBonds
    mol.bondLineWidth = 5
    SetPlotOptions(mol)
    DrawPlots()  

    SaveTestImage("mol_rep_bonds_06")

    # Change atom radius
    mol.drawBondsAs = mol.CylinderBonds
    mol.scaleRadiusBy = mol.Atomic
    mol.radiusScaleFactor = 0.7
    SetPlotOptions(mol)
    DrawPlots()  

    SaveTestImage("mol_rep_bonds_07")

    DeleteAllPlots()
    CloseDatabase(data_path("vasp_test_data", "GaO40W12", "OUTCAR"))

MoleculeOnly()
ReplicateAddBonds()
Exit()


