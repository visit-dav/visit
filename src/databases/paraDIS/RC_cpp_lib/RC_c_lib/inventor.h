#ifndef INVENTOR_H
#define INVENTOR_H

int inventor_drawhex(char *filename, double* x, double* y, double *z, float *color); 
int inventor_drawtet(char *filename, double *x, double *y, double *z, float *color);
int inventor_drawpolygon(char *filename, double *x, double *y, double *z, int numnodes, float *color) ;
int inventor_drawXatPoint(char *filename, double x, double y, double z, float size, float *color);
int inventor_comment(char *filename, char *comment);
#endif
