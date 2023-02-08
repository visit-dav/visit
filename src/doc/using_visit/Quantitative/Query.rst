.. |br| raw:: html

   <br>

Query
-----

VisIt allows you to gather quantitative information from the database being
visualized through the use of queries. A query is a type of calculation that
can either return values from the database or values that are calculated
from data in the database. For example, VisIt's Pick and Lineout capabilities
(described later in this chapter) are specialized point and line queries
that print out the values of variables in the database at points or along
lines. In addition to point and line queries, VisIt provides database
queries that return values that are based on all of the data in a database.

Some queries can even be executed for all of the time states in a database
to yield a Curve plot of the query's behavior over time. This feature will
be covered in more detail a little later.

.. _querywindow:

.. figure:: images/querywindow.png

   Query window

VisIt's queries are available in the **Query Window** (shown in
:numref:`Figure %s <querywindow>`), which you can open by clicking the
**Query** option in the **Main Window's Control** menu. The **Query Window**
consists of upper and lower areas where the upper area allows you to select
a query and set its query parameters. The controls for setting a query's
parameters change as required and some queries have no parameters and thus
have no controls for setting parameters. The bottom area of the window
displays the results of the query once VisIt has finished processing it.
The results for new queries are appended to the output from previous queries
until you clear the **Query results** by clicking the **Clear results**
button.

Query types
~~~~~~~~~~~

VisIt's queries can be divided into three types: database queries, point
queries, and line queries. Database queries usually calculate information
for the database as a whole instead of concentrating on a single zone or
node but some Pick-related database queries do concentrate on cells and
nodes. Point queries calculate information for a point in the database
and several types of variable picking queries fall into this category.
Line queries calculate information along a line. Each type of query has
different controls in the **Query parameters** area (see
:numref:`Figure %s <queryparams_line>`) and as you highlight different
queries, the controls in the **Query parameters** area may change.

.. image:: images/queryparams_db.png

.. image:: images/queryparams_db2.png

.. image:: images/queryparams_point.png

.. _queryparams_line:

.. figure:: images/queryparams_line.png

   Query parameters area

Database queries provide a few different interfaces depending on the query.
Many database queries require no additional input so they have no controls
except for the **Query** button. Other database queries ask whether the
query is to be performed with respect to the original data or the actual
data, which is that data that is left in the plot after subsets have been
removed and operators have transformed the data. Finally, some database
queries ask for a specific domain number and zone or node number.

Point queries provide interfaces in the **Query parameters** area that allow
you to enter a 3D point or a screen space point to use as the point for the
query. Line queries provide an interface that lets you specify the start
and end positions of the line as well as the number of sample points to
consider along the length of the line. Nearly all query types allow you to
provide additional variables to query in a **Variables** text field.

.. _built-in_queries:

Built-in queries
~~~~~~~~~~~~~~~~

