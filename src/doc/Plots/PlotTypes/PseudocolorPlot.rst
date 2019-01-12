.. _pseudocolor_plot_head:

Pseudocolor plot
~~~~~~~~~~~~~~~~

The Pseudocolor plot, shown in :numref:`Figure %s <pseudocolorplot2>`, maps
a scalar variable's data values to colors and uses the colors to "paint" values
onto the variable's computational mesh. The result is a clear picture of the
database geometry painted with variable values that have been mapped to
colors. You might try this plot first when examining a scientific database
for the first time since it reveals so much information about the plotted
variable.

.. _pseudocolorplot2:

.. figure:: ../images/pseudocolorplot.png

   Pseudocolor plot


Variable centering
""""""""""""""""""

Variables in a database can be associated with a mesh in various ways. Databases
supported by VisIt allow variables to be associated with a mesh's zones (cells)
or its nodes. When a variable is associated with a mesh's zones, the variable
field consists of one value for each zone and is said to be
*:term:`zone-centered`* . When a variable is associated with a mesh's nodes,
there are values for each vertex making up the zone and the variable is said to
be *:term:`node-centered`*.

.. _pseudocolorwindow2:

.. figure:: ../images/pseudocolorwindow.png

   Pseudocolor plot attributes window

VisIt's **Pseudocolor plot attributes window**
(shown in :numref:`Figure %s <pseudocolorwindow2>`)
allows you to specify how variables should be centered. There are three settings
for variable centering: **Natural**, **Nodal**, and **Zonal**. **Natural**
variable centering displays the data according to the way the variable was
centered on the mesh. This means that node-centered data will be displayed at
the nodes with colors being linearly interpolated between the nodes, and
zone-centered data will be displayed as zonal values, giving a slightly "blocky"
look to the picture. If **Nodal** centering is selected, all data is displayed
at the nodes regardless of the variable's natural centering. This will produce
a smoother picture, but for variables which are actually zone-centered, you will
lose some data (local minima and maxima). If you select **Zonal** centering, all
data is displayed as if they were zone-centered. This produces a blockier
picture and, again, it blurs minima/maxima for node-centered data.

Limits
""""""

Setting limits for the plot imposes artificial minima and maxima on the plotted
variable. This effectively restricts the range of data used to color the
Pseudocolor plot. You might set limits when you are interested in only a small
range of the data or when data limits need to be maintained for multiple time
steps, as when playing an animation. In fact, we recommend setting the limits
when producing an animation so the colors will correspond to the same values
instead of varying over time with the range of the plotted variable. Setting
limits often highlights a certain range in the data by assigning more colors to
that data range.

To set the limits for the Pseudocolor plot, you must first select the limit
mode. The limit mode determines whether the original data extents (data extents
before any portions of the plot are removed), are used or the current plot data
extents (data extents after any portions of the plot are removed), are used. To
select the limit mode, choose either **Use Original Data** or
**Use Current Plot** from the **Limits** menu.

The limits for the Pseudocolor plot consist of a minimum value and a maximum
value. You may set these limits, and turn them on and off, independently of one
another. That is, the use of one limit does not require the use of the other.
To set a limit, check the **Min** or **Max** check box next to the **Min** or
**Max** text field and type a new limit value into the **Min** or **Max** text
field.

