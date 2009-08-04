// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France


/*-----------------------------------*
*                LOVE               *
*-----------------------------------*/
/*!
 \brief Implementation of class CellInterface (component Interface.Outils)
 \author Thierry Carrard
 \date 9 Mars 2005

 Modifications history :
 */


const static char * CELL_INTERFACE_C_SCCS_ID = "%Z% DSSI/SNEC/LDDC %M%   %I%     %G%";

#include "CellInterface.h"

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

#include <stdio.h>
#include <math.h>

#include "CellInterface2D.h"
#include "CellInterface3D.h"
#include "CellInterfaceCommon.h"

// usefull to avoid numerical errors
#define Clamp(x,min,max) if(x<min) x=min; else if(x>max) x=max

struct VertexInfo
{
      double coord[3];
      double weight;
      int eid[2];
};

struct CWVertex
{
      double angle;
      double coord[3];
      double weight;
      int eid[2];
      inline bool operator < (const CWVertex& v) const { return angle < v.angle; }
};

// ------------------------------------
//         ####     ####
//             #    #   #
//          ###     #   #
//             #    #   #
//         ####     ####
// ------------------------------------
void CellInterface::cellInterface3D( 
   int ncoords,
   double coords[][3],
   int nedge,
   int cellEdges[][2],
   int ntetra,
   int tetraPointIds[][4],
   double fraction, double normal[3] , 
   bool useFractionAsDistance,
   int & np, int eids[], double weights[] ,
   int & nInside, int inPoints[],
   int & nOutside, int outPoints[] )
{
  // normalisation du vecteur normal si la norme >0
   double nlen2 = normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2];
   if( nlen2 > 0 )
   {
      double nlen = sqrt(nlen2);
      normal[0] /= nlen;
      normal[1] /= nlen;
      normal[2] /= nlen;
   }
   else
   {
      normal[0] = 1;
      normal[1] = 0;
      normal[2] = 0;
   }

   double dmin, dmax;
   dmin = dmax = coords[0][0]*normal[0] + coords[0][1]*normal[1] + coords[0][2]*normal[2];
   for(int i=0;i<ncoords;i++)
   {
      double d = coords[i][0]*normal[0] + coords[i][1]*normal[1] + coords[i][2]*normal[2];
      if( d<dmin ) dmin=d;
      else if( d>dmax ) dmax=d;
   }

  // compute plane's offset ( D parameter in Ax+By+Cz+D=0 )
   double d = useFractionAsDistance ? fraction : findTetraSetCuttingPlane( normal, fraction, ncoords, coords, ntetra, tetraPointIds );

  // compute vertex distances to interface plane
   double dist[MAX_CELL_POINTS];
   for(int i=0;i<ncoords;i++)
   {
      dist[i] = coords[i][0]*normal[0] + coords[i][1]*normal[1] + coords[i][2]*normal[2] + d;
   }

  // get in/out points
   nInside=0;
   nOutside=0;
   for(int i=0;i<ncoords;i++)
   {
      if( dist[i] <= 0.0 ) 
      {
	 inPoints[nInside++] = i;
      }
      else
      {
	 outPoints[nOutside++] = i;
      }
   }   

   double center[3] = {0,0,0};
   double polygon[MAX_CELL_POINTS][3];

  // compute intersections between edges and interface plane
   np = 0;
   for(int i=0;i<nedge;i++)
   {
      int e0 = cellEdges[i][0];
      int e1 = cellEdges[i][1];
      if( dist[e0]*dist[e1] < 0 )
      {
	 double edist = dist[e1] - dist[e0];
	 double t;
	 if(edist!=0)
	 {
	    t = ( 0 - dist[e0] ) / edist ;
	    Clamp(t,0,1);
	 }
	 else
	 {
	    t = 0;
	 }
	 
	 for(int c=0;c<3;c++)
	 {
	    polygon[np][c] = coords[e0][c] + t * ( coords[e1][c] - coords[e0][c] ) ;
	    center[c] += polygon[np][c];
	 }
	 eids[np*2+0] = e0;
	 eids[np*2+1] = e1;
	 weights[np] = t;
	 np++;
      }
   }

  // tri des points
   if(np>3)
   {
     // calcul du centre du polygone
      for(int d=0;d<3;d++) center[d] /= np;

     // calcul de la direction dominante, pour retomber sur un cas 2D
      int maxDim = 0;
      if( fabs(normal[1]) > fabs(normal[maxDim]) ) maxDim=1;
      if( fabs(normal[2]) > fabs(normal[maxDim]) ) maxDim=2;
      int xd=0, yd=1;
      switch(maxDim)
      {
	case 0: xd=1; yd=2; break;
	case 1: xd=0; yd=2; break;
	case 2: xd=0; yd=1; break;
      }

     // calcul des angles des points du polygone
      CWVertex pts[MAX_CELL_POINTS];
      for(int i=0;i<np;i++)
      {
	 double vec[3];
	 for(int d=0;d<3;d++)
	 {
	    pts[i].coord[d] = polygon[i][d];
	    vec[d] = polygon[i][d]-center[d];
	 }
	 
	 pts[i].weight = weights[i];
	 pts[i].eid[0] = eids[i*2+0];
	 pts[i].eid[1] = eids[i*2+1];
	 pts[i].angle = atan2( vec[yd], vec[xd] );
      }
      std::sort( pts , pts+np );
      for(int i=0;i<np;i++)
      {
	 weights[i] = pts[i].weight;
	 eids[i*2+0] = pts[i].eid[0];
	 eids[i*2+1] = pts[i].eid[1];
      }
   }
}





// ------------------------------------
//         ####     ####
//             #    #   #
//          ###     #   #
//         #        #   #
//        #####     ####
// ------------------------------------

bool CellInterface::cellInterface2D( 
   double points[][3],
   int nPoints,
   int triangles[][3], // TODO: int [] pour plus d'integration au niveau du dessus
   int nTriangles,
   double fraction, double normal[3] ,
   bool axisSymetric,
   bool useFractionAsDistance,
   int eids[4], double weights[2] ,
   int &polygonPoints, int polygonIds[],
   int &nRemPoints, int remPoints[]
   )
{
   double d = useFractionAsDistance ? fraction : findTriangleSetCuttingPlane( normal, fraction, nPoints, points, nTriangles, triangles , axisSymetric );

  // compute vertex distances to interface plane
   double dist[nPoints];
   for(int i=0;i<nPoints;i++)
   {
      dist[i] = points[i][0]*normal[0] + points[i][1]*normal[1] + points[i][2]*normal[2] + d;
   }

  // compute intersections between edges and interface line
   int np = 0;
   nRemPoints = 0;
   polygonPoints = 0;
   for(int i=0;i<nPoints;i++)
   {
      int edge[2];
      edge[0] = i;
      edge[1] = (i+1)%nPoints; 
      if( dist[i] <= 0.0 ) 
      {
	 polygonIds[polygonPoints++] = i;
      }
      else
      {
	 remPoints[nRemPoints++] = i;
      }
      if( np < 2 )
      {
	 if( dist[edge[0]]*dist[edge[1]] < 0.0 )
	 {
	    double t = ( 0 - dist[edge[0]] ) / ( dist[edge[1]] - dist[edge[0]] );
	    Clamp(t,0,1);
	    eids[np*2+0] = edge[0];
	    eids[np*2+1] = edge[1];
	    weights[np] = t;
	    np++;
	    polygonIds[polygonPoints++] = -np;
	    remPoints[nRemPoints++] = -np;
	 }
      }
   }

   return (np==2);
}
