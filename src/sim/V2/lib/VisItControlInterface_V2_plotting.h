#ifndef VISIT_CONTROL_INTERFACE_V2_PLOTTING_H
#define VISIT_CONTROL_INTERFACE_V2_PLOTTING_H

/* This file contains some experimental functions that are designed to let 
 * the sim set up plots directly. These functions could be deprectated in
 * favor of another approach.
 *
 * Programmer: Brad Whitlock
 * Date: Oct 2012 - May 2013
 */

/* Prototype plot setup/management functions. */
int VisItAddPlot(const char *plotType, const char *var);
int VisItAddOperator(const char *operatorType, int applyToAll);
int VisItDrawPlots(void);
int VisItDeleteActivePlots(void);

/* Maybe having 1 function is better...*/
int VisItSetPlotOptionsC(int id,const char*n,char v);
int VisItSetPlotOptionsUC(int id,const char*n,unsigned char v);
int VisItSetPlotOptionsI(int id,const char*n,int v);
int VisItSetPlotOptionsL(int id,const char*n,long v);
int VisItSetPlotOptionsF(int id,const char*n,float v);
int VisItSetPlotOptionsD(int id,const char*n,double v);
int VisItSetPlotOptionsS(int id,const char*n,const char *v);

int VisItSetPlotOptionsCv(int id,const char*n,const char *v,int L);
int VisItSetPlotOptionsUCv(int id,const char*n,const unsigned char *v,int L);
int VisItSetPlotOptionsIv(int id,const char*n,const int *v,int L);
int VisItSetPlotOptionsLv(int id,const char*n,const long *v,int L);
int VisItSetPlotOptionsFv(int id,const char*n,const float *v,int L);
int VisItSetPlotOptionsDv(int id,const char*n,const double *v,int L);
int VisItSetPlotOptionsSv(int id,const char*n,const char **v,int L);

/* Maybe having 1 function is better...*/
int VisItSetOperatorOptionsC(int pid, int oid,const char*n,char v);
int VisItSetOperatorOptionsUC(int pid, int oid,const char*n,unsigned char v);
int VisItSetOperatorOptionsI(int pid, int oid,const char*n,int v);
int VisItSetOperatorOptionsL(int pid, int oid,const char*n,long v);
int VisItSetOperatorOptionsF(int pid, int oid,const char*n,float v);
int VisItSetOperatorOptionsD(int pid, int oid,const char*n,double v);
int VisItSetOperatorOptionsS(int pid, int oid,const char*n,const char *v);

int VisItSetOperatorOptionsCv(int pid, int oid,const char*n,const char *v,int L);
int VisItSetOperatorOptionsUCv(int pid, int oid,const char*n,const unsigned char *v,int L);
int VisItSetOperatorOptionsIv(int pid, int oid,const char*n,const int *v,int L);
int VisItSetOperatorOptionsLv(int pid, int oid,const char*n,const long *v,int L);
int VisItSetOperatorOptionsFv(int pid, int oid,const char*n,const float *v,int L);
int VisItSetOperatorOptionsDv(int pid, int oid,const char*n,const double *v,int L);
int VisItSetOperatorOptionsSv(int pid, int oid,const char*n,const char **v,int L);

#endif
