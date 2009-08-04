// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France


#ifndef CELL_INTERFACE3D_H
#define CELL_INTERFACE3D_H
/*-----------------------------------*
 *                LOVE               *
 *-----------------------------------*/
/*!
\brief  (component Interface.Outils)
\author Thierry Carrard
\date 4 Decembre 2007

Modifications history :
*/

#ifndef SWIG
const static char * CELL_INTERFACE3_D_H_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";
#endif /*SWIG*/

extern
double findTetraSetCuttingPlane(
   const double normal[3],
   const double fraction,
   const int vertexCount,
   const double vertices[][3],
   const int tetraCount,
   const int tetras[][4]
   );

#endif /* CELL_INTERFACE3_D_H */
