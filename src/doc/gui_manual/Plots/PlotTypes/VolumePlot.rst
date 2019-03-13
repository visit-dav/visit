Volume plot
~~~~~~~~~~~

.. _volume_plot_example:

.. figure:: ../images/volumeplot.png
   
   Volume Plot Example

This plot, shown in (:numref:`Figure %s<volume_plot_example>`), uses both color
and transparency to visualize 3D scalar fields. The values in the data range
have associated color and opacity values that allow parts of the dataset to
become partially or completely transparent. This plot captures internal details
while keeping the whole dataset at least partially visible.

The Volume plot uses a visualization technique known as volume-rendering, which
assigns color and opacity values to a range of data values. The colors and
opacities are collectively known as a volume transfer function. The volume
transfer function determines the colors of the plot and which parts are
visible. The Volume plot uses three types of volume-rendering to visualize
data.

The first volume rendering method, hardware-accelerated splatting, resamples
the entire database onto a small rectilinear grid and then, at each node in
the grid, draws a small textured polygon. The polygon gets its colors and
opacity from the transfer function. This method is fast due to its use of
graphics hardware but it can require a large number of points in the
resampled mesh to look accurate.

Like the first volume rendering method, the second method, hardware-accelerated
3D texturing, resamples the entire database onto a small rectilinear grid. Once
the data has been resampled, it is converted into a 3D texture using the Volume
plot's volume-transfer function and gets loaded into the video card's texture
memory. The Volume plot then draws a set of planes that are perpendicular to
the view vector from back to front, with each plane getting the pre-loaded
texture mapped onto it. The resulting image is very crisp and captures details
not evident when the splatting method is used.

The third volume-rendering technique, called ray-casting, used by the Volume
plot is not hardware accelerated. In ray-casting, a ray is followed in reverse
from the computer screen into the dataset. As a ray progresses through the
dataset, sample points are taken and the sample values are used to determine
a color and opacity value for the sample point. Each sample point along the
ray is composited to form a final color for the screen pixel. Rays are traced
from closest to farthest to allow for early ray termination which stops the
sampling process when the pixel opacity gets above a certain threshold. This
method of volume-rendering yields superior pictures at the cost of speed and
memory use.


.. _volume_plot_atts_window:

.. figure:: ../images/volumewindow.png

.. figure:: ../images/volumewindow2.png
   
   Volume Attributes Window


The **Volume Attributes Window**, shown in 
(:numref:`Figure %s<volume_plot_atts_window>`), is divided into two main tabs.
The **Rendering Options** tab controls the rendering setting. Each volume 
rendering method has a different set of inputs. For example, splatting and
3D Texturing both specify the total number of samples, while ray-casting 
settings are specified in samples-per-ray. Additionally, the **Rendering 
Options** tab contains controls for lighting. **1D transfer function tab** 
controls how the data is mapped onto colors and the opacities to use for
different scalar values.

Setting colors
""""""""""""""

You can design the color component of the volume transfer function using the
controls in **1D Transfer Function** tab of the **Volume Attributes Window**. 
The controls are
similar to the controls for the **Color Table Window**. There is a color
spectrum that has color control points which determine the final look of the
color table. Color control points are added and removed using the ``+``
and ``-`` buttons. Dragging control points with the mouse moves them and 
changes their order. Right-clicking on a color control point displays a
popup color menu from which a new control point color can be chosen.

Limits
""""""

The **1D transfer function** tab provides controls for setting the limits of
the variable being plotted. Limits are artificial minima or maxima that are
specified by the user. Setting the limits to a smaller range of values than
present in the database cause the plot's colors to be distributed among a
smaller range of values, resulting in a plot with more color variety.

To set the limits are set by first clicking the **Min** 
or **Max** check box next to the **Min** or **Max** text field. Clicking a
check box enables a text field into which you can type a new minimum or
maximum value.

Like VisIt's other plots that map scalar values to colors, the Volume plot
allows for the data values to be scaled using Linear, Log, and Skew functions.
To select a scaling function other than linear where values in the data range
are mapped 1:1 to values in the color range, click on the **Log** or **Skew**
radio buttons.

