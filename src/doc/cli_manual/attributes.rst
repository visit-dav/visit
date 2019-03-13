
Attribute Reference
===================

This chapter shows all the attributes that can be set to control the
behavior of VisIt. The attributes themselves are not documented, but
their names are usually quite explanatory. When a member of an attribute
can take values from a given list of options, the default option is printed
first in italic followed by a comma separated list of the other available
options.

The listing is ordered in alphabetical ordering of the name of the
attribute set. For each set the function that will provide you with
these attributes is printed in italic.


.. _AMRStitchCellAttributes:

AMRStitchCellAttributes()
--------------------------------------


  - **CreateCellsOfType**: *DualGridAndStitchCells*, DualGrid, StitchCells


.. _AnimationAttributes:

AnimationAttributes()
------------------------------


  - **animationMode**: *StopMode*, ReversePlayMode, PlayMode
  - **pipelineCachingMode**: 0
  - **frameIncrement**: 1
  - **timeout**: 1
  - **playbackMode**: *Looping*, PlayOnce, Swing


.. _AnnotationAttributes:

AnnotationAttributes()
--------------------------------


  - **axes2D.visible**: 1
  - **axes2D.autoSetTicks**: 1
  - **axes2D.autoSetScaling**: 1
  - **axes2D.lineWidth**: 0
  - **axes2D.tickLocation**: *Outside*, Inside, Both
  - **axes2D.tickAxes**: *BottomLeft*, Off, Bottom, Left, All
  - **axes2D.xAxis.title.visible**: 1
  - **axes2D.xAxis.title.font.font**: *Courier*, Arial, Times
  - **axes2D.xAxis.title.font.scale**: 1
  - **axes2D.xAxis.title.font.useForegroundColor**: 1
  - **axes2D.xAxis.title.font.color**: (0, 0, 0, 255)
  - **axes2D.xAxis.title.font.bold**: 1
  - **axes2D.xAxis.title.font.italic**: 1
  - **axes2D.xAxis.title.userTitle**: 0
  - **axes2D.xAxis.title.userUnits**: 0
  - **axes2D.xAxis.title.title**: "X-Axis"
  - **axes2D.xAxis.title.units**: ""
  - **axes2D.xAxis.label.visible**: 1
  - **axes2D.xAxis.label.font.font**: *Courier*, Arial, Times
  - **axes2D.xAxis.label.font.scale**: 1
  - **axes2D.xAxis.label.font.useForegroundColor**: 1
  - **axes2D.xAxis.label.font.color**: (0, 0, 0, 255)
  - **axes2D.xAxis.label.font.bold**: 1
  - **axes2D.xAxis.label.font.italic**: 1
  - **axes2D.xAxis.label.scaling**: 0
  - **axes2D.xAxis.tickMarks.visible**: 1
  - **axes2D.xAxis.tickMarks.majorMinimum**: 0
  - **axes2D.xAxis.tickMarks.majorMaximum**: 1
  - **axes2D.xAxis.tickMarks.minorSpacing**: 0.02
  - **axes2D.xAxis.tickMarks.majorSpacing**: 0.2
  - **axes2D.xAxis.grid**: 0
  - **axes2D.yAxis.title.visible**: 1
  - **axes2D.yAxis.title.font.font**: *Courier*, Arial, Times
  - **axes2D.yAxis.title.font.scale**: 1
  - **axes2D.yAxis.title.font.useForegroundColor**: 1
  - **axes2D.yAxis.title.font.color**: (0, 0, 0, 255)
  - **axes2D.yAxis.title.font.bold**: 1
  - **axes2D.yAxis.title.font.italic**: 1
  - **axes2D.yAxis.title.userTitle**: 0
  - **axes2D.yAxis.title.userUnits**: 0
  - **axes2D.yAxis.title.title**: "Y-Axis"
  - **axes2D.yAxis.title.units**: ""
  - **axes2D.yAxis.label.visible**: 1
  - **axes2D.yAxis.label.font.font**: *Courier*, Arial, Times
  - **axes2D.yAxis.label.font.scale**: 1
  - **axes2D.yAxis.label.font.useForegroundColor**: 1
  - **axes2D.yAxis.label.font.color**: (0, 0, 0, 255)
  - **axes2D.yAxis.label.font.bold**: 1
  - **axes2D.yAxis.label.font.italic**: 1
  - **axes2D.yAxis.label.scaling**: 0
  - **axes2D.yAxis.tickMarks.visible**: 1
  - **axes2D.yAxis.tickMarks.majorMinimum**: 0
  - **axes2D.yAxis.tickMarks.majorMaximum**: 1
  - **axes2D.yAxis.tickMarks.minorSpacing**: 0.02
  - **axes2D.yAxis.tickMarks.majorSpacing**: 0.2
  - **axes2D.yAxis.grid**: 0
  - **axes3D.visible**: 1
  - **axes3D.autoSetTicks**: 1
  - **axes3D.autoSetScaling**: 1
  - **axes3D.lineWidth**: 0
  - **axes3D.tickLocation**: *Inside*, Outside, Both
  - **axes3D.axesType**: *ClosestTriad*, FurthestTriad, OutsideEdges, StaticTriad, StaticEdges
  - **axes3D.triadFlag**: 1
  - **axes3D.bboxFlag**: 1
  - **axes3D.xAxis.title.visible**: 1
  - **axes3D.xAxis.title.font.font**: *Arial*, Courier, Times
  - **axes3D.xAxis.title.font.scale**: 1
  - **axes3D.xAxis.title.font.useForegroundColor**: 1
  - **axes3D.xAxis.title.font.color**: (0, 0, 0, 255)
  - **axes3D.xAxis.title.font.bold**: 0
  - **axes3D.xAxis.title.font.italic**: 0
  - **axes3D.xAxis.title.userTitle**: 0
  - **axes3D.xAxis.title.userUnits**: 0
  - **axes3D.xAxis.title.title**: "X-Axis"
  - **axes3D.xAxis.title.units**: ""
  - **axes3D.xAxis.label.visible**: 1
  - **axes3D.xAxis.label.font.font**: *Arial*, Courier, Times
  - **axes3D.xAxis.label.font.scale**: 1
  - **axes3D.xAxis.label.font.useForegroundColor**: 1
  - **axes3D.xAxis.label.font.color**: (0, 0, 0, 255)
  - **axes3D.xAxis.label.font.bold**: 0
  - **axes3D.xAxis.label.font.italic**: 0
  - **axes3D.xAxis.label.scaling**: 0
  - **axes3D.xAxis.tickMarks.visible**: 1
  - **axes3D.xAxis.tickMarks.majorMinimum**: 0
  - **axes3D.xAxis.tickMarks.majorMaximum**: 1
  - **axes3D.xAxis.tickMarks.minorSpacing**: 0.02
  - **axes3D.xAxis.tickMarks.majorSpacing**: 0.2
  - **axes3D.xAxis.grid**: 0
  - **axes3D.yAxis.title.visible**: 1
  - **axes3D.yAxis.title.font.font**: *Arial*, Courier, Times
  - **axes3D.yAxis.title.font.scale**: 1
  - **axes3D.yAxis.title.font.useForegroundColor**: 1
  - **axes3D.yAxis.title.font.color**: (0, 0, 0, 255)
  - **axes3D.yAxis.title.font.bold**: 0
  - **axes3D.yAxis.title.font.italic**: 0
  - **axes3D.yAxis.title.userTitle**: 0
  - **axes3D.yAxis.title.userUnits**: 0
  - **axes3D.yAxis.title.title**: "Y-Axis"
  - **axes3D.yAxis.title.units**: ""
  - **axes3D.yAxis.label.visible**: 1
  - **axes3D.yAxis.label.font.font**: *Arial*, Courier, Times
  - **axes3D.yAxis.label.font.scale**: 1
  - **axes3D.yAxis.label.font.useForegroundColor**: 1
  - **axes3D.yAxis.label.font.color**: (0, 0, 0, 255)
  - **axes3D.yAxis.label.font.bold**: 0
  - **axes3D.yAxis.label.font.italic**: 0
  - **axes3D.yAxis.label.scaling**: 0
  - **axes3D.yAxis.tickMarks.visible**: 1
  - **axes3D.yAxis.tickMarks.majorMinimum**: 0
  - **axes3D.yAxis.tickMarks.majorMaximum**: 1
  - **axes3D.yAxis.tickMarks.minorSpacing**: 0.02
  - **axes3D.yAxis.tickMarks.majorSpacing**: 0.2
  - **axes3D.yAxis.grid**: 0
  - **axes3D.zAxis.title.visible**: 1
  - **axes3D.zAxis.title.font.font**: *Arial*, Courier, Times
  - **axes3D.zAxis.title.font.scale**: 1
  - **axes3D.zAxis.title.font.useForegroundColor**: 1
  - **axes3D.zAxis.title.font.color**: (0, 0, 0, 255)
  - **axes3D.zAxis.title.font.bold**: 0
  - **axes3D.zAxis.title.font.italic**: 0
  - **axes3D.zAxis.title.userTitle**: 0
  - **axes3D.zAxis.title.userUnits**: 0
  - **axes3D.zAxis.title.title**: "Z-Axis"
  - **axes3D.zAxis.title.units**: ""
  - **axes3D.zAxis.label.visible**: 1
  - **axes3D.zAxis.label.font.font**: *Arial*, Courier, Times
  - **axes3D.zAxis.label.font.scale**: 1
  - **axes3D.zAxis.label.font.useForegroundColor**: 1
  - **axes3D.zAxis.label.font.color**: (0, 0, 0, 255)
  - **axes3D.zAxis.label.font.bold**: 0
  - **axes3D.zAxis.label.font.italic**: 0
  - **axes3D.zAxis.label.scaling**: 0
  - **axes3D.zAxis.tickMarks.visible**: 1
  - **axes3D.zAxis.tickMarks.majorMinimum**: 0
  - **axes3D.zAxis.tickMarks.majorMaximum**: 1
  - **axes3D.zAxis.tickMarks.minorSpacing**: 0.02
  - **axes3D.zAxis.tickMarks.majorSpacing**: 0.2
  - **axes3D.zAxis.grid**: 0
  - **axes3D.setBBoxLocation**: 0
  - **axes3D.bboxLocation**: (0, 1, 0, 1, 0, 1)
  - **userInfoFlag**: 1
  - **userInfoFont.font**: *Arial*, Courier, Times
  - **userInfoFont.scale**: 1
  - **userInfoFont.useForegroundColor**: 1
  - **userInfoFont.color**: (0, 0, 0, 255)
  - **userInfoFont.bold**: 0
  - **userInfoFont.italic**: 0
  - **databaseInfoFlag**: 1
  - **timeInfoFlag**: 1
  - **databaseInfoFont.font**: *Arial*, Courier, Times
  - **databaseInfoFont.scale**: 1
  - **databaseInfoFont.useForegroundColor**: 1
  - **databaseInfoFont.color**: (0, 0, 0, 255)
  - **databaseInfoFont.bold**: 0
  - **databaseInfoFont.italic**: 0
  - **databaseInfoExpansionMode**: *File*, Directory, Full, Smart, SmartDirectory
  - **databaseInfoTimeScale**: 1
  - **databaseInfoTimeOffset**: 0
  - **legendInfoFlag**: 1
  - **backgroundColor**: (255, 255, 255, 255)
  - **foregroundColor**: (0, 0, 0, 255)
  - **gradientBackgroundStyle**: *Radial*, TopToBottom, BottomToTop, LeftToRight, RightToLeft
  - **gradientColor1**: (0, 0, 255, 255)
  - **gradientColor2**: (0, 0, 0, 255)
  - **backgroundMode**: *Solid*, Gradient, Image, ImageSphere
  - **backgroundImage**: ""
  - **imageRepeatX**: 1
  - **imageRepeatY**: 1
  - **axesArray.visible**: 1
  - **axesArray.ticksVisible**: 1
  - **axesArray.autoSetTicks**: 1
  - **axesArray.autoSetScaling**: 1
  - **axesArray.lineWidth**: 0
  - **axesArray.axes.title.visible**: 1
  - **axesArray.axes.title.font.font**: *Arial*, Courier, Times
  - **axesArray.axes.title.font.scale**: 1
  - **axesArray.axes.title.font.useForegroundColor**: 1
  - **axesArray.axes.title.font.color**: (0, 0, 0, 255)
  - **axesArray.axes.title.font.bold**: 0
  - **axesArray.axes.title.font.italic**: 0
  - **axesArray.axes.title.userTitle**: 0
  - **axesArray.axes.title.userUnits**: 0
  - **axesArray.axes.title.title**: ""
  - **axesArray.axes.title.units**: ""
  - **axesArray.axes.label.visible**: 1
  - **axesArray.axes.label.font.font**: *Arial*, Courier, Times
  - **axesArray.axes.label.font.scale**: 1
  - **axesArray.axes.label.font.useForegroundColor**: 1
  - **axesArray.axes.label.font.color**: (0, 0, 0, 255)
  - **axesArray.axes.label.font.bold**: 0
  - **axesArray.axes.label.font.italic**: 0
  - **axesArray.axes.label.scaling**: 0
  - **axesArray.axes.tickMarks.visible**: 1
  - **axesArray.axes.tickMarks.majorMinimum**: 0
  - **axesArray.axes.tickMarks.majorMaximum**: 1
  - **axesArray.axes.tickMarks.minorSpacing**: 0.02
  - **axesArray.axes.tickMarks.majorSpacing**: 0.2
  - **axesArray.axes.grid**: *0*


