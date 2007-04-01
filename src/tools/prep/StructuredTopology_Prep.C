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
                      MPI_SUM, MPI_COMM_WORLD);
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


