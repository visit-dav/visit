.. _Integral Curve operator:

Integral Curve operator
~~~~~~~~~~~~~~~~~~~~~~~

Source
""""""

Source type
    The user can seed the integral curves using a single point; an arbitrary list of points; samples on a line, plane, circle, sphere, or box; a Named Selection; or Field Data. The following is a list of source types and parameters.

* Point - Seed from a point. The point location can be set interactively using the interactive point tool.

   * Location - X Y Z location of the seed point. 
   
* Point List - Seed from a list of points.

   * Add Point - add a point which can then be edited by the user. Double clicking on the entry enters the editing mode.
   * Delete Point - delete the currently selected entry.
   * Delete All Points - delete all the points in the list.
   * Read Text File - read in a list of points from a text file. The format must have one point per line as either "X Y Z" or "X, Y, Z"
   
* Line - Seed from a line defined by its end points. The line location can be set interactively using the interactive line tool.
 
   * Start - X Y Z location of the line starting point.
   * End - X Y Z location of the line ending point.
   * Sampling - Specify the type of sampling and the number of samples.
   
* Circle - Seed from a cicle defined by a plane and a radius. 

   * Origin - X Y Z location of the origin.
   * Normal - X Y Z vector. The "normal" of the plane.
   * Up Axis - X Y Z vector. The "up axis" serves as the "Y" axis embedded in the plane. (The vector orthogonal to both the "up axis" and the "normal" serves as the "X" axis embedded in the plane.)
   * Radius - radius of the circle.
   * Sampling - Specify the type of sampling, the region from which to sample (interior or boundary) and the number of samples.
   
* Plane - Seed from a plane. The plane location can be set interactively using the interactive plane tool.

   * Origin - X Y Z location of the origin.
   * Normal - X Y Z vector. The "normal" of the plane.
   * Up Axis - X Y Z vector. The "up axis" serves as the "Y" axis embedded in the plane. (The vector orthogonal to both the "up axis" and the "normal" serves as the "X" axis embedded in the plane.)
   * Sampling - Specify the type of sampling, the region from which to sample (interior or boundary) and the number of samples.
      * The sampling "Distance in X" and "Distance in Y" define the dimensions of the rectangle in the plane. 
   
* Sphere - Seed from a sphere defined by a point and a radius. The sphere location and size can be set interactively using the interactive sphere tool.

   * Origin - X Y Z location of the origin.
   * Radius - radius of the sphere.
   * Sampling - Specify the type of sampling, the region from which to sample (interior or boundary) and the number of samples.
   
      * "Samples in Latitude", "Samples in Longitude", and "Samples in R" specifies how many samples to take along each direction.
   
* Box - Seed from a box defined by bounds along the X, Y, and Z axes. The box location and size can be set interactively using the interactive box tool.

   * The "Whole data set" check box indicates that the seeding box should be the same as the bounding box of the entire dataset.
   * X Extents - extents along the X axis.
   * Y Extents - extents along the Y axis.
   * Z Extents - extents along the Z axis. 
   * Sampling - Specify the type of sampling, the region from which to sample (interior or boundary) and the number of samples.
   
* Selection - Seed with a named selection.
    
* Field Data - The seed points are defined by another operator and passed to the Integral Curve operator. The array containing the seed point(s) must begin its name with "Seed Points".

   * Copy to point list - Copy the points to the point list source (see above). This allows points to be edited. 

Sampling type
    For samples taken from a line, circle, plane, sphere or box, there is an option to generate uniform or random samples from the specified region. In more detail:

* Uniform (Default)- sample uniformly along the boundary or the interior. 

   * Samples in X - (Default 1) Create N samples along the X axis.
   * Samples in Y - (Default 1) Create N samples along the Y axis.
   * Samples in Z - (Default 1) Create N samples along the Z axis. 
   
* Random - sample randomly along the boundary or the interior. 

   * Random number of samples (Default 1).
   * Random number Seed - (Default 0) ensures that the same set of random samples is taken each time the plot is regenerated. 

Boundary vs Interior Samples
    Samples from a circle, plane, sphere or box can be either taken from the boundary, or the interior. For example, in the case of a plane sampling source, the samples can either lie along the edges of the planar region, or within the bounded rectangle, as shown below.
    
Field
    See Field attributes that are common to all PICS operators.


