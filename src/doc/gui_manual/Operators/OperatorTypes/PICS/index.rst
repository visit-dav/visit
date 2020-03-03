Parallel Integral Curve System
------------------------------

Within the VisIt infrastructure is the ability to generate integral curves; streamlines (fieldlines) and pathlines. The generation of the curves form the basis of VisIt's Parallel Integral Curve System (PICS), made up of the :ref:`Integral Curve operator`, the :ref:`LCS operator`, the :ref:`Limit Cycle operator`, and the :ref:`Poincare operator`. Much of the underlying infrastructure and interface is the same for each operator. The user selects a series of seed locations where curves are generated, which are then visualized and analyzed.

.. toctree::
    :maxdepth: 1

    IntegralCurve_operator
    LCS_operator
    LimitCycle_operator
    Poincare_operator

Parameters
^^^^^^^^^^

Common to all PICS operators is a four tab GUI: Source, Integration, Appearance, and Advanced. These tabs contain many functions that are common across all three operators. In what follows is a description of those common features.

Source
""""""

Source - See the specific operator for details

Field - Sets the field type so that the native elements are used when interpolating the vector field(s). Options are:

* Default - use VisIt's native VTK mesh structure performing linear interpolation on the vector field.s
* Flash - Evaluates the velocity field via the Lorentz force.

   * Constant - Factor for multiplying the velocity.
   * Velocity - When combined with Leap-Frog integration sets the initial velocity used in the integration.s
   
* M3D-C1 2D - Evaluates the 3D magnetic field via a 2D poloidal 6th order polynomial.

   * Constant - Factor for multiplying the perturbed part of the field.
   
* M3D-C1 3D - Evaluates the 3D magnetic field via a 2D poloidal 6th order polynomial and 1D toroidal 4th order Bezier sspline.

* Nek 5000 - Evaluates the 3D vector field using Nek 5000 spectral elements.
* Nektar++ - Evaluates the 3D vector field using Nektar++ spectral elements.
* NIMROD - Evaluates the 3D magnetic field via a 2D poloidal 3rd order Lagrange-type element and 1D toroidal finite Fourier series element (future implementation).

Integration
"""""""""""

Integration direction - Set the integration direction (:ref:`Integral Curve operator` and :ref:`LCS operator` only). Options are:

* Forward - integrate forward in time.
* Backward - integrate backwards in time. 
* Both - integrate both forward and backwards in time producing two integral curves. 
* Forward Directionless - integrate forward in time assuming a directionless vector field. 
* Backward Directionless - integrate backwards in time assuming a directionless vector field. 
* Both Directionless - integrate both forward and backwards in time assuming a directionless vector field producing two integral curves. 

Eigen vectors are an example of a directionless vector field. To integrate using a directionless field requires that any orientation discontinuity be corrected prior to linear interpolation. That is all vectors must be rotated to match the orientation of the trajectory. The PICS code will do this processing for standard fields (e.g non-higher order elements).

Integrator - Sets the integration type and their associate parameters.

* Forward Euler - the basic Euler integration with a fixed step length, not recommended.

   * Step Length - (Default 0.1) set size to a value. 
   
* Leap Frog - simple method for numerical integration where both the position and velocity are updated using a fixed step length.

   * Step Length - (Default 0.1) set size to a value. 
   
* Dormand-Prince (Runge-Kutta) - is a Runge-Kutta based scheme with adaptive step size control or can it can be clamped to a maximum value if the Limit maximum time step box is checked.

   * Limit maximum time step box - (Default false) clamp the adaptive step size to a maximum value.
   * Two tolerances are available - The adaptive step is set such that the maximum error at each step is less than max(Absolute_tolerance, Relative_tolerance * Velocity). The tolerance can be absolute or relative to the bounding box.
   
* Adams-Bashforth (Multi-step) - A linear multistep integration using a fixed step length.

   * Step Length - (Default 0.1) set size to a value.
   * Two tolerances are available - The adaptive step is set such that the maximum error at each step is less than max(Absolute_tolerance, Relative_tolerance * Velocity). The tolerance can be absolute or relative to the bounding box.
   
* Runge-Kutta - traditional 4th order Runge-Kutta with a fixed step length.

   * Step Length - (Default 0.1) set size to a value 
   
* M3D-C1 2D Integrator (M3D-C1 2D data only) - uses Newton's method under the hood.

   * Step Length - (Default 0.1) set size to a value. 
   * Two tolerances are available - The adaptive step is set such that the maximum error at each step is less than max(Absolute_tolerance, Relative_tolerance * Velocity). The tolerance can be absolute or relative to the bounding box.

Termination - See specific operator for details

Appearance
""""""""""

The user may select the integral curve to be based on an instantaneous or time-varrying vector field producing streamlines and path lines respectively.

* Streamline - (Default) use an instantaneous vector field for the integration. 
* Pathline - use a time-varrying vector field for the integration.

   * Override starting time (Default: off) - instead of starting with the current time step utilize another time for the start time.
   
      * Time (Default: 0) - new starting time.
      
   * Interpolation over time - indicated whether the mesh changes over time. 
   
      * Mesh is static over time - typically the case but can not be assumed. 
      * Mesh is varies over time (Default).

Advanced
""""""""

Parallel integration - The user may select one of four different parallelization options when integrating curves in parallel:

* Parallelize over curves - distribute the curves between the processors. 

   * Domain cache size - number of blocks to hold in memory for level of details (LOD). 
   
* Parallelize over domains - distribute the domains between the processors. 

   * Communication threshold - number of integral curve to process before communication occurs. 
   
* Parallelize over curves and domains - distribute the curves and domains between the processors.
 
   * Communication threshold - number of integral curve to process before communication occurs. 
   * Domain cache size - number of blocks to hold in memory for level of details (LOD). 
   * Work group size - master slave also (currently disabled). 
   
* Have VisIt select the best algorithm (Default)

Warnings - The user can be alerted to various conditions that may occur during the integration 

* Issue warning if the advection limit is not reached (Default on) - if the maximum time or distance is not reached, issue a warning. 
* Issue warning if the spatial boundary is reached (Default on) - if the integral curve reaches the spatial domain boundary, issue a warning. 
* Issue warning when the maximum number of steps is reached (Default on) - the maximum number of steps limits run-a-way integration. 
* Issue warning when a step size underflow is detected (Default on) - if the step size goes to zero, issue a warning. 
* Issue warning when stiffness is detected (Default on) - Stiffness refers to one vector component being so much larger than another that tolerances can't be met. 
* Issue warning when a curve doesn't terminate at a critical point (Default on) - the curve circles around the critical point without stopping. 

   * Speed cutoff for critical points (Default 0.001)














