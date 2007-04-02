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
****************************************************************************/

 
#ifndef SIMHELPERFUNC_H
#define SIMHELPERFUNC_H

#ifdef __cplusplus
extern "C" {
#endif
typedef char *CMD_String[64] ;
void Update_UI_Commands();
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
enum VisItCommandTypes { STATUS_MESSAGE = 9, TIME_LABEL,TIME_VALUE, STEP_LABEL, STEP_VALUE};
#ifdef __cplusplus
}
#endif
#endif
