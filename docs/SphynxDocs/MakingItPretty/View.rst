View
----

The view is one of the most critical properties of a visualization since it determines what parts of the dataset are seen. The view is also one of the most difficult properties to set. It isn't that the act of setting the view is difficult, in fact, it is quit the opposite. The problem with setting the view is finding a flattering view for a database that will continue to be a good view for the entire life of the visualization. Many plots will deform or expand over the course of an animation and you have to decide how to pick a good view. You can pick a view that is zoomed way out and then let your plots expand and deform until they make good use of the visualization window. You can also decide to keep changing the view throughout the animation. A common technique is to interpolate views or do some sort of fly-by animation when the plots in the animation are expanding or not behaving in a static manner. The fly-by animation is used to distract the audience from the fact that you need to change to a more suitable view.

The view in VisIt can be set in two different ways. The first and best way to set the view is to
navigate to it interactively in the visualization window. This is the fastest and most direct way of setting the view. The problem with setting the view in this manner is that it is not very reproducible. It is often the case that users want to look at the same feature in their database using the same view. VisIt provides a
**View Window**
that they can use to set the view information exactly the same every time.

View Window
~~~~~~~~~~~

You can open the
**View Window**
by selecting the
**View**
option from the
**Main Window's Controls**
menu. The
**View Window**
is divided into three tabbed sections. The first tab sets the 2D view, the second tab sets the 3D view, and the last tab sets advanced view options.

Setting the Curve view
""""""""""""""""""""""

Visualization windows that contain Curve plots use a special type of view known as a curve view. A curve view consists of: viewport, domain, and range. The viewport is the area of the visualization window that will be occupied by the plots and is specified using X and Y values in the range [0,1]. The point (0,0) corresponds to the lower-left corner of the visualization window while the point (1,1) corresponds to the visualization window's upper-right corner. To change the viewport, type new numbers into the
**Viewport**
text field on the
**Curve tab**
of the
**View Window**
(
). The minimum and maximum X values should come first, followed by the minimum and maximum Y values.

The domain and range refer to the limits on the X and Y axes. You can set the domain, which is the range of X values that will be displayed in the viewport, by typing new minimum and maximum values into the
**Domain**
text field. You should use domain values that use the same dimensions as the Curve plot that will be plotted in the visualization window. You can set the range, which is the range of Y values that will be displayed in the viewport, by typing new values into the
**Range**
text field.

Setting the 2D view
"""""""""""""""""""

Setting the 2D view is conceptually simple. There are only two pieces of information that you need to supply. The first piece of information that you must enter is the viewport, which is an area of the visualization window in which you want the 2D plots to appear. Imagine that the lower left corner of the visualization window is the origin of a coordinate system and that the upper left and lower right corners both have values of 1. Every point in the visualization window can be characterized as a cartesian coordinate where both values in the coordinate are in the range [0,1]. The viewport is specified by entering four numbers in the form x0 x1 y0 y1 where x0 is the leftmost X value, x1 is the rightmost X value, y0 is the lower Y value, and y1 is the upper Y value that will be used in the viewport. The window is an area in the space occupied by the 2D plots. You can start with a window that is the same size as the plot's spatial extents and then zoom in from there by making the window values smaller and smaller. The window values are also of the form x0 x1 y0 y1. To change the 2D view, type new values into the
**Viewport**
and
**Window**
text fields on the
**View Window's 2D view tab**
(
).

Fullframe mode
""""""""""""""

Some databases yield plots that are so long and skinny that they leave most of the vis window blank when VisIt displays them. A common example is equation of state data, which often has at least 1 exponential dimension. VisIt provides Fullframe mode to stretch long, skinny plots so they fill more of the vis window so it is easier to see them. It is worth noting that Fullframe mode does not preserve a 1:1 aspect ratio for the displayed plots because they are stretched in each dimension so they fit better in the vis window. To activate Fullframe mode, click on the
**Auto**
or
**On**
buttons in the
**View Window**
. When Fullframe mode is set to
**Auto**
, VisIt determines the aspect ratio of the X and Y dimensions for the plots being visualized and automatically scales the plots to fit the window when extents for one of the dimensions are much larger than the extents of the other dimension.

Setting the 3D view
"""""""""""""""""""

Setting the 3D view using controls in the
**View Window's**

**3D view tab**
(see
) demands an understanding of 3D views. A 3D view is essentially a location in space (view normal) looking at another location in space (focus) with a cone of vision (view angle). There are also clipping planes that lie along the view normal that clip the near and far objects from the view.
depicts the various components of a 3D view.

To set the 3D view, first decide on where you want to look from. Type a vector value into the
**View normal**
text field. Next, type the vector valued location of what you want to look at into the
**Focus**
text field. The
**Up axis**
vector is simply a vector that determines which way is up. A good default value for the up axis is 0 1 0. VisIt will often calculate a better value to use for the up axis so it is not too important to figure out the right value. The
**View Angle**
determines how wide the field of view is. The view angle is specified in degrees and a value around 30 is usually sufficient.
**Near clipping**
and
**Far clipping**
are values along the view normal that determine where the near and far clipping planes are to be placed. It is not easy to know that good values for these are so you will have to experiment
**Parallel scale**
acts as a zoom factor and larger values zoom the camera towards the focus. The
**Perspective**
check box applies to 3D visualizations and it causes a more realistic view to be used where objects that are farther away are drawn smaller than closer objects of the same size. VisIt uses a perspective view for 3D visualizations by default.

VisIt supports stereo rendering, during which VisIt draws the image in the visualization window twice with the camera eye positioned in slightly different locations to mimic the differences in images seen by your left eye and your right eye. With the right stereo goggles, the image that you see appears to hover in 3D space within your monitor since the effect of the stereo image adds much more depth to the visualization. You can set the angle that VisIt uses to separate the cameras used to draw the images by typing a new angle into the
**Eye angle**
text field or by using the
**Eye angle**
slider.

Using axis alignment buttons
""""""""""""""""""""""""""""

