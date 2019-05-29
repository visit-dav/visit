.. _Data_Analysis

Data Analysis
=============

.. toctree::
       :maxdepth: 2

This section describes two important abstractions in VisIt: Queries and Expressions.

Queries
-------

What are queries
~~~~~~~~~~~~~~~~

Queries are the mechanism to do data analysis, to pull out a number or curve that describes the data set.

Experiment with queries
~~~~~~~~~~~~~~~~~~~~~~~

1. Go to *Controls->Query*.
2. This brings up the Query window.

[[Image:Query_window.png]]

Variable-related
""""""""""""""""

1. Change the *Display* in the Query window to be *Variable-related*.
2. Go back to the GUI, delete any plots, open up "example.silo", create a Pseudocolor plot of "temp" and click *Draw*.
3. Highlight *MinMax* and click *Query*.

  * The result will be displayed in the *Query results*. It will tell you the minimum, maximum and their locations.

4. Apply the Slice operator to your plot.
5. Do another *MinMax* query.

  * It gives you the same results. This is because the Query parameter *Original data* is selected.  This means the answer is for what is in the file, not what is on the screen.

6. Change the Query parameter to be *Actual data*.
7. Do another *MinMax* query.

  * This time the answer will be the minimum and maximum constrained to the slice.

----

# Now highlight ''Variable Sum'' and click ''Query''.
#* This will sum up all of the values in the data set.
# Now highlight ''Weighted Variable Sum'' and click ''Query''.
#* This will sum up all of the values, but it will weight by area (since you have a slice).
#* For 3D, it will weight by volume.
#* For axi-symmetric 2D calculations, it will weight by revolved volume.
# Note that both queries have options for doing queries over time (grayed out because we don't have a time varying data set).
#* This is for time varying data and will produce a curve in a separate window.
----
# Now highlight ''Lineout''.
#* Note that you must have left ''Project to 2D'' enabled in the Slice operator for this next one to work correctly.
# Change the start point to "-5 -5 0" and the end point to "5 5 0".
# Click ''Query''.
# This is a way to get exact lineouts.
# You can also take 3D lineouts this way.

Pick-related
""""""""""""

* Pick: you give a 3D location and VisIt will tell you about the zone that contains that location.  
* Under the ''Variable-related'' display, there is a ''Pick'' entry.  Highlight that.
* Experiment with the four primary modes:
*# ''Pick using coordinate to determine zone''
*# ''Pick using coordinate to determine node''
*# ''Pick using domain and element Id''
*# ''Pick using global element Id''
*#* Note that these last two have support for both node and zone Ids.
* Note that the locations are *after* the slice.  
* If this is confusing, then remove the Slice operator.

Mesh-related
""""""""""""

# Change the ''Display'' in the Query window to be ''Mesh-related''.
# Experiment with the ''2D area'', ''SpatialExtents'', ''NumZones'', and ''Zone Center'' queries.
#* For the ''Zone Center'' query, you will set the ''Domain'' to "0".  
#* The domain is used for when you have a parallel file, where the data has been "domain decomposed" for parallel processing.

ConnectedComponents related
"""""""""""""""""""""""""""

# If you haven't already removed the slice operator, do that now, so you have just a Pseudocolor plot of "temp".
# Apply the Isovolume operator.  Change the ''Lower bound'' of the Isovolume operator attributes to be "4".
# You will now see a bunch of blobs in space.
# Change the ''Display'' in the Query window to be ''ConnectedComponents-related''.
# Perform the ''Number of Connected Components'' query.
#* It should tell you that there are 15 components.
# Apply the Clip operator with the default settings.
# Perform the ''Number of Connected Components'' query again.
#* It should now say there are 14 components.
#* Operators affect queries.

Queries over Time
-----------------

What are queries over Time
~~~~~~~~~~~~~~~~~~~~~~~~~~

Queries over time perform analysis through time and generate a time-curve.

