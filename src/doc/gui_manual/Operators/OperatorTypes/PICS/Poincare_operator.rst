.. _Poincare operator:

Poincare operator
~~~~~~~~~~~~~~~~~

Constructs a Poincaré section for toroidal geometry, i.e. for streamlines/fieldlines that lie on toroidal geometry (cylindrical coordinates). The basis of constructing a connected plot is to accurately determining the number of toroidal and poloidal windings (aka the winding pair). This process is iterative, starting with a set minimum number of puncture points and continuing until the toroidal and poloidal windings are known or the maximum number of punctures is reached. If an accurate winding pair is determined the puncture points are connected based on it.

A.R. Sanderson, G. Chen, X. Tricoche, E. Cohen. “Understanding Quasi-Periodic Fieldlines and Their Topology in Toroidal Magnetic Fields,” In Topological Methods in Data Analysis and Visualization II, Edited by R. Peikert and H. Carr and H. Hauser and R. Fuchs, Springer, pp. 125--140. 2012.

A.R. Sanderson, G. Chen, X. Tricoche, D. Pugmire, S. Kruger, J. Breslau. “Analysis of Recurrent Patterns in Toroidal Magnetic Fields,” In Proceedings Visualization / Information Visualization 2010, IEEE Transactions on Visualization and Computer Graphics, Vol. 16, No. 4, pp. 1431-1440. 2010.


Source
""""""

The user can seed the fieldlines either as a single point or a series of points along a line.

* Point – (default) create a single seed point. The point location can be set interactively using the interactive point tool. 

   * Location - Cartesian point with three values separated with a space. 

* Point List - one or more seed points are defined via a list of arbitrary points. 

   * Add Point - add a point which can then be edited by the user. Double clicking on the entry enters the editing mode
   * Delete Point - delete the currently selected entry
   * Delete All Points - delete all the points in the list
   * Read Text File - read in a list of points from a text file. The format must have one point per line as either "X Y Z" or "X, Y, Z. 

* Line – create multiple seed points along a line. The line location can be set interactively using the interactive line tool. 

   * Start Point - starting point for defining the line
   * End Point - ending point for defining the line
   * Point density - number of points evenly spaced along the line. 

Note: if the Field is set to be the M3D-C1 integrator the point locations will be converted from Cartesian to Cylindrical coordinates. And in the 2D case phi will be set to 0. 

Field -- see common

Integration
"""""""""""

See common

Punctures - While integrating the streamline (fieldline) to be used the for Poincaré plot the user has the option to require a minimum number of initial puncture points for the analysis. Similarly they may limit it in case of run-a-way fieldlines that can not be fully analyzed.

* Minimum – (Default 50) minimum number of punctures for the analysis.
* Maximum – (Default 500) maximum number of punctures for the analysis. 

Puncture Plane - Puncture plot type –

* Single - the analysis is based on the standard double periodic system (toroidal-poloidal periodicity)
* Double - the analysis is based on the double Poincaré plot. In addition to the toroidal-poloidal periodicity a third periodicity exists that is based on the integration time. 

   * When selecting double Poincaré plot puncture points are accepted if and only if the period is within the tolerance of the period (the period is set as part of the Poincaré Pathline Options). 

      * Period tolerance - when an integral curve punctures the plane, the period must be within the tolerance value. 

   * Because of the possibility for very long integration times integration limits must be utilized. 

      * Maximum number of steps - (default 1000) the maximum number of integration steps that will be allowed. 

      * Limit maximum time elapsed by particles - (Default state off, Default value 10) 

* Puncture Plane – the typical puncture plane is the poloidal plane but in some cases seeing the punctures in the toroidal plane may be useful.

   * Poloidal - (Default) create poloidal punctures
   * Toroidal - create toroidal punctures 

Note: when selecting “Toroidal” the “Analysis” must also be set to “Punctures only” as there is currently no analysis in the toroidal plane. See below 


Analysis
""""""""

Analysis Type - The user may turn the analysis off, “Punctures only” or on, “Full” 

* None - Puncture only – will result in constructing a traditional Poincaré plot using only points. 
*  Full – (Default) will analyze each fieldline’s geometry and attempt to reconstruct the cross sectional profile of the surface which the fieldline lies on. Further, the analysis attempt to identify the topology of the surface. 

Analysis - Parameters affecting the fieldline analysis

* Maximum toroidal winding – (Default 0) user to limit the search of the toroidal winding to lower order values. If zero there is no limit. 
* Override toroidal winding – (Default 0) In some cases (debugging) it may be informative to force the toroidal winding to have set value. If the “Override poloidal winding” is set to zero the poloidal winding will be determined. 
* Override poloidal winding – (Default 0) In some cases (debugging) it may be informative to force the poloidal winding to have set value. It is used only if the “Override toroidal value” is non zero. 
* Winding pair confidence – (Range 0-1, Default 0.9) sets the limit for the number of mismatches in the consistency in the winding pairs. 
* Rational Template Seed Parameter – ignore as it for a works in progress. 

Rational Surface - Allows for the construction of rational surfaces via an iterative process.

* Detect Rational Surface - on/off check box 
* Maximums iterations (Default 1) – sets the number of iterations for constructing the rational surface. Typically, it can be found with 5-10 iterations. 

Note: The rational surface construction is experimental code and does not always work!!!! 

O-Line Analysis Points - Calculate the poloidal winding relative the O-Line (central axis) which provides a more accurate winding value.

* Perform O-Line Analysis - on/off check box 
* O-Line toroidal windings (Default 1) – sets the toroidal winding value, i.e. the period (for the central axis the period would be 1). 
* O-Line Axis Point File button- allows the user to select a text file containing the points along the axis from 0 to 360 degrees (note there is no overlap P(0) != P(n)). 
* O-Line Axis Point File - the name of the O-Line Axis Point File. 

Options - Options for how some of the results are displayed.

* Show islands only (Default off) – culls the results so that only island chains are displayed. 
* Show chaotic fieldlines (as points) (Default off) – chaotic fieldlines can not be classified as such they are not displayed unless checked. 
* Show ridgelines (Default off) – displays the 1D plots of the distance and ridgeline samples. (debugging only). 
* Verbose (Default on) – dumps information regarding the analysis to the terminal. The final summary is useful to the user: 
   
   * Surface id = 0 < 2.35019 0 0.664124 > 121:11 121:11 (11) flux surface with 4 nodes (Complete) 
   * Surface id = 0 
   * seed location < 2.35019 0 0.664124 > 
   * the winding pair 121:11 
   * the toroidal:poloidal periods (as a winding pair) 121:11 
   * the multiplication faction (11) i.e. diving by this number will give the base winding values, in this case 11:1. 
   * surface type: flux surface 
   * number of nodes in each winding group: with 4 nodes 
   * analysis state: complete. 


Appearance
""""""""""

