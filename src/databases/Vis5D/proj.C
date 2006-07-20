/* Notes:
 *   Brad Whitlock, Tue Jul 18 15:51:36 PST 2006
 *   This code was taken from the Vis5D source code files proj.c and globals.h
 *   and turned into a self-contained file for converting between rcl coordinates
 *   and geo coordinates.
 */
    
#include <math.h>
#include <v5d.h>

#define MAXROWS     400
#define MAXCOLUMNS  400

#define DEG2RAD    (M_PI/180.0)
#define RAD2DEG    (180.0/M_PI)
#define RADIUS     6371.23

/* Map projections and vertical coordinate systems: */
/* ZLB 2 Sep 2000 */
#define PROJ_GENERIC_NONEQUAL -1 /* non equally spaced, no specific units */
#define PROJ_MIN_VALUE        -1 /* smallest value of legal projection types */

#define PROJ_GENERIC          0  /* No specific units */
#define PROJ_LINEAR           1  /* Cylindrical-Equidistant (old vis5d) */
#define PROJ_LAMBERT          2  /* Lambert conformal */
#define PROJ_STEREO           3  /* Stereographic */
#define PROJ_ROTATED          4  /* Rotated equidistant */
#define PROJ_MERCATOR         5  /* Mercator */
#define PROJ_CYLINDRICAL     20  /* Cylindrical projection of cyl-equid. */
#define PROJ_SPHERICAL       21  /* Spherical projection of cyl-equid. */

#define VERT_GENERIC          0 /* No specific units */
#define VERT_EQUAL_KM         1 /* Equally spaced in kilometers */
#define VERT_NONEQUAL_KM      2 /* Non-equally spaced in kilometers */
#define VERT_NONEQUAL_MB      3 /* Non-equally spaced in millibars */

int REVERSE_POLES = 1;

typedef struct
{
   int       Nr;                /* Number of rows */
   int       Nc;                /* Number of columns */

   /*** Map projection and vertical coordinate system ***/
   int Projection;              /* One of PROJ_*, as above */
   int UserProjection;          /* Optional override of projection from file */
   float *UserProjArgs;         /* Optional projection args for override */
   float NorthBound;            /* See proj.c for how each of these */
   float SouthBound;            /* variables is used depending on the */
   float WestBound;             /* value of the Projection variable. */
   float EastBound;
   float RowInc;
   float ColInc;
   /* ZLB 02-09-2000 */
   float Latitude[MAXROWS];     /* coordinates of Rows */
   float Longitude[MAXCOLUMNS]; /* coordinates of Columns */
   float Lat1, Lat2;
   float PoleRow, PoleCol;
   float CentralLat;
   float CentralLon;
   float CentralRow;
   float CentralCol;
   float Rotation;             /* radians */
   float Cone;
   float Hemisphere;
   float ConeFactor;
   float CosCentralLat, SinCentralLat;
   float StereoScale, InvScale;
   float CylinderScale;
   float RowIncKm;
   float ColIncKm;
} ProjectionContext;

/* Return the sign of x */
static float sign( float x )
{
   if (x<0.0) {
      return -1.0;
   }
   else if (x>0.0) {
      return 1.0;
   }
   else {
      return 0.0;
   }
}

static void pandg_back( float *lat, float *lon, float a, float b, float r )
{
  float pr, gr, pm, gm;

  /* NOTE - longitude sign switches - b too! */

  pr = DEG2RAD * *lat;
  gr = -DEG2RAD * *lon;
  pm = asin( cos(pr) * cos (gr) );
  gm = atan2(cos(pr) * sin (gr), -sin(pr) );

  *lat = RAD2DEG * asin( sin(a) * sin(pm) - cos(a) * cos(pm) * cos (gm - r) );
  *lon = -RAD2DEG * (-b + atan2(cos(pm) * sin (gm - r),
                   sin(a) * cos(pm) * cos (gm - r) + cos(a) * sin(pm)));
}


/*
 * Initialize all map projection stuff.
 * Input:  ctx - the projection context
 * Return:  1 = success, 0 = failure
 */
