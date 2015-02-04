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
int VisItSetActivePlots(const int *ids, int nids);

/* Set values for the current plot */
int VisItSetPlotOptionsC(const char *name,char v);
int VisItSetPlotOptionsUC(const char *name,unsigned char v);
int VisItSetPlotOptionsB(const char *name,int v);
int VisItSetPlotOptionsI(const char *name,int v);
int VisItSetPlotOptionsL(const char *name,long v);
int VisItSetPlotOptionsF(const char *name,float v);
int VisItSetPlotOptionsD(const char *name,double v);
int VisItSetPlotOptionsS(const char *name,const char *v);

int VisItSetPlotOptionsCv(const char *name,const char *v,int L);
int VisItSetPlotOptionsUCv(const char *name,const unsigned char *v,int L);
int VisItSetPlotOptionsIv(const char *name,const int *v,int L);
int VisItSetPlotOptionsLv(const char *name,const long *v,int L);
int VisItSetPlotOptionsFv(const char *name,const float *v,int L);
int VisItSetPlotOptionsDv(const char *name,const double *v,int L);
int VisItSetPlotOptionsSv(const char *name,const char **v,int L);

/* Set values for the current operator */
int VisItSetOperatorOptionsC(const char *name,char v);
int VisItSetOperatorOptionsUC(const char *name,unsigned char v);
int VisItSetOperatorOptionsB(const char *name,int v);
int VisItSetOperatorOptionsI(const char *name,int v);
int VisItSetOperatorOptionsL(const char *name,long v);
int VisItSetOperatorOptionsF(const char *name,float v);
int VisItSetOperatorOptionsD(const char *name,double v);
int VisItSetOperatorOptionsS(const char *name,const char *v);

int VisItSetOperatorOptionsCv(const char *name,const char *v,int L);
int VisItSetOperatorOptionsUCv(const char *name,const unsigned char *v,int L);
int VisItSetOperatorOptionsIv(const char *name,const int *v,int L);
int VisItSetOperatorOptionsLv(const char *name,const long *v,int L);
int VisItSetOperatorOptionsFv(const char *name,const float *v,int L);
int VisItSetOperatorOptionsDv(const char *name,const double *v,int L);
int VisItSetOperatorOptionsSv(const char *name,const char **v,int L);

#endif
