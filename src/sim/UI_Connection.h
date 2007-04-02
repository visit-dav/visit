#ifndef UI_CONNECTION_H
#define UI_CONNECTION_H

/***************************************************************************/
/*                          UI_Connection.h  -  description
*
* This files implements the simulation side of the connections to the Visit Custom UI.
* It decodes the signals sent over the wire from the visit QT signals and re-issues
* Them on this side.

/*                             -------------------
/* Programmer: Shelly Prevost
 * Date      : Tue Dec 6 2005
 *
 * Modifications:

/***************************************************************************/

typedef void (*slotFunc) (char * cmd);
typedef struct signalInfo { char *name; char* sig; slotFunc slot; } sigInfoConnect;


//void ClickedSignal ( const char *cmd );
//void ValueChangedSignal ( const char *cmd );
void ProcessCustomCommand ( const char *cmd);
void addConnection ( char * name, char* sig, slotFunc theSlot );


#endif
