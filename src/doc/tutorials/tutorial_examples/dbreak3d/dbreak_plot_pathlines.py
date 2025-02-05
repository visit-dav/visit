#
# file: dbreak_plot_pathlines.py
# info: Source in VisIt's Commands window with both
#       dbreak3d_fluid.visit and dbreak3d_boundaries.silo open.
#

def select_dbreak3d_fluid_db():
    srcs = GetGlobalAttributes().sources
    for src in srcs:
        if src.count("dbreak3d_fluid"):
            ActivateDatabase(src)

def select_dbreak3d_bnds_db():
    srcs = GetGlobalAttributes().sources
    for src in srcs:
        if src.count("dbreak3d_bound"):
            ActivateDatabase(src)

def add_water_plot():
    select_dbreak3d_fluid_db()
    AddPlot("Pseudocolor", "alpha1")
    AddOperator("Isovolume")
    iso_atts = IsovolumeAttributes()
    iso_atts.lbound = 0.5
    iso_atts.ubound = 1e+37
    iso_atts.variable = "alpha1"
    SetOperatorOptions(iso_atts)
    pc_atts = PseudocolorAttributes()
    pc_atts.scaling = pc_atts.Linear  # Linear, Log, Skew
    pc_atts.skewFactor = 1
    pc_atts.limitsMode = pc_atts.OriginalData  # OriginalData, CurrentPlot
    pc_atts.minFlag = 0
    pc_atts.maxFlag = 0
    pc_atts.centering = pc_atts.Natural  # Natural, Nodal, Zonal
    pc_atts.colorTableName = "PuBu"
    pc_atts.invertColorTable = 0
    pc_atts.opacityType = pc_atts.FullyOpaque  # ColorTable, FullyOpaque, Constant, Ramp, VariableRange
    pc_atts.renderSurfaces = 1
    pc_atts.renderWireframe = 0
    pc_atts.renderPoints = 0
    pc_atts.smoothingLevel = 0
    pc_atts.legendFlag = 1
    pc_atts.lightingFlag = 1
    SetPlotOptions(pc_atts)

def add_tank_bnds_plot():
    select_dbreak3d_bnds_db()
    AddPlot("Subset", "domains")
    silr = SILRestriction()
    silr.SuspendCorrectnessChecking()
    silr.TurnOnAll()
    for silSet in (5,6):
        silr.TurnOffSet(silSet)
    silr.EnableCorrectnessChecking()
    SetPlotSILRestriction(silr)
    sub_atts = SubsetAttributes()
    sub_atts.colorType = sub_atts.ColorBySingleColor  # ColorBySingleColor, ColorByMultipleColors, 
    sub_atts.legendFlag = 1
    sub_atts.singleColor = (204, 255, 204, 255)
    sub_atts.subsetNames = ("1", "2", "3", "4", "5", "6")
    sub_atts.opacity = 1
    sub_atts.wireframe = 0
    sub_atts.drawInternal = 0
    sub_atts.smoothingLevel = 0
    SetPlotOptions(sub_atts)

def add_pathline_plot():
    select_dbreak3d_fluid_db()
    TimeSliderSetState(0)
    AddPlot("Pseudocolor","operators/IntegralCurve/U")
    iatts = IntegralCurveAttributes()
    iatts.sourceType = iatts.SpecifiedPlane  
    iatts.planeOrigin = (0.12, 0.1, 0.15)
    iatts.planeNormal = (1, 0, 0)
    iatts.planeUpAxis = (0, 1, 0)
    iatts.sampleDensity0 = 8
    iatts.sampleDensity1 = 5
    iatts.sampleDistance0 = 0.225
    iatts.sampleDistance1 = 0.2
    iatts.dataValue = iatts.TimeAbsolute
    iatts.terminateByTime = 1
    iatts.termTime = 4
    iatts.pathlines = 1
    iatts.pathlinesCMFE = iatts.CONN_CMFE
    iatts.issueStiffnessWarnings = 0
    iatts.issueCriticalPointsWarnings = 0
    iatts.issueBoundaryWarnings = 0
    iatts.issueAdvectionWarnings = 0
    SetOperatorOptions(iatts)
    # set style options
    patts = PseudocolorAttributes()
    patts.colorTableName = "RdYlBu"
    patts.lineType = patts.Tube 
    patts.tailStyle = patts.Spheres
    patts.headStyle = patts.Cones
    patts.endPointRadiusBBox = 0.01
    patts.endPointRatio = 5
    SetPlotOptions(patts)
    

    
def setup_pathlines():
    DeleteAllPlots()
    add_water_plot()
    add_tank_bnds_plot()
    add_pathline_plot()
    DrawPlots()
    
setup_pathlines()
    