The axis alignment buttons in the
**3D view tab**
set the view normal, which is the direction that the view points, so it is aligned with one of the 3D axes. Clicking them provides a convenient way to get side, top, and bottom views of the plots in the visualization window.

Using view commands
"""""""""""""""""""

The
**View Window**
contains a
**Commands**
text field that allows you to enter one or more semi-colon delimited legacy MeshTV commands to change the view. The following table has a description of the supported view commands:

+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| **Command** | **Description**                                                                                                                                                                                                                                                                                                                                                                                                                       |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+=============+=======================================================================================================================================================================================================================================================================================================================================================================================================================================+
| pan x y     | Pans the 3D view to the left/right or up/down. The x, y arguments, which are floating point fractions of the screen in                                                                                                                                                                                                                                                                                                                |
|             | the range [0,1], determine how much the view is panned in the X and Y dimensions.                                                                                                                                                                                                                                                                                                                                                     |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| pan3 x y    | Same as pan.                                                                                                                                                                                                                                                                                                                                                                                                                          |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| panx x      | Pans the 3D view left or right. The x argument is a floating point fraction of the screen in the range [0,1].                                                                                                                                                                                                                                                                                                                         |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| pany y      | Pans the 3D view up or down. The yargument is a floating point fraction of the screen in the range [0,1].                                                                                                                                                                                                                                                                                                                             |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ytrans y    | Same as pany.                                                                                                                                                                                                                                                                                                                                                                                                                         |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| rotx x      | Rotates the 3D view about the X-axis x degrees.                                                                                                                                                                                                                                                                                                                                                                                       |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| rx x        | Same as rotx.                                                                                                                                                                                                                                                                                                                                                                                                                         |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| roty y      | Rotates the 3D view about the Y-axis y degrees.                                                                                                                                                                                                                                                                                                                                                                                       |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| rotz z      | Rotates the 3D view about the Z-axis z degrees.                                                                                                                                                                                                                                                                                                                                                                                       |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| rz z        | Same as rotz.                                                                                                                                                                                                                                                                                                                                                                                                                         |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| zoom val    | Scales the 3D zoom factor. If you provide a value of 2.0 for the val argument, the object being viewed will appear twice as large. A value of 0.5 for the val argument will make the object appear only half as large.                                                                                                                                                                                                                |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| zf          | Same as zoom.                                                                                                                                                                                                                                                                                                                                                                                                                         |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| zoom3       | Same as zoom.                                                                                                                                                                                                                                                                                                                                                                                                                         |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| vp          | Sets the window, which is how much space relative to the plot will be visible inside of the viewport, for the 2D view. All arguments are floating point numbers that are in the same range as the plot extents. The x0 and x1 arguments are the minimum and maximum values for the edges of the window in the X dimension. The y0 and y1 arguments are the minimum and maximum values for the edges of the window in the Y dimension. |
| x0 x1 y0 y1 |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| wp          | Sets the window, which is how much space relative to the plot will be visible inside of the viewport, for the 2D view. All arguments are floating point numbers that are in the same range as the plot extents. The x0 and x1 arguments are the minimum and maximum values for the edges of the window in the X dimension. The y0 and y1 arguments are the minimum and maximum values for the edges of the window in the Y dimension. |
| x0 x1 y0 y1 |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| reset       | Resets the 2D and 3D views                                                                                                                                                                                                                                                                                                                                                                                                            |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| recenter    | Recenters the 3D view                                                                                                                                                                                                                                                                                                                                                                                                                 |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| undo        | Changes back to the previous view                                                                                                                                                                                                                                                                                                                                                                                                     |
|             |                                                                                                                                                                                                                                                                                                                                                                                                                                       |
+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+