.. _AxisAttributes:

AxisAttributes()
--------------------


  - **title.visible**: 1
  - **title.font.font**: *Arial*, Courier, Times
  - **title.font.scale**: 1
  - **title.font.useForegroundColor**: 1
  - **title.font.color**: (0, 0, 0, 255)
  - **title.font.bold**: 0
  - **title.font.italic**: 0
  - **title.userTitle**: 0
  - **title.userUnits**: 0
  - **title.title**: ""
  - **title.units**: ""
  - **label.visible**: 1
  - **label.font.font**: *Arial*, Courier, Times
  - **label.font.scale**: 1
  - **label.font.useForegroundColor**: 1
  - **label.font.color**: (0, 0, 0, 255)
  - **label.font.bold**: 0
  - **label.font.italic**: 0
  - **label.scaling**: 0
  - **tickMarks.visible**: 1
  - **tickMarks.majorMinimum**: 0
  - **tickMarks.majorMaximum**: 1
  - **tickMarks.minorSpacing**: 0.02
  - **tickMarks.majorSpacing**: 0.2
  - **grid**: *0*


.. _BoundaryAttributes:

BoundaryAttributes()
----------------------------


  - **colorType**: *ColorByMultipleColors*, ColorBySingleColor, ColorByColorTable
  - **colorTableName**: "Default"
  - **invertColorTable**: 0
  - **filledFlag**: 1
  - **legendFlag**: 1
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **singleColor**: (0, 0, 0, 255)
  - **boundaryNames**: ()
  - **boundaryType**: *Unknown*, Domain, Group, Material
  - **opacity**: 1
  - **wireframe**: 0
  - **smoothingLevel**: 0
  - **pointSize**: 0.05
  - **pointType**: *Point*, Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Sphere
  - **pointSizeVarEnabled**: 0
  - **pointSizeVar**: "default"
  - **pointSizePixels**: *2*


.. _BoundaryOpAttributes:

BoundaryOpAttributes()
--------------------------------


  - **smoothingLevel**: *0*


.. _BoxAttributes:

BoxAttributes()
------------------


  - **amount**: *Some*, All
  - **minx**: 0
  - **maxx**: 1
  - **miny**: 0
  - **maxy**: 1
  - **minz**: 0
  - **maxz**: 1
  - **inverse**: *0*


.. _CartographicProjectionAttributes:

CartographicProjectionAttributes()
--------------------------------------------------------


  - **projectionID**: *aitoff*, eck4, eqdc, hammer, laea, lcc, merc, mill, moll, ortho, wink2
  - **centralMeridian**: *0*


.. _ClipAttributes:

ClipAttributes()
--------------------


  - **quality**: *Fast*, Accurate
  - **funcType**: *Plane*, Sphere
  - **plane1Status**: 1
  - **plane2Status**: 0
  - **plane3Status**: 0
  - **plane1Origin**: (0, 0, 0)
  - **plane2Origin**: (0, 0, 0)
  - **plane3Origin**: (0, 0, 0)
  - **plane1Normal**: (1, 0, 0)
  - **plane2Normal**: (0, 1, 0)
  - **plane3Normal**: (0, 0, 1)
  - **planeInverse**: 0
  - **planeToolControlledClipPlane**: *Plane1*, None, Plane2, Plane3
  - **center**: (0, 0, 0)
  - **radius**: 1
  - **sphereInverse**: *0*


.. _ConeAttributes:

ConeAttributes()
--------------------


  - **angle**: 45
  - **origin**: (0, 0, 0)
  - **normal**: (0, 0, 1)
  - **representation**: *Flattened*, ThreeD, R_Theta
  - **upAxis**: (0, 1, 0)
  - **cutByLength**: 0
  - **length**: *1*


.. _ConnectedComponentsAttributes:

ConnectedComponentsAttributes()
--------------------------------------------------


  - **EnableGhostNeighborsOptimization**: *1*


.. _ConstructDataBinningAttributes:

ConstructDataBinningAttributes()
----------------------------------------------------


  - **name**: ""
  - **varnames**: ()
  - **binType**: ()
  - **binBoundaries**: ()
  - **reductionOperator**: *Average*, Minimum, Maximum, StandardDeviation, Variance, Sum, Count, RMS, PDF
  - **varForReductionOperator**: ""
  - **undefinedValue**: 0
  - **binningScheme**: *Uniform*, Unknown
  - **numBins**: ()
  - **overTime**: 0
  - **timeStart**: 0
  - **timeEnd**: 1
  - **timeStride**: 1
  - **outOfBoundsBehavior**: *Clamp*, Discard


.. _ContourAttributes:

ContourAttributes()
--------------------------


  - **defaultPalette.GetControlPoints(0).colors**: (255, 0, 0, 255)
  - **defaultPalette.GetControlPoints(0).position**: 0
  - **defaultPalette.GetControlPoints(1).colors**: (0, 255, 0, 255)
  - **defaultPalette.GetControlPoints(1).position**: 0.034
  - **defaultPalette.GetControlPoints(2).colors**: (0, 0, 255, 255)
  - **defaultPalette.GetControlPoints(2).position**: 0.069
  - **defaultPalette.GetControlPoints(3).colors**: (0, 255, 255, 255)
  - **defaultPalette.GetControlPoints(3).position**: 0.103
  - **defaultPalette.GetControlPoints(4).colors**: (255, 0, 255, 255)
  - **defaultPalette.GetControlPoints(4).position**: 0.138
  - **defaultPalette.GetControlPoints(5).colors**: (255, 255, 0, 255)
  - **defaultPalette.GetControlPoints(5).position**: 0.172
  - **defaultPalette.GetControlPoints(6).colors**: (255, 135, 0, 255)
  - **defaultPalette.GetControlPoints(6).position**: 0.207
  - **defaultPalette.GetControlPoints(7).colors**: (255, 0, 135, 255)
  - **defaultPalette.GetControlPoints(7).position**: 0.241
  - **defaultPalette.GetControlPoints(8).colors**: (168, 168, 168, 255)
  - **defaultPalette.GetControlPoints(8).position**: 0.276
  - **defaultPalette.GetControlPoints(9).colors**: (255, 68, 68, 255)
  - **defaultPalette.GetControlPoints(9).position**: 0.31
  - **defaultPalette.GetControlPoints(10).colors**: (99, 255, 99, 255)
  - **defaultPalette.GetControlPoints(10).position**: 0.345
  - **defaultPalette.GetControlPoints(11).colors**: (99, 99, 255, 255)
  - **defaultPalette.GetControlPoints(11).position**: 0.379
  - **defaultPalette.GetControlPoints(12).colors**: (40, 165, 165, 255)
  - **defaultPalette.GetControlPoints(12).position**: 0.414
  - **defaultPalette.GetControlPoints(13).colors**: (255, 99, 255, 255)
  - **defaultPalette.GetControlPoints(13).position**: 0.448
  - **defaultPalette.GetControlPoints(14).colors**: (255, 255, 99, 255)
  - **defaultPalette.GetControlPoints(14).position**: 0.483
  - **defaultPalette.GetControlPoints(15).colors**: (255, 170, 99, 255)
  - **defaultPalette.GetControlPoints(15).position**: 0.517
  - **defaultPalette.GetControlPoints(16).colors**: (170, 79, 255, 255)
  - **defaultPalette.GetControlPoints(16).position**: 0.552
  - **defaultPalette.GetControlPoints(17).colors**: (150, 0, 0, 255)
  - **defaultPalette.GetControlPoints(17).position**: 0.586
  - **defaultPalette.GetControlPoints(18).colors**: (0, 150, 0, 255)
  - **defaultPalette.GetControlPoints(18).position**: 0.621
  - **defaultPalette.GetControlPoints(19).colors**: (0, 0, 150, 255)
  - **defaultPalette.GetControlPoints(19).position**: 0.655
  - **defaultPalette.GetControlPoints(20).colors**: (0, 109, 109, 255)
  - **defaultPalette.GetControlPoints(20).position**: 0.69
  - **defaultPalette.GetControlPoints(21).colors**: (150, 0, 150, 255)
  - **defaultPalette.GetControlPoints(21).position**: 0.724
  - **defaultPalette.GetControlPoints(22).colors**: (150, 150, 0, 255)
  - **defaultPalette.GetControlPoints(22).position**: 0.759
  - **defaultPalette.GetControlPoints(23).colors**: (150, 84, 0, 255)
  - **defaultPalette.GetControlPoints(23).position**: 0.793
  - **defaultPalette.GetControlPoints(24).colors**: (160, 0, 79, 255)
  - **defaultPalette.GetControlPoints(24).position**: 0.828
  - **defaultPalette.GetControlPoints(25).colors**: (255, 104, 28, 255)
  - **defaultPalette.GetControlPoints(25).position**: 0.862
  - **defaultPalette.GetControlPoints(26).colors**: (0, 170, 81, 255)
  - **defaultPalette.GetControlPoints(26).position**: 0.897
  - **defaultPalette.GetControlPoints(27).colors**: (68, 255, 124, 255)
  - **defaultPalette.GetControlPoints(27).position**: 0.931
  - **defaultPalette.GetControlPoints(28).colors**: (0, 130, 255, 255)
  - **defaultPalette.GetControlPoints(28).position**: 0.966
  - **defaultPalette.GetControlPoints(29).colors**: (130, 0, 255, 255)
  - **defaultPalette.GetControlPoints(29).position**: 1
  - **defaultPalette.smoothing**: *None*, Linear, CubicSpline
  - **defaultPalette.equalSpacingFlag**: 1
  - **defaultPalette.discreteFlag**: 1
  - **defaultPalette.categoryName**: "Standard"
  - **changedColors**: ()
  - **colorType**: *ColorByMultipleColors*, ColorBySingleColor, ColorByColorTable
  - **colorTableName**: "Default"
  - **invertColorTable**: 0
  - **legendFlag**: 1
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **singleColor**: *(255, 0, 0, 255)*,  SetMultiColor(0,,  (255, 0, 0, 255)),  SetMultiColor(1, (0,,  255, 0, 255)),  SetMultiColor(2, (0,,  0, 255, 255)),  SetMultiColor(3, (0,,  255, 255, 255)),  SetMultiColor(4,,  (255, 0, 255, 255)),  SetMultiColor(5,,  (255, 255, 0, 255)),  SetMultiColor(6,,  (255, 135, 0, 255)),  SetMultiColor(7,,  (255, 0, 135, 255)),  SetMultiColor(8,,  (168, 168, 168, 255)),  SetMultiColor(9,,  (255, 68, 68, 255))
  - **contourNLevels**: 10
  - **contourValue**: ()
  - **contourPercent**: ()
  - **contourMethod**: *Level*, Value, Percent
  - **minFlag**: 0
  - **maxFlag**: 0
  - **min**: 0
  - **max**: 1
  - **scaling**: *Linear*, Log
  - **wireframe**: *0*


