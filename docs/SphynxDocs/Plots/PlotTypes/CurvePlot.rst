Curve Plot
~~~~~~~~~~

The Curve plot, shown in
, displays a simple group of X-Y pair data such as that output by 1D simulations or data produced by Lineouts of 2D or 3D datasets. Curve plots are useful for visualizations where it is useful to plot 1D quantities that evolve over time.

Setting line style and line width
"""""""""""""""""""""""""""""""""

Several Curve plots are often drawn in the same visualization window so it is necessary that Curve plots can be distinguished from each other. Fortunately, VisIt provides controls to change the line style and line width so that Curve plots can be told apart. Line style is a pattern used to draw the line and it is solid by default but it can also be dashed, dotted, or dash-dotted. You choose a new line style by making a selection from the
**Line Style menu**
. The line width, which determines the boldness of the curve, is set by making a selection from the
**Line Width menu**
.

Setting curve color
"""""""""""""""""""

The Curve plot's color can be changed by clicking on the
**Color button**
and making a selection from the
**Popup color menu**
.

Showing curve labels
""""""""""""""""""""

In addition to line style and line width, Curve plots have a label that can be displayed to help distinguish a Curve plot from other Curve plots. Curve plot labels are on by default, but if you want to turn the label off, you can uncheck the
**Labels**
check box.

Drawing points on the Curve plot
""""""""""""""""""""""""""""""""

The Curve plot is composed of a set of (X,Y) pairs through which line segments are drawn to form a curve. To make VisIt draw a point glyph at the location of each (X,Y) point, click the
**Points**
check box. You can control the size of the points by typing a new point size into the
**Point size**
text field.
