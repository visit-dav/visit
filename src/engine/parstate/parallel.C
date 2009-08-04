/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifdef PARALLEL
#include <unistd.h>
#include <parallel.h>
#include <engine_parstate_exports.h>

ENGINE_PARSTATE_API MPI_Datatype PAR_STATEBUFFER;

// *******************************************************************
// Function: PAR_CreateTypes
//
// Purpose:
//   Creates an MPI type that represents the buffer that is
//   communicated when new state information is sent to non-UI processes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 12 16:00:57 PST 2000
//
// Modifications:
//   
// *******************************************************************

void
PAR_CreateTypes()
{
    PAR_StateBuffer buf;
    MPI_Aint        displacements[2];
    MPI_Datatype    types[2];
    int                lengths[2];

    //
    // Create a type matching the PAR_StateBuffer structure.  
    //
    MPI_Address (&buf.nbytes, &displacements[0]);
    MPI_Address (&buf.buffer, &displacements[1]);
    for (int i = 1; i >= 0; i--)
        displacements[i] -= displacements[0];

    types[0] = MPI_INT;
    types[1] = MPI_CHAR;
    lengths[0] = 1;
    lengths[1] = INPUT_BUFFER_SIZE;

    MPI_Type_struct (2, lengths, displacements, types, &PAR_STATEBUFFER);
    MPI_Type_commit (&PAR_STATEBUFFER);
}
#endif
