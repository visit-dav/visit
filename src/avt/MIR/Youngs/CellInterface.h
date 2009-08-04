// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France


#ifndef CELL_INTERFACE_H
#define CELL_INTERFACE_H
/*-----------------------------------*
 *                LOVE               *
 *-----------------------------------*/
/*!
\brief Contains definition of class CellInterface (component Interface.Outils)
\author Thierry Carrard
\date 9 Mars 2005

Modifications history :
*/

#ifndef SWIG
const static char * CELL_INTERFACE_H_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";
#endif /*SWIG*/

/*!
\class CellInterface
Front-end to Youngs core algorithm.
the two provided fonctions find the placement of an interface plane given a triangulated cell, a normal vector and a volume fraction
*/
class CellInterface
{
   public:

      enum {
	 MAX_CELL_POINTS = 128,
	 MAX_CELL_TETRAS = 128
      };


     // ------------------------------------
     //         ####     ####
     //             #    #   #
     //          ###     #   #
     //             #    #   #
     //         ####     ####
     // ------------------------------------

      static void cellInterface3D( 
	 
	// Inputs
	 int ncoords,
	 double coords[][3],
	 int nedge,
	 int cellEdges[][2],
	 int ntetra,
	 int tetraPointIds[][4],
	 double fraction, double normal[3] , 
	 bool useFractionAsDistance,

	// Outputs
	 int & np, int eids[], double weights[] ,
	 int & nInside, int inPoints[],
	 int & nOutside, int outPoints[] );


     // ------------------------------------
     //         ####     ####
     //             #    #   #
     //          ###     #   #
     //         #        #   #
     //        #####     ####
     // ------------------------------------

      static bool cellInterface2D( 

	// Inputs
	 double points[][3],
	 int nPoints,
	 int triangles[][3], 
	 int nTriangles,
	 double fraction, double normal[3] ,
	 bool axisSymetric,
	 bool useFractionAsDistance,

	// Outputs
	 int eids[4], double weights[2] ,
	 int &polygonPoints, int polygonIds[],
	 int &nRemPoints, int remPoints[] );

} ;

#endif /* CELL_INTERFACE_H */
