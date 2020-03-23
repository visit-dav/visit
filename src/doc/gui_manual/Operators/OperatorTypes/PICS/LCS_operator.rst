.. _LCS operator:

Lagrangian Coherent Structure operator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The LCS operator utilizes Lyapunov Exponents based on the Cauchy-Green Tensor
to highlight LCS's in vector fields. The Cauchy-Green tensor is used as an
equivalent definition because it is independent of the mesh discretization. As
such, when performing an FTLE calculation, the time can be specified as one
would for a traditional FTLE, and the resulting value will be based on the
maximal Eigen value.

However, when performing the calculate with Finite Space Lyapunov Exponents
(FSLE), instead of assuming a uniform mesh discretization and specifying the
dispersion distance, we specify a dispersion factor which in a traditional FTLE
is the dispersion distance divided by the initial distance. In the equivalent
definition the dispersion distance is the maximal Eigen value. Thus when the
maximal Eigen value is greater than the specified dispersion factor, then the
exponent is calculated.

For more details see the following paper: `A Comparison of Finite-Time and
Finite-Size Lyapunov Exponents by Ronald Peikert, Armin Pobitzer, Filip Sadlo
and Benjamin Schindler,
<http://www.scivis.ethz.ch/publications/pdf/2014/peikert2014comparison.pdf>`_



Source
^^^^^^

Source types
""""""""""""

The user can seed the integral curves using the native mesh or define their own
rectilinear grid.
   
Native Mesh
    Create seeds at the nodes of the native mesh. 

Regular Grid
    Create seeds at the nodes of a user-defined rectilinear grid. Parameters
    are:

    * Resolution - Three cartesian values separated with a space defining the
      number of samples in each dimension.
    * Data extents - Specify the starting and ending extents, using either the
      native mesh's extents (Full) or user-defined extents (Subset).


Auxilary Grid
"""""""""""""

When calculating the Jacobian for the Cauchy-Green tensor, one can use the
neighboring points from the native mesh or one can specify an auxiliary grid
that allows for the detection of finer features but at greater computational
expense. Using an auxiliary grid is advantageous because it is independent of
the native mesh. It gives more accurate results for higher order elements.
Haller et al. found that for analytic flows (double gyre flow, ABC flow), the
eigenvalues can be calculated from the native mesh. For simulation flows, using
the auxiliary grid for eigenvalue calculations gives better results.

None
    No auxiliary grid

2D
    Use a 2D auxiliary grid (i.e. X and Y directions). Can be used with 2D or
    3D fields.

3D
    Use a 3D auxiliary grid 

Spacing
    The differential spacing between the node and auxiliary grid points. 


Field
"""""

See Field attributes that are common to all PICS operators.



Integration
^^^^^^^^^^^

Specify settings for numerical integrators. In addition to the Integration
attributes common to all PICS operators, the LCS operator supports the
following attributes.


Integration Direction
"""""""""""""""""""""

Sets the integration direction through time. Options are:
    
Forward
    Integrate forward in time.

Backward
    Integrate backward in time.
    
Integrator
""""""""""

See Integrator attributes that are common to all PICS operators.
    
Step Length
"""""""""""

See Step Length attributes that are common to all PICS operators.
    
Tolerances
""""""""""

See Tolerance attributes that are common to all PICS operators.


Appearance
^^^^^^^^^^

The appearance tab specifies how the LCS will be rendered. In addition to the
Appearance attributes common to all PICS operators, the LCS operator supports
the following attributes.


Seed Generation
"""""""""""""""

Threshold limit
    Exclude seeds that are not within the threshold

Radial limit
    Exclude seeds that are closer to other seeds than the radial limit.

Boundary limit
    Exclude seeds closer than the specified boundary limit.

Maximum number of seeds
    The maximum number of seeds generated 


Streamlines vs Pathlines
""""""""""""""""""""""""

See Streamlines vs Pathlines attributes that are common to all PICS operators.


Advanced
^^^^^^^^

The LCS Operator shares the same Parallel attributes as all the other PICS
operators.


Warnings
""""""""

In addition to the Warnings common to all PICS operators, the LCS Operator
supports the following warnings.

Issue warning if the advection limit is not reached
    If the maximum time or distance is not reached, issue a warning.

Issue warning if the spatial boundary is reached
    If the integral curve reaches the spatial domain boundary, issue a warning.


