Database Queries
""""""""""""""""

2D Area
    The 2D area query calculates the area of the 2D plot highlighted in the
    **Plot list** and prints the result to the **Query results**. VisIt can
    produce a Curve plot of this query with respect to time.

3D Surface Area
    The 3D surface area calculates the area of the plot highlighted in the
    **Plot list** and prints the result to the **Query results**. VisIt can
    produce a Curve plot of this query with respect to time.

Connected Component Area
    Performs the same operation as either 2/3D area query except individually
    for each *component* of a disconnected mesh. The query result is a list of
    values, one for each component.

Connected Component Length
    Performs an operation similar to *Connected Component Area* except that it
    works only for 1D components and returns their length. The query result is
    a list of values, one for each component.

Area Between Curves
    The Area Between Curves query calculates the area between 2 curve plots.
    The plots that will serve as input to this query must both be highlighted
    in the **Plot list** or VisIt will issue an error message. Once the area
    has been calculated, the result is printed to the **Query results**.

Centroid
    This query can calculate a centroid (geometric center) or center-of-mass
    of a dataset depending on the plot (and variable) upon which the query is
    performed. On a Pseudocolor plot, the plot's variable will be treated as
    a *density* field. The value of this field at the *center* of each cell
    will be multiplied by the cell's volume to compute a cell-centered mass
    contribution for each cell. If the plot's variable is indeed a true
    density variable, then the result will be the center-of-mass. If the
    plot's variable is not a true density variable (e.g. temperature), the
    result may be nonsensical. If the plot's variable is constant over the
    whole object, the result will be a centroid (geometric center). If the
    query is performed on a Mesh or FilledBoundary plot, constant density will
    be assumed and the result will be a centroid. The results are printed to
    the **Query results**. 

Connected Component Centroid
    Performs the same operation as either *Centroid* query except individually
    for each *component* of a disconnected mesh. The query result is a list of
    values, one for each component.

Chord Length Distribution
    The Chord Length Distribution query calculates a probability density
    function of chord length over a two or three dimensional object. Axially
    symmetric objects (RZ-meshes) are treated as 3D meshes and chords are
    calculated over the revolved, 3D object. A statistical approach, casting
    uniform density, random lines, is used. The result of this query is a
    curve, which is outputted as a separate file. This curve is a probability
    density function over length scale. The name of the resulting file is
    printed to the **Query results**.  

Compactness
    The Compactness query calculates mesh metrics and prints them in the
    **Query results**.

Cycle
    The Cycle query prints the cycle for the plot that is highlighted in the
    **Plot list** to the **Query results**.

Distance from Boundary
    The Distance From Boundary query calculates how much mass is at a given
    distance away from the boundary of a shape. An important distinction for
    this query is that distance from the boundary (for a given point) is not
    defined as the shortest distance to the boundary, but simultaneously as all
    surrounding distances. Axially symmetric objects (RZ-meshes) are treated as
    3D meshes and length scales are calculated over the revolved, 3D object.
    The implementation employs a statistical approach, with the casting of
    uniform density, random lines. The result of this query is a curve, which
    is outputted as a separate file. This curve contains the amount of mass as
    a function of length scale. Integrating the curve between P0 and P1 will
    give the total mass at distance between P0 and P1 (given the interpretation
    above). The name of the resulting file is printed to the **Query results**.

Eulerian
    The Eulerian query calculates the Eulerian number for the mesh that is used
    by the highlighted plot in the Plot list. The results are printed to the
    **Query results**.

Expected Value
    The Expected Value query calculates the integral of :math:`xf(x)dx` for
    some curve f(x). The curve should be highlighted in the **Plot list** and
    prints the result to the **Query results**. This query is intended for
    distribution functions.

Grid Information
    The Grid Information query prints information for each domain in a multi-
    domain mesh. The mesh type is printed as well as the mesh sizes. For
    structured meshes the size information contains the logical mesh dimensions
    (IJK sizes) and for unstructured meshes the size information contains the
    number of nodes and number of cells in the mesh. The query can optionally
    accept a *get_extents* parameter that will cause the spatial extents for
    each domain to be obtained. The query also accepts an optional
    *get_ghosttype* parameter that causes the ghost zone information for each
    domain to be obtained. Both the numerical value and list of or'd values for
    ghost values are obtained. All query outputs are printed to the
    **Queryresults**.

Integrate
    The Integrate query calculates the area under the Curve plot that is
    highlighted in the Plot list and prints the result to the
    **Query results**.

Kurtosis
    The Kurtosis query calculates the kurtosis of a normalized distribution
    function. The normalized distribution function must be represented as a
    Curve plot in VisIt. Kurtosis measures the variability of a distribution by
    comparing the ratios of the fourth and second central moments. The results
    are print to the **Query results**.

L2Norm
    The L2Norm query calculates the L2Norm, or square of the integrated area,
    of a Curve plot. The Curve plot must be highlighted in the **Plot list**.
    The results are printed to the **Query results**.

L2Norm Between Curves
    The L2Norm query takes two Curve plots as input and calculates the L2Norm
    between the 2 curves. Both Curve plots must be highlighted in the
    **Plot list** or VisIt will issue an error message. The results are printed
    to the **Query results**.

Min
    The Min query calculates the minimum value for the variable used by the
    highlighted plot in the **Plot list** and prints the value and the logical
    and physical coordinates where the minimum value was found to the
    **Query results**.

Mass Distribution
    The Mass Distribution query calculates how much mass occurs at different
    length scales over a two or three dimensional object. Axially symmetric
    objects (RZ-meshes) are treated as 3D meshes and length scales are
    calculated over the revolved, 3D object. The implementation employs a
    statistical approach, with the casting of uniform density, random lines.
    The result of this query is a curve, which is outputted as a separate file.
    This curve contains the amount of mass as a function of length scale.
    Integrating the curve between P0 and P1 will give the total mass between
    length scale P0 and length scale P1. The name of the resulting file is
    printed to the **Query results**.

Max
    The Max query calculates the maximum value for the variable used by the
    highlighted plot in the Plot list and prints the value and the logical and
    physical coordinates where the maximum value was found to the
    **Query results**.

MinMax
    The MinMax query calculates the minimum and maximum values for the variable
    used by the highlighted plot in the Plot list and prints the values and
    their logical and physical coordinates in the **Query results**.

Moment of inertia
    This query will calculate the moment of inertia tensor for each cell in a
    three-dimensional dataset. The contribution of each cell is calculated
    assuming its mass all lies at the center of the cell. If the query is
    performed on a Pseudocolor plot, the plot's variable will be assumed to be
    density. If the query is performed on a plot such as a mesh plot or
    FilledBoundary plot, uniform density will be used. The results are printed
    to the **Query results**.

NodeCoords
    The NodeCoords query prints the node coordinates for the specified node and
    prints the values in the **Query results**.

NumNodes
    The NumNodes query prints the number of nodes for the mesh used by the
    highlighted plot in the **Plot list** to the **Query results**.

NumZones
    The NumZones query prints the number of zones for the mesh used by the
    highlighted plot in the **Plot list** to the **Query results**.

Revolved surface area 
    The Revolved surface area query revolves the mesh used by the highlighted
    plot in the **Plot list** about the X-axis and prints the plot's revolved
    surface area to the **Query results**.

Revolved volume
    The Revolved volume area query revolves the mesh used by the highlighted
    plot in the **Plot list** about the X-axis and print's the plot's volume
    to the **Query results**.

Skewness
    The Skewness query calculates the skewness of a normalized distribution
    function. The normalized distribution function must be represented as a
    Curve plot in VisIt. Skewness measures the symmetry of a distribution using
    its second and third central moments. The results are print to the
    **Query results**

Spatial Extents
    The Spatial Extents query calculates the original or actual spatial extents
    for the plot that is highlighted in the **Plot list**. Whether the original
    or actual extents are calculated is determined by setting the options in
    the **Query parameters** area. The spatial extents are printed to the
    **Query results** when the query has finished.

Spherical compactness factor
    This query attempts to measure how spherical a three dimensional shape is.
    The query first determines what the volume of a shape is. It then
    constructs a sphere that has that same volume. Finally, the query positions
    the sphere so that the maximum amount of the original shape is within the
    sphere. The query returns the percentage of the original shape that is
    contained within the sphere. The results are print to the
    **Query results**. VisIt can produce a Curve plot of this query with
    respect to time.

Time
    The Time query prints the time for the plot that is highlighted in the Plot
    list to the **Query results**.

Variable Sum
    The Variable Sum query adds up the variable values for all cells using the
    plot highlighted in the **Plot list** and prints the results to the
    **Query results**. VisIt can produce a Curve plot of this query with
    respect to time.

Connected Component Variable Sum
    Performs the same operation as *Variable Sum* query except individually for
    each *component* of a disconnected mesh. The query result is a list of
    values, one for each component.

Volume
    The Volume query calculates the volume of the mesh used by the plot
    highlighted in the **Plot list** and prints the value to the
    **Query results**. VisIt can use this query to produce a Curve plot of
    volume with respect to time.

Connected Component Volume
    Performs the same operation as *Volume* query except individually for each
    *component* of a disconnected mesh. The query result is a list of values,
    one for each component.

Watertight
    The Watertight query determines if a three-dimensional surface mesh, of the
    plot highlighted in the **Plot list**, is "watertight", meaning that it is
    a closed volume with mesh connectivity such that every edge is incident to
    exactly two faces. This means that no edge can have a duplicate in the
    exact same position. The result of the query is printed in the
    **Query results**.

Weighted Variable Sum
    The Weighted Variable Sum query adds up the variable values, weighted by
    cell size (volume in 3D, area in 2D, length in 1D), for all cells using the
    plot highlighted in the **Plot list** and prints the results to the
    **Query results**. VisIt can produce a Curve plot of this query with respect
    to time.

Connected Component Weighted Variable Sum
    Performs the same operation as *Weighted Variable Sum* query except
    individually for each *component* of a disconnected mesh. The query
    result is a list of values, one for each component.

XRay Image
    See :ref:`XRayQuery <XRayQuery>`.

ZoneCenter
    The ZoneCenter query calculates the zone center for a certain cell in the
    database used by the highlighted plot in the Plot list. The cell center is
    printed to the **Query results** and the **Pick Window**.

Point Queries
"""""""""""""

Pick
    In general, the Pick query allows users to query a single zone or node at
    a user specified location in the dataset. There are several options for
    determining how this zone or node is chosen:

    1. **Pick using coordinates**
    2. **Pick using domain and element id**
    3. **Pick using unique element label**

    It's important to make sure that the plot you wish to query is highlighted
    in the **Plot list**. Information from your picked element, when available,
    will appear in both the **Pick Window** and the **Query results** window.
    If querying a 3D dataset, the queried element need not be on the surface
    of the mesh.

    The Pick query also provides the option to generate a curve with respect
    to time, allowing the user to set the start time, stop time, and stride.
    **Note on performance**: when generating a curve over time, users have
    the option to preserve either the picked *coordinate* or the picked *element*.
    While each of these choices will produce very different results, it's worth
    keeping in mind that preserving the picked *element* will be substantially
    faster than preserving the picked *coordinate* when working with datasets
    with large numbers of time steps.

TrajectoryByNode and TrajectoryByZone
    The TrajectoryByNode and TrajectoryByZone queries first perform a Pick
    using domain and element id on their respective elements, and they then generate
    a curve *plotting one variable with respect to another*.
    You'll notice that, next to the **Variables** parameter, there is a text box containing
    default variables **var_for_x** and **var_for_y**. Replace these defaults with your
    desired variables for the query, and the resulting curve will plot your replacement
    for **var_for_x** with respect to **var_for_y**.


Line Queries
""""""""""""

Lineout
    The Lineout query creates a new instance of the highlighted plot in the
    **Plot list**, applies a Lineout operator, and copies the plot to another
    vis window. The properties of the Lineout operator such as the start and
    end points are set using the controls in the **Query parameters area** of
    the **Query Window**. Creating Lineouts in this manner instead of using
    VisIt's interactive lineout allows you to create 1D Curve plots from 3D
    databases.  

Executing a query
~~~~~~~~~~~~~~~~~

VisIt has many queries from which to choose. You can choose the type of query
to execute by clicking on the name of the query in the **Queries list**. The
**Queries list** usually displays the names of all of the queries that VisIt
knows how to execute. If you instead want to view a subset of the queries,
grouped by function, you can make a selection from the **Display as** combo
box. Once you have clicked on a query in the **Query list**, the
**Query parameters** area updates to show the controls that you need to edit
the parameters for the query. In the case of a point query like Pick, the
only parameters you need to specify are the 3D point where VisIt will extract
values and the names of the variables that you want to examine. Once you
specify the query parameters, click the **Query** button to tell VisIt to
process the query. Once VisIt has fulfilled your request, the query results
are displayed in the **Query results** at the bottom of the **Query Window**.

Querying over time
~~~~~~~~~~~~~~~~~~

Many of VisIt's queries can be executed for every time state in the database
used by the queried plot. The results from a query over time is a Curve plot
that plots the query results with respect to time. The **Query parameters**
area contains a **Time Curve** button when the selected query can be plotted
over time. Clicking the **Time Curve** button executes the selected query
for each time state in the database used by the plot highlighted in the
**Plot list**. VisIt then creates a new Curve plot in a new vis window and
uses the query results versus time as the curve data.

.. _queryovertimewindow:

.. figure:: images/queryovertimewindow.png

   Query Over Time Window

By default, querying over time will force VisIt to execute the selected
query on every time state in the relevant database. If you want to restrict
the number of time states used when querying over time or if you want to
set some general options that also affect how time curves are created, you
can set additional options in the **Query Over Time Window** (see
:numref:`Figure %s <queryovertimewindow>`). If you want to open the
**Query Over Time Window**, click on the **Query over time** option in the
**Controls** menu in VisIt's **Main Window**.

Querying over a time range
""""""""""""""""""""""""""

You can restrict the range of time states that are considered when VisIt
is performing a query over time if you specify a start or end time state
in the **Query Over Time Window**. To set a starting time state, click the
**Starting timestep** check box and enter a new time state into the adjacent
text field. To set an ending time state, click the **Ending timestep**
check box and enter a new ending time state into the adjacent text field.

In addition to setting the starting and ending time states, you can also
specify a stride so VisIt can skip frames in the middle and consider every
Nth frame instead of every frame. If you want to specify a stride, enter
a new stride into the **Stride** text field in the **Query Over Time Window**
and click the **Apply** button.

Setting the axis title
""""""""""""""""""""""