Integration
"""""""""""

Specify settings for numerical integrators. In addition to the Integration attributes common to all PICS operators, the Integral Curve operator supports the following attributes.

Integration Direction
    Sets the integration direction through time. Options are:
    
* Forward - Integrate forward in time.
* Backward - Integrate backward in time.
* Both - Integrate both forward and backward in time, producing two integral curves.
* Forward Directionless - Integrate forward in time assuming a directionless vector field.
* Backward Directionless - Integrate backward in time assuming a directionless vector field.
* Both Directionless - Integrate both forward and backward in time assuming a directionless vector field, producing two integral curves.

  Eigen vectors are an example of a directionless vector field. To integrate using a directionless field requires that any orientation discontinuity be corrected prior to linear interpolation. That is, all vectors must be rotated to match the orientation of the trajectory. The PICS code will do this processing for standard fields (e.g non-higher order elements).
    
Integrator
    See Integrator attributes that are common to all PICS operators.
    
Step Length
    See Step Length attributes that are common to all PICS operators.
    
Integrator
    See Tolerance attributes that are common to all PICS operators.

Termination
    Integral curve termination can be controlled in several different ways. The termination is based on the most conservative criteria, so only one criteria must be met for termination. The options are:

* Maximum number of steps - (default 1000) the maximum number of integration steps that will be allowed. 
* Limit maximum time elapsed by particles - (Default state off, Default value 10) 
* Limit maximum distance traveled by particles - (Default state off, Default value 1000)


Appearance
""""""""""

The appearance tab specifies how the integral curve will be drawn. In addition to the Appearance attributes common to all PICS operators, the Integral Curve operator supports the following attributes.

Data
    Allows the user to set the data value associated with each data point on the integral curve. You can set the data value to one of:

* Solid - Each integral curve is given a single color.
* Seed ID - Each integral curve is given a different color.
* Speed - The magnitude of the vector field at each point.
* Vorticity Magnitude - The vorticity magnitude at each point.
* Arc Length - the overall path length of the integral curve.
* Absolute time - The absolute time associated with each integration step.
* Relative time - The relative time associated with each integration step assuming the seed point is at time equals zero (t = 0).
* Average Distance from seed - a single value based on the average distance from seed.
* Correlation Distance.
* Difference - under construction.
* Variable. Each point along a streamline is colored by the value of a scalar variable. 

Cleanup
    Allows the user to remove points along the integral curve according to difference schemes.

* Keep all points - Keep all points generated.
* Merge points - Use the vtkCleanPolyData algorithm to merge duplicate points using a spatial threshold. If the threshold is zero the tolerance will be based on a fraction of the bounding box. Otherwise the tolerance will be based on an absolute tolerance.
* Delete points before - delete all points that come before a "critical" point defined by a velocity threshold. This cleaning will reveal when an integral curve may stop advecting because of some other reason than the critical point. i.e. the advection continues temporally but not spatially, so this cleaning will remove all duplicate points leaving the last temporal value. If the last point temporal value is different than the value as dictated by the elapsed time or max steps, then the advection may have reached a critical point but terminated because of some other reason. 
* Delete points after - delete all points that come after a "critical" point defined by a velocity threshold. This cleaning will reveal when an integral curve reaches a critical point. i.e. the advection continues temporally but not spatially, so this cleaning will remove all duplicate points leaving the first temporal value. 

  Note: if displaying integral curves using tubes or ribbon regardless of the cleanup setting vtkCleanPolyData will be called. Tubes and ribbons cannot contain duplicate points. 

Crop the integral Curve (for animations)
    Integral curves can be cropped so that they appear to grow over time. This option is useful for creating animation via a python script.
    
* Distance - crop based on the arc length of the integral curve.
* Time - crop based on the integration time of the integral curve.
* Step number - crop based on the step number of the integral curve. 
* From - if selected the cropping will start at the value selected.
* To - if selected the cropping will end at the value selected.

  Below is an example python script that crops an integral curve and assumes the initial curve has been defined via the integral curve operator. ::

   database = "localhost:/Projects/Data/myDatabase.vtk database"
   md = GetMetaData( database )
   #Assume the other attributes of the Integral Curve operator have been set for drawing a path line from time zero to the end time.
   atts = IntegralCurveAttributes()
   atts.cropValue = atts.Time
   atts.cropEndFlag = 1
   # Draw ten steps for each interval
   nsteps = 10
   # loop through each time interval
   for i in range(TimeSliderGetNStates()-1) :
     time = md.times[i]
     nextTime = md.times[i+1]
     # Draw ten steps for each interval
     for j in range(nsteps) :
       atts.cropEnd = time + (nextTime-time) * (j+1) / nsteps
       SetOperatorOptions(atts)

Streamlines vs Pathlines
    See Streamlines vs Pathlines attributes that are common to all PICS operators.


Advanced
""""""""

The Integral Curve Operator shares the same Parallel attributes as all the other PICS operators.

Warnings
    In addition to the Warnings common to all PICS operators, the Integral Curve operator supports the following warnings.

* Issue warning if the advection limit is not reached - If the maximum time or distance is not reached, issue a warning.
* Issue warning if the spatial boundary is reached - If the integral curve reaches the spatial domain boundary, issue a warning.





















