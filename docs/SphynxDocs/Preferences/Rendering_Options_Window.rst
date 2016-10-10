Rendering Options Window
------------------------

The
**Rendering Options Window**
(shown in
) contains controls that allow you to set global options that affect how the plots in the active vis window are drawn and you can also look at information related to the performance of your graphics hardware. You can open the
**Rendering Options Window**
by selecting the
**Rendering**
option in the
**Main Window's Preferences**
menu. The window is divided vertically into two main areas. The top area, or
**Options area**
, contains controls that allow you to set rendering options which affect how your plots look in the vis window. Some topics related to how plots are drawn such as antialiasing, specular
lighting, and shadows are covered in the
**Making it Pretty**
chapter. The bottom area, or
**Information area**
, displays information about how fast VisIt is able to draw graphics on your computer, as well as the size of the plots being drawn in terms of triangle count and spatial dimension

Changing surface representations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Sometimes when visualizing large or complex databases, drawing plots with all of their shaded surfaces can take too long to be interactive, even for fast graphics hardware. To combat this problem, VisIt provides an option that allows you to view all of the plots in the vis window as wireframe outlines or point clouds instead of as shaded surfaces (see
). While being less visually informative, plots drawn as wireframe outlines or as clouds of points can still be useful for visualizations since you can do setup work like setting the view before switching back to a surface representation that is more costly to draw. You click the
**Surfaces**
,
**Wireframe**
, or
**Points**
radio buttons to change the surface representation used to draw plots.

Using display lists
~~~~~~~~~~~~~~~~~~~

VisIt benefits from the use of hardware accelerated graphics and one of the concepts central to hardware accelerated graphics is the display list. A display list is a sequence of simple graphics commands that are stored in a computer's graphics hardware so the hardware can draw the object described by the display list several times more quickly than it could if the graphics commands were issued directly. VisIt tries to make maximum use of display lists when necessary so it can draw plots as fast as possible.

VisIt decides when to use display lists and when to not use display lists. Typically when you are running VisIt on your local workstation with plots that result in fewer than a couple million graphics primitives, VisIt does not use display lists because the cost of creating them is more expensive than just drawing the graphics primitives without display lists. When you run a UNIX
TM
version of VisIt on a remote computer and display the results back to your workstation using an X-server, it is almost always advantageous to create display lists for plot geometry.
Without display lists, VisIt must keep transmitting the plot geometry over the network to the X-server which then uses OpenGL to draw the geometry. By default, VisIt automatically decides when to use display lists and when not to use them but you can force VisIt to either use or not use display lists. If you don't want VisIt to ever use display lists, click the
**Never**
radio button under the
**Use display lists**
options in the
**Rendering Options**

**Window**
. If you want VisIt to always use display lists, click the
**Always**
radio button under the
**Use display lists**
options.

Stereo images
~~~~~~~~~~~~~

Stereo images, which are composites of left and right eye images, can convey additional depth information that cannot be expressed by images that are generated using a single eye point. VisIt provides four forms of stereo images: red/blue, red/green, interlace, and crystal eyes. A red/blue stereo image (see
) is similar to frames from early 3D movies in that it appears stereo only when using red/blue stereo glasses. Unfortunately, red/blue stereo images are not very useful for visualization because colors are lost since most of the color ends up in the magenta range when the red and blue color channels are merged. Red/green stereo suffers similar color loss. Interlaced images alternate lines in the image with left and right eye views so that squinting makes the image look somewhat 3D. VisIt's crystal eyes option requires the use of special virtual reality goggles for images to appear to be 3D but this option is by far the best since it allows interactive frame rates with images that really appear to stand out from the computer monitor. VisIt does not use stereo imaging by default since it makes images draw slower because an image must be drawn for both the left eye and the right eye. If you want to view stereo images using the
**Crystal Eyes**
option, you must provide the
*-stereo*
command line option when you launch VisIt or clicking the
**Crystal Eyes**
radio button will have no effect because vis windows must be created in stereo mode. If you want to try looking at stereo images, click the
**Stereo**
check box and then also click one of the
**Red/Blue**
,
**Red/Green**
,
**Interlace**
, or
**Crystal Eyes**
radio buttons to choose type the stereo image type.

Scalable rendering
~~~~~~~~~~~~~~~~~~

VisIt typically uses graphics hardware on the local computer to very quickly draw plots once they have been generated by the compute engine. This becomes impractical for very large databases because the amount of memory needed to store the graphics commands that draw the plots quickly exceeds the amount of memory in the graphics hardware. Large sets of graphics commands can also degrade performance when they must be shipped over slow networks from the compute engine to the VisIt's viewer. VisIt provides a scalable rendering option that can improve both of these situations by creating the actual plot images, in parallel, on the compute engine, compressing them, and then transmitting only an image to the viewer where the image can be displayed.

Scalable rendering can be orders of magnitude faster for large databases than VisIt's conventional image drawing strategy because large databases are typically processed using a parallel compute engine. When using scalable rendering with a parallel compute engine, VisIt is able to draw small pieces of the plot on each processor in parallel and then glue the image together before sending it to the viewer to be displayed. Not only has the image likely been created faster, but the size of the image is usually on the order of a megabyte instead of the tens or hundreds of megabytes needed to transmit graphics commands, which results in faster transmission of the image to the viewer. The drawback of scalable rendering is that it is usually not as interactive as graphics hardware because each time you want to change the view or interact with the plots, round trip communication with the compute engine is required.

VisIt can automatically determine when to stop sending geometry to the viewer in favor of sending scalably rendered images. You can help VisIt decide when to use scalable rendering by either setting the scalable rendering threshold, which is the number of polygons that must exist in the set of geometry to be rendered on the viewer before the compute engine will kick into scalable rendering mode. You can set the scalable rendering threshold by entering a new number of polygons into the When polygon count exceeds spin box in the
**Rendering Options Window**
. The number that you enter is measured in thousands of polygons.

If you want VisIt to always use scalable rendering, you can click the
**Always**
radio button under the
**Use scalable rendering**
options. If you don't want VisIt to ever use scalable rendering no matter how many polygons are in the set of geometry that your workstation will have to draw, click the
**Never**
check box under the
**Use scalable rendering**
options. Note that if you disable scalable rendering and then you decide to plot a database with millions or hundreds of millions of cells, you risk running out of memory on your workstation or having to wait a very long time for the image to appear. In general, it is much faster to draw images of large databases with scalable rendering.

Frames per second
~~~~~~~~~~~~~~~~~

Frames per second refers to the number of times that VisIt can draw the plots in the vis window in the course of a second. VisIt displays the minimum, average, and maximum frame rates
achieved during the last draw operation, like rotating the image with the mouse, in the
**Renderer **
**Options Window's Information area**
. Some actions that force a redraw do not cause the information to update. An example of this is resizing the vis window. To make VisIt update the frame rate information after each time it draws the plots in the visualization window, check the
**Query after each render**
check box.

Triangle count
~~~~~~~~~~~~~~

Triangle count refers to the number of triangles used to represent the plots in the vis window. VisIt displays the triangle count in the
**Renderer Options Window's Information area**
.

Plot Extents
~~~~~~~~~~~~

The plot extents are the minimum and maximum locations of the plot in each spatial dimension. The plot extents are the smallest bounding box that can contain the plots in the vis window. VisIt displays the plot extents for each dimension in the
**Renderer Options Window's Information area**
.
