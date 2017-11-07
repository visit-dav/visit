.. _Resample operator:

Resample operator
~~~~~~~~~~~~~~~~~

The Resample operator extracts data from any input dataset in a uniform 
fashion, forming a new 2D or 3D rectilinear grid onto which the original 
dataset has been mapped. The Resample operator is useful in a variety of 
contexts such as downsampling a high resolution dataset (shown in
:numref:`Figure %s <resample>`), rendering Constructive Solid Geometry (CSG) 
meshes, or mapping multiple datasets into a common grid for comparison purposes.

.. _resample:

.. figure:: images/resample.png
   :width: 80%
   :align: center

   Resample operator example


Resampling onto a rectilinear grid
""""""""""""""""""""""""""""""""""

Resampling a high resolution dataset onto a rectilinear grid is the most common 
use case for the Resample operator. When a Resample operator is applied to a 
plot, the Resample operator clips out any data values that are not within the 
operator's bounding box. For the data that remains inside the bounding box, 
the operator samples it using the user-specified numbers of samples for the X, 
Y, and Z dimensions. The default for the Resample operator is to use the entire
extents of the dataset.  If you want to choose a smaller region, unselect the
**Resample Entire Extents** checkbox and enter new bounding box information.
The bounding box is specified by entering new start and end values for each 
dimension. For example, if you want to change the locations sampled in the X 
dimension then you could type new floating point values into the **Start X** 
and **End X** text fields. The same pattern applies to changing the locations 
sampled in the Y and Z dimensions. One difference between resampling 2D and 3D 
datasets is that 3D datasets must have the **3D resampling** check box enabled 
to ensure that VisIt uses the user-specified Z-extents and number of samples in 
Z.

Samples for which there was no data in the original input dataset are provided 
with a default value that you can change by typing a new floating point number 
into the **Value for uncovered regions** text field.

.. _resamplewindow:

.. figure:: images/resamplewindow.png
   :width: 50%
   :align: center
  
   Resample attributes window

Using Resample with CSG meshes
""""""""""""""""""""""""""""""

Constructive Solid Geometry (CSG) modeling is a method whereby complex models 
are built by adding and subtracting primitive objects such as spheres, cubes, 
cones, etc. When you plot a CSG mesh in VisIt, VisIt resamples the CSG mesh 
into discrete cells that can be processed as an unstructured mesh and plotted. 
The Resample operator can be used to tell VisIt the granularity at which the 
CSG mesh should be sampled, overriding the CSG mesh's default sampling. 
Naturally, higher numbers of samples in the Resample operator produce a more 
faithful representation of the original CSG mesh. 
:numref:`Figure %s <resamplecsg>`  depicts a CSG model that contains a disc 
within a smooth ring. Note that as the number of samples in the Resample 
operator increases, the model becomes smoother and jagged edges start to 
disappear.

.. _resamplecsg:

.. figure:: images/resamplecsg.png

  The Resample operator can be used to control the resolution of CSG meshes.
  Resolution is increased from left to right.

Resampling surfaces projected to 2D
"""""""""""""""""""""""""""""""""""

Sometimes is is useful to project complex surfaces into 2D and resample them 
onto a 2D mesh so queries and other analysis can be performed.

When you project a complex surface to 2D using the Project operator, all of a 
plot's geometry remains and its Z coordinates are set to zero. This results in 
some areas where the plot is essentially crushed on top of itself, as shown in
:numref:`Figure %s <resampleties>`. When resampling the plot onto a new 2D 
grid, these overlapping areas can be treated in three different ways. You can 
ensure that the top value is taken if you choose the random option by clicking 
on the **random** button in the **Resolve ties** button group. You can use a 
mask variable to decide ties by clicking on the **largest** or **smallest** 
buttons and by selecting an appropriate variable using the 
**Variable to resolve ties** menu.

.. _resampleties:

.. figure:: images/resampleties.png

  Using the Resample operator to create a 2D projection

When used in parallel, the resampled data is distributed across all processors.
This can be changed by unselecting the checkbox.

You can also force the output data to be cell centered by selecting the
**Make output cell centered** checkbox.