When VisIt creates a new Curve plot, after having calculated a query over
time, the horizontal axis label is labeled with the database cycles. If you
prefer to think about time in terms of time state or simulation time then
you can change the axis label by clicking one of the following radio buttons
in the **Query Over Time Window** : **Cycle**, **Time**, **Timestep**.

Setting the time curve's destination window
"""""""""""""""""""""""""""""""""""""""""""

When VisIt creates a Curve plot using the results of a query over time,
the Curve plot is placed in a vis window designated for Curve plots. If
there is no vis window into which the Curve plot can be added, VisIt
creates a new vis window to contain the Curve plot. If you want VisIt to
always place the new Curve plot in a specific window, turn off the
**Use 1st unused window or create new one** check box and enter a new
window number into the **Window#** text field. After setting these
options, subsequent Curve plots created by querying over time will be
added to the specified vis window.


Python Queries
~~~~~~~~~~~~~~~~~~~~

Python Queries allow you to use a Python script to define a custom query.
You can use the Python Query tab in the Query Window to create a Python query:

.. _querywindowpython:

.. figure:: images/querywindow_python.png

   Python Query Editor


In your query you can access and process Python wrapped versions of the VTK
objects that represent your data.
To demonstrate this, here is an example Python Query that computes the
average of a zonal (or cell-centered) field:

