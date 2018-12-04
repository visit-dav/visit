.. _DeferExpression operator:

DeferExpression operator
~~~~~~~~~~~~~~~~~~~~~~~~

The DeferExpression operator is a special-purpose operator that defers 
expression execution until later in VisIt's pipeline execution cycle. This 
means that instead of expression execution taking place before any operators 
are applied, expression execution can instead take place after operators have 
been applied.

Plotting surface normals
""""""""""""""""""""""""

VisIt can use the DeferExpression operator in conjunction with the 
:ref:`ExternalSurface operator` and the surface_normal expression to plot 
surface normals for your plot geometry. To plot surface normals, first create a 
vector expression using the surface_normal expression , which takes the name of 
your plot's mesh as an input argument. Once you have done that, you can create 
a Vector plot of the new expression. Be sure to apply the 
:ref:`ExternalSurface operator` first to convert the plot's 2D cells or 3D 
cells into polygonal geometry that can be used in the surface_normal expression.
Finally, apply the DeferExpression operator and set its variable to your new 
vector expression. This will ensure that the surface_normal expression is not 
evaluated until after the ExternalSurface operator has been applied.

.. _defer:

.. figure:: images/defer.png
   :width: 60%
   :align: center

   DeferExpression operator example

