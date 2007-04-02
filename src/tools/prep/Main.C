/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                                Main.C                                     //
// ************************************************************************* //

//
// Includes
//

#include <MeshtvPrep.h>
#include <CommandLine.h>

#include <visitstream.h>
#include <new.h>
#include <stdlib.h>

#ifdef PARALLEL
    #include <mpi.h>
#endif


//
// Function Prototypes
//

static void    ExitHandler(void);
static void    NewHandler(void);


//
// Global Variables
//

#ifdef PARALLEL
  int    my_rank;
  int    num_processors;
#endif


// ****************************************************************************
//  Function: main
//
//  Purpose:
//      The main function for meshtvprep
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
//  Modifications:
//
//      Hank Childs, Wed Jun 14 10:21:48 PDT 2000
//      Added barrier so no processor would finalize before the rest were done.
//
//      Brad Whitlock, Fri Jun 23 11:13:53 PDT 2000
//      Added a check to prevent processors other than rank 0 from printing
//      usage information. Also added call to MPI_Finalize before the exit
//      call so it doesn't look like the program crashed.
//
// ****************************************************************************

int main(int argc, char *argv[])
{
#ifdef PARALLEL
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processors);
#endif

    //
    // Set up handlers
    //
    atexit(ExitHandler);
    set_new_handler(NewHandler);

    CommandLine  cl;
    if (cl.Parse(argc, argv, cerr) < 0)
    {
#ifdef PARALLEL
        if(my_rank == 0)
            cl.Usage(cerr);
        MPI_Finalize();
        exit(EXIT_FAILURE);
#else
        cl.Usage(cerr);
        exit(EXIT_FAILURE);
#endif
    }

    //
    //  Do the preprocessing.  The MeshtvPrep object takes care of this.
    //
    MeshtvPrep  meshtvprep;
    meshtvprep.PreProcess(cl);

#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
#endif
}


// ****************************************************************************
//  Function: ExitHandler
//
//  Purpose:  
//      Called at exit.  This will wrap up parallel issues if need be.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
// ****************************************************************************

void
ExitHandler(void)
{
#ifdef PARALLEL
    //
    // This should take care of finalizing, but it just ends up causing 
    // problems for some MPI implementations.
    //
    //MPI_Finalize();
#endif
}


// ****************************************************************************
//  Function: NewHandler
//
//  Purpose:
//      Called if any new fails to obtain memory.  Currently does not try
//      to handle any errors, just prints a message and exits.
//
//  Programmer: Hank Childs
//  Creation:   December 1, 1999
//
// ****************************************************************************

void 
NewHandler(void)
{
    cerr << "meshtvprep has run out of memory... (bailing out)" << endl;
    exit(EXIT_FAILURE);
}


