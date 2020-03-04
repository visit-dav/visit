.. _Limit Cycle operator:

Limit Cycle operator
~~~~~~~~~~~~~~~~~~~~

The Integral Curve system allows for the computation of LCS structures using a variety techniques developed by George Haller and his group at ETH Zürich. Haller proposed that the ridges of FTLE are indicators of hyperbolic LCS (repelling and attracting). More recent research by Haller's group has focused a mathematical description of LCS In two dimensional flows, hyperbolic LCSs turn out to be stationary curves of the averaged material shear while elliptic LCSs are stationary curves of the averaged strain. The LCS operator coupled with the Integral Curve (Hyperbolic LCS) and Limit Cycle (Elliptical LCS) operators provide a mechanism calculate these structures and have been developed in cooperation between the VisIt PICS team and the Haller Group.

For complete details on computing elliptical LCS (closed orbits) and hyperbolic LCS (stretch and shrink lines) see the following paper: K. Onu, F. Huhn, & G. Haller, LCS Tool: A Computational platform for Lagrangian coherent structures, J. of Computational Science, 7 (2015) 26-36, http://georgehaller.com/reprints/lcstool.pdf

The Limit Cycle Operator allows one to search for closed material lines (limit cycles) which are elliptic LCSs. To perform this search one defines a Poincaré section where a series of integral curves are advected. Most integral curves will return to the Poincaré section at which point their integration will be stopped. Some integral curves may not return to the Poincaré section. To limit run-a-way integration and like with all integral curve integration the user can control the termination criteria.

For the integral curves that return to the Poincaré section a signed return distance is calculated. Curves with a return distance below the cycle tolerance are considered to be limit cycles. For those curves not meeting the tolerance their return distance is compared to their neighboring integral curves. If a zero crossing is found a binary search is conducted. The binary search is also limited by a maximum number of iterations.


Source
""""""
The user can seed the integral curves using samples on a line. This line defines a Poincaré section which to test against.

* Line - source is specified with the line end points. The line location can be set interactively using the interactive line tool. 
   * Start - X Y Z location of the line starting point.
   * End - X Y Z location of the line ending point. 
* Sampling type - currently the only one sampling type is possible. 
   * Uniform - sample uniformly along the Poincaré section. 
* Samples along line: - number of samples along the Poincaré section.

Field -- see commong


Integration
"""""""""""

See common

Termination - Integral curve termination can be controlled in several different ways. The termination is based on the most conservative criteria, that is only one criteria must be met for termination.

* Maximum number of steps - (default 1000) the maximum number of integration steps that will be allowed. 
* Limit maximum time elapsed by particles - (Default state off, Default value 10) 
* Limit maximum distance traveled by particles - (Default state off, Default value 1000) 


Appearance
""""""""""

Limit cycle search - 

* Cycle tolerance - (Default value 1e-06) The smallest return distance for classifying an integral curve as a limit cycle.
* Maximum iterations - (Default value 10) The maximum numbers of iterations when performing the bi-section method. 
* Show partial results (limit cycle may not be present) - (Default state unchecked) If the maximum number of bi-section iterations has been reached without finding a limit cycle show the integral curves still in the queue.
* Show the signed return distances for the first iteration - (Default state unchecked) Instead of plotting the limit cycles, plot the return distances along the Poincaré section

Data - Allows the user to set the data value associated with each data point on the integral curve

* Solid - Each integral curve is given a single color.
* Seed ID - Each integral curve is given a different color.
* Speed - The magnitude of the vector field at each point.
* Vorticity Magnitude - The vorticity magnitude at each point.
* Arc Length - the overall path length of the integral curve.
* Absolute time - The absolute time associated with each integration step.
* Relative time - The relative time associated with each integration step assuming the seed point is a time equals zero (t = 0).
* Average Distance from seed - a single value based on the average distance from seed.
* Correlation Distance.
* Difference - under construction.
* Variable. Each point along a streamline is colored by the value of a scalar variable. 

Streamlines and Pathlines - see common


Advanced
""""""""

See common


Example
"""""""

