.. _CoordSwapAttributes:

CoordSwapAttributes()
------------------------------


  - **newCoord1**: *Coord1*, Coord2, Coord3
  - **newCoord2**: *Coord2*, Coord1, Coord3
  - **newCoord3**: *Coord3*, Coord1, Coord2


.. _CreateBondsAttributes:

CreateBondsAttributes()
----------------------------------


  - **elementVariable**: "element"
  - **atomicNumber1**: (1, -1)
  - **atomicNumber2**: (-1, -1)
  - **minDist**: (0.4, 0.4)
  - **maxDist**: (1.2, 1.9)
  - **maxBondsClamp**: 10
  - **addPeriodicBonds**: 0
  - **useUnitCellVectors**: 1
  - **periodicInX**: 1
  - **periodicInY**: 1
  - **periodicInZ**: 1
  - **xVector**: (1, 0, 0)
  - **yVector**: (0, 1, 0)
  - **zVector**: *(0, 0, 1)*


.. _CurveAttributes:

CurveAttributes()
----------------------


  - **showLines**: 1
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **showPoints**: 0
  - **symbol**: *Point*, TriangleUp, TriangleDown, Square, Circle, Plus, X
  - **pointSize**: 5
  - **pointFillMode**: *Static*, Dynamic
  - **pointStride**: 1
  - **symbolDensity**: 50
  - **curveColorSource**: *Cycle*, Custom
  - **curveColor**: (0, 0, 0, 255)
  - **showLegend**: 1
  - **showLabels**: 1
  - **designator**: ""
  - **doBallTimeCue**: 0
  - **ballTimeCueColor**: (0, 0, 0, 255)
  - **timeCueBallSize**: 0.01
  - **doLineTimeCue**: 0
  - **lineTimeCueColor**: (0, 0, 0, 255)
  - **lineTimeCueWidth**: 0
  - **doCropTimeCue**: 0
  - **timeForTimeCue**: 0
  - **fillMode**: *NoFill*, Solid, HorizontalGradient, VerticalGradient
  - **fillColor1**: (255, 0, 0, 255)
  - **fillColor2**: (255, 100, 100, 255)
  - **polarToCartesian**: 0
  - **polarCoordinateOrder**: *R_Theta*, Theta_R
  - **angleUnits**: *Radians*, Degrees


.. _CylinderAttributes:

CylinderAttributes()
----------------------------


  - **point1**: (0, 0, 0)
  - **point2**: (1, 0, 0)
  - **radius**: 1
  - **inverse**: *0*


.. _DataBinningAttributes:

DataBinningAttributes()
----------------------------------


  - **numDimensions**: *One*, Two, Three
  - **dim1BinBasedOn**: *Variable*, X, Y, Z
  - **dim1Var**: "default"
  - **dim1SpecifyRange**: 0
  - **dim1MinRange**: 0
  - **dim1MaxRange**: 1
  - **dim1NumBins**: 50
  - **dim2BinBasedOn**: *Variable*, X, Y, Z
  - **dim2Var**: "default"
  - **dim2SpecifyRange**: 0
  - **dim2MinRange**: 0
  - **dim2MaxRange**: 1
  - **dim2NumBins**: 50
  - **dim3BinBasedOn**: *Variable*, X, Y, Z
  - **dim3Var**: "default"
  - **dim3SpecifyRange**: 0
  - **dim3MinRange**: 0
  - **dim3MaxRange**: 1
  - **dim3NumBins**: 50
  - **outOfBoundsBehavior**: *Clamp*, Discard
  - **reductionOperator**: *Average*, Minimum, Maximum, StandardDeviation, Variance, Sum, Count, RMS, PDF
  - **varForReduction**: "default"
  - **emptyVal**: 0
  - **outputType**: *OutputOnBins*, OutputOnInputMesh
  - **removeEmptyValFromCurve**: *1*


.. _DeferExpressionAttributes:

DeferExpressionAttributes()
------------------------------------------


  - **exprs**: *()*


.. _DisplaceAttributes:

DisplaceAttributes()
----------------------------


  - **factor**: 1
  - **variable**: *"default"*


.. _DualMeshAttributes:

DualMeshAttributes()
----------------------------


  - **mode**: *Auto*, NodesToZones, ZonesToNodes


.. _EdgeAttributes:

EdgeAttributes()
--------------------


  - **dummy**: *1*


.. _ElevateAttributes:

ElevateAttributes()
--------------------------


  - **useXYLimits**: 0
  - **limitsMode**: *OriginalData*, CurrentPlot
  - **scaling**: *Linear*, Log, Skew
  - **skewFactor**: 1
  - **minFlag**: 0
  - **min**: 0
  - **maxFlag**: 0
  - **max**: 1
  - **zeroFlag**: 0
  - **variable**: *"default"*


.. _EllipsoidSliceAttributes:

EllipsoidSliceAttributes()
----------------------------------------


  - **origin**: (0, 0, 0)
  - **radii**: (1, 1, 1)
  - **rotationAngle**: *(0, 0, 0)*


.. _ExportDBAttributes:

ExportDBAttributes()
----------------------------


  - **allTimes**: 0
  - **db_type**: ""
  - **db_type_fullname**: ""
  - **filename**: "visit_ex_db"
  - **dirname**: "."
  - **variables**: ()
  - **writeUsingGroups**: 0
  - **groupSize**: 48
  - **opts.types**: ()
  - **opts.help**: *""*


.. _ExternalSurfaceAttributes:

ExternalSurfaceAttributes()
------------------------------------------


  - **removeGhosts**: 0
  - **edgesIn2D**: *1*


.. _ExtrudeAttributes:

ExtrudeAttributes()
--------------------------


  - **axis**: (0, 0, 1)
  - **length**: 1
  - **steps**: 30
  - **preserveOriginalCellNumbers**: *1*


.. _FFTAttributes:

FFTAttributes()
------------------


  - **dummy**: *0*


.. _FilledBoundaryAttributes:

FilledBoundaryAttributes()
----------------------------------------


  - **colorType**: *ColorByMultipleColors*, ColorBySingleColor, ColorByColorTable
  - **colorTableName**: "Default"
  - **invertColorTable**: 0
  - **filledFlag**: 1
  - **legendFlag**: 1
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **singleColor**: (0, 0, 0, 255)
  - **boundaryNames**: ()
  - **boundaryType**: *Unknown*, Domain, Group, Material
  - **opacity**: 1
  - **wireframe**: 0
  - **drawInternal**: 0
  - **smoothingLevel**: 0
  - **cleanZonesOnly**: 0
  - **mixedColor**: (255, 255, 255, 255)
  - **pointSize**: 0.05
  - **pointType**: *Point*, Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Sphere
  - **pointSizeVarEnabled**: 0
  - **pointSizeVar**: "default"
  - **pointSizePixels**: *2*


.. _FluxAttributes:

FluxAttributes()
--------------------


  - **flowField**: "default"
  - **weight**: 0
  - **weightField**: *"default"*


.. _FontAttributes:

FontAttributes()
--------------------


  - **font**: *Arial*, Courier, Times
  - **scale**: 1
  - **useForegroundColor**: 1
  - **color**: (0, 0, 0, 255)
  - **bold**: 0
  - **italic**: *0*


.. _GlobalAttributes:

GlobalAttributes()
------------------------


  - **sources**: ()
  - **windows**: (1)
  - **activeWindow**: 0
  - **iconifiedFlag**: 0
  - **autoUpdateFlag**: 0
  - **replacePlots**: 0
  - **applyOperator**: 1
  - **applySelection**: 1
  - **applyWindow**: 0
  - **executing**: 0
  - **windowLayout**: 1
  - **makeDefaultConfirm**: 1
  - **cloneWindowOnFirstRef**: 1
  - **automaticallyAddOperator**: 0
  - **tryHarderCyclesTimes**: 0
  - **treatAllDBsAsTimeVarying**: 0
  - **createMeshQualityExpressions**: 1
  - **createTimeDerivativeExpressions**: 1
  - **createVectorMagnitudeExpressions**: 1
  - **newPlotsInheritSILRestriction**: 1
  - **userDirForSessionFiles**: 0
  - **saveCrashRecoveryFile**: 1
  - **ignoreExtentsFromDbs**: 0
  - **expandNewPlots**: 0
  - **userRestoreSessionFile**: 0
  - **precisionType**: *Native*, Float, Double
  - **backendType**: *VTK*, DAX, EAVL, PISTON
  - **removeDuplicateNodes**: *0*


.. _HistogramAttributes:

HistogramAttributes()
------------------------------


  - **basedOn**: *ManyZonesForSingleVar*, ManyVarsForSingleZone
  - **histogramType**: *Frequency*, Weighted, Variable
  - **weightVariable**: "default"
  - **limitsMode**: *OriginalData*, CurrentPlot
  - **minFlag**: 0
  - **maxFlag**: 0
  - **min**: 0
  - **max**: 1
  - **numBins**: 32
  - **domain**: 0
  - **zone**: 0
  - **useBinWidths**: 1
  - **outputType**: *Block*, Curve
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **color**: (200, 80, 40, 255)
  - **dataScale**: *Linear*, Log, SquareRoot
  - **binScale**: *Linear*, Log, SquareRoot
  - **normalizeHistogram**: 0
  - **computeAsCDF**: *0*


.. _IndexSelectAttributes:

IndexSelectAttributes()
----------------------------------


  - **maxDim**: *ThreeD*, OneD, TwoD
  - **dim**: *TwoD*, OneD, ThreeD
  - **xAbsMax**: -1
  - **xMin**: 0
  - **xMax**: -1
  - **xIncr**: 1
  - **xWrap**: 0
  - **yAbsMax**: -1
  - **yMin**: 0
  - **yMax**: -1
  - **yIncr**: 1
  - **yWrap**: 0
  - **zAbsMax**: -1
  - **zMin**: 0
  - **zMax**: -1
  - **zIncr**: 1
  - **zWrap**: 0
  - **useWholeCollection**: 1
  - **categoryName**: "Whole"
  - **subsetName**: *"Whole"*


.. _IntegralCurveAttributes:

