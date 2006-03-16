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

void VisItProcessCustomCommand( const char *cmd);
void VisItAddConnection(char * name, char* sig, slotFunc theSlot );


#endif
