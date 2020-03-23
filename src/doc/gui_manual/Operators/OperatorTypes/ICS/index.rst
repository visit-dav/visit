Integral Curve System
=====================

Within the VisIt infrastructure is the ability to generate integral curves:
streamlines and pathlines. An integral curve is a curve that begins at a seed
location and is tangent at every point in a vector field. An integral curve
is computed by numerical integration of the seed location through the vector
field. The image below shows fieldlines through the magnetic field of a
core-collapse supernova simulation from the GenASiS code.

IMAGE HERE

The generation of the curves form the basis of VisIt's Integral Curve
System (ICS), made up of the :ref:`Integral Curve operator`, the :ref:`LCS
operator`, the :ref:`Limit Cycle operator`, and the :ref:`Poincare operator`.
Much of the underlying infrastructure and interface is the same for each
operator. The user selects a series of seed locations where curves are
generated, which are then visualized and analyzed.

The ICS allows for the computation of Lagrangian Coherent Structures (LCS)
using a variety techniques developed by George Haller and his group at ETH
Zürich. Haller proposed that the ridges of Finite Time Lyapunov Exponents
(FTLE) are indicators of hyperbolic LCS (repelling and attracting). In two
dimensional flows, hyperbolic LCS's turn out to be stationary curves of the
averaged material shear while elliptic LCS's are stationary curves of the
averaged strain. The LCS operator coupled with the Integral Curve (Hyperbolic
LCS) and Limit Cycle (Elliptical LCS) operators provide a mechanism to
calculate these structures and have been developed in cooperation between the
VisIt ICS team and the Haller Group.

INCLUDE IMAGE of elliptical and hyperbolic LCS

For complete details on computing elliptical LCS (closed orbits) and hyperbolic
LCS (stretch and shrink lines) see the following paper: `K. Onu, F. Huhn, & G.
Haller, LCS Tool: A Computational platform for Lagrangian coherent structures,
J. of Computational Science, 7 (2015) 26-36
<http://georgehaller.com/reprints/lcstool.pdf>`_

.. toctree::
    :maxdepth: 1

    IntegralCurve_operator
    LCS_operator
    LimitCycle_operator
    Poincare_operator

Parameters
----------

Common to all ICS operators is a four tab GUI: Source, Integration,
Appearance, and Advanced (the Poincare operator also has an Analysis tab).
These tabs contain many functions that are common across all four operators.
The following is a description of those common features. Please note: on a mac
system, the contents of the Source and Integration tabs are combined into a
single Integration tab.


Source
^^^^^^

The set of points to seed the integral curves in the operator. See specific
operator for details.

Field
"""""

Sets the field type so that the native elements are used when interpolating the
vector fields. Each operator provides the following options:

Default
    Use VisIt's native VTK mesh structure performing linear interpolation on
    the vector field.
    
Flash
    Evaluates the velocity field via the Lorentz force. Parameters are:

    * Constant - A constant multiple applied to the velocity.
    * Velocity - When combined with Leap-Frog integration, this sets the
    initial velocity used in the integration.
   
M3D-C1 2D
    Evaluates the 3D magnetic field via a 2D poloidal 6th order polynomial.
    Parameters are:

    * Constant - A constant multiple applied to the perturbed part of the
      field.
   
M3D-C1 3D
    Evaluates the 3D magnetic field via a 2D poloidal 6th order polynomial and
    1D toroidal 4th order Bezier spline.
    
Nek5000
    Evaluates the 3D vector field using `Nek5000 <https://nek5000.mcs.anl.gov/>`_
    spectral elements.
    
Nektar++
    Evaluates the 3D vector field using `Nektar++ <https://www.nektar.info/>`_
    spectral elements.
        
NIMROD
    Evaluates the 3D magnetic field via a 2D poloidal 3rd order Lagrange-type
    element and 1D toroidal finite Fourier series element (future
    implementation).

Integration
^^^^^^^^^^^

Specify settings for the numerical integrator. See each operator for varied
settings.

Integrator
""""""""""