Advanced view features
~~~~~~~~~~~~~~~~~~~~~~

The
**View Window's Advanced tab**
, shown in
, contains features that are not needed by all users.

View centering
""""""""""""""

The view can either be based on the original spatial extents of the plot or the actual current extents which are the plot's current extents after it has been subsetted in some way. By default, VisIt bases the view on the plot's original extents which leaves the remaining bits of a plot, after being subsetted, in the same space as the original plot. This makes it easy to see where the remaining pieces of the plot were situated relative to the whole plot but it does not always make best use of the visualization window. To fill up more of the visualization window, you might want to base the view on the actual current extents which you can select by choosing the
**Actual current extents**
option from the
**View based**
on menu.

Locking views
"""""""""""""

When using more than one visualization window, such as when comparing plots using two different databases side by side, it is often useful for the plots being compared to have the same view. VisIt allows you to lock the views together for the two visualization windows so that when you change the rotate, zoom, etc on plots in any window whose view is locked, all other windows with locked views get the new view. To lock the view for a visualization window, click the
**Locked view**
check box in the
**View Window's Advanced tab**
or click on the Toolbar button to lock views.

Undo view
"""""""""

If you ever accidentally change the view when you didn't want to change it, you can click on the
**Undo view**
button on the
**View Window's Advanced tab**
so set the view back to the previous view. The last 10 views are stored so you can undo up to 10 view changes.

Setting the center of rotation
""""""""""""""""""""""""""""""

The center of rotation is the point about which plots are rotated when you set the view. You can type a new center of rotation into the
**Center**
text field and click the
**User defined center of rotation check box**
if you want to specify your own center of rotation. The center of rotation is, by default, the center of your plots' bounding box. When you zoom in to look at smaller plot features and then you rotate the plot, the far away center of rotation causes the changes to the view to be large. Large view changes when you are zoomed in often make the parts of the plot that you were inspecting go out of the view frustum. If you are zoomed in, you should pick a center of rotation that is close to the surface of the plot that you are inspecting. You can also pick a center of rotation using the
**Choose center**
option in the visualization window's
**Popup menu**
.
