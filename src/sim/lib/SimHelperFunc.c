/* Programmer: Shelly Prevost
 * Date      : Jan 12, 2006
 *
 * Modifications:
 *    
 */

#include <VisItControlInterface_V1.h>
#include <VisItDataInterface_V1.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// set to maximum length of command name string
#define MAX_CMD_STR_LEN 64

// current number of active channels to the custom
// UI update information in the meta data.
int VisIt_CurrentIndex = 0;


// ****************************************************************************
// Function: int VisItFindCMD ( char *name )
//
// Purpose:
//   Searches for a match between name and a name in the metat data custom
//   command array. If it does not find it findCMD returns -1.
//   If it does find a match findCMD return the index for the command
//
// Arguments:
//   name : name of the ui component to search for.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************

int VisItFindCMD (VisIt_SimulationMetaData mdd, char *name )
{
    int found = -1;
    int i;

    for ( i =0; i < mdd.numCustomCommands; i++)
    {
      if ( !strcmp (mdd.customCommands[i].name, name))
      {
        found = i;
      }
    }
    return found;
}

// ****************************************************************************
// Function: void VisItCreateCMD ( char *name )
//
// Purpose:
//   Creates a entry in the meta data to allow updating the ui
//   component that is named name. The entry will not be created
//   if it exceeds the max number of commands or if it is already
//   in the array.
//
// Arguments:
//   name : name of the ui component to search for.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItCreateCMD ( VisIt_SimulationMetaData mdd, char *name )
{
  int index;

  if ( VisIt_CurrentIndex < mdd.numCustomCommands)
  {
      index = VisItFindCMD (mdd, name );
      if ( index < 0 )
      strncpy(mdd.customCommands[VisIt_CurrentIndex].name, name,MAX_CMD_STR_LEN-1);
      VisIt_CurrentIndex++;
  }

}

// ****************************************************************************
// Function: void VisItInitCMD ( VisItSimulationControlCommand *cmd )
//
// Purpose:
//   This function initialize the meta data structure so that
//   it is in a know state.
//
// Arguments:
//   cmd  : the structure in need of initialization
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItInitCMD ( VisIt_SimulationControlCommand *cmd )
{
    cmd->name = malloc(MAX_CMD_STR_LEN);
    strncpy ( cmd->name, "VISIT_NONE",MAX_CMD_STR_LEN);
    cmd->argType = VISIT_CMDARG_NONE;
    cmd->enabled = 1;
    cmd->signal= malloc(MAX_CMD_STR_LEN);
    strncpy ( cmd->name, "VISIT_NONE",MAX_CMD_STR_LEN);
    cmd->text= malloc(MAX_CMD_STR_LEN);
    strncpy ( cmd->text, "VISIT_NONE",MAX_CMD_STR_LEN);
    cmd->className = malloc(MAX_CMD_STR_LEN);
    cmd->className[0] = '\0';
    cmd->value = malloc(MAX_CMD_STR_LEN);
    strncpy ( cmd->value, "10",MAX_CMD_STR_LEN);
    cmd->uiType = malloc(MAX_CMD_STR_LEN);
    cmd->uiType[0] = '\0';
    cmd->isOn = 1;
    cmd->parent = malloc(MAX_CMD_STR_LEN);
    cmd->parent[0] = '\0';

}

// ****************************************************************************
// Function: void VisItSetCMDEnable (char *name, int enabledCMD )
//
// Purpose:
//   this is a helper function to look up the ui component by
//   by name and then set the enable variable.
//
// Arguments:
//   name   : name of the ui component to search for.
//   enable : 0 for false and 1 for true.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItSetCMDEnable (VisIt_SimulationMetaData mdd, char *name, int enabledCMD )
{
    int index;

    index = VisItFindCMD (mdd, name);
    if ( (index >= 0) && ( index < mdd.numCustomCommands))
    {
      mdd.customCommands[index].enabled = enabledCMD;
    }
}

// ****************************************************************************
// Function: void VisItSetCMDIsOn (char *name, int isOn)
//
// Purpose:
//   this is a helper function to look up the ui component by
//   by name and then set the isOn variable.
//
// Arguments:
//   name   : name of the ui component to search for.
//   enable : 0 for false and 1 for true.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItSetCMDIsOn (VisIt_SimulationMetaData mdd, char *name, int isOn )
{
    int index;

    index = VisItFindCMD ( mdd, name);
    if ( (index >= 0) && ( index < mdd.numCustomCommands))
    {
      mdd.customCommands[index].isOn = isOn;
    }
}

// ****************************************************************************
// Function: void VisItSetCMDValue (char *name, int value )
//
// Purpose:
//   Searches for a match between name and a name in the metat data custom
//   command array. If it does not find a match it sets the meta data for
//   that entry to value.
//
// Arguments:
//   name   : name of the ui component to search for.
//   value  : the value to set the found component too.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItSetCMDValue (VisIt_SimulationMetaData mdd, char *name, int value )
{
    int index;
    char strValue[MAX_CMD_STR_LEN];

    sprintf (  strValue, "%5d", value);
    index = VisItFindCMD ( mdd, name);
    if ( (index >= 0) && ( index < mdd.numCustomCommands))
    {
      strncpy(mdd.customCommands[index].value, strValue,  MAX_CMD_STR_LEN-1);
    }
}

