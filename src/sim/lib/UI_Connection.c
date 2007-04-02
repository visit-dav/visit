/***************************************************************************/
/*                          UI_Connection.c  -  description
*
* This files implements the simulation side of the connections to the Visit Custom UI.
* It decodes the signals sent over the wire from the visit QT signals and re-issues
* Them on this side.

 *                             -------------------
 * Programmer: Shelly Prevost
 * Date      : Tue Dec 6 2005
 *
 * Modifications:
 
/***************************************************************************/
 
#include "UI_Connection.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SIG_CONNECTIONS 100
static int lastSigConnection =0;
sigInfoConnect sigConnections[MAX_SIG_CONNECTIONS];


// ****************************************************************************
// Function: char *VisItParseCommand ( const char *cmd, char *signalName, char *buttonName )
//
// Purpose:
//   Relays the valued Changed signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   cmd        : Command to pass on to the connected slots
//   signalName : type of signal
//   buttonName : name of ui component that issued the signal
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

char *VisItParseCommand( const char *cmd, char *signalName, char *buttonName )
{
    char *str = strdup (cmd );
    char *tok = NULL;
    tok = strtok(str, ";" );
    if ( tok ) strcpy( signalName,tok );
    else
    {
        signalName[0] = '\0';
        fprintf(stderr, "parseCommand: signal  not found \n");
    }
    
    tok = strtok(NULL, ";" ); 
    if( tok) strcpy( buttonName, tok);
    else
    {
        buttonName[0] = '\0';
        fprintf(stderr,"parseCommand: invalid command \n");
    }
    return signalName;
}


// ****************************************************************************
// Function: void VisItClickedSignal ( const char *cmd )
//
// Purpose:
//   Relays the clicked signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   cmd : Command to pass on to the connected slots
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

void VisItClickedSignal( const char *cmd )
{
    int i;

    char sig[64];
    char ui_component[256];

    char *action = VisItParseCommand (cmd, sig, ui_component);
    for ( i =0; i < lastSigConnection; i++)
    {
        if ( !strcmp ( sigConnections[i].sig, action))
        {
          if ( !strcmp ( sigConnections[i].name, ui_component))
          {
            ((slotFunc)((sigConnections[i].slot)))(cmd);
          }
        }
    }

    fprintf(stderr, "SIGNAL Clicked with command '%s'\n", cmd);
}


// ****************************************************************************
// Function: VisItValueChangedSignal ( const char *cmd )
//
// Purpose:
//   Relays the valued Changed signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   cmd : Command to pass on to the connected slots
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

void VisItValueChangedSignal( const char *cmd )
{
    int i;
    char sig[64];
    char ui_component[256];

    char *action = VisItParseCommand (cmd, sig, ui_component);
    for ( i =0; i < lastSigConnection; i++)
    {
        if ( !strcmp ( sigConnections[i].sig, action))
        {
          if ( !strcmp ( sigConnections[i].name, ui_component))
          {
            (*sigConnections[i].slot)(cmd);
          }
        }
    }
    
    fprintf(stderr, "SIGNAL ValueChanged with command '%s'\n", cmd);
}



// ****************************************************************************
// Function: void VisItTextChangedSignal ( const char *cmd )
//
// Purpose:
//   Relays the valued Changed signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   cmd : Command to pass on to the connected slots
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

void VisItTextChangedSignal( const char *cmd )
{
     fprintf(stderr, "SIGNAL TextChanged with command '%s'\n", cmd);
}



// ****************************************************************************
// Function: void VisItActivatedSignal ( const char *cmd )
//
// Purpose:
//   Relays the valued Changed signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   cmd : Command to pass on to the connected slots
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

void VisItActivatedSignal( const char *cmd )
{
    fprintf(stderr, "SIGNAL Activated with command '%s'\n", cmd);
}



// ****************************************************************************
// Function: void VisItCurrentChangedSignal ( const char *cmd )
//
// Purpose:
//   Relays the valued Changed signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   cmd : Command to pass on to the connected slots
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

void VisItCurrentChangedSignal( const char *cmd )
{
     fprintf(stderr, "SIGNAL CurrentChanged with command '%s'\n", cmd);
}


// ****************************************************************************
// Function: void VisItAddConnection ( char * name, char* sig, slotFunc theSlot )
//
// Purpose:
//   Relays the valued Changed signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   name       : Name of the ui component that will issue the signal
//   sig        : type of signal
//   theSlot    : The callback that should be called with the named ui issues a sig
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

void VisItAddConnection( char * name, char* sig, slotFunc theSlot )
{
    if ( lastSigConnection < MAX_SIG_CONNECTIONS )
    {
        sigConnections[lastSigConnection].name = strdup (name);
        sigConnections[lastSigConnection].sig = strdup (sig);
        sigConnections[lastSigConnection].slot = theSlot;
        lastSigConnection++;
    }
    else  fprintf(stderr, "Maximum number of signal connections exceeded\n");
}



// ****************************************************************************
// Function: void VisItProcessCustomCommand ( const char *cmd)
//
// Purpose:
//   Relays the valued Changed signal from the VisIt custom UI to the simulation side
//
// Arguments:
//   cmd : Command to pass on to the connected slots
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

void VisItProcessCustomCommand( const char *cmd)
{
    // These are the signal that are generated by
    // the custom UI in VisIt
    int i;
    char sig[64];
    char ui_component[256];

    char *action = VisItParseCommand (cmd, sig, ui_component);
    if ( action != NULL && ui_component != NULL)
    {
        for ( i =0; i < lastSigConnection; i++)
        {
            if ( !strcmp ( sigConnections[i].sig, action))
            {
              if ( !strcmp ( sigConnections[i].name, ui_component))
              {
                (*sigConnections[i].slot)(cmd);
              }
            }
        }
    }
}