Scaling the data
""""""""""""""""

The scale maps data values to color values. VisIt provides three scaling
options: **Linear**, **Log**, and **Skew**. **Linear**, which is the default,
uses a linear mapping of data values to color values. **Log** scaling is
used to map small ranges of data to larger ranges of color. **Skew** scaling
goes one step further by using an exponential function based on a skew factor
to adjust the mapping of data to colors. The function used in skew scaling is
**(s^d-1)/(s-1)** where **s** is a skew factor greater than zero and
**d** is a data value that has been mapped to a range from zero to one. The
mapping of data to colors is changed by changing the skew factor. A skew
factor of one is equivalent to linear scaling but values either larger or
smaller than one produce curves that map either the high or low end of the
data to a larger color range. To change the skew factor, choose **Skew**
scaling and type a new skew factor into the **Skew factor** text field.

Changing the color table
""""""""""""""""""""""""

.. _colortablebutton:

.. figure:: ../images/colortablebutton.png

   Color table button

The Pseudocolor plot can specify which VisIt color table is used for colors.
To change the color table, click on the **Color table** button, shown in
:numref:`Figure %s <colortablebutton>`, and select a new color table name from
the list of color tables. The list of color tables always represents the list
of available VisIt color tables. If you do not care which color table is used,
choose the Default option to use VisIt's active continuous color table. New
color tables can be defined using VisIt's **Color table window** which is
described later in this manual.

Lighting
""""""""

Lighting adds detail and depth to the Pseudocolor plot, two characteristics
that are important for animations. The **Lighting** check box in the lower part
of the **Pseudocolor plot attributes window** turns lighting on and off. Since
lighting is on by default, uncheck the **Lighting** check box to turn lighting
off.

Opacity
"""""""

You can make the Pseudocolor plot transparent by changing its opacity using the
**Opacity** slider. Moving the opacity slider to the left makes the plot more
transparent while moving the slider to the right makes the plot more opaque.
The default is fully opaque.

Changing the point size
"""""""""""""""""""""""

Some databases scalar variables defined on meshes of topological dimension zero.
When these point variables are 2D, VisIt draws them using small points that are
colored by the value of the variable. When a point variable is 3D, VisIt draws
it as a set of 3D cubes colored by the value of the variable. To control how
large the points appear, change the point size attribute by typing a new number
into the **Point size** text field. Larger point size values result in larger
points in the visualization window. The point size can also be scaled by a
scalar variable if you check the **Scale point size by variable** check box and
select a new scalar variable from the **Variable** button. The value ``default``
must be replaced with the name of another scalar variable if you want VisIt to
scale the points with a variable other than the one being plotted by the
Pseudocolor plot.

Changing the point type
"""""""""""""""""""""""

.. _pointtypes3:

.. figure:: ../images/pointtypes.png

   Color table button

The Pseudocolor plot can use five different point types for drawing point meshes
(see :numref:`Figure %s <pointtypes3>`). The Pseudocolor plot draws an object of the
desired point type, scaled by the point size, for each point in a point mesh.
Setting the point type has no effect if the plotted mesh is not a point mesh.
The different point types are as follows: Box, Axis, Icosahedron, Point, and
Sphere. To set the point type choose a new point type from the **Point Type**
combo box. When the point type is set to Box, the Pseudocolor plot draws a small
cube for each point in the point mesh. When the point type is set to Axis, the
Pseudocolor plot draws there small axis-aligned planes for each point in the
point mesh. When the point type is set to Icosahedron, the Pseudocolor plot
draws small icosahedra at each point in the point mesh. When the point type
is set to Point, the Pseudocolor plot uses flat quadrilateral points. When the
point type is set to Sphere, the Pseudocolor plot uses flat quadrilateral
points with an applied texture to make them look like spheres. In general,
setting the point type to Point will cause the Pseudocolor plot to have the
fastest rendering performance.

Geometry smoothing
""""""""""""""""""

Sometimes visualization operations such as material interface reconstruction can
alter mesh surfaces so they are pointy or distorted. The Pseudocolor plot
provides an optional Geometry smoothing option to smooth out the mesh surfaces
so they look better when the plot is visualized. Geometry smoothing is not done
by default, you must click the **Fast** or **High** radio buttons to enable it.
The **Fast** geometry smoothing setting smooths out the geometry a little while
the **High** setting works produces smoother surfaces.

Legend Behavior
"""""""""""""""

The legend for the Pseudocolor plot is a color bar annotated with tick marks and
numerical values. Below the color bar the minimum and maximum data values are
also displayed. Setting the limits for the plot changes *only* the color-bar portion
of the plot's legend. It *does not change* the *Min* and *Max* values printed
just below the color bar. Those values will always display the original data's 
minimum and maximum values, regardless of the limits set for the plot or the
effect of any operators applied to the plot.
