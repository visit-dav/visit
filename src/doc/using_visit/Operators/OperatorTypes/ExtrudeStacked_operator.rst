.. _ExtrudeStacked operator:

ExtrudeStacked operator
~~~~~~~~~~~~~~~~~~~~~~~

The ExtrudeStacked operator uses one or more scalar fields on a 2D
mesh to extrude each node in the input mesh, resulting in a 3D
mesh. The ExtrudeStacked operator can also extrude plots whose input
data was produced from higher dimensional data that has been
sliced. The resulting extruded field can be colored by the extrusion height
(see: :numref:`Figure %s <extrude>`) or an index.

.. _extrude:

.. figure:: images/extrude.png

   ExtrudeStacked operator example: A simple structured grid extruded
   by a scalar cell value.

Using the ExtrudeStacked operator
"""""""""""""""""""""""""""""""""

The ExtrudeStacked operator can be used to create 3D height field
plots by applying the operator to a plot that accepts scalar
values. The ExtrudeStacked operator is quite flexible because it can
extrude by more than one variable and colored by the same variable.

The most useful feature of the ExtrudeStacked operator is its ability
to extrude a mesh using multiple arbitrary scalar variables. By
default, the ExtrudeStacked operator uses the plot's variable to
extrude the mesh. This only works when the plotted variable is a
scalar variable. When you apply the ExtrudeStacked operator to plots
that do not accept scalar variables, the ExtrudeStacked operator will
fail unless a specific scalar variable is choosen int the **Add
Variable** variable menu in the **ExtrudeStacked attributes window**
(see :numref:`Figure %s <extrudeWindow>`).

.. _extrudeWindow:

.. figure:: images/extrudeWindow.png

   ExtrudeStacked operator window

When a variable is added, it is added to the top of the stack and
given the next lragest index. That is the bottom of the stack with the
first variable (index 0). It is possible to reorder and delete
variables via the **Move up** or **Move down** and **Delete** buttons
in the **ExtrudeStacked attributes window**.

As the ExtrudeStacked operator uses a scalar variable to extrude all
of the points in the mesh, the ExtrudeStacked operator has a number of
controls related to scalar data. For example, the ExtrudeStacked
operator allows one to set the minimum and maximum values as well as
scale for each scalar variable. This allows one to eliminate data that
might otherwise cause the extruded plot to be stretched undesirably in
the Z direction. To set minimum and maximum values and scale for the
ExtrudeStacked operator, first select the varaible, the set the
**Min** or **Max** or **Scale** in the **ExtrudeStacked attributes
window**. The values will automatically be updated in the variable
table.

(see :numref:`Figure %s <extrudeScaled>`).

.. _extrudeScaled:

.. figure:: images/extrudeScaled.png

   ExtrudeStacked operator used to create a scaled plot using one
   variable. The left image used node based scalar values used to
   extrude the mesh, the right image scaled the scalar values by two.
   Both are displayed by the node based scalar values (see the next
   section).

It is possible to **Display by** the resulting field via the **Index**
of the varible in the stack or by the scalar value used to extrude the
mesh. If a cell based scalar value is used then the **Node Height**
and the **Cell Height** will be same. If a node based scalar value is
used then the **Node Height** will relfect the scalar value while the
**Cell Height** will be average saclar value for that cell.

(see: :numref:`Figure %s <extrudeStacked>`).

.. _extrudeStacked:

.. figure:: images/extrudeStacked.png

   ExtrudeStacked operator used to create a stacked plot using two
   variables. The left image is displayed by the cell based scalar
   values used to extrude the mesh, the right image is displayed by
   the variable index.
