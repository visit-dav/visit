#include "stdio.h"
#include "string.h"

/* all calls are assumed to be written to the same file, unless the name changes */

static char gPreviousfile[2048]=""; 
static FILE *gFp = NULL; 


FILE *inventor_openfile(char *filename) {
  if (!gFp || strncmp(gPreviousfile, filename, 2048)) {
    strncpy(gPreviousfile, filename, 2047); 
    if (gFp)  {
      fclose(gFp); 
    }
    gFp = fopen(filename, "w"); 
    if (!gFp) {
      printf("Error in drawhex:  could not open file %s\n", filename);       
    }
    else {
      fprintf(gFp, "#Inventor V2.0 ascii\n");
      fprintf(gFp, "Environment { ambientIntensity  1.1 \n");
      fprintf(gFp, "ambientColor      1 1 1}\n");
    }
  }
  return gFp; 
} 

int inventor_comment(char *filename, char *comment) {
  FILE *fp = inventor_openfile(filename); 
  if (!fp) return 1; 
  fprintf(gFp, "# %s\n", comment); 
  return 0;
}
  /* draw a hexahedron in Inventor 2.0 ascii format  (use ivview to view). 
   If same filename is given as previous, then just continues writing in the previous file. Else opens a new file. 
   If x,y,z given, then assumes x, y and z are allocated to hold 8 floats each, one for each vertex, in standard HAR node configuration (0,1,2,3 on bottom, 4,5,6,7 on top, bottom and top nodes ordered so that bottom face has normal pointing IN by right hand rule and top face normal points out, and draws that hex.
   color is RGB float triplet, if color is NULL, then just use white
     return 0 if success
*/
int inventor_drawhex(char *filename, double *x, double *y, double *z, float *color) {
  float default_color[3] = {1,1,1}; 
  int i=0; 
  FILE *fp = inventor_openfile(filename); 
  if (!fp) return 1; 

  if (!x || !y || !z) {
    return 0; 
  }
  
  fprintf(fp, "Separator {\n");
  fprintf(fp, "DrawStyle { style LINES }\n");
  
  if (!color) color = default_color; 
  fprintf(fp, "Material { ambientColor %f %f %f\n", color[0], color[1], color[2]);
  fprintf(fp, "\tdiffuseColor 0 0 0 }\n");
  
  fprintf(fp, "Coordinate3 {  point [\n");
  for (i=0; i<8; i++) {
    fprintf(fp, "%g %g %g,\n", x[i], y[i], z[i]); /* commas */
  }
  /*fprintf(fp, "%g, %g, %g\n", x[i], y[i], z[i]); no comma */
  
  fprintf(fp, "  ] }\n");
  
  fprintf(fp, "IndexedFaceSet { coordIndex [\n");
  fprintf(fp, "0,1,2,3,-1,\n");
  fprintf(fp, "4,7,6,5,-1,\n");
  fprintf(fp, "0,4,5,1,-1,\n");
  fprintf(fp, "1,5,6,2,-1,\n");
  fprintf(fp, "2,6,7,3,-1,\n");
  fprintf(fp, "3,7,4,0,-1,\n");
  fprintf(fp, "] } }");
  fprintf(fp, "\n");
  
  return 0; 
}

 /* same as drawhex, but make a tet (duh), use four nodes instead of 8, duh  -- again, assume base normal faces inward by right hand rule, but it really doesn't matter AFAICT */
int inventor_drawtet(char *filename, double *x, double *y, double *z, float *color) {
  float default_color[3] = {1,1,1}; 
  int i=0; 
  FILE *fp = inventor_openfile(filename); 
  if (!fp) return 1; 

  if (!x || !y || !z) {
    return 0; 
  }
  
  fprintf(fp, "Separator {\n");
  fprintf(fp, "DrawStyle { style LINES }\n");
  
  if (!color) color = default_color; 
  fprintf(fp, "Material { ambientColor %f %f %f\n", color[0], color[1], color[2]);
  fprintf(fp, "\tdiffuseColor 0 0 0 }\n");
  
  fprintf(fp, "Coordinate3 {  point [\n");
  for (i=0; i<4; i++) {
    fprintf(fp, "%g %g %g,\n", x[i], y[i], z[i]); /* commas */
  }
  
  fprintf(fp, "  ] }\n");
  
  fprintf(fp, "IndexedFaceSet { coordIndex [\n");
  fprintf(fp, "0,1,2,-1,\n");
  fprintf(fp, "0,3,1,-1,\n");
  fprintf(fp, "1,3,2,-1,\n");
  fprintf(fp, "0,2,3,-1,\n");
  fprintf(fp, "] } }");
  fprintf(fp, "\n");
  
  return 0; 
}

/* draw a polygon of numnodes, positions given by x,y,z, if color is NULL, then white*/
int inventor_drawpolygon(char *filename, double *x, double *y, double *z, int numnodes, float *color) {
  float default_color[3] = {1,1,1}; 
  int i=0; 
  FILE *fp = inventor_openfile(filename); 
  if (!fp) return 1; 

  if (!x || !y || !z) {
    return 0; 
  }
  
  fprintf(fp, "Separator {\n");
  fprintf(fp, "DrawStyle { style LINES }\n");
  
  if (!color) color = default_color; 
  fprintf(fp, "Material { ambientColor %f %f %f\n", color[0], color[1], color[2]);
  fprintf(fp, "\tdiffuseColor 0 0 0 }\n");
  
  fprintf(fp, "Coordinate3 {  point [\n");
  for (i=0; i<numnodes; i++) {
    fprintf(fp, "%g %g %g,\n", x[i], y[i], z[i]); /* commas */
  }
  
  fprintf(fp, "  ] }\n");
  
  fprintf(fp, "IndexedFaceSet { coordIndex [\n");
  for (i=0; i< numnodes; i++) {
    fprintf(fp, "%d, ", i);
  }
  fprintf(fp, "0 -1, \n"); 
  fprintf(fp, "] } }");
  fprintf(fp, "\n");
  
  return 0; 
}

/* draw an X at the given point */
int inventor_drawXatPoint(char *filename, double x, double y, double z, float size, float *color) {
  float default_color[3] = {1,1,1}; 
  FILE *fp = inventor_openfile(filename); 
  if (!fp) return 1; 

  
  fprintf(fp, "Separator {\n");
  fprintf(fp, "DrawStyle { style LINES }\n");
  
  if (!color) color = default_color; 
  fprintf(fp, "Material { ambientColor %f %f %f\n", color[0], color[1], color[2]);
  fprintf(fp, "\tdiffuseColor 0 0 0 }\n");
  
  fprintf(fp, "Coordinate3 {  point [\n");
  fprintf(fp, "%g %g %g,\n", x+size, y, z+size); /* commas */
  fprintf(fp, "%g %g %g,\n", x+size, y, z-size); /* commas */
  fprintf(fp, "%g %g %g,\n", x-size, y, z+size); /* commas */
  fprintf(fp, "%g %g %g,\n", x-size, y, z-size); /* commas */
  fprintf(fp, "%g %g %g,\n", x,y, z); /* commas */
  
  fprintf(fp, "  ] }\n");
  
  fprintf(fp, "IndexedFaceSet { coordIndex [\n");
  fprintf(fp, "0,1,4-1\n"); 
  fprintf(fp, "0,3,4 -1,\n");
  fprintf(fp, "2,1,4 -1,\n");
  fprintf(fp, "2,3,4 -1,\n");
  fprintf(fp, "] } }");
  fprintf(fp, "\n");
  
  return 0; 
}
  

