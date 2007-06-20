/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
#include <VisItDataInterface_V1.h>
#include "UI_Connection.h"
#include "SimHelperFunc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SIG_CONNECTIONS 100
static int lastSigConnection =0;
sigInfoConnect sigConnections[MAX_SIG_CONNECTIONS];


/*****************************************************************************
** Function: char *VisItParseCommand ( const char *cmd, char *signalName, char *buttonName )
**
** Purpose:
**   Relays the valued Changed signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   cmd        : Command to pass on to the connected slots
**   signalName : type of signal
**   buttonName : name of ui component that issued the signal
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** *****************************************************************************/

char *VisItParseCommand( char *cmd, char *signalName, char *buttonName )
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

/*****************************************************************************
** Function: int VisItGetCommandValue ( const char *cmd, char *signalName, char *buttonName )
**
** Purpose:
**   parse the command string and return the value in retVal
**
** Arguments:
**   cmd        : Command passed from VisIt
**   retValue   : the value in the value position of the cmd string 
**
**   returns an error ( 0 ) if no value is found.
**
** Programmer: Shelly Prevost
** Creation:   Fri Jun 15 15:09:48 PDT 2007
**
** Modifications:
**
** *****************************************************************************/
int VisItGetCommandValue( char *cmd, double *retVal)
{
    int i;
    char value[MAX_CMD_STR_LEN];
    char *str = strdup (cmd );
    char *tok = NULL;
    tok = strtok(str, ";" );
    for ( i = 0; i < 4; i++)
    {
    	tok = strtok(NULL, ";" );
        if ( tok ) strncpy( value,tok,MAX_CMD_STR_LEN );
	else return 0;
    }
    *retVal = atof ( value );
    return 1;
}

/*****************************************************************************
** Function: void VisItClickedSignal ( const char *cmd )
**
** Purpose:
**   Relays the clicked signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   cmd : Command to pass on to the connected slots
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** ****************************************************************************/

void VisItClickedSignal( char *cmd )
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


/******************************************************************************
** Function: VisItValueChangedSignal ( const char *cmd )
**
** Purpose:
**   Relays the valued Changed signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   cmd : Command to pass on to the connected slots
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** *****************************************************************************/

void VisItValueChangedSignal( char *cmd )
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



/*****************************************************************************
** Function: void VisItTextChangedSignal ( const char *cmd )
**
** Purpose:
**   Relays the valued Changed signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   cmd : Command to pass on to the connected slots
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** *****************************************************************************/

void VisItTextChangedSignal( const char *cmd )
{
     fprintf(stderr, "SIGNAL TextChanged with command '%s'\n", cmd);
}



/*****************************************************************************
** Function: void VisItActivatedSignal ( const char *cmd )
**
** Purpose:
**   Relays the valued Changed signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   cmd : Command to pass on to the connected slots
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** *****************************************************************************/

void VisItActivatedSignal( const char *cmd )
{
    fprintf(stderr, "SIGNAL Activated with command '%s'\n", cmd);
}



/*****************************************************************************
** Function: void VisItCurrentChangedSignal ( const char *cmd )
**
** Purpose:
**   Relays the valued Changed signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   cmd : Command to pass on to the connected slots
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** **************************************************************************** */

void VisItCurrentChangedSignal( const char *cmd )
{
     fprintf(stderr, "SIGNAL CurrentChanged with command '%s'\n", cmd);
}


/*****************************************************************************
** Function: void VisItAddConnection ( char * name, char* sig, slotFunc theSlot )
**
** Purpose:
**   Relays the valued Changed signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   name       : Name of the ui component that will issue the signal
**   sig        : type of signal
**   theSlot    : The callback that should be called with the named ui issues a sig
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** *****************************************************************************/

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



/******************************************************************************
** Function: void VisItProcessCustomCommand ( const char *cmd)
**
** Purpose:
**   Relays the valued Changed signal from the VisIt custom UI to the simulation side
**
** Arguments:
**   cmd : Command to pass on to the connected slots
**
** Programmer: Shelly Prevost
** Creation:   Tue Dec 6 2005
**
** Modifications:
**
** *****************************************************************************/

void VisItProcessCustomCommand( char *cmd)
{
    /*These are the signal that are generated by
    the custom UI in VisIt    */
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