IntegralCurveAttributes()
--------------------------------------


  - **sourceType**: *Point*, PointList, Line, Circle, Plane, Sphere, Box, Selection, FieldData
  - **pointSource**: (0, 0, 0)
  - **lineStart**: (0, 0, 0)
  - **lineEnd**: (1, 0, 0)
  - **planeOrigin**: (0, 0, 0)
  - **planeNormal**: (0, 0, 1)
  - **planeUpAxis**: (0, 1, 0)
  - **radius**: 1
  - **sphereOrigin**: (0, 0, 0)
  - **boxExtents**: (0, 1, 0, 1, 0, 1)
  - **useWholeBox**: 1
  - **pointList**: (0, 0, 0, 1, 0, 0, 0, 1, 0)
  - **fieldData**: ()
  - **sampleDensity0**: 2
  - **sampleDensity1**: 2
  - **sampleDensity2**: 2
  - **dataValue**: *TimeAbsolute*, Solid, SeedPointID, Speed, Vorticity, ArcLength, TimeRelative, AverageDistanceFromSeed, CorrelationDistance, Difference, Variable
  - **dataVariable**: ""
  - **integrationDirection**: *Forward*, Backward, Both, ForwardDirectionless, BackwardDirectionless, BothDirectionless
  - **maxSteps**: 1000
  - **terminateByDistance**: 0
  - **termDistance**: 10
  - **terminateByTime**: 0
  - **termTime**: 10
  - **maxStepLength**: 0.1
  - **limitMaximumTimestep**: 0
  - **maxTimeStep**: 0.1
  - **relTol**: 0.0001
  - **absTolSizeType**: *FractionOfBBox*, Absolute
  - **absTolAbsolute**: 1e-06
  - **absTolBBox**: 1e-06
  - **fieldType**: *Default*, FlashField, M3DC12DField, M3DC13DField, Nek5000Field, NektarPPField, NIMRODField
  - **fieldConstant**: 1
  - **velocitySource**: (0, 0, 0)
  - **integrationType**: *DormandPrince*, Euler, Leapfrog, AdamsBashforth, RK4, M3DC12DIntegrator
  - **parallelizationAlgorithmType**: *VisItSelects*, LoadOnDemand, ParallelStaticDomains, MasterSlave
  - **maxProcessCount**: 10
  - **maxDomainCacheSize**: 3
  - **workGroupSize**: 32
  - **pathlines**: 0
  - **pathlinesOverrideStartingTimeFlag**: 0
  - **pathlinesOverrideStartingTime**: 0
  - **pathlinesPeriod**: 0
  - **pathlinesCMFE**: *POS_CMFE*, CONN_CMFE
  - **displayGeometry**: *Lines*, Tubes, Ribbons
  - **cropBeginFlag**: 0
  - **cropBegin**: 0
  - **cropEndFlag**: 0
  - **cropEnd**: 0
  - **cropValue**: *Time*, Distance, StepNumber
  - **sampleDistance0**: 10
  - **sampleDistance1**: 10
  - **sampleDistance2**: 10
  - **fillInterior**: 1
  - **randomSamples**: 0
  - **randomSeed**: 0
  - **numberOfRandomSamples**: 1
  - **issueAdvectionWarnings**: 1
  - **issueBoundaryWarnings**: 1
  - **issueTerminationWarnings**: 1
  - **issueStepsizeWarnings**: 1
  - **issueStiffnessWarnings**: 1
  - **issueCriticalPointsWarnings**: 1
  - **criticalPointThreshold**: 0.001
  - **correlationDistanceAngTol**: 5
  - **correlationDistanceMinDistAbsolute**: 1
  - **correlationDistanceMinDistBBox**: 0.005
  - **correlationDistanceMinDistType**: *FractionOfBBox*, Absolute
  - **selection**: *""*


.. _InverseGhostZoneAttributes:

InverseGhostZoneAttributes()
--------------------------------------------


  - **requestGhostZones**: 1
  - **showDuplicated**: 1
  - **showEnhancedConnectivity**: 1
  - **showReducedConnectivity**: 1
  - **showAMRRefined**: 1
  - **showExterior**: 1
  - **showNotApplicable**: *1*


.. _IsosurfaceAttributes:

IsosurfaceAttributes()
--------------------------------


  - **contourNLevels**: 10
  - **contourValue**: ()
  - **contourPercent**: ()
  - **contourMethod**: *Level*, Value, Percent
  - **minFlag**: 0
  - **min**: 0
  - **maxFlag**: 0
  - **max**: 1
  - **scaling**: *Linear*, Log
  - **variable**: *"default"*


.. _IsovolumeAttributes:

IsovolumeAttributes()
------------------------------


  - **lbound**: -1e+37
  - **ubound**: 1e+37
  - **variable**: *"default"*


.. _KeyframeAttributes:

KeyframeAttributes()
----------------------------


  - **enabled**: 0
  - **nFrames**: 1
  - **nFramesWasUserSet**: *0*


.. _LCSAttributes:

LCSAttributes()
------------------


  - **sourceType**: *NativeMesh*, RegularGrid
  - **Resolution**: (10, 10, 10)
  - **UseDataSetStart**: *Full*, Subset
  - **StartPosition**: (0, 0, 0)
  - **UseDataSetEnd**: *Full*, Subset
  - **EndPosition**: (1, 1, 1)
  - **integrationDirection**: *Forward*, Backward, Both
  - **auxiliaryGrid**: *None*, TwoDim, ThreeDim
  - **auxiliaryGridSpacing**: 0.0001
  - **maxSteps**: 1000
  - **operationType**: *Lyapunov*, IntegrationTime, ArcLength, AverageDistanceFromSeed, EigenValue, EigenVector
  - **cauchyGreenTensor**: *Right*, Left
  - **eigenComponent**: *Largest*, Smallest, Intermediate, PosShearVector, NegShearVector, PosLambdaShearVector, NegLambdaShearVector
  - **eigenWeight**: 1
  - **operatorType**: *BaseValue*, Gradient
  - **terminationType**: *Time*, Distance, Size
  - **terminateBySize**: 0
  - **termSize**: 10
  - **terminateByDistance**: 0
  - **termDistance**: 10
  - **terminateByTime**: 0
  - **termTime**: 10
  - **maxStepLength**: 0.1
  - **limitMaximumTimestep**: 0
  - **maxTimeStep**: 0.1
  - **relTol**: 0.0001
  - **absTolSizeType**: *FractionOfBBox*, Absolute
  - **absTolAbsolute**: 1e-06
  - **absTolBBox**: 1e-06
  - **fieldType**: *Default*, FlashField, M3DC12DField, M3DC13DField, Nek5000Field, NektarPPField, NIMRODField
  - **fieldConstant**: 1
  - **velocitySource**: (0, 0, 0)
  - **integrationType**: *DormandPrince*, Euler, Leapfrog, AdamsBashforth, RK4, M3DC12DIntegrator
  - **clampLogValues**: 0
  - **parallelizationAlgorithmType**: *VisItSelects*, LoadOnDemand, ParallelStaticDomains, MasterSlave
  - **maxProcessCount**: 10
  - **maxDomainCacheSize**: 3
  - **workGroupSize**: 32
  - **pathlines**: 0
  - **pathlinesOverrideStartingTimeFlag**: 0
  - **pathlinesOverrideStartingTime**: 0
  - **pathlinesPeriod**: 0
  - **pathlinesCMFE**: *POS_CMFE*, CONN_CMFE
  - **thresholdLimit**: 0.1
  - **radialLimit**: 0.1
  - **boundaryLimit**: 0.1
  - **seedLimit**: 10
  - **issueAdvectionWarnings**: 1
  - **issueBoundaryWarnings**: 1
  - **issueTerminationWarnings**: 1
  - **issueStepsizeWarnings**: 1
  - **issueStiffnessWarnings**: 1
  - **issueCriticalPointsWarnings**: 1
  - **criticalPointThreshold**: *0.001*


.. _LabelAttributes:

LabelAttributes()
----------------------


  - **legendFlag**: 1
  - **showNodes**: 0
  - **showCells**: 1
  - **restrictNumberOfLabels**: 1
  - **drawLabelsFacing**: *Front*, Back, FrontAndBack
  - **labelDisplayFormat**: *Natural*, LogicalIndex, Index
  - **numberOfLabels**: 200
  - **specifyTextColor1**: 0
  - **textColor1**: (255, 0, 0, 0)
  - **textHeight1**: 0.02
  - **specifyTextColor2**: 0
  - **textColor2**: (0, 0, 255, 0)
  - **textHeight2**: 0.02
  - **horizontalJustification**: *HCenter*, Left, Right
  - **verticalJustification**: *VCenter*, Top, Bottom
  - **depthTestMode**: *LABEL_DT_AUTO*, LABEL_DT_ALWAYS, LABEL_DT_NEVER
  - **formatTemplate**: *"%g"*


.. _LagrangianAttributes:

LagrangianAttributes()
--------------------------------


  - **seedPoint**: (0, 0, 0)
  - **numSteps**: 1000
  - **XAxisSample**: *Step*, Time, ArcLength, Speed, Vorticity, Variable
  - **YAxisSample**: *Step*, Time, ArcLength, Speed, Vorticity, Variable
  - **variable**: *"default"*


.. _LightAttributes:

LightAttributes()
----------------------


  - **enabledFlag**: 1
  - **type**: *Camera*, Ambient, Object
  - **direction**: (0, 0, -1)
  - **color**: (255, 255, 255, 255)
  - **brightness**: *1*


.. _LimitCycleAttributes:

LimitCycleAttributes()
--------------------------------


  - **sourceType**: *Line_*, Plane
  - **lineStart**: (0, 0, 0)
  - **lineEnd**: (1, 0, 0)
  - **planeOrigin**: (0, 0, 0)
  - **planeNormal**: (0, 0, 1)
  - **planeUpAxis**: (0, 1, 0)
  - **sampleDensity0**: 2
  - **sampleDensity1**: 2
  - **dataValue**: *TimeAbsolute*, Solid, SeedPointID, Speed, Vorticity, ArcLength, TimeRelative, AverageDistanceFromSeed, CorrelationDistance, Difference, Variable
  - **dataVariable**: ""
  - **integrationDirection**: *Forward*, Backward, Both, ForwardDirectionless, BackwardDirectionless, BothDirectionless
  - **maxSteps**: 1000
  - **terminateByDistance**: 0
  - **termDistance**: 10
  - **terminateByTime**: 0
  - **termTime**: 10
  - **maxStepLength**: 0.1
  - **limitMaximumTimestep**: 0
  - **maxTimeStep**: 0.1
  - **relTol**: 0.0001
  - **absTolSizeType**: *FractionOfBBox*, Absolute
  - **absTolAbsolute**: 1e-06
  - **absTolBBox**: 1e-06
  - **fieldType**: *Default*, FlashField, M3DC12DField, M3DC13DField, Nek5000Field, NektarPPField, NIMRODField
  - **fieldConstant**: 1
  - **velocitySource**: (0, 0, 0)
  - **integrationType**: *DormandPrince*, Euler, Leapfrog, AdamsBashforth, RK4, M3DC12DIntegrator
  - **parallelizationAlgorithmType**: *VisItSelects*, LoadOnDemand, ParallelStaticDomains, MasterSlave
  - **maxProcessCount**: 10
  - **maxDomainCacheSize**: 3
  - **workGroupSize**: 32
  - **pathlines**: 0
  - **pathlinesOverrideStartingTimeFlag**: 0
  - **pathlinesOverrideStartingTime**: 0
  - **pathlinesPeriod**: 0
  - **pathlinesCMFE**: *POS_CMFE*, CONN_CMFE
  - **sampleDistance0**: 10
  - **sampleDistance1**: 10
  - **sampleDistance2**: 10
  - **fillInterior**: 1
  - **randomSamples**: 0
  - **randomSeed**: 0
  - **numberOfRandomSamples**: 1
  - **forceNodeCenteredData**: 0
  - **cycleTolerance**: 1e-06
  - **maxIterations**: 10
  - **showPartialResults**: 1
  - **showReturnDistances**: 0
  - **issueAdvectionWarnings**: 1
  - **issueBoundaryWarnings**: 1
  - **issueTerminationWarnings**: 1
  - **issueStepsizeWarnings**: 1
  - **issueStiffnessWarnings**: 1
  - **issueCriticalPointsWarnings**: 1
  - **criticalPointThreshold**: 0.001
  - **correlationDistanceAngTol**: 5
  - **correlationDistanceMinDistAbsolute**: 1
  - **correlationDistanceMinDistBBox**: 0.005
  - **correlationDistanceMinDistType**: *FractionOfBBox*, Absolute


