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
//
//  Programmer:  Jeremy Meredith
//  Creation:    September  1, 2004
//
// ****************************************************************************
static void AreaOwned(int rank, int size, int w, int h,
                      int &x1,int &y1, int &x2,int &y2)
{
    x1 = 0;
    x2 = w;
    y1 = (h*rank)/size;
    y2 = ((h*(rank+1))/size);
}

// ****************************************************************************
//  Constructor: avtTiledImageCompositor::avtTiledImageCompositor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 30, 2004
//
//  Modifications:
//
// ****************************************************************************

avtTiledImageCompositor::avtTiledImageCompositor()
{
   chunkSize = 1000000;
}


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
// ****************************************************************************

void
avtTiledImageCompositor::Execute(void)
{
#ifdef PARALLEL
    int rank = PAR_Rank();
    int size = PAR_Size();

    if (inputImages.size() != 1)
    {
        EXCEPTION1(ImproperUseException, "avtTiledImageCompositor expected "
                   "only a single input image per processor.");
    }

    // Get the whole image size
    int width, height;
    inputImages[0]->GetImage().GetSize(&height, &width);

    // Figure out how much of the screen I own
    int x1,y1,x2,y2;
    AreaOwned(rank, size, width, height, x1,y1,x2,y2);
    int mywidth  = x2 - x1;
    int myheight = y2 - y1;

    unsigned char *inrgb = inputImages[0]->GetImage().GetRGBBuffer();

    // Create an output image if we are the root process
    vtkImageData *outputImageData = NULL;
    unsigned char *outrgb = NULL;
    if (rank == mpiRoot)
    {
        outputImageData = avtImageRepresentation::NewImage(width, height);
        outrgb = (unsigned char *)outputImageData->GetScalarPointer(0,0,0);
    }

    // Determine how many pixels need to be sent by each process
    // Note -- count is for each RGB component separately (thus the "*3")
    int pixelSize = mywidth * myheight * 3;
    int *pixelSizes = (rank == mpiRoot) ? new int[size] : NULL;
    MPI_Gather(&pixelSize, 1, MPI_INT,  pixelSizes, 1, MPI_INT,
               mpiRoot, MPI_COMM_WORLD);

    // Count 'em up
    // ... okay, so there's probably no point ....

    // Gather the pixels
    int *displacements = NULL;
    if (rank == mpiRoot)
    {
        displacements = new int[size];
        displacements[0] = 0;
        for (int i=1; i<size; i++)
        {
            displacements[i] = displacements[i-1] + pixelSizes[i-1];
        }
    }
    // NOTE: assumes all pixels are contiguous in memory!
    MPI_Gatherv(&inrgb[3*(width*y1+x1)], pixelSize, MPI_UNSIGNED_CHAR,
                outrgb, pixelSizes, displacements, MPI_UNSIGNED_CHAR,
                mpiRoot, MPI_COMM_WORLD);


    // Set the output
    avtImageRepresentation theOutput(outputImageData);
    SetOutputImage(theOutput);

    // Free the memory (yes, it is safe not to check for NULL in C++)
    delete[] pixelSizes;
    delete[] displacements;
#else
    SetOutputImage(inputImages[0]->GetImage());
#endif
}
