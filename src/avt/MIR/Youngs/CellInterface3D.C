// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France

#include<vector>
using std::vector;

//#define REAL_PRECISION 32
#define REAL_PRECISION 64

//#define DEBUG
#include "CellInterfaceTools3D.h"

double findTetraSetCuttingPlane(
   const double normal[3],
   const double fraction,
   const int vertexCount,
   const double vertices[][3],
   const int tetraCount,
   const int tetras[][4]
   )
{

    REAL3 N = { normal[0], normal[1], normal[2] };
    vector<REAL3> V(vertexCount);
    vector<uchar4> tet(tetraCount);

    for(int i=0;i<vertexCount;i++)
    {
        V[i].x = vertices[i][0] - vertices[0][0] ;
        V[i].y = vertices[i][1] - vertices[0][1] ;
        V[i].z = vertices[i][2] - vertices[0][2] ;
    }

    REAL3 vmin,vmax;
    REAL scale;
    vmin = vmax = V[0];
    for(int i=1;i<vertexCount;i++)
    {
        if( V[i].x < vmin.x ) vmin.x = V[i].x;
        if( V[i].x > vmax.x ) vmax.x = V[i].x;
        if( V[i].y < vmin.y ) vmin.y = V[i].y;
        if( V[i].y > vmax.y ) vmax.y = V[i].y;
        if( V[i].z < vmin.z ) vmin.z = V[i].z;
        if( V[i].z > vmax.z ) vmax.z = V[i].z;
    }
    scale = vmax.x - vmin.x;
    if( (vmax.y-vmin.y) > scale ) scale = vmax.y-vmin.y;
    if( (vmax.z-vmin.z) > scale ) scale = vmax.z-vmin.z;
    for(int i=0;i<vertexCount;i++) V[i] /= scale;

    for(int i=0;i<tetraCount;i++)
    {
        tet[i].x = tetras[i][0];
        tet[i].y = tetras[i][1];
        tet[i].z = tetras[i][2];
        tet[i].w = tetras[i][3];
    }

    double dist0 = vertices[0][0]*normal[0] + vertices[0][1]*normal[1] + vertices[0][2]*normal[2];
    double d = dist0 + findTetraSetCuttingPlane(N, fraction, vertexCount, tetraCount, &tet[0], &V[0] ) * scale;

    return - d;
}