// ****************************************************************************
// Function: void VisItSetCMDText (char *name, char *text )
//
// Purpose:
//   Searches for a match between name and a name in the metat data custom
//   command array. If it does not find a match it sets the text value for
//   that ui entry too text.
//
// Arguments:
//   name : name of the ui component to search for.
//   text : the text to change it too.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItSetCMDText (VisIt_SimulationMetaData mdd, char *name, char *text )
{
    int index;

    index = VisItFindCMD (mdd, name);
    if ( (index >= 0) && ( index < mdd.numCustomCommands))
    {
      strncpy(mdd.customCommands[index].text, text,  MAX_CMD_STR_LEN-1);
    }
}


// ****************************************************************************
// Function: setCMD ( VisItSimulationControlCommand cmd  )
//
// Purpose:
//   Searches for a match between cmd.name and a name in the metat data custom
//   command array. If it does find a match it sets all the values in that
//   entry to all the values in cmd
//
// Arguments:
//   cmd : data structure to copy to the matching entry.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItSetCMD ( VisIt_SimulationMetaData mdd, VisIt_SimulationControlCommand cmd  )
{
    int index;

    index = VisItFindCMD ( mdd, cmd.name);
    if ( (index >= 0) && ( index < mdd.numCustomCommands))
    {
        mdd.customCommands[index].argType = cmd.argType;
        mdd.customCommands[index].enabled = cmd.enabled;
        mdd.customCommands[index].isOn = cmd.isOn;
        strncpy(mdd.customCommands[index].name,       cmd.name,     MAX_CMD_STR_LEN-1);
        strncpy(mdd.customCommands[index].signal,     cmd.signal,   MAX_CMD_STR_LEN-1);
        strncpy(mdd.customCommands[index].text,       cmd.text,     MAX_CMD_STR_LEN-1);
        strncpy(mdd.customCommands[index].className,  cmd.className,MAX_CMD_STR_LEN-1);
        strncpy(mdd.customCommands[index].value,      cmd.value,    MAX_CMD_STR_LEN-1);
        strncpy(mdd.customCommands[index].uiType,     cmd.uiType,   MAX_CMD_STR_LEN-1);
        strncpy(mdd.customCommands[index].parent,     cmd.parent,   MAX_CMD_STR_LEN-1);
    }
}



// ****************************************************************************
// Function: void VisItInitAllCMD(int MaxNumCustCMD  )
//
// Purpose:
//   This functions initializes the meta data so that all the command arrays are
//   initialized to some type of valid data. This prevent problems later on if
//   the meta is sent to VisIt before it has be set.
//
// Arguments:
//   MaxNumCustCMD  : maximum number of command channels that will be open at
//   anyone time.
//
// Programmer: Shelly Prevost
// Creation:   Tue Dec 6 2005
//
// Modifications:
//
// ****************************************************************************
void VisItInitAllCMD(VisIt_SimulationMetaData *mdd, int MaxNumCustCMD  )
{
    // These are the definitions for the
    // generic commands found in the first window
    // when VisIt open the first simulation window
   mdd->numGenericCommands = 6;
   mdd->genericCommands = malloc(sizeof(VisIt_SimulationControlCommand) *mdd->numGenericCommands);

   mdd->genericCommands[0].name = strdup("halt");
   mdd->genericCommands[0].argType = VISIT_CMDARG_NONE;
   mdd->genericCommands[0].enabled = 1;

   mdd->genericCommands[1].name = strdup("step");
   mdd->genericCommands[1].argType = VISIT_CMDARG_NONE;
   mdd->genericCommands[1].enabled = 1;

   mdd->genericCommands[2].name = strdup("run");
   mdd->genericCommands[2].argType = VISIT_CMDARG_NONE;
   mdd->genericCommands[2].enabled = 1;

   mdd->genericCommands[3].name = strdup("restart");
   mdd->genericCommands[3].argType = VISIT_CMDARG_NONE;
   mdd->genericCommands[3].enabled = 1;

   mdd->genericCommands[4].name = strdup("sync");
   mdd->genericCommands[4].argType = VISIT_CMDARG_NONE;
   mdd->genericCommands[4].enabled = 1;

   mdd->genericCommands[5].name = strdup("custom");
   mdd->genericCommands[5].argType = VISIT_CMDARG_NONE;
   mdd->genericCommands[5].enabled = 1;

   mdd->numCustomCommands = MaxNumCustCMD;
   mdd->customCommands = malloc(sizeof(VisIt_SimulationControlCommand) *mdd->numCustomCommands);

    int i;
    for ( i =0; i <mdd->numCustomCommands; i++) VisItInitCMD(&mdd->customCommands[i]);
}
