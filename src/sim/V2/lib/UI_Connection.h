// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef UI_CONNECTION_H
#define UI_CONNECTION_H

/*****************************************************************************
** Helper Functions:
**
** Purpose:
**   These function handle the connecting to VisIt from the simulation. It makes it
**   far less error prone to use the Simulation Steering API.
**
** Notes:
**
** Programmer: Shelly Prevost
** Creation:   Dec. 6, 2005
**
** Modifications:
**
**
****************************************************************************/

typedef void (*slotFunc) (char * cmd);
typedef struct signalInfo { char *name; char* sig; slotFunc slot; } sigInfoConnect;

#ifdef __cplusplus
extern "C" {
#endif
void  VisItProcessCustomCommand( char *cmd);
void  VisItAddConnection(char * name, char* sig, slotFunc theSlot );
char *VisItParseCommand( char *cmd, char *signalName, char *buttonName );
int   VisItGetCommandValue( char *cmd, double *retVal );
void  VisItParseCommandValue( char *cmd, double *retVal);
#ifdef __cplusplus
}
#endif

#endif