int setup_ctx_projection(ProjectionContext *ctx, int Projection, 
    float *projargs, int Nr, int Nc)
{
   float lat1, lat2;
   int i;

   /* BJW */
   ctx->Projection = Projection;
   ctx->Nr = Nr;
   ctx->Nc = Nc;

   switch (ctx->Projection) {
      case PROJ_GENERIC:
         /* FALL-THROUGH */
      case PROJ_LINEAR:
      case PROJ_CYLINDRICAL:
      case PROJ_SPHERICAL:
         ctx->NorthBound = projargs[0];
         ctx->WestBound  = projargs[1];
         ctx->RowInc     = projargs[2];
         ctx->ColInc     = projargs[3];
         break;
      case PROJ_MERCATOR:
         ctx->CentralLat = projargs[0];
         ctx->CentralLon = projargs[1];
         ctx->RowIncKm = projargs[2];
         ctx->ColIncKm = projargs[3];
         break;
      case PROJ_ROTATED:
         ctx->NorthBound = projargs[0];
         ctx->WestBound  = projargs[1];
         ctx->RowInc     = projargs[2];
         ctx->ColInc     = projargs[3];
         ctx->CentralLat = DEG2RAD * projargs[4];
         ctx->CentralLon = DEG2RAD * projargs[5];
         ctx->Rotation   = DEG2RAD * projargs[6];
         break;
      case PROJ_LAMBERT:
         ctx->Lat1       = projargs[0];
         ctx->Lat2       = projargs[1];
         ctx->PoleRow    = projargs[2];
         ctx->PoleCol    = projargs[3];
         ctx->CentralLon = projargs[4];
         ctx->ColInc     = projargs[5];
         break;
      case PROJ_STEREO:
         ctx->CentralLat = projargs[0];
         ctx->CentralLon = projargs[1];
         ctx->CentralRow = projargs[2];
         ctx->CentralCol = projargs[3];
         ctx->ColInc     = projargs[4];
         break;
      /* ZLB 02-09-2000 */
      case PROJ_GENERIC_NONEQUAL:
         for (i=0; i<ctx->Nr; i++) ctx->Longitude[i]=projargs[i];
         for (i=0; i<ctx->Nc; i++) ctx->Latitude[i]=projargs[i+ctx->Nr];
         ctx->NorthBound = ctx->Longitude[ctx->Nr-1];
         ctx->WestBound  = ctx->Latitude[ctx->Nc-1];
         ctx->RowInc     = (ctx->Longitude[ctx->Nr-1]-ctx->Longitude[0])/(ctx->Nr-1);
         ctx->ColInc     = (ctx->Latitude[ctx->Nc-1]-ctx->Latitude[0])/(ctx->Nc-1);
         break;
      default:
         printf("Error: unknown projection type in grid.c\n");
         return 0;
   }


   /*
    * Precompute useful values for coordinate transformations.
    */
   switch (ctx->Projection) {
      case PROJ_GENERIC:
      case PROJ_LINEAR:
      case PROJ_GENERIC_NONEQUAL: /* ZLB */
         ctx->SouthBound = ctx->NorthBound - ctx->RowInc * (ctx->Nr-1);
         ctx->EastBound = ctx->WestBound - ctx->ColInc * (ctx->Nc-1);
         break;
      case PROJ_LAMBERT:
         if (ctx->Lat1==ctx->Lat2) {
            /* polar stereographic? */
            if (ctx->Lat1>0.0) {
               lat1 = (90.0 - ctx->Lat1) * DEG2RAD;
            }
            else {
               lat1 = (90.0 + ctx->Lat1) * DEG2RAD;
            }
            ctx->Cone = cos( lat1 );
            ctx->Hemisphere = 1.0;

         }
         else {
            /* general Lambert conformal */
            float a, b;
            if (sign(ctx->Lat1) != sign(ctx->Lat2)) {
               printf("Error: standard latitudes must have the same sign.\n");
               return 0;
            }
            if (ctx->Lat1<ctx->Lat2) {
               printf("Error: Lat1 must be >= ctx->Lat2\n");
               return 0;
            }
            ctx->Hemisphere = 1.0;

            lat1 = (90.0 - ctx->Lat1) * DEG2RAD;
            lat2 = (90.0 - ctx->Lat2) * DEG2RAD;
            a = log(sin(lat1)) - log(sin(lat2));
            b = log( tan(lat1/2.0) ) - log( tan(lat2/2.0) );
            ctx->Cone = a / b;

         }

         /* Cone is in [-1,1] */
         ctx->ConeFactor = RADIUS * sin(lat1)
                          / (ctx->ColInc * ctx->Cone
                             * pow(tan(lat1/2.0), ctx->Cone) );

         break;
      case PROJ_STEREO:
         ctx->CosCentralLat = cos( ctx->CentralLat * DEG2RAD );
         ctx->SinCentralLat = sin( ctx->CentralLat * DEG2RAD );
         ctx->StereoScale = (2.0 * RADIUS / ctx->ColInc);
         ctx->InvScale = 1.0 / ctx->StereoScale;

         break;
      case PROJ_ROTATED:
         ctx->SouthBound = ctx->NorthBound - ctx->RowInc * (ctx->Nr-1);
         ctx->EastBound = ctx->WestBound - ctx->ColInc * (ctx->Nc-1);

         break;
      case PROJ_CYLINDRICAL:
         if (REVERSE_POLES==-1){
            ctx->CylinderScale = 1.0 / (-1.0*(-90.0-ctx->NorthBound));
         }
         else{
            ctx->CylinderScale = 1.0 / (90.0-ctx->SouthBound);
         }
         ctx->SouthBound = ctx->NorthBound - ctx->RowInc * (ctx->Nr-1);
         ctx->EastBound = ctx->WestBound - ctx->ColInc * (ctx->Nc-1);

         break;
      case PROJ_SPHERICAL:
         ctx->SouthBound = ctx->NorthBound - ctx->RowInc * (ctx->Nr-1);
         ctx->EastBound = ctx->WestBound - ctx->ColInc * (ctx->Nc-1);

         break;
      case PROJ_MERCATOR:
         break;
      default:
         printf("Error in setup_ctx_projection\n");
         return 0;
   }
   /* MJK 12.28.99 */
   /* ZLB 04.06.02 */
   if (ctx->Projection != PROJ_GENERIC &&
       ctx->Projection != PROJ_MERCATOR &&
       ctx->Projection != PROJ_GENERIC_NONEQUAL) {
/*
   if (ctx->Projection != PROJ_GENERIC) {
*/
     if (ctx->SouthBound < -90.0) {
       printf("SouthBound less than -90.0\n");
       return 0;
     }
     if (ctx->NorthBound < ctx->SouthBound) {
       printf("NorthBound less than SouthBound\n");
       return 0;
     }
     if (90.0 < ctx->NorthBound) {
       printf("NorthBound greater than 90.0\n");
       return 0;
     }
   }
   return 1;
}