Setting opacities
"""""""""""""""""

The **1D transfer function** tab provides several controls that allow you
to define the opacity portion of the volume transfer function. The opacity
portion of the volume transfer function determines what can be seen in the
volume-rendered image. Data values with a lower opacity allow more to be seen
and give the plot a gel-like appearance, while data values with higher opacity
appear more solid and occlude objects behind them. The controls for setting
opacities are located at the button of the window in the **Opacity** area.

.. _volume_plot_opacity:

.. figure:: ../images/volume_opacity.png
   
   Volume Plot Opacity Options

You can set opacity three ways. You can hand-draw an opacity map, create it by
designing curves that specify the opacity when they are added together, or use 
the opacities in the color table, if present. All 
methods use the controls shown in :numref:`Figure %s<volume_plot_atts_window>`.

The interaction mode determines how opacity is set. Clicking on the
**Freeform** or **Gaussian** radio buttons selects the interaction mode.
If the interaction mode switches from **Gaussian** to **Freeform**, the shape
constructed by the **Gaussian** controls is copied to the **Freeform** control.
Both controls pretend that the plot's data range is positioned horizontally
such that the values on the left of the control correspond to the low data
values while the values on the right of the control correspond to high data
values. In addition to the color map, there is a histogram of the current data
to aide in setting opacity of interesting values. 
The vertical direction corresponds to the opacity for the given data
value. Taller curves are more opaque while shorter curves are more transparent.


.. _volume_plot_freeform:

.. figure:: ../images/volume_freeform_controls.png
   
   Volume Plot Freeform Opacity Options



To design an opacity map using the **Freeform** control, position the mouse over
it and click the left mouse button while moving the mouse. The shape traced by
the mouse is entered into the **Freeform** control so you can draw the desired
opacity curve. Immediately under the **Freeform** control, there are four
buttons, shown in (:numref:`Figure %s<volume_plot_freeform>`), which can be
used to manipulate the curve. The first three buttons initialize a new curve.
The black button makes all data values completely transparent. The ramp button
creates a linear ramp of opacity that emphasizes high data values. The white
button makes all data values completely opaque. The **Smooth** button smooths
out small bumps in the opacity curve that occur when drawing the curve by hand.

.. _volume_plot_gauss_controls:

.. figure:: ../images/volume_gauss_controls.png
   
   Volume Plot Gaussian Opacity Options

The **Gaussian** control used during Gaussian interaction mode is complex but
it provides precise control over the shape of a curve. The basic paradigm
followed by the **Gaussian** control is that new curves are added and reshaped
to yield the desired opacity curve. You add new curves by clicking and dragging
in the control. Right clicking with the mouse on an existing curve removes the
curve. Each curve has five control points which can change the curve's position
and shape. The control points are shown in along with the shapes that a curve
can assume. A control point changes color when it becomes active so there you
know which control point is used. Curves start as a smooth Gaussian shape but
they can change between the shapes shown in by moving the shape control point
up and down or left and right. Opacity maps are typically created by adding
several curves to the window and altering their shapes and sizes until the
desired image is obtained in the visualization window. The 
**Attenuation slider**, the final control involved in creating an opacity map,
controls the opacity of the entire opacity map defined by the **Freeform**
or **Gaussian** controls. It provides a knob to scale all opacities without
having to modify the opacity map.

Changing the opacity variable
"""""""""""""""""""""""""""""

The variable used to determine opacity does not have to be the plotted
variable. Having a different opacity variable than the plotted variable
is useful for instances in which you want to determine the opacity using a
variable like density while coloring the plot by another variable such as
pressure. To change the opacity variable, select a new variable from the
**Opacity variable** variable menu. By default, the plotted variable is
used as the opacity variable. This is implied when the **Opacity variable**
variable button contains the word default. Even when "default" is chosen, it
is possible to set artificial data limits on the opacity variable by entering
new values into the **Min** or **Max** text fields.

Controlling image quality
"""""""""""""""""""""""""

When the Volume plot is drawn with graphics hardware, the database is resampled
onto a rectilinear grid that is used to place the polygons that are drawn to
produce the image. You can control the coarseness of the resampled grid with the
**Number of samples** text field and slider. To increase the number of sample
points, enter a larger number into the **Number of samples** text field or move
the slider to the right. Note that the slider is on an exponential scale and
moving it to the right increases the number of sample points exponentially.

In addition to setting the number of samples, when the Volume plot is using the
3D texturing method, you can set the number of planes to be drawn from back to
front. Increasing the number of planes can help to reduce the amount of
aliasing in the resulting image. However, as the Volume plot uses a higher
number of planes, more work must be done to draw the plot and it takes a little
longer to draw. To set the number of planes, enter a new number of planes into
the **Number of slices** text field.

When the Volume plot is drawn in ray casting mode, the number of samples along
each ray that is cast through the data becomes important. Having too few sample
points along a ray gives rise to sampling artifacts such as rings or voids.
You should adjust this number until you are satisfied with the image. More
samples generally produce a better image, though the image will take longer to
produce. To change the number of samples per ray, enter a new number of samples
per ray into the **Samples per ray** text field.

When using lighting, the gradient calculation method that the Volume plot uses
influences the quality of the images that are produced. By default, VisIt uses
the Sobel operator, which uses more information from adjacent cells to
calculate a gradient. When the Sobel operator is used to calculate the gradient,
lighting usually looks better. The alternative gradient calculation method is
centered-differences and while it is much less compute intensive than the Sobel
operator, it also produces lesser quality gradient vectors, which results in
images that are not lit as well. To change the gradient calculation method,
click on either the **Centered diff** or **Sobel** radio buttons.

Software rendered images
""""""""""""""""""""""""

The Volume plot uses hardware-accelerated graphics by default. While you will
want to operate in this mode most of the time, since it's faster, images drawn
by software are more accurate. To get a more accurate image, select
**Ray casting** from the **Rendering method** combo box. When the Volume plot
is set to use ray casting as its rendering mode, VisIt recalculates what the
image should look like in software mode. Note that this can be a time-consuming
process if the database being used is large or if the visualization window is
large. We recommend shrinking the size of the visualization window before
changing the rendering method to ray casting to reduce the time and resources
required to draw the plot. It is worth noting that if you have a large dataset
with intricate details, the software volume rendering method is the best method
to use because it scales well in parallel. Using a parallel compute engine can
greatly speed up the rate at which software volume rendering operates as long
as the dataset is domain-decomposed into equal-sized pieces.

Lighting
""""""""

The Volume plot can use lighting to enhance the look of the plot. Lighting is
enabled by default but you can disable it by unchecking the **Lighting** check
box near the bottom of the window. Note that lighting is not currently
available when the Volume plot is using the ray casting volume renderer.
