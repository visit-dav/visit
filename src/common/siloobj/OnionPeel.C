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
//                               OnionPeel.C                                 //
// ************************************************************************* //

#include <string.h>

#include "OnionPeel.h"


//
// Initialization of static consts.  They are not allowed as initializers in
// class definitions.
//

char * const  OnionPeel::SILO_BLOCK_NAME   = "block";
char * const  OnionPeel::SILO_IDENTIFIER   = "OnionPeel";
char * const  OnionPeel::SILO_LIST_NAME    = "list";
char * const  OnionPeel::SILO_OFFSET_NAME  = "offset";


// ****************************************************************************
//  Method: OnionPeel constructor
//
//  Programmer: Hank Childs
//  Creation:   January 5, 1999
//
// ****************************************************************************

OnionPeel::OnionPeel()
{
    domain   = -1;
    list     = NULL;
    meshName = NULL;
    nnodes   = 0;
    offset   = NULL;
}


// ****************************************************************************
//  Method: OnionPeel destructor
//
//  Programmer: Hank Childs
//  Creation:   January 5, 1999
//
// ****************************************************************************

OnionPeel::~OnionPeel()
{
    if (list != NULL)
    {
        delete [] list;
    }
    if (meshName != NULL)
    {
        delete [] meshName;
    }
    if (offset != NULL)
    {
        delete [] offset;
    }
}


// ****************************************************************************
//  Method: OnionPeel::AddMesh
//
//  Purpose:
//      Adds an ucd mesh to the onion peel.
//
//  Note:       Code borrowed heavily from /meshtvx/mwd/segment.c
//              routine init_global_structures.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 1999
//
// ****************************************************************************

void
OnionPeel::AddMesh(DBucdmesh *um, int d)
{
    int   i, j, k;

    domain      = d;
    meshName    = new char[strlen(um->name)+strlen(SILO_IDENTIFIER)+1];
    sprintf(meshName, "%s%s", um->name, SILO_IDENTIFIER);
    nnodes      = um->nnodes;

    int   *size = new int[nnodes];
    for (i = 0 ; i < nnodes ; i++)
    {
        size[i] = 0;
    }

    //
    // Count each occurrence of a vertex.  
    //
    int   curZone = 0;
    int   listPos = 0;
    for (i = 0 ; i < um->zones->nshapes ; i++)
    {
        for (j = 0 ; j < um->zones->shapecnt[i] ; j++)
        {
            if (curZone >= um->zones->min_index && 
                curZone <= um->zones->max_index)
            {
                for (k = 0 ; k < um->zones->shapesize[i] ; k++)
                {
                    int  node = um->zones->nodelist[listPos+k];
                    size[node]++;

                }     // end 'for' over vertices in a polygon.
                listPos += um->zones->shapesize[i];
                curZone++;

            }     // end 'for' over all the polygons of a set topology.
            else
            {
                //
                // This is a phony zone.
                //
                curZone++;
                listPos += um->zones->shapesize[i];
            }
        }     // end 'for' over all the different topologies in this mesh.
    }

    //
    // We are going to store the list of zones as a 1-dimensional mesh, 
    // so create a second array that will be offsets into the array.
    //
    // The automatic variable size now becomes a counter for where the last
    // used position is inside each list.
    //
    offset = new int[nnodes+1];
    offset[0] = 0;
    for (i = 1 ; i <= um->nnodes ; i++)
    {
        offset[i] = offset[i-1] + size[i-1];
        size[i-1] = 0;
    }
    
    //
    // offset[nnodes] = total size of the list when compacted to one dimension.
    //
    list   = new int[offset[nnodes]];
    
    //
    // Now we populate the list with the zones for each node.
    //
    curZone = 0;
    listPos = 0;
    for (i = 0 ; i < um->zones->nshapes ; i++)
    {
        for (j = 0 ; j < um->zones->shapecnt[i] ; j++)
        {
            if (curZone >= um->zones->min_index && 
                curZone <= um->zones->max_index)
            {
                for (k = 0 ; k < um->zones->shapesize[i] ; k++)
                {
                    int  node = um->zones->nodelist[listPos+k];
                    list[offset[node] + size[node]] = curZone ;
                    size[node]++;

                }     // end 'for' over vertices in a polygon.
                listPos += um->zones->shapesize[i];
                curZone++;

            }     // end 'for' over all the polygons of a set topology.
            else
            {
                //
                // This is a phony zone.
                //
                curZone++;
                listPos += um->zones->shapesize[i];
            }
        }     // end 'for' over all the different topologies in this mesh.
    }
    
    delete [] size;
}


// ****************************************************************************
//  Method: OnionPeel::Write
//
//  Purpose:
//      Writes the OnionPeel information out to a file.
//
//  Programmer: Hank Childs
//  Creation:   January 5, 2000
//
// ****************************************************************************

void
OnionPeel::Write(DBfile *dbfile)
{
    //
    // Make sure the directory structure is set up for writing this object.
    //
    DBSetDir(dbfile, "/");
    DBtoc   *toc = DBGetToc(dbfile);
    bool     foundDir = false;
    for (int i = 0 ; i < toc->ndir ; i++)
    {
        if (strcmp(meshName, toc->dir_names[i]) == 0)
        {
            foundDir = true;
            break;
        }
    }
    if (foundDir == false)
    {
        DBMkDir(dbfile, meshName);
    }

    DBSetDir(dbfile, meshName);

    //
    // Create a new directory for this domain.
    //
    char  domDirectory[100];  // Only needs strlen("block")+strlen(MAXINT)
    sprintf(domDirectory, "%s%d", SILO_BLOCK_NAME, domain);
    DBMkDir(dbfile, domDirectory);
    DBSetDir(dbfile, domDirectory);

    // 
    // We're in the correct directory, so we can write the arrays.
    //
    int   dims = nnodes;
    DBWrite(dbfile, SILO_OFFSET_NAME, offset, &dims, 1, DB_INT);
    dims  = offset[nnodes];
    DBWrite(dbfile, SILO_LIST_NAME, list, &dims, 1, DB_INT);
}


