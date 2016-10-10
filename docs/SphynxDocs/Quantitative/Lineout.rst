Lineout
-------

One-dimensional curves, created using data from 2D or 3D plots, are popular for analyzing data
because they are simple to compare. VisIt's visualization windows can be put into a mode that allows you to draw lines, along which data are extracted, in the visualization window. The extracted data are turned into a Curve plot in another visualization window. If no other visualization window exists, VisIt creates one and adds the Curve plot to it. Curve plots are often more useful than 2D Pseudocolor plots because they allow the data along a line to be seen spatially as a 1D curve instead of relying on differences in color to convey information. Furthermore, the curve data can be exported to curve files that allow the data to be imported into other Lawrence Livermore National Laboratory curve analysis software such as
*Ultra*
.

Lineout mode
~~~~~~~~~~~~

You can put the visualization window into lineout mode by selecting
**Lineout**
from the
**Popup menu's Mode**
submenu. Note that lineout mode is only available with 2D plots in this version though you can create 3D lineouts using the Lineout query in the
**Query Window**
. After the visualization window is in lineout mode, you can draw reference lines in the window. Each reference line causes VisIt to extract data from the database along the prescribed path and draw the data as a Curve plot in another visualization window. Each reference line is drawn in a color that matches the initial color of the Curve plot so the reference lines, which have no labels, can be easily associated with their corresponding Curve plots. If lineout's dynamic mode is also set then changes such as a variable or time state change made to the plot that originated a Curve plot via a lineout operation are also reflected in the Curve plot. To clear the reference lines from the visualization window, select the
**Reference lines**
option Clear submenu in the
**Main Window's Window**
menu. An example of the visualization window with reference lines and Curve plots is shown in
.

Dynamic lineout
"""""""""""""""

Normally when you perform a lineout operation, the Curve plot that results from the lineout
operation is in no way connected to the plots in the window that originated the Curve plot. If you want to make variable or time state changes made to the originating plots also affect the Curve plots that were created via lineout, you should set the dynamic lineout flag for lineouts in the
**Lineout Options Window**
(see
). To open the
**Lineout Options Window**
(not the
**Lineout operator attributes window**
), select
**Lineout**
from the
**Controls**
menu in the
**Main Window**
. Click the
**Dynamic**
check box in the
**Lineout Options Window**
to make new Curve plots, created via lineout, be linked to their originating plots so they will be affected by variable or time state changes made to the originating plots.

The
**Lineout Options Window**
also contains some other options affecting dynamic lineouts. When you change the variable in the plot the originated the lineout, the lineout will change. When you change time states for the plot that originated the lineout, the lineout will change. However, you can instead make VisIt create a new Curve plot for the lineout each time the you change the originating plot's time state by changing the
**Time change behavior**
in the
**Lineout Options Window**
from updates curve to creates new curve. When VisIt creates a new curve each time you advance to a new time state, VisIt will put a new curve in the lineout destination window, resulting in many Curve plots (see
). By default, VisIt will make all of the related Curve plots be the same color. You can override this behavior by selecting creates new color instead of repeats color from the
**Lineout Options Window's New Curve**
combo box.

Curve plot
~~~~~~~~~~

Curve plots are created by drawing reference lines. The visualization window must be in lineout mode before references lines can be created. You can create a reference line by positioning the mouse over the first point of interest, clicking the left mouse button and then moving the mouse, while pressing the left mouse button, and releasing the mouse over the second endpoint. Releasing the mouse button creates a reference line along the path that was drawn with the mouse. When you draw a reference line, you cause a Curve plot of the data along the reference line to appear in another visualization window. If another visualization window is not available, VisIt opens a new one before creating the Curve plot. The Curve plot in the second window can be modified by setting the active window to the visualization window that contains the Curve plots.

Setting curve attributes
""""""""""""""""""""""""

Curve plots have a few options that can be set to alter their appearance. You can set the curve's line style, which can be solid, dotted, dashed, or dash-dotted, by making a selection from the
**Line Style**
menu in the
**Curve plot attributes window**
(
). If the Curve plot is difficult to see in the visualization window, try increasing the line thickness by making a selection from the
**Line Width**
menu. You can change the curve's color, by clicking on the
**Color**
button and selecting a new color from the
**Popup color **
menu. If you don't want the curve to have labels, you can turn them off by
unchecking the
**Labels**
check box. Finally, you can make Curve plots display a small point glyph at each data point along the curve if you check the
**Points**
check box and set a point size in the
**Point size**
text field.

Saving curves
"""""""""""""

Once a curve has been generated, it can be saved to a curve file. A curve file is an ASCII text file that contains the X-Y pairs that make up the curve and it is useful for exporting curve data to other curve analysis programs. To save a curve, make sure you first set the active window to the visualization window that contains the curve. Next, save the window using the
*curve*
file format. All of the curves in the visualization window are saved to the specified curve file.

Lineout Operator
~~~~~~~~~~~~~~~~

The Curve plot uses the Lineout operator to extract data from a database along a linear path. The Lineout operator is not generally available since curves are created only through reference lines and not the
**Plot menu**
. Still, once a curve has been created using the Lineout operator, certain attributes of the Lineout can be modified. Note that when you modify the Lineout operator attributes, it is best to turn off the
**Apply operator to all plots**
check box in the
**Main Window**
so that all curves do not get the same set of Lineout operator attributes. Some Lineout operator attributes can be set globally in the
**Global Lineout Window**
, which is accessed by clicking on Lineout option in the
**Main Window's Controls **
menu. The
**Global Lineout Window**
contains mainly the same options as the
** Lineout operator attributes**
window but it applies to all Lineout operators.

Setting lineout endpoints
"""""""""""""""""""""""""

You can modify the line endpoints by typing new coordinates into the
**Lineout operator attribute window's Point 1**
or
**Point 2**
text fields (
). Each endpoint is a 3D coordinate that is specified by three space-separated floating point numbers. If you are performing a Lineout operation on 2D data, you can set the value for the Z coordinate to zero.

Setting the number of lineout samples
"""""""""""""""""""""""""""""""""""""

The Lineout operator works by extracting sample points along a line. The sample points are then
used to create Curve plots. The Lineout operator's default sampling scheme is to sample data values at the intersections between the sampling line and the cell boundaries encountered along the way. This method gives rise to jagged Curve plots favored by many VisIt users. See
for a comparison between the sampling methods. If you instead want to smoothly sample the cells along the sampling line with some number of evenly spaced sample points, you can make the Lineout operator use evenly spaced sampling by clicking on the
**Override Global Lineout Settings**
check box in the
**Lineout operator attributes window**
. Once you've told the Lineout operator to override the global lineout settings click on the
**Use Sampling**
check box and enter a number of sample points. The number of sample points taken along the line determine the fidelity of the Curve plot. Generally, it is best to set the number of sample points such that each cell is sampled at least once. To set the number of sample points, type a new number into the
**Samples**
text field in the
**Lineout operator attributes window**
.

Interactive mode
""""""""""""""""

When the
**Interactive**
check box is checked, changes to the Lineout operator cause similar changes to the reference line that generated the curve. For example, changing the line endpoints causes the reference line to update.

Reference line labels
"""""""""""""""""""""

You can make the reference lines in the window that caused Curve plots to be generated to have labels by checking the Lineout operator's
**Refline Labels**
check box.