.. _LineoutAttributes:

LineoutAttributes()
--------------------------


  - **point1**: (0, 0, 0)
  - **point2**: (1, 1, 0)
  - **interactive**: 0
  - **ignoreGlobal**: 0
  - **samplingOn**: 0
  - **numberOfSamplePoints**: 50
  - **reflineLabels**: *0*


.. _MaterialAttributes:

MaterialAttributes()
----------------------------


  - **smoothing**: 0
  - **forceMIR**: 0
  - **cleanZonesOnly**: 0
  - **needValidConnectivity**: 0
  - **algorithm**: *EquiZ*, EquiT, Isovolume, PLIC, Discrete
  - **iterationEnabled**: 0
  - **numIterations**: 5
  - **iterationDamping**: 0.4
  - **simplifyHeavilyMixedZones**: 0
  - **maxMaterialsPerZone**: 3
  - **isoVolumeFraction**: 0.5
  - **annealingTime**: *10*


.. _MeshAttributes:

MeshAttributes()
--------------------


  - **legendFlag**: 1
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **meshColor**: (0, 0, 0, 255)
  - **meshColorSource**: *Foreground*, MeshCustom
  - **opaqueColorSource**: *Background*, OpaqueCustom
  - **opaqueMode**: *Auto*, On, Off
  - **pointSize**: 0.05
  - **opaqueColor**: (255, 255, 255, 255)
  - **smoothingLevel**: *None*, Fast, High
  - **pointSizeVarEnabled**: 0
  - **pointSizeVar**: "default"
  - **pointType**: *Point*, Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Sphere
  - **showInternal**: 0
  - **pointSizePixels**: 2
  - **opacity**: *1*


.. _MeshManagementAttributes:

MeshManagementAttributes()
----------------------------------------


  - **discretizationTolerance**: (0.02, 0.025, 0.05)
  - **discretizationToleranceX**: ()
  - **discretizationToleranceY**: ()
  - **discretizationToleranceZ**: ()
  - **discretizationMode**: *Uniform*, Adaptive, MultiPass
  - **discretizeBoundaryOnly**: 0
  - **passNativeCSG**: *0*


.. _MoleculeAttributes:

MoleculeAttributes()
----------------------------


  - **drawAtomsAs**: *SphereAtoms*, NoAtoms, ImposterAtoms
  - **scaleRadiusBy**: *Fixed*, Covalent, Atomic, Variable
  - **drawBondsAs**: *CylinderBonds*, NoBonds, LineBonds
  - **colorBonds**: *ColorByAtom*, SingleColor
  - **bondSingleColor**: (128, 128, 128, 255)
  - **radiusVariable**: "default"
  - **radiusScaleFactor**: 1
  - **radiusFixed**: 0.3
  - **atomSphereQuality**: *Medium*, Low, High, Super
  - **bondCylinderQuality**: *Medium*, Low, High, Super
  - **bondRadius**: 0.12
  - **bondLineWidth**: 0
  - **bondLineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **elementColorTable**: "cpk_jmol"
  - **residueTypeColorTable**: "amino_shapely"
  - **residueSequenceColorTable**: "Default"
  - **continuousColorTable**: "Default"
  - **legendFlag**: 1
  - **minFlag**: 0
  - **scalarMin**: 0
  - **maxFlag**: 0
  - **scalarMax**: *1*


.. _MultiCurveAttributes:

MultiCurveAttributes()
--------------------------------


  - **defaultPalette.GetControlPoints(0).colors**: (255, 0, 0, 255)
  - **defaultPalette.GetControlPoints(0).position**: 0
  - **defaultPalette.GetControlPoints(1).colors**: (0, 255, 0, 255)
  - **defaultPalette.GetControlPoints(1).position**: 0.034
  - **defaultPalette.GetControlPoints(2).colors**: (0, 0, 255, 255)
  - **defaultPalette.GetControlPoints(2).position**: 0.069
  - **defaultPalette.GetControlPoints(3).colors**: (0, 255, 255, 255)
  - **defaultPalette.GetControlPoints(3).position**: 0.103
  - **defaultPalette.GetControlPoints(4).colors**: (255, 0, 255, 255)
  - **defaultPalette.GetControlPoints(4).position**: 0.138
  - **defaultPalette.GetControlPoints(5).colors**: (255, 255, 0, 255)
  - **defaultPalette.GetControlPoints(5).position**: 0.172
  - **defaultPalette.GetControlPoints(6).colors**: (255, 135, 0, 255)
  - **defaultPalette.GetControlPoints(6).position**: 0.207
  - **defaultPalette.GetControlPoints(7).colors**: (255, 0, 135, 255)
  - **defaultPalette.GetControlPoints(7).position**: 0.241
  - **defaultPalette.GetControlPoints(8).colors**: (168, 168, 168, 255)
  - **defaultPalette.GetControlPoints(8).position**: 0.276
  - **defaultPalette.GetControlPoints(9).colors**: (255, 68, 68, 255)
  - **defaultPalette.GetControlPoints(9).position**: 0.31
  - **defaultPalette.GetControlPoints(10).colors**: (99, 255, 99, 255)
  - **defaultPalette.GetControlPoints(10).position**: 0.345
  - **defaultPalette.GetControlPoints(11).colors**: (99, 99, 255, 255)
  - **defaultPalette.GetControlPoints(11).position**: 0.379
  - **defaultPalette.GetControlPoints(12).colors**: (40, 165, 165, 255)
  - **defaultPalette.GetControlPoints(12).position**: 0.414
  - **defaultPalette.GetControlPoints(13).colors**: (255, 99, 255, 255)
  - **defaultPalette.GetControlPoints(13).position**: 0.448
  - **defaultPalette.GetControlPoints(14).colors**: (255, 255, 99, 255)
  - **defaultPalette.GetControlPoints(14).position**: 0.483
  - **defaultPalette.GetControlPoints(15).colors**: (255, 170, 99, 255)
  - **defaultPalette.GetControlPoints(15).position**: 0.517
  - **defaultPalette.GetControlPoints(16).colors**: (170, 79, 255, 255)
  - **defaultPalette.GetControlPoints(16).position**: 0.552
  - **defaultPalette.GetControlPoints(17).colors**: (150, 0, 0, 255)
  - **defaultPalette.GetControlPoints(17).position**: 0.586
  - **defaultPalette.GetControlPoints(18).colors**: (0, 150, 0, 255)
  - **defaultPalette.GetControlPoints(18).position**: 0.621
  - **defaultPalette.GetControlPoints(19).colors**: (0, 0, 150, 255)
  - **defaultPalette.GetControlPoints(19).position**: 0.655
  - **defaultPalette.GetControlPoints(20).colors**: (0, 109, 109, 255)
  - **defaultPalette.GetControlPoints(20).position**: 0.69
  - **defaultPalette.GetControlPoints(21).colors**: (150, 0, 150, 255)
  - **defaultPalette.GetControlPoints(21).position**: 0.724
  - **defaultPalette.GetControlPoints(22).colors**: (150, 150, 0, 255)
  - **defaultPalette.GetControlPoints(22).position**: 0.759
  - **defaultPalette.GetControlPoints(23).colors**: (150, 84, 0, 255)
  - **defaultPalette.GetControlPoints(23).position**: 0.793
  - **defaultPalette.GetControlPoints(24).colors**: (160, 0, 79, 255)
  - **defaultPalette.GetControlPoints(24).position**: 0.828
  - **defaultPalette.GetControlPoints(25).colors**: (255, 104, 28, 255)
  - **defaultPalette.GetControlPoints(25).position**: 0.862
  - **defaultPalette.GetControlPoints(26).colors**: (0, 170, 81, 255)
  - **defaultPalette.GetControlPoints(26).position**: 0.897
  - **defaultPalette.GetControlPoints(27).colors**: (68, 255, 124, 255)
  - **defaultPalette.GetControlPoints(27).position**: 0.931
  - **defaultPalette.GetControlPoints(28).colors**: (0, 130, 255, 255)
  - **defaultPalette.GetControlPoints(28).position**: 0.966
  - **defaultPalette.GetControlPoints(29).colors**: (130, 0, 255, 255)
  - **defaultPalette.GetControlPoints(29).position**: 1
  - **defaultPalette.smoothing**: *None*, Linear, CubicSpline
  - **defaultPalette.equalSpacingFlag**: 1
  - **defaultPalette.discreteFlag**: 1
  - **defaultPalette.categoryName**: "Standard"
  - **changedColors**: ()
  - **colorType**: *ColorByMultipleColors*, ColorBySingleColor
  - **singleColor**: *(255, 0, 0, 255)*,
    - SetMultiColor(0, (255, 0, 0, 255))
    - SetMultiColor(1, (0, 255, 0, 255))
    - SetMultiColor(2, (0, 0, 255, 255))
    - SetMultiColor(3, (0, 255, 255, 255))
    - SetMultiColor(4, (255, 0, 255, 255))
    - SetMultiColor(5, (255, 255, 0, 255))
    - SetMultiColor(6, (255, 135, 0, 255))
    - SetMultiColor(7, (255, 0, 135, 255))
    - SetMultiColor(8, (168, 168, 168, 255))
    - SetMultiColor(9, (255, 68, 68, 255))
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **yAxisTitleFormat**: "%g"
  - **useYAxisTickSpacing**: 0
  - **yAxisTickSpacing**: 1
  - **displayMarkers**: 1
  - **markerScale**: 1
  - **markerLineWidth**: 0
  - **markerVariable**: "default"
  - **displayIds**: 0
  - **idVariable**: "default"
  - **legendFlag**: *1*


.. _MultiresControlAttributes:

MultiresControlAttributes()
------------------------------------------


  - **resolution**: 0
  - **maxResolution**: 1
  - **info**: *""*


.. _OnionPeelAttributes:

OnionPeelAttributes()
------------------------------


  - **adjacencyType**: *Node*, Face
  - **useGlobalId**: 0
  - **categoryName**: "Whole"
  - **subsetName**: "Whole"
  - **index**: (1)
  - **logical**: 0
  - **requestedLayer**: 0
  - **seedType**: *SeedCell*, SeedNode
  - **honorOriginalMesh**: *1*


.. _ParallelCoordinatesAttributes:

ParallelCoordinatesAttributes()
--------------------------------------------------


  - **scalarAxisNames**: ()
  - **visualAxisNames**: ()
  - **extentMinima**: ()
  - **extentMaxima**: ()
  - **drawLines**: 1
  - **linesColor**: (128, 0, 0, 255)
  - **drawContext**: 1
  - **contextGamma**: 2
  - **contextNumPartitions**: 128
  - **contextColor**: (0, 220, 0, 255)
  - **drawLinesOnlyIfExtentsOn**: 1
  - **unifyAxisExtents**: 0
  - **linesNumPartitions**: 512
  - **focusGamma**: 4
  - **drawFocusAs**: *BinsOfConstantColor*, IndividualLines, BinsColoredByPopulation


.. _PersistentParticlesAttributes:

