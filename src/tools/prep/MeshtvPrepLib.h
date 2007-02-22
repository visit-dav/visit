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

// ************************************************************************* //
//                             MeshtvPrepLib.h                               //
// ************************************************************************* //

#ifndef MESHTV_PREP_LIB
#define MESHTV_PREP_LIB

#include <silo.h>

#ifdef PARALLEL
#include <mpi.h>
#endif


//
// Enumerated types
//

typedef enum
{
    STATE_FILE                = 0,
    MESH_FILE,             // = 1
    MAX_RESOURCES          // = 2
} RESOURCE_e;


//
// Function prototypes
//

void        CreateSiloOutputFile(char *);
void        ForceControlToRoot(void);
int         GetOutputFileNum(int, int);
void        GetNFileRange(int *, int *);
void        GetDomainRange(int, int *, int *);
DBfile     *GetSiloOutputFile(char *);
void        NumToString(char *, int, int, int);
void        SetNFiles(int);
void        SetOutputDirectory(char *);
void        SortListByNumber(char **, int);
void        ObtainResource(RESOURCE_e);
void        RelinquishResource(RESOURCE_e);
bool        RootWithinGroup(RESOURCE_e);
void        SplitAmongProcessors(int, int *, int *);

#ifdef PARALLEL
MPI_Comm   &GetCommunicator(RESOURCE_e);
#endif


#endif