Data - Allows the user to set the data value associated with each data point and the overall limits

* None - solid color from the single color 
* Safety Factor Q – (Default) use the safety factor 
* Safety Factor P – Use the safety factor as defined when there are two possible choices for the magnetic axis 
* Safety Factor Q == P – Draw the surfaces on if the safety factor Q is equal to the safety factor P 
* Safety Factor Q != P – Draw the surfaces on if the safety factor Q is not equal to the safety factor P 
* Toroidal Windings Q – use the toroidal winding value used in the calculation of Q 
* Toroidal Windings P – use the toroidal winding value used in the calculation of P 
* Poloidal Windings – use the poloidal winding value 
* Fieldline Order – use input order of the seeds used to generate the fieldlines 
* Point Order – use the puncture point index 
* Plane – use the plane value (integer from 0 to N where N is the number of planes) 
* Winding Group Order – use the winding group order (integer from 0 to T where T is the toroidal winding) 
* Winding Point Order - use the index of the puncture points within each winding group 
* Winding Point Order Modulo Order - use the order of the punctures within each winding group modulo the toroidal windings (useful for islands in islands) 

Display - Allows the users to display the results in a single plane or multiple planes. Further, one can reconstruct the 3D surface that the fieldline lies on.

* Mesh Type – Display the results as: 

   * Curves (Default) 
   * Surfaces 
   * Number of planes – (Default 1) draw the results on N planes evenly spaced from 0 to 360. 
   * Base plane (degrees) – (Default 0) For curves adjust the viewing phi angle of 0 to match the simulation phi angle of 0. 
   * Adjust plane index – (Default -1) For surfaces, often the connecting of the first plane to the last plane is off . As such, this adjust the connection index of the winding groups. 

Overlaps - When displaying the data in a connected manner the raw data will often over itself. As such, for visually pleasing results it preferable to remove the overlaps.

* Overlapping curve sections: 

   * Raw – display all of the punctures points in a connected fashion. 
   * Remove (Default) - display all of the punctures points in a connected fashion removing the overlapping sections. 
   * Merge - display all of the punctures points in a connected fashion merging the overlapping sections. Experimental. 
   * Smooth - display all of the punctures points in a connected fashion removing the overlapping sections while smoothing between points. Experimental. 

Sreamlines vs Pathlines -- see common

Pathline Options - See Pathline Options for the other pathline options common to all PICS operators. 

* Period - the period of the integral curve to be used in the analysis of double Poincaré plot. 


Advanced
""""""""

See common

Note: not all PICS warnings are applicable to the Poincaré operator. 


Example
"""""""

