PersistentParticlesAttributes()
--------------------------------------------------


  - **startIndex**: 0
  - **stopIndex**: 1
  - **stride**: 1
  - **startPathType**: *Absolute*, Relative
  - **stopPathType**: *Absolute*, Relative
  - **traceVariableX**: "default"
  - **traceVariableY**: "default"
  - **traceVariableZ**: "default"
  - **connectParticles**: 0
  - **showPoints**: 0
  - **indexVariable**: *"default"*


.. _PoincareAttributes:

PoincareAttributes()
----------------------------


  - **opacityType**: *Explicit*, ColorTable
  - **opacity**: 1
  - **minPunctures**: 50
  - **maxPunctures**: 500
  - **puncturePlotType**: *Single*, Double
  - **maxSteps**: 1000
  - **terminateByTime**: 0
  - **termTime**: 10
  - **puncturePeriodTolerance**: 0.01
  - **puncturePlane**: *Poloidal*, Toroidal, Arbitrary
  - **sourceType**: *SpecifiedPoint*, PointList, SpecifiedLine
  - **pointSource**: (0, 0, 0)
  - **pointList**: (0, 0, 0, 1, 0, 0, 0, 1, 0)
  - **lineStart**: (0, 0, 0)
  - **lineEnd**: (1, 0, 0)
  - **pointDensity**: 1
  - **fieldType**: *Default*, FlashField, M3DC12DField, M3DC13DField, Nek5000Field, NektarPPField, NIMRODField
  - **forceNodeCenteredData**: 0
  - **fieldConstant**: 1
  - **velocitySource**: (0, 0, 0)
  - **integrationType**: *AdamsBashforth*, Euler, Leapfrog, DormandPrince, RK4, M3DC12DIntegrator
  - **coordinateSystem**: *Cartesian*, Cylindrical
  - **maxStepLength**: 0.1
  - **limitMaximumTimestep**: 0
  - **maxTimeStep**: 0.1
  - **relTol**: 0.0001
  - **absTolSizeType**: *FractionOfBBox*, Absolute
  - **absTolAbsolute**: 1e-05
  - **absTolBBox**: 1e-06
  - **analysis**: *Normal*, None
  - **maximumToroidalWinding**: 0
  - **overrideToroidalWinding**: 0
  - **overridePoloidalWinding**: 0
  - **windingPairConfidence**: 0.9
  - **rationalSurfaceFactor**: 0.1
  - **adjustPlane**: -1
  - **overlaps**: *Remove*, Raw, Merge, Smooth
  - **meshType**: *Curves*, Surfaces
  - **numberPlanes**: 1
  - **singlePlane**: 0
  - **min**: 0
  - **max**: 0
  - **minFlag**: 0
  - **maxFlag**: 0
  - **colorType**: *ColorByColorTable*, ColorBySingleColor
  - **singleColor**: (0, 0, 0, 255)
  - **colorTableName**: "Default"
  - **dataValue**: *SafetyFactorQ*, Solid, SafetyFactorP, SafetyFactorQ_NotP, SafetyFactorP_NotQ, ToroidalWindings, PoloidalWindingsQ, PoloidalWindingsP, FieldlineOrder, PointOrder, PlaneOrder, WindingGroupOrder, WindingPointOrder, WindingPointOrderModulo
  - **showRationalSurfaces**: 0
  - **RationalSurfaceMaxIterations**: 2
  - **showOPoints**: 0
  - **OPointMaxIterations**: 2
  - **showXPoints**: 0
  - **XPointMaxIterations**: 2
  - **performOLineAnalysis**: 0
  - **OLineToroidalWinding**: 1
  - **OLineAxisFileName**: ""
  - **showChaotic**: 0
  - **showIslands**: 0
  - **SummaryFlag**: 1
  - **verboseFlag**: 0
  - **show1DPlots**: 0
  - **showLines**: 1
  - **showPoints**: 0
  - **parallelizationAlgorithmType**: *VisItSelects*, LoadOnDemand, ParallelStaticDomains, MasterSlave
  - **maxProcessCount**: 10
  - **maxDomainCacheSize**: 3
  - **workGroupSize**: 32
  - **pathlines**: 0
  - **pathlinesOverrideStartingTimeFlag**: 0
  - **pathlinesOverrideStartingTime**: 0
  - **pathlinesPeriod**: 0
  - **pathlinesCMFE**: *POS_CMFE*, CONN_CMFE
  - **issueTerminationWarnings**: 1
  - **issueStepsizeWarnings**: 1
  - **issueStiffnessWarnings**: 1
  - **issueCriticalPointsWarnings**: 1
  - **criticalPointThreshold**: *0.001*


.. _PrinterAttributes:

PrinterAttributes()
--------------------------


  - **printerName**: ""
  - **printProgram**: "lpr"
  - **documentName**: "untitled"
  - **creator**: ""
  - **numCopies**: 1
  - **portrait**: 1
  - **printColor**: 1
  - **outputToFile**: 0
  - **outputToFileName**: "untitled"
  - **pageSize**: *2*


.. _ProcessAttributes:

ProcessAttributes()
--------------------------


  - **pids**: ()
  - **ppids**: ()
  - **hosts**: ()
  - **isParallel**: 0
  - **memory**: *()*


.. _ProjectAttributes:

ProjectAttributes()
--------------------------


  - **projectionType**: *XYCartesian*, ZYCartesian, XZCartesian, XRCylindrical, YRCylindrical, ZRCylindrical
  - **vectorTransformMethod**: *AsDirection*, None, AsPoint, AsDisplacement


.. _PseudocolorAttributes:

PseudocolorAttributes()
----------------------------------


  - **scaling**: *Linear*, Log, Skew
  - **skewFactor**: 1
  - **limitsMode**: *OriginalData*, CurrentPlot
  - **minFlag**: 0
  - **min**: 0
  - **maxFlag**: 0
  - **max**: 1
  - **centering**: *Natural*, Nodal, Zonal
  - **colorTableName**: "hot"
  - **invertColorTable**: 0
  - **opacityType**: *FullyOpaque*, ColorTable, Constant, Ramp, VariableRange
  - **opacityVariable**: ""
  - **opacity**: 1
  - **opacityVarMin**: 0
  - **opacityVarMax**: 1
  - **opacityVarMinFlag**: 0
  - **opacityVarMaxFlag**: 0
  - **pointSize**: 0.05
  - **pointType**: *Point*, Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Sphere
  - **pointSizeVarEnabled**: 0
  - **pointSizeVar**: "default"
  - **pointSizePixels**: 2
  - **lineType**: *Line*, Tube, Ribbon
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **tubeDisplayDensity**: 10
  - **tubeRadiusSizeType**: *FractionOfBBox*, Absolute
  - **tubeRadiusAbsolute**: 0.125
  - **tubeRadiusBBox**: 0.005
  - **varyTubeRadius**: 0
  - **varyTubeRadiusVariable**: ""
  - **varyTubeRadiusFactor**: 10
  - **endPointType**: *None*, Tails, Heads, Both
  - **endPointStyle**: *Spheres*, Cones
  - **endPointRadiusSizeType**: *FractionOfBBox*, Absolute
  - **endPointRadiusAbsolute**: 1
  - **endPointRadiusBBox**: 0.005
  - **endPointRatio**: 2
  - **renderSurfaces**: 1
  - **renderWireframe**: 0
  - **renderPoints**: 0
  - **smoothingLevel**: 0
  - **legendFlag**: 1
  - **lightingFlag**: *1*


.. _RadialResampleAttributes:

RadialResampleAttributes()
----------------------------------------


  - **isFast**: 0
  - **minTheta**: 0
  - **maxTheta**: 90
  - **deltaTheta**: 5
  - **radius**: 0.5
  - **deltaRadius**: 0.05
  - **center**: (0.5, 0.5, 0.5)
  - **is3D**: 1
  - **minAzimuth**: 0
  - **maxAzimuth**: 180
  - **deltaAzimuth**: *5*


.. _ReflectAttributes:

ReflectAttributes()
--------------------------


  - **octant**: *PXPYPZ*, NXPYPZ, PXNYPZ, NXNYPZ, PXPYNZ, NXPYNZ, PXNYNZ, NXNYNZ
  - **useXBoundary**: 1
  - **specifiedX**: 0
  - **useYBoundary**: 1
  - **specifiedY**: 0
  - **useZBoundary**: 1
  - **specifiedZ**: 0
  - **reflections**: *(1, 0, 1, 0, 0, 0, 0, 0)*


.. _RenderingAttributes:

RenderingAttributes()
------------------------------


  - **antialiasing**: 0
  - **multiresolutionMode**: 0
  - **multiresolutionCellSize**: 0.002
  - **geometryRepresentation**: *Surfaces*, Wireframe, Points
  - **displayListMode**: *Auto*, Never, Always
  - **stereoRendering**: 0
  - **stereoType**: *CrystalEyes*, RedBlue, Interlaced, RedGreen
  - **notifyForEachRender**: 0
  - **scalableActivationMode**: *Auto*, Never, Always
  - **scalableAutoThreshold**: 2000000
  - **specularFlag**: 0
  - **specularCoeff**: 0.6
  - **specularPower**: 10
  - **specularColor**: (255, 255, 255, 255)
  - **doShadowing**: 0
  - **shadowStrength**: 0.5
  - **doDepthCueing**: 0
  - **depthCueingAutomatic**: 1
  - **startCuePoint**: (-10, 0, 0)
  - **endCuePoint**: (10, 0, 0)
  - **compressionActivationMode**: *Never*, Always, Auto
  - **colorTexturingFlag**: 1
  - **compactDomainsActivationMode**: *Never*, Always, Auto
  - **compactDomainsAutoThreshold**: *256*


.. _ReplicateAttributes:

ReplicateAttributes()
------------------------------


  - **useUnitCellVectors**: 0
  - **xVector**: (1, 0, 0)
  - **yVector**: (0, 1, 0)
  - **zVector**: (0, 0, 1)
  - **xReplications**: 1
  - **yReplications**: 1
  - **zReplications**: 1
  - **mergeResults**: 1
  - **replicateUnitCellAtoms**: 0
  - **shiftPeriodicAtomOrigin**: 0
  - **newPeriodicOrigin**: *(0, 0, 0)*


.. _ResampleAttributes:

ResampleAttributes()
----------------------------


  - **useExtents**: 1
  - **startX**: 0
  - **endX**: 1
  - **samplesX**: 10
  - **startY**: 0
  - **endY**: 1
  - **samplesY**: 10
  - **is3D**: 1
  - **startZ**: 0
  - **endZ**: 1
  - **samplesZ**: 10
  - **tieResolver**: *random*, largest, smallest
  - **tieResolverVariable**: "default"
  - **defaultValue**: 0
  - **distributedResample**: 1
  - **cellCenteredOutput**: *0*


.. _RevolveAttributes:

RevolveAttributes()
--------------------------


  - **meshType**: *Auto*, XY, RZ, ZR
  - **autoAxis**: 1
  - **axis**: (1, 0, 0)
  - **startAngle**: 0
  - **stopAngle**: 360
  - **steps**: *30*


.. _SPHResampleAttributes:

SPHResampleAttributes()
----------------------------------


  - **minX**: 0
  - **maxX**: 1
  - **xnum**: 10
  - **minY**: 0
  - **maxY**: 1
  - **ynum**: 10
  - **minZ**: 0
  - **maxZ**: 1
  - **znum**: 10
  - **tensorSupportVariable**: "H"
  - **weightVariable**: "mass"
  - **RK**: 1
  - **memScale**: *0*


.. _SaveWindowAttributes:

