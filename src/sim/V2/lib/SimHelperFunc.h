// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/*****************************************************************************
** Helper Functions:
**
** Purpose:
**   These are a bunch of functions that make it easier and less error
**   prone to use the Simulation Steering API.
**
** Notes: 
**
** Programmer: Shelly Prevost
** Creation:   Dec. 6, 2005
**
** Modifications:
**   Shelly Prevost, Tue Sep 12 16:08:13 PDT 2006
**   I added functions to overide generic button lables.
**   I added VisItCommandTypes enumerated types so that user
**   will have an easier time to use the generic UI
**
**   Shelly Prevost ri Oct 19 14:51:53 PDT 2007
**   Added the VisItGetVersion so that the simulation could verify
**   version numbers at runtime.
**
****************************************************************************/

 
#ifndef SIMHELPERFUNC_H
#define SIMHELPERFUNC_H

#ifdef __cplusplus
extern "C" {
#endif
/* set to maximum length of command name string  */
#define MAX_CMD_STR_LEN 64
typedef char *CMD_String[MAX_CMD_STR_LEN];
extern void VisItInitAllCMD(VisIt_SimulationMetaData *mdd, int MaxNumCustCMD  );
extern int VisItFindCMD (VisIt_SimulationMetaData mdd, char *name, int customCMD );
extern void VisItCreateCMD ( VisIt_SimulationMetaData mdd, char *name );
extern void VisItInitCMD ( VisIt_SimulationControlCommand *cmd );
extern void VisItSetCMDEnable (VisIt_SimulationMetaData mdd, char *name, int enabledCMD );
extern void VisItSetCMDIsOn (VisIt_SimulationMetaData mdd, char *name, int isOn);
extern void VisItSetCMDValue (VisIt_SimulationMetaData mdd, char *name, int value );
extern void VisItSetCMDText (VisIt_SimulationMetaData mdd, char *name, char *text );
extern void VisItSetCMD ( VisIt_SimulationMetaData mdd, VisIt_SimulationControlCommand cmd  );
extern void VisItUpdateMainSimWinGUI ( VisIt_SimulationMetaData mdd, char *name, char *data, int enable );
extern void VisItInitGenericCMD (VisIt_SimulationMetaData mdd, int index,char *name, char *text,char *value, int enable );
extern void VisItLabelGenericButton (VisIt_SimulationMetaData *mdd, int button, char *text,int enable );
extern void VisItSetStatusMessage (VisIt_SimulationMetaData mdd, char *text, char *color);
extern void VisItAddStripChartDataPoint( VisIt_SimulationMetaData mdd, double dataX,double dataY, int enable );
extern void VisItAddStripChartDataPointByName( VisIt_SimulationMetaData mdd, char *name, double dataX,double dataY, int enable );
extern void VisItSetStripChartTabName (VisIt_SimulationMetaData mdd, char *name, char *text );
enum VisItCommandTypes { STATUS_MESSAGE = 9, TIME_LABEL,TIME_VALUE, STEP_LABEL, STEP_VALUE};
extern char *VisItGetVersion();
#ifdef __cplusplus
}
#endif
#endif
