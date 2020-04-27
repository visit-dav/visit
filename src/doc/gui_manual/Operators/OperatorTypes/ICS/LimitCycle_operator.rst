.. _Limit Cycle operator:

Limit Cycle operator
~~~~~~~~~~~~~~~~~~~~

The Limit Cycle Operator detects limit cycles within a vector field. Integral
curves are seeded at a Poincaré section and integrated through the vector
field. Curves that return to the Poincaré section indicate a limit cycle, and
the integration of the curve will stop. Those integral curves that do not
return to the Poincaré section are terminated according to separate termination
criteria.

A signed return distance is calculated for the integral curves that return to
the Poincaré section. Curves with a return distance below the cycle tolerance
are considered to be limit cycles. If a curve does not satisfy the tolerance,
then its return distance is compared to its neighboring integral curves. If a
zero crossing is found, then a binary search is conducted. The binary search is
also limited by a maximum number of iterations.

Source
^^^^^^

The set of points that seed the integral curves that reveal the Limit Cycles.
In addition to the :ref:`common ICS source` attributes common to all ICS
operators, the Limit Cycle operator supports the following attributes:

Source Type
"""""""""""

The source type controls how the seeds for curves are created. The Limit Cycle
operator only supports uniform samples on a line.

Integration
^^^^^^^^^^^

Specify settings for numerical integrators. In addition to the
:ref:`common ICS integration` attributes common to all ICS operators, the Limit
Cycle operator supports the following attributes.

Integration Direction
"""""""""""""""""""""

Sets the integration direction through time. The user can choose from a
combination of forward, backward, and directionless. Eigen vectors are an
example of a directionless vector field. In order to integrate using a
directionless field, any orientation discontinuity must be corrected prior to
linear interpolation. That is, all vectors must be rotated to match the
orientation of the trajectory. The ICS code will do this processing for
standard fields (e.g non-higher order elements).

Termination
"""""""""""

Integral curve termination can be controlled in several different ways. The
termination is based on the most conservative criteria, so only one criteria
must be met for termination. The options are:

Maximum number of steps
    The maximum number of integration steps that will be allowed.  

Appearance
^^^^^^^^^^

The appearance tab specifies how the integral curve will be rendered. In
addition to the :ref:`common ICS appearance` attributes common to all ICS
operators, the Integral Curve operator supports the following attributes.

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
a limit cycle, show the integral curves still in the queue.

Show the signed return distances for the first iteration
""""""""""""""""""""""""""""""""""""""""""""""""""""""""

Instead of plotting the limit cycles, plot the return distances along the
Poincaré section.

Coloring
""""""""

There are various coloring options, the names of which are self-descriptive
such as coloring the curves with a *solid* color or according to a *seed*. Only
those options that require further clarification are described further here.

Average Distance from seed
    Each curve is colored according to the average distance of all the points
    in the curve from the seed.

Variable
    Each curve's color varies by the value of a scalar variable.

Advanced
^^^^^^^^

See :ref:`common ICS advanced` tab attributes that are common to all ICS
operators.