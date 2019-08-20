// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtTiledImageCompositor.C                      //
// ************************************************************************* //
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include <avtParallel.h>
#include <avtTiledImageCompositor.h>
#include <vtkImageData.h>
#include <ImproperUseException.h>
#include <vector>
using std::vector;

#ifdef PARALLEL
// ****************************************************************************
//  Function:  AreaOwned
//
//  Purpose:
//    Returns the x,y lower (inclusive) limit and the x,y upper (exclusive)
//    limit on the region owned by any particular processor when doing
//    parallel scalable rendering with transparency and tiled image
//    compositing.
//
//  NOTE:  THIS CODE IS DUPLICATED IN vtkParallelImageSpaceRedistributor.C
//  TODO -- put this code in 1 place
//  TODO -- what happens there are more ranks than the image is tall??
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  1, 2004
//
//  Modifications:
//
//    Burlen Loring,  Thu Oct  8 10:50:10 PDT 2015
//    use array to store the extent
//
// ****************************************************************************
static
void AreaOwned(int rank, int nranks, int w, int h, int *ext)
{
    ext[0] = 0;
    ext[1] = w - 1;
    ext[2] = (h*rank)/nranks;
    ext[3] = ((h*(rank+1))/nranks) - 1;
}
#endif

// ****************************************************************************
//  Constructor: avtTiledImageCompositor::avtTiledImageCompositor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2004
//
//  Modifications:
//
//    Burlen Loring, Thu Oct  8 10:49:35 PDT 2015
//    Use an initializer list
//
// ****************************************************************************

avtTiledImageCompositor::avtTiledImageCompositor()
    : chunkSize(1000000), bcastResult(false)
{}


// ****************************************************************************
//  Destructor: avtTiledImageCompositor::~avtTiledImageCompositor
//
//  Programmer: Jeremy Meredith
//  Creation:   Mon Aug 30 15:53:33 PDT 2004
//
// ****************************************************************************

avtTiledImageCompositor::~avtTiledImageCompositor()
{
   inputImages.clear();
}


// ****************************************************************************
//  Method: Execute
//
//  Purpose: Perform the composition
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2004
//
//  Modifications:
//
//    Hank Childs, Sun Mar 13 11:19:18 PST 2005
//    Fix memory leak.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Burlen Loring, Fri Sep 11 01:15:02 PDT 2015
//    Eliminated communication of tile sizes as these can
//    be computed locally. Added the ability to broadcast
//    the result to all ranks.
//
// ****************************************************************************

int getNumPixels(int *ext)
{ return (ext[1] - ext[0] + 1)*(ext[3] - ext[2] + 1); }

void
avtTiledImageCompositor::Execute(void)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
    int nranks = PAR_Size();

    if (inputImages.size() != 1)
    {
        EXCEPTION1(ImproperUseException, "avtTiledImageCompositor expected "
                   "only a single input image per processor.");
    }

    // Get the whole image nranks
    int width, height;
    inputImages[0]->GetImage().GetSize(&height, &width);

    // get the tile extents
    vector<int> offs(nranks, 0);
    vector<int> sizes(nranks, 0);
    vector<int> tile(4*nranks, 0);
    for (int i = 0, q = 0; i < nranks; ++i)
    {
        AreaOwned(i, nranks, width, height, &tile[4*i]);
        int n = 3*getNumPixels(&tile[4*i]);
        sizes[i] = n;
        offs[i] = q;
        q += n;
    }

    // Create an output image if we are the root process
    vtkImageData *outputImageData = NULL;
    unsigned char *outrgb = NULL;
    if (bcastResult || (rank == mpiRoot))
    {
        outputImageData = avtImageRepresentation::NewImage(width, height);
        outrgb = (unsigned char *)outputImageData->GetScalarPointer(0,0,0);
    }

    // get the input
    unsigned char *inrgb = inputImages[0]->GetImage().GetRGBBuffer();

    if (bcastResult)
    {
        MPI_Allgatherv(inrgb + offs[rank], sizes[rank] , MPI_UNSIGNED_CHAR,
            outrgb, &sizes[0], &offs[0], MPI_UNSIGNED_CHAR, VISIT_MPI_COMM);
    }
    else
    {
        MPI_Gatherv(inrgb + offs[rank], sizes[rank] , MPI_UNSIGNED_CHAR,
            outrgb, &sizes[0], &offs[0], MPI_UNSIGNED_CHAR, mpiRoot,
            VISIT_MPI_COMM);
    }

    // Set the output
    avtImageRepresentation theOutput(outputImageData);
    SetOutputImage(theOutput);

    // Free the memory
    if (outputImageData != NULL)
        outputImageData->Delete();
#else
    SetOutputImage(inputImages[0]->GetImage());
#endif
}
