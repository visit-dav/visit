#ifndef SIMHELPERFUNC_H
#define SIMHELPERFUNC_H

void Update_UI_Commands();
extern void VisItInitAllCMD(VisIt_SimulationMetaData *mdd, int MaxNumCustCMD  );
extern int VisItFindCMD (VisIt_SimulationMetaData mdd, char *name );
extern void VisItCreateCMD ( VisIt_SimulationMetaData mdd, char *name );
extern void VisItInitCMD ( VisIt_SimulationControlCommand *cmd );
extern void VisItSetCMDEnable (VisIt_SimulationMetaData mdd, char *name, int enabledCMD );
extern void VisItSetCMDIsOn (VisIt_SimulationMetaData mdd, char *name, int isOn);
extern void VisItSetCMDValue (VisIt_SimulationMetaData mdd, char *name, int value );
extern void VisItSetCMDText (VisIt_SimulationMetaData mdd, char *name, char *text );
extern void VisItSetCMD ( VisIt_SimulationMetaData mdd, VisIt_SimulationControlCommand cmd  );


#endif
