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
//                        StructuredTopology_Prep.C                          //
// ************************************************************************* //

#include <visitstream.h>
#include <stdlib.h>
#include <string.h>

#ifdef PARALLEL
    #include <mpi.h>
#endif

#include <SiloObjLib.h>
#include <StructuredTopology_Prep.h>


//
// Class-scoped constants
//

char * const    StructuredTopology_Prep::SILO_LOCATION           = "/";


// ****************************************************************************
//  Method: StructuredTopology_Prep constructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

StructuredTopology_Prep::StructuredTopology_Prep()
{
    dimensionsSize = -1;
    readDimension  = false;
}


// ****************************************************************************
//  Method: StructuredTopology_Prep destructor
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

StructuredTopology_Prep::~StructuredTopology_Prep()
{
    // Do nothing
}


// ****************************************************************************
//  Method: StructuredTopology_Prep::Consolidate
//
//  Purpose:
//      Has the object do additional pre-processing after the initial pass is
//      completed.   
//
//  Note:        This routine currently does nothing.
//
//  Programmer:  Hank Childs
//  Creation:    January 26, 2000
//
// ****************************************************************************

void
StructuredTopology_Prep::Consolidate(void)
{
    // Do nothing
}


// ****************************************************************************
//  Method: StructuredTopology_Prep::WrapUp
//
//  Purpose:
//      Has the object 'wrap up'.  A place to do further processing after all
//      of the information is collected.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
StructuredTopology_Prep::WrapUp(void)
{
#ifdef PARALLEL
    //
    // The dimensions for each domain are spread across all of the processors.
    // Reduce this information to processor 0.  Because all of the arrays have
    // been initialized (to zero), we can sum the arrays.
    //
    // The MPI_IN_PLACE flag does not appear to be valid for this 
    // implementation of MPI, so create a receiving buffer.
    //
    extern   int num_processors;
    extern   int my_rank;
    if (num_processors > 1)
    {
        float   *out = new float[dimensionsSize];
        MPI_Allreduce(dimensions, out, dimensionsSize, MPI_FLOAT,
                      MPI_SUM, VISIT_MPI_COMM);
        for (int i = 0 ; i < dimensionsSize ; i++)
        {
            dimensions[i] = out[i];
        }
        delete [] out;
    }
#endif
}


// ****************************************************************************
//  Method: StructuredTopology_Prep::Write
//
//  Purpose:
//      Writes out the structured topology object to a silo file.
//
//  Note:       The type field for the DBobject, siloObj, must have its memory
//              allocated through a malloc, so that when it is freed, we do
//              not get a free memory mismatch from purify.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
StructuredTopology_Prep::Write(DBfile *dbfile)
{
#ifdef PARALLEL
    extern int   my_rank;
    if (my_rank != 0)
    {
        return;
    }
#endif

    if (DBSetDir(dbfile, SILO_LOCATION) < 0)
    {
        cerr << "Unable to change directories in the silo file to "
             << SILO_LOCATION << "." << endl;
        exit(EXIT_FAILURE);
    }
 
    StructuredTopology::Write(dbfile);
}


// ****************************************************************************
//  Method:  StructuredTopology_Prep::ReadMesh
//
//  Purpose:
//      Reads in the extents from the mesh.
// 
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
StructuredTopology_Prep::ReadMesh(DBquadmesh *qm, int domain)
{
    SetDimension(qm->ndims);

    int  offset = domain*nDimensions*2;
    for (int i = 0 ; i < qm->ndims ; i++)
    {
        dimensions[offset+(2*i)]   = qm->min_extents[i];
        dimensions[offset+(2*i)+1] = qm->max_extents[i];
    }
}


// ****************************************************************************
//  Method:  StructuredTopology_Prep::SetDimension
//
//  Purpose:
//      If the dimension has not already been set, this will set the dimension
//      and create the array.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2000
//
// ****************************************************************************

void
StructuredTopology_Prep::SetDimension(int n)
{
    if (readDimension == true)
    {
        return;
    }

    if (nDomains <= 0)
    {
        //
        // nDomains should have already been set.  This is a problem.
        //
        return;
    }

    nDimensions    = n;
    dimensionsSize = nDimensions*nDomains*2;
    dimensions = new float[dimensionsSize];
    for (int i = 0 ; i < dimensionsSize ; i++)
    {
        dimensions[i] = 0.;
    }

    readDimension = true;
}


// ****************************************************************************
//  Method: StructuredTopology_Prep::SetName
//
//  Purpose:
//      Sets the name of the object.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

void
StructuredTopology_Prep::SetName(char *n)
{
    name = CXX_strdup(n);
}


// ****************************************************************************
//  Method: StructuredTopology_Prep::SetDomains
//
//  Purpose:
//      Sets the number of domains for the object.
//
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

void
StructuredTopology_Prep::SetDomains(int n)
{
    nDomains = n;
}