Experiment with queries over time
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Weighted Variable Sum
"""""""""""""""""""""

<ol>
<li>Go to ''Controls->Query''.</li>
<li>This brings up the Query window.</li>
[[Image:WVS-Query2.png]]
<li>Go back to the GUI, delete any existing plots, open up "wave.visit", and make a Pseudocolor plot of "pressure" and click ''Draw''.</li>
<li>Find and Highlight ''Weighted Variable Sum'' and click ''Do Time Query''.</li>
<li>Options for changing the ''Starting timestep'', ''Ending timestep'' and ''Stride'' will be available.</li>
<ul>
<li>Note that these are 0-origin timestate indices and not cycles or times.</li>
</ul>
<li>Click ''Query''.</li>
<ul>
<li>The result will be displayed in a new Window. By default the x-axis will be cycle and the y-axis will be the weighted summation of the "pressure".</li>
</ul>
[[Image:WVS-TimeCurveWindow.png]]
</ol>

==== Pick ====

<ol>
<li>Pick can do multiple-variable time curves.</li>
<li>Make ''Window 2'' active, delete the plot, and make ''Window 1'' active again.</li>
<li>Find and Highlight ''Pick'' in the Query window and click ''Do Time Query'' to enable time-curve options.</li>
[[Image:TimePickOptions.png]]
<li>Change the ''Variables'' option to add "v" using the ''Variables->Scalars'' dropdown menu.</li>
<li>Select ''Pick using domain and element Id''. Leave the defaults for ''Node Id'' and ''Domain Id'' as "0".</li>
<li>Select ''Preserve Picked Element Id''.</li>
<li>Click ''Query''.</li>
<ul>
<li>The result will be two curves in a single xy plot.</li>
</ul>
<li>Make ''Window 2'' active, delete the plot, and make ''Window 1'' active again.</li>
<li>Change the ''Multiple-variable Time Curve options'' to ''Create Multiple Y-Axes plot''.</li>
<li>Click ''Query''.</li>
<ul>
<li>The result will be a Multi-curve plot (multiple axes) in ''Window 2''.</li>
</ul>
[[Image:MultiCurve TimePick.png]]
<li>'''NOTE:'''  Time Pick can also be performed via the mouse by first setting things up on the ''Time Pick'' tab in the Pick window (''Controls->Pick'').</li>
</ol>

=== Changing global options ===

<ol>
<li>Go to ''Controls->Query over time options''.</li>
<li>This brings up the QueryOverTime window.</li>
[[Image:Global-QueryOverTime-Options.png]]
<li>Here you can change the values displayed in the x-axis for all subsequent queries over time.</li>
<li>You can also change the window used to display time-curves. By default, the first un-used window becomes the time-curve window, and all subsequent time-curves are generated in the same window.</li>
</ol>

== Built-in Queries ==

[[Built-in-Queries | Built-in queries descriptions]]

== Expressions ==

Expressions in VisIt create new mesh variables from existing ones. These are also known as ''derived quantities''.  VisIt's expression system supports only derived quantities that create a new mesh variable defined over the ''entire'' mesh. Given a mesh on which a variable named <tt>pressure</tt> is defined, an example of a very simple expression is <tt>2*pressure</tt>. On the other hand, suppose one wanted to sum (or integrate) <tt>pressure</tt> over the entire mesh (maybe the mesh represents some surface area over which a force calculation is desired). Such an operation is not an expression in VisIt because it does not result in a new variable defined over the entire mesh. In this example, summing pressure over the entire mesh results in a single, scalar, number, like <tt>25.6</tt>.. Such an operation is supported instead by VisIt's Variable Sum Query. This tends to be true in general; Expressions define whole mesh variables while Queries define single numerical values (there are, however, some Queries for which this is not strictly true).

=== A simple algebraic expression, <tt>2*radial</tt> ===

[[Image:Expr_insert_variable_pulldown2.png|300px|thumb|Using Expression Window Insert Variable...]]
[[Image:New_variable_in_menu2.png|thumb|Expression variable appears in plot menus]]

# Open up <tt>noise2d.silo</tt>.
# Put up a Pseudocolor plot of the variable <tt>radial</tt> and hit draw
#* Take note of the legend range, <tt>0...28.28</tt>
# Go to Controls->Expressions
# Click on "New" in the bottom left.
#* This will create an expression and give it a default name, <tt>unnamed1</tt>.
# Rename this expression by typing <tt>radial2</tt> into the ''Name'' field
#* Take note of  the ''Type'' of the variable. By default, VisIt assumes the type of the new variable you are creating is a s scalar mesh variable (e.g. a single numerical value for each node or zone/cell in the mesh). Here, we are indeed creating a scalar variable and so there is no need to adjust the ''Type''. However, in some of the examples that follow, we'll be creating vector mesh variables and if we don't specify the correct type, we'll get an error message.
# Place the cursor in the ''Definition'' pane of the ''Expressions'' dialog.
# Type the number <tt>2</tt> followed by the C/C++ language symbol for multiplication, <tt>*</tt>.
# Now, you can either type the name <tt>radial</tt> or you can go to the ''Insert Variable...'' pulldown menu and find and select the ''radial'' variable there (see picture at right).
# Hit the ''Apply'' button
# Now, go to the main VisIt GUI Panel to the ''Variables'' pulldown
#* Note that <tt>radial2</tt> now appears in the list of variables there.
# Select <tt>radial2</tt> from the pull down and hit draw
#* Visually, the image will not look any different. But, if you take a close look at the legend you will see it is now showing <tt>0...56.57</tt>.

Visit supports several unary and binary algebraic expressions including <tt>+, -, /, *, bitwise-^, bitwise-&, sqrt(), abs(), ciel(), floor(), ln(), log10(), exp()</tt> and more.

=== Accessing coordinates (of a mesh) in Expressions ===

Here, we'll use the category of ''Mesh'' expressions to access the coordinates of a mesh, again, working with <tt>noise2d.silo</tt>

# Go to Controls->Expressions
# Hit ''New'' button and name this expression <tt>Coords</tt>.
# Set the type to Vector Mesh Variable (because coordinates, at least in this 2D example, are a vector quantity)
# Put the cursor in the ''Definition'' pane.
# Go to ''Insert Function...'' and find the ''Mesh'' category of expressions and then, within it, find the <tt>coord</tt> function expression.
#* This should result in the insertion of <tt>coord()</tt> in the ''Definition'' pane and place the cursor between the two parenthesis characters.
#* Note that in almost all cases, the category of ''Mesh'' expressions expect one or more mesh variables as operands.
# Now, go to ''Insert Variable...'' pull down and then to the ''Meshes'' submenu and select ''Mesh''.
#* This should result in ''Mesh'' being inserted between the parentheses in the definition.
# Hit apply.
# Now, we'll define two scalar expressions for the <tt>X</tt> and <tt>Y</tt> coordinates of the mesh. While still in the Expressions window,
## Hit New
## Name the new expression <tt>X</tt>
##* Note that VisIt's expression system is case sensitive so <tt>x</tt> and <tt>X</tt> can be different variable names.
## Leave the type as ''Scalar Mesh Variable''
## Type into the definition pane, <tt>Coords[0]</tt>
##* This expression uses the array bracket dereference operator <tt>[]</tt> to specify a particular component of an array. In this case, the ''array'' being derefrenced is the vector variable defined by <tt>Coords</tt>
##* Note that VisIt's expression system always numbers its array indices starting from zero
## Hit apply
## Now, repeat these steps to define a <tt>Y</tt> expression for the <tt>Y</tt> coordinates
# Finally, we'll define the ''distance'' expression
## Hit New button
## Give the new variable the name <tt>Dist</tt> (Type should be Scalar Mesh Variable)
## Type in the definition <tt>sqrt(X*X+Y*Y)</tt>
## Hit apply

[[Image:Hardy_global_iso_by_dist.png|thumb|300px|Expression example]]

Now, we'll use the new <tt>Dist</tt> variable we've just defined to display some data.

# Delete any existing plots from the plot list
# Add a Pseudocolor plot of <tt>shepardglobal</tt>
# Hit Draw
# Add an Isovolume operator
#* Although this example is a 2D example and so ''volume'' doesn't seem to apply, VisIt's Isovolume operator performs the equivalent areal operation for 2D data.
# Bring up the Isovolume operator attributes (either expand the plot by clicking on the triangle to the left of its name in the plot list and double clicking on the Isovolume operator there or go to the Operator Attributes menu and bring up Isovolume operator attributes that way).
# Set the variable to <tt>Dist</tt>
# Set the ''Lower bound'' to 5 and the ''Upper bound'' to 7.
# Hit Apply
# Hit Draw

You should get the picture at right. In this picture, we are displaying a Pseudocolor plot of <tt>shepardglobal</tt>, but Isovolumed by our <tt>Dist</tt> expression in the range <tt>[5...7]</tt>

This example also demonstrates the use of an expression ''function'', ''coord()'' to operate on a mesh and return its coordinates as a vector variable on the mesh.

VisIt has a variety of expression functions that operate on a Mesh including ''area'' (for 2D meshes), ''volume'' (for 3D meshes, ''revolved_volume'' (for 2D cylindrically symmetric meshes), ''zonetype'', and more. In addition, VisIt includes the entire suite of ''Mesh Quality Expressions'' from the [https://cubit.sandia.gov/public/verdict.html Verdict Library]

=== Creating Vector and Tensor Valued Variables from Scalars ===

If the database contains scalar variables representing the individual components of a vector or tensor, VisIt's Expression system allows you to construct the associated vector (or tensor). You create vectors in VisIt's Expression system using the curly bracket ''vector compose'' <tt>{}</tt> operator. For example, using <tt>noise2d.silo</tt> again as an example, suppose we want to compose a ''Vector'' valued expression that has <tt>shepardglobal</tt> and <tt>hardyglobal</tt> as components. Here are the steps...

# Controls->Expressions
# Hit New and set ''Name'' to <tt>randvec</tt>
# Be sure to also set ''Type'' to Vector Mesh Variable
# Place cursor in ''Definition'' pane and type <tt>{shepardglobal, hardyglobal}</tt>
# Hit Apply
# Go to Plots->Vector
#* You should now see <tt>randvec</tt> appear there as a variable name to plot
# Add the Vector plot of <tt>randvec</tt>
# Hit Draw

[[Image:Vector_tensor.png|300px|thumb|Example of Vector and Tensor Expressions Plotted]]

In the example above, we used the ''vector compose'' operator, <tt>{}</tt> to create a vector variable from multiple scalar variables. We can do the same to create a tensor variable. Recall from calculus that a rank 0 tensor is a scalar, a rank 1 tensor is a vector and a rank 2 tensor is a matrix. So, to create a tensor variable, we use multiple ''vector compose'' operators nesting within another ''vector compose'' operator. Here, solely for the purposes of illustration (e.g. this isn't a physically meaningful tensor) we'll use the <tt>X</tt> and <tt>Y</tt> coordinate component scalars we defined earlier together with the <tt>shepardglobal</tt> and <tt>hardyglobal</tt>.

# Controls->Expressions
# Hit New and set ''Name'' to ''tensor''
# Be sure also to set ''Type'' to Tensor Mesh Variable
# Place cursor in ''Definition'' pane and type <tt>{ {shepardglobal, hardyglobal}, {X,Y} }</tt>
#* Note the two levels of curly braces. The outer level is the whole rank 2 tensor matrix and the inner curly braces are each row of the matrix.
#* Note that you could also have defined the same tensor expression using two vector expressions like so, <tt>{randvec, Coords}</tt>
#* Note that for a symmetric tensor definition, each succeeding row in the matrix would have one less term. In 3D, the first row would have 3 terms, the 2nd 2 terms and the 3rd 1 term. In 2D, the first row would have 2 terms and the 2nd 1 term.
# Hit Apply
# Add a Tensor plot of <tt>tensor</tt> variable and hit Draw

=== Variable Compatibility Gotchas (Tensor Rank, Centering, Mesh) ===

VisIt will allow you to define expressions that it winds up determining to be invalid later when it attempts to execute those expressions. Some common issues are the mixing of incompatible mesh variables in the same expression ''without'' the necessary additional functions to make them compatible.
----

==== Tensor Rank Compatibility ====

For example, what happens if you mix scalar and vector mesh variables (e.g. variables of different ''Tensor Rank'') in the same expression? Again, using <tt>noise2d.silo</tt>

# Define the expression, <tt>foo</tt> as <tt>grad+shepardglobal</tt> with Type ''Vector Mesh Variable''
#* Note that <tt>grad</tt> is a Vector Mesh Variable and <tt>shepardglobal</tt> is a Scalar Mesh Variable.
# Now, attempt to do a Vector plot of <tt>foo</tt>. This works because VisIt will add the scalar to each component of the vector resulting a new vector mesh variable
# But, suppose you instead defined <tt>foo</tt> to be of Type ''Scalar Mesh Variable''
#* VisIt will allow you to define this expression. But, when you go to plot it, the plot will fail.

As an aside, as you go back and forth between the Expressions window creating and/or adjusting expression definitions, VisIt makes no attempt to keep track of all the changes you've made in expressions and automatically update plots as expressions change. You will have to manually clear or delete plots to force VisIt to re-draw plots in which you've changed expressions.

In the above example, if on the other hand, you had set type of <tt>foo</tt> to Scalar Mesh Variable, then VisIt would have failed to plot it because it is adding a scalar and a vector variable and the result of such an operation is ''always'' a vector mesh variable. If what you really intended was a scalar mesh variable, then use one of the expression functions that converts a vector to a scalar (e.g. <tt>magnitude()</tt> function or array dereference operator <tt>[]</tt>) to convert vector mesh variable in your expression to a scalar mesh variable. So, <tt>grad[i]+shephardglobal</tt> where <tt>i</tt> is <tt>0</tt> or <tt>1</tt> would work to define a scalar mesh variable. Or, <tt>magnitude(grad)+shepardglobal</tt> would also have worked.
----

==== Centering Compatibility ====

In <tt>noise2d.silo</tt>, some variables are zone centered and some are node centered. What happens if you combine these in an expression? VisIt will default to zone centering for the result. If this is not the desired result, use the <tt>recenter()</tt> expression function, where appropriate, to adjust centering of some of the terms in your expression. For example, again using <tt>noise2d.silo</tt>.

# Define the scalar mesh variable expression <tt>bar</tt> as <tt>shepardglobal+airVf</tt>
#* For reference, in <tt>noise2d.silo</tt>, <tt>shepardglobal</tt> is node centered while <tt>airVf</tt> is zone centered.
# Do a Pseudocolor plot of <tt>bar</tt>.
#* Note that <tt>bar</tt> displays as a zone centered quantity
# Now, go back to the expression and recenter <tt>airVf</tt> by adjusting the definition to <tt>shepardglobal+recenter(airVf)</tt>
#* The <tt>recenter()</tt> expression function is a ''toggle'' in that it will take whatever the variable's centering is and swap it (node->zone and zone->node)
#* The <tt>recenter()</tt> expression function also takes a second argument, a string of one of the values ''toggle'', ''zonal'', ''nodal'' to force a particular behavior.
<!-- NOTE TO SELF: This won't make sense when edge and face centering are supported -->
#* Note that when you hit Apply, the current plot of <tt>bar</tt> does not change. You need to manually delete and re-create the plot (or clear and re-draw the plots).

Finally, note that these two expressions...
* <tt>shepardglobal+recenter(airVf)</tt>
* <tt>recenter(shepardglobal+airVf)</tt>
both achieve a node-centered result. But, each expression is subtly (and numerically) different. The first recenter's <tt>airVf</tt> to the nodes and then performs the summation operator at each node. In the second, there is an implied recentering of <tt>shepardglobal</tt> to the zones first. Then, the summation operator is applied at each zone center and finally the results are recentered back to the nodes. In all likelihood this results in a numerically lower quality result. The moral is that in a complex series of expressions be sure to take care where you want recentering to occur.
----

==== Mesh Compatibility ====

In many cases, especially in Silo databases, all the available variables in a database are not always defined on the same mesh. This can complicate matters involving expressions in variables from different meshes. 

Just as in the previous two examples of incompatible variables where the solution was to apply some functions to make the variables compatible, we have to do the same thing when variables from different meshes are combined in an expression. The key expression functions which enable this are called ''Cross Mesh Field Evaluation'' or ''CMFE'' functions. We will only briefly touch on these here. CMFEs will be discussed in much greater detail in a tutorial devoted to that topic.

Again, using <tt>noise2d.silo</tt>

# Define the expression<tt>gorf</tt> with definition <tt>PointVar + shepardglobal</tt>
#* Note that <tt>PointVar</tt> is defined on a mesh named <tt>PointMesh</tt> while <tt>shepardglobal</tt> is defined on a mesh named <tt>Mesh</tt>.
# Try to do a Pseudocolor plot of <tt>gorfo</tt>. You will get a plot of points and a warning message like this one...

 The compute engine running on host somehost.com issued the following warning:
 In domain 0, your nodal variable "shepardglobal" has 2500 values, but it should have 100.
 Some values were removed to ensure VisIt runs smoothly.

<!-- NOTE TO SELF: This seems like a troubling outcome. The plot is wrong but user doesn't get any warning of that -->
So, whats happening here? VisIt is deciding to perform the summation operation on the <tt>PointVar</tt>'s mesh. That mesh consists of 100 points. So, when it encounters the <tt>shepardglobal</tt> variable (defined on ''Mesh'' with 50x50 nodes), it simply ignores any values in <tt>shepardgloabl</tt> after the first 100. Most likely, this is not the desired outcome.

We have two options each of which involves ''mapping'' one of the variables onto the other variable's mesh using one of the CMFE expression functions. We can map <tt>shepardglobal</tt> onto <tt>PointMesh</tt> or we can map <tt>PointVar</tt> onto <tt>Mesh</tt>. We'll do both here

===== Mapping <tt>shepardglobal</tt> onto <tt>PointMesh</tt> =====

[[Image:Shepardglobal_mapped_plus_PointVar.png|300px|thumb]]

# Define a new expression named <tt>shepardglobal_mapped</tt>
# Go to ''Insert Function...'', then to the ''Comparisons'' submenu and select <tt>pos_cmfe</tt>
#* This defines a ''position based'' cross-mesh field evaluation function. The other option is a <tt>conn_cmfe</tt> or ''connectivity-based'' which is faster but requires both meshes to be topologically congruent and is not appropriate here.
# A template for the arguments to the <tt>pos_cmfe</tt> will appear in the Definition pane.
# Replace <tt><filename:var></tt> with <tt><./noise2d.silo:shepardglobal></tt>
#* This assumes the <tt>noise2d.silo</tt> file is in the same directory from which VisIt was started.
#* This defines the ''source'' or ''donor'' variable to be mapped onto a new mesh
# Replace <tt><meshname></tt> with <tt>PointMesh</tt>
#* This defines the ''destination'' or ''target'' mesh the variable is to be mapped onto
# Replace <tt><fill-var-for-uncovered-regions></tt> with <tt>-1</tt>
#* This is needed for position-based CMFE's because the donor variable's mesh and target mesh may not always volumetrically overlap 100%. In places where this winds up being the case, VisIt will use this value to fill in.
# Now with <tt>shepardglobal_mapped</tt> defined, you can define the desired expression, <tt>PointVar + shepardglobal_mapped</tt> and this will achieve the desired result and is shown at right.

===== Mapping <tt>PointVar</tt> onto <tt>Mesh</tt> =====

To be completed. But, cannot map point mesh onto a volumetric mesh. VisIt always returns zero overlap.
----

=== Combining Expressions and Queries Is Powerful ===

Suppose you have a database generated by some application code simulating some object being blown apart. Maybe its a 2D, cylindrically symmetric calculation. Next, suppose the code produced a <tt>density</tt> and <tt>velocity</tt> variable. However, what you want to compute is the total mass of some (portion of) of the object that has velocity (magnitude) greater than some threshold, say 5 meters/second. You can use a combination of Expressions, Queries and the Threshold operator to achieve this.

Mass is <tt>density * volume</tt>. You have a 2D mesh, so how do you get volume from something that has only 2 dimensions? You know the mesh represents a calculation that is cylindrically symmetric (revolved around the y-axis). You can use the <tt>revolved_volume()</tt> Expression function to obtain the volume of each zone in the mesh. Then, you can multiply the result of <tt>revolved_volume()</tt> by <tt>density</tt> to get <tt>mass</tt> of each zone in the mesh. Once you have that, you can use threshold operator to display only those zones with velocity (magnitude) greater than 5 and then a variable sum query to add up all the mass moving at that velocity.

Here, we demonstrate the steps involved using the <tt>noise2d.silo</tt> database. Because that database does not quite match the problem assumption described in the preceding paragraphs, we simply  re-purpose a few of the variables in the database to serve as our <tt>density</tt> and <tt>velocity</tt> variables in this example. Namely, we define the expression <tt>density</tt> as an alias for <tt>shephardglobal</tt> and <tt>velocity</tt> as an alias for <tt>grad</tt>.

[[Image:Mass_definition.png|300px|thumb|Mass Expression Definition]]

Here are the steps involved...

# Controls->Expressions
# Hit New
# Set ''Name'' to <tt>mass</tt>
# Make sure Type is Scalar Mesh Variable
# Set ''Definition'' to <tt>revolved_volume(Mesh) * density</tt>.
# Hit Apply
# Hit New again (for a new expression)
# Set ''Name'' to <tt>velmag</tt> (for velocity magnitude)
# Set ''Definition'' to <tt>magnitude(velocity)</tt>
# Go to Plot->Pseudocolor->mass
# Hit Draw [[Image:Mass_plot.png|300px|thumb|Mass plot]]
# Add Operator->Threshold
# Open Threshold operator attributes
# Select the ''default'' variable and then hit ''Delete Selected Variable''
# Go to ''Add Variable'' and select <tt>velmag</tt> from the list of ''Scalars''.
# Set ''Lower Bound'' to 5
# Hit Apply
#* Now the displayed plot changes to show only those parts of the mesh that are moving with velocity greater than 5.
# Controls->Query
# Find the ''Variable Sum Query'' from the list of queries
# Hit the ''Query'' button. The computed result will be a sum of all the individual zones' masses in the mesh for those zones that are moving with velocity greater than 5.
[[Image:Velmag_threshold.png|300px|left|thumb|Threshold attributes]] [[Image:Mass_query.png|300px|right|thumb|Mass plot after threshold and query result]]

=== Automatic, Saved and Database Expressions ===

VisIt defines several types of expressions automatically. For all vector variables from a database, VisIt will automatically define the associated magnitude expressions. For unstructured meshes, VisIt will automatically define ''mesh quality'' expressions. For any databases consisting of multiple time states, VisIt will define ''time derivative'' expressions. This behavior can be controlled by going to VisIt's ''Preferences'' dialog and enabling or disabling various kinds of ''automatic'' expressions.

If you save settings, any expressions you have defined are also saved with the settings. And, they will appear (and sometimes pollute) your menus whether or not they are valid expressions for the currently active database.

Finally, databases are also free to define expressions. In fact, many databases define a large number of expressions for the convenience of their users who often use the expressions in their post-processing workflows. Ordinarily, you never see VisIt's automatic expressions or a database's expressions in the Expression window because they are not editable. However, you can check the ''display expressions from database'' check box in the Expressions window and VisIt will also show these expressions.

