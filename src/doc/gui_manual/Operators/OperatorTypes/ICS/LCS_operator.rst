.. _LCS operator:

Lagrangian Coherent Structure (LCS) operator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The LCS operator utilizes Lyapunov Exponents based on the Cauchy-Green Tensor
to highlight Lagrangian Coherent Structures in vector fields. When performing a
Finite Time Lyapunov Exponent (FTLE) calculation, the time can be specified as
one would for a traditional FTLE, and the resulting value will be based on the
maximal Eigen value.

However, when performing the calculation with Finite Space Lyapunov Exponents
(FSLE), instead of assuming a uniform mesh discretization and specifying the
dispersion distance, we specify a dispersion factor. In a traditional FTLE,
this is the dispersion distance divided by the initial distance. In the
equivalent definition, the dispersion distance is the maximal Eigen value. Thus
when the maximal Eigen value is greater than the specified dispersion factor,
then the exponent is calculated.

More details can be found :download:`in this paper <pdfs/peikert2014fsle.pdf>`.

Source
^^^^^^

The set of points that seed the integral curves that reveal the Lagrangian
Coherent Structures. In addition to the :ref:`common ICS source` attributes
common to all ICS operators, the LCS operator supports the following attributes:

Source types
""""""""""""

The source type controls how the seeds for curves are created. The user can
seed the integral curves using the native mesh or define a rectilinear grid.
The nodes of the mesh are the seed points.

Auxilary Grid
"""""""""""""

When calculating the Jacobian for the Cauchy-Green tensor, one can use the
neighboring points from the native mesh or one can specify an auxiliary grid
that allows for the detection of finer features but at greater computational
expense. Using an auxiliary grid is advantageous because it is independent of
the native mesh, so it gives more accurate results for higher order elements.
For simulation flows, using the auxiliary grid for eigenvalue calculations
gives better results.

Integration
^^^^^^^^^^^

Specify settings for numerical integrators. In addition to the
:ref:`common ICS integration` attributes common to all ICS operators, the LCS
operator supports the following attributes:

Integration Direction
"""""""""""""""""""""

Sets the integration direction through time: either forward or backward.

Appearance
^^^^^^^^^^

The appearance tab specifies how the LCS's will be rendered. In addition to the
:ref:`common ICS appearance` attributes common to all ICS operators, the LCS
operator supports the following attributes.

Seed Generation
"""""""""""""""

Filter the number of seeds generated from the mesh (either native or
rectilinear). There are various self-descriptive filtering options.

Advanced
^^^^^^^^

In addition to the :ref:`common ICS advanced` attributes common to all ICS
operators, the LCS Operator supports the following attributes:

Warnings
""""""""

Issue warning if the advection limit is not reached
    If the maximum time or distance is not reached, issue a warning.

Issue warning if the spatial boundary is reached
    If the integral curve reaches the spatial domain boundary, issue a warning.
