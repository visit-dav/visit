.. _Limit Cycle operator:

Limit Cycle operator
~~~~~~~~~~~~~~~~~~~~

The Limit Cycle Operator allows one to search for closed material lines (limit
cycles) which are elliptic LCS's. To perform this search one defines a Poincaré
section where a series of integral curves are advected. In a Limit Cycle, most
integral curves will return to the Poincaré section at which point their
integration will be stopped. Some integral curves may not return to the
Poincaré section. To limit run-a-way integration, the user can control the
termination criteria.

For the integral curves that return to the Poincaré section a signed return
distance is calculated. Curves with a return distance below the cycle tolerance
are considered to be limit cycles. If a curve does not satisfy the tolerance,
then its return distance is compared to its neighboring integral curves. If a
zero crossing is found, then a binary search is conducted. The binary search is
also limited by a maximum number of iterations.

Source
^^^^^^

Source Type
"""""""""""

The user can seed the integral curves using samples on a line. This line
defines a Poincaré section used to detect limit cycles. Source options are:

Line
    Source is specified with the line end points. The line location can be set
    interactively using the interactive line tool. Options are:
    
    * Start - X Y Z location of the line starting point.
    * End - X Y Z location of the line ending point.
    
Sampling type
"""""""""""""

The Limit Cycle operator only supports uniform samples from the line.

Samples along line
    Number of uniform samples to take along the line defined above.

Field
"""""

See Field attributes that are common to all ICS operators.

Integration
^^^^^^^^^^^

Specify settings for numerical integrators. In addition to the Integration
attributes common to all ICS operators, the Limit Cycle operator supports the
following attributes.

Integration Direction
"""""""""""""""""""""

Sets the integration direction through time. Options are:
    
Forward
    Integrate forward in time.

Backward
    Integrate backward in time.

Both
    Integrate both forward and backward in time, producing two integral curves.

Forward Directionless
    Integrate forward in time assuming a directionless vector field.

Backward Directionless
    Integrate backward in time assuming a directionless vector field.

Both Directionless
    Integrate both forward and backward in time assuming a directionless vector
    field, producing two integral curves.

Eigen vectors are an example of a directionless vector field. To integrate
using a directionless field requires that any orientation discontinuity be
corrected prior to linear interpolation. That is, all vectors must be rotated 
to match the orientation of the trajectory. The ICS will do this
processing for standard fields (e.g non-higher order elements).

Integrator
""""""""""

See Integrator attributes that are common to all ICS operators.
    
Step Length
"""""""""""

See Step Length attributes that are common to all ICS operators.
    
Tolerances
""""""""""

See Tolerance attributes that are common to all ICS operators.

Termination
"""""""""""

Integral curve termination can be controlled in several different ways. The
termination is based on the most conservative criteria, so only one criteria
must be met for termination. The options are:

Maximum number of steps
    The maximum number of integration steps that will be allowed.  

Appearance
^^^^^^^^^^

The appearance tab specifies how the integral curve will be rendered. In addition
to the Appearance attributes common to all ICS operators, the Integral Curve
operator supports the following attributes.

Cycle tolerance
"""""""""""""""

The smallest return distance for classifying an integral curve as a limit
cycle.

Maximum iterations
""""""""""""""""""

The maximum numbers of iterations when performing the bi-section method.

Show partial results
""""""""""""""""""""

If the maximum number of bi-section iterations has been reached without finding
a limit cycle show the integral curves still in the queue.

Show the signed return distances for the first iteration
""""""""""""""""""""""""""""""""""""""""""""""""""""""""

Instead of plotting the limit cycles, plot the return distances along the
Poincaré section.

Data Value
""""""""""

Allows the user to set the data value associated with each data point on the
integral curve. Options are:

Solid
    Every curve is given the same color.

Seed ID
    Each curve is given a different color.

Speed
    Each curve's color varies by the magnitude of the vector field at each
    point along the curve.

Vorticity Magnitude
    Each curve's color varies by the magnitude of the vorticity at each point
    along the curve.

Arc Length
    Each curve is colored according to its path length.

Absolute time
    Each curve's color varies by the absolute time associated with each
    integration step at each point along the curve.

Relative time
    Each curve's color varies by the relative time associated with each
    integration step at each point along the curve assuming the seed point is
    at time zero (t = 0).

Average Distance from seed
    Each curve is colored according to the average distance of all the points
    in the curve from the seed.

Variable
    Each curve's color varies by the value of a scalar variable.

Streamlines vs Pathlines
""""""""""""""""""""""""

See Streamlines vs Pathlines attributes that are common to all ICS operators.

Advanced
^^^^^^^^

See Advanced tab attributes that are common to all ICS operators.