SaveWindowAttributes()
--------------------------------


  - **outputToCurrentDirectory**: 1
  - **outputDirectory**: "."
  - **fileName**: "visit"
  - **family**: 1
  - **format**: *PNG*, BMP, CURVE, JPEG, OBJ, POSTSCRIPT, POVRAY, PPM, RGB, STL, TIFF, ULTRA, VTK, PLY
  - **width**: 1024
  - **height**: 1024
  - **screenCapture**: 0
  - **saveTiled**: 0
  - **quality**: 80
  - **progressive**: 0
  - **binary**: 0
  - **stereo**: 0
  - **compression**: *PackBits*, None, Jpeg, Deflate
  - **forceMerge**: 0
  - **resConstraint**: *ScreenProportions*, NoConstraint, EqualWidthHeight
  - **advancedMultiWindowSave**: *0*


.. _ScatterAttributes:

ScatterAttributes()
--------------------------


  - **var1**: "default"
  - **var1Role**: *Coordinate0*, Coordinate1, Coordinate2, Color, None
  - **var1MinFlag**: 0
  - **var1MaxFlag**: 0
  - **var1Min**: 0
  - **var1Max**: 1
  - **var1Scaling**: *Linear*, Log, Skew
  - **var1SkewFactor**: 1
  - **var2Role**: *Coordinate1*, Coordinate0, Coordinate2, Color, None
  - **var2**: "default"
  - **var2MinFlag**: 0
  - **var2MaxFlag**: 0
  - **var2Min**: 0
  - **var2Max**: 1
  - **var2Scaling**: *Linear*, Log, Skew
  - **var2SkewFactor**: 1
  - **var3Role**: *None*, Coordinate0, Coordinate1, Coordinate2, Color
  - **var3**: "default"
  - **var3MinFlag**: 0
  - **var3MaxFlag**: 0
  - **var3Min**: 0
  - **var3Max**: 1
  - **var3Scaling**: *Linear*, Log, Skew
  - **var3SkewFactor**: 1
  - **var4Role**: *None*, Coordinate0, Coordinate1, Coordinate2, Color
  - **var4**: "default"
  - **var4MinFlag**: 0
  - **var4MaxFlag**: 0
  - **var4Min**: 0
  - **var4Max**: 1
  - **var4Scaling**: *Linear*, Log, Skew
  - **var4SkewFactor**: 1
  - **pointSize**: 0.05
  - **pointSizePixels**: 1
  - **pointType**: *Point*, Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Sphere
  - **scaleCube**: 1
  - **colorType**: *ColorByForegroundColor*, ColorBySingleColor, ColorByColorTable
  - **singleColor**: (255, 0, 0, 255)
  - **colorTableName**: "Default"
  - **invertColorTable**: 0
  - **legendFlag**: *1*


.. _SliceAttributes:

SliceAttributes()
----------------------


  - **originType**: *Intercept*, Point, Percent, Zone, Node
  - **originPoint**: (0, 0, 0)
  - **originIntercept**: 0
  - **originPercent**: 0
  - **originZone**: 0
  - **originNode**: 0
  - **normal**: (0, -1, 0)
  - **axisType**: *YAxis*, XAxis, ZAxis, Arbitrary, ThetaPhi
  - **upAxis**: (0, 0, 1)
  - **project2d**: 1
  - **interactive**: 1
  - **flip**: 0
  - **originZoneDomain**: 0
  - **originNodeDomain**: 0
  - **meshName**: "default"
  - **theta**: 0
  - **phi**: *0*


.. _SmoothOperatorAttributes:

SmoothOperatorAttributes()
----------------------------------------


  - **numIterations**: 20
  - **relaxationFactor**: 0.01
  - **convergence**: 0
  - **maintainFeatures**: 1
  - **featureAngle**: 45
  - **edgeAngle**: 15
  - **smoothBoundaries**: *0*


.. _SphereSliceAttributes:

SphereSliceAttributes()
----------------------------------


  - **origin**: (0, 0, 0)
  - **radius**: *1*


.. _SpreadsheetAttributes:

SpreadsheetAttributes()
----------------------------------


  - **subsetName**: "Whole"
  - **formatString**: "%1.6f"
  - **useColorTable**: 0
  - **colorTableName**: "Default"
  - **showTracerPlane**: 1
  - **tracerColor**: (255, 0, 0, 150)
  - **normal**: *Z*, X, Y
  - **sliceIndex**: 0
  - **spreadsheetFont**: "Courier,12,-1,5,50,0,0,0,0,0"
  - **showPatchOutline**: 1
  - **showCurrentCellOutline**: 0
  - **currentPickType**: 0
  - **currentPickLetter**: ""
  - **pastPickLetters**: *()*


.. _StaggerAttributes:

StaggerAttributes()
--------------------------


  - **offsetX**: 0
  - **offsetY**: 0
  - **offsetZ**: *0*


.. _StatisticalTrendsAttributes:

StatisticalTrendsAttributes()
----------------------------------------------


  - **startIndex**: 0
  - **stopIndex**: 1
  - **stride**: 1
  - **startTrendType**: *Absolute*, Relative
  - **stopTrendType**: *Absolute*, Relative
  - **statisticType**: *Mean*, Sum, Variance, StandardDeviation, Slope, Residuals
  - **trendAxis**: *Step*, Time, Cycle
  - **variableSource**: *Default*, OperatorExpression


.. _StreamlineAttributes:

StreamlineAttributes()
--------------------------------


  - **sourceType**: *SpecifiedPoint*, SpecifiedPointList, SpecifiedLine, SpecifiedCircle, SpecifiedPlane, SpecifiedSphere, SpecifiedBox, Selection
  - **pointSource**: (0, 0, 0)
  - **lineStart**: (0, 0, 0)
  - **lineEnd**: (1, 0, 0)
  - **planeOrigin**: (0, 0, 0)
  - **planeNormal**: (0, 0, 1)
  - **planeUpAxis**: (0, 1, 0)
  - **radius**: 1
  - **sphereOrigin**: (0, 0, 0)
  - **boxExtents**: (0, 1, 0, 1, 0, 1)
  - **useWholeBox**: 1
  - **pointList**: (0, 0, 0, 1, 0, 0, 0, 1, 0)
  - **sampleDensity0**: 2
  - **sampleDensity1**: 2
  - **sampleDensity2**: 2
  - **coloringMethod**: *ColorByTime*, Solid, ColorBySpeed, ColorByVorticity, ColorByLength, ColorBySeedPointID, ColorByVariable, ColorByCorrelationDistance, ColorByNumberDomainsVisited
  - **colorTableName**: "Default"
  - **singleColor**: (0, 0, 0, 255)
  - **legendFlag**: 1
  - **lightingFlag**: 1
  - **integrationDirection**: *Forward*, Backward, Both
  - **maxSteps**: 1000
  - **terminateByDistance**: 0
  - **termDistance**: 10
  - **terminateByTime**: 0
  - **termTime**: 10
  - **maxStepLength**: 0.1
  - **limitMaximumTimestep**: 0
  - **maxTimeStep**: 0.1
  - **relTol**: 0.0001
  - **absTolSizeType**: *FractionOfBBox*, Absolute
  - **absTolAbsolute**: 1e-06
  - **absTolBBox**: 1e-06
  - **fieldType**: *Default*, FlashField, M3DC12DField, M3DC13DField, Nek5000Field, NIMRODField
  - **fieldConstant**: 1
  - **velocitySource**: (0, 0, 0)
  - **integrationType**: *DormandPrince*, Euler, Leapfrog, AdamsBashforth, RK4, M3DC12DIntegrator
  - **parallelizationAlgorithmType**: *VisItSelects*, LoadOnDemand, ParallelStaticDomains, MasterSlave
  - **maxProcessCount**: 10
  - **maxDomainCacheSize**: 3
  - **workGroupSize**: 32
  - **pathlines**: 0
  - **pathlinesOverrideStartingTimeFlag**: 0
  - **pathlinesOverrideStartingTime**: 0
  - **pathlinesPeriod**: 0
  - **pathlinesCMFE**: *POS_CMFE*, CONN_CMFE
  - **coordinateSystem**: *AsIs*, CylindricalToCartesian, CartesianToCylindrical
  - **phiScalingFlag**: 0
  - **phiScaling**: 1
  - **coloringVariable**: ""
  - **legendMinFlag**: 0
  - **legendMaxFlag**: 0
  - **legendMin**: 0
  - **legendMax**: 1
  - **displayBegin**: 0
  - **displayEnd**: 1
  - **displayBeginFlag**: 0
  - **displayEndFlag**: 0
  - **referenceTypeForDisplay**: *Distance*, Time, Step
  - **displayMethod**: *Lines*, Tubes, Ribbons
  - **tubeSizeType**: *FractionOfBBox*, Absolute
  - **tubeRadiusAbsolute**: 0.125
  - **tubeRadiusBBox**: 0.005
  - **ribbonWidthSizeType**: *FractionOfBBox*, Absolute
  - **ribbonWidthAbsolute**: 0.125
  - **ribbonWidthBBox**: 0.01
  - **lineWidth**: 2
  - **showSeeds**: 1
  - **seedRadiusSizeType**: *FractionOfBBox*, Absolute
  - **seedRadiusAbsolute**: 1
  - **seedRadiusBBox**: 0.015
  - **showHeads**: 0
  - **headDisplayType**: *Sphere*, Cone
  - **headRadiusSizeType**: *FractionOfBBox*, Absolute
  - **headRadiusAbsolute**: 0.25
  - **headRadiusBBox**: 0.02
  - **headHeightRatio**: 2
  - **opacityType**: *FullyOpaque*, Constant, Ramp, VariableRange
  - **opacityVariable**: ""
  - **opacity**: 1
  - **opacityVarMin**: 0
  - **opacityVarMax**: 1
  - **opacityVarMinFlag**: 0
  - **opacityVarMaxFlag**: 0
  - **tubeDisplayDensity**: 10
  - **geomDisplayQuality**: *Medium*, Low, High, Super
  - **sampleDistance0**: 10
  - **sampleDistance1**: 10
  - **sampleDistance2**: 10
  - **fillInterior**: 1
  - **randomSamples**: 0
  - **randomSeed**: 0
  - **numberOfRandomSamples**: 1
  - **forceNodeCenteredData**: 0
  - **issueTerminationWarnings**: 1
  - **issueStiffnessWarnings**: 1
  - **issueCriticalPointsWarnings**: 1
  - **criticalPointThreshold**: 0.001
  - **varyTubeRadius**: *None*, Scalar
  - **varyTubeRadiusFactor**: 10
  - **varyTubeRadiusVariable**: ""
  - **correlationDistanceAngTol**: 5
  - **correlationDistanceMinDistAbsolute**: 1
  - **correlationDistanceMinDistBBox**: 0.005
  - **correlationDistanceMinDistType**: *FractionOfBBox*, Absolute
  - **selection**: *""*


.. _SubsetAttributes:

SubsetAttributes()
------------------------


  - **colorType**: *ColorByMultipleColors*, ColorBySingleColor, ColorByColorTable
  - **colorTableName**: "Default"
  - **invertColorTable**: 0
  - **filledFlag**: 1
  - **legendFlag**: 1
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **singleColor**: (0, 0, 0, 255)
  - **subsetNames**: ()
  - **subsetType**: *Unknown*, Domain, Group, Material, EnumScalar, Mesh
  - **opacity**: 1
  - **wireframe**: 0
  - **drawInternal**: 0
  - **smoothingLevel**: 0
  - **pointSize**: 0.05
  - **pointType**: *Point*, Box, Axis, Icosahedron, Octahedron, Tetrahedron, SphereGeometry, Sphere
  - **pointSizeVarEnabled**: 0
  - **pointSizeVar**: "default"
  - **pointSizePixels**: *2*


.. _SurfaceNormalAttributes:

