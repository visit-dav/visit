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

static int lastSigConnection =0;
sigInfoConnect sigConnections[100];


// ****************************************************************************
// Function: char *parseCommand ( const char *cmd, char *signalName, char *buttonName )
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

char *parseCommand ( const char *cmd, char *signalName, char *buttonName )
{
    char *str = strdup (cmd );
    strcpy( signalName, strtok(str, ";" ));
    strcpy( buttonName, strtok(NULL, ";"));
#ifdef DEBUG_PRINT
    printf("name and ui comp = %s %s \n",signalName, buttonName);
#endif

//    fprintf(stderr, "token = %s with command '%s'\n", str2,cmd);
    return signalName;
}


// ****************************************************************************
// Function: void ClickedSignal ( const char *cmd )
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

void ClickedSignal ( const char *cmd )
{
    int i;

    char sig[64];
    char ui_component[256];

    char *action = parseCommand (cmd, sig, ui_component);
#ifdef DEBUG_PRINT
    printf("looking for name = %s \n", ui_component);
#endif
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
#ifdef DEBUG_PRINT
    fprintf(stderr, "SIGNAL Clicked with command '%s'\n", cmd);
#endif
}


// ****************************************************************************
// Function: ValueChangedSignal ( const char *cmd )
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

void ValueChangedSignal ( const char *cmd )
{
    int i;
    char sig[64];
    char ui_component[256];

    char *action = parseCommand (cmd, sig, ui_component);
#ifdef DEBUG_PRINT
    printf("looking for name = %s \n", ui_component);
#endif
    for ( i =0; i < lastSigConnection; i++)
    {
         //if ( !strcmp ( sigConnections[i].sig, "clicked()"))
        if ( !strcmp ( sigConnections[i].sig, action))
        {
            if ( !strcmp ( sigConnections[i].name, ui_component))
            {
                (*sigConnections[i].slot)(cmd);
            }
        }
    }
#ifdef DEBUG_PRINT
    fprintf(stderr, "SIGNAL ValueChanged with command '%s'\n", cmd);
#endif
}



// ****************************************************************************
// Function: void TextChangedSignal ( const char *cmd )
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

void TextChangedSignal ( const char *cmd )
{
#ifdef DEBUG_PRINT
    fprintf(stderr, "SIGNAL TextChanged with command '%s'\n", cmd);
#endif
}



// ****************************************************************************
// Function: void ActivatedSignal ( const char *cmd )
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

void ActivatedSignal ( const char *cmd )
{
#ifdef DEBUG_PRINT
    fprintf(stderr, "SIGNAL Activated with command '%s'\n", cmd);
#endif
}



// ****************************************************************************
// Function: void CurrentChangedSignal ( const char *cmd )
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

void CurrentChangedSignal ( const char *cmd )
{
#ifdef DEBUG_PRINT
    fprintf(stderr, "SIGNAL CurrentChanged with command '%s'\n", cmd);
#endif
}


// ****************************************************************************
// Function: void addConnection ( char * name, char* sig, slotFunc theSlot )
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

void addConnection ( char * name, char* sig, slotFunc theSlot )
{
    sigConnections[lastSigConnection].name = strdup (name);
    sigConnections[lastSigConnection].sig = strdup (sig);
    sigConnections[lastSigConnection].slot = theSlot;
#ifdef DEBUG_PRINT
    printf ( "Added new connection %s %s \n", name, sig);
#endif
    lastSigConnection++;
}



// ****************************************************************************
// Function: void ProcessCustomCommand ( const char *cmd)
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

void ProcessCustomCommand ( const char *cmd)
{   // These are the signal that are generated by
    // the custom UI in VisIt


    int i;
    char sig[64];
    char ui_component[256];

    char *action = parseCommand (cmd, sig, ui_component);
#ifdef DEBUG_PRINT
    printf("looking for name = %s with sig = %s \n", ui_component, action);
#endif
    for ( i =0; i < lastSigConnection; i++)
    {
         //if ( !strcmp ( sigConnections[i].sig, "clicked()"))
        if ( !strcmp ( sigConnections[i].sig, action))
        {
            if ( !strcmp ( sigConnections[i].name, ui_component))
            {
                (*sigConnections[i].slot)(cmd);
            }
        }
    }
#ifdef DEBUG_PRINT
    fprintf(stderr, "SIGNAL %s with command '%s'\n",action, cmd);
#endif
}