::
  
  # simple cell average query
  class CellAvgQuery(SimplePythonQuery):
      def __init__(self):
          SimplePythonQuery.__init__(self)
          self.name = "CellAvgQuery"
          self.description = "Calculate the average cell value."
      def pre_execute(self):
          # init vars used to compute the average
          self.total_ncells = 0
          self.total_sum    = 0.0
      def execute_chunk(self,ds_in,domain_id):
          # sum over cell data array passed to query args
          ncells = ds_in.GetNumberOfCells()
          self.total_ncells += ncells
          cell_data = ds_in.GetCellData().GetArray(self.input_var_names[0])
          for i in range(ncells):
              self.total_sum += cell_data.GetTuple1(i)
      def post_execute(self):
          # calculate average and set results
          res_val = mpicom.sum(self.total_sum) / mpicom.sum(self.total_ncells)
          res_txt = "The average value = " + self.float_format
          res_txt = res_txt % res_val
          self.set_result_text(res_txt)
          self.set_result_value(res_val)
  
  py_filter = CellAvgQuery


This example is from our `pyavt examples. <https://github.com/visit-dav/visit/blob/develop/src/visitpy/pyavt/examples/py_query.py>`_

This example inherits from `SimplePythonQuery`. The base classes of VisIt_'s
Python Filters are defined in `the  pyavt module. <https://github.com/visit-dav/visit/blob/develop/src/visitpy/pyavt/py_src/filters.py>`_