SurfaceNormalAttributes()
--------------------------------------


  - **centering**: *Point*, Cell


.. _TensorAttributes:

TensorAttributes()
------------------------


  - **useStride**: 0
  - **stride**: 1
  - **nTensors**: 400
  - **scale**: 0.25
  - **scaleByMagnitude**: 1
  - **autoScale**: 1
  - **colorByEigenvalues**: 1
  - **useLegend**: 1
  - **tensorColor**: (0, 0, 0, 255)
  - **colorTableName**: "Default"
  - **invertColorTable**: *0*


.. _ThreeSliceAttributes:

ThreeSliceAttributes()
--------------------------------


  - **x**: 0
  - **y**: 0
  - **z**: 0
  - **interactive**: *1*


.. _ThresholdAttributes:

ThresholdAttributes()
------------------------------


  - **outputMeshType**: 0
  - **listedVarNames**: ("default")
  - **zonePortions**: ()
  - **lowerBounds**: ()
  - **upperBounds**: ()
  - **defaultVarName**: "default"
  - **defaultVarIsScalar**: *0*


.. _TransformAttributes:

TransformAttributes()
------------------------------


  - **doRotate**: 0
  - **rotateOrigin**: (0, 0, 0)
  - **rotateAxis**: (0, 0, 1)
  - **rotateAmount**: 0
  - **rotateType**: *Deg*, Rad
  - **doScale**: 0
  - **scaleOrigin**: (0, 0, 0)
  - **scaleX**: 1
  - **scaleY**: 1
  - **scaleZ**: 1
  - **doTranslate**: 0
  - **translateX**: 0
  - **translateY**: 0
  - **translateZ**: 0
  - **transformType**: *Similarity*, Coordinate, Linear
  - **inputCoordSys**: *Cartesian*, Cylindrical, Spherical
  - **outputCoordSys**: *Spherical*, Cartesian, Cylindrical
  - **continuousPhi**: 0
  - **m00**: 1
  - **m01**: 0
  - **m02**: 0
  - **m03**: 0
  - **m10**: 0
  - **m11**: 1
  - **m12**: 0
  - **m13**: 0
  - **m20**: 0
  - **m21**: 0
  - **m22**: 1
  - **m23**: 0
  - **m30**: 0
  - **m31**: 0
  - **m32**: 0
  - **m33**: 1
  - **invertLinearTransform**: 0
  - **vectorTransformMethod**: *AsDirection*, None, AsPoint, AsDisplacement
  - **transformVectors**: *1*


.. _TriangulateRegularPointsAttributes:

TriangulateRegularPointsAttributes()
------------------------------------------------------------


  - **useXGridSpacing**: 0
  - **xGridSpacing**: 1
  - **useYGridSpacing**: 0
  - **yGridSpacing**: *1*


.. _TruecolorAttributes:

TruecolorAttributes()
------------------------------


  - **opacity**: 1
  - **lightingFlag**: *1*


.. _TubeAttributes:

TubeAttributes()
--------------------


  - **scaleByVarFlag**: 0
  - **tubeRadiusType**: *FractionOfBBox*, Absolute
  - **radiusFractionBBox**: 0.01
  - **radiusAbsolute**: 1
  - **scaleVariable**: "default"
  - **fineness**: 5
  - **capping**: *0*


.. _VectorAttributes:

VectorAttributes()
------------------------


  - **glyphLocation**: *AdaptsToMeshResolution*, UniformInSpace
  - **useStride**: 0
  - **stride**: 1
  - **nVectors**: 400
  - **lineStyle**: *SOLID*, DASH, DOT, DOTDASH
  - **lineWidth**: 0
  - **scale**: 0.25
  - **scaleByMagnitude**: 1
  - **autoScale**: 1
  - **headSize**: 0.25
  - **headOn**: 1
  - **colorByMag**: 1
  - **useLegend**: 1
  - **vectorColor**: (0, 0, 0, 255)
  - **colorTableName**: "Default"
  - **invertColorTable**: 0
  - **vectorOrigin**: *Tail*, Head, Middle
  - **minFlag**: 0
  - **maxFlag**: 0
  - **limitsMode**: *OriginalData*, CurrentPlot
  - **min**: 0
  - **max**: 1
  - **lineStem**: *Line*, Cylinder
  - **geometryQuality**: *Fast*, High
  - **stemWidth**: 0.08
  - **origOnly**: 1
  - **glyphType**: *Arrow*, Ellipsoid


.. _ViewAttributes:

ViewAttributes()
--------------------


  - **viewNormal**: (0, 0, 1)
  - **focus**: (0, 0, 0)
  - **viewUp**: (0, 1, 0)
  - **viewAngle**: 30
  - **setScale**: 0
  - **parallelScale**: 1
  - **nearPlane**: 0.001
  - **farPlane**: 100
  - **imagePan**: (0, 0)
  - **imageZoom**: 1
  - **perspective**: 1
  - **windowCoords**: (0, 0, 1, 1)
  - **viewportCoords**: (0.1, 0.1, 0.9, 0.9)
  - **eyeAngle**: *2*


.. _View2DAttributes:

View2DAttributes()
------------------------


  - **windowCoords**: (0, 1, 0, 1)
  - **viewportCoords**: (0.2, 0.95, 0.15, 0.95)
  - **fullFrameActivationMode**: *Auto*, On, Off
  - **fullFrameAutoThreshold**: 100
  - **xScale**: *LINEAR*, LOG
  - **yScale**: *LINEAR*, LOG
  - **windowValid**: *0*


.. _View3DAttributes:

View3DAttributes()
------------------------


  - **viewNormal**: (0, 0, 1)
  - **focus**: (0, 0, 0)
  - **viewUp**: (0, 1, 0)
  - **viewAngle**: 30
  - **parallelScale**: 0.5
  - **nearPlane**: -0.5
  - **farPlane**: 0.5
  - **imagePan**: (0, 0)
  - **imageZoom**: 1
  - **perspective**: 1
  - **eyeAngle**: 2
  - **centerOfRotationSet**: 0
  - **centerOfRotation**: (0, 0, 0)
  - **axis3DScaleFlag**: 0
  - **axis3DScales**: (1, 1, 1)
  - **shear**: (0, 0, 1)
  - **windowValid**: *0*


.. _ViewAxisArrayAttributes:

ViewAxisArrayAttributes()
--------------------------------------


  - **domainCoords**: (0, 1)
  - **rangeCoords**: (0, 1)
  - **viewportCoords**: *(0.15, 0.9, 0.1, 0.85)*


.. _ViewCurveAttributes:

ViewCurveAttributes()
------------------------------


  - **domainCoords**: (0, 1)
  - **rangeCoords**: (0, 1)
  - **viewportCoords**: (0.2, 0.95, 0.15, 0.95)
  - **domainScale**: *LINEAR*, LOG
  - **rangeScale**: *LINEAR*, LOG


.. _VolumeAttributes:

VolumeAttributes()
------------------------


  - **legendFlag**: 1
  - **lightingFlag**: 1
  - **colorControlPoints.GetControlPoints(0).colors**: (0, 0, 255, 255)
  - **colorControlPoints.GetControlPoints(0).position**: 0
  - **colorControlPoints.GetControlPoints(1).colors**: (0, 255, 255, 255)
  - **colorControlPoints.GetControlPoints(1).position**: 0.25
  - **colorControlPoints.GetControlPoints(2).colors**: (0, 255, 0, 255)
  - **colorControlPoints.GetControlPoints(2).position**: 0.5
  - **colorControlPoints.GetControlPoints(3).colors**: (255, 255, 0, 255)
  - **colorControlPoints.GetControlPoints(3).position**: 0.75
  - **colorControlPoints.GetControlPoints(4).colors**: (255, 0, 0, 255)
  - **colorControlPoints.GetControlPoints(4).position**: 1
  - **colorControlPoints.smoothing**: *Linear*, None, CubicSpline
  - **colorControlPoints.equalSpacingFlag**: 0
  - **colorControlPoints.discreteFlag**: 0
  - **colorControlPoints.categoryName**: ""
  - **opacityAttenuation**: 1
  - **opacityMode**: *FreeformMode*, GaussianMode, ColorTableMode,  controlPoints does not,  contain any,  GaussianControlPoint,  objects.
  - **resampleFlag**: 1
  - **resampleTarget**: 50000
  - **opacityVariable**: "default"
  - **compactVariable**: "default"
  - **freeformOpacity**: * (0, 1, 2, 3, 4, 5, 6, 7, 8,*,  9, 10, 11, 12, 13, 14, 15,,  16, 17, 18, 19, 20, 21,,  22, 23, 24, 25, 26, 27,,  28, 29, 30, 31, 32, 33,,  34, 35, 36, 37, 38, 39,,  40, 41, 42, 43, 44, 45,,  46, 47, 48, 49, 50, 51,,  52, 53, 54, 55, 56, 57,,  58, 59, 60, 61, 62, 63,,  64, 65, 66, 67, 68, 69,,  70, 71, 72, 73, 74, 75,,  76, 77, 78, 79, 80, 81,,  82, 83, 84, 85, 86, 87,,  88, 89, 90, 91, 92, 93,,  94, 95, 96, 97, 98, 99,,  100, 101, 102, 103, 104,,  105, 106, 107, 108, 109,,  110, 111, 112, 113, 114,,  115, 116, 117, 118, 119,,  120, 121, 122, 123, 124,,  125, 126, 127, 128, 129,,  130, 131, 132, 133, 134,,  135, 136, 137, 138, 139,,  140, 141, 142, 143, 144,,  145, 146, 147, 148, 149,,  150, 151, 152, 153, 154,,  155, 156, 157, 158, 159,,  160, 161, 162, 163, 164,,  165, 166, 167, 168, 169,,  170, 171, 172, 173, 174,,  175, 176, 177, 178, 179,,  180, 181, 182, 183, 184,,  185, 186, 187, 188, 189,,  190, 191, 192, 193, 194,,  195, 196, 197, 198, 199,,  200, 201, 202, 203, 204,,  205, 206, 207, 208, 209,,  210, 211, 212, 213, 214,,  215, 216, 217, 218, 219,,  220, 221, 222, 223, 224,,  225, 226, 227, 228, 229,,  230, 231, 232, 233, 234,,  235, 236, 237, 238, 239,,  240, 241, 242, 243, 244,,  245, 246, 247, 248, 249,,  250, 251, 252, 253, 254,,  255)
  - **useColorVarMin**: 0
  - **colorVarMin**: 0
  - **useColorVarMax**: 0
  - **colorVarMax**: 0
  - **useOpacityVarMin**: 0
  - **opacityVarMin**: 0
  - **useOpacityVarMax**: 0
  - **opacityVarMax**: 0
  - **smoothData**: 0
  - **samplesPerRay**: 500
  - **rendererType**: *Splatting*, Texture3D, RayCasting, RayCastingIntegration, SLIVR, RayCastingSLIVR, Tuvok
  - **gradientType**: *SobelOperator*, CenteredDifferences
  - **num3DSlices**: 200
  - **scaling**: *Linear*, Log, Skew
  - **skewFactor**: 1
  - **limitsMode**: *OriginalData*, CurrentPlot
  - **sampling**: *Rasterization*, KernelBased, Trilinear
  - **rendererSamples**: *3*,  transferFunction2DWidgets,  does not contain any,  TransferFunctionWidget,  objects.
  - **transferFunctionDim**: 1
  - **lowGradientLightingReduction**: *Lower*, Off, Lowest, Low, Medium, High, Higher, Highest
  - **lowGradientLightingClampFlag**: 0
  - **lowGradientLightingClampValue**: 1
  - **materialProperties**: *(0.4, 0.75, 0, 15)*

