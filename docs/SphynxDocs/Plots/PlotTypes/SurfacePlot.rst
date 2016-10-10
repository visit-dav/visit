Surface Plot
~~~~~~~~~~~~

The Surface plot (example shown in
) takes 2D scalar databases as input and adds a height component to the variable's mesh, resulting in a height map that is then colored by the plotted variable. You might want to use this plot to examine 2D datasets because features of the plotted variable are highlighted by the height of the plot in addition to being highlighted by the plot's colors.

Surface and Wireframe modes
"""""""""""""""""""""""""""

The Surface plot can be drawn in different ways. The default appearance of the Surface plot draws the surface only. Wireframes, which are essentially mesh lines, can also be drawn. You can draw wireframes with or without the surface. To select which parts of the plot are drawn, check
the
**Surface**
and
**Wireframe**
check boxes in the
**Surface plot attributes window**
.

Surface color
"""""""""""""

By default, the Surface plot is colored by the variable value or Z-value as with the Pseudocolor plot. The second coloration scheme uses constant coloration where the entire surface is the same color (see
). To choose a coloration scheme, click on the
**Constant**
or
**Z-Value**
radio buttons. When the plot uses constant coloring, you can change the color by clicking on the color button next to the
**Constant**
radio button and selecting a new color from the popup color menu. When the plot uses coloring based on the
**Z-Value**
, you change colors by selecting a new color table name from the color table button next to the
**Z-Value**
radio button. The available color table names are an up-to-date list of VisIt color table names.

Wireframe properties
""""""""""""""""""""

The Surface plot's wireframe lines can have different line styles, widths, and colors. To set the line width, make a selection from the
**Line width**
menu. To set the line style, make a selection from the
**Line style**
menu. To change the wireframe color, click on the
**Wire color**
button and choose a new color from the popup color menu. Note that wireframes must be enabled to set their properties.

Scaling the data
""""""""""""""""

The scale is map data values to color values and surface height. VisIt provides three scaling options:
**Linear**
,
**Log**
, and
**Skew**
.
**Linear**
, which is the default, uses a linear mapping of data values to color values.
**Log**
scaling is used to map small ranges of data to larger ranges of color (see
).
**Skew**
scaling (
) goes one step further by using an exponential function based on a skew factor to adjust the mapping of data to colors and surface height. The function used in skew scaling is
**(s^d - 1) / (s - 1) **
where s is a skew factor greater than zero and d is a data value that has been mapped to a range from zero to one.
**Skew**
scaling can be customized by changing the skew factor. A skew factor of one is equivalent to linear scaling but values either larger or smaller than one produce curves that map either the high or low end of the data to a larger color range. To change the skew factor, choose
**Skew**
scaling and type a new skew factor into the
**Skew factor**
text field.



Limits
""""""

Setting limits for the plot imposes artificial minima and maxima on the plotted variable. This effectively restricts the range of data used to color and set the height for the Surface plot. You might set limits when you are only interested in a small range of the data or when data limits need to be maintained for multiple time steps, as when playing an animation. Setting limits often highlights a certain range in the data by assigning more colors to that data range.

When setting the limits for the Surface plot, the first option to set is the limit mode. The limit mode determines whether the original data extents are used or the current plot data extents, which may vary, are used. To select the limit mode, choose either
**Use Original Data**
or
**Use Current Plot**
from the
**Limits**
menu
.

The limits for the Surface plot consist of a minimum value and a maximum value. You may set these limits, and turn them on and off, independently of one another. That is, the use of one limit does not require the use of the other. To set a limit, check the
**Min**
or
**Max**
check box next to the
**Min**
or
**Max**
text field and type a new limit value into the
**Min**
or
**Max**
text field.

Lighting
""""""""

Lighting adds detail and depth, two characteristics that are important for animations, to the Surface plot. You can click the
**Lighting**
check box in the lower part of the
**Surface plot attributes window**
to turn lighting on and off. Since lighting is on by default, uncheck the
**Lighting**
check box to turn lighting off.
