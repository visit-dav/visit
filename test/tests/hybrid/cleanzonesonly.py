# ----------------------------------------------------------------------------
#  CLASSES: nightly
#
#  Test Case:  cleanzonesonly.py
#
#  Tests:      plots     - filled boundary
#
#  Defect ID:  '5135
#
#  Programmer: Jeremy Meredith
#  Date:       July  6, 2004
#
# ----------------------------------------------------------------------------

# Turn off all annotation but plot legends
a = AnnotationAttributes()
TurnOffAllAnnotations(a)
a.legendInfoFlag = 1
SetAnnotationAttributes(a)

# -----------------------------------------------------------------------------
#    multicolor mode -- normal plot, then CZO on, then CZO off
# -----------------------------------------------------------------------------
TestSection("Multicolor mode")
OpenDatabase("../data/rect2d.silo")

AddPlot("FilledBoundary", "mat1");
DrawPlots()

v=GetView2D()
v.viewportCoords=(0.3, 0.9, 0.15, 0.9)
SetView2D(v)

Test("hybrid_cleanzonesonly_01")

m = FilledBoundaryAttributes()
m.cleanZonesOnly = 1
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_02")

m.cleanZonesOnly = 0
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_03")

DeleteAllPlots()

# -----------------------------------------------------------------------------
#    continuous colortable mode -- normal plot, then CZO on, then CZO off
# -----------------------------------------------------------------------------
TestSection("Continuous colortable mode")
AddPlot("FilledBoundary", "mat1");

m = FilledBoundaryAttributes();
m.colorType = m.ColorByColorTable
m.colorTableName = 'calewhite'
m.mixedColor=(0,100,100,255)
SetPlotOptions(m)
DrawPlots()

Test("hybrid_cleanzonesonly_04")

m = FilledBoundaryAttributes()
m.cleanZonesOnly = 1
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_05")

m.cleanZonesOnly = 0
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_06")

DeleteAllPlots()

# -----------------------------------------------------------------------------
#    discrete colortable mode -- normal plot, then CZO on, then CZO off
# -----------------------------------------------------------------------------
TestSection("Discrete colortable mode")
AddPlot("FilledBoundary", "mat1");

m = FilledBoundaryAttributes();
m.colorType = m.ColorByColorTable
m.colorTableName = 'levels'
m.mixedColor=(0,0,0,255)
SetPlotOptions(m)
DrawPlots()

Test("hybrid_cleanzonesonly_07")

m = FilledBoundaryAttributes()
m.cleanZonesOnly = 1
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_08")

m.cleanZonesOnly = 0
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_09")

DeleteAllPlots()

# -----------------------------------------------------------------------------
#    singlecolor mode -- normal plot, then CZO on, then CZO off
# -----------------------------------------------------------------------------
TestSection("Singlecolor mode")
AddPlot("FilledBoundary", "mat1");

m = FilledBoundaryAttributes();
m.colorType = m.ColorBySingleColor
m.singleColor = (255, 0, 0, 255)
SetPlotOptions(m)
DrawPlots()

Test("hybrid_cleanzonesonly_10")

m = FilledBoundaryAttributes()
m.cleanZonesOnly = 1
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_11")

m.cleanZonesOnly = 0
SetPlotOptions(m)

Test("hybrid_cleanzonesonly_12")

Exit()