You can select the variables passed to your Query using the Python Query
variable list:

.. _querywindowpythonvars:

.. figure:: images/querywindow_python_vars.png

   Python Query Variable list

In the script, your class needs to implement four methods:

Constructor : 
   Called to initialize the Python Query Filter object. Use this to call
   the base class constructor and provide a name and description of 
   your custom query.

Pre Execute : ``pre_execute(self)``
   This method is called on all MPI tasks before any domains have been processed.
   Use this to initialize any state needed before parallel execution. 
   In this example we initialize variables used to hold the total field value sum
   and the total number of cells.

Execute Chunk : ``execute_chunk(self,ds_in,domain_id)``
   This method is called to process each domain. When VisIt_ runs with MPI,
   `execute_chunk()` will be called in parallel across MPI tasks. 
   ``ds_in`` is a Python-wrapped VTK object and ``domain_id`` provides 
   the domain id of ``ds_in``. 
   In this example, for each domain we get the field value array and update
   the aggregate sum and the total number of cells.

Post Execute : ``post_execute(self)`` 
   This method is called on all MPI tasks after all domains have been processed. 
   Use this to finalize results after parallel execution. 
   In this example, we use MPI to combine the local results across MPI tasks.

The final aspect required is to bind your new Python Query Filter class to `py_filter`,
this is the name VisIt_ uses to connect your Python script to the
Python Filter Runtime in the engine.

When you run your Python Query, results are presented like any other Query: Displayed in the Query window and can be accessed via VisIt_'s Query output CLI functions.





