// Contributed by Thierry Carrard from
// Commissariat a l'Energie Atomique, (CEA)
// BP12, 91297 Arpajon, France


//#define REAL_PRECISION 32
#define REAL_PRECISION 64

//#define DEBUG 1

#define REAL_COORD REAL3
#include "CellInterfaceTools2D.h"
#include "CellInterfaceTools2D_Axis.h"

double findTriangleSetCuttingPlane(
   const double normal[3],
   const double fraction,
   const int vertexCount,
   const double vertices[][3],
   const int triangleCount,
   const int triangles[][3],
   bool axisSymetric
   )
{
    double d;

    uchar3 tri[triangleCount];
    for(int i=0;i<triangleCount;i++)
    {
        tri[i].x = triangles[i][0];
        tri[i].y = triangles[i][1];
        tri[i].z = triangles[i][2];
    }

    if( axisSymetric )
    {
        REAL2 N = { normal[0], normal[1] };
        REAL2 V[vertexCount];
        for(int i=0;i<vertexCount;i++)
        {
            V[i].x = vertices[i][0] - vertices[0][0] ;
            V[i].y = vertices[i][1] - vertices[0][1] ;
        }
        REAL2 vmin,vmax;
        REAL scale;
        vmin = vmax = V[0];
        for(int i=1;i<vertexCount;i++)
        {
            if( V[i].x < vmin.x ) vmin.x = V[i].x;
            if( V[i].x > vmax.x ) vmax.x = V[i].x;
            if( V[i].y < vmin.y ) vmin.y = V[i].y;
            if( V[i].y > vmax.y ) vmax.y = V[i].y;
        }
        scale = vmax.x - vmin.x;
        if( (vmax.y-vmin.y) > scale ) scale = vmax.y-vmin.y;
        for(int i=0;i<vertexCount;i++) V[i] /= scale;
        double dist0 = vertices[0][0]*normal[0] + vertices[0][1]*normal[1] ;
        d = dist0 + findTriangleSetCuttingCone(N, fraction, vertexCount, triangleCount, tri, V ) * scale;
    }
    else
    {
        REAL3 N = { normal[0], normal[1], normal[2] };
        REAL3 V[vertexCount];
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
        double dist0 = vertices[0][0]*normal[0] + vertices[0][1]*normal[1] + vertices[0][2]*normal[2];
        d = dist0 + findTriangleSetCuttingPlane(N, fraction, vertexCount, triangleCount, tri, V ) * scale;
    }

    return - d;
}


