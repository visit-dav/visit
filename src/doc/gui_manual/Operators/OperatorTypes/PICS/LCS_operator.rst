.. _LCS operator:

Lagrangian Coherent Structure operator
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Integral Curve system allows for the computation of LCS structures using a variety techniques developed by George Haller and his group at ETH Zürich. Haller proposed that the ridges of FTLE are indicators of hyperbolic LCS (repelling and attracting). More recent research by Haller's group has focused a mathematical description of LCS In two dimensional flows, hyperbolic LCSs turn out to be stationary curves of the averaged material shear while elliptic LCSs are stationary curves of the averaged strain. The LCS operator coupled with the Integral Curve (Hyperbolic LCS) and Limit Cycle (Elliptical LCS) operators provide a mechanism calculate these structures and have been developed in cooperation between the VisIt PICS team and the Haller Group.

For complete details on computing elliptical LCS (closed orbits) and hyperbolic LCS (stretch and shrink lines) see the following paper: K. Onu, F. Huhn, & G. Haller, LCS Tool: A Computational platform for Lagrangian coherent structures, J. of Computational Science, 7 (2015) 26-36, http://georgehaller.com/reprints/lcstool.pdf

The Lagrangian Coherent Structures operator utilizes Lyapunov Exponents based on the Cauchy-Green Tensor to highlight Lagrangian Coherent Structures in vector fields. The Cauchy-Green tensor is used as an equivalent definition because it is independent of the mesh discretization. As such, when performing a FTLE calculation the time can be specified as one would for a traditional FTLE with the resulting value being based on the maximal Eigen value.

However, when performing a FSLE instead of assuming a uniform mesh discretization and specifying the dispersion distance one would specify a dispersion factor which in a traditional FTLE would be the dispersion distance divided by the initial distance. In the equivalent definition the dispersion distance would be the maximal Eigen value. Thus when the maximal Eigen value is greater than the specified dispersion factor the exponent would be calculated.

For more details see the following paper: A Comparison of Finite-Time and Finite-Size Lyapunov Exponents by Ronald Peikert, Armin Pobitzer, Filip Sadlo and Benjamin Schindler, http://www.scivis.ethz.ch/publications/pdf/2014/peikert2014comparison.pdf 

Source
""""""

The user can seed the integral curves using the native mesh or define a their own rectilinear grid. Source types: 

* Native Mesh – (default) create seeds at the nodes of the native mesh. 
* Rectilinear Grid – create seeds at the nodes of a user defined rectilinear grid. 

   * Resolution - Three cartesian values separated with a space defining the number of samples 
   * Data start extent 

      * Full - Use the native mesh's real world data extent for defining the beginning of the grid 
      * Subset - User defined real world data extents for defining the beginning of the grid 

   * Data end extent 

      * Full - Use the native mesh's real world data extent for defining the beginning of the grid 
      * Subset - User defined real world data extents for defining the end of the grid 

      
Auxilary Grid - When calculating the Jacobean for the Cauchy-Green tensor one can use the neighboring points from the native mesh or one can specify an auxiliary grid which allows for the detection of finer features but at a greater computational expense. Using an auxiliary grid is advantageous because it is independent of the native mesh. Which gives more accurate results for higher order elements. Haller et al. found that for analytic flows (double gyre flow, ABC flow), the eigenvalues can be calculated from the native mesh. For simulation flows, using the auxiliary grid for eigenvalue calculations give better results.

* None - no auxiliary grid
* 2D - use a 2D auxiliary grid (i.e. X and Y directions). Can be used with 2D or 3D fields.
* 3D - use a 3D auxiliary grid 
* Spacing - the differential spacing between the node and auxiliary grid points. 

Field - see common data



Integration
"""""""""""

Integration - See common data

Termination:

Operation type - The user can visualization LCS via traditional techniques using FTLE or FSLE or via experimental techniques based on absolute dispersion; FLLE, the integration time, arc length, or average distance from the seed.

* Lyapunov Exponent (default), Eigen Value, Eigen Vector 
* Tensor - One may select the left or right Cauchy Green:
* Clamp exponent values - clamp the log values to be positive. 
* Eigen Value (Lyapunov Exponent and Eigen Value)

   * Smallest - also produces seed points for
   * Intermediate (3D Only)
   * Largest 

* Eigen Value (Eigen Vector)

   * Smallest - used to compute repelling hyperbolic LCSs (shrink lines) using seeds points (maximal regional values) from the largest Eigen value.
   * Intermediate (3D Only)
   * Largest - used to compute attracting hyperbolic LCSs (stretch lines) using seeds points (minimal regional values) from the smallest Eigen value.
   * Shear Positive - experimental
   * Shear Negative - experimental
   * Lambda Shear Positive - used to compute elliptic LCSs as limit cycles.
   * Lambda Shear Negative - used to compute elliptic LCSs as limit cycles.

* Integration Time - calculate the total integration time using that value for immediate display or a gradient calculation.
* Arc Length - calculate the total arc length using that value for immediate display or a gradient calculation.
* Average distance from seed - calculate the average distance from the seed at each integration step using that value for immediate display or a gradient calculation. 

   * Secondary operation for the integration time, arc length, or average distance from the seed.

      * Base value - return the value calculated by the operation.
      * Gradient - return the log of the gradient of the value calculated by the operation. 

   * Clamping (Gradient only)

      * Clamp exponent values - clamp the log values to be positive. 

Termination - terminate the integration based on the time or distance advected or maximum number of steps. Depending on the operator these options may not all be available. Further they may be either "and" or "or". 

* Limit maximum time elapsed by particles (FTLE) - (Default state off, Default value 10), Finite Time Lyapunov Exponent, this is a measure of relative dispersion based on the advection time.
* Limit maximum advection distance (FLLE) - (Default state off, Default value 10), Finite Length Lyapunov Exponent, this is a measure of absolute dispersion.
* Limit maximum size (FSLE) - (Default state off, Default value 10), Finite Size Lyapunov Exponent, this is a measure of relative dispersion based on the size (distance) between neighboring seeds.
* Maximum number of steps (default 1000) - the maximum number of integration steps that will be allowed regardless or the time or distance. 


Appearance
""""""""""

Seed Generation:

* Threshold limit - exclude seeds that are not within the threshold
* Radial limit - exclude seeds that are closer to other seeds than the radial limit.
* Boundary limit - exclude seeds closer than the specified boundary limit.
* Maximum number of seeds - the maximum number of seeds generated 

Streamlines and pathlines - see common

Advanced
""""""""

See common

Example
"""""""


