Sets the integration scheme. Options are:

Forward Euler
    The basic Euler integration with a fixed step length. This integrator is
    not recommended.

Leap Frog
    Simple method for numerical integration where both the position and
    velocity are updated using a fixed step length.

Runge-Kutta-Dormand-Prince (RKDP)
    A Runge-Kutta based scheme with adaptive step size control.
    
Adams-Bashforth (Multi-step)
    A linear multistep integration using a fixed step length.
    
Runge-Kutta
    Traditional 4th order Runge-Kutta with a fixed step length.
    
M3D-C1 2D Integrator (M3D-C1 2D fields only)
    Uses Newton's method under the hood.
       
Step Length
"""""""""""

The length of the integration step. Most integrators use a fixed step length.
Runge-Kutta-Dormand-Prince uses adaptive step size, which can be clipped by
this value.

Tolerances
""""""""""

RKDP, Adams-Bashforth, and MD3-C1 make use of the tolerance options.
    
RKDP
    Two tolerances are available - The adaptive step is set such that the
    maximum error at each step is less than the maximum between the absolute
    tolerance and the relative tolerance times the value of the vector field at
    the current point. The absolute tolerance can be truly absolute or relative
    to the bounding box.

Adams-Bashforth
    NOT REALLY CLEAR WHAT THIS IS BECAUSE AB USES A FIXED-STEP SIZE. MAYBE THIS
    IS A CLIP?

MD3-C1
    NOT REALLY CLEAR WHAT THIS IS BECAUSE AB USES A FIXED-STEP SIZE. MAYBE THIS
    IS A CLIP?

Termination
"""""""""""

The criteria for terminating the integration. See specific operator for
details.

Appearance
^^^^^^^^^^

Specify appearance settings for the curves. See specific operator for details
on most settings.

Streamlines vs Pathlines
""""""""""""""""""""""""

The user may select the integral curve to be based on an instantaneous or
time-varying vector field producing streamlines or pathlines, respectively. A
streamline is path rendered by an integrator that uses the same vector field for
the entire integration. A pathline uses the vector field that is in-step with
the integrator, so as the integrator steps through time, it uses the
vector field at that new time step.
    
Pathline Options:
    
    * Override starting time - instead of starting with the current time step,
    utilize another time for the start time.
   
       * Time - new starting time.
      
    * Interpolation over time - indicated whether the mesh changes over time. 
   
       * Mesh is static over time - typically the case, but cannot be assumed.
         Confirm this before selecting.
       * Mesh varies over time (Default).

Advanced
^^^^^^^^

Parallel integration
""""""""""""""""""""

The user may select one of four different parallelization options when
integrating curves in parallel:

Parallelize over curves
    Distribute the curves between the processors. 

    * Domain cache size - number of blocks to hold in memory for level of
      details (LOD). 
   
Parallelize over domains
    Distribute the domains between the processors. 

    * Communication threshold - number of integral curve to process before
      communication occurs. 
   
Parallelize over curves and domains
    Distribute the curves and domains between the processors.
 
    * Communication threshold - number of integral curve to process before
      communication occurs. 
    * Domain cache size - number of blocks to hold in memory for level of
      details (LOD). 
    * Work group size - master slave also (currently disabled). 
   
Have VisIt select the best algorithm
    VisIt automagically selects the best parallelization algorithm.

Warnings
""""""""
Alert for various conditions that may occur during the integration.

Issue warning when the maximum number of steps is reached
    The maximum number of steps limits run-a-way integration.

Issue warning when a step size underflow is detected
    If the step size goes to zero, issue a warning.

Issue warning when stiffness is detected
    Stiffness refers to one vector component being so much larger than another
    that tolerances can't be met. 

Issue warning when a curve doesn't terminate at a critical point
    The curve circles around the critical point without converging.