void grid_to_geo (ProjectionContext *ctx, int n,
                  float row[], float col[], 
                  float lat[], float lon[])
{
   int i;

   switch (ctx->Projection) {
      case PROJ_GENERIC:
      case PROJ_LINEAR:
      case PROJ_CYLINDRICAL:
      case PROJ_SPHERICAL:
         for (i=0;i<n;i++) {
            lat[i] = ctx->NorthBound - row[i] * ctx->RowInc;
            lon[i] = ctx->WestBound  - col[i] * ctx->ColInc;
         }
         break;
      case PROJ_MERCATOR:
         {
            float YC, alpha, ic, jc;
            YC = RADIUS * log((1.0 + sin(DEG2RAD*ctx->CentralLat))/cos(DEG2RAD*ctx->CentralLat));
            ic = (ctx->Nr-1)/2.0;
            jc = (ctx->Nc-1)/2.0;
            for (i=0;i<n;i++) {
               alpha = ( (ic-row[i]) * ctx->RowIncKm + YC) / RADIUS;
               lat[i] = 2 * RAD2DEG * atan( exp(alpha) ) - 90.0;
               lon[i] = ctx->CentralLon - RAD2DEG * (col[i]-jc) * ctx->ColIncKm / RADIUS;
            }
         }
         break;
      case PROJ_LAMBERT:
         {
            float xldif, xedif, xrlon, radius;
 
            for (i=0;i<n;i++) {
               xldif = ctx->Hemisphere * (row[i]-ctx->PoleRow) / ctx->ConeFactor;
               xedif = (ctx->PoleCol-col[i]) / ctx->ConeFactor;
               if (xldif==0.0 && xedif==0.0)
                  xrlon = 0.0;
               else
                  xrlon = atan2( xedif, xldif );
               lon[i] = xrlon / ctx->Cone * RAD2DEG + ctx->CentralLon;
               if (lon[i] > 180.0)
                  lon[i] -= 360.0;

               radius = sqrt( xldif*xldif + xedif*xedif );
               if (radius < 0.0001)
                  lat[i] = 90.0 * ctx->Hemisphere;   /* +/-90 */
               else
                  lat[i] = ctx->Hemisphere
                         * (90.0 - 2.0*atan(exp(log(radius)/ctx->Cone))*RAD2DEG);
            }
         }
         break;
      case PROJ_STEREO:
         {
            float xrow, xcol, rho, c, cc, sc;
            
            for (i=0;i<n;i++) {
               xrow = ctx->CentralRow - row[i] - 1;
               xcol = ctx->CentralCol - col[i] - 1;
               rho = xrow*xrow + xcol*xcol;
               if (rho<1.0e-20) {
                  lat[i] = ctx->CentralLat;
                  lon[i] = ctx->CentralLon;
               }
               else {
                  rho = sqrt( rho );
                  c = 2.0 * atan( rho * ctx->InvScale);
                  cc = cos(c);
                  sc = sin(c);
                  lat[i] = RAD2DEG
                       * asin( cc*ctx->SinCentralLat
                               + xrow*sc*ctx->CosCentralLat / rho );
                  lon[i] = ctx->CentralLon + RAD2DEG * atan2( xcol * sc,
                            (rho * ctx->CosCentralLat * cc
                         - xrow * ctx->SinCentralLat * sc) );
                  if (lon[i] < -180.0)  lon[i] += 360.0;
                  else if (lon[i] > 180.0)  lon[i] -= 360.0; 

               }
            }
         }
         break;
      case PROJ_ROTATED:
         {
            float la, lo;

            for (i=0;i<n;i++) {
               lat[i] = ctx->NorthBound - row[i]
                           * (ctx->NorthBound-ctx->SouthBound) / (float) (ctx->Nr-1);
               lon[i] = ctx->WestBound - col[i]
                           * (ctx->WestBound-ctx->EastBound) / (float) (ctx->Nc-1);
               la = lat[i];
               lo = lon[i];
               pandg_back(&la, &lo, ctx->CentralLat, ctx->CentralLon, ctx->Rotation);
               lat[i] = la;
               lon[i] = lo;
            }
         }
         break;
      default:
         printf("Error in grid_to_geo\n");
   }
/* BJW
   for (i=0;i<n;i++) {
      hgt[i] = gridlevel_to_height( ctx, lev[i]);
   }
*/
}
