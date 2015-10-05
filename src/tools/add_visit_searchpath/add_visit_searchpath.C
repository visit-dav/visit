/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

// ***************************************************************************
//  Program: add_visit_searchpath
//
//  Purpose:
//    Add the variable "_visit_searchpath" with the specified search path
//    to a Silo file. If the search path is present then the Silo reader
//    will only search those directories for variables. This may speed up
//    the reading of the Silo file and improve the responsivness of VisIt
//    if the file has many variables stored in the directories being ignored.
//
//  Arguments:
//    Takes a Silo filename and an optional search path. If the search path
//    is not specified then it stores an empty search path, which will cause
//    all the directories to be ignored.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Oct  1 08:38:30 PDT 2015
//
// ***************************************************************************

#include <stdio.h>
#include <string.h>

#include <silo.h>

void
usage()
{
    fprintf(stderr, "Usage: add_visit_searchpath filename [searchpath]\n");
}

int
main(int argc, char *argv[])
{
    //
    // Parse the arguments.
    //
    char *filename = NULL, *searchpath = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-forceversion") == 0)
        {
            i++;
        }
        else
        {
            if (filename != NULL && searchpath != NULL)
            {
                usage();
                return 1;
            }

            if (filename != NULL)
                searchpath = argv[i];
            else
                filename = argv[i];
        }
    }
    if (filename == NULL)
    {
        usage();
        return 2;
    }

    //
    // Open the file.
    //
    DBfile *db = NULL;
    if ((db = DBOpen(filename, DB_HDF5, DB_APPEND)) == NULL)
    {
        fprintf(stderr, "Unable to open file %s\n", argv[1]);
        return 3;
    }

    //
    // Write the path.
    //
    if (searchpath == NULL)
    {
        int length = 1;
        DBWrite(db, "_visit_searchpath", ";", &length, 1, DB_CHAR);
    }
    else
    {
        int length = strlen(searchpath);
        DBWrite(db, "_visit_searchpath", searchpath, &length, 1, DB_CHAR);
    }

    //
    // Close the file.
    //
    DBClose(db);

    return 0;
}
